/************** Begin file tokenize.c ****************************************/
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
** An tokenizer for SQL
**
** This file contains C code that splits an SQL input string up into
** individual tokens and sends those tokens one-by-one over to the
** parser for analysis.
*/
/* #include "sqliteInt.h" */
/* #include <stdlib.h> */

/* Character classes for tokenizing
**
** In the sqlite3GetToken() function, a switch() on aiClass[c] is implemented
** using a lookup table, whereas a switch() directly on c uses a binary search.
** The lookup table is much faster.  To maximize speed, and to ensure that
** a lookup table is used, all of the classes need to be small integers and
** all of them need to be used within the switch.
*/
#define CC_X          0    /* The letter 'x', or start of BLOB literal */
#define CC_KYWD       1    /* Alphabetics or '_'.  Usable in a keyword */
#define CC_ID         2    /* unicode characters usable in IDs */
#define CC_DIGIT      3    /* Digits */
#define CC_DOLLAR     4    /* '$' */
#define CC_VARALPHA   5    /* '@', '#', ':'.  Alphabetic SQL variables */
#define CC_VARNUM     6    /* '?'.  Numeric SQL variables */
#define CC_SPACE      7    /* Space characters */
#define CC_QUOTE      8    /* '"', '\'', or '`'.  String literals, quoted ids */
#define CC_QUOTE2     9    /* '['.   [...] style quoted ids */
#define CC_PIPE      10    /* '|'.   Bitwise OR or concatenate */
#define CC_MINUS     11    /* '-'.  Minus or SQL-style comment */
#define CC_LT        12    /* '<'.  Part of < or <= or <> */
#define CC_GT        13    /* '>'.  Part of > or >= */
#define CC_EQ        14    /* '='.  Part of = or == */
#define CC_BANG      15    /* '!'.  Part of != */
#define CC_SLASH     16    /* '/'.  / or c-style comment */
#define CC_LP        17    /* '(' */
#define CC_RP        18    /* ')' */
#define CC_SEMI      19    /* ';' */
#define CC_PLUS      20    /* '+' */
#define CC_STAR      21    /* '*' */
#define CC_PERCENT   22    /* '%' */
#define CC_COMMA     23    /* ',' */
#define CC_AND       24    /* '&' */
#define CC_TILDA     25    /* '~' */
#define CC_DOT       26    /* '.' */
#define CC_ILLEGAL   27    /* Illegal character */

static const unsigned char aiClass[] = {
#ifdef SQLITE_ASCII
/*         x0  x1  x2  x3  x4  x5  x6  x7  x8  x9  xa  xb  xc  xd  xe  xf */
/* 0x */   27, 27, 27, 27, 27, 27, 27, 27, 27,  7,  7, 27,  7,  7, 27, 27,
/* 1x */   27, 27, 27, 27, 27, 27, 27, 27, 27, 27, 27, 27, 27, 27, 27, 27,
/* 2x */    7, 15,  8,  5,  4, 22, 24,  8, 17, 18, 21, 20, 23, 11, 26, 16,
/* 3x */    3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  5, 19, 12, 14, 13,  6,
/* 4x */    5,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,
/* 5x */    1,  1,  1,  1,  1,  1,  1,  1,  0,  1,  1,  9, 27, 27, 27,  1,
/* 6x */    8,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,
/* 7x */    1,  1,  1,  1,  1,  1,  1,  1,  0,  1,  1, 27, 10, 27, 25, 27,
/* 8x */    2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,
/* 9x */    2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,
/* Ax */    2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,
/* Bx */    2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,
/* Cx */    2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,
/* Dx */    2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,
/* Ex */    2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,
/* Fx */    2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2
#endif
#ifdef SQLITE_EBCDIC
/*         x0  x1  x2  x3  x4  x5  x6  x7  x8  x9  xa  xb  xc  xd  xe  xf */
/* 0x */   27, 27, 27, 27, 27,  7, 27, 27, 27, 27, 27, 27,  7,  7, 27, 27,
/* 1x */   27, 27, 27, 27, 27, 27, 27, 27, 27, 27, 27, 27, 27, 27, 27, 27,
/* 2x */   27, 27, 27, 27, 27,  7, 27, 27, 27, 27, 27, 27, 27, 27, 27, 27,
/* 3x */   27, 27, 27, 27, 27, 27, 27, 27, 27, 27, 27, 27, 27, 27, 27, 27,
/* 4x */    7, 27, 27, 27, 27, 27, 27, 27, 27, 27, 27, 27, 12, 17, 20, 10,
/* 5x */   24, 27, 27, 27, 27, 27, 27, 27, 27, 27, 15,  4, 21, 18, 19, 27,
/* 6x */   11, 16, 27, 27, 27, 27, 27, 27, 27, 27, 27, 23, 22,  1, 13,  7,
/* 7x */   27, 27, 27, 27, 27, 27, 27, 27, 27,  8,  5,  5,  5,  8, 14,  8,
/* 8x */   27,  1,  1,  1,  1,  1,  1,  1,  1,  1, 27, 27, 27, 27, 27, 27,
/* 9x */   27,  1,  1,  1,  1,  1,  1,  1,  1,  1, 27, 27, 27, 27, 27, 27,
/* 9x */   25,  1,  1,  1,  1,  1,  1,  0,  1,  1, 27, 27, 27, 27, 27, 27,
/* Bx */   27, 27, 27, 27, 27, 27, 27, 27, 27, 27,  9, 27, 27, 27, 27, 27,
/* Cx */   27,  1,  1,  1,  1,  1,  1,  1,  1,  1, 27, 27, 27, 27, 27, 27,
/* Dx */   27,  1,  1,  1,  1,  1,  1,  1,  1,  1, 27, 27, 27, 27, 27, 27,
/* Ex */   27, 27,  1,  1,  1,  1,  1,  0,  1,  1, 27, 27, 27, 27, 27, 27,
/* Fx */    3,  3,  3,  3,  3,  3,  3,  3,  3,  3, 27, 27, 27, 27, 27, 27,
#endif
};

/*
** The charMap() macro maps alphabetic characters (only) into their
** lower-case ASCII equivalent.  On ASCII machines, this is just
** an upper-to-lower case map.  On EBCDIC machines we also need
** to adjust the encoding.  The mapping is only valid for alphabetics
** which are the only characters for which this feature is used. 
**
** Used by keywordhash.h
*/
#ifdef SQLITE_ASCII
# define charMap(X) sqlite3UpperToLower[(unsigned char)X]
#endif
#ifdef SQLITE_EBCDIC
# define charMap(X) ebcdicToAscii[(unsigned char)X]
const unsigned char ebcdicToAscii[] = {
/* 0   1   2   3   4   5   6   7   8   9   A   B   C   D   E   F */
   0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  /* 0x */
   0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  /* 1x */
   0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  /* 2x */
   0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  /* 3x */
   0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  /* 4x */
   0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  /* 5x */
   0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0, 95,  0,  0,  /* 6x */
   0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  /* 7x */
   0, 97, 98, 99,100,101,102,103,104,105,  0,  0,  0,  0,  0,  0,  /* 8x */
   0,106,107,108,109,110,111,112,113,114,  0,  0,  0,  0,  0,  0,  /* 9x */
   0,  0,115,116,117,118,119,120,121,122,  0,  0,  0,  0,  0,  0,  /* Ax */
   0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  /* Bx */
   0, 97, 98, 99,100,101,102,103,104,105,  0,  0,  0,  0,  0,  0,  /* Cx */
   0,106,107,108,109,110,111,112,113,114,  0,  0,  0,  0,  0,  0,  /* Dx */
   0,  0,115,116,117,118,119,120,121,122,  0,  0,  0,  0,  0,  0,  /* Ex */
   0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  /* Fx */
};
#endif

/*
** The sqlite3KeywordCode function looks up an identifier to determine if
** it is a keyword.  If it is a keyword, the token code of that keyword is 
** returned.  If the input is not a keyword, TK_ID is returned.
**
** The implementation of this routine was generated by a program,
** mkkeywordhash.c, located in the tool subdirectory of the distribution.
** The output of the mkkeywordhash.c program is written into a file
** named keywordhash.h and then included into this source file by
** the #include below.
*/
/************** Include keywordhash.h in the middle of tokenize.c ************/
/************** Begin file keywordhash.h *************************************/
/***** This file contains automatically generated code ******
**
** The code in this file has been automatically generated by
**
**   sqlite/tool/mkkeywordhash.c
**
** The code in this file implements a function that determines whether
** or not a given identifier is really an SQL keyword.  The same thing
** might be implemented more directly using a hand-written hash table.
** But by using this automatically generated code, the size of the code
** is substantially reduced.  This is important for embedded applications
** on platforms with limited memory.
*/
/* Hash score: 182 */
static int keywordCode(const char *z, int n, int *pType){
  /* zText[] encodes 834 bytes of keywords in 554 bytes */
  /*   REINDEXEDESCAPEACHECKEYBEFOREIGNOREGEXPLAINSTEADDATABASELECT       */
  /*   ABLEFTHENDEFERRABLELSEXCEPTRANSACTIONATURALTERAISEXCLUSIVE         */
  /*   XISTSAVEPOINTERSECTRIGGEREFERENCESCONSTRAINTOFFSETEMPORARY         */
  /*   UNIQUERYWITHOUTERELEASEATTACHAVINGROUPDATEBEGINNERECURSIVE         */
  /*   BETWEENOTNULLIKECASCADELETECASECOLLATECREATECURRENT_DATEDETACH     */
  /*   IMMEDIATEJOINSERTMATCHPLANALYZEPRAGMABORTVALUESVIRTUALIMITWHEN     */
  /*   WHERENAMEAFTEREPLACEANDEFAULTAUTOINCREMENTCASTCOLUMNCOMMIT         */
  /*   CONFLICTCROSSCURRENT_TIMESTAMPRIMARYDEFERREDISTINCTDROPFAIL        */
  /*   FROMFULLGLOBYIFISNULLORDERESTRICTRIGHTROLLBACKROWUNIONUSING        */
  /*   VACUUMVIEWINITIALLY                                                */
  static const char zText[553] = {
    'R','E','I','N','D','E','X','E','D','E','S','C','A','P','E','A','C','H',
    'E','C','K','E','Y','B','E','F','O','R','E','I','G','N','O','R','E','G',
    'E','X','P','L','A','I','N','S','T','E','A','D','D','A','T','A','B','A',
    'S','E','L','E','C','T','A','B','L','E','F','T','H','E','N','D','E','F',
    'E','R','R','A','B','L','E','L','S','E','X','C','E','P','T','R','A','N',
    'S','A','C','T','I','O','N','A','T','U','R','A','L','T','E','R','A','I',
    'S','E','X','C','L','U','S','I','V','E','X','I','S','T','S','A','V','E',
    'P','O','I','N','T','E','R','S','E','C','T','R','I','G','G','E','R','E',
    'F','E','R','E','N','C','E','S','C','O','N','S','T','R','A','I','N','T',
    'O','F','F','S','E','T','E','M','P','O','R','A','R','Y','U','N','I','Q',
    'U','E','R','Y','W','I','T','H','O','U','T','E','R','E','L','E','A','S',
    'E','A','T','T','A','C','H','A','V','I','N','G','R','O','U','P','D','A',
    'T','E','B','E','G','I','N','N','E','R','E','C','U','R','S','I','V','E',
    'B','E','T','W','E','E','N','O','T','N','U','L','L','I','K','E','C','A',
    'S','C','A','D','E','L','E','T','E','C','A','S','E','C','O','L','L','A',
    'T','E','C','R','E','A','T','E','C','U','R','R','E','N','T','_','D','A',
    'T','E','D','E','T','A','C','H','I','M','M','E','D','I','A','T','E','J',
    'O','I','N','S','E','R','T','M','A','T','C','H','P','L','A','N','A','L',
    'Y','Z','E','P','R','A','G','M','A','B','O','R','T','V','A','L','U','E',
    'S','V','I','R','T','U','A','L','I','M','I','T','W','H','E','N','W','H',
    'E','R','E','N','A','M','E','A','F','T','E','R','E','P','L','A','C','E',
    'A','N','D','E','F','A','U','L','T','A','U','T','O','I','N','C','R','E',
    'M','E','N','T','C','A','S','T','C','O','L','U','M','N','C','O','M','M',
    'I','T','C','O','N','F','L','I','C','T','C','R','O','S','S','C','U','R',
    'R','E','N','T','_','T','I','M','E','S','T','A','M','P','R','I','M','A',
    'R','Y','D','E','F','E','R','R','E','D','I','S','T','I','N','C','T','D',
    'R','O','P','F','A','I','L','F','R','O','M','F','U','L','L','G','L','O',
    'B','Y','I','F','I','S','N','U','L','L','O','R','D','E','R','E','S','T',
    'R','I','C','T','R','I','G','H','T','R','O','L','L','B','A','C','K','R',
    'O','W','U','N','I','O','N','U','S','I','N','G','V','A','C','U','U','M',
    'V','I','E','W','I','N','I','T','I','A','L','L','Y',
  };
  static const unsigned char aHash[127] = {
      76, 105, 117,  74,   0,  45,   0,   0,  82,   0,  77,   0,   0,
      42,  12,  78,  15,   0, 116,  85,  54, 112,   0,  19,   0,   0,
     121,   0, 119, 115,   0,  22,  93,   0,   9,   0,   0,  70,  71,
       0,  69,   6,   0,  48,  90, 102,   0, 118, 101,   0,   0,  44,
       0, 103,  24,   0,  17,   0, 122,  53,  23,   0,   5, 110,  25,
      96,   0,   0, 124, 106,  60, 123,  57,  28,  55,   0,  91,   0,
     100,  26,   0,  99,   0,   0,   0,  95,  92,  97,  88, 109,  14,
      39, 108,   0,  81,   0,  18,  89, 111,  32,   0, 120,  80, 113,
      62,  46,  84,   0,   0,  94,  40,  59, 114,   0,  36,   0,   0,
      29,   0,  86,  63,  64,   0,  20,  61,   0,  56,
  };
  static const unsigned char aNext[124] = {
       0,   0,   0,   0,   4,   0,   0,   0,   0,   0,   0,   0,   0,
       0,   2,   0,   0,   0,   0,   0,   0,  13,   0,   0,   0,   0,
       0,   7,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
       0,   0,   0,   0,  33,   0,  21,   0,   0,   0,   0,   0,  50,
       0,  43,   3,  47,   0,   0,   0,   0,  30,   0,  58,   0,  38,
       0,   0,   0,   1,  66,   0,   0,  67,   0,  41,   0,   0,   0,
       0,   0,   0,  49,  65,   0,   0,   0,   0,  31,  52,  16,  34,
      10,   0,   0,   0,   0,   0,   0,   0,  11,  72,  79,   0,   8,
       0, 104,  98,   0, 107,   0,  87,   0,  75,  51,   0,  27,  37,
      73,  83,   0,  35,  68,   0,   0,
  };
  static const unsigned char aLen[124] = {
       7,   7,   5,   4,   6,   4,   5,   3,   6,   7,   3,   6,   6,
       7,   7,   3,   8,   2,   6,   5,   4,   4,   3,  10,   4,   6,
      11,   6,   2,   7,   5,   5,   9,   6,   9,   9,   7,  10,  10,
       4,   6,   2,   3,   9,   4,   2,   6,   5,   7,   4,   5,   7,
       6,   6,   5,   6,   5,   5,   9,   7,   7,   3,   2,   4,   4,
       7,   3,   6,   4,   7,   6,  12,   6,   9,   4,   6,   5,   4,
       7,   6,   5,   6,   7,   5,   4,   5,   6,   5,   7,   3,   7,
      13,   2,   2,   4,   6,   6,   8,   5,  17,  12,   7,   8,   8,
       2,   4,   4,   4,   4,   4,   2,   2,   6,   5,   8,   5,   8,
       3,   5,   5,   6,   4,   9,   3,
  };
  static const unsigned short int aOffset[124] = {
       0,   2,   2,   8,   9,  14,  16,  20,  23,  25,  25,  29,  33,
      36,  41,  46,  48,  53,  54,  59,  62,  65,  67,  69,  78,  81,
      86,  91,  95,  96, 101, 105, 109, 117, 122, 128, 136, 142, 152,
     159, 162, 162, 165, 167, 167, 171, 176, 179, 184, 184, 188, 192,
     199, 204, 209, 212, 218, 221, 225, 234, 240, 240, 240, 243, 246,
     250, 251, 255, 261, 265, 272, 278, 290, 296, 305, 307, 313, 318,
     320, 327, 332, 337, 343, 349, 354, 358, 361, 367, 371, 378, 380,
     387, 389, 391, 400, 404, 410, 416, 424, 429, 429, 445, 452, 459,
     460, 467, 471, 475, 479, 483, 486, 488, 490, 496, 500, 508, 513,
     521, 524, 529, 534, 540, 544, 549,
  };
  static const unsigned char aCode[124] = {
    TK_REINDEX,    TK_INDEXED,    TK_INDEX,      TK_DESC,       TK_ESCAPE,     
    TK_EACH,       TK_CHECK,      TK_KEY,        TK_BEFORE,     TK_FOREIGN,    
    TK_FOR,        TK_IGNORE,     TK_LIKE_KW,    TK_EXPLAIN,    TK_INSTEAD,    
    TK_ADD,        TK_DATABASE,   TK_AS,         TK_SELECT,     TK_TABLE,      
    TK_JOIN_KW,    TK_THEN,       TK_END,        TK_DEFERRABLE, TK_ELSE,       
    TK_EXCEPT,     TK_TRANSACTION,TK_ACTION,     TK_ON,         TK_JOIN_KW,    
    TK_ALTER,      TK_RAISE,      TK_EXCLUSIVE,  TK_EXISTS,     TK_SAVEPOINT,  
    TK_INTERSECT,  TK_TRIGGER,    TK_REFERENCES, TK_CONSTRAINT, TK_INTO,       
    TK_OFFSET,     TK_OF,         TK_SET,        TK_TEMP,       TK_TEMP,       
    TK_OR,         TK_UNIQUE,     TK_QUERY,      TK_WITHOUT,    TK_WITH,       
    TK_JOIN_KW,    TK_RELEASE,    TK_ATTACH,     TK_HAVING,     TK_GROUP,      
    TK_UPDATE,     TK_BEGIN,      TK_JOIN_KW,    TK_RECURSIVE,  TK_BETWEEN,    
    TK_NOTNULL,    TK_NOT,        TK_NO,         TK_NULL,       TK_LIKE_KW,    
    TK_CASCADE,    TK_ASC,        TK_DELETE,     TK_CASE,       TK_COLLATE,    
    TK_CREATE,     TK_CTIME_KW,   TK_DETACH,     TK_IMMEDIATE,  TK_JOIN,       
    TK_INSERT,     TK_MATCH,      TK_PLAN,       TK_ANALYZE,    TK_PRAGMA,     
    TK_ABORT,      TK_VALUES,     TK_VIRTUAL,    TK_LIMIT,      TK_WHEN,       
    TK_WHERE,      TK_RENAME,     TK_AFTER,      TK_REPLACE,    TK_AND,        
    TK_DEFAULT,    TK_AUTOINCR,   TK_TO,         TK_IN,         TK_CAST,       
    TK_COLUMNKW,   TK_COMMIT,     TK_CONFLICT,   TK_JOIN_KW,    TK_CTIME_KW,   
    TK_CTIME_KW,   TK_PRIMARY,    TK_DEFERRED,   TK_DISTINCT,   TK_IS,         
    TK_DROP,       TK_FAIL,       TK_FROM,       TK_JOIN_KW,    TK_LIKE_KW,    
    TK_BY,         TK_IF,         TK_ISNULL,     TK_ORDER,      TK_RESTRICT,   
    TK_JOIN_KW,    TK_ROLLBACK,   TK_ROW,        TK_UNION,      TK_USING,      
    TK_VACUUM,     TK_VIEW,       TK_INITIALLY,  TK_ALL,        
  };
  int i, j;
  const char *zKW;
  if( n>=2 ){
    i = ((charMap(z[0])*4) ^ (charMap(z[n-1])*3) ^ n) % 127;
    for(i=((int)aHash[i])-1; i>=0; i=((int)aNext[i])-1){
      if( aLen[i]!=n ) continue;
      j = 0;
      zKW = &zText[aOffset[i]];
#ifdef SQLITE_ASCII
      while( j<n && (z[j]&~0x20)==zKW[j] ){ j++; }
#endif
#ifdef SQLITE_EBCDIC
      while( j<n && toupper(z[j])==zKW[j] ){ j++; }
#endif
      if( j<n ) continue;
      testcase( i==0 ); /* REINDEX */
      testcase( i==1 ); /* INDEXED */
      testcase( i==2 ); /* INDEX */
      testcase( i==3 ); /* DESC */
      testcase( i==4 ); /* ESCAPE */
      testcase( i==5 ); /* EACH */
      testcase( i==6 ); /* CHECK */
      testcase( i==7 ); /* KEY */
      testcase( i==8 ); /* BEFORE */
      testcase( i==9 ); /* FOREIGN */
      testcase( i==10 ); /* FOR */
      testcase( i==11 ); /* IGNORE */
      testcase( i==12 ); /* REGEXP */
      testcase( i==13 ); /* EXPLAIN */
      testcase( i==14 ); /* INSTEAD */
      testcase( i==15 ); /* ADD */
      testcase( i==16 ); /* DATABASE */
      testcase( i==17 ); /* AS */
      testcase( i==18 ); /* SELECT */
      testcase( i==19 ); /* TABLE */
      testcase( i==20 ); /* LEFT */
      testcase( i==21 ); /* THEN */
      testcase( i==22 ); /* END */
      testcase( i==23 ); /* DEFERRABLE */
      testcase( i==24 ); /* ELSE */
      testcase( i==25 ); /* EXCEPT */
      testcase( i==26 ); /* TRANSACTION */
      testcase( i==27 ); /* ACTION */
      testcase( i==28 ); /* ON */
      testcase( i==29 ); /* NATURAL */
      testcase( i==30 ); /* ALTER */
      testcase( i==31 ); /* RAISE */
      testcase( i==32 ); /* EXCLUSIVE */
      testcase( i==33 ); /* EXISTS */
      testcase( i==34 ); /* SAVEPOINT */
      testcase( i==35 ); /* INTERSECT */
      testcase( i==36 ); /* TRIGGER */
      testcase( i==37 ); /* REFERENCES */
      testcase( i==38 ); /* CONSTRAINT */
      testcase( i==39 ); /* INTO */
      testcase( i==40 ); /* OFFSET */
      testcase( i==41 ); /* OF */
      testcase( i==42 ); /* SET */
      testcase( i==43 ); /* TEMPORARY */
      testcase( i==44 ); /* TEMP */
      testcase( i==45 ); /* OR */
      testcase( i==46 ); /* UNIQUE */
      testcase( i==47 ); /* QUERY */
      testcase( i==48 ); /* WITHOUT */
      testcase( i==49 ); /* WITH */
      testcase( i==50 ); /* OUTER */
      testcase( i==51 ); /* RELEASE */
      testcase( i==52 ); /* ATTACH */
      testcase( i==53 ); /* HAVING */
      testcase( i==54 ); /* GROUP */
      testcase( i==55 ); /* UPDATE */
      testcase( i==56 ); /* BEGIN */
      testcase( i==57 ); /* INNER */
      testcase( i==58 ); /* RECURSIVE */
      testcase( i==59 ); /* BETWEEN */
      testcase( i==60 ); /* NOTNULL */
      testcase( i==61 ); /* NOT */
      testcase( i==62 ); /* NO */
      testcase( i==63 ); /* NULL */
      testcase( i==64 ); /* LIKE */
      testcase( i==65 ); /* CASCADE */
      testcase( i==66 ); /* ASC */
      testcase( i==67 ); /* DELETE */
      testcase( i==68 ); /* CASE */
      testcase( i==69 ); /* COLLATE */
      testcase( i==70 ); /* CREATE */
      testcase( i==71 ); /* CURRENT_DATE */
      testcase( i==72 ); /* DETACH */
      testcase( i==73 ); /* IMMEDIATE */
      testcase( i==74 ); /* JOIN */
      testcase( i==75 ); /* INSERT */
      testcase( i==76 ); /* MATCH */
      testcase( i==77 ); /* PLAN */
      testcase( i==78 ); /* ANALYZE */
      testcase( i==79 ); /* PRAGMA */
      testcase( i==80 ); /* ABORT */
      testcase( i==81 ); /* VALUES */
      testcase( i==82 ); /* VIRTUAL */
      testcase( i==83 ); /* LIMIT */
      testcase( i==84 ); /* WHEN */
      testcase( i==85 ); /* WHERE */
      testcase( i==86 ); /* RENAME */
      testcase( i==87 ); /* AFTER */
      testcase( i==88 ); /* REPLACE */
      testcase( i==89 ); /* AND */
      testcase( i==90 ); /* DEFAULT */
      testcase( i==91 ); /* AUTOINCREMENT */
      testcase( i==92 ); /* TO */
      testcase( i==93 ); /* IN */
      testcase( i==94 ); /* CAST */
      testcase( i==95 ); /* COLUMN */
      testcase( i==96 ); /* COMMIT */
      testcase( i==97 ); /* CONFLICT */
      testcase( i==98 ); /* CROSS */
      testcase( i==99 ); /* CURRENT_TIMESTAMP */
      testcase( i==100 ); /* CURRENT_TIME */
      testcase( i==101 ); /* PRIMARY */
      testcase( i==102 ); /* DEFERRED */
      testcase( i==103 ); /* DISTINCT */
      testcase( i==104 ); /* IS */
      testcase( i==105 ); /* DROP */
      testcase( i==106 ); /* FAIL */
      testcase( i==107 ); /* FROM */
      testcase( i==108 ); /* FULL */
      testcase( i==109 ); /* GLOB */
      testcase( i==110 ); /* BY */
      testcase( i==111 ); /* IF */
      testcase( i==112 ); /* ISNULL */
      testcase( i==113 ); /* ORDER */
      testcase( i==114 ); /* RESTRICT */
      testcase( i==115 ); /* RIGHT */
      testcase( i==116 ); /* ROLLBACK */
      testcase( i==117 ); /* ROW */
      testcase( i==118 ); /* UNION */
      testcase( i==119 ); /* USING */
      testcase( i==120 ); /* VACUUM */
      testcase( i==121 ); /* VIEW */
      testcase( i==122 ); /* INITIALLY */
      testcase( i==123 ); /* ALL */
      *pType = aCode[i];
      break;
    }
  }
  return n;
}
SQLITE_PRIVATE int sqlite3KeywordCode(const unsigned char *z, int n){
  int id = TK_ID;
  keywordCode((char*)z, n, &id);
  return id;
}
#define SQLITE_N_KEYWORD 124

/************** End of keywordhash.h *****************************************/
/************** Continuing where we left off in tokenize.c *******************/


/*
** If X is a character that can be used in an identifier then
** IdChar(X) will be true.  Otherwise it is false.
**
** For ASCII, any character with the high-order bit set is
** allowed in an identifier.  For 7-bit characters, 
** sqlite3IsIdChar[X] must be 1.
**
** For EBCDIC, the rules are more complex but have the same
** end result.
**
** Ticket #1066.  the SQL standard does not allow '$' in the
** middle of identifiers.  But many SQL implementations do. 
** SQLite will allow '$' in identifiers for compatibility.
** But the feature is undocumented.
*/
#ifdef SQLITE_ASCII
#define IdChar(C)  ((sqlite3CtypeMap[(unsigned char)C]&0x46)!=0)
#endif
#ifdef SQLITE_EBCDIC
SQLITE_PRIVATE const char sqlite3IsEbcdicIdChar[] = {
/* x0 x1 x2 x3 x4 x5 x6 x7 x8 x9 xA xB xC xD xE xF */
    0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0,  /* 4x */
    0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 1, 0, 0, 0, 0,  /* 5x */
    0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 1, 0, 0,  /* 6x */
    0, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0,  /* 7x */
    0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 1, 1, 1, 0,  /* 8x */
    0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 1, 0, 1, 0,  /* 9x */
    1, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 1, 1, 1, 0,  /* Ax */
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  /* Bx */
    0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 1, 1, 1, 1, 1,  /* Cx */
    0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 1, 1, 1, 1, 1,  /* Dx */
    0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 0, 1, 1, 1, 1, 1,  /* Ex */
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 1, 1, 1, 1, 0,  /* Fx */
};
#define IdChar(C)  (((c=C)>=0x42 && sqlite3IsEbcdicIdChar[c-0x40]))
#endif

/* Make the IdChar function accessible from ctime.c */
#ifndef SQLITE_OMIT_COMPILEOPTION_DIAGS
SQLITE_PRIVATE int sqlite3IsIdChar(u8 c){ return IdChar(c); }
#endif


/*
** Return the length (in bytes) of the token that begins at z[0]. 
** Store the token type in *tokenType before returning.
*/
SQLITE_PRIVATE int sqlite3GetToken(const unsigned char *z, int *tokenType){
  int i, c;
  switch( aiClass[*z] ){  /* Switch on the character-class of the first byte
                          ** of the token. See the comment on the CC_ defines
                          ** above. */
    case CC_SPACE: {
      testcase( z[0]==' ' );
      testcase( z[0]=='\t' );
      testcase( z[0]=='\n' );
      testcase( z[0]=='\f' );
      testcase( z[0]=='\r' );
      for(i=1; sqlite3Isspace(z[i]); i++){}
      *tokenType = TK_SPACE;
      return i;
    }
    case CC_MINUS: {
      if( z[1]=='-' ){
        for(i=2; (c=z[i])!=0 && c!='\n'; i++){}
        *tokenType = TK_SPACE;   /* IMP: R-22934-25134 */
        return i;
      }
      *tokenType = TK_MINUS;
      return 1;
    }
    case CC_LP: {
      *tokenType = TK_LP;
      return 1;
    }
    case CC_RP: {
      *tokenType = TK_RP;
      return 1;
    }
    case CC_SEMI: {
      *tokenType = TK_SEMI;
      return 1;
    }
    case CC_PLUS: {
      *tokenType = TK_PLUS;
      return 1;
    }
    case CC_STAR: {
      *tokenType = TK_STAR;
      return 1;
    }
    case CC_SLASH: {
      if( z[1]!='*' || z[2]==0 ){
        *tokenType = TK_SLASH;
        return 1;
      }
      for(i=3, c=z[2]; (c!='*' || z[i]!='/') && (c=z[i])!=0; i++){}
      if( c ) i++;
      *tokenType = TK_SPACE;   /* IMP: R-22934-25134 */
      return i;
    }
    case CC_PERCENT: {
      *tokenType = TK_REM;
      return 1;
    }
    case CC_EQ: {
      *tokenType = TK_EQ;
      return 1 + (z[1]=='=');
    }
    case CC_LT: {
      if( (c=z[1])=='=' ){
        *tokenType = TK_LE;
        return 2;
      }else if( c=='>' ){
        *tokenType = TK_NE;
        return 2;
      }else if( c=='<' ){
        *tokenType = TK_LSHIFT;
        return 2;
      }else{
        *tokenType = TK_LT;
        return 1;
      }
    }
    case CC_GT: {
      if( (c=z[1])=='=' ){
        *tokenType = TK_GE;
        return 2;
      }else if( c=='>' ){
        *tokenType = TK_RSHIFT;
        return 2;
      }else{
        *tokenType = TK_GT;
        return 1;
      }
    }
    case CC_BANG: {
      if( z[1]!='=' ){
        *tokenType = TK_ILLEGAL;
        return 1;
      }else{
        *tokenType = TK_NE;
        return 2;
      }
    }
    case CC_PIPE: {
      if( z[1]!='|' ){
        *tokenType = TK_BITOR;
        return 1;
      }else{
        *tokenType = TK_CONCAT;
        return 2;
      }
    }
    case CC_COMMA: {
      *tokenType = TK_COMMA;
      return 1;
    }
    case CC_AND: {
      *tokenType = TK_BITAND;
      return 1;
    }
    case CC_TILDA: {
      *tokenType = TK_BITNOT;
      return 1;
    }
    case CC_QUOTE: {
      int delim = z[0];
      testcase( delim=='`' );
      testcase( delim=='\'' );
      testcase( delim=='"' );
      for(i=1; (c=z[i])!=0; i++){
        if( c==delim ){
          if( z[i+1]==delim ){
            i++;
          }else{
            break;
          }
        }
      }
      if( c=='\'' ){
        *tokenType = TK_STRING;
        return i+1;
      }else if( c!=0 ){
        *tokenType = TK_ID;
        return i+1;
      }else{
        *tokenType = TK_ILLEGAL;
        return i;
      }
    }
    case CC_DOT: {
#ifndef SQLITE_OMIT_FLOATING_POINT
      if( !sqlite3Isdigit(z[1]) )
#endif
      {
        *tokenType = TK_DOT;
        return 1;
      }
      /* If the next character is a digit, this is a floating point
      ** number that begins with ".".  Fall thru into the next case */
    }
    case CC_DIGIT: {
      testcase( z[0]=='0' );  testcase( z[0]=='1' );  testcase( z[0]=='2' );
      testcase( z[0]=='3' );  testcase( z[0]=='4' );  testcase( z[0]=='5' );
      testcase( z[0]=='6' );  testcase( z[0]=='7' );  testcase( z[0]=='8' );
      testcase( z[0]=='9' );
      *tokenType = TK_INTEGER;
#ifndef SQLITE_OMIT_HEX_INTEGER
      if( z[0]=='0' && (z[1]=='x' || z[1]=='X') && sqlite3Isxdigit(z[2]) ){
        for(i=3; sqlite3Isxdigit(z[i]); i++){}
        return i;
      }
#endif
      for(i=0; sqlite3Isdigit(z[i]); i++){}
#ifndef SQLITE_OMIT_FLOATING_POINT
      if( z[i]=='.' ){
        i++;
        while( sqlite3Isdigit(z[i]) ){ i++; }
        *tokenType = TK_FLOAT;
      }
      if( (z[i]=='e' || z[i]=='E') &&
           ( sqlite3Isdigit(z[i+1]) 
            || ((z[i+1]=='+' || z[i+1]=='-') && sqlite3Isdigit(z[i+2]))
           )
      ){
        i += 2;
        while( sqlite3Isdigit(z[i]) ){ i++; }
        *tokenType = TK_FLOAT;
      }
#endif
      while( IdChar(z[i]) ){
        *tokenType = TK_ILLEGAL;
        i++;
      }
      return i;
    }
    case CC_QUOTE2: {
      for(i=1, c=z[0]; c!=']' && (c=z[i])!=0; i++){}
      *tokenType = c==']' ? TK_ID : TK_ILLEGAL;
      return i;
    }
    case CC_VARNUM: {
      *tokenType = TK_VARIABLE;
      for(i=1; sqlite3Isdigit(z[i]); i++){}
      return i;
    }
    case CC_DOLLAR:
    case CC_VARALPHA: {
      int n = 0;
      testcase( z[0]=='$' );  testcase( z[0]=='@' );
      testcase( z[0]==':' );  testcase( z[0]=='#' );
      *tokenType = TK_VARIABLE;
      for(i=1; (c=z[i])!=0; i++){
        if( IdChar(c) ){
          n++;
#ifndef SQLITE_OMIT_TCL_VARIABLE
        }else if( c=='(' && n>0 ){
          do{
            i++;
          }while( (c=z[i])!=0 && !sqlite3Isspace(c) && c!=')' );
          if( c==')' ){
            i++;
          }else{
            *tokenType = TK_ILLEGAL;
          }
          break;
        }else if( c==':' && z[i+1]==':' ){
          i++;
#endif
        }else{
          break;
        }
      }
      if( n==0 ) *tokenType = TK_ILLEGAL;
      return i;
    }
    case CC_KYWD: {
      for(i=1; aiClass[z[i]]<=CC_KYWD; i++){}
      if( IdChar(z[i]) ){
        /* This token started out using characters that can appear in keywords,
        ** but z[i] is a character not allowed within keywords, so this must
        ** be an identifier instead */
        i++;
        break;
      }
      *tokenType = TK_ID;
      return keywordCode((char*)z, i, tokenType);
    }
    case CC_X: {
#ifndef SQLITE_OMIT_BLOB_LITERAL
      testcase( z[0]=='x' ); testcase( z[0]=='X' );
      if( z[1]=='\'' ){
        *tokenType = TK_BLOB;
        for(i=2; sqlite3Isxdigit(z[i]); i++){}
        if( z[i]!='\'' || i%2 ){
          *tokenType = TK_ILLEGAL;
          while( z[i] && z[i]!='\'' ){ i++; }
        }
        if( z[i] ) i++;
        return i;
      }
#endif
      /* If it is not a BLOB literal, then it must be an ID, since no
      ** SQL keywords start with the letter 'x'.  Fall through */
    }
    case CC_ID: {
      i = 1;
      break;
    }
    default: {
      *tokenType = TK_ILLEGAL;
      return 1;
    }
  }
  while( IdChar(z[i]) ){ i++; }
  *tokenType = TK_ID;
  return i;
}

/*
** Run the parser on the given SQL string.  The parser structure is
** passed in.  An SQLITE_ status code is returned.  If an error occurs
** then an and attempt is made to write an error message into 
** memory obtained from sqlite3_malloc() and to make *pzErrMsg point to that
** error message.
*/
SQLITE_PRIVATE int sqlite3RunParser(Parse *pParse, const char *zSql, char **pzErrMsg){
  int nErr = 0;                   /* Number of errors encountered */
  int i;                          /* Loop counter */
  void *pEngine;                  /* The LEMON-generated LALR(1) parser */
  int tokenType;                  /* type of the next token */
  int lastTokenParsed = -1;       /* type of the previous token */
  sqlite3 *db = pParse->db;       /* The database connection */
  int mxSqlLen;                   /* Max length of an SQL string */

  assert( zSql!=0 );
  mxSqlLen = db->aLimit[SQLITE_LIMIT_SQL_LENGTH];
  if( db->nVdbeActive==0 ){
    db->u1.isInterrupted = 0;
  }
  pParse->rc = SQLITE_OK;
  pParse->zTail = zSql;
  i = 0;
  assert( pzErrMsg!=0 );
  /* sqlite3ParserTrace(stdout, "parser: "); */
  pEngine = sqlite3ParserAlloc(sqlite3Malloc);
  if( pEngine==0 ){
    sqlite3OomFault(db);
    return SQLITE_NOMEM_BKPT;
  }
  assert( pParse->pNewTable==0 );
  assert( pParse->pNewTrigger==0 );
  assert( pParse->nVar==0 );
  assert( pParse->nzVar==0 );
  assert( pParse->azVar==0 );
  while( zSql[i]!=0 ){
    assert( i>=0 );
    pParse->sLastToken.z = &zSql[i];
    pParse->sLastToken.n = sqlite3GetToken((unsigned char*)&zSql[i],&tokenType);
    i += pParse->sLastToken.n;
    if( i>mxSqlLen ){
      pParse->rc = SQLITE_TOOBIG;
      break;
    }
    if( tokenType>=TK_SPACE ){
      assert( tokenType==TK_SPACE || tokenType==TK_ILLEGAL );
      if( db->u1.isInterrupted ){
        pParse->rc = SQLITE_INTERRUPT;
        break;
      }
      if( tokenType==TK_ILLEGAL ){
        sqlite3ErrorMsg(pParse, "unrecognized token: \"%T\"",
                        &pParse->sLastToken);
        break;
      }
    }else{
      sqlite3Parser(pEngine, tokenType, pParse->sLastToken, pParse);
      lastTokenParsed = tokenType;
      if( pParse->rc!=SQLITE_OK || db->mallocFailed ) break;
    }
  }
  assert( nErr==0 );
  pParse->zTail = &zSql[i];
  if( pParse->rc==SQLITE_OK && db->mallocFailed==0 ){
    assert( zSql[i]==0 );
    if( lastTokenParsed!=TK_SEMI ){
      sqlite3Parser(pEngine, TK_SEMI, pParse->sLastToken, pParse);
    }
    if( pParse->rc==SQLITE_OK && db->mallocFailed==0 ){
      sqlite3Parser(pEngine, 0, pParse->sLastToken, pParse);
    }
  }
#ifdef YYTRACKMAXSTACKDEPTH
  sqlite3_mutex_enter(sqlite3MallocMutex());
  sqlite3StatusHighwater(SQLITE_STATUS_PARSER_STACK,
      sqlite3ParserStackPeak(pEngine)
  );
  sqlite3_mutex_leave(sqlite3MallocMutex());
#endif /* YYDEBUG */
  sqlite3ParserFree(pEngine, sqlite3_free);
  if( db->mallocFailed ){
    pParse->rc = SQLITE_NOMEM_BKPT;
  }
  if( pParse->rc!=SQLITE_OK && pParse->rc!=SQLITE_DONE && pParse->zErrMsg==0 ){
    pParse->zErrMsg = sqlite3MPrintf(db, "%s", sqlite3ErrStr(pParse->rc));
  }
  assert( pzErrMsg!=0 );
  if( pParse->zErrMsg ){
    *pzErrMsg = pParse->zErrMsg;
    sqlite3_log(pParse->rc, "%s", *pzErrMsg);
    pParse->zErrMsg = 0;
    nErr++;
  }
  if( pParse->pVdbe && pParse->nErr>0 && pParse->nested==0 ){
    sqlite3VdbeDelete(pParse->pVdbe);
    pParse->pVdbe = 0;
  }
#ifndef SQLITE_OMIT_SHARED_CACHE
  if( pParse->nested==0 ){
    sqlite3DbFree(db, pParse->aTableLock);
    pParse->aTableLock = 0;
    pParse->nTableLock = 0;
  }
#endif
#ifndef SQLITE_OMIT_VIRTUALTABLE
  sqlite3_free(pParse->apVtabLock);
#endif

  if( !IN_DECLARE_VTAB ){
    /* If the pParse->declareVtab flag is set, do not delete any table 
    ** structure built up in pParse->pNewTable. The calling code (see vtab.c)
    ** will take responsibility for freeing the Table structure.
    */
    sqlite3DeleteTable(db, pParse->pNewTable);
  }

  if( pParse->pWithToFree ) sqlite3WithDelete(db, pParse->pWithToFree);
  sqlite3DeleteTrigger(db, pParse->pNewTrigger);
  for(i=pParse->nzVar-1; i>=0; i--) sqlite3DbFree(db, pParse->azVar[i]);
  sqlite3DbFree(db, pParse->azVar);
  while( pParse->pAinc ){
    AutoincInfo *p = pParse->pAinc;
    pParse->pAinc = p->pNext;
    sqlite3DbFree(db, p);
  }
  while( pParse->pZombieTab ){
    Table *p = pParse->pZombieTab;
    pParse->pZombieTab = p->pNextZombie;
    sqlite3DeleteTable(db, p);
  }
  assert( nErr==0 || pParse->rc!=SQLITE_OK );
  return nErr;
}

/************** End of tokenize.c ********************************************/
/************** Begin file complete.c ****************************************/
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
** An tokenizer for SQL
**
** This file contains C code that implements the sqlite3_complete() API.
** This code used to be part of the tokenizer.c source file.  But by
** separating it out, the code will be automatically omitted from
** static links that do not use it.
*/
/* #include "sqliteInt.h" */
#ifndef SQLITE_OMIT_COMPLETE

/*
** This is defined in tokenize.c.  We just have to import the definition.
*/
#ifndef SQLITE_AMALGAMATION
#ifdef SQLITE_ASCII
#define IdChar(C)  ((sqlite3CtypeMap[(unsigned char)C]&0x46)!=0)
#endif
#ifdef SQLITE_EBCDIC
SQLITE_PRIVATE const char sqlite3IsEbcdicIdChar[];
#define IdChar(C)  (((c=C)>=0x42 && sqlite3IsEbcdicIdChar[c-0x40]))
#endif
#endif /* SQLITE_AMALGAMATION */


/*
** Token types used by the sqlite3_complete() routine.  See the header
** comments on that procedure for additional information.
*/
#define tkSEMI    0
#define tkWS      1
#define tkOTHER   2
#ifndef SQLITE_OMIT_TRIGGER
#define tkEXPLAIN 3
#define tkCREATE  4
#define tkTEMP    5
#define tkTRIGGER 6
#define tkEND     7
#endif

/*
** Return TRUE if the given SQL string ends in a semicolon.
**
** Special handling is require for CREATE TRIGGER statements.
** Whenever the CREATE TRIGGER keywords are seen, the statement
** must end with ";END;".
**
** This implementation uses a state machine with 8 states:
**
**   (0) INVALID   We have not yet seen a non-whitespace character.
**
**   (1) START     At the beginning or end of an SQL statement.  This routine
**                 returns 1 if it ends in the START state and 0 if it ends
**                 in any other state.
**
**   (2) NORMAL    We are in the middle of statement which ends with a single
**                 semicolon.
**
**   (3) EXPLAIN   The keyword EXPLAIN has been seen at the beginning of 
**                 a statement.
**
**   (4) CREATE    The keyword CREATE has been seen at the beginning of a
**                 statement, possibly preceded by EXPLAIN and/or followed by
**                 TEMP or TEMPORARY
**
**   (5) TRIGGER   We are in the middle of a trigger definition that must be
**                 ended by a semicolon, the keyword END, and another semicolon.
**
**   (6) SEMI      We've seen the first semicolon in the ";END;" that occurs at
**                 the end of a trigger definition.
**
**   (7) END       We've seen the ";END" of the ";END;" that occurs at the end
**                 of a trigger definition.
**
** Transitions between states above are determined by tokens extracted
** from the input.  The following tokens are significant:
**
**   (0) tkSEMI      A semicolon.
**   (1) tkWS        Whitespace.
**   (2) tkOTHER     Any other SQL token.
**   (3) tkEXPLAIN   The "explain" keyword.
**   (4) tkCREATE    The "create" keyword.
**   (5) tkTEMP      The "temp" or "temporary" keyword.
**   (6) tkTRIGGER   The "trigger" keyword.
**   (7) tkEND       The "end" keyword.
**
** Whitespace never causes a state transition and is always ignored.
** This means that a SQL string of all whitespace is invalid.
**
** If we compile with SQLITE_OMIT_TRIGGER, all of the computation needed
** to recognize the end of a trigger can be omitted.  All we have to do
** is look for a semicolon that is not part of an string or comment.
*/
SQLITE_API int SQLITE_STDCALL sqlite3_complete(const char *zSql){
  u8 state = 0;   /* Current state, using numbers defined in header comment */
  u8 token;       /* Value of the next token */

#ifndef SQLITE_OMIT_TRIGGER
  /* A complex statement machine used to detect the end of a CREATE TRIGGER
  ** statement.  This is the normal case.
  */
  static const u8 trans[8][8] = {
                     /* Token:                                                */
     /* State:       **  SEMI  WS  OTHER  EXPLAIN  CREATE  TEMP  TRIGGER  END */
     /* 0 INVALID: */ {    1,  0,     2,       3,      4,    2,       2,   2, },
     /* 1   START: */ {    1,  1,     2,       3,      4,    2,       2,   2, },
     /* 2  NORMAL: */ {    1,  2,     2,       2,      2,    2,       2,   2, },
     /* 3 EXPLAIN: */ {    1,  3,     3,       2,      4,    2,       2,   2, },
     /* 4  CREATE: */ {    1,  4,     2,       2,      2,    4,       5,   2, },
     /* 5 TRIGGER: */ {    6,  5,     5,       5,      5,    5,       5,   5, },
     /* 6    SEMI: */ {    6,  6,     5,       5,      5,    5,       5,   7, },
     /* 7     END: */ {    1,  7,     5,       5,      5,    5,       5,   5, },
  };
#else
  /* If triggers are not supported by this compile then the statement machine
  ** used to detect the end of a statement is much simpler
  */
  static const u8 trans[3][3] = {
                     /* Token:           */
     /* State:       **  SEMI  WS  OTHER */
     /* 0 INVALID: */ {    1,  0,     2, },
     /* 1   START: */ {    1,  1,     2, },
     /* 2  NORMAL: */ {    1,  2,     2, },
  };
#endif /* SQLITE_OMIT_TRIGGER */

#ifdef SQLITE_ENABLE_API_ARMOR
  if( zSql==0 ){
    (void)SQLITE_MISUSE_BKPT;
    return 0;
  }
#endif

  while( *zSql ){
    switch( *zSql ){
      case ';': {  /* A semicolon */
        token = tkSEMI;
        break;
      }
      case ' ':
      case '\r':
      case '\t':
      case '\n':
      case '\f': {  /* White space is ignored */
        token = tkWS;
        break;
      }
      case '/': {   /* C-style comments */
        if( zSql[1]!='*' ){
          token = tkOTHER;
          break;
        }
        zSql += 2;
        while( zSql[0] && (zSql[0]!='*' || zSql[1]!='/') ){ zSql++; }
        if( zSql[0]==0 ) return 0;
        zSql++;
        token = tkWS;
        break;
      }
      case '-': {   /* SQL-style comments from "--" to end of line */
        if( zSql[1]!='-' ){
          token = tkOTHER;
          break;
        }
        while( *zSql && *zSql!='\n' ){ zSql++; }
        if( *zSql==0 ) return state==1;
        token = tkWS;
        break;
      }
      case '[': {   /* Microsoft-style identifiers in [...] */
        zSql++;
        while( *zSql && *zSql!=']' ){ zSql++; }
        if( *zSql==0 ) return 0;
        token = tkOTHER;
        break;
      }
      case '`':     /* Grave-accent quoted symbols used by MySQL */
      case '"':     /* single- and double-quoted strings */
      case '\'': {
        int c = *zSql;
        zSql++;
        while( *zSql && *zSql!=c ){ zSql++; }
        if( *zSql==0 ) return 0;
        token = tkOTHER;
        break;
      }
      default: {
#ifdef SQLITE_EBCDIC
        unsigned char c;
#endif
        if( IdChar((u8)*zSql) ){
          /* Keywords and unquoted identifiers */
          int nId;
          for(nId=1; IdChar(zSql[nId]); nId++){}
#ifdef SQLITE_OMIT_TRIGGER
          token = tkOTHER;
#else
          switch( *zSql ){
            case 'c': case 'C': {
              if( nId==6 && sqlite3StrNICmp(zSql, "create", 6)==0 ){
                token = tkCREATE;
              }else{
                token = tkOTHER;
              }
              break;
            }
            case 't': case 'T': {
              if( nId==7 && sqlite3StrNICmp(zSql, "trigger", 7)==0 ){
                token = tkTRIGGER;
              }else if( nId==4 && sqlite3StrNICmp(zSql, "temp", 4)==0 ){
                token = tkTEMP;
              }else if( nId==9 && sqlite3StrNICmp(zSql, "temporary", 9)==0 ){
                token = tkTEMP;
              }else{
                token = tkOTHER;
              }
              break;
            }
            case 'e':  case 'E': {
              if( nId==3 && sqlite3StrNICmp(zSql, "end", 3)==0 ){
                token = tkEND;
              }else
#ifndef SQLITE_OMIT_EXPLAIN
              if( nId==7 && sqlite3StrNICmp(zSql, "explain", 7)==0 ){
                token = tkEXPLAIN;
              }else
#endif
              {
                token = tkOTHER;
              }
              break;
            }
            default: {
              token = tkOTHER;
              break;
            }
          }
#endif /* SQLITE_OMIT_TRIGGER */
          zSql += nId-1;
        }else{
          /* Operators and special symbols */
          token = tkOTHER;
        }
        break;
      }
    }
    state = trans[state][token];
    zSql++;
  }
  return state==1;
}

#ifndef SQLITE_OMIT_UTF16
/*
** This routine is the same as the sqlite3_complete() routine described
** above, except that the parameter is required to be UTF-16 encoded, not
** UTF-8.
*/
SQLITE_API int SQLITE_STDCALL sqlite3_complete16(const void *zSql){
  sqlite3_value *pVal;
  char const *zSql8;
  int rc;

#ifndef SQLITE_OMIT_AUTOINIT
  rc = sqlite3_initialize();
  if( rc ) return rc;
#endif
  pVal = sqlite3ValueNew(0);
  sqlite3ValueSetStr(pVal, -1, zSql, SQLITE_UTF16NATIVE, SQLITE_STATIC);
  zSql8 = sqlite3ValueText(pVal, SQLITE_UTF8);
  if( zSql8 ){
    rc = sqlite3_complete(zSql8);
  }else{
    rc = SQLITE_NOMEM_BKPT;
  }
  sqlite3ValueFree(pVal);
  return rc & 0xff;
}
#endif /* SQLITE_OMIT_UTF16 */
#endif /* SQLITE_OMIT_COMPLETE */

/************** End of complete.c ********************************************/
