/************** Begin file bitvec.c ******************************************/
/*
** 2008 February 16
**
** The author disclaims copyright to this source code.  In place of
** a legal notice, here is a blessing:
**
**    May you do good and not evil.
**    May you find forgiveness for yourself and forgive others.
**    May you share freely, never taking more than you give.
**
*************************************************************************
** This file implements an object that represents a fixed-length
** bitmap.  Bits are numbered starting with 1.
**
** A bitmap is used to record which pages of a database file have been
** journalled during a transaction, or which pages have the "dont-write"
** property.  Usually only a few pages are meet either condition.
** So the bitmap is usually sparse and has low cardinality.
** But sometimes (for example when during a DROP of a large table) most
** or all of the pages in a database can get journalled.  In those cases, 
** the bitmap becomes dense with high cardinality.  The algorithm needs 
** to handle both cases well.
**
** The size of the bitmap is fixed when the object is created.
**
** All bits are clear when the bitmap is created.  Individual bits
** may be set or cleared one at a time.
**
** Test operations are about 100 times more common that set operations.
** Clear operations are exceedingly rare.  There are usually between
** 5 and 500 set operations per Bitvec object, though the number of sets can
** sometimes grow into tens of thousands or larger.  The size of the
** Bitvec object is the number of pages in the database file at the
** start of a transaction, and is thus usually less than a few thousand,
** but can be as large as 2 billion for a really big database.
*/
/* #include "sqliteInt.h" */

/* Size of the Bitvec structure in bytes. */
#define BITVEC_SZ        512

/* Round the union size down to the nearest pointer boundary, since that's how 
** it will be aligned within the Bitvec struct. */
#define BITVEC_USIZE \
    (((BITVEC_SZ-(3*sizeof(u32)))/sizeof(Bitvec*))*sizeof(Bitvec*))

/* Type of the array "element" for the bitmap representation. 
** Should be a power of 2, and ideally, evenly divide into BITVEC_USIZE. 
** Setting this to the "natural word" size of your CPU may improve
** performance. */
#define BITVEC_TELEM     u8
/* Size, in bits, of the bitmap element. */
#define BITVEC_SZELEM    8
/* Number of elements in a bitmap array. */
#define BITVEC_NELEM     (BITVEC_USIZE/sizeof(BITVEC_TELEM))
/* Number of bits in the bitmap array. */
#define BITVEC_NBIT      (BITVEC_NELEM*BITVEC_SZELEM)

/* Number of u32 values in hash table. */
#define BITVEC_NINT      (BITVEC_USIZE/sizeof(u32))
/* Maximum number of entries in hash table before 
** sub-dividing and re-hashing. */
#define BITVEC_MXHASH    (BITVEC_NINT/2)
/* Hashing function for the aHash representation.
** Empirical testing showed that the *37 multiplier 
** (an arbitrary prime)in the hash function provided 
** no fewer collisions than the no-op *1. */
#define BITVEC_HASH(X)   (((X)*1)%BITVEC_NINT)

#define BITVEC_NPTR      (BITVEC_USIZE/sizeof(Bitvec *))


/*
** A bitmap is an instance of the following structure.
**
** This bitmap records the existence of zero or more bits
** with values between 1 and iSize, inclusive.
**
** There are three possible representations of the bitmap.
** If iSize<=BITVEC_NBIT, then Bitvec.u.aBitmap[] is a straight
** bitmap.  The least significant bit is bit 1.
**
** If iSize>BITVEC_NBIT and iDivisor==0 then Bitvec.u.aHash[] is
** a hash table that will hold up to BITVEC_MXHASH distinct values.
**
** Otherwise, the value i is redirected into one of BITVEC_NPTR
** sub-bitmaps pointed to by Bitvec.u.apSub[].  Each subbitmap
** handles up to iDivisor separate values of i.  apSub[0] holds
** values between 1 and iDivisor.  apSub[1] holds values between
** iDivisor+1 and 2*iDivisor.  apSub[N] holds values between
** N*iDivisor+1 and (N+1)*iDivisor.  Each subbitmap is normalized
** to hold deal with values between 1 and iDivisor.
*/
struct Bitvec {
  u32 iSize;      /* Maximum bit index.  Max iSize is 4,294,967,296. */
  u32 nSet;       /* Number of bits that are set - only valid for aHash
                  ** element.  Max is BITVEC_NINT.  For BITVEC_SZ of 512,
                  ** this would be 125. */
  u32 iDivisor;   /* Number of bits handled by each apSub[] entry. */
                  /* Should >=0 for apSub element. */
                  /* Max iDivisor is max(u32) / BITVEC_NPTR + 1.  */
                  /* For a BITVEC_SZ of 512, this would be 34,359,739. */
  union {
    BITVEC_TELEM aBitmap[BITVEC_NELEM];    /* Bitmap representation */
    u32 aHash[BITVEC_NINT];      /* Hash table representation */
    Bitvec *apSub[BITVEC_NPTR];  /* Recursive representation */
  } u;
};

/*
** Create a new bitmap object able to handle bits between 0 and iSize,
** inclusive.  Return a pointer to the new object.  Return NULL if 
** malloc fails.
*/
SQLITE_PRIVATE Bitvec *sqlite3BitvecCreate(u32 iSize){
  Bitvec *p;
  assert( sizeof(*p)==BITVEC_SZ );
  p = sqlite3MallocZero( sizeof(*p) );
  if( p ){
    p->iSize = iSize;
  }
  return p;
}

/*
** Check to see if the i-th bit is set.  Return true or false.
** If p is NULL (if the bitmap has not been created) or if
** i is out of range, then return false.
*/
SQLITE_PRIVATE int sqlite3BitvecTestNotNull(Bitvec *p, u32 i){
  assert( p!=0 );
  i--;
  if( i>=p->iSize ) return 0;
  while( p->iDivisor ){
    u32 bin = i/p->iDivisor;
    i = i%p->iDivisor;
    p = p->u.apSub[bin];
    if (!p) {
      return 0;
    }
  }
  if( p->iSize<=BITVEC_NBIT ){
    return (p->u.aBitmap[i/BITVEC_SZELEM] & (1<<(i&(BITVEC_SZELEM-1))))!=0;
  } else{
    u32 h = BITVEC_HASH(i++);
    while( p->u.aHash[h] ){
      if( p->u.aHash[h]==i ) return 1;
      h = (h+1) % BITVEC_NINT;
    }
    return 0;
  }
}
SQLITE_PRIVATE int sqlite3BitvecTest(Bitvec *p, u32 i){
  return p!=0 && sqlite3BitvecTestNotNull(p,i);
}

/*
** Set the i-th bit.  Return 0 on success and an error code if
** anything goes wrong.
**
** This routine might cause sub-bitmaps to be allocated.  Failing
** to get the memory needed to hold the sub-bitmap is the only
** that can go wrong with an insert, assuming p and i are valid.
**
** The calling function must ensure that p is a valid Bitvec object
** and that the value for "i" is within range of the Bitvec object.
** Otherwise the behavior is undefined.
*/
SQLITE_PRIVATE int sqlite3BitvecSet(Bitvec *p, u32 i){
  u32 h;
  if( p==0 ) return SQLITE_OK;
  assert( i>0 );
  assert( i<=p->iSize );
  i--;
  while((p->iSize > BITVEC_NBIT) && p->iDivisor) {
    u32 bin = i/p->iDivisor;
    i = i%p->iDivisor;
    if( p->u.apSub[bin]==0 ){
      p->u.apSub[bin] = sqlite3BitvecCreate( p->iDivisor );
      if( p->u.apSub[bin]==0 ) return SQLITE_NOMEM_BKPT;
    }
    p = p->u.apSub[bin];
  }
  if( p->iSize<=BITVEC_NBIT ){
    p->u.aBitmap[i/BITVEC_SZELEM] |= 1 << (i&(BITVEC_SZELEM-1));
    return SQLITE_OK;
  }
  h = BITVEC_HASH(i++);
  /* if there wasn't a hash collision, and this doesn't */
  /* completely fill the hash, then just add it without */
  /* worring about sub-dividing and re-hashing. */
  if( !p->u.aHash[h] ){
    if (p->nSet<(BITVEC_NINT-1)) {
      goto bitvec_set_end;
    } else {
      goto bitvec_set_rehash;
    }
  }
  /* there was a collision, check to see if it's already */
  /* in hash, if not, try to find a spot for it */
  do {
    if( p->u.aHash[h]==i ) return SQLITE_OK;
    h++;
    if( h>=BITVEC_NINT ) h = 0;
  } while( p->u.aHash[h] );
  /* we didn't find it in the hash.  h points to the first */
  /* available free spot. check to see if this is going to */
  /* make our hash too "full".  */
bitvec_set_rehash:
  if( p->nSet>=BITVEC_MXHASH ){
    unsigned int j;
    int rc;
    u32 *aiValues = sqlite3StackAllocRaw(0, sizeof(p->u.aHash));
    if( aiValues==0 ){
      return SQLITE_NOMEM_BKPT;
    }else{
      memcpy(aiValues, p->u.aHash, sizeof(p->u.aHash));
      memset(p->u.apSub, 0, sizeof(p->u.apSub));
      p->iDivisor = (p->iSize + BITVEC_NPTR - 1)/BITVEC_NPTR;
      rc = sqlite3BitvecSet(p, i);
      for(j=0; j<BITVEC_NINT; j++){
        if( aiValues[j] ) rc |= sqlite3BitvecSet(p, aiValues[j]);
      }
      sqlite3StackFree(0, aiValues);
      return rc;
    }
  }
bitvec_set_end:
  p->nSet++;
  p->u.aHash[h] = i;
  return SQLITE_OK;
}

/*
** Clear the i-th bit.
**
** pBuf must be a pointer to at least BITVEC_SZ bytes of temporary storage
** that BitvecClear can use to rebuilt its hash table.
*/
SQLITE_PRIVATE void sqlite3BitvecClear(Bitvec *p, u32 i, void *pBuf){
  if( p==0 ) return;
  assert( i>0 );
  i--;
  while( p->iDivisor ){
    u32 bin = i/p->iDivisor;
    i = i%p->iDivisor;
    p = p->u.apSub[bin];
    if (!p) {
      return;
    }
  }
  if( p->iSize<=BITVEC_NBIT ){
    p->u.aBitmap[i/BITVEC_SZELEM] &= ~(1 << (i&(BITVEC_SZELEM-1)));
  }else{
    unsigned int j;
    u32 *aiValues = pBuf;
    memcpy(aiValues, p->u.aHash, sizeof(p->u.aHash));
    memset(p->u.aHash, 0, sizeof(p->u.aHash));
    p->nSet = 0;
    for(j=0; j<BITVEC_NINT; j++){
      if( aiValues[j] && aiValues[j]!=(i+1) ){
        u32 h = BITVEC_HASH(aiValues[j]-1);
        p->nSet++;
        while( p->u.aHash[h] ){
          h++;
          if( h>=BITVEC_NINT ) h = 0;
        }
        p->u.aHash[h] = aiValues[j];
      }
    }
  }
}

/*
** Destroy a bitmap object.  Reclaim all memory used.
*/
SQLITE_PRIVATE void sqlite3BitvecDestroy(Bitvec *p){
  if( p==0 ) return;
  if( p->iDivisor ){
    unsigned int i;
    for(i=0; i<BITVEC_NPTR; i++){
      sqlite3BitvecDestroy(p->u.apSub[i]);
    }
  }
  sqlite3_free(p);
}

/*
** Return the value of the iSize parameter specified when Bitvec *p
** was created.
*/
SQLITE_PRIVATE u32 sqlite3BitvecSize(Bitvec *p){
  return p->iSize;
}

#ifndef SQLITE_OMIT_BUILTIN_TEST
/*
** Let V[] be an array of unsigned characters sufficient to hold
** up to N bits.  Let I be an integer between 0 and N.  0<=I<N.
** Then the following macros can be used to set, clear, or test
** individual bits within V.
*/
#define SETBIT(V,I)      V[I>>3] |= (1<<(I&7))
#define CLEARBIT(V,I)    V[I>>3] &= ~(1<<(I&7))
#define TESTBIT(V,I)     (V[I>>3]&(1<<(I&7)))!=0

/*
** This routine runs an extensive test of the Bitvec code.
**
** The input is an array of integers that acts as a program
** to test the Bitvec.  The integers are opcodes followed
** by 0, 1, or 3 operands, depending on the opcode.  Another
** opcode follows immediately after the last operand.
**
** There are 6 opcodes numbered from 0 through 5.  0 is the
** "halt" opcode and causes the test to end.
**
**    0          Halt and return the number of errors
**    1 N S X    Set N bits beginning with S and incrementing by X
**    2 N S X    Clear N bits beginning with S and incrementing by X
**    3 N        Set N randomly chosen bits
**    4 N        Clear N randomly chosen bits
**    5 N S X    Set N bits from S increment X in array only, not in bitvec
**
** The opcodes 1 through 4 perform set and clear operations are performed
** on both a Bitvec object and on a linear array of bits obtained from malloc.
** Opcode 5 works on the linear array only, not on the Bitvec.
** Opcode 5 is used to deliberately induce a fault in order to
** confirm that error detection works.
**
** At the conclusion of the test the linear array is compared
** against the Bitvec object.  If there are any differences,
** an error is returned.  If they are the same, zero is returned.
**
** If a memory allocation error occurs, return -1.
*/
SQLITE_PRIVATE int sqlite3BitvecBuiltinTest(int sz, int *aOp){
  Bitvec *pBitvec = 0;
  unsigned char *pV = 0;
  int rc = -1;
  int i, nx, pc, op;
  void *pTmpSpace;

  /* Allocate the Bitvec to be tested and a linear array of
  ** bits to act as the reference */
  pBitvec = sqlite3BitvecCreate( sz );
  pV = sqlite3MallocZero( (sz+7)/8 + 1 );
  pTmpSpace = sqlite3_malloc64(BITVEC_SZ);
  if( pBitvec==0 || pV==0 || pTmpSpace==0  ) goto bitvec_end;

  /* NULL pBitvec tests */
  sqlite3BitvecSet(0, 1);
  sqlite3BitvecClear(0, 1, pTmpSpace);

  /* Run the program */
  pc = 0;
  while( (op = aOp[pc])!=0 ){
    switch( op ){
      case 1:
      case 2:
      case 5: {
        nx = 4;
        i = aOp[pc+2] - 1;
        aOp[pc+2] += aOp[pc+3];
        break;
      }
      case 3:
      case 4: 
      default: {
        nx = 2;
        sqlite3_randomness(sizeof(i), &i);
        break;
      }
    }
    if( (--aOp[pc+1]) > 0 ) nx = 0;
    pc += nx;
    i = (i & 0x7fffffff)%sz;
    if( (op & 1)!=0 ){
      SETBIT(pV, (i+1));
      if( op!=5 ){
        if( sqlite3BitvecSet(pBitvec, i+1) ) goto bitvec_end;
      }
    }else{
      CLEARBIT(pV, (i+1));
      sqlite3BitvecClear(pBitvec, i+1, pTmpSpace);
    }
  }

  /* Test to make sure the linear array exactly matches the
  ** Bitvec object.  Start with the assumption that they do
  ** match (rc==0).  Change rc to non-zero if a discrepancy
  ** is found.
  */
  rc = sqlite3BitvecTest(0,0) + sqlite3BitvecTest(pBitvec, sz+1)
          + sqlite3BitvecTest(pBitvec, 0)
          + (sqlite3BitvecSize(pBitvec) - sz);
  for(i=1; i<=sz; i++){
    if(  (TESTBIT(pV,i))!=sqlite3BitvecTest(pBitvec,i) ){
      rc = i;
      break;
    }
  }

  /* Free allocated structure */
bitvec_end:
  sqlite3_free(pTmpSpace);
  sqlite3_free(pV);
  sqlite3BitvecDestroy(pBitvec);
  return rc;
}
#endif /* SQLITE_OMIT_BUILTIN_TEST */

/************** End of bitvec.c **********************************************/
/************** Begin file pcache.c ******************************************/
/*
** 2008 August 05
**
** The author disclaims copyright to this source code.  In place of
** a legal notice, here is a blessing:
**
**    May you do good and not evil.
**    May you find forgiveness for yourself and forgive others.
**    May you share freely, never taking more than you give.
**
*************************************************************************
** This file implements that page cache.
*/
/* #include "sqliteInt.h" */

/*
** A complete page cache is an instance of this structure.  Every
** entry in the cache holds a single page of the database file.  The
** btree layer only operates on the cached copy of the database pages.
**
** A page cache entry is "clean" if it exactly matches what is currently
** on disk.  A page is "dirty" if it has been modified and needs to be
** persisted to disk.
**
** pDirty, pDirtyTail, pSynced:
**   All dirty pages are linked into the doubly linked list using
**   PgHdr.pDirtyNext and pDirtyPrev. The list is maintained in LRU order
**   such that p was added to the list more recently than p->pDirtyNext.
**   PCache.pDirty points to the first (newest) element in the list and
**   pDirtyTail to the last (oldest).
**
**   The PCache.pSynced variable is used to optimize searching for a dirty
**   page to eject from the cache mid-transaction. It is better to eject
**   a page that does not require a journal sync than one that does. 
**   Therefore, pSynced is maintained to that it *almost* always points
**   to either the oldest page in the pDirty/pDirtyTail list that has a
**   clear PGHDR_NEED_SYNC flag or to a page that is older than this one
**   (so that the right page to eject can be found by following pDirtyPrev
**   pointers).
*/
struct PCache {
  PgHdr *pDirty, *pDirtyTail;         /* List of dirty pages in LRU order */
  PgHdr *pSynced;                     /* Last synced page in dirty page list */
  int nRefSum;                        /* Sum of ref counts over all pages */
  int szCache;                        /* Configured cache size */
  int szSpill;                        /* Size before spilling occurs */
  int szPage;                         /* Size of every page in this cache */
  int szExtra;                        /* Size of extra space for each page */
  u8 bPurgeable;                      /* True if pages are on backing store */
  u8 eCreate;                         /* eCreate value for for xFetch() */
  int (*xStress)(void*,PgHdr*);       /* Call to try make a page clean */
  void *pStress;                      /* Argument to xStress */
  sqlite3_pcache *pCache;             /* Pluggable cache module */
};

/********************************** Test and Debug Logic **********************/
/*
** Debug tracing macros.  Enable by by changing the "0" to "1" and
** recompiling.
**
** When sqlite3PcacheTrace is 1, single line trace messages are issued.
** When sqlite3PcacheTrace is 2, a dump of the pcache showing all cache entries
** is displayed for many operations, resulting in a lot of output.
*/
#if defined(SQLITE_DEBUG) && 0
  int sqlite3PcacheTrace = 2;       /* 0: off  1: simple  2: cache dumps */
  int sqlite3PcacheMxDump = 9999;   /* Max cache entries for pcacheDump() */
# define pcacheTrace(X) if(sqlite3PcacheTrace){sqlite3DebugPrintf X;}
  void pcacheDump(PCache *pCache){
    int N;
    int i, j;
    sqlite3_pcache_page *pLower;
    PgHdr *pPg;
    unsigned char *a;
  
    if( sqlite3PcacheTrace<2 ) return;
    if( pCache->pCache==0 ) return;
    N = sqlite3PcachePagecount(pCache);
    if( N>sqlite3PcacheMxDump ) N = sqlite3PcacheMxDump;
    for(i=1; i<=N; i++){
       pLower = sqlite3GlobalConfig.pcache2.xFetch(pCache->pCache, i, 0);
       if( pLower==0 ) continue;
       pPg = (PgHdr*)pLower->pExtra;
       printf("%3d: nRef %2d flgs %02x data ", i, pPg->nRef, pPg->flags);
       a = (unsigned char *)pLower->pBuf;
       for(j=0; j<12; j++) printf("%02x", a[j]);
       printf("\n");
       if( pPg->pPage==0 ){
         sqlite3GlobalConfig.pcache2.xUnpin(pCache->pCache, pLower, 0);
       }
    }
  }
  #else
# define pcacheTrace(X)
# define pcacheDump(X)
#endif

/*
** Check invariants on a PgHdr entry.  Return true if everything is OK.
** Return false if any invariant is violated.
**
** This routine is for use inside of assert() statements only.  For
** example:
**
**          assert( sqlite3PcachePageSanity(pPg) );
*/
#if SQLITE_DEBUG
SQLITE_PRIVATE int sqlite3PcachePageSanity(PgHdr *pPg){
  PCache *pCache;
  assert( pPg!=0 );
  assert( pPg->pgno>0 );    /* Page number is 1 or more */
  pCache = pPg->pCache;
  assert( pCache!=0 );      /* Every page has an associated PCache */
  if( pPg->flags & PGHDR_CLEAN ){
    assert( (pPg->flags & PGHDR_DIRTY)==0 );/* Cannot be both CLEAN and DIRTY */
    assert( pCache->pDirty!=pPg );          /* CLEAN pages not on dirty list */
    assert( pCache->pDirtyTail!=pPg );
  }
  /* WRITEABLE pages must also be DIRTY */
  if( pPg->flags & PGHDR_WRITEABLE ){
    assert( pPg->flags & PGHDR_DIRTY );     /* WRITEABLE implies DIRTY */
  }
  /* NEED_SYNC can be set independently of WRITEABLE.  This can happen,
  ** for example, when using the sqlite3PagerDontWrite() optimization:
  **    (1)  Page X is journalled, and gets WRITEABLE and NEED_SEEK.
  **    (2)  Page X moved to freelist, WRITEABLE is cleared
  **    (3)  Page X reused, WRITEABLE is set again
  ** If NEED_SYNC had been cleared in step 2, then it would not be reset
  ** in step 3, and page might be written into the database without first
  ** syncing the rollback journal, which might cause corruption on a power
  ** loss.
  **
  ** Another example is when the database page size is smaller than the
  ** disk sector size.  When any page of a sector is journalled, all pages
  ** in that sector are marked NEED_SYNC even if they are still CLEAN, just
  ** in case they are later modified, since all pages in the same sector
  ** must be journalled and synced before any of those pages can be safely
  ** written.
  */
  return 1;
}
#endif /* SQLITE_DEBUG */


/********************************** Linked List Management ********************/

/* Allowed values for second argument to pcacheManageDirtyList() */
#define PCACHE_DIRTYLIST_REMOVE   1    /* Remove pPage from dirty list */
#define PCACHE_DIRTYLIST_ADD      2    /* Add pPage to the dirty list */
#define PCACHE_DIRTYLIST_FRONT    3    /* Move pPage to the front of the list */

/*
** Manage pPage's participation on the dirty list.  Bits of the addRemove
** argument determines what operation to do.  The 0x01 bit means first
** remove pPage from the dirty list.  The 0x02 means add pPage back to
** the dirty list.  Doing both moves pPage to the front of the dirty list.
*/
static void pcacheManageDirtyList(PgHdr *pPage, u8 addRemove){
  PCache *p = pPage->pCache;

  pcacheTrace(("%p.DIRTYLIST.%s %d\n", p,
                addRemove==1 ? "REMOVE" : addRemove==2 ? "ADD" : "FRONT",
                pPage->pgno));
  if( addRemove & PCACHE_DIRTYLIST_REMOVE ){
    assert( pPage->pDirtyNext || pPage==p->pDirtyTail );
    assert( pPage->pDirtyPrev || pPage==p->pDirty );
  
    /* Update the PCache1.pSynced variable if necessary. */
    if( p->pSynced==pPage ){
      p->pSynced = pPage->pDirtyPrev;
    }
  
    if( pPage->pDirtyNext ){
      pPage->pDirtyNext->pDirtyPrev = pPage->pDirtyPrev;
    }else{
      assert( pPage==p->pDirtyTail );
      p->pDirtyTail = pPage->pDirtyPrev;
    }
    if( pPage->pDirtyPrev ){
      pPage->pDirtyPrev->pDirtyNext = pPage->pDirtyNext;
    }else{
      /* If there are now no dirty pages in the cache, set eCreate to 2. 
      ** This is an optimization that allows sqlite3PcacheFetch() to skip
      ** searching for a dirty page to eject from the cache when it might
      ** otherwise have to.  */
      assert( pPage==p->pDirty );
      p->pDirty = pPage->pDirtyNext;
      assert( p->bPurgeable || p->eCreate==2 );
      if( p->pDirty==0 ){         /*OPTIMIZATION-IF-TRUE*/
        assert( p->bPurgeable==0 || p->eCreate==1 );
        p->eCreate = 2;
      }
    }
    pPage->pDirtyNext = 0;
    pPage->pDirtyPrev = 0;
  }
  if( addRemove & PCACHE_DIRTYLIST_ADD ){
    assert( pPage->pDirtyNext==0 && pPage->pDirtyPrev==0 && p->pDirty!=pPage );
  
    pPage->pDirtyNext = p->pDirty;
    if( pPage->pDirtyNext ){
      assert( pPage->pDirtyNext->pDirtyPrev==0 );
      pPage->pDirtyNext->pDirtyPrev = pPage;
    }else{
      p->pDirtyTail = pPage;
      if( p->bPurgeable ){
        assert( p->eCreate==2 );
        p->eCreate = 1;
      }
    }
    p->pDirty = pPage;

    /* If pSynced is NULL and this page has a clear NEED_SYNC flag, set
    ** pSynced to point to it. Checking the NEED_SYNC flag is an 
    ** optimization, as if pSynced points to a page with the NEED_SYNC
    ** flag set sqlite3PcacheFetchStress() searches through all newer 
    ** entries of the dirty-list for a page with NEED_SYNC clear anyway.  */
    if( !p->pSynced 
     && 0==(pPage->flags&PGHDR_NEED_SYNC)   /*OPTIMIZATION-IF-FALSE*/
    ){
      p->pSynced = pPage;
    }
  }
  pcacheDump(p);
}

/*
** Wrapper around the pluggable caches xUnpin method. If the cache is
** being used for an in-memory database, this function is a no-op.
*/
static void pcacheUnpin(PgHdr *p){
  if( p->pCache->bPurgeable ){
    pcacheTrace(("%p.UNPIN %d\n", p->pCache, p->pgno));
    sqlite3GlobalConfig.pcache2.xUnpin(p->pCache->pCache, p->pPage, 0);
    pcacheDump(p->pCache);
  }
}

/*
** Compute the number of pages of cache requested.   p->szCache is the
** cache size requested by the "PRAGMA cache_size" statement.
*/
static int numberOfCachePages(PCache *p){
  if( p->szCache>=0 ){
    /* IMPLEMENTATION-OF: R-42059-47211 If the argument N is positive then the
    ** suggested cache size is set to N. */
    return p->szCache;
  }else{
    /* IMPLEMENTATION-OF: R-61436-13639 If the argument N is negative, then
    ** the number of cache pages is adjusted to use approximately abs(N*1024)
    ** bytes of memory. */
    return (int)((-1024*(i64)p->szCache)/(p->szPage+p->szExtra));
  }
}

/*************************************************** General Interfaces ******
**
** Initialize and shutdown the page cache subsystem. Neither of these 
** functions are threadsafe.
*/
SQLITE_PRIVATE int sqlite3PcacheInitialize(void){
  if( sqlite3GlobalConfig.pcache2.xInit==0 ){
    /* IMPLEMENTATION-OF: R-26801-64137 If the xInit() method is NULL, then the
    ** built-in default page cache is used instead of the application defined
    ** page cache. */
    sqlite3PCacheSetDefault();
  }
  return sqlite3GlobalConfig.pcache2.xInit(sqlite3GlobalConfig.pcache2.pArg);
}
SQLITE_PRIVATE void sqlite3PcacheShutdown(void){
  if( sqlite3GlobalConfig.pcache2.xShutdown ){
    /* IMPLEMENTATION-OF: R-26000-56589 The xShutdown() method may be NULL. */
    sqlite3GlobalConfig.pcache2.xShutdown(sqlite3GlobalConfig.pcache2.pArg);
  }
}

/*
** Return the size in bytes of a PCache object.
*/
SQLITE_PRIVATE int sqlite3PcacheSize(void){ return sizeof(PCache); }

/*
** Create a new PCache object. Storage space to hold the object
** has already been allocated and is passed in as the p pointer. 
** The caller discovers how much space needs to be allocated by 
** calling sqlite3PcacheSize().
*/
SQLITE_PRIVATE int sqlite3PcacheOpen(
  int szPage,                  /* Size of every page */
  int szExtra,                 /* Extra space associated with each page */
  int bPurgeable,              /* True if pages are on backing store */
  int (*xStress)(void*,PgHdr*),/* Call to try to make pages clean */
  void *pStress,               /* Argument to xStress */
  PCache *p                    /* Preallocated space for the PCache */
){
  memset(p, 0, sizeof(PCache));
  p->szPage = 1;
  p->szExtra = szExtra;
  p->bPurgeable = bPurgeable;
  p->eCreate = 2;
  p->xStress = xStress;
  p->pStress = pStress;
  p->szCache = 100;
  p->szSpill = 1;
  pcacheTrace(("%p.OPEN szPage %d bPurgeable %d\n",p,szPage,bPurgeable));
  return sqlite3PcacheSetPageSize(p, szPage);
}

/*
** Change the page size for PCache object. The caller must ensure that there
** are no outstanding page references when this function is called.
*/
SQLITE_PRIVATE int sqlite3PcacheSetPageSize(PCache *pCache, int szPage){
  assert( pCache->nRefSum==0 && pCache->pDirty==0 );
  if( pCache->szPage ){
    sqlite3_pcache *pNew;
    pNew = sqlite3GlobalConfig.pcache2.xCreate(
                szPage, pCache->szExtra + ROUND8(sizeof(PgHdr)),
                pCache->bPurgeable
    );
    if( pNew==0 ) return SQLITE_NOMEM_BKPT;
    sqlite3GlobalConfig.pcache2.xCachesize(pNew, numberOfCachePages(pCache));
    if( pCache->pCache ){
      sqlite3GlobalConfig.pcache2.xDestroy(pCache->pCache);
    }
    pCache->pCache = pNew;
    pCache->szPage = szPage;
    pcacheTrace(("%p.PAGESIZE %d\n",pCache,szPage));
  }
  return SQLITE_OK;
}

/*
** Try to obtain a page from the cache.
**
** This routine returns a pointer to an sqlite3_pcache_page object if
** such an object is already in cache, or if a new one is created.
** This routine returns a NULL pointer if the object was not in cache
** and could not be created.
**
** The createFlags should be 0 to check for existing pages and should
** be 3 (not 1, but 3) to try to create a new page.
**
** If the createFlag is 0, then NULL is always returned if the page
** is not already in the cache.  If createFlag is 1, then a new page
** is created only if that can be done without spilling dirty pages
** and without exceeding the cache size limit.
**
** The caller needs to invoke sqlite3PcacheFetchFinish() to properly
** initialize the sqlite3_pcache_page object and convert it into a
** PgHdr object.  The sqlite3PcacheFetch() and sqlite3PcacheFetchFinish()
** routines are split this way for performance reasons. When separated
** they can both (usually) operate without having to push values to
** the stack on entry and pop them back off on exit, which saves a
** lot of pushing and popping.
*/
SQLITE_PRIVATE sqlite3_pcache_page *sqlite3PcacheFetch(
  PCache *pCache,       /* Obtain the page from this cache */
  Pgno pgno,            /* Page number to obtain */
  int createFlag        /* If true, create page if it does not exist already */
){
  int eCreate;
  sqlite3_pcache_page *pRes;

  assert( pCache!=0 );
  assert( pCache->pCache!=0 );
  assert( createFlag==3 || createFlag==0 );
  assert( pgno>0 );
  assert( pCache->eCreate==((pCache->bPurgeable && pCache->pDirty) ? 1 : 2) );

  /* eCreate defines what to do if the page does not exist.
  **    0     Do not allocate a new page.  (createFlag==0)
  **    1     Allocate a new page if doing so is inexpensive.
  **          (createFlag==1 AND bPurgeable AND pDirty)
  **    2     Allocate a new page even it doing so is difficult.
  **          (createFlag==1 AND !(bPurgeable AND pDirty)
  */
  eCreate = createFlag & pCache->eCreate;
  assert( eCreate==0 || eCreate==1 || eCreate==2 );
  assert( createFlag==0 || pCache->eCreate==eCreate );
  assert( createFlag==0 || eCreate==1+(!pCache->bPurgeable||!pCache->pDirty) );
  pRes = sqlite3GlobalConfig.pcache2.xFetch(pCache->pCache, pgno, eCreate);
  pcacheTrace(("%p.FETCH %d%s (result: %p)\n",pCache,pgno,
               createFlag?" create":"",pRes));
  return pRes;
}

/*
** If the sqlite3PcacheFetch() routine is unable to allocate a new
** page because no clean pages are available for reuse and the cache
** size limit has been reached, then this routine can be invoked to 
** try harder to allocate a page.  This routine might invoke the stress
** callback to spill dirty pages to the journal.  It will then try to
** allocate the new page and will only fail to allocate a new page on
** an OOM error.
**
** This routine should be invoked only after sqlite3PcacheFetch() fails.
*/
SQLITE_PRIVATE int sqlite3PcacheFetchStress(
  PCache *pCache,                 /* Obtain the page from this cache */
  Pgno pgno,                      /* Page number to obtain */
  sqlite3_pcache_page **ppPage    /* Write result here */
){
  PgHdr *pPg;
  if( pCache->eCreate==2 ) return 0;

  if( sqlite3PcachePagecount(pCache)>pCache->szSpill ){
    /* Find a dirty page to write-out and recycle. First try to find a 
    ** page that does not require a journal-sync (one with PGHDR_NEED_SYNC
    ** cleared), but if that is not possible settle for any other 
    ** unreferenced dirty page.
    **
    ** If the LRU page in the dirty list that has a clear PGHDR_NEED_SYNC
    ** flag is currently referenced, then the following may leave pSynced
    ** set incorrectly (pointing to other than the LRU page with NEED_SYNC
    ** cleared). This is Ok, as pSynced is just an optimization.  */
    for(pPg=pCache->pSynced; 
        pPg && (pPg->nRef || (pPg->flags&PGHDR_NEED_SYNC)); 
        pPg=pPg->pDirtyPrev
    );
    pCache->pSynced = pPg;
    if( !pPg ){
      for(pPg=pCache->pDirtyTail; pPg && pPg->nRef; pPg=pPg->pDirtyPrev);
    }
    if( pPg ){
      int rc;
#ifdef SQLITE_LOG_CACHE_SPILL
      sqlite3_log(SQLITE_FULL, 
                  "spill page %d making room for %d - cache used: %d/%d",
                  pPg->pgno, pgno,
                  sqlite3GlobalConfig.pcache.xPagecount(pCache->pCache),
                numberOfCachePages(pCache));
#endif
      pcacheTrace(("%p.SPILL %d\n",pCache,pPg->pgno));
      rc = pCache->xStress(pCache->pStress, pPg);
      pcacheDump(pCache);
      if( rc!=SQLITE_OK && rc!=SQLITE_BUSY ){
        return rc;
      }
    }
  }
  *ppPage = sqlite3GlobalConfig.pcache2.xFetch(pCache->pCache, pgno, 2);
  return *ppPage==0 ? SQLITE_NOMEM_BKPT : SQLITE_OK;
}

/*
** This is a helper routine for sqlite3PcacheFetchFinish()
**
** In the uncommon case where the page being fetched has not been
** initialized, this routine is invoked to do the initialization.
** This routine is broken out into a separate function since it
** requires extra stack manipulation that can be avoided in the common
** case.
*/
static SQLITE_NOINLINE PgHdr *pcacheFetchFinishWithInit(
  PCache *pCache,             /* Obtain the page from this cache */
  Pgno pgno,                  /* Page number obtained */
  sqlite3_pcache_page *pPage  /* Page obtained by prior PcacheFetch() call */
){
  PgHdr *pPgHdr;
  assert( pPage!=0 );
  pPgHdr = (PgHdr*)pPage->pExtra;
  assert( pPgHdr->pPage==0 );
  memset(pPgHdr, 0, sizeof(PgHdr));
  pPgHdr->pPage = pPage;
  pPgHdr->pData = pPage->pBuf;
  pPgHdr->pExtra = (void *)&pPgHdr[1];
  memset(pPgHdr->pExtra, 0, pCache->szExtra);
  pPgHdr->pCache = pCache;
  pPgHdr->pgno = pgno;
  pPgHdr->flags = PGHDR_CLEAN;
  return sqlite3PcacheFetchFinish(pCache,pgno,pPage);
}

/*
** This routine converts the sqlite3_pcache_page object returned by
** sqlite3PcacheFetch() into an initialized PgHdr object.  This routine
** must be called after sqlite3PcacheFetch() in order to get a usable
** result.
*/
SQLITE_PRIVATE PgHdr *sqlite3PcacheFetchFinish(
  PCache *pCache,             /* Obtain the page from this cache */
  Pgno pgno,                  /* Page number obtained */
  sqlite3_pcache_page *pPage  /* Page obtained by prior PcacheFetch() call */
){
  PgHdr *pPgHdr;

  assert( pPage!=0 );
  pPgHdr = (PgHdr *)pPage->pExtra;

  if( !pPgHdr->pPage ){
    return pcacheFetchFinishWithInit(pCache, pgno, pPage);
  }
  pCache->nRefSum++;
  pPgHdr->nRef++;
  assert( sqlite3PcachePageSanity(pPgHdr) );
  return pPgHdr;
}

/*
** Decrement the reference count on a page. If the page is clean and the
** reference count drops to 0, then it is made eligible for recycling.
*/
SQLITE_PRIVATE void SQLITE_NOINLINE sqlite3PcacheRelease(PgHdr *p){
  assert( p->nRef>0 );
  p->pCache->nRefSum--;
  if( (--p->nRef)==0 ){
    if( p->flags&PGHDR_CLEAN ){
      pcacheUnpin(p);
    }else if( p->pDirtyPrev!=0 ){ /*OPTIMIZATION-IF-FALSE*/
      /* Move the page to the head of the dirty list. If p->pDirtyPrev==0,
      ** then page p is already at the head of the dirty list and the
      ** following call would be a no-op. Hence the OPTIMIZATION-IF-FALSE
      ** tag above.  */
      pcacheManageDirtyList(p, PCACHE_DIRTYLIST_FRONT);
    }
  }
}

/*
** Increase the reference count of a supplied page by 1.
*/
SQLITE_PRIVATE void sqlite3PcacheRef(PgHdr *p){
  assert(p->nRef>0);
  assert( sqlite3PcachePageSanity(p) );
  p->nRef++;
  p->pCache->nRefSum++;
}

/*
** Drop a page from the cache. There must be exactly one reference to the
** page. This function deletes that reference, so after it returns the
** page pointed to by p is invalid.
*/
SQLITE_PRIVATE void sqlite3PcacheDrop(PgHdr *p){
  assert( p->nRef==1 );
  assert( sqlite3PcachePageSanity(p) );
  if( p->flags&PGHDR_DIRTY ){
    pcacheManageDirtyList(p, PCACHE_DIRTYLIST_REMOVE);
  }
  p->pCache->nRefSum--;
  sqlite3GlobalConfig.pcache2.xUnpin(p->pCache->pCache, p->pPage, 1);
}

/*
** Make sure the page is marked as dirty. If it isn't dirty already,
** make it so.
*/
SQLITE_PRIVATE void sqlite3PcacheMakeDirty(PgHdr *p){
  assert( p->nRef>0 );
  assert( sqlite3PcachePageSanity(p) );
  if( p->flags & (PGHDR_CLEAN|PGHDR_DONT_WRITE) ){    /*OPTIMIZATION-IF-FALSE*/
    p->flags &= ~PGHDR_DONT_WRITE;
    if( p->flags & PGHDR_CLEAN ){
      p->flags ^= (PGHDR_DIRTY|PGHDR_CLEAN);
      pcacheTrace(("%p.DIRTY %d\n",p->pCache,p->pgno));
      assert( (p->flags & (PGHDR_DIRTY|PGHDR_CLEAN))==PGHDR_DIRTY );
      pcacheManageDirtyList(p, PCACHE_DIRTYLIST_ADD);
    }
    assert( sqlite3PcachePageSanity(p) );
  }
}

/*
** Make sure the page is marked as clean. If it isn't clean already,
** make it so.
*/
SQLITE_PRIVATE void sqlite3PcacheMakeClean(PgHdr *p){
  assert( sqlite3PcachePageSanity(p) );
  if( ALWAYS((p->flags & PGHDR_DIRTY)!=0) ){
    assert( (p->flags & PGHDR_CLEAN)==0 );
    pcacheManageDirtyList(p, PCACHE_DIRTYLIST_REMOVE);
    p->flags &= ~(PGHDR_DIRTY|PGHDR_NEED_SYNC|PGHDR_WRITEABLE);
    p->flags |= PGHDR_CLEAN;
    pcacheTrace(("%p.CLEAN %d\n",p->pCache,p->pgno));
    assert( sqlite3PcachePageSanity(p) );
    if( p->nRef==0 ){
      pcacheUnpin(p);
    }
  }
}

/*
** Make every page in the cache clean.
*/
SQLITE_PRIVATE void sqlite3PcacheCleanAll(PCache *pCache){
  PgHdr *p;
  pcacheTrace(("%p.CLEAN-ALL\n",pCache));
  while( (p = pCache->pDirty)!=0 ){
    sqlite3PcacheMakeClean(p);
  }
}

/*
** Clear the PGHDR_NEED_SYNC and PGHDR_WRITEABLE flag from all dirty pages.
*/
SQLITE_PRIVATE void sqlite3PcacheClearWritable(PCache *pCache){
  PgHdr *p;
  pcacheTrace(("%p.CLEAR-WRITEABLE\n",pCache));
  for(p=pCache->pDirty; p; p=p->pDirtyNext){
    p->flags &= ~(PGHDR_NEED_SYNC|PGHDR_WRITEABLE);
  }
  pCache->pSynced = pCache->pDirtyTail;
}

/*
** Clear the PGHDR_NEED_SYNC flag from all dirty pages.
*/
SQLITE_PRIVATE void sqlite3PcacheClearSyncFlags(PCache *pCache){
  PgHdr *p;
  for(p=pCache->pDirty; p; p=p->pDirtyNext){
    p->flags &= ~PGHDR_NEED_SYNC;
  }
  pCache->pSynced = pCache->pDirtyTail;
}

/*
** Change the page number of page p to newPgno. 
*/
SQLITE_PRIVATE void sqlite3PcacheMove(PgHdr *p, Pgno newPgno){
  PCache *pCache = p->pCache;
  assert( p->nRef>0 );
  assert( newPgno>0 );
  assert( sqlite3PcachePageSanity(p) );
  pcacheTrace(("%p.MOVE %d -> %d\n",pCache,p->pgno,newPgno));
  sqlite3GlobalConfig.pcache2.xRekey(pCache->pCache, p->pPage, p->pgno,newPgno);
  p->pgno = newPgno;
  if( (p->flags&PGHDR_DIRTY) && (p->flags&PGHDR_NEED_SYNC) ){
    pcacheManageDirtyList(p, PCACHE_DIRTYLIST_FRONT);
  }
}

/*
** Drop every cache entry whose page number is greater than "pgno". The
** caller must ensure that there are no outstanding references to any pages
** other than page 1 with a page number greater than pgno.
**
** If there is a reference to page 1 and the pgno parameter passed to this
** function is 0, then the data area associated with page 1 is zeroed, but
** the page object is not dropped.
*/
SQLITE_PRIVATE void sqlite3PcacheTruncate(PCache *pCache, Pgno pgno){
  if( pCache->pCache ){
    PgHdr *p;
    PgHdr *pNext;
    pcacheTrace(("%p.TRUNCATE %d\n",pCache,pgno));
    for(p=pCache->pDirty; p; p=pNext){
      pNext = p->pDirtyNext;
      /* This routine never gets call with a positive pgno except right
      ** after sqlite3PcacheCleanAll().  So if there are dirty pages,
      ** it must be that pgno==0.
      */
      assert( p->pgno>0 );
      if( p->pgno>pgno ){
        assert( p->flags&PGHDR_DIRTY );
        sqlite3PcacheMakeClean(p);
      }
    }
    if( pgno==0 && pCache->nRefSum ){
      sqlite3_pcache_page *pPage1;
      pPage1 = sqlite3GlobalConfig.pcache2.xFetch(pCache->pCache,1,0);
      if( ALWAYS(pPage1) ){  /* Page 1 is always available in cache, because
                             ** pCache->nRefSum>0 */
        memset(pPage1->pBuf, 0, pCache->szPage);
        pgno = 1;
      }
    }
    sqlite3GlobalConfig.pcache2.xTruncate(pCache->pCache, pgno+1);
  }
}

/*
** Close a cache.
*/
SQLITE_PRIVATE void sqlite3PcacheClose(PCache *pCache){
  assert( pCache->pCache!=0 );
  pcacheTrace(("%p.CLOSE\n",pCache));
  sqlite3GlobalConfig.pcache2.xDestroy(pCache->pCache);
}

/* 
** Discard the contents of the cache.
*/
SQLITE_PRIVATE void sqlite3PcacheClear(PCache *pCache){
  sqlite3PcacheTruncate(pCache, 0);
}

/*
** Merge two lists of pages connected by pDirty and in pgno order.
** Do not both fixing the pDirtyPrev pointers.
*/
static PgHdr *pcacheMergeDirtyList(PgHdr *pA, PgHdr *pB){
  PgHdr result, *pTail;
  pTail = &result;
  while( pA && pB ){
    if( pA->pgno<pB->pgno ){
      pTail->pDirty = pA;
      pTail = pA;
      pA = pA->pDirty;
    }else{
      pTail->pDirty = pB;
      pTail = pB;
      pB = pB->pDirty;
    }
  }
  if( pA ){
    pTail->pDirty = pA;
  }else if( pB ){
    pTail->pDirty = pB;
  }else{
    pTail->pDirty = 0;
  }
  return result.pDirty;
}

/*
** Sort the list of pages in accending order by pgno.  Pages are
** connected by pDirty pointers.  The pDirtyPrev pointers are
** corrupted by this sort.
**
** Since there cannot be more than 2^31 distinct pages in a database,
** there cannot be more than 31 buckets required by the merge sorter.
** One extra bucket is added to catch overflow in case something
** ever changes to make the previous sentence incorrect.
*/
#define N_SORT_BUCKET  32
static PgHdr *pcacheSortDirtyList(PgHdr *pIn){
  PgHdr *a[N_SORT_BUCKET], *p;
  int i;
  memset(a, 0, sizeof(a));
  while( pIn ){
    p = pIn;
    pIn = p->pDirty;
    p->pDirty = 0;
    for(i=0; ALWAYS(i<N_SORT_BUCKET-1); i++){
      if( a[i]==0 ){
        a[i] = p;
        break;
      }else{
        p = pcacheMergeDirtyList(a[i], p);
        a[i] = 0;
      }
    }
    if( NEVER(i==N_SORT_BUCKET-1) ){
      /* To get here, there need to be 2^(N_SORT_BUCKET) elements in
      ** the input list.  But that is impossible.
      */
      a[i] = pcacheMergeDirtyList(a[i], p);
    }
  }
  p = a[0];
  for(i=1; i<N_SORT_BUCKET; i++){
    p = pcacheMergeDirtyList(p, a[i]);
  }
  return p;
}

/*
** Return a list of all dirty pages in the cache, sorted by page number.
*/
SQLITE_PRIVATE PgHdr *sqlite3PcacheDirtyList(PCache *pCache){
  PgHdr *p;
  for(p=pCache->pDirty; p; p=p->pDirtyNext){
    p->pDirty = p->pDirtyNext;
  }
  return pcacheSortDirtyList(pCache->pDirty);
}

/* 
** Return the total number of references to all pages held by the cache.
**
** This is not the total number of pages referenced, but the sum of the
** reference count for all pages.
*/
SQLITE_PRIVATE int sqlite3PcacheRefCount(PCache *pCache){
  return pCache->nRefSum;
}

/*
** Return the number of references to the page supplied as an argument.
*/
SQLITE_PRIVATE int sqlite3PcachePageRefcount(PgHdr *p){
  return p->nRef;
}

/* 
** Return the total number of pages in the cache.
*/
SQLITE_PRIVATE int sqlite3PcachePagecount(PCache *pCache){
  assert( pCache->pCache!=0 );
  return sqlite3GlobalConfig.pcache2.xPagecount(pCache->pCache);
}

#ifdef SQLITE_TEST
/*
** Get the suggested cache-size value.
*/
SQLITE_PRIVATE int sqlite3PcacheGetCachesize(PCache *pCache){
  return numberOfCachePages(pCache);
}
#endif

/*
** Set the suggested cache-size value.
*/
SQLITE_PRIVATE void sqlite3PcacheSetCachesize(PCache *pCache, int mxPage){
  assert( pCache->pCache!=0 );
  pCache->szCache = mxPage;
  sqlite3GlobalConfig.pcache2.xCachesize(pCache->pCache,
                                         numberOfCachePages(pCache));
}

/*
** Set the suggested cache-spill value.  Make no changes if if the
** argument is zero.  Return the effective cache-spill size, which will
** be the larger of the szSpill and szCache.
*/
SQLITE_PRIVATE int sqlite3PcacheSetSpillsize(PCache *p, int mxPage){
  int res;
  assert( p->pCache!=0 );
  if( mxPage ){
    if( mxPage<0 ){
      mxPage = (int)((-1024*(i64)mxPage)/(p->szPage+p->szExtra));
    }
    p->szSpill = mxPage;
  }
  res = numberOfCachePages(p);
  if( res<p->szSpill ) res = p->szSpill; 
  return res;
}

/*
** Free up as much memory as possible from the page cache.
*/
SQLITE_PRIVATE void sqlite3PcacheShrink(PCache *pCache){
  assert( pCache->pCache!=0 );
  sqlite3GlobalConfig.pcache2.xShrink(pCache->pCache);
}

/*
** Return the size of the header added by this middleware layer
** in the page-cache hierarchy.
*/
SQLITE_PRIVATE int sqlite3HeaderSizePcache(void){ return ROUND8(sizeof(PgHdr)); }

/*
** Return the number of dirty pages currently in the cache, as a percentage
** of the configured cache size.
*/
SQLITE_PRIVATE int sqlite3PCachePercentDirty(PCache *pCache){
  PgHdr *pDirty;
  int nDirty = 0;
  int nCache = numberOfCachePages(pCache);
  for(pDirty=pCache->pDirty; pDirty; pDirty=pDirty->pDirtyNext) nDirty++;
  return nCache ? (int)(((i64)nDirty * 100) / nCache) : 0;
}

#if defined(SQLITE_CHECK_PAGES) || defined(SQLITE_DEBUG)
/*
** For all dirty pages currently in the cache, invoke the specified
** callback. This is only used if the SQLITE_CHECK_PAGES macro is
** defined.
*/
SQLITE_PRIVATE void sqlite3PcacheIterateDirty(PCache *pCache, void (*xIter)(PgHdr *)){
  PgHdr *pDirty;
  for(pDirty=pCache->pDirty; pDirty; pDirty=pDirty->pDirtyNext){
    xIter(pDirty);
  }
}
#endif

/************** End of pcache.c **********************************************/
/************** Begin file pcache1.c *****************************************/
/*
** 2008 November 05
**
** The author disclaims copyright to this source code.  In place of
** a legal notice, here is a blessing:
**
**    May you do good and not evil.
**    May you find forgiveness for yourself and forgive others.
**    May you share freely, never taking more than you give.
**
*************************************************************************
**
** This file implements the default page cache implementation (the
** sqlite3_pcache interface). It also contains part of the implementation
** of the SQLITE_CONFIG_PAGECACHE and sqlite3_release_memory() features.
** If the default page cache implementation is overridden, then neither of
** these two features are available.
**
** A Page cache line looks like this:
**
**  -------------------------------------------------------------
**  |  database page content   |  PgHdr1  |  MemPage  |  PgHdr  |
**  -------------------------------------------------------------
**
** The database page content is up front (so that buffer overreads tend to
** flow harmlessly into the PgHdr1, MemPage, and PgHdr extensions).   MemPage
** is the extension added by the btree.c module containing information such
** as the database page number and how that database page is used.  PgHdr
** is added by the pcache.c layer and contains information used to keep track
** of which pages are "dirty".  PgHdr1 is an extension added by this
** module (pcache1.c).  The PgHdr1 header is a subclass of sqlite3_pcache_page.
** PgHdr1 contains information needed to look up a page by its page number.
** The superclass sqlite3_pcache_page.pBuf points to the start of the
** database page content and sqlite3_pcache_page.pExtra points to PgHdr.
**
** The size of the extension (MemPage+PgHdr+PgHdr1) can be determined at
** runtime using sqlite3_config(SQLITE_CONFIG_PCACHE_HDRSZ, &size).  The
** sizes of the extensions sum to 272 bytes on x64 for 3.8.10, but this
** size can vary according to architecture, compile-time options, and
** SQLite library version number.
**
** If SQLITE_PCACHE_SEPARATE_HEADER is defined, then the extension is obtained
** using a separate memory allocation from the database page content.  This
** seeks to overcome the "clownshoe" problem (also called "internal
** fragmentation" in academic literature) of allocating a few bytes more
** than a power of two with the memory allocator rounding up to the next
** power of two, and leaving the rounded-up space unused.
**
** This module tracks pointers to PgHdr1 objects.  Only pcache.c communicates
** with this module.  Information is passed back and forth as PgHdr1 pointers.
**
** The pcache.c and pager.c modules deal pointers to PgHdr objects.
** The btree.c module deals with pointers to MemPage objects.
**
** SOURCE OF PAGE CACHE MEMORY:
**
** Memory for a page might come from any of three sources:
**
**    (1)  The general-purpose memory allocator - sqlite3Malloc()
**    (2)  Global page-cache memory provided using sqlite3_config() with
**         SQLITE_CONFIG_PAGECACHE.
**    (3)  PCache-local bulk allocation.
**
** The third case is a chunk of heap memory (defaulting to 100 pages worth)
** that is allocated when the page cache is created.  The size of the local
** bulk allocation can be adjusted using 
**
**     sqlite3_config(SQLITE_CONFIG_PAGECACHE, (void*)0, 0, N).
**
** If N is positive, then N pages worth of memory are allocated using a single
** sqlite3Malloc() call and that memory is used for the first N pages allocated.
** Or if N is negative, then -1024*N bytes of memory are allocated and used
** for as many pages as can be accomodated.
**
** Only one of (2) or (3) can be used.  Once the memory available to (2) or
** (3) is exhausted, subsequent allocations fail over to the general-purpose
** memory allocator (1).
**
** Earlier versions of SQLite used only methods (1) and (2).  But experiments
** show that method (3) with N==100 provides about a 5% performance boost for
** common workloads.
*/
/* #include "sqliteInt.h" */

typedef struct PCache1 PCache1;
typedef struct PgHdr1 PgHdr1;
typedef struct PgFreeslot PgFreeslot;
typedef struct PGroup PGroup;

/*
** Each cache entry is represented by an instance of the following 
** structure. Unless SQLITE_PCACHE_SEPARATE_HEADER is defined, a buffer of
** PgHdr1.pCache->szPage bytes is allocated directly before this structure 
** in memory.
*/
struct PgHdr1 {
  sqlite3_pcache_page page;      /* Base class. Must be first. pBuf & pExtra */
  unsigned int iKey;             /* Key value (page number) */
  u8 isPinned;                   /* Page in use, not on the LRU list */
  u8 isBulkLocal;                /* This page from bulk local storage */
  u8 isAnchor;                   /* This is the PGroup.lru element */
  PgHdr1 *pNext;                 /* Next in hash table chain */
  PCache1 *pCache;               /* Cache that currently owns this page */
  PgHdr1 *pLruNext;              /* Next in LRU list of unpinned pages */
  PgHdr1 *pLruPrev;              /* Previous in LRU list of unpinned pages */
};

/* Each page cache (or PCache) belongs to a PGroup.  A PGroup is a set 
** of one or more PCaches that are able to recycle each other's unpinned
** pages when they are under memory pressure.  A PGroup is an instance of
** the following object.
**
** This page cache implementation works in one of two modes:
**
**   (1)  Every PCache is the sole member of its own PGroup.  There is
**        one PGroup per PCache.
**
**   (2)  There is a single global PGroup that all PCaches are a member
**        of.
**
** Mode 1 uses more memory (since PCache instances are not able to rob
** unused pages from other PCaches) but it also operates without a mutex,
** and is therefore often faster.  Mode 2 requires a mutex in order to be
** threadsafe, but recycles pages more efficiently.
**
** For mode (1), PGroup.mutex is NULL.  For mode (2) there is only a single
** PGroup which is the pcache1.grp global variable and its mutex is
** SQLITE_MUTEX_STATIC_LRU.
*/
struct PGroup {
  sqlite3_mutex *mutex;          /* MUTEX_STATIC_LRU or NULL */
  unsigned int nMaxPage;         /* Sum of nMax for purgeable caches */
  unsigned int nMinPage;         /* Sum of nMin for purgeable caches */
  unsigned int mxPinned;         /* nMaxpage + 10 - nMinPage */
  unsigned int nCurrentPage;     /* Number of purgeable pages allocated */
  PgHdr1 lru;                    /* The beginning and end of the LRU list */
};

/* Each page cache is an instance of the following object.  Every
** open database file (including each in-memory database and each
** temporary or transient database) has a single page cache which
** is an instance of this object.
**
** Pointers to structures of this type are cast and returned as 
** opaque sqlite3_pcache* handles.
*/
struct PCache1 {
  /* Cache configuration parameters. Page size (szPage) and the purgeable
  ** flag (bPurgeable) are set when the cache is created. nMax may be 
  ** modified at any time by a call to the pcache1Cachesize() method.
  ** The PGroup mutex must be held when accessing nMax.
  */
  PGroup *pGroup;                     /* PGroup this cache belongs to */
  int szPage;                         /* Size of database content section */
  int szExtra;                        /* sizeof(MemPage)+sizeof(PgHdr) */
  int szAlloc;                        /* Total size of one pcache line */
  int bPurgeable;                     /* True if cache is purgeable */
  unsigned int nMin;                  /* Minimum number of pages reserved */
  unsigned int nMax;                  /* Configured "cache_size" value */
  unsigned int n90pct;                /* nMax*9/10 */
  unsigned int iMaxKey;               /* Largest key seen since xTruncate() */

  /* Hash table of all pages. The following variables may only be accessed
  ** when the accessor is holding the PGroup mutex.
  */
  unsigned int nRecyclable;           /* Number of pages in the LRU list */
  unsigned int nPage;                 /* Total number of pages in apHash */
  unsigned int nHash;                 /* Number of slots in apHash[] */
  PgHdr1 **apHash;                    /* Hash table for fast lookup by key */
  PgHdr1 *pFree;                      /* List of unused pcache-local pages */
  void *pBulk;                        /* Bulk memory used by pcache-local */
};

/*
** Free slots in the allocator used to divide up the global page cache
** buffer provided using the SQLITE_CONFIG_PAGECACHE mechanism.
*/
struct PgFreeslot {
  PgFreeslot *pNext;  /* Next free slot */
};

/*
** Global data used by this cache.
*/
static SQLITE_WSD struct PCacheGlobal {
  PGroup grp;                    /* The global PGroup for mode (2) */

  /* Variables related to SQLITE_CONFIG_PAGECACHE settings.  The
  ** szSlot, nSlot, pStart, pEnd, nReserve, and isInit values are all
  ** fixed at sqlite3_initialize() time and do not require mutex protection.
  ** The nFreeSlot and pFree values do require mutex protection.
  */
  int isInit;                    /* True if initialized */
  int separateCache;             /* Use a new PGroup for each PCache */
  int nInitPage;                 /* Initial bulk allocation size */   
  int szSlot;                    /* Size of each free slot */
  int nSlot;                     /* The number of pcache slots */
  int nReserve;                  /* Try to keep nFreeSlot above this */
  void *pStart, *pEnd;           /* Bounds of global page cache memory */
  /* Above requires no mutex.  Use mutex below for variable that follow. */
  sqlite3_mutex *mutex;          /* Mutex for accessing the following: */
  PgFreeslot *pFree;             /* Free page blocks */
  int nFreeSlot;                 /* Number of unused pcache slots */
  /* The following value requires a mutex to change.  We skip the mutex on
  ** reading because (1) most platforms read a 32-bit integer atomically and
  ** (2) even if an incorrect value is read, no great harm is done since this
  ** is really just an optimization. */
  int bUnderPressure;            /* True if low on PAGECACHE memory */
} pcache1_g;

/*
** All code in this file should access the global structure above via the
** alias "pcache1". This ensures that the WSD emulation is used when
** compiling for systems that do not support real WSD.
*/
#define pcache1 (GLOBAL(struct PCacheGlobal, pcache1_g))

/*
** Macros to enter and leave the PCache LRU mutex.
*/
#if !defined(SQLITE_ENABLE_MEMORY_MANAGEMENT) || SQLITE_THREADSAFE==0
# define pcache1EnterMutex(X)  assert((X)->mutex==0)
# define pcache1LeaveMutex(X)  assert((X)->mutex==0)
# define PCACHE1_MIGHT_USE_GROUP_MUTEX 0
#else
# define pcache1EnterMutex(X) sqlite3_mutex_enter((X)->mutex)
# define pcache1LeaveMutex(X) sqlite3_mutex_leave((X)->mutex)
# define PCACHE1_MIGHT_USE_GROUP_MUTEX 1
#endif

/******************************************************************************/
/******** Page Allocation/SQLITE_CONFIG_PCACHE Related Functions **************/


/*
** This function is called during initialization if a static buffer is 
** supplied to use for the page-cache by passing the SQLITE_CONFIG_PAGECACHE
** verb to sqlite3_config(). Parameter pBuf points to an allocation large
** enough to contain 'n' buffers of 'sz' bytes each.
**
** This routine is called from sqlite3_initialize() and so it is guaranteed
** to be serialized already.  There is no need for further mutexing.
*/
SQLITE_PRIVATE void sqlite3PCacheBufferSetup(void *pBuf, int sz, int n){
  if( pcache1.isInit ){
    PgFreeslot *p;
    if( pBuf==0 ) sz = n = 0;
    sz = ROUNDDOWN8(sz);
    pcache1.szSlot = sz;
    pcache1.nSlot = pcache1.nFreeSlot = n;
    pcache1.nReserve = n>90 ? 10 : (n/10 + 1);
    pcache1.pStart = pBuf;
    pcache1.pFree = 0;
    pcache1.bUnderPressure = 0;
    while( n-- ){
      p = (PgFreeslot*)pBuf;
      p->pNext = pcache1.pFree;
      pcache1.pFree = p;
      pBuf = (void*)&((char*)pBuf)[sz];
    }
    pcache1.pEnd = pBuf;
  }
}

/*
** Try to initialize the pCache->pFree and pCache->pBulk fields.  Return
** true if pCache->pFree ends up containing one or more free pages.
*/
static int pcache1InitBulk(PCache1 *pCache){
  i64 szBulk;
  char *zBulk;
  if( pcache1.nInitPage==0 ) return 0;
  /* Do not bother with a bulk allocation if the cache size very small */
  if( pCache->nMax<3 ) return 0;
  sqlite3BeginBenignMalloc();
  if( pcache1.nInitPage>0 ){
    szBulk = pCache->szAlloc * (i64)pcache1.nInitPage;
  }else{
    szBulk = -1024 * (i64)pcache1.nInitPage;
  }
  if( szBulk > pCache->szAlloc*(i64)pCache->nMax ){
    szBulk = pCache->szAlloc*pCache->nMax;
  }
  zBulk = pCache->pBulk = sqlite3Malloc( szBulk );
  sqlite3EndBenignMalloc();
  if( zBulk ){
    int nBulk = sqlite3MallocSize(zBulk)/pCache->szAlloc;
    int i;
    for(i=0; i<nBulk; i++){
      PgHdr1 *pX = (PgHdr1*)&zBulk[pCache->szPage];
      pX->page.pBuf = zBulk;
      pX->page.pExtra = &pX[1];
      pX->isBulkLocal = 1;
      pX->isAnchor = 0;
      pX->pNext = pCache->pFree;
      pCache->pFree = pX;
      zBulk += pCache->szAlloc;
    }
  }
  return pCache->pFree!=0;
}

/*
** Malloc function used within this file to allocate space from the buffer
** configured using sqlite3_config(SQLITE_CONFIG_PAGECACHE) option. If no 
** such buffer exists or there is no space left in it, this function falls 
** back to sqlite3Malloc().
**
** Multiple threads can run this routine at the same time.  Global variables
** in pcache1 need to be protected via mutex.
*/
static void *pcache1Alloc(int nByte){
  void *p = 0;
  assert( sqlite3_mutex_notheld(pcache1.grp.mutex) );
  if( nByte<=pcache1.szSlot ){
    sqlite3_mutex_enter(pcache1.mutex);
    p = (PgHdr1 *)pcache1.pFree;
    if( p ){
      pcache1.pFree = pcache1.pFree->pNext;
      pcache1.nFreeSlot--;
      pcache1.bUnderPressure = pcache1.nFreeSlot<pcache1.nReserve;
      assert( pcache1.nFreeSlot>=0 );
      sqlite3StatusHighwater(SQLITE_STATUS_PAGECACHE_SIZE, nByte);
      sqlite3StatusUp(SQLITE_STATUS_PAGECACHE_USED, 1);
    }
    sqlite3_mutex_leave(pcache1.mutex);
  }
  if( p==0 ){
    /* Memory is not available in the SQLITE_CONFIG_PAGECACHE pool.  Get
    ** it from sqlite3Malloc instead.
    */
    p = sqlite3Malloc(nByte);
#ifndef SQLITE_DISABLE_PAGECACHE_OVERFLOW_STATS
    if( p ){
      int sz = sqlite3MallocSize(p);
      sqlite3_mutex_enter(pcache1.mutex);
      sqlite3StatusHighwater(SQLITE_STATUS_PAGECACHE_SIZE, nByte);
      sqlite3StatusUp(SQLITE_STATUS_PAGECACHE_OVERFLOW, sz);
      sqlite3_mutex_leave(pcache1.mutex);
    }
#endif
    sqlite3MemdebugSetType(p, MEMTYPE_PCACHE);
  }
  return p;
}

/*
** Free an allocated buffer obtained from pcache1Alloc().
*/
static void pcache1Free(void *p){
  if( p==0 ) return;
  if( SQLITE_WITHIN(p, pcache1.pStart, pcache1.pEnd) ){
    PgFreeslot *pSlot;
    sqlite3_mutex_enter(pcache1.mutex);
    sqlite3StatusDown(SQLITE_STATUS_PAGECACHE_USED, 1);
    pSlot = (PgFreeslot*)p;
    pSlot->pNext = pcache1.pFree;
    pcache1.pFree = pSlot;
    pcache1.nFreeSlot++;
    pcache1.bUnderPressure = pcache1.nFreeSlot<pcache1.nReserve;
    assert( pcache1.nFreeSlot<=pcache1.nSlot );
    sqlite3_mutex_leave(pcache1.mutex);
  }else{
    assert( sqlite3MemdebugHasType(p, MEMTYPE_PCACHE) );
    sqlite3MemdebugSetType(p, MEMTYPE_HEAP);
#ifndef SQLITE_DISABLE_PAGECACHE_OVERFLOW_STATS
    {
      int nFreed = 0;
      nFreed = sqlite3MallocSize(p);
      sqlite3_mutex_enter(pcache1.mutex);
      sqlite3StatusDown(SQLITE_STATUS_PAGECACHE_OVERFLOW, nFreed);
      sqlite3_mutex_leave(pcache1.mutex);
    }
#endif
    sqlite3_free(p);
  }
}

#ifdef SQLITE_ENABLE_MEMORY_MANAGEMENT
/*
** Return the size of a pcache allocation
*/
static int pcache1MemSize(void *p){
  if( p>=pcache1.pStart && p<pcache1.pEnd ){
    return pcache1.szSlot;
  }else{
    int iSize;
    assert( sqlite3MemdebugHasType(p, MEMTYPE_PCACHE) );
    sqlite3MemdebugSetType(p, MEMTYPE_HEAP);
    iSize = sqlite3MallocSize(p);
    sqlite3MemdebugSetType(p, MEMTYPE_PCACHE);
    return iSize;
  }
}
#endif /* SQLITE_ENABLE_MEMORY_MANAGEMENT */

/*
** Allocate a new page object initially associated with cache pCache.
*/
static PgHdr1 *pcache1AllocPage(PCache1 *pCache, int benignMalloc){
  PgHdr1 *p = 0;
  void *pPg;

  assert( sqlite3_mutex_held(pCache->pGroup->mutex) );
  if( pCache->pFree || (pCache->nPage==0 && pcache1InitBulk(pCache)) ){
    p = pCache->pFree;
    pCache->pFree = p->pNext;
    p->pNext = 0;
  }else{
#ifdef SQLITE_ENABLE_MEMORY_MANAGEMENT
    /* The group mutex must be released before pcache1Alloc() is called. This
    ** is because it might call sqlite3_release_memory(), which assumes that 
    ** this mutex is not held. */
    assert( pcache1.separateCache==0 );
    assert( pCache->pGroup==&pcache1.grp );
    pcache1LeaveMutex(pCache->pGroup);
#endif
    if( benignMalloc ){ sqlite3BeginBenignMalloc(); }
#ifdef SQLITE_PCACHE_SEPARATE_HEADER
    pPg = pcache1Alloc(pCache->szPage);
    p = sqlite3Malloc(sizeof(PgHdr1) + pCache->szExtra);
    if( !pPg || !p ){
      pcache1Free(pPg);
      sqlite3_free(p);
      pPg = 0;
    }
#else
    pPg = pcache1Alloc(pCache->szAlloc);
    p = (PgHdr1 *)&((u8 *)pPg)[pCache->szPage];
#endif
    if( benignMalloc ){ sqlite3EndBenignMalloc(); }
#ifdef SQLITE_ENABLE_MEMORY_MANAGEMENT
    pcache1EnterMutex(pCache->pGroup);
#endif
    if( pPg==0 ) return 0;
    p->page.pBuf = pPg;
    p->page.pExtra = &p[1];
    p->isBulkLocal = 0;
    p->isAnchor = 0;
  }
  if( pCache->bPurgeable ){
    pCache->pGroup->nCurrentPage++;
  }
  return p;
}

/*
** Free a page object allocated by pcache1AllocPage().
*/
static void pcache1FreePage(PgHdr1 *p){
  PCache1 *pCache;
  assert( p!=0 );
  pCache = p->pCache;
  assert( sqlite3_mutex_held(p->pCache->pGroup->mutex) );
  if( p->isBulkLocal ){
    p->pNext = pCache->pFree;
    pCache->pFree = p;
  }else{
    pcache1Free(p->page.pBuf);
#ifdef SQLITE_PCACHE_SEPARATE_HEADER
    sqlite3_free(p);
#endif
  }
  if( pCache->bPurgeable ){
    pCache->pGroup->nCurrentPage--;
  }
}

/*
** Malloc function used by SQLite to obtain space from the buffer configured
** using sqlite3_config(SQLITE_CONFIG_PAGECACHE) option. If no such buffer
** exists, this function falls back to sqlite3Malloc().
*/
SQLITE_PRIVATE void *sqlite3PageMalloc(int sz){
  return pcache1Alloc(sz);
}

/*
** Free an allocated buffer obtained from sqlite3PageMalloc().
*/
SQLITE_PRIVATE void sqlite3PageFree(void *p){
  pcache1Free(p);
}


/*
** Return true if it desirable to avoid allocating a new page cache
** entry.
**
** If memory was allocated specifically to the page cache using
** SQLITE_CONFIG_PAGECACHE but that memory has all been used, then
** it is desirable to avoid allocating a new page cache entry because
** presumably SQLITE_CONFIG_PAGECACHE was suppose to be sufficient
** for all page cache needs and we should not need to spill the
** allocation onto the heap.
**
** Or, the heap is used for all page cache memory but the heap is
** under memory pressure, then again it is desirable to avoid
** allocating a new page cache entry in order to avoid stressing
** the heap even further.
*/
static int pcache1UnderMemoryPressure(PCache1 *pCache){
  if( pcache1.nSlot && (pCache->szPage+pCache->szExtra)<=pcache1.szSlot ){
    return pcache1.bUnderPressure;
  }else{
    return sqlite3HeapNearlyFull();
  }
}

/******************************************************************************/
/******** General Implementation Functions ************************************/

/*
** This function is used to resize the hash table used by the cache passed
** as the first argument.
**
** The PCache mutex must be held when this function is called.
*/
static void pcache1ResizeHash(PCache1 *p){
  PgHdr1 **apNew;
  unsigned int nNew;
  unsigned int i;

  assert( sqlite3_mutex_held(p->pGroup->mutex) );

  nNew = p->nHash*2;
  if( nNew<256 ){
    nNew = 256;
  }

  pcache1LeaveMutex(p->pGroup);
  if( p->nHash ){ sqlite3BeginBenignMalloc(); }
  apNew = (PgHdr1 **)sqlite3MallocZero(sizeof(PgHdr1 *)*nNew);
  if( p->nHash ){ sqlite3EndBenignMalloc(); }
  pcache1EnterMutex(p->pGroup);
  if( apNew ){
    for(i=0; i<p->nHash; i++){
      PgHdr1 *pPage;
      PgHdr1 *pNext = p->apHash[i];
      while( (pPage = pNext)!=0 ){
        unsigned int h = pPage->iKey % nNew;
        pNext = pPage->pNext;
        pPage->pNext = apNew[h];
        apNew[h] = pPage;
      }
    }
    sqlite3_free(p->apHash);
    p->apHash = apNew;
    p->nHash = nNew;
  }
}

/*
** This function is used internally to remove the page pPage from the 
** PGroup LRU list, if is part of it. If pPage is not part of the PGroup
** LRU list, then this function is a no-op.
**
** The PGroup mutex must be held when this function is called.
*/
static PgHdr1 *pcache1PinPage(PgHdr1 *pPage){
  PCache1 *pCache;

  assert( pPage!=0 );
  assert( pPage->isPinned==0 );
  pCache = pPage->pCache;
  assert( pPage->pLruNext );
  assert( pPage->pLruPrev );
  assert( sqlite3_mutex_held(pCache->pGroup->mutex) );
  pPage->pLruPrev->pLruNext = pPage->pLruNext;
  pPage->pLruNext->pLruPrev = pPage->pLruPrev;
  pPage->pLruNext = 0;
  pPage->pLruPrev = 0;
  pPage->isPinned = 1;
  assert( pPage->isAnchor==0 );
  assert( pCache->pGroup->lru.isAnchor==1 );
  pCache->nRecyclable--;
  return pPage;
}


/*
** Remove the page supplied as an argument from the hash table 
** (PCache1.apHash structure) that it is currently stored in.
** Also free the page if freePage is true.
**
** The PGroup mutex must be held when this function is called.
*/
static void pcache1RemoveFromHash(PgHdr1 *pPage, int freeFlag){
  unsigned int h;
  PCache1 *pCache = pPage->pCache;
  PgHdr1 **pp;

  assert( sqlite3_mutex_held(pCache->pGroup->mutex) );
  h = pPage->iKey % pCache->nHash;
  for(pp=&pCache->apHash[h]; (*pp)!=pPage; pp=&(*pp)->pNext);
  *pp = (*pp)->pNext;

  pCache->nPage--;
  if( freeFlag ) pcache1FreePage(pPage);
}

/*
** If there are currently more than nMaxPage pages allocated, try
** to recycle pages to reduce the number allocated to nMaxPage.
*/
static void pcache1EnforceMaxPage(PCache1 *pCache){
  PGroup *pGroup = pCache->pGroup;
  PgHdr1 *p;
  assert( sqlite3_mutex_held(pGroup->mutex) );
  while( pGroup->nCurrentPage>pGroup->nMaxPage
      && (p=pGroup->lru.pLruPrev)->isAnchor==0
  ){
    assert( p->pCache->pGroup==pGroup );
    assert( p->isPinned==0 );
    pcache1PinPage(p);
    pcache1RemoveFromHash(p, 1);
  }
  if( pCache->nPage==0 && pCache->pBulk ){
    sqlite3_free(pCache->pBulk);
    pCache->pBulk = pCache->pFree = 0;
  }
}

/*
** Discard all pages from cache pCache with a page number (key value) 
** greater than or equal to iLimit. Any pinned pages that meet this 
** criteria are unpinned before they are discarded.
**
** The PCache mutex must be held when this function is called.
*/
static void pcache1TruncateUnsafe(
  PCache1 *pCache,             /* The cache to truncate */
  unsigned int iLimit          /* Drop pages with this pgno or larger */
){
  TESTONLY( unsigned int nPage = 0; )  /* To assert pCache->nPage is correct */
  unsigned int h;
  assert( sqlite3_mutex_held(pCache->pGroup->mutex) );
  for(h=0; h<pCache->nHash; h++){
    PgHdr1 **pp = &pCache->apHash[h]; 
    PgHdr1 *pPage;
    while( (pPage = *pp)!=0 ){
      if( pPage->iKey>=iLimit ){
        pCache->nPage--;
        *pp = pPage->pNext;
        if( !pPage->isPinned ) pcache1PinPage(pPage);
        pcache1FreePage(pPage);
      }else{
        pp = &pPage->pNext;
        TESTONLY( nPage++; )
      }
    }
  }
  assert( pCache->nPage==nPage );
}

/******************************************************************************/
/******** sqlite3_pcache Methods **********************************************/

/*
** Implementation of the sqlite3_pcache.xInit method.
*/
static int pcache1Init(void *NotUsed){
  UNUSED_PARAMETER(NotUsed);
  assert( pcache1.isInit==0 );
  memset(&pcache1, 0, sizeof(pcache1));


  /*
  ** The pcache1.separateCache variable is true if each PCache has its own
  ** private PGroup (mode-1).  pcache1.separateCache is false if the single
  ** PGroup in pcache1.grp is used for all page caches (mode-2).
  **
  **   *  Always use a unified cache (mode-2) if ENABLE_MEMORY_MANAGEMENT
  **
  **   *  Use a unified cache in single-threaded applications that have
  **      configured a start-time buffer for use as page-cache memory using
  **      sqlite3_config(SQLITE_CONFIG_PAGECACHE, pBuf, sz, N) with non-NULL 
  **      pBuf argument.
  **
  **   *  Otherwise use separate caches (mode-1)
  */
#if defined(SQLITE_ENABLE_MEMORY_MANAGEMENT)
  pcache1.separateCache = 0;
#elif SQLITE_THREADSAFE
  pcache1.separateCache = sqlite3GlobalConfig.pPage==0
                          || sqlite3GlobalConfig.bCoreMutex>0;
#else
  pcache1.separateCache = sqlite3GlobalConfig.pPage==0;
#endif

#if SQLITE_THREADSAFE
  if( sqlite3GlobalConfig.bCoreMutex ){
    pcache1.grp.mutex = sqlite3MutexAlloc(SQLITE_MUTEX_STATIC_LRU);
    pcache1.mutex = sqlite3MutexAlloc(SQLITE_MUTEX_STATIC_PMEM);
  }
#endif
  if( pcache1.separateCache
   && sqlite3GlobalConfig.nPage!=0
   && sqlite3GlobalConfig.pPage==0
  ){
    pcache1.nInitPage = sqlite3GlobalConfig.nPage;
  }else{
    pcache1.nInitPage = 0;
  }
  pcache1.grp.mxPinned = 10;
  pcache1.isInit = 1;
  return SQLITE_OK;
}

/*
** Implementation of the sqlite3_pcache.xShutdown method.
** Note that the static mutex allocated in xInit does 
** not need to be freed.
*/
static void pcache1Shutdown(void *NotUsed){
  UNUSED_PARAMETER(NotUsed);
  assert( pcache1.isInit!=0 );
  memset(&pcache1, 0, sizeof(pcache1));
}

/* forward declaration */
static void pcache1Destroy(sqlite3_pcache *p);

/*
** Implementation of the sqlite3_pcache.xCreate method.
**
** Allocate a new cache.
*/
static sqlite3_pcache *pcache1Create(int szPage, int szExtra, int bPurgeable){
  PCache1 *pCache;      /* The newly created page cache */
  PGroup *pGroup;       /* The group the new page cache will belong to */
  int sz;               /* Bytes of memory required to allocate the new cache */

  assert( (szPage & (szPage-1))==0 && szPage>=512 && szPage<=65536 );
  assert( szExtra < 300 );

  sz = sizeof(PCache1) + sizeof(PGroup)*pcache1.separateCache;
  pCache = (PCache1 *)sqlite3MallocZero(sz);
  if( pCache ){
    if( pcache1.separateCache ){
      pGroup = (PGroup*)&pCache[1];
      pGroup->mxPinned = 10;
    }else{
      pGroup = &pcache1.grp;
    }
    if( pGroup->lru.isAnchor==0 ){
      pGroup->lru.isAnchor = 1;
      pGroup->lru.pLruPrev = pGroup->lru.pLruNext = &pGroup->lru;
    }
    pCache->pGroup = pGroup;
    pCache->szPage = szPage;
    pCache->szExtra = szExtra;
    pCache->szAlloc = szPage + szExtra + ROUND8(sizeof(PgHdr1));
    pCache->bPurgeable = (bPurgeable ? 1 : 0);
    pcache1EnterMutex(pGroup);
    pcache1ResizeHash(pCache);
    if( bPurgeable ){
      pCache->nMin = 10;
      pGroup->nMinPage += pCache->nMin;
      pGroup->mxPinned = pGroup->nMaxPage + 10 - pGroup->nMinPage;
    }
    pcache1LeaveMutex(pGroup);
    if( pCache->nHash==0 ){
      pcache1Destroy((sqlite3_pcache*)pCache);
      pCache = 0;
    }
  }
  return (sqlite3_pcache *)pCache;
}

/*
** Implementation of the sqlite3_pcache.xCachesize method. 
**
** Configure the cache_size limit for a cache.
*/
static void pcache1Cachesize(sqlite3_pcache *p, int nMax){
  PCache1 *pCache = (PCache1 *)p;
  if( pCache->bPurgeable ){
    PGroup *pGroup = pCache->pGroup;
    pcache1EnterMutex(pGroup);
    pGroup->nMaxPage += (nMax - pCache->nMax);
    pGroup->mxPinned = pGroup->nMaxPage + 10 - pGroup->nMinPage;
    pCache->nMax = nMax;
    pCache->n90pct = pCache->nMax*9/10;
    pcache1EnforceMaxPage(pCache);
    pcache1LeaveMutex(pGroup);
  }
}

/*
** Implementation of the sqlite3_pcache.xShrink method. 
**
** Free up as much memory as possible.
*/
static void pcache1Shrink(sqlite3_pcache *p){
  PCache1 *pCache = (PCache1*)p;
  if( pCache->bPurgeable ){
    PGroup *pGroup = pCache->pGroup;
    int savedMaxPage;
    pcache1EnterMutex(pGroup);
    savedMaxPage = pGroup->nMaxPage;
    pGroup->nMaxPage = 0;
    pcache1EnforceMaxPage(pCache);
    pGroup->nMaxPage = savedMaxPage;
    pcache1LeaveMutex(pGroup);
  }
}

/*
** Implementation of the sqlite3_pcache.xPagecount method. 
*/
static int pcache1Pagecount(sqlite3_pcache *p){
  int n;
  PCache1 *pCache = (PCache1*)p;
  pcache1EnterMutex(pCache->pGroup);
  n = pCache->nPage;
  pcache1LeaveMutex(pCache->pGroup);
  return n;
}


/*
** Implement steps 3, 4, and 5 of the pcache1Fetch() algorithm described
** in the header of the pcache1Fetch() procedure.
**
** This steps are broken out into a separate procedure because they are
** usually not needed, and by avoiding the stack initialization required
** for these steps, the main pcache1Fetch() procedure can run faster.
*/
static SQLITE_NOINLINE PgHdr1 *pcache1FetchStage2(
  PCache1 *pCache, 
  unsigned int iKey, 
  int createFlag
){
  unsigned int nPinned;
  PGroup *pGroup = pCache->pGroup;
  PgHdr1 *pPage = 0;

  /* Step 3: Abort if createFlag is 1 but the cache is nearly full */
  assert( pCache->nPage >= pCache->nRecyclable );
  nPinned = pCache->nPage - pCache->nRecyclable;
  assert( pGroup->mxPinned == pGroup->nMaxPage + 10 - pGroup->nMinPage );
  assert( pCache->n90pct == pCache->nMax*9/10 );
  if( createFlag==1 && (
        nPinned>=pGroup->mxPinned
     || nPinned>=pCache->n90pct
     || (pcache1UnderMemoryPressure(pCache) && pCache->nRecyclable<nPinned)
  )){
    return 0;
  }

  if( pCache->nPage>=pCache->nHash ) pcache1ResizeHash(pCache);
  assert( pCache->nHash>0 && pCache->apHash );

  /* Step 4. Try to recycle a page. */
  if( pCache->bPurgeable
   && !pGroup->lru.pLruPrev->isAnchor
   && ((pCache->nPage+1>=pCache->nMax) || pcache1UnderMemoryPressure(pCache))
  ){
    PCache1 *pOther;
    pPage = pGroup->lru.pLruPrev;
    assert( pPage->isPinned==0 );
    pcache1RemoveFromHash(pPage, 0);
    pcache1PinPage(pPage);
    pOther = pPage->pCache;
    if( pOther->szAlloc != pCache->szAlloc ){
      pcache1FreePage(pPage);
      pPage = 0;
    }else{
      pGroup->nCurrentPage -= (pOther->bPurgeable - pCache->bPurgeable);
    }
  }

  /* Step 5. If a usable page buffer has still not been found, 
  ** attempt to allocate a new one. 
  */
  if( !pPage ){
    pPage = pcache1AllocPage(pCache, createFlag==1);
  }

  if( pPage ){
    unsigned int h = iKey % pCache->nHash;
    pCache->nPage++;
    pPage->iKey = iKey;
    pPage->pNext = pCache->apHash[h];
    pPage->pCache = pCache;
    pPage->pLruPrev = 0;
    pPage->pLruNext = 0;
    pPage->isPinned = 1;
    *(void **)pPage->page.pExtra = 0;
    pCache->apHash[h] = pPage;
    if( iKey>pCache->iMaxKey ){
      pCache->iMaxKey = iKey;
    }
  }
  return pPage;
}

/*
** Implementation of the sqlite3_pcache.xFetch method. 
**
** Fetch a page by key value.
**
** Whether or not a new page may be allocated by this function depends on
** the value of the createFlag argument.  0 means do not allocate a new
** page.  1 means allocate a new page if space is easily available.  2 
** means to try really hard to allocate a new page.
**
** For a non-purgeable cache (a cache used as the storage for an in-memory
** database) there is really no difference between createFlag 1 and 2.  So
** the calling function (pcache.c) will never have a createFlag of 1 on
** a non-purgeable cache.
**
** There are three different approaches to obtaining space for a page,
** depending on the value of parameter createFlag (which may be 0, 1 or 2).
**
**   1. Regardless of the value of createFlag, the cache is searched for a 
**      copy of the requested page. If one is found, it is returned.
**
**   2. If createFlag==0 and the page is not already in the cache, NULL is
**      returned.
**
**   3. If createFlag is 1, and the page is not already in the cache, then
**      return NULL (do not allocate a new page) if any of the following
**      conditions are true:
**
**       (a) the number of pages pinned by the cache is greater than
**           PCache1.nMax, or
**
**       (b) the number of pages pinned by the cache is greater than
**           the sum of nMax for all purgeable caches, less the sum of 
**           nMin for all other purgeable caches, or
**
**   4. If none of the first three conditions apply and the cache is marked
**      as purgeable, and if one of the following is true:
**
**       (a) The number of pages allocated for the cache is already 
**           PCache1.nMax, or
**
**       (b) The number of pages allocated for all purgeable caches is
**           already equal to or greater than the sum of nMax for all
**           purgeable caches,
**
**       (c) The system is under memory pressure and wants to avoid
**           unnecessary pages cache entry allocations
**
**      then attempt to recycle a page from the LRU list. If it is the right
**      size, return the recycled buffer. Otherwise, free the buffer and
**      proceed to step 5. 
**
**   5. Otherwise, allocate and return a new page buffer.
**
** There are two versions of this routine.  pcache1FetchWithMutex() is
** the general case.  pcache1FetchNoMutex() is a faster implementation for
** the common case where pGroup->mutex is NULL.  The pcache1Fetch() wrapper
** invokes the appropriate routine.
*/
static PgHdr1 *pcache1FetchNoMutex(
  sqlite3_pcache *p, 
  unsigned int iKey, 
  int createFlag
){
  PCache1 *pCache = (PCache1 *)p;
  PgHdr1 *pPage = 0;

  /* Step 1: Search the hash table for an existing entry. */
  pPage = pCache->apHash[iKey % pCache->nHash];
  while( pPage && pPage->iKey!=iKey ){ pPage = pPage->pNext; }

  /* Step 2: If the page was found in the hash table, then return it.
  ** If the page was not in the hash table and createFlag is 0, abort.
  ** Otherwise (page not in hash and createFlag!=0) continue with
  ** subsequent steps to try to create the page. */
  if( pPage ){
    if( !pPage->isPinned ){
      return pcache1PinPage(pPage);
    }else{
      return pPage;
    }
  }else if( createFlag ){
    /* Steps 3, 4, and 5 implemented by this subroutine */
    return pcache1FetchStage2(pCache, iKey, createFlag);
  }else{
    return 0;
  }
}
#if PCACHE1_MIGHT_USE_GROUP_MUTEX
static PgHdr1 *pcache1FetchWithMutex(
  sqlite3_pcache *p, 
  unsigned int iKey, 
  int createFlag
){
  PCache1 *pCache = (PCache1 *)p;
  PgHdr1 *pPage;

  pcache1EnterMutex(pCache->pGroup);
  pPage = pcache1FetchNoMutex(p, iKey, createFlag);
  assert( pPage==0 || pCache->iMaxKey>=iKey );
  pcache1LeaveMutex(pCache->pGroup);
  return pPage;
}
#endif
static sqlite3_pcache_page *pcache1Fetch(
  sqlite3_pcache *p, 
  unsigned int iKey, 
  int createFlag
){
#if PCACHE1_MIGHT_USE_GROUP_MUTEX || defined(SQLITE_DEBUG)
  PCache1 *pCache = (PCache1 *)p;
#endif

  assert( offsetof(PgHdr1,page)==0 );
  assert( pCache->bPurgeable || createFlag!=1 );
  assert( pCache->bPurgeable || pCache->nMin==0 );
  assert( pCache->bPurgeable==0 || pCache->nMin==10 );
  assert( pCache->nMin==0 || pCache->bPurgeable );
  assert( pCache->nHash>0 );
#if PCACHE1_MIGHT_USE_GROUP_MUTEX
  if( pCache->pGroup->mutex ){
    return (sqlite3_pcache_page*)pcache1FetchWithMutex(p, iKey, createFlag);
  }else
#endif
  {
    return (sqlite3_pcache_page*)pcache1FetchNoMutex(p, iKey, createFlag);
  }
}


/*
** Implementation of the sqlite3_pcache.xUnpin method.
**
** Mark a page as unpinned (eligible for asynchronous recycling).
*/
static void pcache1Unpin(
  sqlite3_pcache *p, 
  sqlite3_pcache_page *pPg, 
  int reuseUnlikely
){
  PCache1 *pCache = (PCache1 *)p;
  PgHdr1 *pPage = (PgHdr1 *)pPg;
  PGroup *pGroup = pCache->pGroup;
 
  assert( pPage->pCache==pCache );
  pcache1EnterMutex(pGroup);

  /* It is an error to call this function if the page is already 
  ** part of the PGroup LRU list.
  */
  assert( pPage->pLruPrev==0 && pPage->pLruNext==0 );
  assert( pPage->isPinned==1 );

  if( reuseUnlikely || pGroup->nCurrentPage>pGroup->nMaxPage ){
    pcache1RemoveFromHash(pPage, 1);
  }else{
    /* Add the page to the PGroup LRU list. */
    PgHdr1 **ppFirst = &pGroup->lru.pLruNext;
    pPage->pLruPrev = &pGroup->lru;
    (pPage->pLruNext = *ppFirst)->pLruPrev = pPage;
    *ppFirst = pPage;
    pCache->nRecyclable++;
    pPage->isPinned = 0;
  }

  pcache1LeaveMutex(pCache->pGroup);
}

/*
** Implementation of the sqlite3_pcache.xRekey method. 
*/
static void pcache1Rekey(
  sqlite3_pcache *p,
  sqlite3_pcache_page *pPg,
  unsigned int iOld,
  unsigned int iNew
){
  PCache1 *pCache = (PCache1 *)p;
  PgHdr1 *pPage = (PgHdr1 *)pPg;
  PgHdr1 **pp;
  unsigned int h; 
  assert( pPage->iKey==iOld );
  assert( pPage->pCache==pCache );

  pcache1EnterMutex(pCache->pGroup);

  h = iOld%pCache->nHash;
  pp = &pCache->apHash[h];
  while( (*pp)!=pPage ){
    pp = &(*pp)->pNext;
  }
  *pp = pPage->pNext;

  h = iNew%pCache->nHash;
  pPage->iKey = iNew;
  pPage->pNext = pCache->apHash[h];
  pCache->apHash[h] = pPage;
  if( iNew>pCache->iMaxKey ){
    pCache->iMaxKey = iNew;
  }

  pcache1LeaveMutex(pCache->pGroup);
}

/*
** Implementation of the sqlite3_pcache.xTruncate method. 
**
** Discard all unpinned pages in the cache with a page number equal to
** or greater than parameter iLimit. Any pinned pages with a page number
** equal to or greater than iLimit are implicitly unpinned.
*/
static void pcache1Truncate(sqlite3_pcache *p, unsigned int iLimit){
  PCache1 *pCache = (PCache1 *)p;
  pcache1EnterMutex(pCache->pGroup);
  if( iLimit<=pCache->iMaxKey ){
    pcache1TruncateUnsafe(pCache, iLimit);
    pCache->iMaxKey = iLimit-1;
  }
  pcache1LeaveMutex(pCache->pGroup);
}

/*
** Implementation of the sqlite3_pcache.xDestroy method. 
**
** Destroy a cache allocated using pcache1Create().
*/
static void pcache1Destroy(sqlite3_pcache *p){
  PCache1 *pCache = (PCache1 *)p;
  PGroup *pGroup = pCache->pGroup;
  assert( pCache->bPurgeable || (pCache->nMax==0 && pCache->nMin==0) );
  pcache1EnterMutex(pGroup);
  pcache1TruncateUnsafe(pCache, 0);
  assert( pGroup->nMaxPage >= pCache->nMax );
  pGroup->nMaxPage -= pCache->nMax;
  assert( pGroup->nMinPage >= pCache->nMin );
  pGroup->nMinPage -= pCache->nMin;
  pGroup->mxPinned = pGroup->nMaxPage + 10 - pGroup->nMinPage;
  pcache1EnforceMaxPage(pCache);
  pcache1LeaveMutex(pGroup);
  sqlite3_free(pCache->pBulk);
  sqlite3_free(pCache->apHash);
  sqlite3_free(pCache);
}

/*
** This function is called during initialization (sqlite3_initialize()) to
** install the default pluggable cache module, assuming the user has not
** already provided an alternative.
*/
SQLITE_PRIVATE void sqlite3PCacheSetDefault(void){
  static const sqlite3_pcache_methods2 defaultMethods = {
    1,                       /* iVersion */
    0,                       /* pArg */
    pcache1Init,             /* xInit */
    pcache1Shutdown,         /* xShutdown */
    pcache1Create,           /* xCreate */
    pcache1Cachesize,        /* xCachesize */
    pcache1Pagecount,        /* xPagecount */
    pcache1Fetch,            /* xFetch */
    pcache1Unpin,            /* xUnpin */
    pcache1Rekey,            /* xRekey */
    pcache1Truncate,         /* xTruncate */
    pcache1Destroy,          /* xDestroy */
    pcache1Shrink            /* xShrink */
  };
  sqlite3_config(SQLITE_CONFIG_PCACHE2, &defaultMethods);
}

/*
** Return the size of the header on each page of this PCACHE implementation.
*/
SQLITE_PRIVATE int sqlite3HeaderSizePcache1(void){ return ROUND8(sizeof(PgHdr1)); }

/*
** Return the global mutex used by this PCACHE implementation.  The
** sqlite3_status() routine needs access to this mutex.
*/
SQLITE_PRIVATE sqlite3_mutex *sqlite3Pcache1Mutex(void){
  return pcache1.mutex;
}

#ifdef SQLITE_ENABLE_MEMORY_MANAGEMENT
/*
** This function is called to free superfluous dynamically allocated memory
** held by the pager system. Memory in use by any SQLite pager allocated
** by the current thread may be sqlite3_free()ed.
**
** nReq is the number of bytes of memory required. Once this much has
** been released, the function returns. The return value is the total number 
** of bytes of memory released.
*/
SQLITE_PRIVATE int sqlite3PcacheReleaseMemory(int nReq){
  int nFree = 0;
  assert( sqlite3_mutex_notheld(pcache1.grp.mutex) );
  assert( sqlite3_mutex_notheld(pcache1.mutex) );
  if( sqlite3GlobalConfig.nPage==0 ){
    PgHdr1 *p;
    pcache1EnterMutex(&pcache1.grp);
    while( (nReq<0 || nFree<nReq)
       &&  (p=pcache1.grp.lru.pLruPrev)!=0
       &&  p->isAnchor==0
    ){
      nFree += pcache1MemSize(p->page.pBuf);
#ifdef SQLITE_PCACHE_SEPARATE_HEADER
      nFree += sqlite3MemSize(p);
#endif
      assert( p->isPinned==0 );
      pcache1PinPage(p);
      pcache1RemoveFromHash(p, 1);
    }
    pcache1LeaveMutex(&pcache1.grp);
  }
  return nFree;
}
#endif /* SQLITE_ENABLE_MEMORY_MANAGEMENT */

#ifdef SQLITE_TEST
/*
** This function is used by test procedures to inspect the internal state
** of the global cache.
*/
SQLITE_PRIVATE void sqlite3PcacheStats(
  int *pnCurrent,      /* OUT: Total number of pages cached */
  int *pnMax,          /* OUT: Global maximum cache size */
  int *pnMin,          /* OUT: Sum of PCache1.nMin for purgeable caches */
  int *pnRecyclable    /* OUT: Total number of pages available for recycling */
){
  PgHdr1 *p;
  int nRecyclable = 0;
  for(p=pcache1.grp.lru.pLruNext; p && !p->isAnchor; p=p->pLruNext){
    assert( p->isPinned==0 );
    nRecyclable++;
  }
  *pnCurrent = pcache1.grp.nCurrentPage;
  *pnMax = (int)pcache1.grp.nMaxPage;
  *pnMin = (int)pcache1.grp.nMinPage;
  *pnRecyclable = nRecyclable;
}
#endif

/************** End of pcache1.c *********************************************/
/************** Begin file rowset.c ******************************************/
/*
** 2008 December 3
**
** The author disclaims copyright to this source code.  In place of
** a legal notice, here is a blessing:
**
**    May you do good and not evil.
**    May you find forgiveness for yourself and forgive others.
**    May you share freely, never taking more than you give.
**
*************************************************************************
**
** This module implements an object we call a "RowSet".
**
** The RowSet object is a collection of rowids.  Rowids
** are inserted into the RowSet in an arbitrary order.  Inserts
** can be intermixed with tests to see if a given rowid has been
** previously inserted into the RowSet.
**
** After all inserts are finished, it is possible to extract the
** elements of the RowSet in sorted order.  Once this extraction
** process has started, no new elements may be inserted.
**
** Hence, the primitive operations for a RowSet are:
**
**    CREATE
**    INSERT
**    TEST
**    SMALLEST
**    DESTROY
**
** The CREATE and DESTROY primitives are the constructor and destructor,
** obviously.  The INSERT primitive adds a new element to the RowSet.
** TEST checks to see if an element is already in the RowSet.  SMALLEST
** extracts the least value from the RowSet.
**
** The INSERT primitive might allocate additional memory.  Memory is
** allocated in chunks so most INSERTs do no allocation.  There is an 
** upper bound on the size of allocated memory.  No memory is freed
** until DESTROY.
**
** The TEST primitive includes a "batch" number.  The TEST primitive
** will only see elements that were inserted before the last change
** in the batch number.  In other words, if an INSERT occurs between
** two TESTs where the TESTs have the same batch nubmer, then the
** value added by the INSERT will not be visible to the second TEST.
** The initial batch number is zero, so if the very first TEST contains
** a non-zero batch number, it will see all prior INSERTs.
**
** No INSERTs may occurs after a SMALLEST.  An assertion will fail if
** that is attempted.
**
** The cost of an INSERT is roughly constant.  (Sometimes new memory
** has to be allocated on an INSERT.)  The cost of a TEST with a new
** batch number is O(NlogN) where N is the number of elements in the RowSet.
** The cost of a TEST using the same batch number is O(logN).  The cost
** of the first SMALLEST is O(NlogN).  Second and subsequent SMALLEST
** primitives are constant time.  The cost of DESTROY is O(N).
**
** TEST and SMALLEST may not be used by the same RowSet.  This used to
** be possible, but the feature was not used, so it was removed in order
** to simplify the code.
*/
/* #include "sqliteInt.h" */


/*
** Target size for allocation chunks.
*/
#define ROWSET_ALLOCATION_SIZE 1024

/*
** The number of rowset entries per allocation chunk.
*/
#define ROWSET_ENTRY_PER_CHUNK  \
                       ((ROWSET_ALLOCATION_SIZE-8)/sizeof(struct RowSetEntry))

/*
** Each entry in a RowSet is an instance of the following object.
**
** This same object is reused to store a linked list of trees of RowSetEntry
** objects.  In that alternative use, pRight points to the next entry
** in the list, pLeft points to the tree, and v is unused.  The
** RowSet.pForest value points to the head of this forest list.
*/
struct RowSetEntry {            
  i64 v;                        /* ROWID value for this entry */
  struct RowSetEntry *pRight;   /* Right subtree (larger entries) or list */
  struct RowSetEntry *pLeft;    /* Left subtree (smaller entries) */
};

/*
** RowSetEntry objects are allocated in large chunks (instances of the
** following structure) to reduce memory allocation overhead.  The
** chunks are kept on a linked list so that they can be deallocated
** when the RowSet is destroyed.
*/
struct RowSetChunk {
  struct RowSetChunk *pNextChunk;        /* Next chunk on list of them all */
  struct RowSetEntry aEntry[ROWSET_ENTRY_PER_CHUNK]; /* Allocated entries */
};

/*
** A RowSet in an instance of the following structure.
**
** A typedef of this structure if found in sqliteInt.h.
*/
struct RowSet {
  struct RowSetChunk *pChunk;    /* List of all chunk allocations */
  sqlite3 *db;                   /* The database connection */
  struct RowSetEntry *pEntry;    /* List of entries using pRight */
  struct RowSetEntry *pLast;     /* Last entry on the pEntry list */
  struct RowSetEntry *pFresh;    /* Source of new entry objects */
  struct RowSetEntry *pForest;   /* List of binary trees of entries */
  u16 nFresh;                    /* Number of objects on pFresh */
  u16 rsFlags;                   /* Various flags */
  int iBatch;                    /* Current insert batch */
};

/*
** Allowed values for RowSet.rsFlags
*/
#define ROWSET_SORTED  0x01   /* True if RowSet.pEntry is sorted */
#define ROWSET_NEXT    0x02   /* True if sqlite3RowSetNext() has been called */

/*
** Turn bulk memory into a RowSet object.  N bytes of memory
** are available at pSpace.  The db pointer is used as a memory context
** for any subsequent allocations that need to occur.
** Return a pointer to the new RowSet object.
**
** It must be the case that N is sufficient to make a Rowset.  If not
** an assertion fault occurs.
** 
** If N is larger than the minimum, use the surplus as an initial
** allocation of entries available to be filled.
*/
SQLITE_PRIVATE RowSet *sqlite3RowSetInit(sqlite3 *db, void *pSpace, unsigned int N){
  RowSet *p;
  assert( N >= ROUND8(sizeof(*p)) );
  p = pSpace;
  p->pChunk = 0;
  p->db = db;
  p->pEntry = 0;
  p->pLast = 0;
  p->pForest = 0;
  p->pFresh = (struct RowSetEntry*)(ROUND8(sizeof(*p)) + (char*)p);
  p->nFresh = (u16)((N - ROUND8(sizeof(*p)))/sizeof(struct RowSetEntry));
  p->rsFlags = ROWSET_SORTED;
  p->iBatch = 0;
  return p;
}

/*
** Deallocate all chunks from a RowSet.  This frees all memory that
** the RowSet has allocated over its lifetime.  This routine is
** the destructor for the RowSet.
*/
SQLITE_PRIVATE void sqlite3RowSetClear(RowSet *p){
  struct RowSetChunk *pChunk, *pNextChunk;
  for(pChunk=p->pChunk; pChunk; pChunk = pNextChunk){
    pNextChunk = pChunk->pNextChunk;
    sqlite3DbFree(p->db, pChunk);
  }
  p->pChunk = 0;
  p->nFresh = 0;
  p->pEntry = 0;
  p->pLast = 0;
  p->pForest = 0;
  p->rsFlags = ROWSET_SORTED;
}

/*
** Allocate a new RowSetEntry object that is associated with the
** given RowSet.  Return a pointer to the new and completely uninitialized
** objected.
**
** In an OOM situation, the RowSet.db->mallocFailed flag is set and this
** routine returns NULL.
*/
static struct RowSetEntry *rowSetEntryAlloc(RowSet *p){
  assert( p!=0 );
  if( p->nFresh==0 ){  /*OPTIMIZATION-IF-FALSE*/
    /* We could allocate a fresh RowSetEntry each time one is needed, but it
    ** is more efficient to pull a preallocated entry from the pool */
    struct RowSetChunk *pNew;
    pNew = sqlite3DbMallocRawNN(p->db, sizeof(*pNew));
    if( pNew==0 ){
      return 0;
    }
    pNew->pNextChunk = p->pChunk;
    p->pChunk = pNew;
    p->pFresh = pNew->aEntry;
    p->nFresh = ROWSET_ENTRY_PER_CHUNK;
  }
  p->nFresh--;
  return p->pFresh++;
}

/*
** Insert a new value into a RowSet.
**
** The mallocFailed flag of the database connection is set if a
** memory allocation fails.
*/
SQLITE_PRIVATE void sqlite3RowSetInsert(RowSet *p, i64 rowid){
  struct RowSetEntry *pEntry;  /* The new entry */
  struct RowSetEntry *pLast;   /* The last prior entry */

  /* This routine is never called after sqlite3RowSetNext() */
  assert( p!=0 && (p->rsFlags & ROWSET_NEXT)==0 );

  pEntry = rowSetEntryAlloc(p);
  if( pEntry==0 ) return;
  pEntry->v = rowid;
  pEntry->pRight = 0;
  pLast = p->pLast;
  if( pLast ){
    if( rowid<=pLast->v ){  /*OPTIMIZATION-IF-FALSE*/
      /* Avoid unnecessary sorts by preserving the ROWSET_SORTED flags
      ** where possible */
      p->rsFlags &= ~ROWSET_SORTED;
    }
    pLast->pRight = pEntry;
  }else{
    p->pEntry = pEntry;
  }
  p->pLast = pEntry;
}

/*
** Merge two lists of RowSetEntry objects.  Remove duplicates.
**
** The input lists are connected via pRight pointers and are 
** assumed to each already be in sorted order.
*/
static struct RowSetEntry *rowSetEntryMerge(
  struct RowSetEntry *pA,    /* First sorted list to be merged */
  struct RowSetEntry *pB     /* Second sorted list to be merged */
){
  struct RowSetEntry head;
  struct RowSetEntry *pTail;

  pTail = &head;
  while( pA && pB ){
    assert( pA->pRight==0 || pA->v<=pA->pRight->v );
    assert( pB->pRight==0 || pB->v<=pB->pRight->v );
    if( pA->v<pB->v ){
      pTail->pRight = pA;
      pA = pA->pRight;
      pTail = pTail->pRight;
    }else if( pB->v<pA->v ){
      pTail->pRight = pB;
      pB = pB->pRight;
      pTail = pTail->pRight;
    }else{
      pA = pA->pRight;
    }
  }
  if( pA ){
    assert( pA->pRight==0 || pA->v<=pA->pRight->v );
    pTail->pRight = pA;
  }else{
    assert( pB==0 || pB->pRight==0 || pB->v<=pB->pRight->v );
    pTail->pRight = pB;
  }
  return head.pRight;
}

/*
** Sort all elements on the list of RowSetEntry objects into order of
** increasing v.
*/ 
static struct RowSetEntry *rowSetEntrySort(struct RowSetEntry *pIn){
  unsigned int i;
  struct RowSetEntry *pNext, *aBucket[40];

  memset(aBucket, 0, sizeof(aBucket));
  while( pIn ){
    pNext = pIn->pRight;
    pIn->pRight = 0;
    for(i=0; aBucket[i]; i++){
      pIn = rowSetEntryMerge(aBucket[i], pIn);
      aBucket[i] = 0;
    }
    aBucket[i] = pIn;
    pIn = pNext;
  }
  pIn = 0;
  for(i=0; i<sizeof(aBucket)/sizeof(aBucket[0]); i++){
    pIn = rowSetEntryMerge(pIn, aBucket[i]);
  }
  return pIn;
}


/*
** The input, pIn, is a binary tree (or subtree) of RowSetEntry objects.
** Convert this tree into a linked list connected by the pRight pointers
** and return pointers to the first and last elements of the new list.
*/
static void rowSetTreeToList(
  struct RowSetEntry *pIn,         /* Root of the input tree */
  struct RowSetEntry **ppFirst,    /* Write head of the output list here */
  struct RowSetEntry **ppLast      /* Write tail of the output list here */
){
  assert( pIn!=0 );
  if( pIn->pLeft ){
    struct RowSetEntry *p;
    rowSetTreeToList(pIn->pLeft, ppFirst, &p);
    p->pRight = pIn;
  }else{
    *ppFirst = pIn;
  }
  if( pIn->pRight ){
    rowSetTreeToList(pIn->pRight, &pIn->pRight, ppLast);
  }else{
    *ppLast = pIn;
  }
  assert( (*ppLast)->pRight==0 );
}


/*
** Convert a sorted list of elements (connected by pRight) into a binary
** tree with depth of iDepth.  A depth of 1 means the tree contains a single
** node taken from the head of *ppList.  A depth of 2 means a tree with
** three nodes.  And so forth.
**
** Use as many entries from the input list as required and update the
** *ppList to point to the unused elements of the list.  If the input
** list contains too few elements, then construct an incomplete tree
** and leave *ppList set to NULL.
**
** Return a pointer to the root of the constructed binary tree.
*/
static struct RowSetEntry *rowSetNDeepTree(
  struct RowSetEntry **ppList,
  int iDepth
){
  struct RowSetEntry *p;         /* Root of the new tree */
  struct RowSetEntry *pLeft;     /* Left subtree */
  if( *ppList==0 ){ /*OPTIMIZATION-IF-TRUE*/
    /* Prevent unnecessary deep recursion when we run out of entries */
    return 0; 
  }
  if( iDepth>1 ){   /*OPTIMIZATION-IF-TRUE*/
    /* This branch causes a *balanced* tree to be generated.  A valid tree
    ** is still generated without this branch, but the tree is wildly
    ** unbalanced and inefficient. */
    pLeft = rowSetNDeepTree(ppList, iDepth-1);
    p = *ppList;
    if( p==0 ){     /*OPTIMIZATION-IF-FALSE*/
      /* It is safe to always return here, but the resulting tree
      ** would be unbalanced */
      return pLeft;
    }
    p->pLeft = pLeft;
    *ppList = p->pRight;
    p->pRight = rowSetNDeepTree(ppList, iDepth-1);
  }else{
    p = *ppList;
    *ppList = p->pRight;
    p->pLeft = p->pRight = 0;
  }
  return p;
}

/*
** Convert a sorted list of elements into a binary tree. Make the tree
** as deep as it needs to be in order to contain the entire list.
*/
static struct RowSetEntry *rowSetListToTree(struct RowSetEntry *pList){
  int iDepth;           /* Depth of the tree so far */
  struct RowSetEntry *p;       /* Current tree root */
  struct RowSetEntry *pLeft;   /* Left subtree */

  assert( pList!=0 );
  p = pList;
  pList = p->pRight;
  p->pLeft = p->pRight = 0;
  for(iDepth=1; pList; iDepth++){
    pLeft = p;
    p = pList;
    pList = p->pRight;
    p->pLeft = pLeft;
    p->pRight = rowSetNDeepTree(&pList, iDepth);
  }
  return p;
}

/*
** Extract the smallest element from the RowSet.
** Write the element into *pRowid.  Return 1 on success.  Return
** 0 if the RowSet is already empty.
**
** After this routine has been called, the sqlite3RowSetInsert()
** routine may not be called again.
**
** This routine may not be called after sqlite3RowSetTest() has
** been used.  Older versions of RowSet allowed that, but as the
** capability was not used by the code generator, it was removed
** for code economy.
*/
SQLITE_PRIVATE int sqlite3RowSetNext(RowSet *p, i64 *pRowid){
  assert( p!=0 );
  assert( p->pForest==0 );  /* Cannot be used with sqlite3RowSetText() */

  /* Merge the forest into a single sorted list on first call */
  if( (p->rsFlags & ROWSET_NEXT)==0 ){  /*OPTIMIZATION-IF-FALSE*/
    if( (p->rsFlags & ROWSET_SORTED)==0 ){  /*OPTIMIZATION-IF-FALSE*/
      p->pEntry = rowSetEntrySort(p->pEntry);
    }
    p->rsFlags |= ROWSET_SORTED|ROWSET_NEXT;
  }

  /* Return the next entry on the list */
  if( p->pEntry ){
    *pRowid = p->pEntry->v;
    p->pEntry = p->pEntry->pRight;
    if( p->pEntry==0 ){ /*OPTIMIZATION-IF-TRUE*/
      /* Free memory immediately, rather than waiting on sqlite3_finalize() */
      sqlite3RowSetClear(p);
    }
    return 1;
  }else{
    return 0;
  }
}

/*
** Check to see if element iRowid was inserted into the rowset as
** part of any insert batch prior to iBatch.  Return 1 or 0.
**
** If this is the first test of a new batch and if there exist entries
** on pRowSet->pEntry, then sort those entries into the forest at
** pRowSet->pForest so that they can be tested.
*/
SQLITE_PRIVATE int sqlite3RowSetTest(RowSet *pRowSet, int iBatch, sqlite3_int64 iRowid){
  struct RowSetEntry *p, *pTree;

  /* This routine is never called after sqlite3RowSetNext() */
  assert( pRowSet!=0 && (pRowSet->rsFlags & ROWSET_NEXT)==0 );

  /* Sort entries into the forest on the first test of a new batch.
  ** To save unnecessary work, only do this when the batch number changes.
  */
  if( iBatch!=pRowSet->iBatch ){  /*OPTIMIZATION-IF-FALSE*/
    p = pRowSet->pEntry;
    if( p ){
      struct RowSetEntry **ppPrevTree = &pRowSet->pForest;
      if( (pRowSet->rsFlags & ROWSET_SORTED)==0 ){ /*OPTIMIZATION-IF-FALSE*/
        /* Only sort the current set of entiries if they need it */
        p = rowSetEntrySort(p);
      }
      for(pTree = pRowSet->pForest; pTree; pTree=pTree->pRight){
        ppPrevTree = &pTree->pRight;
        if( pTree->pLeft==0 ){
          pTree->pLeft = rowSetListToTree(p);
          break;
        }else{
          struct RowSetEntry *pAux, *pTail;
          rowSetTreeToList(pTree->pLeft, &pAux, &pTail);
          pTree->pLeft = 0;
          p = rowSetEntryMerge(pAux, p);
        }
      }
      if( pTree==0 ){
        *ppPrevTree = pTree = rowSetEntryAlloc(pRowSet);
        if( pTree ){
          pTree->v = 0;
          pTree->pRight = 0;
          pTree->pLeft = rowSetListToTree(p);
        }
      }
      pRowSet->pEntry = 0;
      pRowSet->pLast = 0;
      pRowSet->rsFlags |= ROWSET_SORTED;
    }
    pRowSet->iBatch = iBatch;
  }

  /* Test to see if the iRowid value appears anywhere in the forest.
  ** Return 1 if it does and 0 if not.
  */
  for(pTree = pRowSet->pForest; pTree; pTree=pTree->pRight){
    p = pTree->pLeft;
    while( p ){
      if( p->v<iRowid ){
        p = p->pRight;
      }else if( p->v>iRowid ){
        p = p->pLeft;
      }else{
        return 1;
      }
    }
  }
  return 0;
}

/************** End of rowset.c **********************************************/
