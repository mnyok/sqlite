/************** Begin file delete.c ******************************************/
/*
** 2001 September 15
**
** The author disclaims copyright to this source code.  In place of
** a legal notice, here is a blessing:
**
**    May you do good and not evil.
**    May you find forgiveness for yourself and forgive others.
**    May you share freely, never taking more than you give.
**
*************************************************************************
** This file contains C code routines that are called by the parser
** in order to generate code for DELETE FROM statements.
*/
/* #include "sqliteInt.h" */

/*
** While a SrcList can in general represent multiple tables and subqueries
** (as in the FROM clause of a SELECT statement) in this case it contains
** the name of a single table, as one might find in an INSERT, DELETE,
** or UPDATE statement.  Look up that table in the symbol table and
** return a pointer.  Set an error message and return NULL if the table 
** name is not found or if any other error occurs.
**
** The following fields are initialized appropriate in pSrc:
**
**    pSrc->a[0].pTab       Pointer to the Table object
**    pSrc->a[0].pIndex     Pointer to the INDEXED BY index, if there is one
**
*/
SQLITE_PRIVATE Table *sqlite3SrcListLookup(Parse *pParse, SrcList *pSrc){
  struct SrcList_item *pItem = pSrc->a;
  Table *pTab;
  assert( pItem && pSrc->nSrc==1 );
  pTab = sqlite3LocateTableItem(pParse, 0, pItem);
  sqlite3DeleteTable(pParse->db, pItem->pTab);
  pItem->pTab = pTab;
  if( pTab ){
    pTab->nRef++;
  }
  if( sqlite3IndexedByLookup(pParse, pItem) ){
    pTab = 0;
  }
  return pTab;
}

/*
** Check to make sure the given table is writable.  If it is not
** writable, generate an error message and return 1.  If it is
** writable return 0;
*/
SQLITE_PRIVATE int sqlite3IsReadOnly(Parse *pParse, Table *pTab, int viewOk){
  /* A table is not writable under the following circumstances:
  **
  **   1) It is a virtual table and no implementation of the xUpdate method
  **      has been provided, or
  **   2) It is a system table (i.e. sqlite_master), this call is not
  **      part of a nested parse and writable_schema pragma has not 
  **      been specified.
  **
  ** In either case leave an error message in pParse and return non-zero.
  */
  if( ( IsVirtual(pTab) 
     && sqlite3GetVTable(pParse->db, pTab)->pMod->pModule->xUpdate==0 )
   || ( (pTab->tabFlags & TF_Readonly)!=0
     && (pParse->db->flags & SQLITE_WriteSchema)==0
     && pParse->nested==0 )
  ){
    sqlite3ErrorMsg(pParse, "table %s may not be modified", pTab->zName);
    return 1;
  }

#ifndef SQLITE_OMIT_VIEW
  if( !viewOk && pTab->pSelect ){
    sqlite3ErrorMsg(pParse,"cannot modify %s because it is a view",pTab->zName);
    return 1;
  }
#endif
  return 0;
}


#if !defined(SQLITE_OMIT_VIEW) && !defined(SQLITE_OMIT_TRIGGER)
/*
** Evaluate a view and store its result in an ephemeral table.  The
** pWhere argument is an optional WHERE clause that restricts the
** set of rows in the view that are to be added to the ephemeral table.
*/
SQLITE_PRIVATE void sqlite3MaterializeView(
  Parse *pParse,       /* Parsing context */
  Table *pView,        /* View definition */
  Expr *pWhere,        /* Optional WHERE clause to be added */
  int iCur             /* Cursor number for ephemeral table */
){
  SelectDest dest;
  Select *pSel;
  SrcList *pFrom;
  sqlite3 *db = pParse->db;
  int iDb = sqlite3SchemaToIndex(db, pView->pSchema);
  pWhere = sqlite3ExprDup(db, pWhere, 0);
  pFrom = sqlite3SrcListAppend(db, 0, 0, 0);
  if( pFrom ){
    assert( pFrom->nSrc==1 );
    pFrom->a[0].zName = sqlite3DbStrDup(db, pView->zName);
    pFrom->a[0].zDatabase = sqlite3DbStrDup(db, db->aDb[iDb].zName);
    assert( pFrom->a[0].pOn==0 );
    assert( pFrom->a[0].pUsing==0 );
  }
  pSel = sqlite3SelectNew(pParse, 0, pFrom, pWhere, 0, 0, 0, 
                          SF_IncludeHidden, 0, 0);
  sqlite3SelectDestInit(&dest, SRT_EphemTab, iCur);
  sqlite3Select(pParse, pSel, &dest);
  sqlite3SelectDelete(db, pSel);
}
#endif /* !defined(SQLITE_OMIT_VIEW) && !defined(SQLITE_OMIT_TRIGGER) */

#if defined(SQLITE_ENABLE_UPDATE_DELETE_LIMIT) && !defined(SQLITE_OMIT_SUBQUERY)
/*
** Generate an expression tree to implement the WHERE, ORDER BY,
** and LIMIT/OFFSET portion of DELETE and UPDATE statements.
**
**     DELETE FROM table_wxyz WHERE a<5 ORDER BY a LIMIT 1;
**                            \__________________________/
**                               pLimitWhere (pInClause)
*/
SQLITE_PRIVATE Expr *sqlite3LimitWhere(
  Parse *pParse,               /* The parser context */
  SrcList *pSrc,               /* the FROM clause -- which tables to scan */
  Expr *pWhere,                /* The WHERE clause.  May be null */
  ExprList *pOrderBy,          /* The ORDER BY clause.  May be null */
  Expr *pLimit,                /* The LIMIT clause.  May be null */
  Expr *pOffset,               /* The OFFSET clause.  May be null */
  char *zStmtType              /* Either DELETE or UPDATE.  For err msgs. */
){
  Expr *pWhereRowid = NULL;    /* WHERE rowid .. */
  Expr *pInClause = NULL;      /* WHERE rowid IN ( select ) */
  Expr *pSelectRowid = NULL;   /* SELECT rowid ... */
  ExprList *pEList = NULL;     /* Expression list contaning only pSelectRowid */
  SrcList *pSelectSrc = NULL;  /* SELECT rowid FROM x ... (dup of pSrc) */
  Select *pSelect = NULL;      /* Complete SELECT tree */

  /* Check that there isn't an ORDER BY without a LIMIT clause.
  */
  if( pOrderBy && (pLimit == 0) ) {
    sqlite3ErrorMsg(pParse, "ORDER BY without LIMIT on %s", zStmtType);
    goto limit_where_cleanup;
  }

  /* We only need to generate a select expression if there
  ** is a limit/offset term to enforce.
  */
  if( pLimit == 0 ) {
    /* if pLimit is null, pOffset will always be null as well. */
    assert( pOffset == 0 );
    return pWhere;
  }

  /* Generate a select expression tree to enforce the limit/offset 
  ** term for the DELETE or UPDATE statement.  For example:
  **   DELETE FROM table_a WHERE col1=1 ORDER BY col2 LIMIT 1 OFFSET 1
  ** becomes:
  **   DELETE FROM table_a WHERE rowid IN ( 
  **     SELECT rowid FROM table_a WHERE col1=1 ORDER BY col2 LIMIT 1 OFFSET 1
  **   );
  */

  pSelectRowid = sqlite3PExpr(pParse, TK_ROW, 0, 0, 0);
  if( pSelectRowid == 0 ) goto limit_where_cleanup;
  pEList = sqlite3ExprListAppend(pParse, 0, pSelectRowid);
  if( pEList == 0 ) goto limit_where_cleanup;

  /* duplicate the FROM clause as it is needed by both the DELETE/UPDATE tree
  ** and the SELECT subtree. */
  pSelectSrc = sqlite3SrcListDup(pParse->db, pSrc, 0);
  if( pSelectSrc == 0 ) {
    sqlite3ExprListDelete(pParse->db, pEList);
    goto limit_where_cleanup;
  }

  /* generate the SELECT expression tree. */
  pSelect = sqlite3SelectNew(pParse,pEList,pSelectSrc,pWhere,0,0,
                             pOrderBy,0,pLimit,pOffset);
  if( pSelect == 0 ) return 0;

  /* now generate the new WHERE rowid IN clause for the DELETE/UDPATE */
  pWhereRowid = sqlite3PExpr(pParse, TK_ROW, 0, 0, 0);
  pInClause = pWhereRowid ? sqlite3PExpr(pParse, TK_IN, pWhereRowid, 0, 0) : 0;
  sqlite3PExprAddSelect(pParse, pInClause, pSelect);
  return pInClause;

limit_where_cleanup:
  sqlite3ExprDelete(pParse->db, pWhere);
  sqlite3ExprListDelete(pParse->db, pOrderBy);
  sqlite3ExprDelete(pParse->db, pLimit);
  sqlite3ExprDelete(pParse->db, pOffset);
  return 0;
}
#endif /* defined(SQLITE_ENABLE_UPDATE_DELETE_LIMIT) */
       /*      && !defined(SQLITE_OMIT_SUBQUERY) */

/*
** Generate code for a DELETE FROM statement.
**
**     DELETE FROM table_wxyz WHERE a<5 AND b NOT NULL;
**                 \________/       \________________/
**                  pTabList              pWhere
*/
SQLITE_PRIVATE void sqlite3DeleteFrom(
  Parse *pParse,         /* The parser context */
  SrcList *pTabList,     /* The table from which we should delete things */
  Expr *pWhere           /* The WHERE clause.  May be null */
){
  Vdbe *v;               /* The virtual database engine */
  Table *pTab;           /* The table from which records will be deleted */
  const char *zDb;       /* Name of database holding pTab */
  int i;                 /* Loop counter */
  WhereInfo *pWInfo;     /* Information about the WHERE clause */
  Index *pIdx;           /* For looping over indices of the table */
  int iTabCur;           /* Cursor number for the table */
  int iDataCur = 0;      /* VDBE cursor for the canonical data source */
  int iIdxCur = 0;       /* Cursor number of the first index */
  int nIdx;              /* Number of indices */
  sqlite3 *db;           /* Main database structure */
  AuthContext sContext;  /* Authorization context */
  NameContext sNC;       /* Name context to resolve expressions in */
  int iDb;               /* Database number */
  int memCnt = -1;       /* Memory cell used for change counting */
  int rcauth;            /* Value returned by authorization callback */
  int eOnePass;          /* ONEPASS_OFF or _SINGLE or _MULTI */
  int aiCurOnePass[2];   /* The write cursors opened by WHERE_ONEPASS */
  u8 *aToOpen = 0;       /* Open cursor iTabCur+j if aToOpen[j] is true */
  Index *pPk;            /* The PRIMARY KEY index on the table */
  int iPk = 0;           /* First of nPk registers holding PRIMARY KEY value */
  i16 nPk = 1;           /* Number of columns in the PRIMARY KEY */
  int iKey;              /* Memory cell holding key of row to be deleted */
  i16 nKey;              /* Number of memory cells in the row key */
  int iEphCur = 0;       /* Ephemeral table holding all primary key values */
  int iRowSet = 0;       /* Register for rowset of rows to delete */
  int addrBypass = 0;    /* Address of jump over the delete logic */
  int addrLoop = 0;      /* Top of the delete loop */
  int addrEphOpen = 0;   /* Instruction to open the Ephemeral table */
  int bComplex;          /* True if there are triggers or FKs or
                         ** subqueries in the WHERE clause */
 
#ifndef SQLITE_OMIT_TRIGGER
  int isView;                  /* True if attempting to delete from a view */
  Trigger *pTrigger;           /* List of table triggers, if required */
#endif

  memset(&sContext, 0, sizeof(sContext));
  db = pParse->db;
  if( pParse->nErr || db->mallocFailed ){
    goto delete_from_cleanup;
  }
  assert( pTabList->nSrc==1 );

  /* Locate the table which we want to delete.  This table has to be
  ** put in an SrcList structure because some of the subroutines we
  ** will be calling are designed to work with multiple tables and expect
  ** an SrcList* parameter instead of just a Table* parameter.
  */
  pTab = sqlite3SrcListLookup(pParse, pTabList);
  if( pTab==0 )  goto delete_from_cleanup;

  /* Figure out if we have any triggers and if the table being
  ** deleted from is a view
  */
#ifndef SQLITE_OMIT_TRIGGER
  pTrigger = sqlite3TriggersExist(pParse, pTab, TK_DELETE, 0, 0);
  isView = pTab->pSelect!=0;
  bComplex = pTrigger || sqlite3FkRequired(pParse, pTab, 0, 0);
#else
# define pTrigger 0
# define isView 0
#endif
#ifdef SQLITE_OMIT_VIEW
# undef isView
# define isView 0
#endif

  /* If pTab is really a view, make sure it has been initialized.
  */
  if( sqlite3ViewGetColumnNames(pParse, pTab) ){
    goto delete_from_cleanup;
  }

  if( sqlite3IsReadOnly(pParse, pTab, (pTrigger?1:0)) ){
    goto delete_from_cleanup;
  }
  iDb = sqlite3SchemaToIndex(db, pTab->pSchema);
  assert( iDb<db->nDb );
  zDb = db->aDb[iDb].zName;
  rcauth = sqlite3AuthCheck(pParse, SQLITE_DELETE, pTab->zName, 0, zDb);
  assert( rcauth==SQLITE_OK || rcauth==SQLITE_DENY || rcauth==SQLITE_IGNORE );
  if( rcauth==SQLITE_DENY ){
    goto delete_from_cleanup;
  }
  assert(!isView || pTrigger);

  /* Assign cursor numbers to the table and all its indices.
  */
  assert( pTabList->nSrc==1 );
  iTabCur = pTabList->a[0].iCursor = pParse->nTab++;
  for(nIdx=0, pIdx=pTab->pIndex; pIdx; pIdx=pIdx->pNext, nIdx++){
    pParse->nTab++;
  }

  /* Start the view context
  */
  if( isView ){
    sqlite3AuthContextPush(pParse, &sContext, pTab->zName);
  }

  /* Begin generating code.
  */
  v = sqlite3GetVdbe(pParse);
  if( v==0 ){
    goto delete_from_cleanup;
  }
  if( pParse->nested==0 ) sqlite3VdbeCountChanges(v);
  sqlite3BeginWriteOperation(pParse, 1, iDb);

  /* If we are trying to delete from a view, realize that view into
  ** an ephemeral table.
  */
#if !defined(SQLITE_OMIT_VIEW) && !defined(SQLITE_OMIT_TRIGGER)
  if( isView ){
    sqlite3MaterializeView(pParse, pTab, pWhere, iTabCur);
    iDataCur = iIdxCur = iTabCur;
  }
#endif

  /* Resolve the column names in the WHERE clause.
  */
  memset(&sNC, 0, sizeof(sNC));
  sNC.pParse = pParse;
  sNC.pSrcList = pTabList;
  if( sqlite3ResolveExprNames(&sNC, pWhere) ){
    goto delete_from_cleanup;
  }

  /* Initialize the counter of the number of rows deleted, if
  ** we are counting rows.
  */
  if( db->flags & SQLITE_CountRows ){
    memCnt = ++pParse->nMem;
    sqlite3VdbeAddOp2(v, OP_Integer, 0, memCnt);
  }

#ifndef SQLITE_OMIT_TRUNCATE_OPTIMIZATION
  /* Special case: A DELETE without a WHERE clause deletes everything.
  ** It is easier just to erase the whole table. Prior to version 3.6.5,
  ** this optimization caused the row change count (the value returned by 
  ** API function sqlite3_count_changes) to be set incorrectly.  */
  if( rcauth==SQLITE_OK
   && pWhere==0
   && !bComplex
   && !IsVirtual(pTab)
#ifdef SQLITE_ENABLE_PREUPDATE_HOOK
   && db->xPreUpdateCallback==0
#endif
  ){
    assert( !isView );
    sqlite3TableLock(pParse, iDb, pTab->tnum, 1, pTab->zName);
    if( HasRowid(pTab) ){
      sqlite3VdbeAddOp4(v, OP_Clear, pTab->tnum, iDb, memCnt,
                        pTab->zName, P4_STATIC);
    }
    for(pIdx=pTab->pIndex; pIdx; pIdx=pIdx->pNext){
      assert( pIdx->pSchema==pTab->pSchema );
      sqlite3VdbeAddOp2(v, OP_Clear, pIdx->tnum, iDb);
    }
  }else
#endif /* SQLITE_OMIT_TRUNCATE_OPTIMIZATION */
  {
    u16 wcf = WHERE_ONEPASS_DESIRED|WHERE_DUPLICATES_OK|WHERE_SEEK_TABLE;
    if( sNC.ncFlags & NC_VarSelect ) bComplex = 1;
    wcf |= (bComplex ? 0 : WHERE_ONEPASS_MULTIROW);
    if( HasRowid(pTab) ){
      /* For a rowid table, initialize the RowSet to an empty set */
      pPk = 0;
      nPk = 1;
      iRowSet = ++pParse->nMem;
      sqlite3VdbeAddOp2(v, OP_Null, 0, iRowSet);
    }else{
      /* For a WITHOUT ROWID table, create an ephemeral table used to
      ** hold all primary keys for rows to be deleted. */
      pPk = sqlite3PrimaryKeyIndex(pTab);
      assert( pPk!=0 );
      nPk = pPk->nKeyCol;
      iPk = pParse->nMem+1;
      pParse->nMem += nPk;
      iEphCur = pParse->nTab++;
      addrEphOpen = sqlite3VdbeAddOp2(v, OP_OpenEphemeral, iEphCur, nPk);
      sqlite3VdbeSetP4KeyInfo(pParse, pPk);
    }
  
    /* Construct a query to find the rowid or primary key for every row
    ** to be deleted, based on the WHERE clause. Set variable eOnePass
    ** to indicate the strategy used to implement this delete:
    **
    **  ONEPASS_OFF:    Two-pass approach - use a FIFO for rowids/PK values.
    **  ONEPASS_SINGLE: One-pass approach - at most one row deleted.
    **  ONEPASS_MULTI:  One-pass approach - any number of rows may be deleted.
    */
    pWInfo = sqlite3WhereBegin(pParse, pTabList, pWhere, 0, 0, wcf, iTabCur+1);
    if( pWInfo==0 ) goto delete_from_cleanup;
    eOnePass = sqlite3WhereOkOnePass(pWInfo, aiCurOnePass);
    assert( IsVirtual(pTab)==0 || eOnePass!=ONEPASS_MULTI );
    assert( IsVirtual(pTab) || bComplex || eOnePass!=ONEPASS_OFF );
  
    /* Keep track of the number of rows to be deleted */
    if( db->flags & SQLITE_CountRows ){
      sqlite3VdbeAddOp2(v, OP_AddImm, memCnt, 1);
    }
  
    /* Extract the rowid or primary key for the current row */
    if( pPk ){
      for(i=0; i<nPk; i++){
        assert( pPk->aiColumn[i]>=0 );
        sqlite3ExprCodeGetColumnOfTable(v, pTab, iTabCur,
                                        pPk->aiColumn[i], iPk+i);
      }
      iKey = iPk;
    }else{
      iKey = pParse->nMem + 1;
      iKey = sqlite3ExprCodeGetColumn(pParse, pTab, -1, iTabCur, iKey, 0);
      if( iKey>pParse->nMem ) pParse->nMem = iKey;
    }
  
    if( eOnePass!=ONEPASS_OFF ){
      /* For ONEPASS, no need to store the rowid/primary-key. There is only
      ** one, so just keep it in its register(s) and fall through to the
      ** delete code.  */
      nKey = nPk; /* OP_Found will use an unpacked key */
      aToOpen = sqlite3DbMallocRawNN(db, nIdx+2);
      if( aToOpen==0 ){
        sqlite3WhereEnd(pWInfo);
        goto delete_from_cleanup;
      }
      memset(aToOpen, 1, nIdx+1);
      aToOpen[nIdx+1] = 0;
      if( aiCurOnePass[0]>=0 ) aToOpen[aiCurOnePass[0]-iTabCur] = 0;
      if( aiCurOnePass[1]>=0 ) aToOpen[aiCurOnePass[1]-iTabCur] = 0;
      if( addrEphOpen ) sqlite3VdbeChangeToNoop(v, addrEphOpen);
    }else{
      if( pPk ){
        /* Add the PK key for this row to the temporary table */
        iKey = ++pParse->nMem;
        nKey = 0;   /* Zero tells OP_Found to use a composite key */
        sqlite3VdbeAddOp4(v, OP_MakeRecord, iPk, nPk, iKey,
            sqlite3IndexAffinityStr(pParse->db, pPk), nPk);
        sqlite3VdbeAddOp2(v, OP_IdxInsert, iEphCur, iKey);
      }else{
        /* Add the rowid of the row to be deleted to the RowSet */
        nKey = 1;  /* OP_Seek always uses a single rowid */
        sqlite3VdbeAddOp2(v, OP_RowSetAdd, iRowSet, iKey);
      }
    }
  
    /* If this DELETE cannot use the ONEPASS strategy, this is the 
    ** end of the WHERE loop */
    if( eOnePass!=ONEPASS_OFF ){
      addrBypass = sqlite3VdbeMakeLabel(v);
    }else{
      sqlite3WhereEnd(pWInfo);
    }
  
    /* Unless this is a view, open cursors for the table we are 
    ** deleting from and all its indices. If this is a view, then the
    ** only effect this statement has is to fire the INSTEAD OF 
    ** triggers.
    */
    if( !isView ){
      int iAddrOnce = 0;
      if( eOnePass==ONEPASS_MULTI ){
        iAddrOnce = sqlite3CodeOnce(pParse); VdbeCoverage(v);
      }
      testcase( IsVirtual(pTab) );
      sqlite3OpenTableAndIndices(pParse, pTab, OP_OpenWrite, OPFLAG_FORDELETE,
                                 iTabCur, aToOpen, &iDataCur, &iIdxCur);
      assert( pPk || IsVirtual(pTab) || iDataCur==iTabCur );
      assert( pPk || IsVirtual(pTab) || iIdxCur==iDataCur+1 );
      if( eOnePass==ONEPASS_MULTI ) sqlite3VdbeJumpHere(v, iAddrOnce);
    }
  
    /* Set up a loop over the rowids/primary-keys that were found in the
    ** where-clause loop above.
    */
    if( eOnePass!=ONEPASS_OFF ){
      assert( nKey==nPk );  /* OP_Found will use an unpacked key */
      if( !IsVirtual(pTab) && aToOpen[iDataCur-iTabCur] ){
        assert( pPk!=0 || pTab->pSelect!=0 );
        sqlite3VdbeAddOp4Int(v, OP_NotFound, iDataCur, addrBypass, iKey, nKey);
        VdbeCoverage(v);
      }
    }else if( pPk ){
      addrLoop = sqlite3VdbeAddOp1(v, OP_Rewind, iEphCur); VdbeCoverage(v);
      sqlite3VdbeAddOp2(v, OP_RowKey, iEphCur, iKey);
      assert( nKey==0 );  /* OP_Found will use a composite key */
    }else{
      addrLoop = sqlite3VdbeAddOp3(v, OP_RowSetRead, iRowSet, 0, iKey);
      VdbeCoverage(v);
      assert( nKey==1 );
    }  
  
    /* Delete the row */
#ifndef SQLITE_OMIT_VIRTUALTABLE
    if( IsVirtual(pTab) ){
      const char *pVTab = (const char *)sqlite3GetVTable(db, pTab);
      sqlite3VtabMakeWritable(pParse, pTab);
      sqlite3VdbeAddOp4(v, OP_VUpdate, 0, 1, iKey, pVTab, P4_VTAB);
      sqlite3VdbeChangeP5(v, OE_Abort);
      assert( eOnePass==ONEPASS_OFF || eOnePass==ONEPASS_SINGLE );
      sqlite3MayAbort(pParse);
      if( eOnePass==ONEPASS_SINGLE && sqlite3IsToplevel(pParse) ){
        pParse->isMultiWrite = 0;
      }
    }else
#endif
    {
      int count = (pParse->nested==0);    /* True to count changes */
      int iIdxNoSeek = -1;
      if( bComplex==0 && aiCurOnePass[1]!=iDataCur ){
        iIdxNoSeek = aiCurOnePass[1];
      }
      sqlite3GenerateRowDelete(pParse, pTab, pTrigger, iDataCur, iIdxCur,
          iKey, nKey, count, OE_Default, eOnePass, iIdxNoSeek);
    }
  
    /* End of the loop over all rowids/primary-keys. */
    if( eOnePass!=ONEPASS_OFF ){
      sqlite3VdbeResolveLabel(v, addrBypass);
      sqlite3WhereEnd(pWInfo);
    }else if( pPk ){
      sqlite3VdbeAddOp2(v, OP_Next, iEphCur, addrLoop+1); VdbeCoverage(v);
      sqlite3VdbeJumpHere(v, addrLoop);
    }else{
      sqlite3VdbeGoto(v, addrLoop);
      sqlite3VdbeJumpHere(v, addrLoop);
    }     
  
    /* Close the cursors open on the table and its indexes. */
    if( !isView && !IsVirtual(pTab) ){
      if( !pPk ) sqlite3VdbeAddOp1(v, OP_Close, iDataCur);
      for(i=0, pIdx=pTab->pIndex; pIdx; i++, pIdx=pIdx->pNext){
        sqlite3VdbeAddOp1(v, OP_Close, iIdxCur + i);
      }
    }
  } /* End non-truncate path */

  /* Update the sqlite_sequence table by storing the content of the
  ** maximum rowid counter values recorded while inserting into
  ** autoincrement tables.
  */
  if( pParse->nested==0 && pParse->pTriggerTab==0 ){
    sqlite3AutoincrementEnd(pParse);
  }

  /* Return the number of rows that were deleted. If this routine is 
  ** generating code because of a call to sqlite3NestedParse(), do not
  ** invoke the callback function.
  */
  if( (db->flags&SQLITE_CountRows) && !pParse->nested && !pParse->pTriggerTab ){
    sqlite3VdbeAddOp2(v, OP_ResultRow, memCnt, 1);
    sqlite3VdbeSetNumCols(v, 1);
    sqlite3VdbeSetColName(v, 0, COLNAME_NAME, "rows deleted", SQLITE_STATIC);
  }

delete_from_cleanup:
  sqlite3AuthContextPop(&sContext);
  sqlite3SrcListDelete(db, pTabList);
  sqlite3ExprDelete(db, pWhere);
  sqlite3DbFree(db, aToOpen);
  return;
}
/* Make sure "isView" and other macros defined above are undefined. Otherwise
** they may interfere with compilation of other functions in this file
** (or in another file, if this file becomes part of the amalgamation).  */
#ifdef isView
 #undef isView
#endif
#ifdef pTrigger
 #undef pTrigger
#endif

/*
** This routine generates VDBE code that causes a single row of a
** single table to be deleted.  Both the original table entry and
** all indices are removed.
**
** Preconditions:
**
**   1.  iDataCur is an open cursor on the btree that is the canonical data
**       store for the table.  (This will be either the table itself,
**       in the case of a rowid table, or the PRIMARY KEY index in the case
**       of a WITHOUT ROWID table.)
**
**   2.  Read/write cursors for all indices of pTab must be open as
**       cursor number iIdxCur+i for the i-th index.
**
**   3.  The primary key for the row to be deleted must be stored in a
**       sequence of nPk memory cells starting at iPk.  If nPk==0 that means
**       that a search record formed from OP_MakeRecord is contained in the
**       single memory location iPk.
**
** eMode:
**   Parameter eMode may be passed either ONEPASS_OFF (0), ONEPASS_SINGLE, or
**   ONEPASS_MULTI.  If eMode is not ONEPASS_OFF, then the cursor
**   iDataCur already points to the row to delete. If eMode is ONEPASS_OFF
**   then this function must seek iDataCur to the entry identified by iPk
**   and nPk before reading from it.
**
**   If eMode is ONEPASS_MULTI, then this call is being made as part
**   of a ONEPASS delete that affects multiple rows. In this case, if 
**   iIdxNoSeek is a valid cursor number (>=0), then its position should
**   be preserved following the delete operation. Or, if iIdxNoSeek is not
**   a valid cursor number, the position of iDataCur should be preserved
**   instead.
**
** iIdxNoSeek:
**   If iIdxNoSeek is a valid cursor number (>=0), then it identifies an
**   index cursor (from within array of cursors starting at iIdxCur) that
**   already points to the index entry to be deleted.
*/
SQLITE_PRIVATE void sqlite3GenerateRowDelete(
  Parse *pParse,     /* Parsing context */
  Table *pTab,       /* Table containing the row to be deleted */
  Trigger *pTrigger, /* List of triggers to (potentially) fire */
  int iDataCur,      /* Cursor from which column data is extracted */
  int iIdxCur,       /* First index cursor */
  int iPk,           /* First memory cell containing the PRIMARY KEY */
  i16 nPk,           /* Number of PRIMARY KEY memory cells */
  u8 count,          /* If non-zero, increment the row change counter */
  u8 onconf,         /* Default ON CONFLICT policy for triggers */
  u8 eMode,          /* ONEPASS_OFF, _SINGLE, or _MULTI.  See above */
  int iIdxNoSeek     /* Cursor number of cursor that does not need seeking */
){
  Vdbe *v = pParse->pVdbe;        /* Vdbe */
  int iOld = 0;                   /* First register in OLD.* array */
  int iLabel;                     /* Label resolved to end of generated code */
  u8 opSeek;                      /* Seek opcode */

  /* Vdbe is guaranteed to have been allocated by this stage. */
  assert( v );
  VdbeModuleComment((v, "BEGIN: GenRowDel(%d,%d,%d,%d)",
                         iDataCur, iIdxCur, iPk, (int)nPk));

  /* Seek cursor iCur to the row to delete. If this row no longer exists 
  ** (this can happen if a trigger program has already deleted it), do
  ** not attempt to delete it or fire any DELETE triggers.  */
  iLabel = sqlite3VdbeMakeLabel(v);
  opSeek = HasRowid(pTab) ? OP_NotExists : OP_NotFound;
  if( eMode==ONEPASS_OFF ){
    sqlite3VdbeAddOp4Int(v, opSeek, iDataCur, iLabel, iPk, nPk);
    VdbeCoverageIf(v, opSeek==OP_NotExists);
    VdbeCoverageIf(v, opSeek==OP_NotFound);
  }
 
  /* If there are any triggers to fire, allocate a range of registers to
  ** use for the old.* references in the triggers.  */
  if( sqlite3FkRequired(pParse, pTab, 0, 0) || pTrigger ){
    u32 mask;                     /* Mask of OLD.* columns in use */
    int iCol;                     /* Iterator used while populating OLD.* */
    int addrStart;                /* Start of BEFORE trigger programs */

    /* TODO: Could use temporary registers here. Also could attempt to
    ** avoid copying the contents of the rowid register.  */
    mask = sqlite3TriggerColmask(
        pParse, pTrigger, 0, 0, TRIGGER_BEFORE|TRIGGER_AFTER, pTab, onconf
    );
    mask |= sqlite3FkOldmask(pParse, pTab);
    iOld = pParse->nMem+1;
    pParse->nMem += (1 + pTab->nCol);

    /* Populate the OLD.* pseudo-table register array. These values will be 
    ** used by any BEFORE and AFTER triggers that exist.  */
    sqlite3VdbeAddOp2(v, OP_Copy, iPk, iOld);
    for(iCol=0; iCol<pTab->nCol; iCol++){
      testcase( mask!=0xffffffff && iCol==31 );
      testcase( mask!=0xffffffff && iCol==32 );
      if( mask==0xffffffff || (iCol<=31 && (mask & MASKBIT32(iCol))!=0) ){
        sqlite3ExprCodeGetColumnOfTable(v, pTab, iDataCur, iCol, iOld+iCol+1);
      }
    }

    /* Invoke BEFORE DELETE trigger programs. */
    addrStart = sqlite3VdbeCurrentAddr(v);
    sqlite3CodeRowTrigger(pParse, pTrigger, 
        TK_DELETE, 0, TRIGGER_BEFORE, pTab, iOld, onconf, iLabel
    );

    /* If any BEFORE triggers were coded, then seek the cursor to the 
    ** row to be deleted again. It may be that the BEFORE triggers moved
    ** the cursor or of already deleted the row that the cursor was
    ** pointing to.
    */
    if( addrStart<sqlite3VdbeCurrentAddr(v) ){
      sqlite3VdbeAddOp4Int(v, opSeek, iDataCur, iLabel, iPk, nPk);
      VdbeCoverageIf(v, opSeek==OP_NotExists);
      VdbeCoverageIf(v, opSeek==OP_NotFound);
    }

    /* Do FK processing. This call checks that any FK constraints that
    ** refer to this table (i.e. constraints attached to other tables) 
    ** are not violated by deleting this row.  */
    sqlite3FkCheck(pParse, pTab, iOld, 0, 0, 0);
  }

  /* Delete the index and table entries. Skip this step if pTab is really
  ** a view (in which case the only effect of the DELETE statement is to
  ** fire the INSTEAD OF triggers).  
  **
  ** If variable 'count' is non-zero, then this OP_Delete instruction should
  ** invoke the update-hook. The pre-update-hook, on the other hand should
  ** be invoked unless table pTab is a system table. The difference is that
  ** the update-hook is not invoked for rows removed by REPLACE, but the 
  ** pre-update-hook is.
  */ 
  if( pTab->pSelect==0 ){
    u8 p5 = 0;
    sqlite3GenerateRowIndexDelete(pParse, pTab, iDataCur, iIdxCur,0,iIdxNoSeek);
    sqlite3VdbeAddOp2(v, OP_Delete, iDataCur, (count?OPFLAG_NCHANGE:0));
    sqlite3VdbeChangeP4(v, -1, (char*)pTab, P4_TABLE);
    if( eMode!=ONEPASS_OFF ){
      sqlite3VdbeChangeP5(v, OPFLAG_AUXDELETE);
    }
    if( iIdxNoSeek>=0 ){
      sqlite3VdbeAddOp1(v, OP_Delete, iIdxNoSeek);
    }
    if( eMode==ONEPASS_MULTI ) p5 |= OPFLAG_SAVEPOSITION;
    sqlite3VdbeChangeP5(v, p5);
  }

  /* Do any ON CASCADE, SET NULL or SET DEFAULT operations required to
  ** handle rows (possibly in other tables) that refer via a foreign key
  ** to the row just deleted. */ 
  sqlite3FkActions(pParse, pTab, 0, iOld, 0, 0);

  /* Invoke AFTER DELETE trigger programs. */
  sqlite3CodeRowTrigger(pParse, pTrigger, 
      TK_DELETE, 0, TRIGGER_AFTER, pTab, iOld, onconf, iLabel
  );

  /* Jump here if the row had already been deleted before any BEFORE
  ** trigger programs were invoked. Or if a trigger program throws a 
  ** RAISE(IGNORE) exception.  */
  sqlite3VdbeResolveLabel(v, iLabel);
  VdbeModuleComment((v, "END: GenRowDel()"));
}

/*
** This routine generates VDBE code that causes the deletion of all
** index entries associated with a single row of a single table, pTab
**
** Preconditions:
**
**   1.  A read/write cursor "iDataCur" must be open on the canonical storage
**       btree for the table pTab.  (This will be either the table itself
**       for rowid tables or to the primary key index for WITHOUT ROWID
**       tables.)
**
**   2.  Read/write cursors for all indices of pTab must be open as
**       cursor number iIdxCur+i for the i-th index.  (The pTab->pIndex
**       index is the 0-th index.)
**
**   3.  The "iDataCur" cursor must be already be positioned on the row
**       that is to be deleted.
*/
SQLITE_PRIVATE void sqlite3GenerateRowIndexDelete(
  Parse *pParse,     /* Parsing and code generating context */
  Table *pTab,       /* Table containing the row to be deleted */
  int iDataCur,      /* Cursor of table holding data. */
  int iIdxCur,       /* First index cursor */
  int *aRegIdx,      /* Only delete if aRegIdx!=0 && aRegIdx[i]>0 */
  int iIdxNoSeek     /* Do not delete from this cursor */
){
  int i;             /* Index loop counter */
  int r1 = -1;       /* Register holding an index key */
  int iPartIdxLabel; /* Jump destination for skipping partial index entries */
  Index *pIdx;       /* Current index */
  Index *pPrior = 0; /* Prior index */
  Vdbe *v;           /* The prepared statement under construction */
  Index *pPk;        /* PRIMARY KEY index, or NULL for rowid tables */

  v = pParse->pVdbe;
  pPk = HasRowid(pTab) ? 0 : sqlite3PrimaryKeyIndex(pTab);
  for(i=0, pIdx=pTab->pIndex; pIdx; i++, pIdx=pIdx->pNext){
    assert( iIdxCur+i!=iDataCur || pPk==pIdx );
    if( aRegIdx!=0 && aRegIdx[i]==0 ) continue;
    if( pIdx==pPk ) continue;
    if( iIdxCur+i==iIdxNoSeek ) continue;
    VdbeModuleComment((v, "GenRowIdxDel for %s", pIdx->zName));
    r1 = sqlite3GenerateIndexKey(pParse, pIdx, iDataCur, 0, 1,
        &iPartIdxLabel, pPrior, r1);
    sqlite3VdbeAddOp3(v, OP_IdxDelete, iIdxCur+i, r1,
        pIdx->uniqNotNull ? pIdx->nKeyCol : pIdx->nColumn);
    sqlite3ResolvePartIdxLabel(pParse, iPartIdxLabel);
    pPrior = pIdx;
  }
}

/*
** Generate code that will assemble an index key and stores it in register
** regOut.  The key with be for index pIdx which is an index on pTab.
** iCur is the index of a cursor open on the pTab table and pointing to
** the entry that needs indexing.  If pTab is a WITHOUT ROWID table, then
** iCur must be the cursor of the PRIMARY KEY index.
**
** Return a register number which is the first in a block of
** registers that holds the elements of the index key.  The
** block of registers has already been deallocated by the time
** this routine returns.
**
** If *piPartIdxLabel is not NULL, fill it in with a label and jump
** to that label if pIdx is a partial index that should be skipped.
** The label should be resolved using sqlite3ResolvePartIdxLabel().
** A partial index should be skipped if its WHERE clause evaluates
** to false or null.  If pIdx is not a partial index, *piPartIdxLabel
** will be set to zero which is an empty label that is ignored by
** sqlite3ResolvePartIdxLabel().
**
** The pPrior and regPrior parameters are used to implement a cache to
** avoid unnecessary register loads.  If pPrior is not NULL, then it is
** a pointer to a different index for which an index key has just been
** computed into register regPrior.  If the current pIdx index is generating
** its key into the same sequence of registers and if pPrior and pIdx share
** a column in common, then the register corresponding to that column already
** holds the correct value and the loading of that register is skipped.
** This optimization is helpful when doing a DELETE or an INTEGRITY_CHECK 
** on a table with multiple indices, and especially with the ROWID or
** PRIMARY KEY columns of the index.
*/
SQLITE_PRIVATE int sqlite3GenerateIndexKey(
  Parse *pParse,       /* Parsing context */
  Index *pIdx,         /* The index for which to generate a key */
  int iDataCur,        /* Cursor number from which to take column data */
  int regOut,          /* Put the new key into this register if not 0 */
  int prefixOnly,      /* Compute only a unique prefix of the key */
  int *piPartIdxLabel, /* OUT: Jump to this label to skip partial index */
  Index *pPrior,       /* Previously generated index key */
  int regPrior         /* Register holding previous generated key */
){
  Vdbe *v = pParse->pVdbe;
  int j;
  int regBase;
  int nCol;

  if( piPartIdxLabel ){
    if( pIdx->pPartIdxWhere ){
      *piPartIdxLabel = sqlite3VdbeMakeLabel(v);
      pParse->iSelfTab = iDataCur;
      sqlite3ExprCachePush(pParse);
      sqlite3ExprIfFalseDup(pParse, pIdx->pPartIdxWhere, *piPartIdxLabel, 
                            SQLITE_JUMPIFNULL);
    }else{
      *piPartIdxLabel = 0;
    }
  }
  nCol = (prefixOnly && pIdx->uniqNotNull) ? pIdx->nKeyCol : pIdx->nColumn;
  regBase = sqlite3GetTempRange(pParse, nCol);
  if( pPrior && (regBase!=regPrior || pPrior->pPartIdxWhere) ) pPrior = 0;
  for(j=0; j<nCol; j++){
    if( pPrior
     && pPrior->aiColumn[j]==pIdx->aiColumn[j]
     && pPrior->aiColumn[j]!=XN_EXPR
    ){
      /* This column was already computed by the previous index */
      continue;
    }
    sqlite3ExprCodeLoadIndexColumn(pParse, pIdx, iDataCur, j, regBase+j);
    /* If the column affinity is REAL but the number is an integer, then it
    ** might be stored in the table as an integer (using a compact
    ** representation) then converted to REAL by an OP_RealAffinity opcode.
    ** But we are getting ready to store this value back into an index, where
    ** it should be converted by to INTEGER again.  So omit the OP_RealAffinity
    ** opcode if it is present */
    sqlite3VdbeDeletePriorOpcode(v, OP_RealAffinity);
  }
  if( regOut ){
    sqlite3VdbeAddOp3(v, OP_MakeRecord, regBase, nCol, regOut);
  }
  sqlite3ReleaseTempRange(pParse, regBase, nCol);
  return regBase;
}

/*
** If a prior call to sqlite3GenerateIndexKey() generated a jump-over label
** because it was a partial index, then this routine should be called to
** resolve that label.
*/
SQLITE_PRIVATE void sqlite3ResolvePartIdxLabel(Parse *pParse, int iLabel){
  if( iLabel ){
    sqlite3VdbeResolveLabel(pParse->pVdbe, iLabel);
    sqlite3ExprCachePop(pParse);
  }
}

/************** End of delete.c **********************************************/
/************** Begin file func.c ********************************************/
/*
** 2002 February 23
**
** The author disclaims copyright to this source code.  In place of
** a legal notice, here is a blessing:
**
**    May you do good and not evil.
**    May you find forgiveness for yourself and forgive others.
**    May you share freely, never taking more than you give.
**
*************************************************************************
** This file contains the C-language implementations for many of the SQL
** functions of SQLite.  (Some function, and in particular the date and
** time functions, are implemented separately.)
*/
/* #include "sqliteInt.h" */
/* #include <stdlib.h> */
/* #include <assert.h> */
/* #include "vdbeInt.h" */

/*
** Return the collating function associated with a function.
*/
static CollSeq *sqlite3GetFuncCollSeq(sqlite3_context *context){
  VdbeOp *pOp;
  assert( context->pVdbe!=0 );
  pOp = &context->pVdbe->aOp[context->iOp-1];
  assert( pOp->opcode==OP_CollSeq );
  assert( pOp->p4type==P4_COLLSEQ );
  return pOp->p4.pColl;
}

/*
** Indicate that the accumulator load should be skipped on this
** iteration of the aggregate loop.
*/
static void sqlite3SkipAccumulatorLoad(sqlite3_context *context){
  context->skipFlag = 1;
}

/*
** Implementation of the non-aggregate min() and max() functions
*/
static void minmaxFunc(
  sqlite3_context *context,
  int argc,
  sqlite3_value **argv
){
  int i;
  int mask;    /* 0 for min() or 0xffffffff for max() */
  int iBest;
  CollSeq *pColl;

  assert( argc>1 );
  mask = sqlite3_user_data(context)==0 ? 0 : -1;
  pColl = sqlite3GetFuncCollSeq(context);
  assert( pColl );
  assert( mask==-1 || mask==0 );
  iBest = 0;
  if( sqlite3_value_type(argv[0])==SQLITE_NULL ) return;
  for(i=1; i<argc; i++){
    if( sqlite3_value_type(argv[i])==SQLITE_NULL ) return;
    if( (sqlite3MemCompare(argv[iBest], argv[i], pColl)^mask)>=0 ){
      testcase( mask==0 );
      iBest = i;
    }
  }
  sqlite3_result_value(context, argv[iBest]);
}

/*
** Return the type of the argument.
*/
static void typeofFunc(
  sqlite3_context *context,
  int NotUsed,
  sqlite3_value **argv
){
  const char *z = 0;
  UNUSED_PARAMETER(NotUsed);
  switch( sqlite3_value_type(argv[0]) ){
    case SQLITE_INTEGER: z = "integer"; break;
    case SQLITE_TEXT:    z = "text";    break;
    case SQLITE_FLOAT:   z = "real";    break;
    case SQLITE_BLOB:    z = "blob";    break;
    default:             z = "null";    break;
  }
  sqlite3_result_text(context, z, -1, SQLITE_STATIC);
}


/*
** Implementation of the length() function
*/
static void lengthFunc(
  sqlite3_context *context,
  int argc,
  sqlite3_value **argv
){
  int len;

  assert( argc==1 );
  UNUSED_PARAMETER(argc);
  switch( sqlite3_value_type(argv[0]) ){
    case SQLITE_BLOB:
    case SQLITE_INTEGER:
    case SQLITE_FLOAT: {
      sqlite3_result_int(context, sqlite3_value_bytes(argv[0]));
      break;
    }
    case SQLITE_TEXT: {
      const unsigned char *z = sqlite3_value_text(argv[0]);
      if( z==0 ) return;
      len = 0;
      while( *z ){
        len++;
        SQLITE_SKIP_UTF8(z);
      }
      sqlite3_result_int(context, len);
      break;
    }
    default: {
      sqlite3_result_null(context);
      break;
    }
  }
}

/*
** Implementation of the abs() function.
**
** IMP: R-23979-26855 The abs(X) function returns the absolute value of
** the numeric argument X. 
*/
static void absFunc(sqlite3_context *context, int argc, sqlite3_value **argv){
  assert( argc==1 );
  UNUSED_PARAMETER(argc);
  switch( sqlite3_value_type(argv[0]) ){
    case SQLITE_INTEGER: {
      i64 iVal = sqlite3_value_int64(argv[0]);
      if( iVal<0 ){
        if( iVal==SMALLEST_INT64 ){
          /* IMP: R-31676-45509 If X is the integer -9223372036854775808
          ** then abs(X) throws an integer overflow error since there is no
          ** equivalent positive 64-bit two complement value. */
          sqlite3_result_error(context, "integer overflow", -1);
          return;
        }
        iVal = -iVal;
      } 
      sqlite3_result_int64(context, iVal);
      break;
    }
    case SQLITE_NULL: {
      /* IMP: R-37434-19929 Abs(X) returns NULL if X is NULL. */
      sqlite3_result_null(context);
      break;
    }
    default: {
      /* Because sqlite3_value_double() returns 0.0 if the argument is not
      ** something that can be converted into a number, we have:
      ** IMP: R-01992-00519 Abs(X) returns 0.0 if X is a string or blob
      ** that cannot be converted to a numeric value.
      */
      double rVal = sqlite3_value_double(argv[0]);
      if( rVal<0 ) rVal = -rVal;
      sqlite3_result_double(context, rVal);
      break;
    }
  }
}

/*
** Implementation of the instr() function.
**
** instr(haystack,needle) finds the first occurrence of needle
** in haystack and returns the number of previous characters plus 1,
** or 0 if needle does not occur within haystack.
**
** If both haystack and needle are BLOBs, then the result is one more than
** the number of bytes in haystack prior to the first occurrence of needle,
** or 0 if needle never occurs in haystack.
*/
static void instrFunc(
  sqlite3_context *context,
  int argc,
  sqlite3_value **argv
){
  const unsigned char *zHaystack;
  const unsigned char *zNeedle;
  int nHaystack;
  int nNeedle;
  int typeHaystack, typeNeedle;
  int N = 1;
  int isText;

  UNUSED_PARAMETER(argc);
  typeHaystack = sqlite3_value_type(argv[0]);
  typeNeedle = sqlite3_value_type(argv[1]);
  if( typeHaystack==SQLITE_NULL || typeNeedle==SQLITE_NULL ) return;
  nHaystack = sqlite3_value_bytes(argv[0]);
  nNeedle = sqlite3_value_bytes(argv[1]);
  if( typeHaystack==SQLITE_BLOB && typeNeedle==SQLITE_BLOB ){
    zHaystack = sqlite3_value_blob(argv[0]);
    zNeedle = sqlite3_value_blob(argv[1]);
    isText = 0;
  }else{
    zHaystack = sqlite3_value_text(argv[0]);
    zNeedle = sqlite3_value_text(argv[1]);
    isText = 1;
  }
  while( nNeedle<=nHaystack && memcmp(zHaystack, zNeedle, nNeedle)!=0 ){
    N++;
    do{
      nHaystack--;
      zHaystack++;
    }while( isText && (zHaystack[0]&0xc0)==0x80 );
  }
  if( nNeedle>nHaystack ) N = 0;
  sqlite3_result_int(context, N);
}

/*
** Implementation of the printf() function.
*/
static void printfFunc(
  sqlite3_context *context,
  int argc,
  sqlite3_value **argv
){
  PrintfArguments x;
  StrAccum str;
  const char *zFormat;
  int n;
  sqlite3 *db = sqlite3_context_db_handle(context);

  if( argc>=1 && (zFormat = (const char*)sqlite3_value_text(argv[0]))!=0 ){
    x.nArg = argc-1;
    x.nUsed = 0;
    x.apArg = argv+1;
    sqlite3StrAccumInit(&str, db, 0, 0, db->aLimit[SQLITE_LIMIT_LENGTH]);
    str.printfFlags = SQLITE_PRINTF_SQLFUNC;
    sqlite3XPrintf(&str, zFormat, &x);
    n = str.nChar;
    sqlite3_result_text(context, sqlite3StrAccumFinish(&str), n,
                        SQLITE_DYNAMIC);
  }
}

/*
** Implementation of the substr() function.
**
** substr(x,p1,p2)  returns p2 characters of x[] beginning with p1.
** p1 is 1-indexed.  So substr(x,1,1) returns the first character
** of x.  If x is text, then we actually count UTF-8 characters.
** If x is a blob, then we count bytes.
**
** If p1 is negative, then we begin abs(p1) from the end of x[].
**
** If p2 is negative, return the p2 characters preceding p1.
*/
static void substrFunc(
  sqlite3_context *context,
  int argc,
  sqlite3_value **argv
){
  const unsigned char *z;
  const unsigned char *z2;
  int len;
  int p0type;
  i64 p1, p2;
  int negP2 = 0;

  assert( argc==3 || argc==2 );
  if( sqlite3_value_type(argv[1])==SQLITE_NULL
   || (argc==3 && sqlite3_value_type(argv[2])==SQLITE_NULL)
  ){
    return;
  }
  p0type = sqlite3_value_type(argv[0]);
  p1 = sqlite3_value_int(argv[1]);
  if( p0type==SQLITE_BLOB ){
    len = sqlite3_value_bytes(argv[0]);
    z = sqlite3_value_blob(argv[0]);
    if( z==0 ) return;
    assert( len==sqlite3_value_bytes(argv[0]) );
  }else{
    z = sqlite3_value_text(argv[0]);
    if( z==0 ) return;
    len = 0;
    if( p1<0 ){
      for(z2=z; *z2; len++){
        SQLITE_SKIP_UTF8(z2);
      }
    }
  }
#ifdef SQLITE_SUBSTR_COMPATIBILITY
  /* If SUBSTR_COMPATIBILITY is defined then substr(X,0,N) work the same as
  ** as substr(X,1,N) - it returns the first N characters of X.  This
  ** is essentially a back-out of the bug-fix in check-in [5fc125d362df4b8]
  ** from 2009-02-02 for compatibility of applications that exploited the
  ** old buggy behavior. */
  if( p1==0 ) p1 = 1; /* <rdar://problem/6778339> */
#endif
  if( argc==3 ){
    p2 = sqlite3_value_int(argv[2]);
    if( p2<0 ){
      p2 = -p2;
      negP2 = 1;
    }
  }else{
    p2 = sqlite3_context_db_handle(context)->aLimit[SQLITE_LIMIT_LENGTH];
  }
  if( p1<0 ){
    p1 += len;
    if( p1<0 ){
      p2 += p1;
      if( p2<0 ) p2 = 0;
      p1 = 0;
    }
  }else if( p1>0 ){
    p1--;
  }else if( p2>0 ){
    p2--;
  }
  if( negP2 ){
    p1 -= p2;
    if( p1<0 ){
      p2 += p1;
      p1 = 0;
    }
  }
  assert( p1>=0 && p2>=0 );
  if( p0type!=SQLITE_BLOB ){
    while( *z && p1 ){
      SQLITE_SKIP_UTF8(z);
      p1--;
    }
    for(z2=z; *z2 && p2; p2--){
      SQLITE_SKIP_UTF8(z2);
    }
    sqlite3_result_text64(context, (char*)z, z2-z, SQLITE_TRANSIENT,
                          SQLITE_UTF8);
  }else{
    if( p1+p2>len ){
      p2 = len-p1;
      if( p2<0 ) p2 = 0;
    }
    sqlite3_result_blob64(context, (char*)&z[p1], (u64)p2, SQLITE_TRANSIENT);
  }
}

/*
** Implementation of the round() function
*/
#ifndef SQLITE_OMIT_FLOATING_POINT
static void roundFunc(sqlite3_context *context, int argc, sqlite3_value **argv){
  int n = 0;
  double r;
  char *zBuf;
  assert( argc==1 || argc==2 );
  if( argc==2 ){
    if( SQLITE_NULL==sqlite3_value_type(argv[1]) ) return;
    n = sqlite3_value_int(argv[1]);
    if( n>30 ) n = 30;
    if( n<0 ) n = 0;
  }
  if( sqlite3_value_type(argv[0])==SQLITE_NULL ) return;
  r = sqlite3_value_double(argv[0]);
  /* If Y==0 and X will fit in a 64-bit int,
  ** handle the rounding directly,
  ** otherwise use printf.
  */
  if( n==0 && r>=0 && r<LARGEST_INT64-1 ){
    r = (double)((sqlite_int64)(r+0.5));
  }else if( n==0 && r<0 && (-r)<LARGEST_INT64-1 ){
    r = -(double)((sqlite_int64)((-r)+0.5));
  }else{
    zBuf = sqlite3_mprintf("%.*f",n,r);
    if( zBuf==0 ){
      sqlite3_result_error_nomem(context);
      return;
    }
    sqlite3AtoF(zBuf, &r, sqlite3Strlen30(zBuf), SQLITE_UTF8);
    sqlite3_free(zBuf);
  }
  sqlite3_result_double(context, r);
}
#endif

/*
** Allocate nByte bytes of space using sqlite3Malloc(). If the
** allocation fails, call sqlite3_result_error_nomem() to notify
** the database handle that malloc() has failed and return NULL.
** If nByte is larger than the maximum string or blob length, then
** raise an SQLITE_TOOBIG exception and return NULL.
*/
static void *contextMalloc(sqlite3_context *context, i64 nByte){
  char *z;
  sqlite3 *db = sqlite3_context_db_handle(context);
  assert( nByte>0 );
  testcase( nByte==db->aLimit[SQLITE_LIMIT_LENGTH] );
  testcase( nByte==db->aLimit[SQLITE_LIMIT_LENGTH]+1 );
  if( nByte>db->aLimit[SQLITE_LIMIT_LENGTH] ){
    sqlite3_result_error_toobig(context);
    z = 0;
  }else{
    z = sqlite3Malloc(nByte);
    if( !z ){
      sqlite3_result_error_nomem(context);
    }
  }
  return z;
}

/*
** Implementation of the upper() and lower() SQL functions.
*/
static void upperFunc(sqlite3_context *context, int argc, sqlite3_value **argv){
  char *z1;
  const char *z2;
  int i, n;
  UNUSED_PARAMETER(argc);
  z2 = (char*)sqlite3_value_text(argv[0]);
  n = sqlite3_value_bytes(argv[0]);
  /* Verify that the call to _bytes() does not invalidate the _text() pointer */
  assert( z2==(char*)sqlite3_value_text(argv[0]) );
  if( z2 ){
    z1 = contextMalloc(context, ((i64)n)+1);
    if( z1 ){
      for(i=0; i<n; i++){
        z1[i] = (char)sqlite3Toupper(z2[i]);
      }
      sqlite3_result_text(context, z1, n, sqlite3_free);
    }
  }
}
static void lowerFunc(sqlite3_context *context, int argc, sqlite3_value **argv){
  char *z1;
  const char *z2;
  int i, n;
  UNUSED_PARAMETER(argc);
  z2 = (char*)sqlite3_value_text(argv[0]);
  n = sqlite3_value_bytes(argv[0]);
  /* Verify that the call to _bytes() does not invalidate the _text() pointer */
  assert( z2==(char*)sqlite3_value_text(argv[0]) );
  if( z2 ){
    z1 = contextMalloc(context, ((i64)n)+1);
    if( z1 ){
      for(i=0; i<n; i++){
        z1[i] = sqlite3Tolower(z2[i]);
      }
      sqlite3_result_text(context, z1, n, sqlite3_free);
    }
  }
}

/*
** Some functions like COALESCE() and IFNULL() and UNLIKELY() are implemented
** as VDBE code so that unused argument values do not have to be computed.
** However, we still need some kind of function implementation for this
** routines in the function table.  The noopFunc macro provides this.
** noopFunc will never be called so it doesn't matter what the implementation
** is.  We might as well use the "version()" function as a substitute.
*/
#define noopFunc versionFunc   /* Substitute function - never called */

/*
** Implementation of random().  Return a random integer.  
*/
static void randomFunc(
  sqlite3_context *context,
  int NotUsed,
  sqlite3_value **NotUsed2
){
  sqlite_int64 r;
  UNUSED_PARAMETER2(NotUsed, NotUsed2);
  sqlite3_randomness(sizeof(r), &r);
  if( r<0 ){
    /* We need to prevent a random number of 0x8000000000000000 
    ** (or -9223372036854775808) since when you do abs() of that
    ** number of you get the same value back again.  To do this
    ** in a way that is testable, mask the sign bit off of negative
    ** values, resulting in a positive value.  Then take the 
    ** 2s complement of that positive value.  The end result can
    ** therefore be no less than -9223372036854775807.
    */
    r = -(r & LARGEST_INT64);
  }
  sqlite3_result_int64(context, r);
}

/*
** Implementation of randomblob(N).  Return a random blob
** that is N bytes long.
*/
static void randomBlob(
  sqlite3_context *context,
  int argc,
  sqlite3_value **argv
){
  int n;
  unsigned char *p;
  assert( argc==1 );
  UNUSED_PARAMETER(argc);
  n = sqlite3_value_int(argv[0]);
  if( n<1 ){
    n = 1;
  }
  p = contextMalloc(context, n);
  if( p ){
    sqlite3_randomness(n, p);
    sqlite3_result_blob(context, (char*)p, n, sqlite3_free);
  }
}

/*
** Implementation of the last_insert_rowid() SQL function.  The return
** value is the same as the sqlite3_last_insert_rowid() API function.
*/
static void last_insert_rowid(
  sqlite3_context *context, 
  int NotUsed, 
  sqlite3_value **NotUsed2
){
  sqlite3 *db = sqlite3_context_db_handle(context);
  UNUSED_PARAMETER2(NotUsed, NotUsed2);
  /* IMP: R-51513-12026 The last_insert_rowid() SQL function is a
  ** wrapper around the sqlite3_last_insert_rowid() C/C++ interface
  ** function. */
  sqlite3_result_int64(context, sqlite3_last_insert_rowid(db));
}

/*
** Implementation of the changes() SQL function.
**
** IMP: R-62073-11209 The changes() SQL function is a wrapper
** around the sqlite3_changes() C/C++ function and hence follows the same
** rules for counting changes.
*/
static void changes(
  sqlite3_context *context,
  int NotUsed,
  sqlite3_value **NotUsed2
){
  sqlite3 *db = sqlite3_context_db_handle(context);
  UNUSED_PARAMETER2(NotUsed, NotUsed2);
  sqlite3_result_int(context, sqlite3_changes(db));
}

/*
** Implementation of the total_changes() SQL function.  The return value is
** the same as the sqlite3_total_changes() API function.
*/
static void total_changes(
  sqlite3_context *context,
  int NotUsed,
  sqlite3_value **NotUsed2
){
  sqlite3 *db = sqlite3_context_db_handle(context);
  UNUSED_PARAMETER2(NotUsed, NotUsed2);
  /* IMP: R-52756-41993 This function is a wrapper around the
  ** sqlite3_total_changes() C/C++ interface. */
  sqlite3_result_int(context, sqlite3_total_changes(db));
}

/*
** A structure defining how to do GLOB-style comparisons.
*/
struct compareInfo {
  u8 matchAll;          /* "*" or "%" */
  u8 matchOne;          /* "?" or "_" */
  u8 matchSet;          /* "[" or 0 */
  u8 noCase;            /* true to ignore case differences */
};

/*
** For LIKE and GLOB matching on EBCDIC machines, assume that every
** character is exactly one byte in size.  Also, provde the Utf8Read()
** macro for fast reading of the next character in the common case where
** the next character is ASCII.
*/
#if defined(SQLITE_EBCDIC)
# define sqlite3Utf8Read(A)        (*((*A)++))
# define Utf8Read(A)               (*(A++))
#else
# define Utf8Read(A)               (A[0]<0x80?*(A++):sqlite3Utf8Read(&A))
#endif

static const struct compareInfo globInfo = { '*', '?', '[', 0 };
/* The correct SQL-92 behavior is for the LIKE operator to ignore
** case.  Thus  'a' LIKE 'A' would be true. */
static const struct compareInfo likeInfoNorm = { '%', '_',   0, 1 };
/* If SQLITE_CASE_SENSITIVE_LIKE is defined, then the LIKE operator
** is case sensitive causing 'a' LIKE 'A' to be false */
static const struct compareInfo likeInfoAlt = { '%', '_',   0, 0 };

/*
** Compare two UTF-8 strings for equality where the first string can
** potentially be a "glob" or "like" expression.  Return true (1) if they
** are the same and false (0) if they are different.
**
** Globbing rules:
**
**      '*'       Matches any sequence of zero or more characters.
**
**      '?'       Matches exactly one character.
**
**     [...]      Matches one character from the enclosed list of
**                characters.
**
**     [^...]     Matches one character not in the enclosed list.
**
** With the [...] and [^...] matching, a ']' character can be included
** in the list by making it the first character after '[' or '^'.  A
** range of characters can be specified using '-'.  Example:
** "[a-z]" matches any single lower-case letter.  To match a '-', make
** it the last character in the list.
**
** Like matching rules:
** 
**      '%'       Matches any sequence of zero or more characters
**
***     '_'       Matches any one character
**
**      Ec        Where E is the "esc" character and c is any other
**                character, including '%', '_', and esc, match exactly c.
**
** The comments within this routine usually assume glob matching.
**
** This routine is usually quick, but can be N**2 in the worst case.
*/
static int patternCompare(
  const u8 *zPattern,              /* The glob pattern */
  const u8 *zString,               /* The string to compare against the glob */
  const struct compareInfo *pInfo, /* Information about how to do the compare */
  u32 matchOther                   /* The escape char (LIKE) or '[' (GLOB) */
){
  u32 c, c2;                       /* Next pattern and input string chars */
  u32 matchOne = pInfo->matchOne;  /* "?" or "_" */
  u32 matchAll = pInfo->matchAll;  /* "*" or "%" */
  u8 noCase = pInfo->noCase;       /* True if uppercase==lowercase */
  const u8 *zEscaped = 0;          /* One past the last escaped input char */
  
  while( (c = Utf8Read(zPattern))!=0 ){
    if( c==matchAll ){  /* Match "*" */
      /* Skip over multiple "*" characters in the pattern.  If there
      ** are also "?" characters, skip those as well, but consume a
      ** single character of the input string for each "?" skipped */
      while( (c=Utf8Read(zPattern)) == matchAll || c == matchOne ){
        if( c==matchOne && sqlite3Utf8Read(&zString)==0 ){
          return 0;
        }
      }
      if( c==0 ){
        return 1;   /* "*" at the end of the pattern matches */
      }else if( c==matchOther ){
        if( pInfo->matchSet==0 ){
          c = sqlite3Utf8Read(&zPattern);
          if( c==0 ) return 0;
        }else{
          /* "[...]" immediately follows the "*".  We have to do a slow
          ** recursive search in this case, but it is an unusual case. */
          assert( matchOther<0x80 );  /* '[' is a single-byte character */
          while( *zString
                 && patternCompare(&zPattern[-1],zString,pInfo,matchOther)==0 ){
            SQLITE_SKIP_UTF8(zString);
          }
          return *zString!=0;
        }
      }

      /* At this point variable c contains the first character of the
      ** pattern string past the "*".  Search in the input string for the
      ** first matching character and recursively contine the match from
      ** that point.
      **
      ** For a case-insensitive search, set variable cx to be the same as
      ** c but in the other case and search the input string for either
      ** c or cx.
      */
      if( c<=0x80 ){
        u32 cx;
        if( noCase ){
          cx = sqlite3Toupper(c);
          c = sqlite3Tolower(c);
        }else{
          cx = c;
        }
        while( (c2 = *(zString++))!=0 ){
          if( c2!=c && c2!=cx ) continue;
          if( patternCompare(zPattern,zString,pInfo,matchOther) ) return 1;
        }
      }else{
        while( (c2 = Utf8Read(zString))!=0 ){
          if( c2!=c ) continue;
          if( patternCompare(zPattern,zString,pInfo,matchOther) ) return 1;
        }
      }
      return 0;
    }
    if( c==matchOther ){
      if( pInfo->matchSet==0 ){
        c = sqlite3Utf8Read(&zPattern);
        if( c==0 ) return 0;
        zEscaped = zPattern;
      }else{
        u32 prior_c = 0;
        int seen = 0;
        int invert = 0;
        c = sqlite3Utf8Read(&zString);
        if( c==0 ) return 0;
        c2 = sqlite3Utf8Read(&zPattern);
        if( c2=='^' ){
          invert = 1;
          c2 = sqlite3Utf8Read(&zPattern);
        }
        if( c2==']' ){
          if( c==']' ) seen = 1;
          c2 = sqlite3Utf8Read(&zPattern);
        }
        while( c2 && c2!=']' ){
          if( c2=='-' && zPattern[0]!=']' && zPattern[0]!=0 && prior_c>0 ){
            c2 = sqlite3Utf8Read(&zPattern);
            if( c>=prior_c && c<=c2 ) seen = 1;
            prior_c = 0;
          }else{
            if( c==c2 ){
              seen = 1;
            }
            prior_c = c2;
          }
          c2 = sqlite3Utf8Read(&zPattern);
        }
        if( c2==0 || (seen ^ invert)==0 ){
          return 0;
        }
        continue;
      }
    }
    c2 = Utf8Read(zString);
    if( c==c2 ) continue;
    if( noCase && c<0x80 && c2<0x80 && sqlite3Tolower(c)==sqlite3Tolower(c2) ){
      continue;
    }
    if( c==matchOne && zPattern!=zEscaped && c2!=0 ) continue;
    return 0;
  }
  return *zString==0;
}

/*
** The sqlite3_strglob() interface.
*/
SQLITE_API int SQLITE_STDCALL sqlite3_strglob(const char *zGlobPattern, const char *zString){
  return patternCompare((u8*)zGlobPattern, (u8*)zString, &globInfo, '[')==0;
}

/*
** The sqlite3_strlike() interface.
*/
SQLITE_API int SQLITE_STDCALL sqlite3_strlike(const char *zPattern, const char *zStr, unsigned int esc){
  return patternCompare((u8*)zPattern, (u8*)zStr, &likeInfoNorm, esc)==0;
}

/*
** Count the number of times that the LIKE operator (or GLOB which is
** just a variation of LIKE) gets called.  This is used for testing
** only.
*/
#ifdef SQLITE_TEST
SQLITE_API int sqlite3_like_count = 0;
#endif


/*
** Implementation of the like() SQL function.  This function implements
** the build-in LIKE operator.  The first argument to the function is the
** pattern and the second argument is the string.  So, the SQL statements:
**
**       A LIKE B
**
** is implemented as like(B,A).
**
** This same function (with a different compareInfo structure) computes
** the GLOB operator.
*/
static void likeFunc(
  sqlite3_context *context, 
  int argc, 
  sqlite3_value **argv
){
  const unsigned char *zA, *zB;
  u32 escape;
  int nPat;
  sqlite3 *db = sqlite3_context_db_handle(context);
  struct compareInfo *pInfo = sqlite3_user_data(context);

#ifdef SQLITE_LIKE_DOESNT_MATCH_BLOBS
  if( sqlite3_value_type(argv[0])==SQLITE_BLOB
   || sqlite3_value_type(argv[1])==SQLITE_BLOB
  ){
#ifdef SQLITE_TEST
    sqlite3_like_count++;
#endif
    sqlite3_result_int(context, 0);
    return;
  }
#endif
  zB = sqlite3_value_text(argv[0]);
  zA = sqlite3_value_text(argv[1]);

  /* Limit the length of the LIKE or GLOB pattern to avoid problems
  ** of deep recursion and N*N behavior in patternCompare().
  */
  nPat = sqlite3_value_bytes(argv[0]);
  testcase( nPat==db->aLimit[SQLITE_LIMIT_LIKE_PATTERN_LENGTH] );
  testcase( nPat==db->aLimit[SQLITE_LIMIT_LIKE_PATTERN_LENGTH]+1 );
  if( nPat > db->aLimit[SQLITE_LIMIT_LIKE_PATTERN_LENGTH] ){
    sqlite3_result_error(context, "LIKE or GLOB pattern too complex", -1);
    return;
  }
  assert( zB==sqlite3_value_text(argv[0]) );  /* Encoding did not change */

  if( argc==3 ){
    /* The escape character string must consist of a single UTF-8 character.
    ** Otherwise, return an error.
    */
    const unsigned char *zEsc = sqlite3_value_text(argv[2]);
    if( zEsc==0 ) return;
    if( sqlite3Utf8CharLen((char*)zEsc, -1)!=1 ){
      sqlite3_result_error(context, 
          "ESCAPE expression must be a single character", -1);
      return;
    }
    escape = sqlite3Utf8Read(&zEsc);
  }else{
    escape = pInfo->matchSet;
  }
  if( zA && zB ){
#ifdef SQLITE_TEST
    sqlite3_like_count++;
#endif
    sqlite3_result_int(context, patternCompare(zB, zA, pInfo, escape));
  }
}

/*
** Implementation of the NULLIF(x,y) function.  The result is the first
** argument if the arguments are different.  The result is NULL if the
** arguments are equal to each other.
*/
static void nullifFunc(
  sqlite3_context *context,
  int NotUsed,
  sqlite3_value **argv
){
  CollSeq *pColl = sqlite3GetFuncCollSeq(context);
  UNUSED_PARAMETER(NotUsed);
  if( sqlite3MemCompare(argv[0], argv[1], pColl)!=0 ){
    sqlite3_result_value(context, argv[0]);
  }
}

/*
** Implementation of the sqlite_version() function.  The result is the version
** of the SQLite library that is running.
*/
static void versionFunc(
  sqlite3_context *context,
  int NotUsed,
  sqlite3_value **NotUsed2
){
  UNUSED_PARAMETER2(NotUsed, NotUsed2);
  /* IMP: R-48699-48617 This function is an SQL wrapper around the
  ** sqlite3_libversion() C-interface. */
  sqlite3_result_text(context, sqlite3_libversion(), -1, SQLITE_STATIC);
}

/*
** Implementation of the sqlite_source_id() function. The result is a string
** that identifies the particular version of the source code used to build
** SQLite.
*/
static void sourceidFunc(
  sqlite3_context *context,
  int NotUsed,
  sqlite3_value **NotUsed2
){
  UNUSED_PARAMETER2(NotUsed, NotUsed2);
  /* IMP: R-24470-31136 This function is an SQL wrapper around the
  ** sqlite3_sourceid() C interface. */
  sqlite3_result_text(context, sqlite3_sourceid(), -1, SQLITE_STATIC);
}

/*
** Implementation of the sqlite_log() function.  This is a wrapper around
** sqlite3_log().  The return value is NULL.  The function exists purely for
** its side-effects.
*/
static void errlogFunc(
  sqlite3_context *context,
  int argc,
  sqlite3_value **argv
){
  UNUSED_PARAMETER(argc);
  UNUSED_PARAMETER(context);
  sqlite3_log(sqlite3_value_int(argv[0]), "%s", sqlite3_value_text(argv[1]));
}

/*
** Implementation of the sqlite_compileoption_used() function.
** The result is an integer that identifies if the compiler option
** was used to build SQLite.
*/
#ifndef SQLITE_OMIT_COMPILEOPTION_DIAGS
static void compileoptionusedFunc(
  sqlite3_context *context,
  int argc,
  sqlite3_value **argv
){
  const char *zOptName;
  assert( argc==1 );
  UNUSED_PARAMETER(argc);
  /* IMP: R-39564-36305 The sqlite_compileoption_used() SQL
  ** function is a wrapper around the sqlite3_compileoption_used() C/C++
  ** function.
  */
  if( (zOptName = (const char*)sqlite3_value_text(argv[0]))!=0 ){
    sqlite3_result_int(context, sqlite3_compileoption_used(zOptName));
  }
}
#endif /* SQLITE_OMIT_COMPILEOPTION_DIAGS */

/*
** Implementation of the sqlite_compileoption_get() function. 
** The result is a string that identifies the compiler options 
** used to build SQLite.
*/
#ifndef SQLITE_OMIT_COMPILEOPTION_DIAGS
static void compileoptiongetFunc(
  sqlite3_context *context,
  int argc,
  sqlite3_value **argv
){
  int n;
  assert( argc==1 );
  UNUSED_PARAMETER(argc);
  /* IMP: R-04922-24076 The sqlite_compileoption_get() SQL function
  ** is a wrapper around the sqlite3_compileoption_get() C/C++ function.
  */
  n = sqlite3_value_int(argv[0]);
  sqlite3_result_text(context, sqlite3_compileoption_get(n), -1, SQLITE_STATIC);
}
#endif /* SQLITE_OMIT_COMPILEOPTION_DIAGS */

/* Array for converting from half-bytes (nybbles) into ASCII hex
** digits. */
static const char hexdigits[] = {
  '0', '1', '2', '3', '4', '5', '6', '7',
  '8', '9', 'A', 'B', 'C', 'D', 'E', 'F' 
};

/*
** Implementation of the QUOTE() function.  This function takes a single
** argument.  If the argument is numeric, the return value is the same as
** the argument.  If the argument is NULL, the return value is the string
** "NULL".  Otherwise, the argument is enclosed in single quotes with
** single-quote escapes.
*/
static void quoteFunc(sqlite3_context *context, int argc, sqlite3_value **argv){
  assert( argc==1 );
  UNUSED_PARAMETER(argc);
  switch( sqlite3_value_type(argv[0]) ){
    case SQLITE_FLOAT: {
      double r1, r2;
      char zBuf[50];
      r1 = sqlite3_value_double(argv[0]);
      sqlite3_snprintf(sizeof(zBuf), zBuf, "%!.15g", r1);
      sqlite3AtoF(zBuf, &r2, 20, SQLITE_UTF8);
      if( r1!=r2 ){
        sqlite3_snprintf(sizeof(zBuf), zBuf, "%!.20e", r1);
      }
      sqlite3_result_text(context, zBuf, -1, SQLITE_TRANSIENT);
      break;
    }
    case SQLITE_INTEGER: {
      sqlite3_result_value(context, argv[0]);
      break;
    }
    case SQLITE_BLOB: {
      char *zText = 0;
      char const *zBlob = sqlite3_value_blob(argv[0]);
      int nBlob = sqlite3_value_bytes(argv[0]);
      assert( zBlob==sqlite3_value_blob(argv[0]) ); /* No encoding change */
      zText = (char *)contextMalloc(context, (2*(i64)nBlob)+4); 
      if( zText ){
        int i;
        for(i=0; i<nBlob; i++){
          zText[(i*2)+2] = hexdigits[(zBlob[i]>>4)&0x0F];
          zText[(i*2)+3] = hexdigits[(zBlob[i])&0x0F];
        }
        zText[(nBlob*2)+2] = '\'';
        zText[(nBlob*2)+3] = '\0';
        zText[0] = 'X';
        zText[1] = '\'';
        sqlite3_result_text(context, zText, -1, SQLITE_TRANSIENT);
        sqlite3_free(zText);
      }
      break;
    }
    case SQLITE_TEXT: {
      int i,j;
      u64 n;
      const unsigned char *zArg = sqlite3_value_text(argv[0]);
      char *z;

      if( zArg==0 ) return;
      for(i=0, n=0; zArg[i]; i++){ if( zArg[i]=='\'' ) n++; }
      z = contextMalloc(context, ((i64)i)+((i64)n)+3);
      if( z ){
        z[0] = '\'';
        for(i=0, j=1; zArg[i]; i++){
          z[j++] = zArg[i];
          if( zArg[i]=='\'' ){
            z[j++] = '\'';
          }
        }
        z[j++] = '\'';
        z[j] = 0;
        sqlite3_result_text(context, z, j, sqlite3_free);
      }
      break;
    }
    default: {
      assert( sqlite3_value_type(argv[0])==SQLITE_NULL );
      sqlite3_result_text(context, "NULL", 4, SQLITE_STATIC);
      break;
    }
  }
}

/*
** The unicode() function.  Return the integer unicode code-point value
** for the first character of the input string. 
*/
static void unicodeFunc(
  sqlite3_context *context,
  int argc,
  sqlite3_value **argv
){
  const unsigned char *z = sqlite3_value_text(argv[0]);
  (void)argc;
  if( z && z[0] ) sqlite3_result_int(context, sqlite3Utf8Read(&z));
}

/*
** The char() function takes zero or more arguments, each of which is
** an integer.  It constructs a string where each character of the string
** is the unicode character for the corresponding integer argument.
*/
static void charFunc(
  sqlite3_context *context,
  int argc,
  sqlite3_value **argv
){
  unsigned char *z, *zOut;
  int i;
  zOut = z = sqlite3_malloc64( argc*4+1 );
  if( z==0 ){
    sqlite3_result_error_nomem(context);
    return;
  }
  for(i=0; i<argc; i++){
    sqlite3_int64 x;
    unsigned c;
    x = sqlite3_value_int64(argv[i]);
    if( x<0 || x>0x10ffff ) x = 0xfffd;
    c = (unsigned)(x & 0x1fffff);
    if( c<0x00080 ){
      *zOut++ = (u8)(c&0xFF);
    }else if( c<0x00800 ){
      *zOut++ = 0xC0 + (u8)((c>>6)&0x1F);
      *zOut++ = 0x80 + (u8)(c & 0x3F);
    }else if( c<0x10000 ){
      *zOut++ = 0xE0 + (u8)((c>>12)&0x0F);
      *zOut++ = 0x80 + (u8)((c>>6) & 0x3F);
      *zOut++ = 0x80 + (u8)(c & 0x3F);
    }else{
      *zOut++ = 0xF0 + (u8)((c>>18) & 0x07);
      *zOut++ = 0x80 + (u8)((c>>12) & 0x3F);
      *zOut++ = 0x80 + (u8)((c>>6) & 0x3F);
      *zOut++ = 0x80 + (u8)(c & 0x3F);
    }                                                    \
  }
  sqlite3_result_text64(context, (char*)z, zOut-z, sqlite3_free, SQLITE_UTF8);
}

/*
** The hex() function.  Interpret the argument as a blob.  Return
** a hexadecimal rendering as text.
*/
static void hexFunc(
  sqlite3_context *context,
  int argc,
  sqlite3_value **argv
){
  int i, n;
  const unsigned char *pBlob;
  char *zHex, *z;
  assert( argc==1 );
  UNUSED_PARAMETER(argc);
  pBlob = sqlite3_value_blob(argv[0]);
  n = sqlite3_value_bytes(argv[0]);
  assert( pBlob==sqlite3_value_blob(argv[0]) );  /* No encoding change */
  z = zHex = contextMalloc(context, ((i64)n)*2 + 1);
  if( zHex ){
    for(i=0; i<n; i++, pBlob++){
      unsigned char c = *pBlob;
      *(z++) = hexdigits[(c>>4)&0xf];
      *(z++) = hexdigits[c&0xf];
    }
    *z = 0;
    sqlite3_result_text(context, zHex, n*2, sqlite3_free);
  }
}

/*
** The zeroblob(N) function returns a zero-filled blob of size N bytes.
*/
static void zeroblobFunc(
  sqlite3_context *context,
  int argc,
  sqlite3_value **argv
){
  i64 n;
  int rc;
  assert( argc==1 );
  UNUSED_PARAMETER(argc);
  n = sqlite3_value_int64(argv[0]);
  if( n<0 ) n = 0;
  rc = sqlite3_result_zeroblob64(context, n); /* IMP: R-00293-64994 */
  if( rc ){
    sqlite3_result_error_code(context, rc);
  }
}

/*
** The replace() function.  Three arguments are all strings: call
** them A, B, and C. The result is also a string which is derived
** from A by replacing every occurrence of B with C.  The match
** must be exact.  Collating sequences are not used.
*/
static void replaceFunc(
  sqlite3_context *context,
  int argc,
  sqlite3_value **argv
){
  const unsigned char *zStr;        /* The input string A */
  const unsigned char *zPattern;    /* The pattern string B */
  const unsigned char *zRep;        /* The replacement string C */
  unsigned char *zOut;              /* The output */
  int nStr;                /* Size of zStr */
  int nPattern;            /* Size of zPattern */
  int nRep;                /* Size of zRep */
  i64 nOut;                /* Maximum size of zOut */
  int loopLimit;           /* Last zStr[] that might match zPattern[] */
  int i, j;                /* Loop counters */

  assert( argc==3 );
  UNUSED_PARAMETER(argc);
  zStr = sqlite3_value_text(argv[0]);
  if( zStr==0 ) return;
  nStr = sqlite3_value_bytes(argv[0]);
  assert( zStr==sqlite3_value_text(argv[0]) );  /* No encoding change */
  zPattern = sqlite3_value_text(argv[1]);
  if( zPattern==0 ){
    assert( sqlite3_value_type(argv[1])==SQLITE_NULL
            || sqlite3_context_db_handle(context)->mallocFailed );
    return;
  }
  if( zPattern[0]==0 ){
    assert( sqlite3_value_type(argv[1])!=SQLITE_NULL );
    sqlite3_result_value(context, argv[0]);
    return;
  }
  nPattern = sqlite3_value_bytes(argv[1]);
  assert( zPattern==sqlite3_value_text(argv[1]) );  /* No encoding change */
  zRep = sqlite3_value_text(argv[2]);
  if( zRep==0 ) return;
  nRep = sqlite3_value_bytes(argv[2]);
  assert( zRep==sqlite3_value_text(argv[2]) );
  nOut = nStr + 1;
  assert( nOut<SQLITE_MAX_LENGTH );
  zOut = contextMalloc(context, (i64)nOut);
  if( zOut==0 ){
    return;
  }
  loopLimit = nStr - nPattern;  
  for(i=j=0; i<=loopLimit; i++){
    if( zStr[i]!=zPattern[0] || memcmp(&zStr[i], zPattern, nPattern) ){
      zOut[j++] = zStr[i];
    }else{
      u8 *zOld;
      sqlite3 *db = sqlite3_context_db_handle(context);
      nOut += nRep - nPattern;
      testcase( nOut-1==db->aLimit[SQLITE_LIMIT_LENGTH] );
      testcase( nOut-2==db->aLimit[SQLITE_LIMIT_LENGTH] );
      if( nOut-1>db->aLimit[SQLITE_LIMIT_LENGTH] ){
        sqlite3_result_error_toobig(context);
        sqlite3_free(zOut);
        return;
      }
      zOld = zOut;
      zOut = sqlite3_realloc64(zOut, (int)nOut);
      if( zOut==0 ){
        sqlite3_result_error_nomem(context);
        sqlite3_free(zOld);
        return;
      }
      memcpy(&zOut[j], zRep, nRep);
      j += nRep;
      i += nPattern-1;
    }
  }
  assert( j+nStr-i+1==nOut );
  memcpy(&zOut[j], &zStr[i], nStr-i);
  j += nStr - i;
  assert( j<=nOut );
  zOut[j] = 0;
  sqlite3_result_text(context, (char*)zOut, j, sqlite3_free);
}

/*
** Implementation of the TRIM(), LTRIM(), and RTRIM() functions.
** The userdata is 0x1 for left trim, 0x2 for right trim, 0x3 for both.
*/
static void trimFunc(
  sqlite3_context *context,
  int argc,
  sqlite3_value **argv
){
  const unsigned char *zIn;         /* Input string */
  const unsigned char *zCharSet;    /* Set of characters to trim */
  int nIn;                          /* Number of bytes in input */
  int flags;                        /* 1: trimleft  2: trimright  3: trim */
  int i;                            /* Loop counter */
  unsigned char *aLen = 0;          /* Length of each character in zCharSet */
  unsigned char **azChar = 0;       /* Individual characters in zCharSet */
  int nChar;                        /* Number of characters in zCharSet */

  if( sqlite3_value_type(argv[0])==SQLITE_NULL ){
    return;
  }
  zIn = sqlite3_value_text(argv[0]);
  if( zIn==0 ) return;
  nIn = sqlite3_value_bytes(argv[0]);
  assert( zIn==sqlite3_value_text(argv[0]) );
  if( argc==1 ){
    static const unsigned char lenOne[] = { 1 };
    static unsigned char * const azOne[] = { (u8*)" " };
    nChar = 1;
    aLen = (u8*)lenOne;
    azChar = (unsigned char **)azOne;
    zCharSet = 0;
  }else if( (zCharSet = sqlite3_value_text(argv[1]))==0 ){
    return;
  }else{
    const unsigned char *z;
    for(z=zCharSet, nChar=0; *z; nChar++){
      SQLITE_SKIP_UTF8(z);
    }
    if( nChar>0 ){
      azChar = contextMalloc(context, ((i64)nChar)*(sizeof(char*)+1));
      if( azChar==0 ){
        return;
      }
      aLen = (unsigned char*)&azChar[nChar];
      for(z=zCharSet, nChar=0; *z; nChar++){
        azChar[nChar] = (unsigned char *)z;
        SQLITE_SKIP_UTF8(z);
        aLen[nChar] = (u8)(z - azChar[nChar]);
      }
    }
  }
  if( nChar>0 ){
    flags = SQLITE_PTR_TO_INT(sqlite3_user_data(context));
    if( flags & 1 ){
      while( nIn>0 ){
        int len = 0;
        for(i=0; i<nChar; i++){
          len = aLen[i];
          if( len<=nIn && memcmp(zIn, azChar[i], len)==0 ) break;
        }
        if( i>=nChar ) break;
        zIn += len;
        nIn -= len;
      }
    }
    if( flags & 2 ){
      while( nIn>0 ){
        int len = 0;
        for(i=0; i<nChar; i++){
          len = aLen[i];
          if( len<=nIn && memcmp(&zIn[nIn-len],azChar[i],len)==0 ) break;
        }
        if( i>=nChar ) break;
        nIn -= len;
      }
    }
    if( zCharSet ){
      sqlite3_free(azChar);
    }
  }
  sqlite3_result_text(context, (char*)zIn, nIn, SQLITE_TRANSIENT);
}


/* IMP: R-25361-16150 This function is omitted from SQLite by default. It
** is only available if the SQLITE_SOUNDEX compile-time option is used
** when SQLite is built.
*/
#ifdef SQLITE_SOUNDEX
/*
** Compute the soundex encoding of a word.
**
** IMP: R-59782-00072 The soundex(X) function returns a string that is the
** soundex encoding of the string X. 
*/
static void soundexFunc(
  sqlite3_context *context,
  int argc,
  sqlite3_value **argv
){
  char zResult[8];
  const u8 *zIn;
  int i, j;
  static const unsigned char iCode[] = {
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 1, 2, 3, 0, 1, 2, 0, 0, 2, 2, 4, 5, 5, 0,
    1, 2, 6, 2, 3, 0, 1, 0, 2, 0, 2, 0, 0, 0, 0, 0,
    0, 0, 1, 2, 3, 0, 1, 2, 0, 0, 2, 2, 4, 5, 5, 0,
    1, 2, 6, 2, 3, 0, 1, 0, 2, 0, 2, 0, 0, 0, 0, 0,
  };
  assert( argc==1 );
  zIn = (u8*)sqlite3_value_text(argv[0]);
  if( zIn==0 ) zIn = (u8*)"";
  for(i=0; zIn[i] && !sqlite3Isalpha(zIn[i]); i++){}
  if( zIn[i] ){
    u8 prevcode = iCode[zIn[i]&0x7f];
    zResult[0] = sqlite3Toupper(zIn[i]);
    for(j=1; j<4 && zIn[i]; i++){
      int code = iCode[zIn[i]&0x7f];
      if( code>0 ){
        if( code!=prevcode ){
          prevcode = code;
          zResult[j++] = code + '0';
        }
      }else{
        prevcode = 0;
      }
    }
    while( j<4 ){
      zResult[j++] = '0';
    }
    zResult[j] = 0;
    sqlite3_result_text(context, zResult, 4, SQLITE_TRANSIENT);
  }else{
    /* IMP: R-64894-50321 The string "?000" is returned if the argument
    ** is NULL or contains no ASCII alphabetic characters. */
    sqlite3_result_text(context, "?000", 4, SQLITE_STATIC);
  }
}
#endif /* SQLITE_SOUNDEX */

#ifndef SQLITE_OMIT_LOAD_EXTENSION
/*
** A function that loads a shared-library extension then returns NULL.
*/
static void loadExt(sqlite3_context *context, int argc, sqlite3_value **argv){
  const char *zFile = (const char *)sqlite3_value_text(argv[0]);
  const char *zProc;
  sqlite3 *db = sqlite3_context_db_handle(context);
  char *zErrMsg = 0;

  /* Disallow the load_extension() SQL function unless the SQLITE_LoadExtFunc
  ** flag is set.  See the sqlite3_enable_load_extension() API.
  */
  if( (db->flags & SQLITE_LoadExtFunc)==0 ){
    sqlite3_result_error(context, "not authorized", -1);
    return;
  }

  if( argc==2 ){
    zProc = (const char *)sqlite3_value_text(argv[1]);
  }else{
    zProc = 0;
  }
  if( zFile && sqlite3_load_extension(db, zFile, zProc, &zErrMsg) ){
    sqlite3_result_error(context, zErrMsg, -1);
    sqlite3_free(zErrMsg);
  }
}
#endif


/*
** An instance of the following structure holds the context of a
** sum() or avg() aggregate computation.
*/
typedef struct SumCtx SumCtx;
struct SumCtx {
  double rSum;      /* Floating point sum */
  i64 iSum;         /* Integer sum */   
  i64 cnt;          /* Number of elements summed */
  u8 overflow;      /* True if integer overflow seen */
  u8 approx;        /* True if non-integer value was input to the sum */
};

/*
** Routines used to compute the sum, average, and total.
**
** The SUM() function follows the (broken) SQL standard which means
** that it returns NULL if it sums over no inputs.  TOTAL returns
** 0.0 in that case.  In addition, TOTAL always returns a float where
** SUM might return an integer if it never encounters a floating point
** value.  TOTAL never fails, but SUM might through an exception if
** it overflows an integer.
*/
static void sumStep(sqlite3_context *context, int argc, sqlite3_value **argv){
  SumCtx *p;
  int type;
  assert( argc==1 );
  UNUSED_PARAMETER(argc);
  p = sqlite3_aggregate_context(context, sizeof(*p));
  type = sqlite3_value_numeric_type(argv[0]);
  if( p && type!=SQLITE_NULL ){
    p->cnt++;
    if( type==SQLITE_INTEGER ){
      i64 v = sqlite3_value_int64(argv[0]);
      p->rSum += v;
      if( (p->approx|p->overflow)==0 && sqlite3AddInt64(&p->iSum, v) ){
        p->overflow = 1;
      }
    }else{
      p->rSum += sqlite3_value_double(argv[0]);
      p->approx = 1;
    }
  }
}
static void sumFinalize(sqlite3_context *context){
  SumCtx *p;
  p = sqlite3_aggregate_context(context, 0);
  if( p && p->cnt>0 ){
    if( p->overflow ){
      sqlite3_result_error(context,"integer overflow",-1);
    }else if( p->approx ){
      sqlite3_result_double(context, p->rSum);
    }else{
      sqlite3_result_int64(context, p->iSum);
    }
  }
}
static void avgFinalize(sqlite3_context *context){
  SumCtx *p;
  p = sqlite3_aggregate_context(context, 0);
  if( p && p->cnt>0 ){
    sqlite3_result_double(context, p->rSum/(double)p->cnt);
  }
}
static void totalFinalize(sqlite3_context *context){
  SumCtx *p;
  p = sqlite3_aggregate_context(context, 0);
  /* (double)0 In case of SQLITE_OMIT_FLOATING_POINT... */
  sqlite3_result_double(context, p ? p->rSum : (double)0);
}

/*
** The following structure keeps track of state information for the
** count() aggregate function.
*/
typedef struct CountCtx CountCtx;
struct CountCtx {
  i64 n;
};

/*
** Routines to implement the count() aggregate function.
*/
static void countStep(sqlite3_context *context, int argc, sqlite3_value **argv){
  CountCtx *p;
  p = sqlite3_aggregate_context(context, sizeof(*p));
  if( (argc==0 || SQLITE_NULL!=sqlite3_value_type(argv[0])) && p ){
    p->n++;
  }

#ifndef SQLITE_OMIT_DEPRECATED
  /* The sqlite3_aggregate_count() function is deprecated.  But just to make
  ** sure it still operates correctly, verify that its count agrees with our 
  ** internal count when using count(*) and when the total count can be
  ** expressed as a 32-bit integer. */
  assert( argc==1 || p==0 || p->n>0x7fffffff
          || p->n==sqlite3_aggregate_count(context) );
#endif
}   
static void countFinalize(sqlite3_context *context){
  CountCtx *p;
  p = sqlite3_aggregate_context(context, 0);
  sqlite3_result_int64(context, p ? p->n : 0);
}

/*
** Routines to implement min() and max() aggregate functions.
*/
static void minmaxStep(
  sqlite3_context *context, 
  int NotUsed, 
  sqlite3_value **argv
){
  Mem *pArg  = (Mem *)argv[0];
  Mem *pBest;
  UNUSED_PARAMETER(NotUsed);

  pBest = (Mem *)sqlite3_aggregate_context(context, sizeof(*pBest));
  if( !pBest ) return;

  if( sqlite3_value_type(argv[0])==SQLITE_NULL ){
    if( pBest->flags ) sqlite3SkipAccumulatorLoad(context);
  }else if( pBest->flags ){
    int max;
    int cmp;
    CollSeq *pColl = sqlite3GetFuncCollSeq(context);
    /* This step function is used for both the min() and max() aggregates,
    ** the only difference between the two being that the sense of the
    ** comparison is inverted. For the max() aggregate, the
    ** sqlite3_user_data() function returns (void *)-1. For min() it
    ** returns (void *)db, where db is the sqlite3* database pointer.
    ** Therefore the next statement sets variable 'max' to 1 for the max()
    ** aggregate, or 0 for min().
    */
    max = sqlite3_user_data(context)!=0;
    cmp = sqlite3MemCompare(pBest, pArg, pColl);
    if( (max && cmp<0) || (!max && cmp>0) ){
      sqlite3VdbeMemCopy(pBest, pArg);
    }else{
      sqlite3SkipAccumulatorLoad(context);
    }
  }else{
    pBest->db = sqlite3_context_db_handle(context);
    sqlite3VdbeMemCopy(pBest, pArg);
  }
}
static void minMaxFinalize(sqlite3_context *context){
  sqlite3_value *pRes;
  pRes = (sqlite3_value *)sqlite3_aggregate_context(context, 0);
  if( pRes ){
    if( pRes->flags ){
      sqlite3_result_value(context, pRes);
    }
    sqlite3VdbeMemRelease(pRes);
  }
}

/*
** group_concat(EXPR, ?SEPARATOR?)
*/
static void groupConcatStep(
  sqlite3_context *context,
  int argc,
  sqlite3_value **argv
){
  const char *zVal;
  StrAccum *pAccum;
  const char *zSep;
  int nVal, nSep;
  assert( argc==1 || argc==2 );
  if( sqlite3_value_type(argv[0])==SQLITE_NULL ) return;
  pAccum = (StrAccum*)sqlite3_aggregate_context(context, sizeof(*pAccum));

  if( pAccum ){
    sqlite3 *db = sqlite3_context_db_handle(context);
    int firstTerm = pAccum->mxAlloc==0;
    pAccum->mxAlloc = db->aLimit[SQLITE_LIMIT_LENGTH];
    if( !firstTerm ){
      if( argc==2 ){
        zSep = (char*)sqlite3_value_text(argv[1]);
        nSep = sqlite3_value_bytes(argv[1]);
      }else{
        zSep = ",";
        nSep = 1;
      }
      if( nSep ) sqlite3StrAccumAppend(pAccum, zSep, nSep);
    }
    zVal = (char*)sqlite3_value_text(argv[0]);
    nVal = sqlite3_value_bytes(argv[0]);
    if( zVal ) sqlite3StrAccumAppend(pAccum, zVal, nVal);
  }
}
static void groupConcatFinalize(sqlite3_context *context){
  StrAccum *pAccum;
  pAccum = sqlite3_aggregate_context(context, 0);
  if( pAccum ){
    if( pAccum->accError==STRACCUM_TOOBIG ){
      sqlite3_result_error_toobig(context);
    }else if( pAccum->accError==STRACCUM_NOMEM ){
      sqlite3_result_error_nomem(context);
    }else{    
      sqlite3_result_text(context, sqlite3StrAccumFinish(pAccum), -1, 
                          sqlite3_free);
    }
  }
}

/*
** This routine does per-connection function registration.  Most
** of the built-in functions above are part of the global function set.
** This routine only deals with those that are not global.
*/
SQLITE_PRIVATE void sqlite3RegisterPerConnectionBuiltinFunctions(sqlite3 *db){
  int rc = sqlite3_overload_function(db, "MATCH", 2);
  assert( rc==SQLITE_NOMEM || rc==SQLITE_OK );
  if( rc==SQLITE_NOMEM ){
    sqlite3OomFault(db);
  }
}

/*
** Set the LIKEOPT flag on the 2-argument function with the given name.
*/
static void setLikeOptFlag(sqlite3 *db, const char *zName, u8 flagVal){
  FuncDef *pDef;
  pDef = sqlite3FindFunction(db, zName, 2, SQLITE_UTF8, 0);
  if( ALWAYS(pDef) ){
    pDef->funcFlags |= flagVal;
  }
}

/*
** Register the built-in LIKE and GLOB functions.  The caseSensitive
** parameter determines whether or not the LIKE operator is case
** sensitive.  GLOB is always case sensitive.
*/
SQLITE_PRIVATE void sqlite3RegisterLikeFunctions(sqlite3 *db, int caseSensitive){
  struct compareInfo *pInfo;
  if( caseSensitive ){
    pInfo = (struct compareInfo*)&likeInfoAlt;
  }else{
    pInfo = (struct compareInfo*)&likeInfoNorm;
  }
  sqlite3CreateFunc(db, "like", 2, SQLITE_UTF8, pInfo, likeFunc, 0, 0, 0);
  sqlite3CreateFunc(db, "like", 3, SQLITE_UTF8, pInfo, likeFunc, 0, 0, 0);
  sqlite3CreateFunc(db, "glob", 2, SQLITE_UTF8, 
      (struct compareInfo*)&globInfo, likeFunc, 0, 0, 0);
  setLikeOptFlag(db, "glob", SQLITE_FUNC_LIKE | SQLITE_FUNC_CASE);
  setLikeOptFlag(db, "like", 
      caseSensitive ? (SQLITE_FUNC_LIKE | SQLITE_FUNC_CASE) : SQLITE_FUNC_LIKE);
}

/*
** pExpr points to an expression which implements a function.  If
** it is appropriate to apply the LIKE optimization to that function
** then set aWc[0] through aWc[2] to the wildcard characters and
** return TRUE.  If the function is not a LIKE-style function then
** return FALSE.
**
** *pIsNocase is set to true if uppercase and lowercase are equivalent for
** the function (default for LIKE).  If the function makes the distinction
** between uppercase and lowercase (as does GLOB) then *pIsNocase is set to
** false.
*/
SQLITE_PRIVATE int sqlite3IsLikeFunction(sqlite3 *db, Expr *pExpr, int *pIsNocase, char *aWc){
  FuncDef *pDef;
  if( pExpr->op!=TK_FUNCTION 
   || !pExpr->x.pList 
   || pExpr->x.pList->nExpr!=2
  ){
    return 0;
  }
  assert( !ExprHasProperty(pExpr, EP_xIsSelect) );
  pDef = sqlite3FindFunction(db, pExpr->u.zToken, 2, SQLITE_UTF8, 0);
  if( NEVER(pDef==0) || (pDef->funcFlags & SQLITE_FUNC_LIKE)==0 ){
    return 0;
  }

  /* The memcpy() statement assumes that the wildcard characters are
  ** the first three statements in the compareInfo structure.  The
  ** asserts() that follow verify that assumption
  */
  memcpy(aWc, pDef->pUserData, 3);
  assert( (char*)&likeInfoAlt == (char*)&likeInfoAlt.matchAll );
  assert( &((char*)&likeInfoAlt)[1] == (char*)&likeInfoAlt.matchOne );
  assert( &((char*)&likeInfoAlt)[2] == (char*)&likeInfoAlt.matchSet );
  *pIsNocase = (pDef->funcFlags & SQLITE_FUNC_CASE)==0;
  return 1;
}

/*
** All of the FuncDef structures in the aBuiltinFunc[] array above
** to the global function hash table.  This occurs at start-time (as
** a consequence of calling sqlite3_initialize()).
**
** After this routine runs
*/
SQLITE_PRIVATE void sqlite3RegisterBuiltinFunctions(void){
  /*
  ** The following array holds FuncDef structures for all of the functions
  ** defined in this file.
  **
  ** The array cannot be constant since changes are made to the
  ** FuncDef.pHash elements at start-time.  The elements of this array
  ** are read-only after initialization is complete.
  **
  ** For peak efficiency, put the most frequently used function last.
  */
  static FuncDef aBuiltinFunc[] = {
#ifdef SQLITE_SOUNDEX
    FUNCTION(soundex,            1, 0, 0, soundexFunc      ),
#endif
#ifndef SQLITE_OMIT_LOAD_EXTENSION
    VFUNCTION(load_extension,    1, 0, 0, loadExt          ),
    VFUNCTION(load_extension,    2, 0, 0, loadExt          ),
#endif
#if SQLITE_USER_AUTHENTICATION
    FUNCTION(sqlite_crypt,       2, 0, 0, sqlite3CryptFunc ),
#endif
#ifndef SQLITE_OMIT_COMPILEOPTION_DIAGS
    DFUNCTION(sqlite_compileoption_used,1, 0, 0, compileoptionusedFunc  ),
    DFUNCTION(sqlite_compileoption_get, 1, 0, 0, compileoptiongetFunc  ),
#endif /* SQLITE_OMIT_COMPILEOPTION_DIAGS */
    FUNCTION2(unlikely,          1, 0, 0, noopFunc,  SQLITE_FUNC_UNLIKELY),
    FUNCTION2(likelihood,        2, 0, 0, noopFunc,  SQLITE_FUNC_UNLIKELY),
    FUNCTION2(likely,            1, 0, 0, noopFunc,  SQLITE_FUNC_UNLIKELY),
    FUNCTION(ltrim,              1, 1, 0, trimFunc         ),
    FUNCTION(ltrim,              2, 1, 0, trimFunc         ),
    FUNCTION(rtrim,              1, 2, 0, trimFunc         ),
    FUNCTION(rtrim,              2, 2, 0, trimFunc         ),
    FUNCTION(trim,               1, 3, 0, trimFunc         ),
    FUNCTION(trim,               2, 3, 0, trimFunc         ),
    FUNCTION(min,               -1, 0, 1, minmaxFunc       ),
    FUNCTION(min,                0, 0, 1, 0                ),
    AGGREGATE2(min,              1, 0, 1, minmaxStep,      minMaxFinalize,
                                          SQLITE_FUNC_MINMAX ),
    FUNCTION(max,               -1, 1, 1, minmaxFunc       ),
    FUNCTION(max,                0, 1, 1, 0                ),
    AGGREGATE2(max,              1, 1, 1, minmaxStep,      minMaxFinalize,
                                          SQLITE_FUNC_MINMAX ),
    FUNCTION2(typeof,            1, 0, 0, typeofFunc,  SQLITE_FUNC_TYPEOF),
    FUNCTION2(length,            1, 0, 0, lengthFunc,  SQLITE_FUNC_LENGTH),
    FUNCTION(instr,              2, 0, 0, instrFunc        ),
    FUNCTION(printf,            -1, 0, 0, printfFunc       ),
    FUNCTION(unicode,            1, 0, 0, unicodeFunc      ),
    FUNCTION(char,              -1, 0, 0, charFunc         ),
    FUNCTION(abs,                1, 0, 0, absFunc          ),
#ifndef SQLITE_OMIT_FLOATING_POINT
    FUNCTION(round,              1, 0, 0, roundFunc        ),
    FUNCTION(round,              2, 0, 0, roundFunc        ),
#endif
    FUNCTION(upper,              1, 0, 0, upperFunc        ),
    FUNCTION(lower,              1, 0, 0, lowerFunc        ),
    FUNCTION(hex,                1, 0, 0, hexFunc          ),
    FUNCTION2(ifnull,            2, 0, 0, noopFunc,  SQLITE_FUNC_COALESCE),
    VFUNCTION(random,            0, 0, 0, randomFunc       ),
    VFUNCTION(randomblob,        1, 0, 0, randomBlob       ),
    FUNCTION(nullif,             2, 0, 1, nullifFunc       ),
    DFUNCTION(sqlite_version,    0, 0, 0, versionFunc      ),
    DFUNCTION(sqlite_source_id,  0, 0, 0, sourceidFunc     ),
    FUNCTION(sqlite_log,         2, 0, 0, errlogFunc       ),
    FUNCTION(quote,              1, 0, 0, quoteFunc        ),
    VFUNCTION(last_insert_rowid, 0, 0, 0, last_insert_rowid),
    VFUNCTION(changes,           0, 0, 0, changes          ),
    VFUNCTION(total_changes,     0, 0, 0, total_changes    ),
    FUNCTION(replace,            3, 0, 0, replaceFunc      ),
    FUNCTION(zeroblob,           1, 0, 0, zeroblobFunc     ),
    FUNCTION(substr,             2, 0, 0, substrFunc       ),
    FUNCTION(substr,             3, 0, 0, substrFunc       ),
    AGGREGATE(sum,               1, 0, 0, sumStep,         sumFinalize    ),
    AGGREGATE(total,             1, 0, 0, sumStep,         totalFinalize    ),
    AGGREGATE(avg,               1, 0, 0, sumStep,         avgFinalize    ),
    AGGREGATE2(count,            0, 0, 0, countStep,       countFinalize,
               SQLITE_FUNC_COUNT  ),
    AGGREGATE(count,             1, 0, 0, countStep,       countFinalize  ),
    AGGREGATE(group_concat,      1, 0, 0, groupConcatStep, groupConcatFinalize),
    AGGREGATE(group_concat,      2, 0, 0, groupConcatStep, groupConcatFinalize),
  
    LIKEFUNC(glob, 2, &globInfo, SQLITE_FUNC_LIKE|SQLITE_FUNC_CASE),
  #ifdef SQLITE_CASE_SENSITIVE_LIKE
    LIKEFUNC(like, 2, &likeInfoAlt, SQLITE_FUNC_LIKE|SQLITE_FUNC_CASE),
    LIKEFUNC(like, 3, &likeInfoAlt, SQLITE_FUNC_LIKE|SQLITE_FUNC_CASE),
  #else
    LIKEFUNC(like, 2, &likeInfoNorm, SQLITE_FUNC_LIKE),
    LIKEFUNC(like, 3, &likeInfoNorm, SQLITE_FUNC_LIKE),
  #endif
    FUNCTION(coalesce,           1, 0, 0, 0                ),
    FUNCTION(coalesce,           0, 0, 0, 0                ),
    FUNCTION2(coalesce,         -1, 0, 0, noopFunc,  SQLITE_FUNC_COALESCE),
  };
#ifndef SQLITE_OMIT_ALTERTABLE
  sqlite3AlterFunctions();
#endif
#if defined(SQLITE_ENABLE_STAT3) || defined(SQLITE_ENABLE_STAT4)
  sqlite3AnalyzeFunctions();
#endif
  sqlite3RegisterDateTimeFunctions();
  sqlite3InsertBuiltinFuncs(aBuiltinFunc, ArraySize(aBuiltinFunc));

#if 0  /* Enable to print out how the built-in functions are hashed */
  {
    int i;
    FuncDef *p;
    for(i=0; i<SQLITE_FUNC_HASH_SZ; i++){
      printf("FUNC-HASH %02d:", i);
      for(p=sqlite3BuiltinFunctions.a[i]; p; p=p->u.pHash){
        int n = sqlite3Strlen30(p->zName);
        int h = p->zName[0] + n;
        printf(" %s(%d)", p->zName, h);
      }
      printf("\n");
    }
  }
#endif
}

/************** End of func.c ************************************************/
/************** Begin file fkey.c ********************************************/
/*
**
** The author disclaims copyright to this source code.  In place of
** a legal notice, here is a blessing:
**
**    May you do good and not evil.
**    May you find forgiveness for yourself and forgive others.
**    May you share freely, never taking more than you give.
**
*************************************************************************
** This file contains code used by the compiler to add foreign key
** support to compiled SQL statements.
*/
/* #include "sqliteInt.h" */

#ifndef SQLITE_OMIT_FOREIGN_KEY
#ifndef SQLITE_OMIT_TRIGGER

/*
** Deferred and Immediate FKs
** --------------------------
**
** Foreign keys in SQLite come in two flavours: deferred and immediate.
** If an immediate foreign key constraint is violated,
** SQLITE_CONSTRAINT_FOREIGNKEY is returned and the current
** statement transaction rolled back. If a 
** deferred foreign key constraint is violated, no action is taken 
** immediately. However if the application attempts to commit the 
** transaction before fixing the constraint violation, the attempt fails.
**
** Deferred constraints are implemented using a simple counter associated
** with the database handle. The counter is set to zero each time a 
** database transaction is opened. Each time a statement is executed 
** that causes a foreign key violation, the counter is incremented. Each
** time a statement is executed that removes an existing violation from
** the database, the counter is decremented. When the transaction is
** committed, the commit fails if the current value of the counter is
** greater than zero. This scheme has two big drawbacks:
**
**   * When a commit fails due to a deferred foreign key constraint, 
**     there is no way to tell which foreign constraint is not satisfied,
**     or which row it is not satisfied for.
**
**   * If the database contains foreign key violations when the 
**     transaction is opened, this may cause the mechanism to malfunction.
**
** Despite these problems, this approach is adopted as it seems simpler
** than the alternatives.
**
** INSERT operations:
**
**   I.1) For each FK for which the table is the child table, search
**        the parent table for a match. If none is found increment the
**        constraint counter.
**
**   I.2) For each FK for which the table is the parent table, 
**        search the child table for rows that correspond to the new
**        row in the parent table. Decrement the counter for each row
**        found (as the constraint is now satisfied).
**
** DELETE operations:
**
**   D.1) For each FK for which the table is the child table, 
**        search the parent table for a row that corresponds to the 
**        deleted row in the child table. If such a row is not found, 
**        decrement the counter.
**
**   D.2) For each FK for which the table is the parent table, search 
**        the child table for rows that correspond to the deleted row 
**        in the parent table. For each found increment the counter.
**
** UPDATE operations:
**
**   An UPDATE command requires that all 4 steps above are taken, but only
**   for FK constraints for which the affected columns are actually 
**   modified (values must be compared at runtime).
**
** Note that I.1 and D.1 are very similar operations, as are I.2 and D.2.
** This simplifies the implementation a bit.
**
** For the purposes of immediate FK constraints, the OR REPLACE conflict
** resolution is considered to delete rows before the new row is inserted.
** If a delete caused by OR REPLACE violates an FK constraint, an exception
** is thrown, even if the FK constraint would be satisfied after the new 
** row is inserted.
**
** Immediate constraints are usually handled similarly. The only difference 
** is that the counter used is stored as part of each individual statement
** object (struct Vdbe). If, after the statement has run, its immediate
** constraint counter is greater than zero,
** it returns SQLITE_CONSTRAINT_FOREIGNKEY
** and the statement transaction is rolled back. An exception is an INSERT
** statement that inserts a single row only (no triggers). In this case,
** instead of using a counter, an exception is thrown immediately if the
** INSERT violates a foreign key constraint. This is necessary as such
** an INSERT does not open a statement transaction.
**
** TODO: How should dropping a table be handled? How should renaming a 
** table be handled?
**
**
** Query API Notes
** ---------------
**
** Before coding an UPDATE or DELETE row operation, the code-generator
** for those two operations needs to know whether or not the operation
** requires any FK processing and, if so, which columns of the original
** row are required by the FK processing VDBE code (i.e. if FKs were
** implemented using triggers, which of the old.* columns would be 
** accessed). No information is required by the code-generator before
** coding an INSERT operation. The functions used by the UPDATE/DELETE
** generation code to query for this information are:
**
**   sqlite3FkRequired() - Test to see if FK processing is required.
**   sqlite3FkOldmask()  - Query for the set of required old.* columns.
**
**
** Externally accessible module functions
** --------------------------------------
**
**   sqlite3FkCheck()    - Check for foreign key violations.
**   sqlite3FkActions()  - Code triggers for ON UPDATE/ON DELETE actions.
**   sqlite3FkDelete()   - Delete an FKey structure.
*/

/*
** VDBE Calling Convention
** -----------------------
**
** Example:
**
**   For the following INSERT statement:
**
**     CREATE TABLE t1(a, b INTEGER PRIMARY KEY, c);
**     INSERT INTO t1 VALUES(1, 2, 3.1);
**
**   Register (x):        2    (type integer)
**   Register (x+1):      1    (type integer)
**   Register (x+2):      NULL (type NULL)
**   Register (x+3):      3.1  (type real)
*/

/*
** A foreign key constraint requires that the key columns in the parent
** table are collectively subject to a UNIQUE or PRIMARY KEY constraint.
** Given that pParent is the parent table for foreign key constraint pFKey, 
** search the schema for a unique index on the parent key columns. 
**
** If successful, zero is returned. If the parent key is an INTEGER PRIMARY 
** KEY column, then output variable *ppIdx is set to NULL. Otherwise, *ppIdx 
** is set to point to the unique index. 
** 
** If the parent key consists of a single column (the foreign key constraint
** is not a composite foreign key), output variable *paiCol is set to NULL.
** Otherwise, it is set to point to an allocated array of size N, where
** N is the number of columns in the parent key. The first element of the
** array is the index of the child table column that is mapped by the FK
** constraint to the parent table column stored in the left-most column
** of index *ppIdx. The second element of the array is the index of the
** child table column that corresponds to the second left-most column of
** *ppIdx, and so on.
**
** If the required index cannot be found, either because:
**
**   1) The named parent key columns do not exist, or
**
**   2) The named parent key columns do exist, but are not subject to a
**      UNIQUE or PRIMARY KEY constraint, or
**
**   3) No parent key columns were provided explicitly as part of the
**      foreign key definition, and the parent table does not have a
**      PRIMARY KEY, or
**
**   4) No parent key columns were provided explicitly as part of the
**      foreign key definition, and the PRIMARY KEY of the parent table 
**      consists of a different number of columns to the child key in 
**      the child table.
**
** then non-zero is returned, and a "foreign key mismatch" error loaded
** into pParse. If an OOM error occurs, non-zero is returned and the
** pParse->db->mallocFailed flag is set.
*/
SQLITE_PRIVATE int sqlite3FkLocateIndex(
  Parse *pParse,                  /* Parse context to store any error in */
  Table *pParent,                 /* Parent table of FK constraint pFKey */
  FKey *pFKey,                    /* Foreign key to find index for */
  Index **ppIdx,                  /* OUT: Unique index on parent table */
  int **paiCol                    /* OUT: Map of index columns in pFKey */
){
  Index *pIdx = 0;                    /* Value to return via *ppIdx */
  int *aiCol = 0;                     /* Value to return via *paiCol */
  int nCol = pFKey->nCol;             /* Number of columns in parent key */
  char *zKey = pFKey->aCol[0].zCol;   /* Name of left-most parent key column */

  /* The caller is responsible for zeroing output parameters. */
  assert( ppIdx && *ppIdx==0 );
  assert( !paiCol || *paiCol==0 );
  assert( pParse );

  /* If this is a non-composite (single column) foreign key, check if it 
  ** maps to the INTEGER PRIMARY KEY of table pParent. If so, leave *ppIdx 
  ** and *paiCol set to zero and return early. 
  **
  ** Otherwise, for a composite foreign key (more than one column), allocate
  ** space for the aiCol array (returned via output parameter *paiCol).
  ** Non-composite foreign keys do not require the aiCol array.
  */
  if( nCol==1 ){
    /* The FK maps to the IPK if any of the following are true:
    **
    **   1) There is an INTEGER PRIMARY KEY column and the FK is implicitly 
    **      mapped to the primary key of table pParent, or
    **   2) The FK is explicitly mapped to a column declared as INTEGER
    **      PRIMARY KEY.
    */
    if( pParent->iPKey>=0 ){
      if( !zKey ) return 0;
      if( !sqlite3StrICmp(pParent->aCol[pParent->iPKey].zName, zKey) ) return 0;
    }
  }else if( paiCol ){
    assert( nCol>1 );
    aiCol = (int *)sqlite3DbMallocRawNN(pParse->db, nCol*sizeof(int));
    if( !aiCol ) return 1;
    *paiCol = aiCol;
  }

  for(pIdx=pParent->pIndex; pIdx; pIdx=pIdx->pNext){
    if( pIdx->nKeyCol==nCol && IsUniqueIndex(pIdx) ){ 
      /* pIdx is a UNIQUE index (or a PRIMARY KEY) and has the right number
      ** of columns. If each indexed column corresponds to a foreign key
      ** column of pFKey, then this index is a winner.  */

      if( zKey==0 ){
        /* If zKey is NULL, then this foreign key is implicitly mapped to 
        ** the PRIMARY KEY of table pParent. The PRIMARY KEY index may be 
        ** identified by the test.  */
        if( IsPrimaryKeyIndex(pIdx) ){
          if( aiCol ){
            int i;
            for(i=0; i<nCol; i++) aiCol[i] = pFKey->aCol[i].iFrom;
          }
          break;
        }
      }else{
        /* If zKey is non-NULL, then this foreign key was declared to
        ** map to an explicit list of columns in table pParent. Check if this
        ** index matches those columns. Also, check that the index uses
        ** the default collation sequences for each column. */
        int i, j;
        for(i=0; i<nCol; i++){
          i16 iCol = pIdx->aiColumn[i];     /* Index of column in parent tbl */
          const char *zDfltColl;            /* Def. collation for column */
          char *zIdxCol;                    /* Name of indexed column */

          if( iCol<0 ) break; /* No foreign keys against expression indexes */

          /* If the index uses a collation sequence that is different from
          ** the default collation sequence for the column, this index is
          ** unusable. Bail out early in this case.  */
          zDfltColl = pParent->aCol[iCol].zColl;
          if( !zDfltColl ) zDfltColl = sqlite3StrBINARY;
          if( sqlite3StrICmp(pIdx->azColl[i], zDfltColl) ) break;

          zIdxCol = pParent->aCol[iCol].zName;
          for(j=0; j<nCol; j++){
            if( sqlite3StrICmp(pFKey->aCol[j].zCol, zIdxCol)==0 ){
              if( aiCol ) aiCol[i] = pFKey->aCol[j].iFrom;
              break;
            }
          }
          if( j==nCol ) break;
        }
        if( i==nCol ) break;      /* pIdx is usable */
      }
    }
  }

  if( !pIdx ){
    if( !pParse->disableTriggers ){
      sqlite3ErrorMsg(pParse,
           "foreign key mismatch - \"%w\" referencing \"%w\"",
           pFKey->pFrom->zName, pFKey->zTo);
    }
    sqlite3DbFree(pParse->db, aiCol);
    return 1;
  }

  *ppIdx = pIdx;
  return 0;
}

/*
** This function is called when a row is inserted into or deleted from the 
** child table of foreign key constraint pFKey. If an SQL UPDATE is executed 
** on the child table of pFKey, this function is invoked twice for each row
** affected - once to "delete" the old row, and then again to "insert" the
** new row.
**
** Each time it is called, this function generates VDBE code to locate the
** row in the parent table that corresponds to the row being inserted into 
** or deleted from the child table. If the parent row can be found, no 
** special action is taken. Otherwise, if the parent row can *not* be
** found in the parent table:
**
**   Operation | FK type   | Action taken
**   --------------------------------------------------------------------------
**   INSERT      immediate   Increment the "immediate constraint counter".
**
**   DELETE      immediate   Decrement the "immediate constraint counter".
**
**   INSERT      deferred    Increment the "deferred constraint counter".
**
**   DELETE      deferred    Decrement the "deferred constraint counter".
**
** These operations are identified in the comment at the top of this file 
** (fkey.c) as "I.1" and "D.1".
*/
static void fkLookupParent(
  Parse *pParse,        /* Parse context */
  int iDb,              /* Index of database housing pTab */
  Table *pTab,          /* Parent table of FK pFKey */
  Index *pIdx,          /* Unique index on parent key columns in pTab */
  FKey *pFKey,          /* Foreign key constraint */
  int *aiCol,           /* Map from parent key columns to child table columns */
  int regData,          /* Address of array containing child table row */
  int nIncr,            /* Increment constraint counter by this */
  int isIgnore          /* If true, pretend pTab contains all NULL values */
){
  int i;                                    /* Iterator variable */
  Vdbe *v = sqlite3GetVdbe(pParse);         /* Vdbe to add code to */
  int iCur = pParse->nTab - 1;              /* Cursor number to use */
  int iOk = sqlite3VdbeMakeLabel(v);        /* jump here if parent key found */

  /* If nIncr is less than zero, then check at runtime if there are any
  ** outstanding constraints to resolve. If there are not, there is no need
  ** to check if deleting this row resolves any outstanding violations.
  **
  ** Check if any of the key columns in the child table row are NULL. If 
  ** any are, then the constraint is considered satisfied. No need to 
  ** search for a matching row in the parent table.  */
  if( nIncr<0 ){
    sqlite3VdbeAddOp2(v, OP_FkIfZero, pFKey->isDeferred, iOk);
    VdbeCoverage(v);
  }
  for(i=0; i<pFKey->nCol; i++){
    int iReg = aiCol[i] + regData + 1;
    sqlite3VdbeAddOp2(v, OP_IsNull, iReg, iOk); VdbeCoverage(v);
  }

  if( isIgnore==0 ){
    if( pIdx==0 ){
      /* If pIdx is NULL, then the parent key is the INTEGER PRIMARY KEY
      ** column of the parent table (table pTab).  */
      int iMustBeInt;               /* Address of MustBeInt instruction */
      int regTemp = sqlite3GetTempReg(pParse);
  
      /* Invoke MustBeInt to coerce the child key value to an integer (i.e. 
      ** apply the affinity of the parent key). If this fails, then there
      ** is no matching parent key. Before using MustBeInt, make a copy of
      ** the value. Otherwise, the value inserted into the child key column
      ** will have INTEGER affinity applied to it, which may not be correct.  */
      sqlite3VdbeAddOp2(v, OP_SCopy, aiCol[0]+1+regData, regTemp);
      iMustBeInt = sqlite3VdbeAddOp2(v, OP_MustBeInt, regTemp, 0);
      VdbeCoverage(v);
  
      /* If the parent table is the same as the child table, and we are about
      ** to increment the constraint-counter (i.e. this is an INSERT operation),
      ** then check if the row being inserted matches itself. If so, do not
      ** increment the constraint-counter.  */
      if( pTab==pFKey->pFrom && nIncr==1 ){
        sqlite3VdbeAddOp3(v, OP_Eq, regData, iOk, regTemp); VdbeCoverage(v);
        sqlite3VdbeChangeP5(v, SQLITE_NOTNULL);
      }
  
      sqlite3OpenTable(pParse, iCur, iDb, pTab, OP_OpenRead);
      sqlite3VdbeAddOp3(v, OP_NotExists, iCur, 0, regTemp); VdbeCoverage(v);
      sqlite3VdbeGoto(v, iOk);
      sqlite3VdbeJumpHere(v, sqlite3VdbeCurrentAddr(v)-2);
      sqlite3VdbeJumpHere(v, iMustBeInt);
      sqlite3ReleaseTempReg(pParse, regTemp);
    }else{
      int nCol = pFKey->nCol;
      int regTemp = sqlite3GetTempRange(pParse, nCol);
      int regRec = sqlite3GetTempReg(pParse);
  
      sqlite3VdbeAddOp3(v, OP_OpenRead, iCur, pIdx->tnum, iDb);
      sqlite3VdbeSetP4KeyInfo(pParse, pIdx);
      for(i=0; i<nCol; i++){
        sqlite3VdbeAddOp2(v, OP_Copy, aiCol[i]+1+regData, regTemp+i);
      }
  
      /* If the parent table is the same as the child table, and we are about
      ** to increment the constraint-counter (i.e. this is an INSERT operation),
      ** then check if the row being inserted matches itself. If so, do not
      ** increment the constraint-counter. 
      **
      ** If any of the parent-key values are NULL, then the row cannot match 
      ** itself. So set JUMPIFNULL to make sure we do the OP_Found if any
      ** of the parent-key values are NULL (at this point it is known that
      ** none of the child key values are).
      */
      if( pTab==pFKey->pFrom && nIncr==1 ){
        int iJump = sqlite3VdbeCurrentAddr(v) + nCol + 1;
        for(i=0; i<nCol; i++){
          int iChild = aiCol[i]+1+regData;
          int iParent = pIdx->aiColumn[i]+1+regData;
          assert( pIdx->aiColumn[i]>=0 );
          assert( aiCol[i]!=pTab->iPKey );
          if( pIdx->aiColumn[i]==pTab->iPKey ){
            /* The parent key is a composite key that includes the IPK column */
            iParent = regData;
          }
          sqlite3VdbeAddOp3(v, OP_Ne, iChild, iJump, iParent); VdbeCoverage(v);
          sqlite3VdbeChangeP5(v, SQLITE_JUMPIFNULL);
        }
        sqlite3VdbeGoto(v, iOk);
      }
  
      sqlite3VdbeAddOp4(v, OP_MakeRecord, regTemp, nCol, regRec,
                        sqlite3IndexAffinityStr(pParse->db,pIdx), nCol);
      sqlite3VdbeAddOp4Int(v, OP_Found, iCur, iOk, regRec, 0); VdbeCoverage(v);
  
      sqlite3ReleaseTempReg(pParse, regRec);
      sqlite3ReleaseTempRange(pParse, regTemp, nCol);
    }
  }

  if( !pFKey->isDeferred && !(pParse->db->flags & SQLITE_DeferFKs)
   && !pParse->pToplevel 
   && !pParse->isMultiWrite 
  ){
    /* Special case: If this is an INSERT statement that will insert exactly
    ** one row into the table, raise a constraint immediately instead of
    ** incrementing a counter. This is necessary as the VM code is being
    ** generated for will not open a statement transaction.  */
    assert( nIncr==1 );
    sqlite3HaltConstraint(pParse, SQLITE_CONSTRAINT_FOREIGNKEY,
        OE_Abort, 0, P4_STATIC, P5_ConstraintFK);
  }else{
    if( nIncr>0 && pFKey->isDeferred==0 ){
      sqlite3MayAbort(pParse);
    }
    sqlite3VdbeAddOp2(v, OP_FkCounter, pFKey->isDeferred, nIncr);
  }

  sqlite3VdbeResolveLabel(v, iOk);
  sqlite3VdbeAddOp1(v, OP_Close, iCur);
}


/*
** Return an Expr object that refers to a memory register corresponding
** to column iCol of table pTab.
**
** regBase is the first of an array of register that contains the data
** for pTab.  regBase itself holds the rowid.  regBase+1 holds the first
** column.  regBase+2 holds the second column, and so forth.
*/
static Expr *exprTableRegister(
  Parse *pParse,     /* Parsing and code generating context */
  Table *pTab,       /* The table whose content is at r[regBase]... */
  int regBase,       /* Contents of table pTab */
  i16 iCol           /* Which column of pTab is desired */
){
  Expr *pExpr;
  Column *pCol;
  const char *zColl;
  sqlite3 *db = pParse->db;

  pExpr = sqlite3Expr(db, TK_REGISTER, 0);
  if( pExpr ){
    if( iCol>=0 && iCol!=pTab->iPKey ){
      pCol = &pTab->aCol[iCol];
      pExpr->iTable = regBase + iCol + 1;
      pExpr->affinity = pCol->affinity;
      zColl = pCol->zColl;
      if( zColl==0 ) zColl = db->pDfltColl->zName;
      pExpr = sqlite3ExprAddCollateString(pParse, pExpr, zColl);
    }else{
      pExpr->iTable = regBase;
      pExpr->affinity = SQLITE_AFF_INTEGER;
    }
  }
  return pExpr;
}

/*
** Return an Expr object that refers to column iCol of table pTab which
** has cursor iCur.
*/
static Expr *exprTableColumn(
  sqlite3 *db,      /* The database connection */
  Table *pTab,      /* The table whose column is desired */
  int iCursor,      /* The open cursor on the table */
  i16 iCol          /* The column that is wanted */
){
  Expr *pExpr = sqlite3Expr(db, TK_COLUMN, 0);
  if( pExpr ){
    pExpr->pTab = pTab;
    pExpr->iTable = iCursor;
    pExpr->iColumn = iCol;
  }
  return pExpr;
}

/*
** This function is called to generate code executed when a row is deleted
** from the parent table of foreign key constraint pFKey and, if pFKey is 
** deferred, when a row is inserted into the same table. When generating
** code for an SQL UPDATE operation, this function may be called twice -
** once to "delete" the old row and once to "insert" the new row.
**
** Parameter nIncr is passed -1 when inserting a row (as this may decrease
** the number of FK violations in the db) or +1 when deleting one (as this
** may increase the number of FK constraint problems).
**
** The code generated by this function scans through the rows in the child
** table that correspond to the parent table row being deleted or inserted.
** For each child row found, one of the following actions is taken:
**
**   Operation | FK type   | Action taken
**   --------------------------------------------------------------------------
**   DELETE      immediate   Increment the "immediate constraint counter".
**                           Or, if the ON (UPDATE|DELETE) action is RESTRICT,
**                           throw a "FOREIGN KEY constraint failed" exception.
**
**   INSERT      immediate   Decrement the "immediate constraint counter".
**
**   DELETE      deferred    Increment the "deferred constraint counter".
**                           Or, if the ON (UPDATE|DELETE) action is RESTRICT,
**                           throw a "FOREIGN KEY constraint failed" exception.
**
**   INSERT      deferred    Decrement the "deferred constraint counter".
**
** These operations are identified in the comment at the top of this file 
** (fkey.c) as "I.2" and "D.2".
*/
static void fkScanChildren(
  Parse *pParse,                  /* Parse context */
  SrcList *pSrc,                  /* The child table to be scanned */
  Table *pTab,                    /* The parent table */
  Index *pIdx,                    /* Index on parent covering the foreign key */
  FKey *pFKey,                    /* The foreign key linking pSrc to pTab */
  int *aiCol,                     /* Map from pIdx cols to child table cols */
  int regData,                    /* Parent row data starts here */
  int nIncr                       /* Amount to increment deferred counter by */
){
  sqlite3 *db = pParse->db;       /* Database handle */
  int i;                          /* Iterator variable */
  Expr *pWhere = 0;               /* WHERE clause to scan with */
  NameContext sNameContext;       /* Context used to resolve WHERE clause */
  WhereInfo *pWInfo;              /* Context used by sqlite3WhereXXX() */
  int iFkIfZero = 0;              /* Address of OP_FkIfZero */
  Vdbe *v = sqlite3GetVdbe(pParse);

  assert( pIdx==0 || pIdx->pTable==pTab );
  assert( pIdx==0 || pIdx->nKeyCol==pFKey->nCol );
  assert( pIdx!=0 || pFKey->nCol==1 );
  assert( pIdx!=0 || HasRowid(pTab) );

  if( nIncr<0 ){
    iFkIfZero = sqlite3VdbeAddOp2(v, OP_FkIfZero, pFKey->isDeferred, 0);
    VdbeCoverage(v);
  }

  /* Create an Expr object representing an SQL expression like:
  **
  **   <parent-key1> = <child-key1> AND <parent-key2> = <child-key2> ...
  **
  ** The collation sequence used for the comparison should be that of
  ** the parent key columns. The affinity of the parent key column should
  ** be applied to each child key value before the comparison takes place.
  */
  for(i=0; i<pFKey->nCol; i++){
    Expr *pLeft;                  /* Value from parent table row */
    Expr *pRight;                 /* Column ref to child table */
    Expr *pEq;                    /* Expression (pLeft = pRight) */
    i16 iCol;                     /* Index of column in child table */ 
    const char *zCol;             /* Name of column in child table */

    iCol = pIdx ? pIdx->aiColumn[i] : -1;
    pLeft = exprTableRegister(pParse, pTab, regData, iCol);
    iCol = aiCol ? aiCol[i] : pFKey->aCol[0].iFrom;
    assert( iCol>=0 );
    zCol = pFKey->pFrom->aCol[iCol].zName;
    pRight = sqlite3Expr(db, TK_ID, zCol);
    pEq = sqlite3PExpr(pParse, TK_EQ, pLeft, pRight, 0);
    pWhere = sqlite3ExprAnd(db, pWhere, pEq);
  }

  /* If the child table is the same as the parent table, then add terms
  ** to the WHERE clause that prevent this entry from being scanned.
  ** The added WHERE clause terms are like this:
  **
  **     $current_rowid!=rowid
  **     NOT( $current_a==a AND $current_b==b AND ... )
  **
  ** The first form is used for rowid tables.  The second form is used
  ** for WITHOUT ROWID tables.  In the second form, the primary key is
  ** (a,b,...)
  */
  if( pTab==pFKey->pFrom && nIncr>0 ){
    Expr *pNe;                    /* Expression (pLeft != pRight) */
    Expr *pLeft;                  /* Value from parent table row */
    Expr *pRight;                 /* Column ref to child table */
    if( HasRowid(pTab) ){
      pLeft = exprTableRegister(pParse, pTab, regData, -1);
      pRight = exprTableColumn(db, pTab, pSrc->a[0].iCursor, -1);
      pNe = sqlite3PExpr(pParse, TK_NE, pLeft, pRight, 0);
    }else{
      Expr *pEq, *pAll = 0;
      Index *pPk = sqlite3PrimaryKeyIndex(pTab);
      assert( pIdx!=0 );
      for(i=0; i<pPk->nKeyCol; i++){
        i16 iCol = pIdx->aiColumn[i];
        assert( iCol>=0 );
        pLeft = exprTableRegister(pParse, pTab, regData, iCol);
        pRight = exprTableColumn(db, pTab, pSrc->a[0].iCursor, iCol);
        pEq = sqlite3PExpr(pParse, TK_EQ, pLeft, pRight, 0);
        pAll = sqlite3ExprAnd(db, pAll, pEq);
      }
      pNe = sqlite3PExpr(pParse, TK_NOT, pAll, 0, 0);
    }
    pWhere = sqlite3ExprAnd(db, pWhere, pNe);
  }

  /* Resolve the references in the WHERE clause. */
  memset(&sNameContext, 0, sizeof(NameContext));
  sNameContext.pSrcList = pSrc;
  sNameContext.pParse = pParse;
  sqlite3ResolveExprNames(&sNameContext, pWhere);

  /* Create VDBE to loop through the entries in pSrc that match the WHERE
  ** clause. For each row found, increment either the deferred or immediate
  ** foreign key constraint counter. */
  pWInfo = sqlite3WhereBegin(pParse, pSrc, pWhere, 0, 0, 0, 0);
  sqlite3VdbeAddOp2(v, OP_FkCounter, pFKey->isDeferred, nIncr);
  if( pWInfo ){
    sqlite3WhereEnd(pWInfo);
  }

  /* Clean up the WHERE clause constructed above. */
  sqlite3ExprDelete(db, pWhere);
  if( iFkIfZero ){
    sqlite3VdbeJumpHere(v, iFkIfZero);
  }
}

/*
** This function returns a linked list of FKey objects (connected by
** FKey.pNextTo) holding all children of table pTab.  For example,
** given the following schema:
**
**   CREATE TABLE t1(a PRIMARY KEY);
**   CREATE TABLE t2(b REFERENCES t1(a);
**
** Calling this function with table "t1" as an argument returns a pointer
** to the FKey structure representing the foreign key constraint on table
** "t2". Calling this function with "t2" as the argument would return a
** NULL pointer (as there are no FK constraints for which t2 is the parent
** table).
*/
SQLITE_PRIVATE FKey *sqlite3FkReferences(Table *pTab){
  return (FKey *)sqlite3HashFind(&pTab->pSchema->fkeyHash, pTab->zName);
}

/*
** The second argument is a Trigger structure allocated by the 
** fkActionTrigger() routine. This function deletes the Trigger structure
** and all of its sub-components.
**
** The Trigger structure or any of its sub-components may be allocated from
** the lookaside buffer belonging to database handle dbMem.
*/
static void fkTriggerDelete(sqlite3 *dbMem, Trigger *p){
  if( p ){
    TriggerStep *pStep = p->step_list;
    sqlite3ExprDelete(dbMem, pStep->pWhere);
    sqlite3ExprListDelete(dbMem, pStep->pExprList);
    sqlite3SelectDelete(dbMem, pStep->pSelect);
    sqlite3ExprDelete(dbMem, p->pWhen);
    sqlite3DbFree(dbMem, p);
  }
}

/*
** This function is called to generate code that runs when table pTab is
** being dropped from the database. The SrcList passed as the second argument
** to this function contains a single entry guaranteed to resolve to
** table pTab.
**
** Normally, no code is required. However, if either
**
**   (a) The table is the parent table of a FK constraint, or
**   (b) The table is the child table of a deferred FK constraint and it is
**       determined at runtime that there are outstanding deferred FK 
**       constraint violations in the database,
**
** then the equivalent of "DELETE FROM <tbl>" is executed before dropping
** the table from the database. Triggers are disabled while running this
** DELETE, but foreign key actions are not.
*/
SQLITE_PRIVATE void sqlite3FkDropTable(Parse *pParse, SrcList *pName, Table *pTab){
  sqlite3 *db = pParse->db;
  if( (db->flags&SQLITE_ForeignKeys) && !IsVirtual(pTab) && !pTab->pSelect ){
    int iSkip = 0;
    Vdbe *v = sqlite3GetVdbe(pParse);

    assert( v );                  /* VDBE has already been allocated */
    if( sqlite3FkReferences(pTab)==0 ){
      /* Search for a deferred foreign key constraint for which this table
      ** is the child table. If one cannot be found, return without 
      ** generating any VDBE code. If one can be found, then jump over
      ** the entire DELETE if there are no outstanding deferred constraints
      ** when this statement is run.  */
      FKey *p;
      for(p=pTab->pFKey; p; p=p->pNextFrom){
        if( p->isDeferred || (db->flags & SQLITE_DeferFKs) ) break;
      }
      if( !p ) return;
      iSkip = sqlite3VdbeMakeLabel(v);
      sqlite3VdbeAddOp2(v, OP_FkIfZero, 1, iSkip); VdbeCoverage(v);
    }

    pParse->disableTriggers = 1;
    sqlite3DeleteFrom(pParse, sqlite3SrcListDup(db, pName, 0), 0);
    pParse->disableTriggers = 0;

    /* If the DELETE has generated immediate foreign key constraint 
    ** violations, halt the VDBE and return an error at this point, before
    ** any modifications to the schema are made. This is because statement
    ** transactions are not able to rollback schema changes.  
    **
    ** If the SQLITE_DeferFKs flag is set, then this is not required, as
    ** the statement transaction will not be rolled back even if FK
    ** constraints are violated.
    */
    if( (db->flags & SQLITE_DeferFKs)==0 ){
      sqlite3VdbeAddOp2(v, OP_FkIfZero, 0, sqlite3VdbeCurrentAddr(v)+2);
      VdbeCoverage(v);
      sqlite3HaltConstraint(pParse, SQLITE_CONSTRAINT_FOREIGNKEY,
          OE_Abort, 0, P4_STATIC, P5_ConstraintFK);
    }

    if( iSkip ){
      sqlite3VdbeResolveLabel(v, iSkip);
    }
  }
}


/*
** The second argument points to an FKey object representing a foreign key
** for which pTab is the child table. An UPDATE statement against pTab
** is currently being processed. For each column of the table that is 
** actually updated, the corresponding element in the aChange[] array
** is zero or greater (if a column is unmodified the corresponding element
** is set to -1). If the rowid column is modified by the UPDATE statement
** the bChngRowid argument is non-zero.
**
** This function returns true if any of the columns that are part of the
** child key for FK constraint *p are modified.
*/
static int fkChildIsModified(
  Table *pTab,                    /* Table being updated */
  FKey *p,                        /* Foreign key for which pTab is the child */
  int *aChange,                   /* Array indicating modified columns */
  int bChngRowid                  /* True if rowid is modified by this update */
){
  int i;
  for(i=0; i<p->nCol; i++){
    int iChildKey = p->aCol[i].iFrom;
    if( aChange[iChildKey]>=0 ) return 1;
    if( iChildKey==pTab->iPKey && bChngRowid ) return 1;
  }
  return 0;
}

/*
** The second argument points to an FKey object representing a foreign key
** for which pTab is the parent table. An UPDATE statement against pTab
** is currently being processed. For each column of the table that is 
** actually updated, the corresponding element in the aChange[] array
** is zero or greater (if a column is unmodified the corresponding element
** is set to -1). If the rowid column is modified by the UPDATE statement
** the bChngRowid argument is non-zero.
**
** This function returns true if any of the columns that are part of the
** parent key for FK constraint *p are modified.
*/
static int fkParentIsModified(
  Table *pTab, 
  FKey *p, 
  int *aChange, 
  int bChngRowid
){
  int i;
  for(i=0; i<p->nCol; i++){
    char *zKey = p->aCol[i].zCol;
    int iKey;
    for(iKey=0; iKey<pTab->nCol; iKey++){
      if( aChange[iKey]>=0 || (iKey==pTab->iPKey && bChngRowid) ){
        Column *pCol = &pTab->aCol[iKey];
        if( zKey ){
          if( 0==sqlite3StrICmp(pCol->zName, zKey) ) return 1;
        }else if( pCol->colFlags & COLFLAG_PRIMKEY ){
          return 1;
        }
      }
    }
  }
  return 0;
}

/*
** Return true if the parser passed as the first argument is being
** used to code a trigger that is really a "SET NULL" action belonging
** to trigger pFKey.
*/
static int isSetNullAction(Parse *pParse, FKey *pFKey){
  Parse *pTop = sqlite3ParseToplevel(pParse);
  if( pTop->pTriggerPrg ){
    Trigger *p = pTop->pTriggerPrg->pTrigger;
    if( (p==pFKey->apTrigger[0] && pFKey->aAction[0]==OE_SetNull)
     || (p==pFKey->apTrigger[1] && pFKey->aAction[1]==OE_SetNull)
    ){
      return 1;
    }
  }
  return 0;
}

/*
** This function is called when inserting, deleting or updating a row of
** table pTab to generate VDBE code to perform foreign key constraint 
** processing for the operation.
**
** For a DELETE operation, parameter regOld is passed the index of the
** first register in an array of (pTab->nCol+1) registers containing the
** rowid of the row being deleted, followed by each of the column values
** of the row being deleted, from left to right. Parameter regNew is passed
** zero in this case.
**
** For an INSERT operation, regOld is passed zero and regNew is passed the
** first register of an array of (pTab->nCol+1) registers containing the new
** row data.
**
** For an UPDATE operation, this function is called twice. Once before
** the original record is deleted from the table using the calling convention
** described for DELETE. Then again after the original record is deleted
** but before the new record is inserted using the INSERT convention. 
*/
SQLITE_PRIVATE void sqlite3FkCheck(
  Parse *pParse,                  /* Parse context */
  Table *pTab,                    /* Row is being deleted from this table */ 
  int regOld,                     /* Previous row data is stored here */
  int regNew,                     /* New row data is stored here */
  int *aChange,                   /* Array indicating UPDATEd columns (or 0) */
  int bChngRowid                  /* True if rowid is UPDATEd */
){
  sqlite3 *db = pParse->db;       /* Database handle */
  FKey *pFKey;                    /* Used to iterate through FKs */
  int iDb;                        /* Index of database containing pTab */
  const char *zDb;                /* Name of database containing pTab */
  int isIgnoreErrors = pParse->disableTriggers;

  /* Exactly one of regOld and regNew should be non-zero. */
  assert( (regOld==0)!=(regNew==0) );

  /* If foreign-keys are disabled, this function is a no-op. */
  if( (db->flags&SQLITE_ForeignKeys)==0 ) return;

  iDb = sqlite3SchemaToIndex(db, pTab->pSchema);
  zDb = db->aDb[iDb].zName;

  /* Loop through all the foreign key constraints for which pTab is the
  ** child table (the table that the foreign key definition is part of).  */
  for(pFKey=pTab->pFKey; pFKey; pFKey=pFKey->pNextFrom){
    Table *pTo;                   /* Parent table of foreign key pFKey */
    Index *pIdx = 0;              /* Index on key columns in pTo */
    int *aiFree = 0;
    int *aiCol;
    int iCol;
    int i;
    int bIgnore = 0;

    if( aChange 
     && sqlite3_stricmp(pTab->zName, pFKey->zTo)!=0
     && fkChildIsModified(pTab, pFKey, aChange, bChngRowid)==0 
    ){
      continue;
    }

    /* Find the parent table of this foreign key. Also find a unique index 
    ** on the parent key columns in the parent table. If either of these 
    ** schema items cannot be located, set an error in pParse and return 
    ** early.  */
    if( pParse->disableTriggers ){
      pTo = sqlite3FindTable(db, pFKey->zTo, zDb);
    }else{
      pTo = sqlite3LocateTable(pParse, 0, pFKey->zTo, zDb);
    }
    if( !pTo || sqlite3FkLocateIndex(pParse, pTo, pFKey, &pIdx, &aiFree) ){
      assert( isIgnoreErrors==0 || (regOld!=0 && regNew==0) );
      if( !isIgnoreErrors || db->mallocFailed ) return;
      if( pTo==0 ){
        /* If isIgnoreErrors is true, then a table is being dropped. In this
        ** case SQLite runs a "DELETE FROM xxx" on the table being dropped
        ** before actually dropping it in order to check FK constraints.
        ** If the parent table of an FK constraint on the current table is
        ** missing, behave as if it is empty. i.e. decrement the relevant
        ** FK counter for each row of the current table with non-NULL keys.
        */
        Vdbe *v = sqlite3GetVdbe(pParse);
        int iJump = sqlite3VdbeCurrentAddr(v) + pFKey->nCol + 1;
        for(i=0; i<pFKey->nCol; i++){
          int iReg = pFKey->aCol[i].iFrom + regOld + 1;
          sqlite3VdbeAddOp2(v, OP_IsNull, iReg, iJump); VdbeCoverage(v);
        }
        sqlite3VdbeAddOp2(v, OP_FkCounter, pFKey->isDeferred, -1);
      }
      continue;
    }
    assert( pFKey->nCol==1 || (aiFree && pIdx) );

    if( aiFree ){
      aiCol = aiFree;
    }else{
      iCol = pFKey->aCol[0].iFrom;
      aiCol = &iCol;
    }
    for(i=0; i<pFKey->nCol; i++){
      if( aiCol[i]==pTab->iPKey ){
        aiCol[i] = -1;
      }
      assert( pIdx==0 || pIdx->aiColumn[i]>=0 );
#ifndef SQLITE_OMIT_AUTHORIZATION
      /* Request permission to read the parent key columns. If the 
      ** authorization callback returns SQLITE_IGNORE, behave as if any
      ** values read from the parent table are NULL. */
      if( db->xAuth ){
        int rcauth;
        char *zCol = pTo->aCol[pIdx ? pIdx->aiColumn[i] : pTo->iPKey].zName;
        rcauth = sqlite3AuthReadCol(pParse, pTo->zName, zCol, iDb);
        bIgnore = (rcauth==SQLITE_IGNORE);
      }
#endif
    }

    /* Take a shared-cache advisory read-lock on the parent table. Allocate 
    ** a cursor to use to search the unique index on the parent key columns 
    ** in the parent table.  */
    sqlite3TableLock(pParse, iDb, pTo->tnum, 0, pTo->zName);
    pParse->nTab++;

    if( regOld!=0 ){
      /* A row is being removed from the child table. Search for the parent.
      ** If the parent does not exist, removing the child row resolves an 
      ** outstanding foreign key constraint violation. */
      fkLookupParent(pParse, iDb, pTo, pIdx, pFKey, aiCol, regOld, -1, bIgnore);
    }
    if( regNew!=0 && !isSetNullAction(pParse, pFKey) ){
      /* A row is being added to the child table. If a parent row cannot
      ** be found, adding the child row has violated the FK constraint. 
      **
      ** If this operation is being performed as part of a trigger program
      ** that is actually a "SET NULL" action belonging to this very 
      ** foreign key, then omit this scan altogether. As all child key
      ** values are guaranteed to be NULL, it is not possible for adding
      ** this row to cause an FK violation.  */
      fkLookupParent(pParse, iDb, pTo, pIdx, pFKey, aiCol, regNew, +1, bIgnore);
    }

    sqlite3DbFree(db, aiFree);
  }

  /* Loop through all the foreign key constraints that refer to this table.
  ** (the "child" constraints) */
  for(pFKey = sqlite3FkReferences(pTab); pFKey; pFKey=pFKey->pNextTo){
    Index *pIdx = 0;              /* Foreign key index for pFKey */
    SrcList *pSrc;
    int *aiCol = 0;

    if( aChange && fkParentIsModified(pTab, pFKey, aChange, bChngRowid)==0 ){
      continue;
    }

    if( !pFKey->isDeferred && !(db->flags & SQLITE_DeferFKs) 
     && !pParse->pToplevel && !pParse->isMultiWrite 
    ){
      assert( regOld==0 && regNew!=0 );
      /* Inserting a single row into a parent table cannot cause (or fix)
      ** an immediate foreign key violation. So do nothing in this case.  */
      continue;
    }

    if( sqlite3FkLocateIndex(pParse, pTab, pFKey, &pIdx, &aiCol) ){
      if( !isIgnoreErrors || db->mallocFailed ) return;
      continue;
    }
    assert( aiCol || pFKey->nCol==1 );

    /* Create a SrcList structure containing the child table.  We need the
    ** child table as a SrcList for sqlite3WhereBegin() */
    pSrc = sqlite3SrcListAppend(db, 0, 0, 0);
    if( pSrc ){
      struct SrcList_item *pItem = pSrc->a;
      pItem->pTab = pFKey->pFrom;
      pItem->zName = pFKey->pFrom->zName;
      pItem->pTab->nRef++;
      pItem->iCursor = pParse->nTab++;
  
      if( regNew!=0 ){
        fkScanChildren(pParse, pSrc, pTab, pIdx, pFKey, aiCol, regNew, -1);
      }
      if( regOld!=0 ){
        int eAction = pFKey->aAction[aChange!=0];
        fkScanChildren(pParse, pSrc, pTab, pIdx, pFKey, aiCol, regOld, 1);
        /* If this is a deferred FK constraint, or a CASCADE or SET NULL
        ** action applies, then any foreign key violations caused by
        ** removing the parent key will be rectified by the action trigger.
        ** So do not set the "may-abort" flag in this case.
        **
        ** Note 1: If the FK is declared "ON UPDATE CASCADE", then the
        ** may-abort flag will eventually be set on this statement anyway
        ** (when this function is called as part of processing the UPDATE
        ** within the action trigger).
        **
        ** Note 2: At first glance it may seem like SQLite could simply omit
        ** all OP_FkCounter related scans when either CASCADE or SET NULL
        ** applies. The trouble starts if the CASCADE or SET NULL action 
        ** trigger causes other triggers or action rules attached to the 
        ** child table to fire. In these cases the fk constraint counters
        ** might be set incorrectly if any OP_FkCounter related scans are 
        ** omitted.  */
        if( !pFKey->isDeferred && eAction!=OE_Cascade && eAction!=OE_SetNull ){
          sqlite3MayAbort(pParse);
        }
      }
      pItem->zName = 0;
      sqlite3SrcListDelete(db, pSrc);
    }
    sqlite3DbFree(db, aiCol);
  }
}

#define COLUMN_MASK(x) (((x)>31) ? 0xffffffff : ((u32)1<<(x)))

/*
** This function is called before generating code to update or delete a 
** row contained in table pTab.
*/
SQLITE_PRIVATE u32 sqlite3FkOldmask(
  Parse *pParse,                  /* Parse context */
  Table *pTab                     /* Table being modified */
){
  u32 mask = 0;
  if( pParse->db->flags&SQLITE_ForeignKeys ){
    FKey *p;
    int i;
    for(p=pTab->pFKey; p; p=p->pNextFrom){
      for(i=0; i<p->nCol; i++) mask |= COLUMN_MASK(p->aCol[i].iFrom);
    }
    for(p=sqlite3FkReferences(pTab); p; p=p->pNextTo){
      Index *pIdx = 0;
      sqlite3FkLocateIndex(pParse, pTab, p, &pIdx, 0);
      if( pIdx ){
        for(i=0; i<pIdx->nKeyCol; i++){
          assert( pIdx->aiColumn[i]>=0 );
          mask |= COLUMN_MASK(pIdx->aiColumn[i]);
        }
      }
    }
  }
  return mask;
}


/*
** This function is called before generating code to update or delete a 
** row contained in table pTab. If the operation is a DELETE, then
** parameter aChange is passed a NULL value. For an UPDATE, aChange points
** to an array of size N, where N is the number of columns in table pTab.
** If the i'th column is not modified by the UPDATE, then the corresponding 
** entry in the aChange[] array is set to -1. If the column is modified,
** the value is 0 or greater. Parameter chngRowid is set to true if the
** UPDATE statement modifies the rowid fields of the table.
**
** If any foreign key processing will be required, this function returns
** true. If there is no foreign key related processing, this function 
** returns false.
*/
SQLITE_PRIVATE int sqlite3FkRequired(
  Parse *pParse,                  /* Parse context */
  Table *pTab,                    /* Table being modified */
  int *aChange,                   /* Non-NULL for UPDATE operations */
  int chngRowid                   /* True for UPDATE that affects rowid */
){
  if( pParse->db->flags&SQLITE_ForeignKeys ){
    if( !aChange ){
      /* A DELETE operation. Foreign key processing is required if the 
      ** table in question is either the child or parent table for any 
      ** foreign key constraint.  */
      return (sqlite3FkReferences(pTab) || pTab->pFKey);
    }else{
      /* This is an UPDATE. Foreign key processing is only required if the
      ** operation modifies one or more child or parent key columns. */
      FKey *p;

      /* Check if any child key columns are being modified. */
      for(p=pTab->pFKey; p; p=p->pNextFrom){
        if( fkChildIsModified(pTab, p, aChange, chngRowid) ) return 1;
      }

      /* Check if any parent key columns are being modified. */
      for(p=sqlite3FkReferences(pTab); p; p=p->pNextTo){
        if( fkParentIsModified(pTab, p, aChange, chngRowid) ) return 1;
      }
    }
  }
  return 0;
}

/*
** This function is called when an UPDATE or DELETE operation is being 
** compiled on table pTab, which is the parent table of foreign-key pFKey.
** If the current operation is an UPDATE, then the pChanges parameter is
** passed a pointer to the list of columns being modified. If it is a
** DELETE, pChanges is passed a NULL pointer.
**
** It returns a pointer to a Trigger structure containing a trigger
** equivalent to the ON UPDATE or ON DELETE action specified by pFKey.
** If the action is "NO ACTION" or "RESTRICT", then a NULL pointer is
** returned (these actions require no special handling by the triggers
** sub-system, code for them is created by fkScanChildren()).
**
** For example, if pFKey is the foreign key and pTab is table "p" in 
** the following schema:
**
**   CREATE TABLE p(pk PRIMARY KEY);
**   CREATE TABLE c(ck REFERENCES p ON DELETE CASCADE);
**
** then the returned trigger structure is equivalent to:
**
**   CREATE TRIGGER ... DELETE ON p BEGIN
**     DELETE FROM c WHERE ck = old.pk;
**   END;
**
** The returned pointer is cached as part of the foreign key object. It
** is eventually freed along with the rest of the foreign key object by 
** sqlite3FkDelete().
*/
static Trigger *fkActionTrigger(
  Parse *pParse,                  /* Parse context */
  Table *pTab,                    /* Table being updated or deleted from */
  FKey *pFKey,                    /* Foreign key to get action for */
  ExprList *pChanges              /* Change-list for UPDATE, NULL for DELETE */
){
  sqlite3 *db = pParse->db;       /* Database handle */
  int action;                     /* One of OE_None, OE_Cascade etc. */
  Trigger *pTrigger;              /* Trigger definition to return */
  int iAction = (pChanges!=0);    /* 1 for UPDATE, 0 for DELETE */

  action = pFKey->aAction[iAction];
  if( action==OE_Restrict && (db->flags & SQLITE_DeferFKs) ){
    return 0;
  }
  pTrigger = pFKey->apTrigger[iAction];

  if( action!=OE_None && !pTrigger ){
    char const *zFrom;            /* Name of child table */
    int nFrom;                    /* Length in bytes of zFrom */
    Index *pIdx = 0;              /* Parent key index for this FK */
    int *aiCol = 0;               /* child table cols -> parent key cols */
    TriggerStep *pStep = 0;        /* First (only) step of trigger program */
    Expr *pWhere = 0;             /* WHERE clause of trigger step */
    ExprList *pList = 0;          /* Changes list if ON UPDATE CASCADE */
    Select *pSelect = 0;          /* If RESTRICT, "SELECT RAISE(...)" */
    int i;                        /* Iterator variable */
    Expr *pWhen = 0;              /* WHEN clause for the trigger */

    if( sqlite3FkLocateIndex(pParse, pTab, pFKey, &pIdx, &aiCol) ) return 0;
    assert( aiCol || pFKey->nCol==1 );

    for(i=0; i<pFKey->nCol; i++){
      Token tOld = { "old", 3 };  /* Literal "old" token */
      Token tNew = { "new", 3 };  /* Literal "new" token */
      Token tFromCol;             /* Name of column in child table */
      Token tToCol;               /* Name of column in parent table */
      int iFromCol;               /* Idx of column in child table */
      Expr *pEq;                  /* tFromCol = OLD.tToCol */

      iFromCol = aiCol ? aiCol[i] : pFKey->aCol[0].iFrom;
      assert( iFromCol>=0 );
      assert( pIdx!=0 || (pTab->iPKey>=0 && pTab->iPKey<pTab->nCol) );
      assert( pIdx==0 || pIdx->aiColumn[i]>=0 );
      sqlite3TokenInit(&tToCol,
                   pTab->aCol[pIdx ? pIdx->aiColumn[i] : pTab->iPKey].zName);
      sqlite3TokenInit(&tFromCol, pFKey->pFrom->aCol[iFromCol].zName);

      /* Create the expression "OLD.zToCol = zFromCol". It is important
      ** that the "OLD.zToCol" term is on the LHS of the = operator, so
      ** that the affinity and collation sequence associated with the
      ** parent table are used for the comparison. */
      pEq = sqlite3PExpr(pParse, TK_EQ,
          sqlite3PExpr(pParse, TK_DOT, 
            sqlite3ExprAlloc(db, TK_ID, &tOld, 0),
            sqlite3ExprAlloc(db, TK_ID, &tToCol, 0)
          , 0),
          sqlite3ExprAlloc(db, TK_ID, &tFromCol, 0)
      , 0);
      pWhere = sqlite3ExprAnd(db, pWhere, pEq);

      /* For ON UPDATE, construct the next term of the WHEN clause.
      ** The final WHEN clause will be like this:
      **
      **    WHEN NOT(old.col1 IS new.col1 AND ... AND old.colN IS new.colN)
      */
      if( pChanges ){
        pEq = sqlite3PExpr(pParse, TK_IS,
            sqlite3PExpr(pParse, TK_DOT, 
              sqlite3ExprAlloc(db, TK_ID, &tOld, 0),
              sqlite3ExprAlloc(db, TK_ID, &tToCol, 0),
              0),
            sqlite3PExpr(pParse, TK_DOT, 
              sqlite3ExprAlloc(db, TK_ID, &tNew, 0),
              sqlite3ExprAlloc(db, TK_ID, &tToCol, 0),
              0),
            0);
        pWhen = sqlite3ExprAnd(db, pWhen, pEq);
      }
  
      if( action!=OE_Restrict && (action!=OE_Cascade || pChanges) ){
        Expr *pNew;
        if( action==OE_Cascade ){
          pNew = sqlite3PExpr(pParse, TK_DOT, 
            sqlite3ExprAlloc(db, TK_ID, &tNew, 0),
            sqlite3ExprAlloc(db, TK_ID, &tToCol, 0)
          , 0);
        }else if( action==OE_SetDflt ){
          Expr *pDflt = pFKey->pFrom->aCol[iFromCol].pDflt;
          if( pDflt ){
            pNew = sqlite3ExprDup(db, pDflt, 0);
          }else{
            pNew = sqlite3PExpr(pParse, TK_NULL, 0, 0, 0);
          }
        }else{
          pNew = sqlite3PExpr(pParse, TK_NULL, 0, 0, 0);
        }
        pList = sqlite3ExprListAppend(pParse, pList, pNew);
        sqlite3ExprListSetName(pParse, pList, &tFromCol, 0);
      }
    }
    sqlite3DbFree(db, aiCol);

    zFrom = pFKey->pFrom->zName;
    nFrom = sqlite3Strlen30(zFrom);

    if( action==OE_Restrict ){
      Token tFrom;
      Expr *pRaise; 

      tFrom.z = zFrom;
      tFrom.n = nFrom;
      pRaise = sqlite3Expr(db, TK_RAISE, "FOREIGN KEY constraint failed");
      if( pRaise ){
        pRaise->affinity = OE_Abort;
      }
      pSelect = sqlite3SelectNew(pParse, 
          sqlite3ExprListAppend(pParse, 0, pRaise),
          sqlite3SrcListAppend(db, 0, &tFrom, 0),
          pWhere,
          0, 0, 0, 0, 0, 0
      );
      pWhere = 0;
    }

    /* Disable lookaside memory allocation */
    db->lookaside.bDisable++;

    pTrigger = (Trigger *)sqlite3DbMallocZero(db, 
        sizeof(Trigger) +         /* struct Trigger */
        sizeof(TriggerStep) +     /* Single step in trigger program */
        nFrom + 1                 /* Space for pStep->zTarget */
    );
    if( pTrigger ){
      pStep = pTrigger->step_list = (TriggerStep *)&pTrigger[1];
      pStep->zTarget = (char *)&pStep[1];
      memcpy((char *)pStep->zTarget, zFrom, nFrom);
  
      pStep->pWhere = sqlite3ExprDup(db, pWhere, EXPRDUP_REDUCE);
      pStep->pExprList = sqlite3ExprListDup(db, pList, EXPRDUP_REDUCE);
      pStep->pSelect = sqlite3SelectDup(db, pSelect, EXPRDUP_REDUCE);
      if( pWhen ){
        pWhen = sqlite3PExpr(pParse, TK_NOT, pWhen, 0, 0);
        pTrigger->pWhen = sqlite3ExprDup(db, pWhen, EXPRDUP_REDUCE);
      }
    }

    /* Re-enable the lookaside buffer, if it was disabled earlier. */
    db->lookaside.bDisable--;

    sqlite3ExprDelete(db, pWhere);
    sqlite3ExprDelete(db, pWhen);
    sqlite3ExprListDelete(db, pList);
    sqlite3SelectDelete(db, pSelect);
    if( db->mallocFailed==1 ){
      fkTriggerDelete(db, pTrigger);
      return 0;
    }
    assert( pStep!=0 );

    switch( action ){
      case OE_Restrict:
        pStep->op = TK_SELECT; 
        break;
      case OE_Cascade: 
        if( !pChanges ){ 
          pStep->op = TK_DELETE; 
          break; 
        }
      default:
        pStep->op = TK_UPDATE;
    }
    pStep->pTrig = pTrigger;
    pTrigger->pSchema = pTab->pSchema;
    pTrigger->pTabSchema = pTab->pSchema;
    pFKey->apTrigger[iAction] = pTrigger;
    pTrigger->op = (pChanges ? TK_UPDATE : TK_DELETE);
  }

  return pTrigger;
}

/*
** This function is called when deleting or updating a row to implement
** any required CASCADE, SET NULL or SET DEFAULT actions.
*/
SQLITE_PRIVATE void sqlite3FkActions(
  Parse *pParse,                  /* Parse context */
  Table *pTab,                    /* Table being updated or deleted from */
  ExprList *pChanges,             /* Change-list for UPDATE, NULL for DELETE */
  int regOld,                     /* Address of array containing old row */
  int *aChange,                   /* Array indicating UPDATEd columns (or 0) */
  int bChngRowid                  /* True if rowid is UPDATEd */
){
  /* If foreign-key support is enabled, iterate through all FKs that 
  ** refer to table pTab. If there is an action associated with the FK 
  ** for this operation (either update or delete), invoke the associated 
  ** trigger sub-program.  */
  if( pParse->db->flags&SQLITE_ForeignKeys ){
    FKey *pFKey;                  /* Iterator variable */
    for(pFKey = sqlite3FkReferences(pTab); pFKey; pFKey=pFKey->pNextTo){
      if( aChange==0 || fkParentIsModified(pTab, pFKey, aChange, bChngRowid) ){
        Trigger *pAct = fkActionTrigger(pParse, pTab, pFKey, pChanges);
        if( pAct ){
          sqlite3CodeRowTriggerDirect(pParse, pAct, pTab, regOld, OE_Abort, 0);
        }
      }
    }
  }
}

#endif /* ifndef SQLITE_OMIT_TRIGGER */

/*
** Free all memory associated with foreign key definitions attached to
** table pTab. Remove the deleted foreign keys from the Schema.fkeyHash
** hash table.
*/
SQLITE_PRIVATE void sqlite3FkDelete(sqlite3 *db, Table *pTab){
  FKey *pFKey;                    /* Iterator variable */
  FKey *pNext;                    /* Copy of pFKey->pNextFrom */

  assert( db==0 || sqlite3SchemaMutexHeld(db, 0, pTab->pSchema) );
  for(pFKey=pTab->pFKey; pFKey; pFKey=pNext){

    /* Remove the FK from the fkeyHash hash table. */
    if( !db || db->pnBytesFreed==0 ){
      if( pFKey->pPrevTo ){
        pFKey->pPrevTo->pNextTo = pFKey->pNextTo;
      }else{
        void *p = (void *)pFKey->pNextTo;
        const char *z = (p ? pFKey->pNextTo->zTo : pFKey->zTo);
        sqlite3HashInsert(&pTab->pSchema->fkeyHash, z, p);
      }
      if( pFKey->pNextTo ){
        pFKey->pNextTo->pPrevTo = pFKey->pPrevTo;
      }
    }

    /* EV: R-30323-21917 Each foreign key constraint in SQLite is
    ** classified as either immediate or deferred.
    */
    assert( pFKey->isDeferred==0 || pFKey->isDeferred==1 );

    /* Delete any triggers created to implement actions for this FK. */
#ifndef SQLITE_OMIT_TRIGGER
    fkTriggerDelete(db, pFKey->apTrigger[0]);
    fkTriggerDelete(db, pFKey->apTrigger[1]);
#endif

    pNext = pFKey->pNextFrom;
    sqlite3DbFree(db, pFKey);
  }
}
#endif /* ifndef SQLITE_OMIT_FOREIGN_KEY */

/************** End of fkey.c ************************************************/
