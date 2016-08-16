/************** Begin file utf.c *********************************************/
/*
** 2004 April 13
**
** The author disclaims copyright to this source code.  In place of
** a legal notice, here is a blessing:
**
**    May you do good and not evil.
**    May you find forgiveness for yourself and forgive others.
**    May you share freely, never taking more than you give.
**
*************************************************************************
** This file contains routines used to translate between UTF-8, 
** UTF-16, UTF-16BE, and UTF-16LE.
**
** Notes on UTF-8:
**
**   Byte-0    Byte-1    Byte-2    Byte-3    Value
**  0xxxxxxx                                 00000000 00000000 0xxxxxxx
**  110yyyyy  10xxxxxx                       00000000 00000yyy yyxxxxxx
**  1110zzzz  10yyyyyy  10xxxxxx             00000000 zzzzyyyy yyxxxxxx
**  11110uuu  10uuzzzz  10yyyyyy  10xxxxxx   000uuuuu zzzzyyyy yyxxxxxx
**
**
** Notes on UTF-16:  (with wwww+1==uuuuu)
**
**      Word-0               Word-1          Value
**  110110ww wwzzzzyy   110111yy yyxxxxxx    000uuuuu zzzzyyyy yyxxxxxx
**  zzzzyyyy yyxxxxxx                        00000000 zzzzyyyy yyxxxxxx
**
**
** BOM or Byte Order Mark:
**     0xff 0xfe   little-endian utf-16 follows
**     0xfe 0xff   big-endian utf-16 follows
**
*/
/* #include "sqliteInt.h" */
/* #include <assert.h> */
/* #include "vdbeInt.h" */

#if !defined(SQLITE_AMALGAMATION) && SQLITE_BYTEORDER==0
/*
** The following constant value is used by the SQLITE_BIGENDIAN and
** SQLITE_LITTLEENDIAN macros.
*/
SQLITE_PRIVATE const int sqlite3one = 1;
#endif /* SQLITE_AMALGAMATION && SQLITE_BYTEORDER==0 */

/*
** This lookup table is used to help decode the first byte of
** a multi-byte UTF8 character.
*/
static const unsigned char sqlite3Utf8Trans1[] = {
  0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
  0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f,
  0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17,
  0x18, 0x19, 0x1a, 0x1b, 0x1c, 0x1d, 0x1e, 0x1f,
  0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
  0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f,
  0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
  0x00, 0x01, 0x02, 0x03, 0x00, 0x01, 0x00, 0x00,
};


#define WRITE_UTF8(zOut, c) {                          \
  if( c<0x00080 ){                                     \
    *zOut++ = (u8)(c&0xFF);                            \
  }                                                    \
  else if( c<0x00800 ){                                \
    *zOut++ = 0xC0 + (u8)((c>>6)&0x1F);                \
    *zOut++ = 0x80 + (u8)(c & 0x3F);                   \
  }                                                    \
  else if( c<0x10000 ){                                \
    *zOut++ = 0xE0 + (u8)((c>>12)&0x0F);               \
    *zOut++ = 0x80 + (u8)((c>>6) & 0x3F);              \
    *zOut++ = 0x80 + (u8)(c & 0x3F);                   \
  }else{                                               \
    *zOut++ = 0xF0 + (u8)((c>>18) & 0x07);             \
    *zOut++ = 0x80 + (u8)((c>>12) & 0x3F);             \
    *zOut++ = 0x80 + (u8)((c>>6) & 0x3F);              \
    *zOut++ = 0x80 + (u8)(c & 0x3F);                   \
  }                                                    \
}

#define WRITE_UTF16LE(zOut, c) {                                    \
  if( c<=0xFFFF ){                                                  \
    *zOut++ = (u8)(c&0x00FF);                                       \
    *zOut++ = (u8)((c>>8)&0x00FF);                                  \
  }else{                                                            \
    *zOut++ = (u8)(((c>>10)&0x003F) + (((c-0x10000)>>10)&0x00C0));  \
    *zOut++ = (u8)(0x00D8 + (((c-0x10000)>>18)&0x03));              \
    *zOut++ = (u8)(c&0x00FF);                                       \
    *zOut++ = (u8)(0x00DC + ((c>>8)&0x03));                         \
  }                                                                 \
}

#define WRITE_UTF16BE(zOut, c) {                                    \
  if( c<=0xFFFF ){                                                  \
    *zOut++ = (u8)((c>>8)&0x00FF);                                  \
    *zOut++ = (u8)(c&0x00FF);                                       \
  }else{                                                            \
    *zOut++ = (u8)(0x00D8 + (((c-0x10000)>>18)&0x03));              \
    *zOut++ = (u8)(((c>>10)&0x003F) + (((c-0x10000)>>10)&0x00C0));  \
    *zOut++ = (u8)(0x00DC + ((c>>8)&0x03));                         \
    *zOut++ = (u8)(c&0x00FF);                                       \
  }                                                                 \
}

#define READ_UTF16LE(zIn, TERM, c){                                   \
  c = (*zIn++);                                                       \
  c += ((*zIn++)<<8);                                                 \
  if( c>=0xD800 && c<0xE000 && TERM ){                                \
    int c2 = (*zIn++);                                                \
    c2 += ((*zIn++)<<8);                                              \
    c = (c2&0x03FF) + ((c&0x003F)<<10) + (((c&0x03C0)+0x0040)<<10);   \
  }                                                                   \
}

#define READ_UTF16BE(zIn, TERM, c){                                   \
  c = ((*zIn++)<<8);                                                  \
  c += (*zIn++);                                                      \
  if( c>=0xD800 && c<0xE000 && TERM ){                                \
    int c2 = ((*zIn++)<<8);                                           \
    c2 += (*zIn++);                                                   \
    c = (c2&0x03FF) + ((c&0x003F)<<10) + (((c&0x03C0)+0x0040)<<10);   \
  }                                                                   \
}

/*
** Translate a single UTF-8 character.  Return the unicode value.
**
** During translation, assume that the byte that zTerm points
** is a 0x00.
**
** Write a pointer to the next unread byte back into *pzNext.
**
** Notes On Invalid UTF-8:
**
**  *  This routine never allows a 7-bit character (0x00 through 0x7f) to
**     be encoded as a multi-byte character.  Any multi-byte character that
**     attempts to encode a value between 0x00 and 0x7f is rendered as 0xfffd.
**
**  *  This routine never allows a UTF16 surrogate value to be encoded.
**     If a multi-byte character attempts to encode a value between
**     0xd800 and 0xe000 then it is rendered as 0xfffd.
**
**  *  Bytes in the range of 0x80 through 0xbf which occur as the first
**     byte of a character are interpreted as single-byte characters
**     and rendered as themselves even though they are technically
**     invalid characters.
**
**  *  This routine accepts over-length UTF8 encodings
**     for unicode values 0x80 and greater.  It does not change over-length
**     encodings to 0xfffd as some systems recommend.
*/
#define READ_UTF8(zIn, zTerm, c)                           \
  c = *(zIn++);                                            \
  if( c>=0xc0 ){                                           \
    c = sqlite3Utf8Trans1[c-0xc0];                         \
    while( zIn!=zTerm && (*zIn & 0xc0)==0x80 ){            \
      c = (c<<6) + (0x3f & *(zIn++));                      \
    }                                                      \
    if( c<0x80                                             \
        || (c&0xFFFFF800)==0xD800                          \
        || (c&0xFFFFFFFE)==0xFFFE ){  c = 0xFFFD; }        \
  }
SQLITE_PRIVATE u32 sqlite3Utf8Read(
  const unsigned char **pz    /* Pointer to string from which to read char */
){
  unsigned int c;

  /* Same as READ_UTF8() above but without the zTerm parameter.
  ** For this routine, we assume the UTF8 string is always zero-terminated.
  */
  c = *((*pz)++);
  if( c>=0xc0 ){
    c = sqlite3Utf8Trans1[c-0xc0];
    while( (*(*pz) & 0xc0)==0x80 ){
      c = (c<<6) + (0x3f & *((*pz)++));
    }
    if( c<0x80
        || (c&0xFFFFF800)==0xD800
        || (c&0xFFFFFFFE)==0xFFFE ){  c = 0xFFFD; }
  }
  return c;
}




/*
** If the TRANSLATE_TRACE macro is defined, the value of each Mem is
** printed on stderr on the way into and out of sqlite3VdbeMemTranslate().
*/ 
/* #define TRANSLATE_TRACE 1 */

#ifndef SQLITE_OMIT_UTF16
/*
** This routine transforms the internal text encoding used by pMem to
** desiredEnc. It is an error if the string is already of the desired
** encoding, or if *pMem does not contain a string value.
*/
SQLITE_PRIVATE SQLITE_NOINLINE int sqlite3VdbeMemTranslate(Mem *pMem, u8 desiredEnc){
  int len;                    /* Maximum length of output string in bytes */
  unsigned char *zOut;                  /* Output buffer */
  unsigned char *zIn;                   /* Input iterator */
  unsigned char *zTerm;                 /* End of input */
  unsigned char *z;                     /* Output iterator */
  unsigned int c;

  assert( pMem->db==0 || sqlite3_mutex_held(pMem->db->mutex) );
  assert( pMem->flags&MEM_Str );
  assert( pMem->enc!=desiredEnc );
  assert( pMem->enc!=0 );
  assert( pMem->n>=0 );

#if defined(TRANSLATE_TRACE) && defined(SQLITE_DEBUG)
  {
    char zBuf[100];
    sqlite3VdbeMemPrettyPrint(pMem, zBuf);
    fprintf(stderr, "INPUT:  %s\n", zBuf);
  }
#endif

  /* If the translation is between UTF-16 little and big endian, then 
  ** all that is required is to swap the byte order. This case is handled
  ** differently from the others.
  */
  if( pMem->enc!=SQLITE_UTF8 && desiredEnc!=SQLITE_UTF8 ){
    u8 temp;
    int rc;
    rc = sqlite3VdbeMemMakeWriteable(pMem);
    if( rc!=SQLITE_OK ){
      assert( rc==SQLITE_NOMEM );
      return SQLITE_NOMEM_BKPT;
    }
    zIn = (u8*)pMem->z;
    zTerm = &zIn[pMem->n&~1];
    while( zIn<zTerm ){
      temp = *zIn;
      *zIn = *(zIn+1);
      zIn++;
      *zIn++ = temp;
    }
    pMem->enc = desiredEnc;
    goto translate_out;
  }

  /* Set len to the maximum number of bytes required in the output buffer. */
  if( desiredEnc==SQLITE_UTF8 ){
    /* When converting from UTF-16, the maximum growth results from
    ** translating a 2-byte character to a 4-byte UTF-8 character.
    ** A single byte is required for the output string
    ** nul-terminator.
    */
    pMem->n &= ~1;
    len = pMem->n * 2 + 1;
  }else{
    /* When converting from UTF-8 to UTF-16 the maximum growth is caused
    ** when a 1-byte UTF-8 character is translated into a 2-byte UTF-16
    ** character. Two bytes are required in the output buffer for the
    ** nul-terminator.
    */
    len = pMem->n * 2 + 2;
  }

  /* Set zIn to point at the start of the input buffer and zTerm to point 1
  ** byte past the end.
  **
  ** Variable zOut is set to point at the output buffer, space obtained
  ** from sqlite3_malloc().
  */
  zIn = (u8*)pMem->z;
  zTerm = &zIn[pMem->n];
  zOut = sqlite3DbMallocRaw(pMem->db, len);
  if( !zOut ){
    return SQLITE_NOMEM_BKPT;
  }
  z = zOut;

  if( pMem->enc==SQLITE_UTF8 ){
    if( desiredEnc==SQLITE_UTF16LE ){
      /* UTF-8 -> UTF-16 Little-endian */
      while( zIn<zTerm ){
        READ_UTF8(zIn, zTerm, c);
        WRITE_UTF16LE(z, c);
      }
    }else{
      assert( desiredEnc==SQLITE_UTF16BE );
      /* UTF-8 -> UTF-16 Big-endian */
      while( zIn<zTerm ){
        READ_UTF8(zIn, zTerm, c);
        WRITE_UTF16BE(z, c);
      }
    }
    pMem->n = (int)(z - zOut);
    *z++ = 0;
  }else{
    assert( desiredEnc==SQLITE_UTF8 );
    if( pMem->enc==SQLITE_UTF16LE ){
      /* UTF-16 Little-endian -> UTF-8 */
      while( zIn<zTerm ){
        READ_UTF16LE(zIn, zIn<zTerm, c); 
        WRITE_UTF8(z, c);
      }
    }else{
      /* UTF-16 Big-endian -> UTF-8 */
      while( zIn<zTerm ){
        READ_UTF16BE(zIn, zIn<zTerm, c); 
        WRITE_UTF8(z, c);
      }
    }
    pMem->n = (int)(z - zOut);
  }
  *z = 0;
  assert( (pMem->n+(desiredEnc==SQLITE_UTF8?1:2))<=len );

  c = pMem->flags;
  sqlite3VdbeMemRelease(pMem);
  pMem->flags = MEM_Str|MEM_Term|(c&(MEM_AffMask|MEM_Subtype));
  pMem->enc = desiredEnc;
  pMem->z = (char*)zOut;
  pMem->zMalloc = pMem->z;
  pMem->szMalloc = sqlite3DbMallocSize(pMem->db, pMem->z);

translate_out:
#if defined(TRANSLATE_TRACE) && defined(SQLITE_DEBUG)
  {
    char zBuf[100];
    sqlite3VdbeMemPrettyPrint(pMem, zBuf);
    fprintf(stderr, "OUTPUT: %s\n", zBuf);
  }
#endif
  return SQLITE_OK;
}

/*
** This routine checks for a byte-order mark at the beginning of the 
** UTF-16 string stored in *pMem. If one is present, it is removed and
** the encoding of the Mem adjusted. This routine does not do any
** byte-swapping, it just sets Mem.enc appropriately.
**
** The allocation (static, dynamic etc.) and encoding of the Mem may be
** changed by this function.
*/
SQLITE_PRIVATE int sqlite3VdbeMemHandleBom(Mem *pMem){
  int rc = SQLITE_OK;
  u8 bom = 0;

  assert( pMem->n>=0 );
  if( pMem->n>1 ){
    u8 b1 = *(u8 *)pMem->z;
    u8 b2 = *(((u8 *)pMem->z) + 1);
    if( b1==0xFE && b2==0xFF ){
      bom = SQLITE_UTF16BE;
    }
    if( b1==0xFF && b2==0xFE ){
      bom = SQLITE_UTF16LE;
    }
  }
  
  if( bom ){
    rc = sqlite3VdbeMemMakeWriteable(pMem);
    if( rc==SQLITE_OK ){
      pMem->n -= 2;
      memmove(pMem->z, &pMem->z[2], pMem->n);
      pMem->z[pMem->n] = '\0';
      pMem->z[pMem->n+1] = '\0';
      pMem->flags |= MEM_Term;
      pMem->enc = bom;
    }
  }
  return rc;
}
#endif /* SQLITE_OMIT_UTF16 */

/*
** pZ is a UTF-8 encoded unicode string. If nByte is less than zero,
** return the number of unicode characters in pZ up to (but not including)
** the first 0x00 byte. If nByte is not less than zero, return the
** number of unicode characters in the first nByte of pZ (or up to 
** the first 0x00, whichever comes first).
*/
SQLITE_PRIVATE int sqlite3Utf8CharLen(const char *zIn, int nByte){
  int r = 0;
  const u8 *z = (const u8*)zIn;
  const u8 *zTerm;
  if( nByte>=0 ){
    zTerm = &z[nByte];
  }else{
    zTerm = (const u8*)(-1);
  }
  assert( z<=zTerm );
  while( *z!=0 && z<zTerm ){
    SQLITE_SKIP_UTF8(z);
    r++;
  }
  return r;
}

/* This test function is not currently used by the automated test-suite. 
** Hence it is only available in debug builds.
*/
#if defined(SQLITE_TEST) && defined(SQLITE_DEBUG)
/*
** Translate UTF-8 to UTF-8.
**
** This has the effect of making sure that the string is well-formed
** UTF-8.  Miscoded characters are removed.
**
** The translation is done in-place and aborted if the output
** overruns the input.
*/
SQLITE_PRIVATE int sqlite3Utf8To8(unsigned char *zIn){
  unsigned char *zOut = zIn;
  unsigned char *zStart = zIn;
  u32 c;

  while( zIn[0] && zOut<=zIn ){
    c = sqlite3Utf8Read((const u8**)&zIn);
    if( c!=0xfffd ){
      WRITE_UTF8(zOut, c);
    }
  }
  *zOut = 0;
  return (int)(zOut - zStart);
}
#endif

#ifndef SQLITE_OMIT_UTF16
/*
** Convert a UTF-16 string in the native encoding into a UTF-8 string.
** Memory to hold the UTF-8 string is obtained from sqlite3_malloc and must
** be freed by the calling function.
**
** NULL is returned if there is an allocation error.
*/
SQLITE_PRIVATE char *sqlite3Utf16to8(sqlite3 *db, const void *z, int nByte, u8 enc){
  Mem m;
  memset(&m, 0, sizeof(m));
  m.db = db;
  sqlite3VdbeMemSetStr(&m, z, nByte, enc, SQLITE_STATIC);
  sqlite3VdbeChangeEncoding(&m, SQLITE_UTF8);
  if( db->mallocFailed ){
    sqlite3VdbeMemRelease(&m);
    m.z = 0;
  }
  assert( (m.flags & MEM_Term)!=0 || db->mallocFailed );
  assert( (m.flags & MEM_Str)!=0 || db->mallocFailed );
  assert( m.z || db->mallocFailed );
  return m.z;
}

/*
** zIn is a UTF-16 encoded unicode string at least nChar characters long.
** Return the number of bytes in the first nChar unicode characters
** in pZ.  nChar must be non-negative.
*/
SQLITE_PRIVATE int sqlite3Utf16ByteLen(const void *zIn, int nChar){
  int c;
  unsigned char const *z = zIn;
  int n = 0;
  
  if( SQLITE_UTF16NATIVE==SQLITE_UTF16BE ){
    while( n<nChar ){
      READ_UTF16BE(z, 1, c);
      n++;
    }
  }else{
    while( n<nChar ){
      READ_UTF16LE(z, 1, c);
      n++;
    }
  }
  return (int)(z-(unsigned char const *)zIn);
}

#if defined(SQLITE_TEST)
/*
** This routine is called from the TCL test function "translate_selftest".
** It checks that the primitives for serializing and deserializing
** characters in each encoding are inverses of each other.
*/
SQLITE_PRIVATE void sqlite3UtfSelfTest(void){
  unsigned int i, t;
  unsigned char zBuf[20];
  unsigned char *z;
  int n;
  unsigned int c;

  for(i=0; i<0x00110000; i++){
    z = zBuf;
    WRITE_UTF8(z, i);
    n = (int)(z-zBuf);
    assert( n>0 && n<=4 );
    z[0] = 0;
    z = zBuf;
    c = sqlite3Utf8Read((const u8**)&z);
    t = i;
    if( i>=0xD800 && i<=0xDFFF ) t = 0xFFFD;
    if( (i&0xFFFFFFFE)==0xFFFE ) t = 0xFFFD;
    assert( c==t );
    assert( (z-zBuf)==n );
  }
  for(i=0; i<0x00110000; i++){
    if( i>=0xD800 && i<0xE000 ) continue;
    z = zBuf;
    WRITE_UTF16LE(z, i);
    n = (int)(z-zBuf);
    assert( n>0 && n<=4 );
    z[0] = 0;
    z = zBuf;
    READ_UTF16LE(z, 1, c);
    assert( c==i );
    assert( (z-zBuf)==n );
  }
  for(i=0; i<0x00110000; i++){
    if( i>=0xD800 && i<0xE000 ) continue;
    z = zBuf;
    WRITE_UTF16BE(z, i);
    n = (int)(z-zBuf);
    assert( n>0 && n<=4 );
    z[0] = 0;
    z = zBuf;
    READ_UTF16BE(z, 1, c);
    assert( c==i );
    assert( (z-zBuf)==n );
  }
}
#endif /* SQLITE_TEST */
#endif /* SQLITE_OMIT_UTF16 */

/************** End of utf.c *************************************************/
/************** Begin file util.c ********************************************/
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
** Utility functions used throughout sqlite.
**
** This file contains functions for allocating memory, comparing
** strings, and stuff like that.
**
*/
/* #include "sqliteInt.h" */
/* #include <stdarg.h> */
#if HAVE_ISNAN || SQLITE_HAVE_ISNAN
# include <math.h>
#endif

/*
** Routine needed to support the testcase() macro.
*/
#ifdef SQLITE_COVERAGE_TEST
SQLITE_PRIVATE void sqlite3Coverage(int x){
  static unsigned dummy = 0;
  dummy += (unsigned)x;
}
#endif

/*
** Give a callback to the test harness that can be used to simulate faults
** in places where it is difficult or expensive to do so purely by means
** of inputs.
**
** The intent of the integer argument is to let the fault simulator know
** which of multiple sqlite3FaultSim() calls has been hit.
**
** Return whatever integer value the test callback returns, or return
** SQLITE_OK if no test callback is installed.
*/
#ifndef SQLITE_OMIT_BUILTIN_TEST
SQLITE_PRIVATE int sqlite3FaultSim(int iTest){
  int (*xCallback)(int) = sqlite3GlobalConfig.xTestCallback;
  return xCallback ? xCallback(iTest) : SQLITE_OK;
}
#endif

#ifndef SQLITE_OMIT_FLOATING_POINT
/*
** Return true if the floating point value is Not a Number (NaN).
**
** Use the math library isnan() function if compiled with SQLITE_HAVE_ISNAN.
** Otherwise, we have our own implementation that works on most systems.
*/
SQLITE_PRIVATE int sqlite3IsNaN(double x){
  int rc;   /* The value return */
#if !SQLITE_HAVE_ISNAN && !HAVE_ISNAN
  /*
  ** Systems that support the isnan() library function should probably
  ** make use of it by compiling with -DSQLITE_HAVE_ISNAN.  But we have
  ** found that many systems do not have a working isnan() function so
  ** this implementation is provided as an alternative.
  **
  ** This NaN test sometimes fails if compiled on GCC with -ffast-math.
  ** On the other hand, the use of -ffast-math comes with the following
  ** warning:
  **
  **      This option [-ffast-math] should never be turned on by any
  **      -O option since it can result in incorrect output for programs
  **      which depend on an exact implementation of IEEE or ISO 
  **      rules/specifications for math functions.
  **
  ** Under MSVC, this NaN test may fail if compiled with a floating-
  ** point precision mode other than /fp:precise.  From the MSDN 
  ** documentation:
  **
  **      The compiler [with /fp:precise] will properly handle comparisons 
  **      involving NaN. For example, x != x evaluates to true if x is NaN 
  **      ...
  */
#ifdef __FAST_MATH__
# error SQLite will not work correctly with the -ffast-math option of GCC.
#endif
  volatile double y = x;
  volatile double z = y;
  rc = (y!=z);
#else  /* if HAVE_ISNAN */
  rc = isnan(x);
#endif /* HAVE_ISNAN */
  testcase( rc );
  return rc;
}
#endif /* SQLITE_OMIT_FLOATING_POINT */

/*
** Compute a string length that is limited to what can be stored in
** lower 30 bits of a 32-bit signed integer.
**
** The value returned will never be negative.  Nor will it ever be greater
** than the actual length of the string.  For very long strings (greater
** than 1GiB) the value returned might be less than the true string length.
*/
SQLITE_PRIVATE int sqlite3Strlen30(const char *z){
  if( z==0 ) return 0;
  return 0x3fffffff & (int)strlen(z);
}

/*
** Return the declared type of a column.  Or return zDflt if the column 
** has no declared type.
**
** The column type is an extra string stored after the zero-terminator on
** the column name if and only if the COLFLAG_HASTYPE flag is set.
*/
SQLITE_PRIVATE char *sqlite3ColumnType(Column *pCol, char *zDflt){
  if( (pCol->colFlags & COLFLAG_HASTYPE)==0 ) return zDflt;
  return pCol->zName + strlen(pCol->zName) + 1;
}

/*
** Helper function for sqlite3Error() - called rarely.  Broken out into
** a separate routine to avoid unnecessary register saves on entry to
** sqlite3Error().
*/
static SQLITE_NOINLINE void  sqlite3ErrorFinish(sqlite3 *db, int err_code){
  if( db->pErr ) sqlite3ValueSetNull(db->pErr);
  sqlite3SystemError(db, err_code);
}

/*
** Set the current error code to err_code and clear any prior error message.
** Also set iSysErrno (by calling sqlite3System) if the err_code indicates
** that would be appropriate.
*/
SQLITE_PRIVATE void sqlite3Error(sqlite3 *db, int err_code){
  assert( db!=0 );
  db->errCode = err_code;
  if( err_code || db->pErr ) sqlite3ErrorFinish(db, err_code);
}

/*
** Load the sqlite3.iSysErrno field if that is an appropriate thing
** to do based on the SQLite error code in rc.
*/
SQLITE_PRIVATE void sqlite3SystemError(sqlite3 *db, int rc){
  if( rc==SQLITE_IOERR_NOMEM ) return;
  rc &= 0xff;
  if( rc==SQLITE_CANTOPEN || rc==SQLITE_IOERR ){
    db->iSysErrno = sqlite3OsGetLastError(db->pVfs);
  }
}

/*
** Set the most recent error code and error string for the sqlite
** handle "db". The error code is set to "err_code".
**
** If it is not NULL, string zFormat specifies the format of the
** error string in the style of the printf functions: The following
** format characters are allowed:
**
**      %s      Insert a string
**      %z      A string that should be freed after use
**      %d      Insert an integer
**      %T      Insert a token
**      %S      Insert the first element of a SrcList
**
** zFormat and any string tokens that follow it are assumed to be
** encoded in UTF-8.
**
** To clear the most recent error for sqlite handle "db", sqlite3Error
** should be called with err_code set to SQLITE_OK and zFormat set
** to NULL.
*/
SQLITE_PRIVATE void sqlite3ErrorWithMsg(sqlite3 *db, int err_code, const char *zFormat, ...){
  assert( db!=0 );
  db->errCode = err_code;
  sqlite3SystemError(db, err_code);
  if( zFormat==0 ){
    sqlite3Error(db, err_code);
  }else if( db->pErr || (db->pErr = sqlite3ValueNew(db))!=0 ){
    char *z;
    va_list ap;
    va_start(ap, zFormat);
    z = sqlite3VMPrintf(db, zFormat, ap);
    va_end(ap);
    sqlite3ValueSetStr(db->pErr, -1, z, SQLITE_UTF8, SQLITE_DYNAMIC);
  }
}

/*
** Add an error message to pParse->zErrMsg and increment pParse->nErr.
** The following formatting characters are allowed:
**
**      %s      Insert a string
**      %z      A string that should be freed after use
**      %d      Insert an integer
**      %T      Insert a token
**      %S      Insert the first element of a SrcList
**
** This function should be used to report any error that occurs while
** compiling an SQL statement (i.e. within sqlite3_prepare()). The
** last thing the sqlite3_prepare() function does is copy the error
** stored by this function into the database handle using sqlite3Error().
** Functions sqlite3Error() or sqlite3ErrorWithMsg() should be used
** during statement execution (sqlite3_step() etc.).
*/
SQLITE_PRIVATE void sqlite3ErrorMsg(Parse *pParse, const char *zFormat, ...){
  char *zMsg;
  va_list ap;
  sqlite3 *db = pParse->db;
  va_start(ap, zFormat);
  zMsg = sqlite3VMPrintf(db, zFormat, ap);
  va_end(ap);
  if( db->suppressErr ){
    sqlite3DbFree(db, zMsg);
  }else{
    pParse->nErr++;
    sqlite3DbFree(db, pParse->zErrMsg);
    pParse->zErrMsg = zMsg;
    pParse->rc = SQLITE_ERROR;
  }
}

/*
** Convert an SQL-style quoted string into a normal string by removing
** the quote characters.  The conversion is done in-place.  If the
** input does not begin with a quote character, then this routine
** is a no-op.
**
** The input string must be zero-terminated.  A new zero-terminator
** is added to the dequoted string.
**
** The return value is -1 if no dequoting occurs or the length of the
** dequoted string, exclusive of the zero terminator, if dequoting does
** occur.
**
** 2002-Feb-14: This routine is extended to remove MS-Access style
** brackets from around identifiers.  For example:  "[a-b-c]" becomes
** "a-b-c".
*/
SQLITE_PRIVATE void sqlite3Dequote(char *z){
  char quote;
  int i, j;
  if( z==0 ) return;
  quote = z[0];
  if( !sqlite3Isquote(quote) ) return;
  if( quote=='[' ) quote = ']';
  for(i=1, j=0;; i++){
    assert( z[i] );
    if( z[i]==quote ){
      if( z[i+1]==quote ){
        z[j++] = quote;
        i++;
      }else{
        break;
      }
    }else{
      z[j++] = z[i];
    }
  }
  z[j] = 0;
}

/*
** Generate a Token object from a string
*/
SQLITE_PRIVATE void sqlite3TokenInit(Token *p, char *z){
  p->z = z;
  p->n = sqlite3Strlen30(z);
}

/* Convenient short-hand */
#define UpperToLower sqlite3UpperToLower

/*
** Some systems have stricmp().  Others have strcasecmp().  Because
** there is no consistency, we will define our own.
**
** IMPLEMENTATION-OF: R-30243-02494 The sqlite3_stricmp() and
** sqlite3_strnicmp() APIs allow applications and extensions to compare
** the contents of two buffers containing UTF-8 strings in a
** case-independent fashion, using the same definition of "case
** independence" that SQLite uses internally when comparing identifiers.
*/
SQLITE_API int SQLITE_STDCALL sqlite3_stricmp(const char *zLeft, const char *zRight){
  if( zLeft==0 ){
    return zRight ? -1 : 0;
  }else if( zRight==0 ){
    return 1;
  }
  return sqlite3StrICmp(zLeft, zRight);
}
SQLITE_PRIVATE int sqlite3StrICmp(const char *zLeft, const char *zRight){
  unsigned char *a, *b;
  int c;
  a = (unsigned char *)zLeft;
  b = (unsigned char *)zRight;
  for(;;){
    c = (int)UpperToLower[*a] - (int)UpperToLower[*b];
    if( c || *a==0 ) break;
    a++;
    b++;
  }
  return c;
}
SQLITE_API int SQLITE_STDCALL sqlite3_strnicmp(const char *zLeft, const char *zRight, int N){
  register unsigned char *a, *b;
  if( zLeft==0 ){
    return zRight ? -1 : 0;
  }else if( zRight==0 ){
    return 1;
  }
  a = (unsigned char *)zLeft;
  b = (unsigned char *)zRight;
  while( N-- > 0 && *a!=0 && UpperToLower[*a]==UpperToLower[*b]){ a++; b++; }
  return N<0 ? 0 : UpperToLower[*a] - UpperToLower[*b];
}

/*
** The string z[] is an text representation of a real number.
** Convert this string to a double and write it into *pResult.
**
** The string z[] is length bytes in length (bytes, not characters) and
** uses the encoding enc.  The string is not necessarily zero-terminated.
**
** Return TRUE if the result is a valid real number (or integer) and FALSE
** if the string is empty or contains extraneous text.  Valid numbers
** are in one of these formats:
**
**    [+-]digits[E[+-]digits]
**    [+-]digits.[digits][E[+-]digits]
**    [+-].digits[E[+-]digits]
**
** Leading and trailing whitespace is ignored for the purpose of determining
** validity.
**
** If some prefix of the input string is a valid number, this routine
** returns FALSE but it still converts the prefix and writes the result
** into *pResult.
*/
SQLITE_PRIVATE int sqlite3AtoF(const char *z, double *pResult, int length, u8 enc){
#ifndef SQLITE_OMIT_FLOATING_POINT
  int incr;
  const char *zEnd = z + length;
  /* sign * significand * (10 ^ (esign * exponent)) */
  int sign = 1;    /* sign of significand */
  i64 s = 0;       /* significand */
  int d = 0;       /* adjust exponent for shifting decimal point */
  int esign = 1;   /* sign of exponent */
  int e = 0;       /* exponent */
  int eValid = 1;  /* True exponent is either not used or is well-formed */
  double result;
  int nDigits = 0;
  int nonNum = 0;  /* True if input contains UTF16 with high byte non-zero */

  assert( enc==SQLITE_UTF8 || enc==SQLITE_UTF16LE || enc==SQLITE_UTF16BE );
  *pResult = 0.0;   /* Default return value, in case of an error */

  if( enc==SQLITE_UTF8 ){
    incr = 1;
  }else{
    int i;
    incr = 2;
    assert( SQLITE_UTF16LE==2 && SQLITE_UTF16BE==3 );
    for(i=3-enc; i<length && z[i]==0; i+=2){}
    nonNum = i<length;
    zEnd = &z[i^1];
    z += (enc&1);
  }

  /* skip leading spaces */
  while( z<zEnd && sqlite3Isspace(*z) ) z+=incr;
  if( z>=zEnd ) return 0;

  /* get sign of significand */
  if( *z=='-' ){
    sign = -1;
    z+=incr;
  }else if( *z=='+' ){
    z+=incr;
  }

  /* copy max significant digits to significand */
  while( z<zEnd && sqlite3Isdigit(*z) && s<((LARGEST_INT64-9)/10) ){
    s = s*10 + (*z - '0');
    z+=incr, nDigits++;
  }

  /* skip non-significant significand digits
  ** (increase exponent by d to shift decimal left) */
  while( z<zEnd && sqlite3Isdigit(*z) ) z+=incr, nDigits++, d++;
  if( z>=zEnd ) goto do_atof_calc;

  /* if decimal point is present */
  if( *z=='.' ){
    z+=incr;
    /* copy digits from after decimal to significand
    ** (decrease exponent by d to shift decimal right) */
    while( z<zEnd && sqlite3Isdigit(*z) ){
      if( s<((LARGEST_INT64-9)/10) ){
        s = s*10 + (*z - '0');
        d--;
      }
      z+=incr, nDigits++;
    }
  }
  if( z>=zEnd ) goto do_atof_calc;

  /* if exponent is present */
  if( *z=='e' || *z=='E' ){
    z+=incr;
    eValid = 0;

    /* This branch is needed to avoid a (harmless) buffer overread.  The 
    ** special comment alerts the mutation tester that the correct answer
    ** is obtained even if the branch is omitted */
    if( z>=zEnd ) goto do_atof_calc;              /*PREVENTS-HARMLESS-OVERREAD*/

    /* get sign of exponent */
    if( *z=='-' ){
      esign = -1;
      z+=incr;
    }else if( *z=='+' ){
      z+=incr;
    }
    /* copy digits to exponent */
    while( z<zEnd && sqlite3Isdigit(*z) ){
      e = e<10000 ? (e*10 + (*z - '0')) : 10000;
      z+=incr;
      eValid = 1;
    }
  }

  /* skip trailing spaces */
  while( z<zEnd && sqlite3Isspace(*z) ) z+=incr;

do_atof_calc:
  /* adjust exponent by d, and update sign */
  e = (e*esign) + d;
  if( e<0 ) {
    esign = -1;
    e *= -1;
  } else {
    esign = 1;
  }

  if( s==0 ) {
    /* In the IEEE 754 standard, zero is signed. */
    result = sign<0 ? -(double)0 : (double)0;
  } else {
    /* Attempt to reduce exponent.
    **
    ** Branches that are not required for the correct answer but which only
    ** help to obtain the correct answer faster are marked with special
    ** comments, as a hint to the mutation tester.
    */
    while( e>0 ){                                       /*OPTIMIZATION-IF-TRUE*/
      if( esign>0 ){
        if( s>=(LARGEST_INT64/10) ) break;             /*OPTIMIZATION-IF-FALSE*/
        s *= 10;
      }else{
        if( s%10!=0 ) break;                           /*OPTIMIZATION-IF-FALSE*/
        s /= 10;
      }
      e--;
    }

    /* adjust the sign of significand */
    s = sign<0 ? -s : s;

    if( e==0 ){                                         /*OPTIMIZATION-IF-TRUE*/
      result = (double)s;
    }else{
      LONGDOUBLE_TYPE scale = 1.0;
      /* attempt to handle extremely small/large numbers better */
      if( e>307 ){                                      /*OPTIMIZATION-IF-TRUE*/
        if( e<342 ){                                    /*OPTIMIZATION-IF-TRUE*/
          while( e%308 ) { scale *= 1.0e+1; e -= 1; }
          if( esign<0 ){
            result = s / scale;
            result /= 1.0e+308;
          }else{
            result = s * scale;
            result *= 1.0e+308;
          }
        }else{ assert( e>=342 );
          if( esign<0 ){
            result = 0.0*s;
          }else{
            result = 1e308*1e308*s;  /* Infinity */
          }
        }
      }else{
        /* 1.0e+22 is the largest power of 10 than can be 
        ** represented exactly. */
        while( e%22 ) { scale *= 1.0e+1; e -= 1; }
        while( e>0 ) { scale *= 1.0e+22; e -= 22; }
        if( esign<0 ){
          result = s / scale;
        }else{
          result = s * scale;
        }
      }
    }
  }

  /* store the result */
  *pResult = result;

  /* return true if number and no extra non-whitespace chracters after */
  return z==zEnd && nDigits>0 && eValid && nonNum==0;
#else
  return !sqlite3Atoi64(z, pResult, length, enc);
#endif /* SQLITE_OMIT_FLOATING_POINT */
}

/*
** Compare the 19-character string zNum against the text representation
** value 2^63:  9223372036854775808.  Return negative, zero, or positive
** if zNum is less than, equal to, or greater than the string.
** Note that zNum must contain exactly 19 characters.
**
** Unlike memcmp() this routine is guaranteed to return the difference
** in the values of the last digit if the only difference is in the
** last digit.  So, for example,
**
**      compare2pow63("9223372036854775800", 1)
**
** will return -8.
*/
static int compare2pow63(const char *zNum, int incr){
  int c = 0;
  int i;
                    /* 012345678901234567 */
  const char *pow63 = "922337203685477580";
  for(i=0; c==0 && i<18; i++){
    c = (zNum[i*incr]-pow63[i])*10;
  }
  if( c==0 ){
    c = zNum[18*incr] - '8';
    testcase( c==(-1) );
    testcase( c==0 );
    testcase( c==(+1) );
  }
  return c;
}

/*
** Convert zNum to a 64-bit signed integer.  zNum must be decimal. This
** routine does *not* accept hexadecimal notation.
**
** If the zNum value is representable as a 64-bit twos-complement 
** integer, then write that value into *pNum and return 0.
**
** If zNum is exactly 9223372036854775808, return 2.  This special
** case is broken out because while 9223372036854775808 cannot be a 
** signed 64-bit integer, its negative -9223372036854775808 can be.
**
** If zNum is too big for a 64-bit integer and is not
** 9223372036854775808  or if zNum contains any non-numeric text,
** then return 1.
**
** length is the number of bytes in the string (bytes, not characters).
** The string is not necessarily zero-terminated.  The encoding is
** given by enc.
*/
SQLITE_PRIVATE int sqlite3Atoi64(const char *zNum, i64 *pNum, int length, u8 enc){
  int incr;
  u64 u = 0;
  int neg = 0; /* assume positive */
  int i;
  int c = 0;
  int nonNum = 0;  /* True if input contains UTF16 with high byte non-zero */
  const char *zStart;
  const char *zEnd = zNum + length;
  assert( enc==SQLITE_UTF8 || enc==SQLITE_UTF16LE || enc==SQLITE_UTF16BE );
  if( enc==SQLITE_UTF8 ){
    incr = 1;
  }else{
    incr = 2;
    assert( SQLITE_UTF16LE==2 && SQLITE_UTF16BE==3 );
    for(i=3-enc; i<length && zNum[i]==0; i+=2){}
    nonNum = i<length;
    zEnd = &zNum[i^1];
    zNum += (enc&1);
  }
  while( zNum<zEnd && sqlite3Isspace(*zNum) ) zNum+=incr;
  if( zNum<zEnd ){
    if( *zNum=='-' ){
      neg = 1;
      zNum+=incr;
    }else if( *zNum=='+' ){
      zNum+=incr;
    }
  }
  zStart = zNum;
  while( zNum<zEnd && zNum[0]=='0' ){ zNum+=incr; } /* Skip leading zeros. */
  for(i=0; &zNum[i]<zEnd && (c=zNum[i])>='0' && c<='9'; i+=incr){
    u = u*10 + c - '0';
  }
  if( u>LARGEST_INT64 ){
    *pNum = neg ? SMALLEST_INT64 : LARGEST_INT64;
  }else if( neg ){
    *pNum = -(i64)u;
  }else{
    *pNum = (i64)u;
  }
  testcase( i==18 );
  testcase( i==19 );
  testcase( i==20 );
  if( &zNum[i]<zEnd              /* Extra bytes at the end */
   || (i==0 && zStart==zNum)     /* No digits */
   || i>19*incr                  /* Too many digits */
   || nonNum                     /* UTF16 with high-order bytes non-zero */
  ){
    /* zNum is empty or contains non-numeric text or is longer
    ** than 19 digits (thus guaranteeing that it is too large) */
    return 1;
  }else if( i<19*incr ){
    /* Less than 19 digits, so we know that it fits in 64 bits */
    assert( u<=LARGEST_INT64 );
    return 0;
  }else{
    /* zNum is a 19-digit numbers.  Compare it against 9223372036854775808. */
    c = compare2pow63(zNum, incr);
    if( c<0 ){
      /* zNum is less than 9223372036854775808 so it fits */
      assert( u<=LARGEST_INT64 );
      return 0;
    }else if( c>0 ){
      /* zNum is greater than 9223372036854775808 so it overflows */
      return 1;
    }else{
      /* zNum is exactly 9223372036854775808.  Fits if negative.  The
      ** special case 2 overflow if positive */
      assert( u-1==LARGEST_INT64 );
      return neg ? 0 : 2;
    }
  }
}

/*
** Transform a UTF-8 integer literal, in either decimal or hexadecimal,
** into a 64-bit signed integer.  This routine accepts hexadecimal literals,
** whereas sqlite3Atoi64() does not.
**
** Returns:
**
**     0    Successful transformation.  Fits in a 64-bit signed integer.
**     1    Integer too large for a 64-bit signed integer or is malformed
**     2    Special case of 9223372036854775808
*/
SQLITE_PRIVATE int sqlite3DecOrHexToI64(const char *z, i64 *pOut){
#ifndef SQLITE_OMIT_HEX_INTEGER
  if( z[0]=='0'
   && (z[1]=='x' || z[1]=='X')
  ){
    u64 u = 0;
    int i, k;
    for(i=2; z[i]=='0'; i++){}
    for(k=i; sqlite3Isxdigit(z[k]); k++){
      u = u*16 + sqlite3HexToInt(z[k]);
    }
    memcpy(pOut, &u, 8);
    return (z[k]==0 && k-i<=16) ? 0 : 1;
  }else
#endif /* SQLITE_OMIT_HEX_INTEGER */
  {
    return sqlite3Atoi64(z, pOut, sqlite3Strlen30(z), SQLITE_UTF8);
  }
}

/*
** If zNum represents an integer that will fit in 32-bits, then set
** *pValue to that integer and return true.  Otherwise return false.
**
** This routine accepts both decimal and hexadecimal notation for integers.
**
** Any non-numeric characters that following zNum are ignored.
** This is different from sqlite3Atoi64() which requires the
** input number to be zero-terminated.
*/
SQLITE_PRIVATE int sqlite3GetInt32(const char *zNum, int *pValue){
  sqlite_int64 v = 0;
  int i, c;
  int neg = 0;
  if( zNum[0]=='-' ){
    neg = 1;
    zNum++;
  }else if( zNum[0]=='+' ){
    zNum++;
  }
#ifndef SQLITE_OMIT_HEX_INTEGER
  else if( zNum[0]=='0'
        && (zNum[1]=='x' || zNum[1]=='X')
        && sqlite3Isxdigit(zNum[2])
  ){
    u32 u = 0;
    zNum += 2;
    while( zNum[0]=='0' ) zNum++;
    for(i=0; sqlite3Isxdigit(zNum[i]) && i<8; i++){
      u = u*16 + sqlite3HexToInt(zNum[i]);
    }
    if( (u&0x80000000)==0 && sqlite3Isxdigit(zNum[i])==0 ){
      memcpy(pValue, &u, 4);
      return 1;
    }else{
      return 0;
    }
  }
#endif
  while( zNum[0]=='0' ) zNum++;
  for(i=0; i<11 && (c = zNum[i] - '0')>=0 && c<=9; i++){
    v = v*10 + c;
  }

  /* The longest decimal representation of a 32 bit integer is 10 digits:
  **
  **             1234567890
  **     2^31 -> 2147483648
  */
  testcase( i==10 );
  if( i>10 ){
    return 0;
  }
  testcase( v-neg==2147483647 );
  if( v-neg>2147483647 ){
    return 0;
  }
  if( neg ){
    v = -v;
  }
  *pValue = (int)v;
  return 1;
}

/*
** Return a 32-bit integer value extracted from a string.  If the
** string is not an integer, just return 0.
*/
SQLITE_PRIVATE int sqlite3Atoi(const char *z){
  int x = 0;
  if( z ) sqlite3GetInt32(z, &x);
  return x;
}

/*
** The variable-length integer encoding is as follows:
**
** KEY:
**         A = 0xxxxxxx    7 bits of data and one flag bit
**         B = 1xxxxxxx    7 bits of data and one flag bit
**         C = xxxxxxxx    8 bits of data
**
**  7 bits - A
** 14 bits - BA
** 21 bits - BBA
** 28 bits - BBBA
** 35 bits - BBBBA
** 42 bits - BBBBBA
** 49 bits - BBBBBBA
** 56 bits - BBBBBBBA
** 64 bits - BBBBBBBBC
*/

/*
** Write a 64-bit variable-length integer to memory starting at p[0].
** The length of data write will be between 1 and 9 bytes.  The number
** of bytes written is returned.
**
** A variable-length integer consists of the lower 7 bits of each byte
** for all bytes that have the 8th bit set and one byte with the 8th
** bit clear.  Except, if we get to the 9th byte, it stores the full
** 8 bits and is the last byte.
*/
static int SQLITE_NOINLINE putVarint64(unsigned char *p, u64 v){
  int i, j, n;
  u8 buf[10];
  if( v & (((u64)0xff000000)<<32) ){
    p[8] = (u8)v;
    v >>= 8;
    for(i=7; i>=0; i--){
      p[i] = (u8)((v & 0x7f) | 0x80);
      v >>= 7;
    }
    return 9;
  }    
  n = 0;
  do{
    buf[n++] = (u8)((v & 0x7f) | 0x80);
    v >>= 7;
  }while( v!=0 );
  buf[0] &= 0x7f;
  assert( n<=9 );
  for(i=0, j=n-1; j>=0; j--, i++){
    p[i] = buf[j];
  }
  return n;
}
SQLITE_PRIVATE int sqlite3PutVarint(unsigned char *p, u64 v){
  if( v<=0x7f ){
    p[0] = v&0x7f;
    return 1;
  }
  if( v<=0x3fff ){
    p[0] = ((v>>7)&0x7f)|0x80;
    p[1] = v&0x7f;
    return 2;
  }
  return putVarint64(p,v);
}

/*
** Bitmasks used by sqlite3GetVarint().  These precomputed constants
** are defined here rather than simply putting the constant expressions
** inline in order to work around bugs in the RVT compiler.
**
** SLOT_2_0     A mask for  (0x7f<<14) | 0x7f
**
** SLOT_4_2_0   A mask for  (0x7f<<28) | SLOT_2_0
*/
#define SLOT_2_0     0x001fc07f
#define SLOT_4_2_0   0xf01fc07f


/*
** Read a 64-bit variable-length integer from memory starting at p[0].
** Return the number of bytes read.  The value is stored in *v.
*/
SQLITE_PRIVATE u8 sqlite3GetVarint(const unsigned char *p, u64 *v){
  u32 a,b,s;

  a = *p;
  /* a: p0 (unmasked) */
  if (!(a&0x80))
  {
    *v = a;
    return 1;
  }

  p++;
  b = *p;
  /* b: p1 (unmasked) */
  if (!(b&0x80))
  {
    a &= 0x7f;
    a = a<<7;
    a |= b;
    *v = a;
    return 2;
  }

  /* Verify that constants are precomputed correctly */
  assert( SLOT_2_0 == ((0x7f<<14) | (0x7f)) );
  assert( SLOT_4_2_0 == ((0xfU<<28) | (0x7f<<14) | (0x7f)) );

  p++;
  a = a<<14;
  a |= *p;
  /* a: p0<<14 | p2 (unmasked) */
  if (!(a&0x80))
  {
    a &= SLOT_2_0;
    b &= 0x7f;
    b = b<<7;
    a |= b;
    *v = a;
    return 3;
  }

  /* CSE1 from below */
  a &= SLOT_2_0;
  p++;
  b = b<<14;
  b |= *p;
  /* b: p1<<14 | p3 (unmasked) */
  if (!(b&0x80))
  {
    b &= SLOT_2_0;
    /* moved CSE1 up */
    /* a &= (0x7f<<14)|(0x7f); */
    a = a<<7;
    a |= b;
    *v = a;
    return 4;
  }

  /* a: p0<<14 | p2 (masked) */
  /* b: p1<<14 | p3 (unmasked) */
  /* 1:save off p0<<21 | p1<<14 | p2<<7 | p3 (masked) */
  /* moved CSE1 up */
  /* a &= (0x7f<<14)|(0x7f); */
  b &= SLOT_2_0;
  s = a;
  /* s: p0<<14 | p2 (masked) */

  p++;
  a = a<<14;
  a |= *p;
  /* a: p0<<28 | p2<<14 | p4 (unmasked) */
  if (!(a&0x80))
  {
    /* we can skip these cause they were (effectively) done above
    ** while calculating s */
    /* a &= (0x7f<<28)|(0x7f<<14)|(0x7f); */
    /* b &= (0x7f<<14)|(0x7f); */
    b = b<<7;
    a |= b;
    s = s>>18;
    *v = ((u64)s)<<32 | a;
    return 5;
  }

  /* 2:save off p0<<21 | p1<<14 | p2<<7 | p3 (masked) */
  s = s<<7;
  s |= b;
  /* s: p0<<21 | p1<<14 | p2<<7 | p3 (masked) */

  p++;
  b = b<<14;
  b |= *p;
  /* b: p1<<28 | p3<<14 | p5 (unmasked) */
  if (!(b&0x80))
  {
    /* we can skip this cause it was (effectively) done above in calc'ing s */
    /* b &= (0x7f<<28)|(0x7f<<14)|(0x7f); */
    a &= SLOT_2_0;
    a = a<<7;
    a |= b;
    s = s>>18;
    *v = ((u64)s)<<32 | a;
    return 6;
  }

  p++;
  a = a<<14;
  a |= *p;
  /* a: p2<<28 | p4<<14 | p6 (unmasked) */
  if (!(a&0x80))
  {
    a &= SLOT_4_2_0;
    b &= SLOT_2_0;
    b = b<<7;
    a |= b;
    s = s>>11;
    *v = ((u64)s)<<32 | a;
    return 7;
  }

  /* CSE2 from below */
  a &= SLOT_2_0;
  p++;
  b = b<<14;
  b |= *p;
  /* b: p3<<28 | p5<<14 | p7 (unmasked) */
  if (!(b&0x80))
  {
    b &= SLOT_4_2_0;
    /* moved CSE2 up */
    /* a &= (0x7f<<14)|(0x7f); */
    a = a<<7;
    a |= b;
    s = s>>4;
    *v = ((u64)s)<<32 | a;
    return 8;
  }

  p++;
  a = a<<15;
  a |= *p;
  /* a: p4<<29 | p6<<15 | p8 (unmasked) */

  /* moved CSE2 up */
  /* a &= (0x7f<<29)|(0x7f<<15)|(0xff); */
  b &= SLOT_2_0;
  b = b<<8;
  a |= b;

  s = s<<4;
  b = p[-4];
  b &= 0x7f;
  b = b>>3;
  s |= b;

  *v = ((u64)s)<<32 | a;

  return 9;
}

/*
** Read a 32-bit variable-length integer from memory starting at p[0].
** Return the number of bytes read.  The value is stored in *v.
**
** If the varint stored in p[0] is larger than can fit in a 32-bit unsigned
** integer, then set *v to 0xffffffff.
**
** A MACRO version, getVarint32, is provided which inlines the 
** single-byte case.  All code should use the MACRO version as 
** this function assumes the single-byte case has already been handled.
*/
SQLITE_PRIVATE u8 sqlite3GetVarint32(const unsigned char *p, u32 *v){
  u32 a,b;

  /* The 1-byte case.  Overwhelmingly the most common.  Handled inline
  ** by the getVarin32() macro */
  a = *p;
  /* a: p0 (unmasked) */
#ifndef getVarint32
  if (!(a&0x80))
  {
    /* Values between 0 and 127 */
    *v = a;
    return 1;
  }
#endif

  /* The 2-byte case */
  p++;
  b = *p;
  /* b: p1 (unmasked) */
  if (!(b&0x80))
  {
    /* Values between 128 and 16383 */
    a &= 0x7f;
    a = a<<7;
    *v = a | b;
    return 2;
  }

  /* The 3-byte case */
  p++;
  a = a<<14;
  a |= *p;
  /* a: p0<<14 | p2 (unmasked) */
  if (!(a&0x80))
  {
    /* Values between 16384 and 2097151 */
    a &= (0x7f<<14)|(0x7f);
    b &= 0x7f;
    b = b<<7;
    *v = a | b;
    return 3;
  }

  /* A 32-bit varint is used to store size information in btrees.
  ** Objects are rarely larger than 2MiB limit of a 3-byte varint.
  ** A 3-byte varint is sufficient, for example, to record the size
  ** of a 1048569-byte BLOB or string.
  **
  ** We only unroll the first 1-, 2-, and 3- byte cases.  The very
  ** rare larger cases can be handled by the slower 64-bit varint
  ** routine.
  */
#if 1
  {
    u64 v64;
    u8 n;

    p -= 2;
    n = sqlite3GetVarint(p, &v64);
    assert( n>3 && n<=9 );
    if( (v64 & SQLITE_MAX_U32)!=v64 ){
      *v = 0xffffffff;
    }else{
      *v = (u32)v64;
    }
    return n;
  }

#else
  /* For following code (kept for historical record only) shows an
  ** unrolling for the 3- and 4-byte varint cases.  This code is
  ** slightly faster, but it is also larger and much harder to test.
  */
  p++;
  b = b<<14;
  b |= *p;
  /* b: p1<<14 | p3 (unmasked) */
  if (!(b&0x80))
  {
    /* Values between 2097152 and 268435455 */
    b &= (0x7f<<14)|(0x7f);
    a &= (0x7f<<14)|(0x7f);
    a = a<<7;
    *v = a | b;
    return 4;
  }

  p++;
  a = a<<14;
  a |= *p;
  /* a: p0<<28 | p2<<14 | p4 (unmasked) */
  if (!(a&0x80))
  {
    /* Values  between 268435456 and 34359738367 */
    a &= SLOT_4_2_0;
    b &= SLOT_4_2_0;
    b = b<<7;
    *v = a | b;
    return 5;
  }

  /* We can only reach this point when reading a corrupt database
  ** file.  In that case we are not in any hurry.  Use the (relatively
  ** slow) general-purpose sqlite3GetVarint() routine to extract the
  ** value. */
  {
    u64 v64;
    u8 n;

    p -= 4;
    n = sqlite3GetVarint(p, &v64);
    assert( n>5 && n<=9 );
    *v = (u32)v64;
    return n;
  }
#endif
}

/*
** Return the number of bytes that will be needed to store the given
** 64-bit integer.
*/
SQLITE_PRIVATE int sqlite3VarintLen(u64 v){
  int i;
  for(i=1; (v >>= 7)!=0; i++){ assert( i<10 ); }
  return i;
}


/*
** Read or write a four-byte big-endian integer value.
*/
SQLITE_PRIVATE u32 sqlite3Get4byte(const u8 *p){
#if SQLITE_BYTEORDER==4321
  u32 x;
  memcpy(&x,p,4);
  return x;
#elif SQLITE_BYTEORDER==1234 && !defined(SQLITE_DISABLE_INTRINSIC) \
    && defined(__GNUC__) && GCC_VERSION>=4003000
  u32 x;
  memcpy(&x,p,4);
  return __builtin_bswap32(x);
#elif SQLITE_BYTEORDER==1234 && !defined(SQLITE_DISABLE_INTRINSIC) \
    && defined(_MSC_VER) && _MSC_VER>=1300
  u32 x;
  memcpy(&x,p,4);
  return _byteswap_ulong(x);
#else
  testcase( p[0]&0x80 );
  return ((unsigned)p[0]<<24) | (p[1]<<16) | (p[2]<<8) | p[3];
#endif
}
SQLITE_PRIVATE void sqlite3Put4byte(unsigned char *p, u32 v){
#if SQLITE_BYTEORDER==4321
  memcpy(p,&v,4);
#elif SQLITE_BYTEORDER==1234 && !defined(SQLITE_DISABLE_INTRINSIC) \
    && defined(__GNUC__) && GCC_VERSION>=4003000
  u32 x = __builtin_bswap32(v);
  memcpy(p,&x,4);
#elif SQLITE_BYTEORDER==1234 && !defined(SQLITE_DISABLE_INTRINSIC) \
    && defined(_MSC_VER) && _MSC_VER>=1300
  u32 x = _byteswap_ulong(v);
  memcpy(p,&x,4);
#else
  p[0] = (u8)(v>>24);
  p[1] = (u8)(v>>16);
  p[2] = (u8)(v>>8);
  p[3] = (u8)v;
#endif
}



/*
** Translate a single byte of Hex into an integer.
** This routine only works if h really is a valid hexadecimal
** character:  0..9a..fA..F
*/
SQLITE_PRIVATE u8 sqlite3HexToInt(int h){
  assert( (h>='0' && h<='9') ||  (h>='a' && h<='f') ||  (h>='A' && h<='F') );
#ifdef SQLITE_ASCII
  h += 9*(1&(h>>6));
#endif
#ifdef SQLITE_EBCDIC
  h += 9*(1&~(h>>4));
#endif
  return (u8)(h & 0xf);
}

#if !defined(SQLITE_OMIT_BLOB_LITERAL) || defined(SQLITE_HAS_CODEC)
/*
** Convert a BLOB literal of the form "x'hhhhhh'" into its binary
** value.  Return a pointer to its binary value.  Space to hold the
** binary value has been obtained from malloc and must be freed by
** the calling routine.
*/
SQLITE_PRIVATE void *sqlite3HexToBlob(sqlite3 *db, const char *z, int n){
  char *zBlob;
  int i;

  zBlob = (char *)sqlite3DbMallocRawNN(db, n/2 + 1);
  n--;
  if( zBlob ){
    for(i=0; i<n; i+=2){
      zBlob[i/2] = (sqlite3HexToInt(z[i])<<4) | sqlite3HexToInt(z[i+1]);
    }
    zBlob[i/2] = 0;
  }
  return zBlob;
}
#endif /* !SQLITE_OMIT_BLOB_LITERAL || SQLITE_HAS_CODEC */

/*
** Log an error that is an API call on a connection pointer that should
** not have been used.  The "type" of connection pointer is given as the
** argument.  The zType is a word like "NULL" or "closed" or "invalid".
*/
static void logBadConnection(const char *zType){
  sqlite3_log(SQLITE_MISUSE, 
     "API call with %s database connection pointer",
     zType
  );
}

/*
** Check to make sure we have a valid db pointer.  This test is not
** foolproof but it does provide some measure of protection against
** misuse of the interface such as passing in db pointers that are
** NULL or which have been previously closed.  If this routine returns
** 1 it means that the db pointer is valid and 0 if it should not be
** dereferenced for any reason.  The calling function should invoke
** SQLITE_MISUSE immediately.
**
** sqlite3SafetyCheckOk() requires that the db pointer be valid for
** use.  sqlite3SafetyCheckSickOrOk() allows a db pointer that failed to
** open properly and is not fit for general use but which can be
** used as an argument to sqlite3_errmsg() or sqlite3_close().
*/
SQLITE_PRIVATE int sqlite3SafetyCheckOk(sqlite3 *db){
  u32 magic;
  if( db==0 ){
    logBadConnection("NULL");
    return 0;
  }
  magic = db->magic;
  if( magic!=SQLITE_MAGIC_OPEN ){
    if( sqlite3SafetyCheckSickOrOk(db) ){
      testcase( sqlite3GlobalConfig.xLog!=0 );
      logBadConnection("unopened");
    }
    return 0;
  }else{
    return 1;
  }
}
SQLITE_PRIVATE int sqlite3SafetyCheckSickOrOk(sqlite3 *db){
  u32 magic;
  magic = db->magic;
  if( magic!=SQLITE_MAGIC_SICK &&
      magic!=SQLITE_MAGIC_OPEN &&
      magic!=SQLITE_MAGIC_BUSY ){
    testcase( sqlite3GlobalConfig.xLog!=0 );
    logBadConnection("invalid");
    return 0;
  }else{
    return 1;
  }
}

/*
** Attempt to add, substract, or multiply the 64-bit signed value iB against
** the other 64-bit signed integer at *pA and store the result in *pA.
** Return 0 on success.  Or if the operation would have resulted in an
** overflow, leave *pA unchanged and return 1.
*/
SQLITE_PRIVATE int sqlite3AddInt64(i64 *pA, i64 iB){
  i64 iA = *pA;
  testcase( iA==0 ); testcase( iA==1 );
  testcase( iB==-1 ); testcase( iB==0 );
  if( iB>=0 ){
    testcase( iA>0 && LARGEST_INT64 - iA == iB );
    testcase( iA>0 && LARGEST_INT64 - iA == iB - 1 );
    if( iA>0 && LARGEST_INT64 - iA < iB ) return 1;
  }else{
    testcase( iA<0 && -(iA + LARGEST_INT64) == iB + 1 );
    testcase( iA<0 && -(iA + LARGEST_INT64) == iB + 2 );
    if( iA<0 && -(iA + LARGEST_INT64) > iB + 1 ) return 1;
  }
  *pA += iB;
  return 0; 
}
SQLITE_PRIVATE int sqlite3SubInt64(i64 *pA, i64 iB){
  testcase( iB==SMALLEST_INT64+1 );
  if( iB==SMALLEST_INT64 ){
    testcase( (*pA)==(-1) ); testcase( (*pA)==0 );
    if( (*pA)>=0 ) return 1;
    *pA -= iB;
    return 0;
  }else{
    return sqlite3AddInt64(pA, -iB);
  }
}
#define TWOPOWER32 (((i64)1)<<32)
#define TWOPOWER31 (((i64)1)<<31)
SQLITE_PRIVATE int sqlite3MulInt64(i64 *pA, i64 iB){
  i64 iA = *pA;
  i64 iA1, iA0, iB1, iB0, r;

  iA1 = iA/TWOPOWER32;
  iA0 = iA % TWOPOWER32;
  iB1 = iB/TWOPOWER32;
  iB0 = iB % TWOPOWER32;
  if( iA1==0 ){
    if( iB1==0 ){
      *pA *= iB;
      return 0;
    }
    r = iA0*iB1;
  }else if( iB1==0 ){
    r = iA1*iB0;
  }else{
    /* If both iA1 and iB1 are non-zero, overflow will result */
    return 1;
  }
  testcase( r==(-TWOPOWER31)-1 );
  testcase( r==(-TWOPOWER31) );
  testcase( r==TWOPOWER31 );
  testcase( r==TWOPOWER31-1 );
  if( r<(-TWOPOWER31) || r>=TWOPOWER31 ) return 1;
  r *= TWOPOWER32;
  if( sqlite3AddInt64(&r, iA0*iB0) ) return 1;
  *pA = r;
  return 0;
}

/*
** Compute the absolute value of a 32-bit signed integer, of possible.  Or 
** if the integer has a value of -2147483648, return +2147483647
*/
SQLITE_PRIVATE int sqlite3AbsInt32(int x){
  if( x>=0 ) return x;
  if( x==(int)0x80000000 ) return 0x7fffffff;
  return -x;
}

#ifdef SQLITE_ENABLE_8_3_NAMES
/*
** If SQLITE_ENABLE_8_3_NAMES is set at compile-time and if the database
** filename in zBaseFilename is a URI with the "8_3_names=1" parameter and
** if filename in z[] has a suffix (a.k.a. "extension") that is longer than
** three characters, then shorten the suffix on z[] to be the last three
** characters of the original suffix.
**
** If SQLITE_ENABLE_8_3_NAMES is set to 2 at compile-time, then always
** do the suffix shortening regardless of URI parameter.
**
** Examples:
**
**     test.db-journal    =>   test.nal
**     test.db-wal        =>   test.wal
**     test.db-shm        =>   test.shm
**     test.db-mj7f3319fa =>   test.9fa
*/
SQLITE_PRIVATE void sqlite3FileSuffix3(const char *zBaseFilename, char *z){
#if SQLITE_ENABLE_8_3_NAMES<2
  if( sqlite3_uri_boolean(zBaseFilename, "8_3_names", 0) )
#endif
  {
    int i, sz;
    sz = sqlite3Strlen30(z);
    for(i=sz-1; i>0 && z[i]!='/' && z[i]!='.'; i--){}
    if( z[i]=='.' && ALWAYS(sz>i+4) ) memmove(&z[i+1], &z[sz-3], 4);
  }
}
#endif

/* 
** Find (an approximate) sum of two LogEst values.  This computation is
** not a simple "+" operator because LogEst is stored as a logarithmic
** value.
** 
*/
SQLITE_PRIVATE LogEst sqlite3LogEstAdd(LogEst a, LogEst b){
  static const unsigned char x[] = {
     10, 10,                         /* 0,1 */
      9, 9,                          /* 2,3 */
      8, 8,                          /* 4,5 */
      7, 7, 7,                       /* 6,7,8 */
      6, 6, 6,                       /* 9,10,11 */
      5, 5, 5,                       /* 12-14 */
      4, 4, 4, 4,                    /* 15-18 */
      3, 3, 3, 3, 3, 3,              /* 19-24 */
      2, 2, 2, 2, 2, 2, 2,           /* 25-31 */
  };
  if( a>=b ){
    if( a>b+49 ) return a;
    if( a>b+31 ) return a+1;
    return a+x[a-b];
  }else{
    if( b>a+49 ) return b;
    if( b>a+31 ) return b+1;
    return b+x[b-a];
  }
}

/*
** Convert an integer into a LogEst.  In other words, compute an
** approximation for 10*log2(x).
*/
SQLITE_PRIVATE LogEst sqlite3LogEst(u64 x){
  static LogEst a[] = { 0, 2, 3, 5, 6, 7, 8, 9 };
  LogEst y = 40;
  if( x<8 ){
    if( x<2 ) return 0;
    while( x<8 ){  y -= 10; x <<= 1; }
  }else{
    while( x>255 ){ y += 40; x >>= 4; }  /*OPTIMIZATION-IF-TRUE*/
    while( x>15 ){  y += 10; x >>= 1; }
  }
  return a[x&7] + y - 10;
}

#ifndef SQLITE_OMIT_VIRTUALTABLE
/*
** Convert a double into a LogEst
** In other words, compute an approximation for 10*log2(x).
*/
SQLITE_PRIVATE LogEst sqlite3LogEstFromDouble(double x){
  u64 a;
  LogEst e;
  assert( sizeof(x)==8 && sizeof(a)==8 );
  if( x<=1 ) return 0;
  if( x<=2000000000 ) return sqlite3LogEst((u64)x);
  memcpy(&a, &x, 8);
  e = (a>>52) - 1022;
  return e*10;
}
#endif /* SQLITE_OMIT_VIRTUALTABLE */

#if defined(SQLITE_ENABLE_STMT_SCANSTATUS) || \
    defined(SQLITE_ENABLE_STAT3_OR_STAT4) || \
    defined(SQLITE_EXPLAIN_ESTIMATED_ROWS)
/*
** Convert a LogEst into an integer.
**
** Note that this routine is only used when one or more of various
** non-standard compile-time options is enabled.
*/
SQLITE_PRIVATE u64 sqlite3LogEstToInt(LogEst x){
  u64 n;
  n = x%10;
  x /= 10;
  if( n>=5 ) n -= 2;
  else if( n>=1 ) n -= 1;
#if defined(SQLITE_ENABLE_STMT_SCANSTATUS) || \
    defined(SQLITE_EXPLAIN_ESTIMATED_ROWS)
  if( x>60 ) return (u64)LARGEST_INT64;
#else
  /* If only SQLITE_ENABLE_STAT3_OR_STAT4 is on, then the largest input
  ** possible to this routine is 310, resulting in a maximum x of 31 */
  assert( x<=60 );
#endif
  return x>=3 ? (n+8)<<(x-3) : (n+8)>>(3-x);
}
#endif /* defined SCANSTAT or STAT4 or ESTIMATED_ROWS */

/************** End of util.c ************************************************/
/************** Begin file hash.c ********************************************/
/*
** 2001 September 22
**
** The author disclaims copyright to this source code.  In place of
** a legal notice, here is a blessing:
**
**    May you do good and not evil.
**    May you find forgiveness for yourself and forgive others.
**    May you share freely, never taking more than you give.
**
*************************************************************************
** This is the implementation of generic hash-tables
** used in SQLite.
*/
/* #include "sqliteInt.h" */
/* #include <assert.h> */

/* Turn bulk memory into a hash table object by initializing the
** fields of the Hash structure.
**
** "pNew" is a pointer to the hash table that is to be initialized.
*/
SQLITE_PRIVATE void sqlite3HashInit(Hash *pNew){
  assert( pNew!=0 );
  pNew->first = 0;
  pNew->count = 0;
  pNew->htsize = 0;
  pNew->ht = 0;
}

/* Remove all entries from a hash table.  Reclaim all memory.
** Call this routine to delete a hash table or to reset a hash table
** to the empty state.
*/
SQLITE_PRIVATE void sqlite3HashClear(Hash *pH){
  HashElem *elem;         /* For looping over all elements of the table */

  assert( pH!=0 );
  elem = pH->first;
  pH->first = 0;
  sqlite3_free(pH->ht);
  pH->ht = 0;
  pH->htsize = 0;
  while( elem ){
    HashElem *next_elem = elem->next;
    sqlite3_free(elem);
    elem = next_elem;
  }
  pH->count = 0;
}

/*
** The hashing function.
*/
static unsigned int strHash(const char *z){
  unsigned int h = 0;
  unsigned char c;
  while( (c = (unsigned char)*z++)!=0 ){     /*OPTIMIZATION-IF-TRUE*/
    h = (h<<3) ^ h ^ sqlite3UpperToLower[c];
  }
  return h;
}


/* Link pNew element into the hash table pH.  If pEntry!=0 then also
** insert pNew into the pEntry hash bucket.
*/
static void insertElement(
  Hash *pH,              /* The complete hash table */
  struct _ht *pEntry,    /* The entry into which pNew is inserted */
  HashElem *pNew         /* The element to be inserted */
){
  HashElem *pHead;       /* First element already in pEntry */
  if( pEntry ){
    pHead = pEntry->count ? pEntry->chain : 0;
    pEntry->count++;
    pEntry->chain = pNew;
  }else{
    pHead = 0;
  }
  if( pHead ){
    pNew->next = pHead;
    pNew->prev = pHead->prev;
    if( pHead->prev ){ pHead->prev->next = pNew; }
    else             { pH->first = pNew; }
    pHead->prev = pNew;
  }else{
    pNew->next = pH->first;
    if( pH->first ){ pH->first->prev = pNew; }
    pNew->prev = 0;
    pH->first = pNew;
  }
}


/* Resize the hash table so that it cantains "new_size" buckets.
**
** The hash table might fail to resize if sqlite3_malloc() fails or
** if the new size is the same as the prior size.
** Return TRUE if the resize occurs and false if not.
*/
static int rehash(Hash *pH, unsigned int new_size){
  struct _ht *new_ht;            /* The new hash table */
  HashElem *elem, *next_elem;    /* For looping over existing elements */

#if SQLITE_MALLOC_SOFT_LIMIT>0
  if( new_size*sizeof(struct _ht)>SQLITE_MALLOC_SOFT_LIMIT ){
    new_size = SQLITE_MALLOC_SOFT_LIMIT/sizeof(struct _ht);
  }
  if( new_size==pH->htsize ) return 0;
#endif

  /* The inability to allocates space for a larger hash table is
  ** a performance hit but it is not a fatal error.  So mark the
  ** allocation as a benign. Use sqlite3Malloc()/memset(0) instead of 
  ** sqlite3MallocZero() to make the allocation, as sqlite3MallocZero()
  ** only zeroes the requested number of bytes whereas this module will
  ** use the actual amount of space allocated for the hash table (which
  ** may be larger than the requested amount).
  */
  sqlite3BeginBenignMalloc();
  new_ht = (struct _ht *)sqlite3Malloc( new_size*sizeof(struct _ht) );
  sqlite3EndBenignMalloc();

  if( new_ht==0 ) return 0;
  sqlite3_free(pH->ht);
  pH->ht = new_ht;
  pH->htsize = new_size = sqlite3MallocSize(new_ht)/sizeof(struct _ht);
  memset(new_ht, 0, new_size*sizeof(struct _ht));
  for(elem=pH->first, pH->first=0; elem; elem = next_elem){
    unsigned int h = strHash(elem->pKey) % new_size;
    next_elem = elem->next;
    insertElement(pH, &new_ht[h], elem);
  }
  return 1;
}

/* This function (for internal use only) locates an element in an
** hash table that matches the given key.  The hash for this key is
** also computed and returned in the *pH parameter.
*/
static HashElem *findElementWithHash(
  const Hash *pH,     /* The pH to be searched */
  const char *pKey,   /* The key we are searching for */
  unsigned int *pHash /* Write the hash value here */
){
  HashElem *elem;                /* Used to loop thru the element list */
  int count;                     /* Number of elements left to test */
  unsigned int h;                /* The computed hash */

  if( pH->ht ){   /*OPTIMIZATION-IF-TRUE*/
    struct _ht *pEntry;
    h = strHash(pKey) % pH->htsize;
    pEntry = &pH->ht[h];
    elem = pEntry->chain;
    count = pEntry->count;
  }else{
    h = 0;
    elem = pH->first;
    count = pH->count;
  }
  *pHash = h;
  while( count-- ){
    assert( elem!=0 );
    if( sqlite3StrICmp(elem->pKey,pKey)==0 ){ 
      return elem;
    }
    elem = elem->next;
  }
  return 0;
}

/* Remove a single entry from the hash table given a pointer to that
** element and a hash on the element's key.
*/
static void removeElementGivenHash(
  Hash *pH,         /* The pH containing "elem" */
  HashElem* elem,   /* The element to be removed from the pH */
  unsigned int h    /* Hash value for the element */
){
  struct _ht *pEntry;
  if( elem->prev ){
    elem->prev->next = elem->next; 
  }else{
    pH->first = elem->next;
  }
  if( elem->next ){
    elem->next->prev = elem->prev;
  }
  if( pH->ht ){
    pEntry = &pH->ht[h];
    if( pEntry->chain==elem ){
      pEntry->chain = elem->next;
    }
    pEntry->count--;
    assert( pEntry->count>=0 );
  }
  sqlite3_free( elem );
  pH->count--;
  if( pH->count==0 ){
    assert( pH->first==0 );
    assert( pH->count==0 );
    sqlite3HashClear(pH);
  }
}

/* Attempt to locate an element of the hash table pH with a key
** that matches pKey.  Return the data for this element if it is
** found, or NULL if there is no match.
*/
SQLITE_PRIVATE void *sqlite3HashFind(const Hash *pH, const char *pKey){
  HashElem *elem;    /* The element that matches key */
  unsigned int h;    /* A hash on key */

  assert( pH!=0 );
  assert( pKey!=0 );
  elem = findElementWithHash(pH, pKey, &h);
  return elem ? elem->data : 0;
}

/* Insert an element into the hash table pH.  The key is pKey
** and the data is "data".
**
** If no element exists with a matching key, then a new
** element is created and NULL is returned.
**
** If another element already exists with the same key, then the
** new data replaces the old data and the old data is returned.
** The key is not copied in this instance.  If a malloc fails, then
** the new data is returned and the hash table is unchanged.
**
** If the "data" parameter to this function is NULL, then the
** element corresponding to "key" is removed from the hash table.
*/
SQLITE_PRIVATE void *sqlite3HashInsert(Hash *pH, const char *pKey, void *data){
  unsigned int h;       /* the hash of the key modulo hash table size */
  HashElem *elem;       /* Used to loop thru the element list */
  HashElem *new_elem;   /* New element added to the pH */

  assert( pH!=0 );
  assert( pKey!=0 );
  elem = findElementWithHash(pH,pKey,&h);
  if( elem ){
    void *old_data = elem->data;
    if( data==0 ){
      removeElementGivenHash(pH,elem,h);
    }else{
      elem->data = data;
      elem->pKey = pKey;
    }
    return old_data;
  }
  if( data==0 ) return 0;
  new_elem = (HashElem*)sqlite3Malloc( sizeof(HashElem) );
  if( new_elem==0 ) return data;
  new_elem->pKey = pKey;
  new_elem->data = data;
  pH->count++;
  if( pH->count>=10 && pH->count > 2*pH->htsize ){
    if( rehash(pH, pH->count*2) ){
      assert( pH->htsize>0 );
      h = strHash(pKey) % pH->htsize;
    }
  }
  insertElement(pH, pH->ht ? &pH->ht[h] : 0, new_elem);
  return 0;
}

/************** End of hash.c ************************************************/
/************** Begin file opcodes.c *****************************************/
/* Automatically generated.  Do not edit */
/* See the tool/mkopcodec.tcl script for details. */
#if !defined(SQLITE_OMIT_EXPLAIN) \
 || defined(VDBE_PROFILE) \
 || defined(SQLITE_DEBUG)
#if defined(SQLITE_ENABLE_EXPLAIN_COMMENTS) || defined(SQLITE_DEBUG)
# define OpHelp(X) "\0" X
#else
# define OpHelp(X)
#endif
SQLITE_PRIVATE const char *sqlite3OpcodeName(int i){
 static const char *const azName[] = {
    /*   0 */ "Savepoint"        OpHelp(""),
    /*   1 */ "AutoCommit"       OpHelp(""),
    /*   2 */ "Transaction"      OpHelp(""),
    /*   3 */ "SorterNext"       OpHelp(""),
    /*   4 */ "PrevIfOpen"       OpHelp(""),
    /*   5 */ "NextIfOpen"       OpHelp(""),
    /*   6 */ "Prev"             OpHelp(""),
    /*   7 */ "Next"             OpHelp(""),
    /*   8 */ "Checkpoint"       OpHelp(""),
    /*   9 */ "JournalMode"      OpHelp(""),
    /*  10 */ "Vacuum"           OpHelp(""),
    /*  11 */ "VFilter"          OpHelp("iplan=r[P3] zplan='P4'"),
    /*  12 */ "VUpdate"          OpHelp("data=r[P3@P2]"),
    /*  13 */ "Goto"             OpHelp(""),
    /*  14 */ "Gosub"            OpHelp(""),
    /*  15 */ "InitCoroutine"    OpHelp(""),
    /*  16 */ "Yield"            OpHelp(""),
    /*  17 */ "MustBeInt"        OpHelp(""),
    /*  18 */ "Jump"             OpHelp(""),
    /*  19 */ "Not"              OpHelp("r[P2]= !r[P1]"),
    /*  20 */ "Once"             OpHelp(""),
    /*  21 */ "If"               OpHelp(""),
    /*  22 */ "IfNot"            OpHelp(""),
    /*  23 */ "SeekLT"           OpHelp("key=r[P3@P4]"),
    /*  24 */ "SeekLE"           OpHelp("key=r[P3@P4]"),
    /*  25 */ "SeekGE"           OpHelp("key=r[P3@P4]"),
    /*  26 */ "SeekGT"           OpHelp("key=r[P3@P4]"),
    /*  27 */ "Or"               OpHelp("r[P3]=(r[P1] || r[P2])"),
    /*  28 */ "And"              OpHelp("r[P3]=(r[P1] && r[P2])"),
    /*  29 */ "NoConflict"       OpHelp("key=r[P3@P4]"),
    /*  30 */ "NotFound"         OpHelp("key=r[P3@P4]"),
    /*  31 */ "Found"            OpHelp("key=r[P3@P4]"),
    /*  32 */ "NotExists"        OpHelp("intkey=r[P3]"),
    /*  33 */ "Last"             OpHelp(""),
    /*  34 */ "IsNull"           OpHelp("if r[P1]==NULL goto P2"),
    /*  35 */ "NotNull"          OpHelp("if r[P1]!=NULL goto P2"),
    /*  36 */ "Ne"               OpHelp("if r[P1]!=r[P3] goto P2"),
    /*  37 */ "Eq"               OpHelp("if r[P1]==r[P3] goto P2"),
    /*  38 */ "Gt"               OpHelp("if r[P1]>r[P3] goto P2"),
    /*  39 */ "Le"               OpHelp("if r[P1]<=r[P3] goto P2"),
    /*  40 */ "Lt"               OpHelp("if r[P1]<r[P3] goto P2"),
    /*  41 */ "Ge"               OpHelp("if r[P1]>=r[P3] goto P2"),
    /*  42 */ "SorterSort"       OpHelp(""),
    /*  43 */ "BitAnd"           OpHelp("r[P3]=r[P1]&r[P2]"),
    /*  44 */ "BitOr"            OpHelp("r[P3]=r[P1]|r[P2]"),
    /*  45 */ "ShiftLeft"        OpHelp("r[P3]=r[P2]<<r[P1]"),
    /*  46 */ "ShiftRight"       OpHelp("r[P3]=r[P2]>>r[P1]"),
    /*  47 */ "Add"              OpHelp("r[P3]=r[P1]+r[P2]"),
    /*  48 */ "Subtract"         OpHelp("r[P3]=r[P2]-r[P1]"),
    /*  49 */ "Multiply"         OpHelp("r[P3]=r[P1]*r[P2]"),
    /*  50 */ "Divide"           OpHelp("r[P3]=r[P2]/r[P1]"),
    /*  51 */ "Remainder"        OpHelp("r[P3]=r[P2]%r[P1]"),
    /*  52 */ "Concat"           OpHelp("r[P3]=r[P2]+r[P1]"),
    /*  53 */ "Sort"             OpHelp(""),
    /*  54 */ "BitNot"           OpHelp("r[P1]= ~r[P1]"),
    /*  55 */ "Rewind"           OpHelp(""),
    /*  56 */ "IdxLE"            OpHelp("key=r[P3@P4]"),
    /*  57 */ "IdxGT"            OpHelp("key=r[P3@P4]"),
    /*  58 */ "IdxLT"            OpHelp("key=r[P3@P4]"),
    /*  59 */ "IdxGE"            OpHelp("key=r[P3@P4]"),
    /*  60 */ "RowSetRead"       OpHelp("r[P3]=rowset(P1)"),
    /*  61 */ "RowSetTest"       OpHelp("if r[P3] in rowset(P1) goto P2"),
    /*  62 */ "Program"          OpHelp(""),
    /*  63 */ "FkIfZero"         OpHelp("if fkctr[P1]==0 goto P2"),
    /*  64 */ "IfPos"            OpHelp("if r[P1]>0 then r[P1]-=P3, goto P2"),
    /*  65 */ "IfNotZero"        OpHelp("if r[P1]!=0 then r[P1]-=P3, goto P2"),
    /*  66 */ "DecrJumpZero"     OpHelp("if (--r[P1])==0 goto P2"),
    /*  67 */ "IncrVacuum"       OpHelp(""),
    /*  68 */ "VNext"            OpHelp(""),
    /*  69 */ "Init"             OpHelp("Start at P2"),
    /*  70 */ "Return"           OpHelp(""),
    /*  71 */ "EndCoroutine"     OpHelp(""),
    /*  72 */ "HaltIfNull"       OpHelp("if r[P3]=null halt"),
    /*  73 */ "Halt"             OpHelp(""),
    /*  74 */ "Integer"          OpHelp("r[P2]=P1"),
    /*  75 */ "Int64"            OpHelp("r[P2]=P4"),
    /*  76 */ "String"           OpHelp("r[P2]='P4' (len=P1)"),
    /*  77 */ "Null"             OpHelp("r[P2..P3]=NULL"),
    /*  78 */ "SoftNull"         OpHelp("r[P1]=NULL"),
    /*  79 */ "Blob"             OpHelp("r[P2]=P4 (len=P1)"),
    /*  80 */ "Variable"         OpHelp("r[P2]=parameter(P1,P4)"),
    /*  81 */ "Move"             OpHelp("r[P2@P3]=r[P1@P3]"),
    /*  82 */ "Copy"             OpHelp("r[P2@P3+1]=r[P1@P3+1]"),
    /*  83 */ "SCopy"            OpHelp("r[P2]=r[P1]"),
    /*  84 */ "IntCopy"          OpHelp("r[P2]=r[P1]"),
    /*  85 */ "ResultRow"        OpHelp("output=r[P1@P2]"),
    /*  86 */ "CollSeq"          OpHelp(""),
    /*  87 */ "Function0"        OpHelp("r[P3]=func(r[P2@P5])"),
    /*  88 */ "Function"         OpHelp("r[P3]=func(r[P2@P5])"),
    /*  89 */ "AddImm"           OpHelp("r[P1]=r[P1]+P2"),
    /*  90 */ "RealAffinity"     OpHelp(""),
    /*  91 */ "Cast"             OpHelp("affinity(r[P1])"),
    /*  92 */ "Permutation"      OpHelp(""),
    /*  93 */ "Compare"          OpHelp("r[P1@P3] <-> r[P2@P3]"),
    /*  94 */ "Column"           OpHelp("r[P3]=PX"),
    /*  95 */ "Affinity"         OpHelp("affinity(r[P1@P2])"),
    /*  96 */ "MakeRecord"       OpHelp("r[P3]=mkrec(r[P1@P2])"),
    /*  97 */ "String8"          OpHelp("r[P2]='P4'"),
    /*  98 */ "Count"            OpHelp("r[P2]=count()"),
    /*  99 */ "ReadCookie"       OpHelp(""),
    /* 100 */ "SetCookie"        OpHelp(""),
    /* 101 */ "ReopenIdx"        OpHelp("root=P2 iDb=P3"),
    /* 102 */ "OpenRead"         OpHelp("root=P2 iDb=P3"),
    /* 103 */ "OpenWrite"        OpHelp("root=P2 iDb=P3"),
    /* 104 */ "OpenAutoindex"    OpHelp("nColumn=P2"),
    /* 105 */ "OpenEphemeral"    OpHelp("nColumn=P2"),
    /* 106 */ "SorterOpen"       OpHelp(""),
    /* 107 */ "SequenceTest"     OpHelp("if( cursor[P1].ctr++ ) pc = P2"),
    /* 108 */ "OpenPseudo"       OpHelp("P3 columns in r[P2]"),
    /* 109 */ "Close"            OpHelp(""),
    /* 110 */ "ColumnsUsed"      OpHelp(""),
    /* 111 */ "Sequence"         OpHelp("r[P2]=cursor[P1].ctr++"),
    /* 112 */ "NewRowid"         OpHelp("r[P2]=rowid"),
    /* 113 */ "Insert"           OpHelp("intkey=r[P3] data=r[P2]"),
    /* 114 */ "InsertInt"        OpHelp("intkey=P3 data=r[P2]"),
    /* 115 */ "Delete"           OpHelp(""),
    /* 116 */ "ResetCount"       OpHelp(""),
    /* 117 */ "SorterCompare"    OpHelp("if key(P1)!=trim(r[P3],P4) goto P2"),
    /* 118 */ "SorterData"       OpHelp("r[P2]=data"),
    /* 119 */ "RowKey"           OpHelp("r[P2]=key"),
    /* 120 */ "RowData"          OpHelp("r[P2]=data"),
    /* 121 */ "Rowid"            OpHelp("r[P2]=rowid"),
    /* 122 */ "NullRow"          OpHelp(""),
    /* 123 */ "SorterInsert"     OpHelp(""),
    /* 124 */ "IdxInsert"        OpHelp("key=r[P2]"),
    /* 125 */ "IdxDelete"        OpHelp("key=r[P2@P3]"),
    /* 126 */ "Seek"             OpHelp("Move P3 to P1.rowid"),
    /* 127 */ "IdxRowid"         OpHelp("r[P2]=rowid"),
    /* 128 */ "Destroy"          OpHelp(""),
    /* 129 */ "Clear"            OpHelp(""),
    /* 130 */ "ResetSorter"      OpHelp(""),
    /* 131 */ "CreateIndex"      OpHelp("r[P2]=root iDb=P1"),
    /* 132 */ "CreateTable"      OpHelp("r[P2]=root iDb=P1"),
    /* 133 */ "Real"             OpHelp("r[P2]=P4"),
    /* 134 */ "ParseSchema"      OpHelp(""),
    /* 135 */ "LoadAnalysis"     OpHelp(""),
    /* 136 */ "DropTable"        OpHelp(""),
    /* 137 */ "DropIndex"        OpHelp(""),
    /* 138 */ "DropTrigger"      OpHelp(""),
    /* 139 */ "IntegrityCk"      OpHelp(""),
    /* 140 */ "RowSetAdd"        OpHelp("rowset(P1)=r[P2]"),
    /* 141 */ "Param"            OpHelp(""),
    /* 142 */ "FkCounter"        OpHelp("fkctr[P1]+=P2"),
    /* 143 */ "MemMax"           OpHelp("r[P1]=max(r[P1],r[P2])"),
    /* 144 */ "OffsetLimit"      OpHelp("if r[P1]>0 then r[P2]=r[P1]+max(0,r[P3]) else r[P2]=(-1)"),
    /* 145 */ "AggStep0"         OpHelp("accum=r[P3] step(r[P2@P5])"),
    /* 146 */ "AggStep"          OpHelp("accum=r[P3] step(r[P2@P5])"),
    /* 147 */ "AggFinal"         OpHelp("accum=r[P1] N=P2"),
    /* 148 */ "Expire"           OpHelp(""),
    /* 149 */ "TableLock"        OpHelp("iDb=P1 root=P2 write=P3"),
    /* 150 */ "VBegin"           OpHelp(""),
    /* 151 */ "VCreate"          OpHelp(""),
    /* 152 */ "VDestroy"         OpHelp(""),
    /* 153 */ "VOpen"            OpHelp(""),
    /* 154 */ "VColumn"          OpHelp("r[P3]=vcolumn(P2)"),
    /* 155 */ "VRename"          OpHelp(""),
    /* 156 */ "Pagecount"        OpHelp(""),
    /* 157 */ "MaxPgcnt"         OpHelp(""),
    /* 158 */ "CursorHint"       OpHelp(""),
    /* 159 */ "Noop"             OpHelp(""),
    /* 160 */ "Explain"          OpHelp(""),
  };
  return azName[i];
}
#endif

/************** End of opcodes.c *********************************************/
