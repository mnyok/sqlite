/************** Begin file sqliteInt.h ***************************************/
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
** Internal interface definitions for SQLite.
**
*/
#ifndef _SQLITEINT_H_
#define _SQLITEINT_H_

/* Special Comments:
**
** Some comments have special meaning to the tools that measure test
** coverage:
**
**    NO_TEST                     - The branches on this line are not
**                                  measured by branch coverage.  This is
**                                  used on lines of code that actually
**                                  implement parts of coverage testing.
**
**    OPTIMIZATION-IF-TRUE        - This branch is allowed to alway be false
**                                  and the correct answer is still obtained,
**                                  though perhaps more slowly.
**
**    OPTIMIZATION-IF-FALSE       - This branch is allowed to alway be true
**                                  and the correct answer is still obtained,
**                                  though perhaps more slowly.
**
**    PREVENTS-HARMLESS-OVERREAD  - This branch prevents a buffer overread
**                                  that would be harmless and undetectable
**                                  if it did occur.  
**
** In all cases, the special comment must be enclosed in the usual
** slash-asterisk...asterisk-slash comment marks, with no spaces between the 
** asterisks and the comment text.
*/

/*
** Make sure that rand_s() is available on Windows systems with MSVC 2005
** or higher.
*/
#if defined(_MSC_VER) && _MSC_VER>=1400
#  define _CRT_RAND_S
#endif

/*
** Include the header file used to customize the compiler options for MSVC.
** This should be done first so that it can successfully prevent spurious
** compiler warnings due to subsequent content in this file and other files
** that are included by this file.
*/
/************** Include msvc.h in the middle of sqliteInt.h ******************/
/************** Begin file msvc.h ********************************************/
/*
** 2015 January 12
**
** The author disclaims copyright to this source code.  In place of
** a legal notice, here is a blessing:
**
**    May you do good and not evil.
**    May you find forgiveness for yourself and forgive others.
**    May you share freely, never taking more than you give.
**
******************************************************************************
**
** This file contains code that is specific to MSVC.
*/
#ifndef _MSVC_H_
#define _MSVC_H_

#if defined(_MSC_VER)
#pragma warning(disable : 4054)
#pragma warning(disable : 4055)
#pragma warning(disable : 4100)
#pragma warning(disable : 4127)
#pragma warning(disable : 4130)
#pragma warning(disable : 4152)
#pragma warning(disable : 4189)
#pragma warning(disable : 4206)
#pragma warning(disable : 4210)
#pragma warning(disable : 4232)
#pragma warning(disable : 4244)
#pragma warning(disable : 4305)
#pragma warning(disable : 4306)
#pragma warning(disable : 4702)
#pragma warning(disable : 4706)
#endif /* defined(_MSC_VER) */

#endif /* _MSVC_H_ */

/************** End of msvc.h ************************************************/
/************** Continuing where we left off in sqliteInt.h ******************/

/*
** Special setup for VxWorks
*/
/************** Include vxworks.h in the middle of sqliteInt.h ***************/
/************** Begin file vxworks.h *****************************************/
/*
** 2015-03-02
**
** The author disclaims copyright to this source code.  In place of
** a legal notice, here is a blessing:
**
**    May you do good and not evil.
**    May you find forgiveness for yourself and forgive others.
**    May you share freely, never taking more than you give.
**
******************************************************************************
**
** This file contains code that is specific to Wind River's VxWorks
*/
#if defined(__RTP__) || defined(_WRS_KERNEL)
/* This is VxWorks.  Set up things specially for that OS
*/
#include <vxWorks.h>
#include <pthread.h>  /* amalgamator: dontcache */
#define OS_VXWORKS 1
#define SQLITE_OS_OTHER 0
#define SQLITE_HOMEGROWN_RECURSIVE_MUTEX 1
#define SQLITE_OMIT_LOAD_EXTENSION 1
#define SQLITE_ENABLE_LOCKING_STYLE 0
#define HAVE_UTIME 1
#else
/* This is not VxWorks. */
#define OS_VXWORKS 0
#define HAVE_FCHOWN 1
#define HAVE_READLINK 1
#define HAVE_LSTAT 1
#endif /* defined(_WRS_KERNEL) */

/************** End of vxworks.h *********************************************/
/************** Continuing where we left off in sqliteInt.h ******************/

/*
** These #defines should enable >2GB file support on POSIX if the
** underlying operating system supports it.  If the OS lacks
** large file support, or if the OS is windows, these should be no-ops.
**
** Ticket #2739:  The _LARGEFILE_SOURCE macro must appear before any
** system #includes.  Hence, this block of code must be the very first
** code in all source files.
**
** Large file support can be disabled using the -DSQLITE_DISABLE_LFS switch
** on the compiler command line.  This is necessary if you are compiling
** on a recent machine (ex: Red Hat 7.2) but you want your code to work
** on an older machine (ex: Red Hat 6.0).  If you compile on Red Hat 7.2
** without this option, LFS is enable.  But LFS does not exist in the kernel
** in Red Hat 6.0, so the code won't work.  Hence, for maximum binary
** portability you should omit LFS.
**
** The previous paragraph was written in 2005.  (This paragraph is written
** on 2008-11-28.) These days, all Linux kernels support large files, so
** you should probably leave LFS enabled.  But some embedded platforms might
** lack LFS in which case the SQLITE_DISABLE_LFS macro might still be useful.
**
** Similar is true for Mac OS X.  LFS is only supported on Mac OS X 9 and later.
*/
#ifndef SQLITE_DISABLE_LFS
# define _LARGE_FILE       1
# ifndef _FILE_OFFSET_BITS
#   define _FILE_OFFSET_BITS 64
# endif
# define _LARGEFILE_SOURCE 1
#endif

/* What version of GCC is being used.  0 means GCC is not being used */
#ifdef __GNUC__
# define GCC_VERSION (__GNUC__*1000000+__GNUC_MINOR__*1000+__GNUC_PATCHLEVEL__)
#else
# define GCC_VERSION 0
#endif

/* Needed for various definitions... */
#if defined(__GNUC__) && !defined(_GNU_SOURCE)
# define _GNU_SOURCE
#endif

#if defined(__OpenBSD__) && !defined(_BSD_SOURCE)
# define _BSD_SOURCE
#endif

/*
** For MinGW, check to see if we can include the header file containing its
** version information, among other things.  Normally, this internal MinGW
** header file would [only] be included automatically by other MinGW header
** files; however, the contained version information is now required by this
** header file to work around binary compatibility issues (see below) and
** this is the only known way to reliably obtain it.  This entire #if block
** would be completely unnecessary if there was any other way of detecting
** MinGW via their preprocessor (e.g. if they customized their GCC to define
** some MinGW-specific macros).  When compiling for MinGW, either the
** _HAVE_MINGW_H or _HAVE__MINGW_H (note the extra underscore) macro must be
** defined; otherwise, detection of conditions specific to MinGW will be
** disabled.
*/
#if defined(_HAVE_MINGW_H)
# include "mingw.h"
#elif defined(_HAVE__MINGW_H)
# include "_mingw.h"
#endif

/*
** For MinGW version 4.x (and higher), check to see if the _USE_32BIT_TIME_T
** define is required to maintain binary compatibility with the MSVC runtime
** library in use (e.g. for Windows XP).
*/
#if !defined(_USE_32BIT_TIME_T) && !defined(_USE_64BIT_TIME_T) && \
    defined(_WIN32) && !defined(_WIN64) && \
    defined(__MINGW_MAJOR_VERSION) && __MINGW_MAJOR_VERSION >= 4 && \
    defined(__MSVCRT__)
# define _USE_32BIT_TIME_T
#endif

/* The public SQLite interface.  The _FILE_OFFSET_BITS macro must appear
** first in QNX.  Also, the _USE_32BIT_TIME_T macro must appear first for
** MinGW.
*/
/************** Include sqlite3.h in the middle of sqliteInt.h ***************/
/************** Begin file sqlite3.h *****************************************/
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
** This header file defines the interface that the SQLite library
** presents to client programs.  If a C-function, structure, datatype,
** or constant definition does not appear in this file, then it is
** not a published API of SQLite, is subject to change without
** notice, and should not be referenced by programs that use SQLite.
**
** Some of the definitions that are in this file are marked as
** "experimental".  Experimental interfaces are normally new
** features recently added to SQLite.  We do not anticipate changes
** to experimental interfaces but reserve the right to make minor changes
** if experience from use "in the wild" suggest such changes are prudent.
**
** The official C-language API documentation for SQLite is derived
** from comments in this file.  This file is the authoritative source
** on how SQLite interfaces are supposed to operate.
**
** The name of this file under configuration management is "sqlite.h.in".
** The makefile makes some minor changes to this file (such as inserting
** the version number) and changes its name to "sqlite3.h" as
** part of the build process.
*/
#ifndef _SQLITE3_H_
#define _SQLITE3_H_
#include <stdarg.h>     /* Needed for the definition of va_list */

/*
** Make sure we can call this stuff from C++.
*/
#if 0
extern "C" {
#endif


/*
** Provide the ability to override linkage features of the interface.
*/
#ifndef SQLITE_EXTERN
# define SQLITE_EXTERN extern
#endif
#ifndef SQLITE_API
# define SQLITE_API
#endif
#ifndef SQLITE_CDECL
# define SQLITE_CDECL
#endif
#ifndef SQLITE_STDCALL
# define SQLITE_STDCALL
#endif

/*
** These no-op macros are used in front of interfaces to mark those
** interfaces as either deprecated or experimental.  New applications
** should not use deprecated interfaces - they are supported for backwards
** compatibility only.  Application writers should be aware that
** experimental interfaces are subject to change in point releases.
**
** These macros used to resolve to various kinds of compiler magic that
** would generate warning messages when they were used.  But that
** compiler magic ended up generating such a flurry of bug reports
** that we have taken it all out and gone back to using simple
** noop macros.
*/
#define SQLITE_DEPRECATED
#define SQLITE_EXPERIMENTAL

/*
** Ensure these symbols were not defined by some previous header file.
*/
#ifdef SQLITE_VERSION
# undef SQLITE_VERSION
#endif
#ifdef SQLITE_VERSION_NUMBER
# undef SQLITE_VERSION_NUMBER
#endif

/*
** CAPI3REF: Compile-Time Library Version Numbers
**
** ^(The [SQLITE_VERSION] C preprocessor macro in the sqlite3.h header
** evaluates to a string literal that is the SQLite version in the
** format "X.Y.Z" where X is the major version number (always 3 for
** SQLite3) and Y is the minor version number and Z is the release number.)^
** ^(The [SQLITE_VERSION_NUMBER] C preprocessor macro resolves to an integer
** with the value (X*1000000 + Y*1000 + Z) where X, Y, and Z are the same
** numbers used in [SQLITE_VERSION].)^
** The SQLITE_VERSION_NUMBER for any given release of SQLite will also
** be larger than the release from which it is derived.  Either Y will
** be held constant and Z will be incremented or else Y will be incremented
** and Z will be reset to zero.
**
** Since version 3.6.18, SQLite source code has been stored in the
** <a href="http://www.fossil-scm.org/">Fossil configuration management
** system</a>.  ^The SQLITE_SOURCE_ID macro evaluates to
** a string which identifies a particular check-in of SQLite
** within its configuration management system.  ^The SQLITE_SOURCE_ID
** string contains the date and time of the check-in (UTC) and an SHA1
** hash of the entire source tree.
**
** See also: [sqlite3_libversion()],
** [sqlite3_libversion_number()], [sqlite3_sourceid()],
** [sqlite_version()] and [sqlite_source_id()].
*/
#define SQLITE_VERSION        "3.13.0"
#define SQLITE_VERSION_NUMBER 3013000
#define SQLITE_SOURCE_ID      "2016-05-18 10:57:30 fc49f556e48970561d7ab6a2f24fdd7d9eb81ff2"

/*
** CAPI3REF: Run-Time Library Version Numbers
** KEYWORDS: sqlite3_version, sqlite3_sourceid
**
** These interfaces provide the same information as the [SQLITE_VERSION],
** [SQLITE_VERSION_NUMBER], and [SQLITE_SOURCE_ID] C preprocessor macros
** but are associated with the library instead of the header file.  ^(Cautious
** programmers might include assert() statements in their application to
** verify that values returned by these interfaces match the macros in
** the header, and thus ensure that the application is
** compiled with matching library and header files.
**
** <blockquote><pre>
** assert( sqlite3_libversion_number()==SQLITE_VERSION_NUMBER );
** assert( strcmp(sqlite3_sourceid(),SQLITE_SOURCE_ID)==0 );
** assert( strcmp(sqlite3_libversion(),SQLITE_VERSION)==0 );
** </pre></blockquote>)^
**
** ^The sqlite3_version[] string constant contains the text of [SQLITE_VERSION]
** macro.  ^The sqlite3_libversion() function returns a pointer to the
** to the sqlite3_version[] string constant.  The sqlite3_libversion()
** function is provided for use in DLLs since DLL users usually do not have
** direct access to string constants within the DLL.  ^The
** sqlite3_libversion_number() function returns an integer equal to
** [SQLITE_VERSION_NUMBER].  ^The sqlite3_sourceid() function returns 
** a pointer to a string constant whose value is the same as the 
** [SQLITE_SOURCE_ID] C preprocessor macro.
**
** See also: [sqlite_version()] and [sqlite_source_id()].
*/
SQLITE_API const char sqlite3_version[] = SQLITE_VERSION;
SQLITE_API const char *SQLITE_STDCALL sqlite3_libversion(void);
SQLITE_API const char *SQLITE_STDCALL sqlite3_sourceid(void);
SQLITE_API int SQLITE_STDCALL sqlite3_libversion_number(void);

/*
** CAPI3REF: Run-Time Library Compilation Options Diagnostics
**
** ^The sqlite3_compileoption_used() function returns 0 or 1 
** indicating whether the specified option was defined at 
** compile time.  ^The SQLITE_ prefix may be omitted from the 
** option name passed to sqlite3_compileoption_used().  
**
** ^The sqlite3_compileoption_get() function allows iterating
** over the list of options that were defined at compile time by
** returning the N-th compile time option string.  ^If N is out of range,
** sqlite3_compileoption_get() returns a NULL pointer.  ^The SQLITE_ 
** prefix is omitted from any strings returned by 
** sqlite3_compileoption_get().
**
** ^Support for the diagnostic functions sqlite3_compileoption_used()
** and sqlite3_compileoption_get() may be omitted by specifying the 
** [SQLITE_OMIT_COMPILEOPTION_DIAGS] option at compile time.
**
** See also: SQL functions [sqlite_compileoption_used()] and
** [sqlite_compileoption_get()] and the [compile_options pragma].
*/
#ifndef SQLITE_OMIT_COMPILEOPTION_DIAGS
SQLITE_API int SQLITE_STDCALL sqlite3_compileoption_used(const char *zOptName);
SQLITE_API const char *SQLITE_STDCALL sqlite3_compileoption_get(int N);
#endif

/*
** CAPI3REF: Test To See If The Library Is Threadsafe
**
** ^The sqlite3_threadsafe() function returns zero if and only if
** SQLite was compiled with mutexing code omitted due to the
** [SQLITE_THREADSAFE] compile-time option being set to 0.
**
** SQLite can be compiled with or without mutexes.  When
** the [SQLITE_THREADSAFE] C preprocessor macro is 1 or 2, mutexes
** are enabled and SQLite is threadsafe.  When the
** [SQLITE_THREADSAFE] macro is 0, 
** the mutexes are omitted.  Without the mutexes, it is not safe
** to use SQLite concurrently from more than one thread.
**
** Enabling mutexes incurs a measurable performance penalty.
** So if speed is of utmost importance, it makes sense to disable
** the mutexes.  But for maximum safety, mutexes should be enabled.
** ^The default behavior is for mutexes to be enabled.
**
** This interface can be used by an application to make sure that the
** version of SQLite that it is linking against was compiled with
** the desired setting of the [SQLITE_THREADSAFE] macro.
**
** This interface only reports on the compile-time mutex setting
** of the [SQLITE_THREADSAFE] flag.  If SQLite is compiled with
** SQLITE_THREADSAFE=1 or =2 then mutexes are enabled by default but
** can be fully or partially disabled using a call to [sqlite3_config()]
** with the verbs [SQLITE_CONFIG_SINGLETHREAD], [SQLITE_CONFIG_MULTITHREAD],
** or [SQLITE_CONFIG_SERIALIZED].  ^(The return value of the
** sqlite3_threadsafe() function shows only the compile-time setting of
** thread safety, not any run-time changes to that setting made by
** sqlite3_config(). In other words, the return value from sqlite3_threadsafe()
** is unchanged by calls to sqlite3_config().)^
**
** See the [threading mode] documentation for additional information.
*/
SQLITE_API int SQLITE_STDCALL sqlite3_threadsafe(void);

/*
** CAPI3REF: Database Connection Handle
** KEYWORDS: {database connection} {database connections}
**
** Each open SQLite database is represented by a pointer to an instance of
** the opaque structure named "sqlite3".  It is useful to think of an sqlite3
** pointer as an object.  The [sqlite3_open()], [sqlite3_open16()], and
** [sqlite3_open_v2()] interfaces are its constructors, and [sqlite3_close()]
** and [sqlite3_close_v2()] are its destructors.  There are many other
** interfaces (such as
** [sqlite3_prepare_v2()], [sqlite3_create_function()], and
** [sqlite3_busy_timeout()] to name but three) that are methods on an
** sqlite3 object.
*/
typedef struct sqlite3 sqlite3;

/*
** CAPI3REF: 64-Bit Integer Types
** KEYWORDS: sqlite_int64 sqlite_uint64
**
** Because there is no cross-platform way to specify 64-bit integer types
** SQLite includes typedefs for 64-bit signed and unsigned integers.
**
** The sqlite3_int64 and sqlite3_uint64 are the preferred type definitions.
** The sqlite_int64 and sqlite_uint64 types are supported for backwards
** compatibility only.
**
** ^The sqlite3_int64 and sqlite_int64 types can store integer values
** between -9223372036854775808 and +9223372036854775807 inclusive.  ^The
** sqlite3_uint64 and sqlite_uint64 types can store integer values 
** between 0 and +18446744073709551615 inclusive.
*/
#ifdef SQLITE_INT64_TYPE
  typedef SQLITE_INT64_TYPE sqlite_int64;
  typedef unsigned SQLITE_INT64_TYPE sqlite_uint64;
#elif defined(_MSC_VER) || defined(__BORLANDC__)
  typedef __int64 sqlite_int64;
  typedef unsigned __int64 sqlite_uint64;
#else
  typedef long long int sqlite_int64;
  typedef unsigned long long int sqlite_uint64;
#endif
typedef sqlite_int64 sqlite3_int64;
typedef sqlite_uint64 sqlite3_uint64;

/*
** If compiling for a processor that lacks floating point support,
** substitute integer for floating-point.
*/
#ifdef SQLITE_OMIT_FLOATING_POINT
# define double sqlite3_int64
#endif

/*
** CAPI3REF: Closing A Database Connection
** DESTRUCTOR: sqlite3
**
** ^The sqlite3_close() and sqlite3_close_v2() routines are destructors
** for the [sqlite3] object.
** ^Calls to sqlite3_close() and sqlite3_close_v2() return [SQLITE_OK] if
** the [sqlite3] object is successfully destroyed and all associated
** resources are deallocated.
**
** ^If the database connection is associated with unfinalized prepared
** statements or unfinished sqlite3_backup objects then sqlite3_close()
** will leave the database connection open and return [SQLITE_BUSY].
** ^If sqlite3_close_v2() is called with unfinalized prepared statements
** and/or unfinished sqlite3_backups, then the database connection becomes
** an unusable "zombie" which will automatically be deallocated when the
** last prepared statement is finalized or the last sqlite3_backup is
** finished.  The sqlite3_close_v2() interface is intended for use with
** host languages that are garbage collected, and where the order in which
** destructors are called is arbitrary.
**
** Applications should [sqlite3_finalize | finalize] all [prepared statements],
** [sqlite3_blob_close | close] all [BLOB handles], and 
** [sqlite3_backup_finish | finish] all [sqlite3_backup] objects associated
** with the [sqlite3] object prior to attempting to close the object.  ^If
** sqlite3_close_v2() is called on a [database connection] that still has
** outstanding [prepared statements], [BLOB handles], and/or
** [sqlite3_backup] objects then it returns [SQLITE_OK] and the deallocation
** of resources is deferred until all [prepared statements], [BLOB handles],
** and [sqlite3_backup] objects are also destroyed.
**
** ^If an [sqlite3] object is destroyed while a transaction is open,
** the transaction is automatically rolled back.
**
** The C parameter to [sqlite3_close(C)] and [sqlite3_close_v2(C)]
** must be either a NULL
** pointer or an [sqlite3] object pointer obtained
** from [sqlite3_open()], [sqlite3_open16()], or
** [sqlite3_open_v2()], and not previously closed.
** ^Calling sqlite3_close() or sqlite3_close_v2() with a NULL pointer
** argument is a harmless no-op.
*/
SQLITE_API int SQLITE_STDCALL sqlite3_close(sqlite3*);
SQLITE_API int SQLITE_STDCALL sqlite3_close_v2(sqlite3*);

/*
** The type for a callback function.
** This is legacy and deprecated.  It is included for historical
** compatibility and is not documented.
*/
typedef int (*sqlite3_callback)(void*,int,char**, char**);

/*
** CAPI3REF: One-Step Query Execution Interface
** METHOD: sqlite3
**
** The sqlite3_exec() interface is a convenience wrapper around
** [sqlite3_prepare_v2()], [sqlite3_step()], and [sqlite3_finalize()],
** that allows an application to run multiple statements of SQL
** without having to use a lot of C code. 
**
** ^The sqlite3_exec() interface runs zero or more UTF-8 encoded,
** semicolon-separate SQL statements passed into its 2nd argument,
** in the context of the [database connection] passed in as its 1st
** argument.  ^If the callback function of the 3rd argument to
** sqlite3_exec() is not NULL, then it is invoked for each result row
** coming out of the evaluated SQL statements.  ^The 4th argument to
** sqlite3_exec() is relayed through to the 1st argument of each
** callback invocation.  ^If the callback pointer to sqlite3_exec()
** is NULL, then no callback is ever invoked and result rows are
** ignored.
**
** ^If an error occurs while evaluating the SQL statements passed into
** sqlite3_exec(), then execution of the current statement stops and
** subsequent statements are skipped.  ^If the 5th parameter to sqlite3_exec()
** is not NULL then any error message is written into memory obtained
** from [sqlite3_malloc()] and passed back through the 5th parameter.
** To avoid memory leaks, the application should invoke [sqlite3_free()]
** on error message strings returned through the 5th parameter of
** sqlite3_exec() after the error message string is no longer needed.
** ^If the 5th parameter to sqlite3_exec() is not NULL and no errors
** occur, then sqlite3_exec() sets the pointer in its 5th parameter to
** NULL before returning.
**
** ^If an sqlite3_exec() callback returns non-zero, the sqlite3_exec()
** routine returns SQLITE_ABORT without invoking the callback again and
** without running any subsequent SQL statements.
**
** ^The 2nd argument to the sqlite3_exec() callback function is the
** number of columns in the result.  ^The 3rd argument to the sqlite3_exec()
** callback is an array of pointers to strings obtained as if from
** [sqlite3_column_text()], one for each column.  ^If an element of a
** result row is NULL then the corresponding string pointer for the
** sqlite3_exec() callback is a NULL pointer.  ^The 4th argument to the
** sqlite3_exec() callback is an array of pointers to strings where each
** entry represents the name of corresponding result column as obtained
** from [sqlite3_column_name()].
**
** ^If the 2nd parameter to sqlite3_exec() is a NULL pointer, a pointer
** to an empty string, or a pointer that contains only whitespace and/or 
** SQL comments, then no SQL statements are evaluated and the database
** is not changed.
**
** Restrictions:
**
** <ul>
** <li> The application must ensure that the 1st parameter to sqlite3_exec()
**      is a valid and open [database connection].
** <li> The application must not close the [database connection] specified by
**      the 1st parameter to sqlite3_exec() while sqlite3_exec() is running.
** <li> The application must not modify the SQL statement text passed into
**      the 2nd parameter of sqlite3_exec() while sqlite3_exec() is running.
** </ul>
*/
SQLITE_API int SQLITE_STDCALL sqlite3_exec(
  sqlite3*,                                  /* An open database */
  const char *sql,                           /* SQL to be evaluated */
  int (*callback)(void*,int,char**,char**),  /* Callback function */
  void *,                                    /* 1st argument to callback */
  char **errmsg                              /* Error msg written here */
);

/*
** CAPI3REF: Result Codes
** KEYWORDS: {result code definitions}
**
** Many SQLite functions return an integer result code from the set shown
** here in order to indicate success or failure.
**
** New error codes may be added in future versions of SQLite.
**
** See also: [extended result code definitions]
*/
#define SQLITE_OK           0   /* Successful result */
/* beginning-of-error-codes */
#define SQLITE_ERROR        1   /* SQL error or missing database */
#define SQLITE_INTERNAL     2   /* Internal logic error in SQLite */
#define SQLITE_PERM         3   /* Access permission denied */
#define SQLITE_ABORT        4   /* Callback routine requested an abort */
#define SQLITE_BUSY         5   /* The database file is locked */
#define SQLITE_LOCKED       6   /* A table in the database is locked */
#define SQLITE_NOMEM        7   /* A malloc() failed */
#define SQLITE_READONLY     8   /* Attempt to write a readonly database */
#define SQLITE_INTERRUPT    9   /* Operation terminated by sqlite3_interrupt()*/
#define SQLITE_IOERR       10   /* Some kind of disk I/O error occurred */
#define SQLITE_CORRUPT     11   /* The database disk image is malformed */
#define SQLITE_NOTFOUND    12   /* Unknown opcode in sqlite3_file_control() */
#define SQLITE_FULL        13   /* Insertion failed because database is full */
#define SQLITE_CANTOPEN    14   /* Unable to open the database file */
#define SQLITE_PROTOCOL    15   /* Database lock protocol error */
#define SQLITE_EMPTY       16   /* Database is empty */
#define SQLITE_SCHEMA      17   /* The database schema changed */
#define SQLITE_TOOBIG      18   /* String or BLOB exceeds size limit */
#define SQLITE_CONSTRAINT  19   /* Abort due to constraint violation */
#define SQLITE_MISMATCH    20   /* Data type mismatch */
#define SQLITE_MISUSE      21   /* Library used incorrectly */
#define SQLITE_NOLFS       22   /* Uses OS features not supported on host */
#define SQLITE_AUTH        23   /* Authorization denied */
#define SQLITE_FORMAT      24   /* Auxiliary database format error */
#define SQLITE_RANGE       25   /* 2nd parameter to sqlite3_bind out of range */
#define SQLITE_NOTADB      26   /* File opened that is not a database file */
#define SQLITE_NOTICE      27   /* Notifications from sqlite3_log() */
#define SQLITE_WARNING     28   /* Warnings from sqlite3_log() */
#define SQLITE_ROW         100  /* sqlite3_step() has another row ready */
#define SQLITE_DONE        101  /* sqlite3_step() has finished executing */
/* end-of-error-codes */

/*
** CAPI3REF: Extended Result Codes
** KEYWORDS: {extended result code definitions}
**
** In its default configuration, SQLite API routines return one of 30 integer
** [result codes].  However, experience has shown that many of
** these result codes are too coarse-grained.  They do not provide as
** much information about problems as programmers might like.  In an effort to
** address this, newer versions of SQLite (version 3.3.8 and later) include
** support for additional result codes that provide more detailed information
** about errors. These [extended result codes] are enabled or disabled
** on a per database connection basis using the
** [sqlite3_extended_result_codes()] API.  Or, the extended code for
** the most recent error can be obtained using
** [sqlite3_extended_errcode()].
*/
#define SQLITE_IOERR_READ              (SQLITE_IOERR | (1<<8))
#define SQLITE_IOERR_SHORT_READ        (SQLITE_IOERR | (2<<8))
#define SQLITE_IOERR_WRITE             (SQLITE_IOERR | (3<<8))
#define SQLITE_IOERR_FSYNC             (SQLITE_IOERR | (4<<8))
#define SQLITE_IOERR_DIR_FSYNC         (SQLITE_IOERR | (5<<8))
#define SQLITE_IOERR_TRUNCATE          (SQLITE_IOERR | (6<<8))
#define SQLITE_IOERR_FSTAT             (SQLITE_IOERR | (7<<8))
#define SQLITE_IOERR_UNLOCK            (SQLITE_IOERR | (8<<8))
#define SQLITE_IOERR_RDLOCK            (SQLITE_IOERR | (9<<8))
#define SQLITE_IOERR_DELETE            (SQLITE_IOERR | (10<<8))
#define SQLITE_IOERR_BLOCKED           (SQLITE_IOERR | (11<<8))
#define SQLITE_IOERR_NOMEM             (SQLITE_IOERR | (12<<8))
#define SQLITE_IOERR_ACCESS            (SQLITE_IOERR | (13<<8))
#define SQLITE_IOERR_CHECKRESERVEDLOCK (SQLITE_IOERR | (14<<8))
#define SQLITE_IOERR_LOCK              (SQLITE_IOERR | (15<<8))
#define SQLITE_IOERR_CLOSE             (SQLITE_IOERR | (16<<8))
#define SQLITE_IOERR_DIR_CLOSE         (SQLITE_IOERR | (17<<8))
#define SQLITE_IOERR_SHMOPEN           (SQLITE_IOERR | (18<<8))
#define SQLITE_IOERR_SHMSIZE           (SQLITE_IOERR | (19<<8))
#define SQLITE_IOERR_SHMLOCK           (SQLITE_IOERR | (20<<8))
#define SQLITE_IOERR_SHMMAP            (SQLITE_IOERR | (21<<8))
#define SQLITE_IOERR_SEEK              (SQLITE_IOERR | (22<<8))
#define SQLITE_IOERR_DELETE_NOENT      (SQLITE_IOERR | (23<<8))
#define SQLITE_IOERR_MMAP              (SQLITE_IOERR | (24<<8))
#define SQLITE_IOERR_GETTEMPPATH       (SQLITE_IOERR | (25<<8))
#define SQLITE_IOERR_CONVPATH          (SQLITE_IOERR | (26<<8))
#define SQLITE_IOERR_VNODE             (SQLITE_IOERR | (27<<8))
#define SQLITE_IOERR_AUTH              (SQLITE_IOERR | (28<<8))
#define SQLITE_LOCKED_SHAREDCACHE      (SQLITE_LOCKED |  (1<<8))
#define SQLITE_BUSY_RECOVERY           (SQLITE_BUSY   |  (1<<8))
#define SQLITE_BUSY_SNAPSHOT           (SQLITE_BUSY   |  (2<<8))
#define SQLITE_CANTOPEN_NOTEMPDIR      (SQLITE_CANTOPEN | (1<<8))
#define SQLITE_CANTOPEN_ISDIR          (SQLITE_CANTOPEN | (2<<8))
#define SQLITE_CANTOPEN_FULLPATH       (SQLITE_CANTOPEN | (3<<8))
#define SQLITE_CANTOPEN_CONVPATH       (SQLITE_CANTOPEN | (4<<8))
#define SQLITE_CORRUPT_VTAB            (SQLITE_CORRUPT | (1<<8))
#define SQLITE_READONLY_RECOVERY       (SQLITE_READONLY | (1<<8))
#define SQLITE_READONLY_CANTLOCK       (SQLITE_READONLY | (2<<8))
#define SQLITE_READONLY_ROLLBACK       (SQLITE_READONLY | (3<<8))
#define SQLITE_READONLY_DBMOVED        (SQLITE_READONLY | (4<<8))
#define SQLITE_ABORT_ROLLBACK          (SQLITE_ABORT | (2<<8))
#define SQLITE_CONSTRAINT_CHECK        (SQLITE_CONSTRAINT | (1<<8))
#define SQLITE_CONSTRAINT_COMMITHOOK   (SQLITE_CONSTRAINT | (2<<8))
#define SQLITE_CONSTRAINT_FOREIGNKEY   (SQLITE_CONSTRAINT | (3<<8))
#define SQLITE_CONSTRAINT_FUNCTION     (SQLITE_CONSTRAINT | (4<<8))
#define SQLITE_CONSTRAINT_NOTNULL      (SQLITE_CONSTRAINT | (5<<8))
#define SQLITE_CONSTRAINT_PRIMARYKEY   (SQLITE_CONSTRAINT | (6<<8))
#define SQLITE_CONSTRAINT_TRIGGER      (SQLITE_CONSTRAINT | (7<<8))
#define SQLITE_CONSTRAINT_UNIQUE       (SQLITE_CONSTRAINT | (8<<8))
#define SQLITE_CONSTRAINT_VTAB         (SQLITE_CONSTRAINT | (9<<8))
#define SQLITE_CONSTRAINT_ROWID        (SQLITE_CONSTRAINT |(10<<8))
#define SQLITE_NOTICE_RECOVER_WAL      (SQLITE_NOTICE | (1<<8))
#define SQLITE_NOTICE_RECOVER_ROLLBACK (SQLITE_NOTICE | (2<<8))
#define SQLITE_WARNING_AUTOINDEX       (SQLITE_WARNING | (1<<8))
#define SQLITE_AUTH_USER               (SQLITE_AUTH | (1<<8))

/*
** CAPI3REF: Flags For File Open Operations
**
** These bit values are intended for use in the
** 3rd parameter to the [sqlite3_open_v2()] interface and
** in the 4th parameter to the [sqlite3_vfs.xOpen] method.
*/
#define SQLITE_OPEN_READONLY         0x00000001  /* Ok for sqlite3_open_v2() */
#define SQLITE_OPEN_READWRITE        0x00000002  /* Ok for sqlite3_open_v2() */
#define SQLITE_OPEN_CREATE           0x00000004  /* Ok for sqlite3_open_v2() */
#define SQLITE_OPEN_DELETEONCLOSE    0x00000008  /* VFS only */
#define SQLITE_OPEN_EXCLUSIVE        0x00000010  /* VFS only */
#define SQLITE_OPEN_AUTOPROXY        0x00000020  /* VFS only */
#define SQLITE_OPEN_URI              0x00000040  /* Ok for sqlite3_open_v2() */
#define SQLITE_OPEN_MEMORY           0x00000080  /* Ok for sqlite3_open_v2() */
#define SQLITE_OPEN_MAIN_DB          0x00000100  /* VFS only */
#define SQLITE_OPEN_TEMP_DB          0x00000200  /* VFS only */
#define SQLITE_OPEN_TRANSIENT_DB     0x00000400  /* VFS only */
#define SQLITE_OPEN_MAIN_JOURNAL     0x00000800  /* VFS only */
#define SQLITE_OPEN_TEMP_JOURNAL     0x00001000  /* VFS only */
#define SQLITE_OPEN_SUBJOURNAL       0x00002000  /* VFS only */
#define SQLITE_OPEN_MASTER_JOURNAL   0x00004000  /* VFS only */
#define SQLITE_OPEN_NOMUTEX          0x00008000  /* Ok for sqlite3_open_v2() */
#define SQLITE_OPEN_FULLMUTEX        0x00010000  /* Ok for sqlite3_open_v2() */
#define SQLITE_OPEN_SHAREDCACHE      0x00020000  /* Ok for sqlite3_open_v2() */
#define SQLITE_OPEN_PRIVATECACHE     0x00040000  /* Ok for sqlite3_open_v2() */
#define SQLITE_OPEN_WAL              0x00080000  /* VFS only */

/* Reserved:                         0x00F00000 */

/*
** CAPI3REF: Device Characteristics
**
** The xDeviceCharacteristics method of the [sqlite3_io_methods]
** object returns an integer which is a vector of these
** bit values expressing I/O characteristics of the mass storage
** device that holds the file that the [sqlite3_io_methods]
** refers to.
**
** The SQLITE_IOCAP_ATOMIC property means that all writes of
** any size are atomic.  The SQLITE_IOCAP_ATOMICnnn values
** mean that writes of blocks that are nnn bytes in size and
** are aligned to an address which is an integer multiple of
** nnn are atomic.  The SQLITE_IOCAP_SAFE_APPEND value means
** that when data is appended to a file, the data is appended
** first then the size of the file is extended, never the other
** way around.  The SQLITE_IOCAP_SEQUENTIAL property means that
** information is written to disk in the same order as calls
** to xWrite().  The SQLITE_IOCAP_POWERSAFE_OVERWRITE property means that
** after reboot following a crash or power loss, the only bytes in a
** file that were written at the application level might have changed
** and that adjacent bytes, even bytes within the same sector are
** guaranteed to be unchanged.  The SQLITE_IOCAP_UNDELETABLE_WHEN_OPEN
** flag indicate that a file cannot be deleted when open.  The
** SQLITE_IOCAP_IMMUTABLE flag indicates that the file is on
** read-only media and cannot be changed even by processes with
** elevated privileges.
*/
#define SQLITE_IOCAP_ATOMIC                 0x00000001
#define SQLITE_IOCAP_ATOMIC512              0x00000002
#define SQLITE_IOCAP_ATOMIC1K               0x00000004
#define SQLITE_IOCAP_ATOMIC2K               0x00000008
#define SQLITE_IOCAP_ATOMIC4K               0x00000010
#define SQLITE_IOCAP_ATOMIC8K               0x00000020
#define SQLITE_IOCAP_ATOMIC16K              0x00000040
#define SQLITE_IOCAP_ATOMIC32K              0x00000080
#define SQLITE_IOCAP_ATOMIC64K              0x00000100
#define SQLITE_IOCAP_SAFE_APPEND            0x00000200
#define SQLITE_IOCAP_SEQUENTIAL             0x00000400
#define SQLITE_IOCAP_UNDELETABLE_WHEN_OPEN  0x00000800
#define SQLITE_IOCAP_POWERSAFE_OVERWRITE    0x00001000
#define SQLITE_IOCAP_IMMUTABLE              0x00002000

/*
** CAPI3REF: File Locking Levels
**
** SQLite uses one of these integer values as the second
** argument to calls it makes to the xLock() and xUnlock() methods
** of an [sqlite3_io_methods] object.
*/
#define SQLITE_LOCK_NONE          0
#define SQLITE_LOCK_SHARED        1
#define SQLITE_LOCK_RESERVED      2
#define SQLITE_LOCK_PENDING       3
#define SQLITE_LOCK_EXCLUSIVE     4

/*
** CAPI3REF: Synchronization Type Flags
**
** When SQLite invokes the xSync() method of an
** [sqlite3_io_methods] object it uses a combination of
** these integer values as the second argument.
**
** When the SQLITE_SYNC_DATAONLY flag is used, it means that the
** sync operation only needs to flush data to mass storage.  Inode
** information need not be flushed. If the lower four bits of the flag
** equal SQLITE_SYNC_NORMAL, that means to use normal fsync() semantics.
** If the lower four bits equal SQLITE_SYNC_FULL, that means
** to use Mac OS X style fullsync instead of fsync().
**
** Do not confuse the SQLITE_SYNC_NORMAL and SQLITE_SYNC_FULL flags
** with the [PRAGMA synchronous]=NORMAL and [PRAGMA synchronous]=FULL
** settings.  The [synchronous pragma] determines when calls to the
** xSync VFS method occur and applies uniformly across all platforms.
** The SQLITE_SYNC_NORMAL and SQLITE_SYNC_FULL flags determine how
** energetic or rigorous or forceful the sync operations are and
** only make a difference on Mac OSX for the default SQLite code.
** (Third-party VFS implementations might also make the distinction
** between SQLITE_SYNC_NORMAL and SQLITE_SYNC_FULL, but among the
** operating systems natively supported by SQLite, only Mac OSX
** cares about the difference.)
*/
#define SQLITE_SYNC_NORMAL        0x00002
#define SQLITE_SYNC_FULL          0x00003
#define SQLITE_SYNC_DATAONLY      0x00010

/*
** CAPI3REF: OS Interface Open File Handle
**
** An [sqlite3_file] object represents an open file in the 
** [sqlite3_vfs | OS interface layer].  Individual OS interface
** implementations will
** want to subclass this object by appending additional fields
** for their own use.  The pMethods entry is a pointer to an
** [sqlite3_io_methods] object that defines methods for performing
** I/O operations on the open file.
*/
typedef struct sqlite3_file sqlite3_file;
struct sqlite3_file {
  const struct sqlite3_io_methods *pMethods;  /* Methods for an open file */
};

/*
** CAPI3REF: OS Interface File Virtual Methods Object
**
** Every file opened by the [sqlite3_vfs.xOpen] method populates an
** [sqlite3_file] object (or, more commonly, a subclass of the
** [sqlite3_file] object) with a pointer to an instance of this object.
** This object defines the methods used to perform various operations
** against the open file represented by the [sqlite3_file] object.
**
** If the [sqlite3_vfs.xOpen] method sets the sqlite3_file.pMethods element 
** to a non-NULL pointer, then the sqlite3_io_methods.xClose method
** may be invoked even if the [sqlite3_vfs.xOpen] reported that it failed.  The
** only way to prevent a call to xClose following a failed [sqlite3_vfs.xOpen]
** is for the [sqlite3_vfs.xOpen] to set the sqlite3_file.pMethods element
** to NULL.
**
** The flags argument to xSync may be one of [SQLITE_SYNC_NORMAL] or
** [SQLITE_SYNC_FULL].  The first choice is the normal fsync().
** The second choice is a Mac OS X style fullsync.  The [SQLITE_SYNC_DATAONLY]
** flag may be ORed in to indicate that only the data of the file
** and not its inode needs to be synced.
**
** The integer values to xLock() and xUnlock() are one of
** <ul>
** <li> [SQLITE_LOCK_NONE],
** <li> [SQLITE_LOCK_SHARED],
** <li> [SQLITE_LOCK_RESERVED],
** <li> [SQLITE_LOCK_PENDING], or
** <li> [SQLITE_LOCK_EXCLUSIVE].
** </ul>
** xLock() increases the lock. xUnlock() decreases the lock.
** The xCheckReservedLock() method checks whether any database connection,
** either in this process or in some other process, is holding a RESERVED,
** PENDING, or EXCLUSIVE lock on the file.  It returns true
** if such a lock exists and false otherwise.
**
** The xFileControl() method is a generic interface that allows custom
** VFS implementations to directly control an open file using the
** [sqlite3_file_control()] interface.  The second "op" argument is an
** integer opcode.  The third argument is a generic pointer intended to
** point to a structure that may contain arguments or space in which to
** write return values.  Potential uses for xFileControl() might be
** functions to enable blocking locks with timeouts, to change the
** locking strategy (for example to use dot-file locks), to inquire
** about the status of a lock, or to break stale locks.  The SQLite
** core reserves all opcodes less than 100 for its own use.
** A [file control opcodes | list of opcodes] less than 100 is available.
** Applications that define a custom xFileControl method should use opcodes
** greater than 100 to avoid conflicts.  VFS implementations should
** return [SQLITE_NOTFOUND] for file control opcodes that they do not
** recognize.
**
** The xSectorSize() method returns the sector size of the
** device that underlies the file.  The sector size is the
** minimum write that can be performed without disturbing
** other bytes in the file.  The xDeviceCharacteristics()
** method returns a bit vector describing behaviors of the
** underlying device:
**
** <ul>
** <li> [SQLITE_IOCAP_ATOMIC]
** <li> [SQLITE_IOCAP_ATOMIC512]
** <li> [SQLITE_IOCAP_ATOMIC1K]
** <li> [SQLITE_IOCAP_ATOMIC2K]
** <li> [SQLITE_IOCAP_ATOMIC4K]
** <li> [SQLITE_IOCAP_ATOMIC8K]
** <li> [SQLITE_IOCAP_ATOMIC16K]
** <li> [SQLITE_IOCAP_ATOMIC32K]
** <li> [SQLITE_IOCAP_ATOMIC64K]
** <li> [SQLITE_IOCAP_SAFE_APPEND]
** <li> [SQLITE_IOCAP_SEQUENTIAL]
** </ul>
**
** The SQLITE_IOCAP_ATOMIC property means that all writes of
** any size are atomic.  The SQLITE_IOCAP_ATOMICnnn values
** mean that writes of blocks that are nnn bytes in size and
** are aligned to an address which is an integer multiple of
** nnn are atomic.  The SQLITE_IOCAP_SAFE_APPEND value means
** that when data is appended to a file, the data is appended
** first then the size of the file is extended, never the other
** way around.  The SQLITE_IOCAP_SEQUENTIAL property means that
** information is written to disk in the same order as calls
** to xWrite().
**
** If xRead() returns SQLITE_IOERR_SHORT_READ it must also fill
** in the unread portions of the buffer with zeros.  A VFS that
** fails to zero-fill short reads might seem to work.  However,
** failure to zero-fill short reads will eventually lead to
** database corruption.
*/
typedef struct sqlite3_io_methods sqlite3_io_methods;
struct sqlite3_io_methods {
  int iVersion;
  int (*xClose)(sqlite3_file*);
  int (*xRead)(sqlite3_file*, void*, int iAmt, sqlite3_int64 iOfst);
  int (*xWrite)(sqlite3_file*, const void*, int iAmt, sqlite3_int64 iOfst);
  int (*xTruncate)(sqlite3_file*, sqlite3_int64 size);
  int (*xSync)(sqlite3_file*, int flags);
  int (*xFileSize)(sqlite3_file*, sqlite3_int64 *pSize);
  int (*xLock)(sqlite3_file*, int);
  int (*xUnlock)(sqlite3_file*, int);
  int (*xCheckReservedLock)(sqlite3_file*, int *pResOut);
  int (*xFileControl)(sqlite3_file*, int op, void *pArg);
  int (*xSectorSize)(sqlite3_file*);
  int (*xDeviceCharacteristics)(sqlite3_file*);
  /* Methods above are valid for version 1 */
  int (*xShmMap)(sqlite3_file*, int iPg, int pgsz, int, void volatile**);
  int (*xShmLock)(sqlite3_file*, int offset, int n, int flags);
  void (*xShmBarrier)(sqlite3_file*);
  int (*xShmUnmap)(sqlite3_file*, int deleteFlag);
  /* Methods above are valid for version 2 */
  int (*xFetch)(sqlite3_file*, sqlite3_int64 iOfst, int iAmt, void **pp);
  int (*xUnfetch)(sqlite3_file*, sqlite3_int64 iOfst, void *p);
  /* Methods above are valid for version 3 */
  /* Additional methods may be added in future releases */
};

/*
** CAPI3REF: Standard File Control Opcodes
** KEYWORDS: {file control opcodes} {file control opcode}
**
** These integer constants are opcodes for the xFileControl method
** of the [sqlite3_io_methods] object and for the [sqlite3_file_control()]
** interface.
**
** <ul>
** <li>[[SQLITE_FCNTL_LOCKSTATE]]
** The [SQLITE_FCNTL_LOCKSTATE] opcode is used for debugging.  This
** opcode causes the xFileControl method to write the current state of
** the lock (one of [SQLITE_LOCK_NONE], [SQLITE_LOCK_SHARED],
** [SQLITE_LOCK_RESERVED], [SQLITE_LOCK_PENDING], or [SQLITE_LOCK_EXCLUSIVE])
** into an integer that the pArg argument points to. This capability
** is used during testing and is only available when the SQLITE_TEST
** compile-time option is used.
**
** <li>[[SQLITE_FCNTL_SIZE_HINT]]
** The [SQLITE_FCNTL_SIZE_HINT] opcode is used by SQLite to give the VFS
** layer a hint of how large the database file will grow to be during the
** current transaction.  This hint is not guaranteed to be accurate but it
** is often close.  The underlying VFS might choose to preallocate database
** file space based on this hint in order to help writes to the database
** file run faster.
**
** <li>[[SQLITE_FCNTL_CHUNK_SIZE]]
** The [SQLITE_FCNTL_CHUNK_SIZE] opcode is used to request that the VFS
** extends and truncates the database file in chunks of a size specified
** by the user. The fourth argument to [sqlite3_file_control()] should 
** point to an integer (type int) containing the new chunk-size to use
** for the nominated database. Allocating database file space in large
** chunks (say 1MB at a time), may reduce file-system fragmentation and
** improve performance on some systems.
**
** <li>[[SQLITE_FCNTL_FILE_POINTER]]
** The [SQLITE_FCNTL_FILE_POINTER] opcode is used to obtain a pointer
** to the [sqlite3_file] object associated with a particular database
** connection.  See also [SQLITE_FCNTL_JOURNAL_POINTER].
**
** <li>[[SQLITE_FCNTL_JOURNAL_POINTER]]
** The [SQLITE_FCNTL_JOURNAL_POINTER] opcode is used to obtain a pointer
** to the [sqlite3_file] object associated with the journal file (either
** the [rollback journal] or the [write-ahead log]) for a particular database
** connection.  See also [SQLITE_FCNTL_FILE_POINTER].
**
** <li>[[SQLITE_FCNTL_SYNC_OMITTED]]
** No longer in use.
**
** <li>[[SQLITE_FCNTL_SYNC]]
** The [SQLITE_FCNTL_SYNC] opcode is generated internally by SQLite and
** sent to the VFS immediately before the xSync method is invoked on a
** database file descriptor. Or, if the xSync method is not invoked 
** because the user has configured SQLite with 
** [PRAGMA synchronous | PRAGMA synchronous=OFF] it is invoked in place 
** of the xSync method. In most cases, the pointer argument passed with
** this file-control is NULL. However, if the database file is being synced
** as part of a multi-database commit, the argument points to a nul-terminated
** string containing the transactions master-journal file name. VFSes that 
** do not need this signal should silently ignore this opcode. Applications 
** should not call [sqlite3_file_control()] with this opcode as doing so may 
** disrupt the operation of the specialized VFSes that do require it.  
**
** <li>[[SQLITE_FCNTL_COMMIT_PHASETWO]]
** The [SQLITE_FCNTL_COMMIT_PHASETWO] opcode is generated internally by SQLite
** and sent to the VFS after a transaction has been committed immediately
** but before the database is unlocked. VFSes that do not need this signal
** should silently ignore this opcode. Applications should not call
** [sqlite3_file_control()] with this opcode as doing so may disrupt the 
** operation of the specialized VFSes that do require it.  
**
** <li>[[SQLITE_FCNTL_WIN32_AV_RETRY]]
** ^The [SQLITE_FCNTL_WIN32_AV_RETRY] opcode is used to configure automatic
** retry counts and intervals for certain disk I/O operations for the
** windows [VFS] in order to provide robustness in the presence of
** anti-virus programs.  By default, the windows VFS will retry file read,
** file write, and file delete operations up to 10 times, with a delay
** of 25 milliseconds before the first retry and with the delay increasing
** by an additional 25 milliseconds with each subsequent retry.  This
** opcode allows these two values (10 retries and 25 milliseconds of delay)
** to be adjusted.  The values are changed for all database connections
** within the same process.  The argument is a pointer to an array of two
** integers where the first integer i the new retry count and the second
** integer is the delay.  If either integer is negative, then the setting
** is not changed but instead the prior value of that setting is written
** into the array entry, allowing the current retry settings to be
** interrogated.  The zDbName parameter is ignored.
**
** <li>[[SQLITE_FCNTL_PERSIST_WAL]]
** ^The [SQLITE_FCNTL_PERSIST_WAL] opcode is used to set or query the
** persistent [WAL | Write Ahead Log] setting.  By default, the auxiliary
** write ahead log and shared memory files used for transaction control
** are automatically deleted when the latest connection to the database
** closes.  Setting persistent WAL mode causes those files to persist after
** close.  Persisting the files is useful when other processes that do not
** have write permission on the directory containing the database file want
** to read the database file, as the WAL and shared memory files must exist
** in order for the database to be readable.  The fourth parameter to
** [sqlite3_file_control()] for this opcode should be a pointer to an integer.
** That integer is 0 to disable persistent WAL mode or 1 to enable persistent
** WAL mode.  If the integer is -1, then it is overwritten with the current
** WAL persistence setting.
**
** <li>[[SQLITE_FCNTL_POWERSAFE_OVERWRITE]]
** ^The [SQLITE_FCNTL_POWERSAFE_OVERWRITE] opcode is used to set or query the
** persistent "powersafe-overwrite" or "PSOW" setting.  The PSOW setting
** determines the [SQLITE_IOCAP_POWERSAFE_OVERWRITE] bit of the
** xDeviceCharacteristics methods. The fourth parameter to
** [sqlite3_file_control()] for this opcode should be a pointer to an integer.
** That integer is 0 to disable zero-damage mode or 1 to enable zero-damage
** mode.  If the integer is -1, then it is overwritten with the current
** zero-damage mode setting.
**
** <li>[[SQLITE_FCNTL_OVERWRITE]]
** ^The [SQLITE_FCNTL_OVERWRITE] opcode is invoked by SQLite after opening
** a write transaction to indicate that, unless it is rolled back for some
** reason, the entire database file will be overwritten by the current 
** transaction. This is used by VACUUM operations.
**
** <li>[[SQLITE_FCNTL_VFSNAME]]
** ^The [SQLITE_FCNTL_VFSNAME] opcode can be used to obtain the names of
** all [VFSes] in the VFS stack.  The names are of all VFS shims and the
** final bottom-level VFS are written into memory obtained from 
** [sqlite3_malloc()] and the result is stored in the char* variable
** that the fourth parameter of [sqlite3_file_control()] points to.
** The caller is responsible for freeing the memory when done.  As with
** all file-control actions, there is no guarantee that this will actually
** do anything.  Callers should initialize the char* variable to a NULL
** pointer in case this file-control is not implemented.  This file-control
** is intended for diagnostic use only.
**
** <li>[[SQLITE_FCNTL_VFS_POINTER]]
** ^The [SQLITE_FCNTL_VFS_POINTER] opcode finds a pointer to the top-level
** [VFSes] currently in use.  ^(The argument X in
** sqlite3_file_control(db,SQLITE_FCNTL_VFS_POINTER,X) must be
** of type "[sqlite3_vfs] **".  This opcodes will set *X
** to a pointer to the top-level VFS.)^
** ^When there are multiple VFS shims in the stack, this opcode finds the
** upper-most shim only.
**
** <li>[[SQLITE_FCNTL_PRAGMA]]
** ^Whenever a [PRAGMA] statement is parsed, an [SQLITE_FCNTL_PRAGMA] 
** file control is sent to the open [sqlite3_file] object corresponding
** to the database file to which the pragma statement refers. ^The argument
** to the [SQLITE_FCNTL_PRAGMA] file control is an array of
** pointers to strings (char**) in which the second element of the array
** is the name of the pragma and the third element is the argument to the
** pragma or NULL if the pragma has no argument.  ^The handler for an
** [SQLITE_FCNTL_PRAGMA] file control can optionally make the first element
** of the char** argument point to a string obtained from [sqlite3_mprintf()]
** or the equivalent and that string will become the result of the pragma or
** the error message if the pragma fails. ^If the
** [SQLITE_FCNTL_PRAGMA] file control returns [SQLITE_NOTFOUND], then normal 
** [PRAGMA] processing continues.  ^If the [SQLITE_FCNTL_PRAGMA]
** file control returns [SQLITE_OK], then the parser assumes that the
** VFS has handled the PRAGMA itself and the parser generates a no-op
** prepared statement if result string is NULL, or that returns a copy
** of the result string if the string is non-NULL.
** ^If the [SQLITE_FCNTL_PRAGMA] file control returns
** any result code other than [SQLITE_OK] or [SQLITE_NOTFOUND], that means
** that the VFS encountered an error while handling the [PRAGMA] and the
** compilation of the PRAGMA fails with an error.  ^The [SQLITE_FCNTL_PRAGMA]
** file control occurs at the beginning of pragma statement analysis and so
** it is able to override built-in [PRAGMA] statements.
**
** <li>[[SQLITE_FCNTL_BUSYHANDLER]]
** ^The [SQLITE_FCNTL_BUSYHANDLER]
** file-control may be invoked by SQLite on the database file handle
** shortly after it is opened in order to provide a custom VFS with access
** to the connections busy-handler callback. The argument is of type (void **)
** - an array of two (void *) values. The first (void *) actually points
** to a function of type (int (*)(void *)). In order to invoke the connections
** busy-handler, this function should be invoked with the second (void *) in
** the array as the only argument. If it returns non-zero, then the operation
** should be retried. If it returns zero, the custom VFS should abandon the
** current operation.
**
** <li>[[SQLITE_FCNTL_TEMPFILENAME]]
** ^Application can invoke the [SQLITE_FCNTL_TEMPFILENAME] file-control
** to have SQLite generate a
** temporary filename using the same algorithm that is followed to generate
** temporary filenames for TEMP tables and other internal uses.  The
** argument should be a char** which will be filled with the filename
** written into memory obtained from [sqlite3_malloc()].  The caller should
** invoke [sqlite3_free()] on the result to avoid a memory leak.
**
** <li>[[SQLITE_FCNTL_MMAP_SIZE]]
** The [SQLITE_FCNTL_MMAP_SIZE] file control is used to query or set the
** maximum number of bytes that will be used for memory-mapped I/O.
** The argument is a pointer to a value of type sqlite3_int64 that
** is an advisory maximum number of bytes in the file to memory map.  The
** pointer is overwritten with the old value.  The limit is not changed if
** the value originally pointed to is negative, and so the current limit 
** can be queried by passing in a pointer to a negative number.  This
** file-control is used internally to implement [PRAGMA mmap_size].
**
** <li>[[SQLITE_FCNTL_TRACE]]
** The [SQLITE_FCNTL_TRACE] file control provides advisory information
** to the VFS about what the higher layers of the SQLite stack are doing.
** This file control is used by some VFS activity tracing [shims].
** The argument is a zero-terminated string.  Higher layers in the
** SQLite stack may generate instances of this file control if
** the [SQLITE_USE_FCNTL_TRACE] compile-time option is enabled.
**
** <li>[[SQLITE_FCNTL_HAS_MOVED]]
** The [SQLITE_FCNTL_HAS_MOVED] file control interprets its argument as a
** pointer to an integer and it writes a boolean into that integer depending
** on whether or not the file has been renamed, moved, or deleted since it
** was first opened.
**
** <li>[[SQLITE_FCNTL_WIN32_SET_HANDLE]]
** The [SQLITE_FCNTL_WIN32_SET_HANDLE] opcode is used for debugging.  This
** opcode causes the xFileControl method to swap the file handle with the one
** pointed to by the pArg argument.  This capability is used during testing
** and only needs to be supported when SQLITE_TEST is defined.
**
** <li>[[SQLITE_FCNTL_WAL_BLOCK]]
** The [SQLITE_FCNTL_WAL_BLOCK] is a signal to the VFS layer that it might
** be advantageous to block on the next WAL lock if the lock is not immediately
** available.  The WAL subsystem issues this signal during rare
** circumstances in order to fix a problem with priority inversion.
** Applications should <em>not</em> use this file-control.
**
** <li>[[SQLITE_FCNTL_ZIPVFS]]
** The [SQLITE_FCNTL_ZIPVFS] opcode is implemented by zipvfs only. All other
** VFS should return SQLITE_NOTFOUND for this opcode.
**
** <li>[[SQLITE_FCNTL_RBU]]
** The [SQLITE_FCNTL_RBU] opcode is implemented by the special VFS used by
** the RBU extension only.  All other VFS should return SQLITE_NOTFOUND for
** this opcode.  
** </ul>
*/
#define SQLITE_FCNTL_LOCKSTATE               1
#define SQLITE_FCNTL_GET_LOCKPROXYFILE       2
#define SQLITE_FCNTL_SET_LOCKPROXYFILE       3
#define SQLITE_FCNTL_LAST_ERRNO              4
#define SQLITE_FCNTL_SIZE_HINT               5
#define SQLITE_FCNTL_CHUNK_SIZE              6
#define SQLITE_FCNTL_FILE_POINTER            7
#define SQLITE_FCNTL_SYNC_OMITTED            8
#define SQLITE_FCNTL_WIN32_AV_RETRY          9
#define SQLITE_FCNTL_PERSIST_WAL            10
#define SQLITE_FCNTL_OVERWRITE              11
#define SQLITE_FCNTL_VFSNAME                12
#define SQLITE_FCNTL_POWERSAFE_OVERWRITE    13
#define SQLITE_FCNTL_PRAGMA                 14
#define SQLITE_FCNTL_BUSYHANDLER            15
#define SQLITE_FCNTL_TEMPFILENAME           16
#define SQLITE_FCNTL_MMAP_SIZE              18
#define SQLITE_FCNTL_TRACE                  19
#define SQLITE_FCNTL_HAS_MOVED              20
#define SQLITE_FCNTL_SYNC                   21
#define SQLITE_FCNTL_COMMIT_PHASETWO        22
#define SQLITE_FCNTL_WIN32_SET_HANDLE       23
#define SQLITE_FCNTL_WAL_BLOCK              24
#define SQLITE_FCNTL_ZIPVFS                 25
#define SQLITE_FCNTL_RBU                    26
#define SQLITE_FCNTL_VFS_POINTER            27
#define SQLITE_FCNTL_JOURNAL_POINTER        28

/* deprecated names */
#define SQLITE_GET_LOCKPROXYFILE      SQLITE_FCNTL_GET_LOCKPROXYFILE
#define SQLITE_SET_LOCKPROXYFILE      SQLITE_FCNTL_SET_LOCKPROXYFILE
#define SQLITE_LAST_ERRNO             SQLITE_FCNTL_LAST_ERRNO


/*
** CAPI3REF: Mutex Handle
**
** The mutex module within SQLite defines [sqlite3_mutex] to be an
** abstract type for a mutex object.  The SQLite core never looks
** at the internal representation of an [sqlite3_mutex].  It only
** deals with pointers to the [sqlite3_mutex] object.
**
** Mutexes are created using [sqlite3_mutex_alloc()].
*/
typedef struct sqlite3_mutex sqlite3_mutex;

/*
** CAPI3REF: OS Interface Object
**
** An instance of the sqlite3_vfs object defines the interface between
** the SQLite core and the underlying operating system.  The "vfs"
** in the name of the object stands for "virtual file system".  See
** the [VFS | VFS documentation] for further information.
**
** The value of the iVersion field is initially 1 but may be larger in
** future versions of SQLite.  Additional fields may be appended to this
** object when the iVersion value is increased.  Note that the structure
** of the sqlite3_vfs object changes in the transaction between
** SQLite version 3.5.9 and 3.6.0 and yet the iVersion field was not
** modified.
**
** The szOsFile field is the size of the subclassed [sqlite3_file]
** structure used by this VFS.  mxPathname is the maximum length of
** a pathname in this VFS.
**
** Registered sqlite3_vfs objects are kept on a linked list formed by
** the pNext pointer.  The [sqlite3_vfs_register()]
** and [sqlite3_vfs_unregister()] interfaces manage this list
** in a thread-safe way.  The [sqlite3_vfs_find()] interface
** searches the list.  Neither the application code nor the VFS
** implementation should use the pNext pointer.
**
** The pNext field is the only field in the sqlite3_vfs
** structure that SQLite will ever modify.  SQLite will only access
** or modify this field while holding a particular static mutex.
** The application should never modify anything within the sqlite3_vfs
** object once the object has been registered.
**
** The zName field holds the name of the VFS module.  The name must
** be unique across all VFS modules.
**
** [[sqlite3_vfs.xOpen]]
** ^SQLite guarantees that the zFilename parameter to xOpen
** is either a NULL pointer or string obtained
** from xFullPathname() with an optional suffix added.
** ^If a suffix is added to the zFilename parameter, it will
** consist of a single "-" character followed by no more than
** 11 alphanumeric and/or "-" characters.
** ^SQLite further guarantees that
** the string will be valid and unchanged until xClose() is
** called. Because of the previous sentence,
** the [sqlite3_file] can safely store a pointer to the
** filename if it needs to remember the filename for some reason.
** If the zFilename parameter to xOpen is a NULL pointer then xOpen
** must invent its own temporary name for the file.  ^Whenever the 
** xFilename parameter is NULL it will also be the case that the
** flags parameter will include [SQLITE_OPEN_DELETEONCLOSE].
**
** The flags argument to xOpen() includes all bits set in
** the flags argument to [sqlite3_open_v2()].  Or if [sqlite3_open()]
** or [sqlite3_open16()] is used, then flags includes at least
** [SQLITE_OPEN_READWRITE] | [SQLITE_OPEN_CREATE]. 
** If xOpen() opens a file read-only then it sets *pOutFlags to
** include [SQLITE_OPEN_READONLY].  Other bits in *pOutFlags may be set.
**
** ^(SQLite will also add one of the following flags to the xOpen()
** call, depending on the object being opened:
**
** <ul>
** <li>  [SQLITE_OPEN_MAIN_DB]
** <li>  [SQLITE_OPEN_MAIN_JOURNAL]
** <li>  [SQLITE_OPEN_TEMP_DB]
** <li>  [SQLITE_OPEN_TEMP_JOURNAL]
** <li>  [SQLITE_OPEN_TRANSIENT_DB]
** <li>  [SQLITE_OPEN_SUBJOURNAL]
** <li>  [SQLITE_OPEN_MASTER_JOURNAL]
** <li>  [SQLITE_OPEN_WAL]
** </ul>)^
**
** The file I/O implementation can use the object type flags to
** change the way it deals with files.  For example, an application
** that does not care about crash recovery or rollback might make
** the open of a journal file a no-op.  Writes to this journal would
** also be no-ops, and any attempt to read the journal would return
** SQLITE_IOERR.  Or the implementation might recognize that a database
** file will be doing page-aligned sector reads and writes in a random
** order and set up its I/O subsystem accordingly.
**
** SQLite might also add one of the following flags to the xOpen method:
**
** <ul>
** <li> [SQLITE_OPEN_DELETEONCLOSE]
** <li> [SQLITE_OPEN_EXCLUSIVE]
** </ul>
**
** The [SQLITE_OPEN_DELETEONCLOSE] flag means the file should be
** deleted when it is closed.  ^The [SQLITE_OPEN_DELETEONCLOSE]
** will be set for TEMP databases and their journals, transient
** databases, and subjournals.
**
** ^The [SQLITE_OPEN_EXCLUSIVE] flag is always used in conjunction
** with the [SQLITE_OPEN_CREATE] flag, which are both directly
** analogous to the O_EXCL and O_CREAT flags of the POSIX open()
** API.  The SQLITE_OPEN_EXCLUSIVE flag, when paired with the 
** SQLITE_OPEN_CREATE, is used to indicate that file should always
** be created, and that it is an error if it already exists.
** It is <i>not</i> used to indicate the file should be opened 
** for exclusive access.
**
** ^At least szOsFile bytes of memory are allocated by SQLite
** to hold the  [sqlite3_file] structure passed as the third
** argument to xOpen.  The xOpen method does not have to
** allocate the structure; it should just fill it in.  Note that
** the xOpen method must set the sqlite3_file.pMethods to either
** a valid [sqlite3_io_methods] object or to NULL.  xOpen must do
** this even if the open fails.  SQLite expects that the sqlite3_file.pMethods
** element will be valid after xOpen returns regardless of the success
** or failure of the xOpen call.
**
** [[sqlite3_vfs.xAccess]]
** ^The flags argument to xAccess() may be [SQLITE_ACCESS_EXISTS]
** to test for the existence of a file, or [SQLITE_ACCESS_READWRITE] to
** test whether a file is readable and writable, or [SQLITE_ACCESS_READ]
** to test whether a file is at least readable.   The file can be a
** directory.
**
** ^SQLite will always allocate at least mxPathname+1 bytes for the
** output buffer xFullPathname.  The exact size of the output buffer
** is also passed as a parameter to both  methods. If the output buffer
** is not large enough, [SQLITE_CANTOPEN] should be returned. Since this is
** handled as a fatal error by SQLite, vfs implementations should endeavor
** to prevent this by setting mxPathname to a sufficiently large value.
**
** The xRandomness(), xSleep(), xCurrentTime(), and xCurrentTimeInt64()
** interfaces are not strictly a part of the filesystem, but they are
** included in the VFS structure for completeness.
** The xRandomness() function attempts to return nBytes bytes
** of good-quality randomness into zOut.  The return value is
** the actual number of bytes of randomness obtained.
** The xSleep() method causes the calling thread to sleep for at
** least the number of microseconds given.  ^The xCurrentTime()
** method returns a Julian Day Number for the current date and time as
** a floating point value.
** ^The xCurrentTimeInt64() method returns, as an integer, the Julian
** Day Number multiplied by 86400000 (the number of milliseconds in 
** a 24-hour day).  
** ^SQLite will use the xCurrentTimeInt64() method to get the current
** date and time if that method is available (if iVersion is 2 or 
** greater and the function pointer is not NULL) and will fall back
** to xCurrentTime() if xCurrentTimeInt64() is unavailable.
**
** ^The xSetSystemCall(), xGetSystemCall(), and xNestSystemCall() interfaces
** are not used by the SQLite core.  These optional interfaces are provided
** by some VFSes to facilitate testing of the VFS code. By overriding 
** system calls with functions under its control, a test program can
** simulate faults and error conditions that would otherwise be difficult
** or impossible to induce.  The set of system calls that can be overridden
** varies from one VFS to another, and from one version of the same VFS to the
** next.  Applications that use these interfaces must be prepared for any
** or all of these interfaces to be NULL or for their behavior to change
** from one release to the next.  Applications must not attempt to access
** any of these methods if the iVersion of the VFS is less than 3.
*/
typedef struct sqlite3_vfs sqlite3_vfs;
typedef void (*sqlite3_syscall_ptr)(void);
struct sqlite3_vfs {
  int iVersion;            /* Structure version number (currently 3) */
  int szOsFile;            /* Size of subclassed sqlite3_file */
  int mxPathname;          /* Maximum file pathname length */
  sqlite3_vfs *pNext;      /* Next registered VFS */
  const char *zName;       /* Name of this virtual file system */
  void *pAppData;          /* Pointer to application-specific data */
  int (*xOpen)(sqlite3_vfs*, const char *zName, sqlite3_file*,
               int flags, int *pOutFlags);
  int (*xDelete)(sqlite3_vfs*, const char *zName, int syncDir);
  int (*xAccess)(sqlite3_vfs*, const char *zName, int flags, int *pResOut);
  int (*xFullPathname)(sqlite3_vfs*, const char *zName, int nOut, char *zOut);
  void *(*xDlOpen)(sqlite3_vfs*, const char *zFilename);
  void (*xDlError)(sqlite3_vfs*, int nByte, char *zErrMsg);
  void (*(*xDlSym)(sqlite3_vfs*,void*, const char *zSymbol))(void);
  void (*xDlClose)(sqlite3_vfs*, void*);
  int (*xRandomness)(sqlite3_vfs*, int nByte, char *zOut);
  int (*xSleep)(sqlite3_vfs*, int microseconds);
  int (*xCurrentTime)(sqlite3_vfs*, double*);
  int (*xGetLastError)(sqlite3_vfs*, int, char *);
  /*
  ** The methods above are in version 1 of the sqlite_vfs object
  ** definition.  Those that follow are added in version 2 or later
  */
  int (*xCurrentTimeInt64)(sqlite3_vfs*, sqlite3_int64*);
  /*
  ** The methods above are in versions 1 and 2 of the sqlite_vfs object.
  ** Those below are for version 3 and greater.
  */
  int (*xSetSystemCall)(sqlite3_vfs*, const char *zName, sqlite3_syscall_ptr);
  sqlite3_syscall_ptr (*xGetSystemCall)(sqlite3_vfs*, const char *zName);
  const char *(*xNextSystemCall)(sqlite3_vfs*, const char *zName);
  /*
  ** The methods above are in versions 1 through 3 of the sqlite_vfs object.
  ** New fields may be appended in future versions.  The iVersion
  ** value will increment whenever this happens. 
  */
};

/*
** CAPI3REF: Flags for the xAccess VFS method
**
** These integer constants can be used as the third parameter to
** the xAccess method of an [sqlite3_vfs] object.  They determine
** what kind of permissions the xAccess method is looking for.
** With SQLITE_ACCESS_EXISTS, the xAccess method
** simply checks whether the file exists.
** With SQLITE_ACCESS_READWRITE, the xAccess method
** checks whether the named directory is both readable and writable
** (in other words, if files can be added, removed, and renamed within
** the directory).
** The SQLITE_ACCESS_READWRITE constant is currently used only by the
** [temp_store_directory pragma], though this could change in a future
** release of SQLite.
** With SQLITE_ACCESS_READ, the xAccess method
** checks whether the file is readable.  The SQLITE_ACCESS_READ constant is
** currently unused, though it might be used in a future release of
** SQLite.
*/
#define SQLITE_ACCESS_EXISTS    0
#define SQLITE_ACCESS_READWRITE 1   /* Used by PRAGMA temp_store_directory */
#define SQLITE_ACCESS_READ      2   /* Unused */

/*
** CAPI3REF: Flags for the xShmLock VFS method
**
** These integer constants define the various locking operations
** allowed by the xShmLock method of [sqlite3_io_methods].  The
** following are the only legal combinations of flags to the
** xShmLock method:
**
** <ul>
** <li>  SQLITE_SHM_LOCK | SQLITE_SHM_SHARED
** <li>  SQLITE_SHM_LOCK | SQLITE_SHM_EXCLUSIVE
** <li>  SQLITE_SHM_UNLOCK | SQLITE_SHM_SHARED
** <li>  SQLITE_SHM_UNLOCK | SQLITE_SHM_EXCLUSIVE
** </ul>
**
** When unlocking, the same SHARED or EXCLUSIVE flag must be supplied as
** was given on the corresponding lock.  
**
** The xShmLock method can transition between unlocked and SHARED or
** between unlocked and EXCLUSIVE.  It cannot transition between SHARED
** and EXCLUSIVE.
*/
#define SQLITE_SHM_UNLOCK       1
#define SQLITE_SHM_LOCK         2
#define SQLITE_SHM_SHARED       4
#define SQLITE_SHM_EXCLUSIVE    8

/*
** CAPI3REF: Maximum xShmLock index
**
** The xShmLock method on [sqlite3_io_methods] may use values
** between 0 and this upper bound as its "offset" argument.
** The SQLite core will never attempt to acquire or release a
** lock outside of this range
*/
#define SQLITE_SHM_NLOCK        8


/*
** CAPI3REF: Initialize The SQLite Library
**
** ^The sqlite3_initialize() routine initializes the
** SQLite library.  ^The sqlite3_shutdown() routine
** deallocates any resources that were allocated by sqlite3_initialize().
** These routines are designed to aid in process initialization and
** shutdown on embedded systems.  Workstation applications using
** SQLite normally do not need to invoke either of these routines.
**
** A call to sqlite3_initialize() is an "effective" call if it is
** the first time sqlite3_initialize() is invoked during the lifetime of
** the process, or if it is the first time sqlite3_initialize() is invoked
** following a call to sqlite3_shutdown().  ^(Only an effective call
** of sqlite3_initialize() does any initialization.  All other calls
** are harmless no-ops.)^
**
** A call to sqlite3_shutdown() is an "effective" call if it is the first
** call to sqlite3_shutdown() since the last sqlite3_initialize().  ^(Only
** an effective call to sqlite3_shutdown() does any deinitialization.
** All other valid calls to sqlite3_shutdown() are harmless no-ops.)^
**
** The sqlite3_initialize() interface is threadsafe, but sqlite3_shutdown()
** is not.  The sqlite3_shutdown() interface must only be called from a
** single thread.  All open [database connections] must be closed and all
** other SQLite resources must be deallocated prior to invoking
** sqlite3_shutdown().
**
** Among other things, ^sqlite3_initialize() will invoke
** sqlite3_os_init().  Similarly, ^sqlite3_shutdown()
** will invoke sqlite3_os_end().
**
** ^The sqlite3_initialize() routine returns [SQLITE_OK] on success.
** ^If for some reason, sqlite3_initialize() is unable to initialize
** the library (perhaps it is unable to allocate a needed resource such
** as a mutex) it returns an [error code] other than [SQLITE_OK].
**
** ^The sqlite3_initialize() routine is called internally by many other
** SQLite interfaces so that an application usually does not need to
** invoke sqlite3_initialize() directly.  For example, [sqlite3_open()]
** calls sqlite3_initialize() so the SQLite library will be automatically
** initialized when [sqlite3_open()] is called if it has not be initialized
** already.  ^However, if SQLite is compiled with the [SQLITE_OMIT_AUTOINIT]
** compile-time option, then the automatic calls to sqlite3_initialize()
** are omitted and the application must call sqlite3_initialize() directly
** prior to using any other SQLite interface.  For maximum portability,
** it is recommended that applications always invoke sqlite3_initialize()
** directly prior to using any other SQLite interface.  Future releases
** of SQLite may require this.  In other words, the behavior exhibited
** when SQLite is compiled with [SQLITE_OMIT_AUTOINIT] might become the
** default behavior in some future release of SQLite.
**
** The sqlite3_os_init() routine does operating-system specific
** initialization of the SQLite library.  The sqlite3_os_end()
** routine undoes the effect of sqlite3_os_init().  Typical tasks
** performed by these routines include allocation or deallocation
** of static resources, initialization of global variables,
** setting up a default [sqlite3_vfs] module, or setting up
** a default configuration using [sqlite3_config()].
**
** The application should never invoke either sqlite3_os_init()
** or sqlite3_os_end() directly.  The application should only invoke
** sqlite3_initialize() and sqlite3_shutdown().  The sqlite3_os_init()
** interface is called automatically by sqlite3_initialize() and
** sqlite3_os_end() is called by sqlite3_shutdown().  Appropriate
** implementations for sqlite3_os_init() and sqlite3_os_end()
** are built into SQLite when it is compiled for Unix, Windows, or OS/2.
** When [custom builds | built for other platforms]
** (using the [SQLITE_OS_OTHER=1] compile-time
** option) the application must supply a suitable implementation for
** sqlite3_os_init() and sqlite3_os_end().  An application-supplied
** implementation of sqlite3_os_init() or sqlite3_os_end()
** must return [SQLITE_OK] on success and some other [error code] upon
** failure.
*/
SQLITE_API int SQLITE_STDCALL sqlite3_initialize(void);
SQLITE_API int SQLITE_STDCALL sqlite3_shutdown(void);
SQLITE_API int SQLITE_STDCALL sqlite3_os_init(void);
SQLITE_API int SQLITE_STDCALL sqlite3_os_end(void);

/*
** CAPI3REF: Configuring The SQLite Library
**
** The sqlite3_config() interface is used to make global configuration
** changes to SQLite in order to tune SQLite to the specific needs of
** the application.  The default configuration is recommended for most
** applications and so this routine is usually not necessary.  It is
** provided to support rare applications with unusual needs.
**
** <b>The sqlite3_config() interface is not threadsafe. The application
** must ensure that no other SQLite interfaces are invoked by other
** threads while sqlite3_config() is running.</b>
**
** The sqlite3_config() interface
** may only be invoked prior to library initialization using
** [sqlite3_initialize()] or after shutdown by [sqlite3_shutdown()].
** ^If sqlite3_config() is called after [sqlite3_initialize()] and before
** [sqlite3_shutdown()] then it will return SQLITE_MISUSE.
** Note, however, that ^sqlite3_config() can be called as part of the
** implementation of an application-defined [sqlite3_os_init()].
**
** The first argument to sqlite3_config() is an integer
** [configuration option] that determines
** what property of SQLite is to be configured.  Subsequent arguments
** vary depending on the [configuration option]
** in the first argument.
**
** ^When a configuration option is set, sqlite3_config() returns [SQLITE_OK].
** ^If the option is unknown or SQLite is unable to set the option
** then this routine returns a non-zero [error code].
*/
SQLITE_API int SQLITE_CDECL sqlite3_config(int, ...);

/*
** CAPI3REF: Configure database connections
** METHOD: sqlite3
**
** The sqlite3_db_config() interface is used to make configuration
** changes to a [database connection].  The interface is similar to
** [sqlite3_config()] except that the changes apply to a single
** [database connection] (specified in the first argument).
**
** The second argument to sqlite3_db_config(D,V,...)  is the
** [SQLITE_DBCONFIG_LOOKASIDE | configuration verb] - an integer code 
** that indicates what aspect of the [database connection] is being configured.
** Subsequent arguments vary depending on the configuration verb.
**
** ^Calls to sqlite3_db_config() return SQLITE_OK if and only if
** the call is considered successful.
*/
SQLITE_API int SQLITE_CDECL sqlite3_db_config(sqlite3*, int op, ...);

/*
** CAPI3REF: Memory Allocation Routines
**
** An instance of this object defines the interface between SQLite
** and low-level memory allocation routines.
**
** This object is used in only one place in the SQLite interface.
** A pointer to an instance of this object is the argument to
** [sqlite3_config()] when the configuration option is
** [SQLITE_CONFIG_MALLOC] or [SQLITE_CONFIG_GETMALLOC].  
** By creating an instance of this object
** and passing it to [sqlite3_config]([SQLITE_CONFIG_MALLOC])
** during configuration, an application can specify an alternative
** memory allocation subsystem for SQLite to use for all of its
** dynamic memory needs.
**
** Note that SQLite comes with several [built-in memory allocators]
** that are perfectly adequate for the overwhelming majority of applications
** and that this object is only useful to a tiny minority of applications
** with specialized memory allocation requirements.  This object is
** also used during testing of SQLite in order to specify an alternative
** memory allocator that simulates memory out-of-memory conditions in
** order to verify that SQLite recovers gracefully from such
** conditions.
**
** The xMalloc, xRealloc, and xFree methods must work like the
** malloc(), realloc() and free() functions from the standard C library.
** ^SQLite guarantees that the second argument to
** xRealloc is always a value returned by a prior call to xRoundup.
**
** xSize should return the allocated size of a memory allocation
** previously obtained from xMalloc or xRealloc.  The allocated size
** is always at least as big as the requested size but may be larger.
**
** The xRoundup method returns what would be the allocated size of
** a memory allocation given a particular requested size.  Most memory
** allocators round up memory allocations at least to the next multiple
** of 8.  Some allocators round up to a larger multiple or to a power of 2.
** Every memory allocation request coming in through [sqlite3_malloc()]
** or [sqlite3_realloc()] first calls xRoundup.  If xRoundup returns 0, 
** that causes the corresponding memory allocation to fail.
**
** The xInit method initializes the memory allocator.  For example,
** it might allocate any require mutexes or initialize internal data
** structures.  The xShutdown method is invoked (indirectly) by
** [sqlite3_shutdown()] and should deallocate any resources acquired
** by xInit.  The pAppData pointer is used as the only parameter to
** xInit and xShutdown.
**
** SQLite holds the [SQLITE_MUTEX_STATIC_MASTER] mutex when it invokes
** the xInit method, so the xInit method need not be threadsafe.  The
** xShutdown method is only called from [sqlite3_shutdown()] so it does
** not need to be threadsafe either.  For all other methods, SQLite
** holds the [SQLITE_MUTEX_STATIC_MEM] mutex as long as the
** [SQLITE_CONFIG_MEMSTATUS] configuration option is turned on (which
** it is by default) and so the methods are automatically serialized.
** However, if [SQLITE_CONFIG_MEMSTATUS] is disabled, then the other
** methods must be threadsafe or else make their own arrangements for
** serialization.
**
** SQLite will never invoke xInit() more than once without an intervening
** call to xShutdown().
*/
typedef struct sqlite3_mem_methods sqlite3_mem_methods;
struct sqlite3_mem_methods {
  void *(*xMalloc)(int);         /* Memory allocation function */
  void (*xFree)(void*);          /* Free a prior allocation */
  void *(*xRealloc)(void*,int);  /* Resize an allocation */
  int (*xSize)(void*);           /* Return the size of an allocation */
  int (*xRoundup)(int);          /* Round up request size to allocation size */
  int (*xInit)(void*);           /* Initialize the memory allocator */
  void (*xShutdown)(void*);      /* Deinitialize the memory allocator */
  void *pAppData;                /* Argument to xInit() and xShutdown() */
};

/*
** CAPI3REF: Configuration Options
** KEYWORDS: {configuration option}
**
** These constants are the available integer configuration options that
** can be passed as the first argument to the [sqlite3_config()] interface.
**
** New configuration options may be added in future releases of SQLite.
** Existing configuration options might be discontinued.  Applications
** should check the return code from [sqlite3_config()] to make sure that
** the call worked.  The [sqlite3_config()] interface will return a
** non-zero [error code] if a discontinued or unsupported configuration option
** is invoked.
**
** <dl>
** [[SQLITE_CONFIG_SINGLETHREAD]] <dt>SQLITE_CONFIG_SINGLETHREAD</dt>
** <dd>There are no arguments to this option.  ^This option sets the
** [threading mode] to Single-thread.  In other words, it disables
** all mutexing and puts SQLite into a mode where it can only be used
** by a single thread.   ^If SQLite is compiled with
** the [SQLITE_THREADSAFE | SQLITE_THREADSAFE=0] compile-time option then
** it is not possible to change the [threading mode] from its default
** value of Single-thread and so [sqlite3_config()] will return 
** [SQLITE_ERROR] if called with the SQLITE_CONFIG_SINGLETHREAD
** configuration option.</dd>
**
** [[SQLITE_CONFIG_MULTITHREAD]] <dt>SQLITE_CONFIG_MULTITHREAD</dt>
** <dd>There are no arguments to this option.  ^This option sets the
** [threading mode] to Multi-thread.  In other words, it disables
** mutexing on [database connection] and [prepared statement] objects.
** The application is responsible for serializing access to
** [database connections] and [prepared statements].  But other mutexes
** are enabled so that SQLite will be safe to use in a multi-threaded
** environment as long as no two threads attempt to use the same
** [database connection] at the same time.  ^If SQLite is compiled with
** the [SQLITE_THREADSAFE | SQLITE_THREADSAFE=0] compile-time option then
** it is not possible to set the Multi-thread [threading mode] and
** [sqlite3_config()] will return [SQLITE_ERROR] if called with the
** SQLITE_CONFIG_MULTITHREAD configuration option.</dd>
**
** [[SQLITE_CONFIG_SERIALIZED]] <dt>SQLITE_CONFIG_SERIALIZED</dt>
** <dd>There are no arguments to this option.  ^This option sets the
** [threading mode] to Serialized. In other words, this option enables
** all mutexes including the recursive
** mutexes on [database connection] and [prepared statement] objects.
** In this mode (which is the default when SQLite is compiled with
** [SQLITE_THREADSAFE=1]) the SQLite library will itself serialize access
** to [database connections] and [prepared statements] so that the
** application is free to use the same [database connection] or the
** same [prepared statement] in different threads at the same time.
** ^If SQLite is compiled with
** the [SQLITE_THREADSAFE | SQLITE_THREADSAFE=0] compile-time option then
** it is not possible to set the Serialized [threading mode] and
** [sqlite3_config()] will return [SQLITE_ERROR] if called with the
** SQLITE_CONFIG_SERIALIZED configuration option.</dd>
**
** [[SQLITE_CONFIG_MALLOC]] <dt>SQLITE_CONFIG_MALLOC</dt>
** <dd> ^(The SQLITE_CONFIG_MALLOC option takes a single argument which is 
** a pointer to an instance of the [sqlite3_mem_methods] structure.
** The argument specifies
** alternative low-level memory allocation routines to be used in place of
** the memory allocation routines built into SQLite.)^ ^SQLite makes
** its own private copy of the content of the [sqlite3_mem_methods] structure
** before the [sqlite3_config()] call returns.</dd>
**
** [[SQLITE_CONFIG_GETMALLOC]] <dt>SQLITE_CONFIG_GETMALLOC</dt>
** <dd> ^(The SQLITE_CONFIG_GETMALLOC option takes a single argument which
** is a pointer to an instance of the [sqlite3_mem_methods] structure.
** The [sqlite3_mem_methods]
** structure is filled with the currently defined memory allocation routines.)^
** This option can be used to overload the default memory allocation
** routines with a wrapper that simulations memory allocation failure or
** tracks memory usage, for example. </dd>
**
** [[SQLITE_CONFIG_MEMSTATUS]] <dt>SQLITE_CONFIG_MEMSTATUS</dt>
** <dd> ^The SQLITE_CONFIG_MEMSTATUS option takes single argument of type int,
** interpreted as a boolean, which enables or disables the collection of
** memory allocation statistics. ^(When memory allocation statistics are
** disabled, the following SQLite interfaces become non-operational:
**   <ul>
**   <li> [sqlite3_memory_used()]
**   <li> [sqlite3_memory_highwater()]
**   <li> [sqlite3_soft_heap_limit64()]
**   <li> [sqlite3_status64()]
**   </ul>)^
** ^Memory allocation statistics are enabled by default unless SQLite is
** compiled with [SQLITE_DEFAULT_MEMSTATUS]=0 in which case memory
** allocation statistics are disabled by default.
** </dd>
**
** [[SQLITE_CONFIG_SCRATCH]] <dt>SQLITE_CONFIG_SCRATCH</dt>
** <dd> ^The SQLITE_CONFIG_SCRATCH option specifies a static memory buffer
** that SQLite can use for scratch memory.  ^(There are three arguments
** to SQLITE_CONFIG_SCRATCH:  A pointer an 8-byte
** aligned memory buffer from which the scratch allocations will be
** drawn, the size of each scratch allocation (sz),
** and the maximum number of scratch allocations (N).)^
** The first argument must be a pointer to an 8-byte aligned buffer
** of at least sz*N bytes of memory.
** ^SQLite will not use more than one scratch buffers per thread.
** ^SQLite will never request a scratch buffer that is more than 6
** times the database page size.
** ^If SQLite needs needs additional
** scratch memory beyond what is provided by this configuration option, then 
** [sqlite3_malloc()] will be used to obtain the memory needed.<p>
** ^When the application provides any amount of scratch memory using
** SQLITE_CONFIG_SCRATCH, SQLite avoids unnecessary large
** [sqlite3_malloc|heap allocations].
** This can help [Robson proof|prevent memory allocation failures] due to heap
** fragmentation in low-memory embedded systems.
** </dd>
**
** [[SQLITE_CONFIG_PAGECACHE]] <dt>SQLITE_CONFIG_PAGECACHE</dt>
** <dd> ^The SQLITE_CONFIG_PAGECACHE option specifies a memory pool
** that SQLite can use for the database page cache with the default page
** cache implementation.  
** This configuration option is a no-op if an application-define page
** cache implementation is loaded using the [SQLITE_CONFIG_PCACHE2].
** ^There are three arguments to SQLITE_CONFIG_PAGECACHE: A pointer to
** 8-byte aligned memory (pMem), the size of each page cache line (sz),
** and the number of cache lines (N).
** The sz argument should be the size of the largest database page
** (a power of two between 512 and 65536) plus some extra bytes for each
** page header.  ^The number of extra bytes needed by the page header
** can be determined using [SQLITE_CONFIG_PCACHE_HDRSZ].
** ^It is harmless, apart from the wasted memory,
** for the sz parameter to be larger than necessary.  The pMem
** argument must be either a NULL pointer or a pointer to an 8-byte
** aligned block of memory of at least sz*N bytes, otherwise
** subsequent behavior is undefined.
** ^When pMem is not NULL, SQLite will strive to use the memory provided
** to satisfy page cache needs, falling back to [sqlite3_malloc()] if
** a page cache line is larger than sz bytes or if all of the pMem buffer
** is exhausted.
** ^If pMem is NULL and N is non-zero, then each database connection
** does an initial bulk allocation for page cache memory
** from [sqlite3_malloc()] sufficient for N cache lines if N is positive or
** of -1024*N bytes if N is negative, . ^If additional
** page cache memory is needed beyond what is provided by the initial
** allocation, then SQLite goes to [sqlite3_malloc()] separately for each
** additional cache line. </dd>
**
** [[SQLITE_CONFIG_HEAP]] <dt>SQLITE_CONFIG_HEAP</dt>
** <dd> ^The SQLITE_CONFIG_HEAP option specifies a static memory buffer 
** that SQLite will use for all of its dynamic memory allocation needs
** beyond those provided for by [SQLITE_CONFIG_SCRATCH] and
** [SQLITE_CONFIG_PAGECACHE].
** ^The SQLITE_CONFIG_HEAP option is only available if SQLite is compiled
** with either [SQLITE_ENABLE_MEMSYS3] or [SQLITE_ENABLE_MEMSYS5] and returns
** [SQLITE_ERROR] if invoked otherwise.
** ^There are three arguments to SQLITE_CONFIG_HEAP:
** An 8-byte aligned pointer to the memory,
** the number of bytes in the memory buffer, and the minimum allocation size.
** ^If the first pointer (the memory pointer) is NULL, then SQLite reverts
** to using its default memory allocator (the system malloc() implementation),
** undoing any prior invocation of [SQLITE_CONFIG_MALLOC].  ^If the
** memory pointer is not NULL then the alternative memory
** allocator is engaged to handle all of SQLites memory allocation needs.
** The first pointer (the memory pointer) must be aligned to an 8-byte
** boundary or subsequent behavior of SQLite will be undefined.
** The minimum allocation size is capped at 2**12. Reasonable values
** for the minimum allocation size are 2**5 through 2**8.</dd>
**
** [[SQLITE_CONFIG_MUTEX]] <dt>SQLITE_CONFIG_MUTEX</dt>
** <dd> ^(The SQLITE_CONFIG_MUTEX option takes a single argument which is a
** pointer to an instance of the [sqlite3_mutex_methods] structure.
** The argument specifies alternative low-level mutex routines to be used
** in place the mutex routines built into SQLite.)^  ^SQLite makes a copy of
** the content of the [sqlite3_mutex_methods] structure before the call to
** [sqlite3_config()] returns. ^If SQLite is compiled with
** the [SQLITE_THREADSAFE | SQLITE_THREADSAFE=0] compile-time option then
** the entire mutexing subsystem is omitted from the build and hence calls to
** [sqlite3_config()] with the SQLITE_CONFIG_MUTEX configuration option will
** return [SQLITE_ERROR].</dd>
**
** [[SQLITE_CONFIG_GETMUTEX]] <dt>SQLITE_CONFIG_GETMUTEX</dt>
** <dd> ^(The SQLITE_CONFIG_GETMUTEX option takes a single argument which
** is a pointer to an instance of the [sqlite3_mutex_methods] structure.  The
** [sqlite3_mutex_methods]
** structure is filled with the currently defined mutex routines.)^
** This option can be used to overload the default mutex allocation
** routines with a wrapper used to track mutex usage for performance
** profiling or testing, for example.   ^If SQLite is compiled with
** the [SQLITE_THREADSAFE | SQLITE_THREADSAFE=0] compile-time option then
** the entire mutexing subsystem is omitted from the build and hence calls to
** [sqlite3_config()] with the SQLITE_CONFIG_GETMUTEX configuration option will
** return [SQLITE_ERROR].</dd>
**
** [[SQLITE_CONFIG_LOOKASIDE]] <dt>SQLITE_CONFIG_LOOKASIDE</dt>
** <dd> ^(The SQLITE_CONFIG_LOOKASIDE option takes two arguments that determine
** the default size of lookaside memory on each [database connection].
** The first argument is the
** size of each lookaside buffer slot and the second is the number of
** slots allocated to each database connection.)^  ^(SQLITE_CONFIG_LOOKASIDE
** sets the <i>default</i> lookaside size. The [SQLITE_DBCONFIG_LOOKASIDE]
** option to [sqlite3_db_config()] can be used to change the lookaside
** configuration on individual connections.)^ </dd>
**
** [[SQLITE_CONFIG_PCACHE2]] <dt>SQLITE_CONFIG_PCACHE2</dt>
** <dd> ^(The SQLITE_CONFIG_PCACHE2 option takes a single argument which is 
** a pointer to an [sqlite3_pcache_methods2] object.  This object specifies
** the interface to a custom page cache implementation.)^
** ^SQLite makes a copy of the [sqlite3_pcache_methods2] object.</dd>
**
** [[SQLITE_CONFIG_GETPCACHE2]] <dt>SQLITE_CONFIG_GETPCACHE2</dt>
** <dd> ^(The SQLITE_CONFIG_GETPCACHE2 option takes a single argument which
** is a pointer to an [sqlite3_pcache_methods2] object.  SQLite copies of
** the current page cache implementation into that object.)^ </dd>
**
** [[SQLITE_CONFIG_LOG]] <dt>SQLITE_CONFIG_LOG</dt>
** <dd> The SQLITE_CONFIG_LOG option is used to configure the SQLite
** global [error log].
** (^The SQLITE_CONFIG_LOG option takes two arguments: a pointer to a
** function with a call signature of void(*)(void*,int,const char*), 
** and a pointer to void. ^If the function pointer is not NULL, it is
** invoked by [sqlite3_log()] to process each logging event.  ^If the
** function pointer is NULL, the [sqlite3_log()] interface becomes a no-op.
** ^The void pointer that is the second argument to SQLITE_CONFIG_LOG is
** passed through as the first parameter to the application-defined logger
** function whenever that function is invoked.  ^The second parameter to
** the logger function is a copy of the first parameter to the corresponding
** [sqlite3_log()] call and is intended to be a [result code] or an
** [extended result code].  ^The third parameter passed to the logger is
** log message after formatting via [sqlite3_snprintf()].
** The SQLite logging interface is not reentrant; the logger function
** supplied by the application must not invoke any SQLite interface.
** In a multi-threaded application, the application-defined logger
** function must be threadsafe. </dd>
**
** [[SQLITE_CONFIG_URI]] <dt>SQLITE_CONFIG_URI
** <dd>^(The SQLITE_CONFIG_URI option takes a single argument of type int.
** If non-zero, then URI handling is globally enabled. If the parameter is zero,
** then URI handling is globally disabled.)^ ^If URI handling is globally
** enabled, all filenames passed to [sqlite3_open()], [sqlite3_open_v2()],
** [sqlite3_open16()] or
** specified as part of [ATTACH] commands are interpreted as URIs, regardless
** of whether or not the [SQLITE_OPEN_URI] flag is set when the database
** connection is opened. ^If it is globally disabled, filenames are
** only interpreted as URIs if the SQLITE_OPEN_URI flag is set when the
** database connection is opened. ^(By default, URI handling is globally
** disabled. The default value may be changed by compiling with the
** [SQLITE_USE_URI] symbol defined.)^
**
** [[SQLITE_CONFIG_COVERING_INDEX_SCAN]] <dt>SQLITE_CONFIG_COVERING_INDEX_SCAN
** <dd>^The SQLITE_CONFIG_COVERING_INDEX_SCAN option takes a single integer
** argument which is interpreted as a boolean in order to enable or disable
** the use of covering indices for full table scans in the query optimizer.
** ^The default setting is determined
** by the [SQLITE_ALLOW_COVERING_INDEX_SCAN] compile-time option, or is "on"
** if that compile-time option is omitted.
** The ability to disable the use of covering indices for full table scans
** is because some incorrectly coded legacy applications might malfunction
** when the optimization is enabled.  Providing the ability to
** disable the optimization allows the older, buggy application code to work
** without change even with newer versions of SQLite.
**
** [[SQLITE_CONFIG_PCACHE]] [[SQLITE_CONFIG_GETPCACHE]]
** <dt>SQLITE_CONFIG_PCACHE and SQLITE_CONFIG_GETPCACHE
** <dd> These options are obsolete and should not be used by new code.
** They are retained for backwards compatibility but are now no-ops.
** </dd>
**
** [[SQLITE_CONFIG_SQLLOG]]
** <dt>SQLITE_CONFIG_SQLLOG
** <dd>This option is only available if sqlite is compiled with the
** [SQLITE_ENABLE_SQLLOG] pre-processor macro defined. The first argument should
** be a pointer to a function of type void(*)(void*,sqlite3*,const char*, int).
** The second should be of type (void*). The callback is invoked by the library
** in three separate circumstances, identified by the value passed as the
** fourth parameter. If the fourth parameter is 0, then the database connection
** passed as the second argument has just been opened. The third argument
** points to a buffer containing the name of the main database file. If the
** fourth parameter is 1, then the SQL statement that the third parameter
** points to has just been executed. Or, if the fourth parameter is 2, then
** the connection being passed as the second parameter is being closed. The
** third parameter is passed NULL In this case.  An example of using this
** configuration option can be seen in the "test_sqllog.c" source file in
** the canonical SQLite source tree.</dd>
**
** [[SQLITE_CONFIG_MMAP_SIZE]]
** <dt>SQLITE_CONFIG_MMAP_SIZE
** <dd>^SQLITE_CONFIG_MMAP_SIZE takes two 64-bit integer (sqlite3_int64) values
** that are the default mmap size limit (the default setting for
** [PRAGMA mmap_size]) and the maximum allowed mmap size limit.
** ^The default setting can be overridden by each database connection using
** either the [PRAGMA mmap_size] command, or by using the
** [SQLITE_FCNTL_MMAP_SIZE] file control.  ^(The maximum allowed mmap size
** will be silently truncated if necessary so that it does not exceed the
** compile-time maximum mmap size set by the
** [SQLITE_MAX_MMAP_SIZE] compile-time option.)^
** ^If either argument to this option is negative, then that argument is
** changed to its compile-time default.
**
** [[SQLITE_CONFIG_WIN32_HEAPSIZE]]
** <dt>SQLITE_CONFIG_WIN32_HEAPSIZE
** <dd>^The SQLITE_CONFIG_WIN32_HEAPSIZE option is only available if SQLite is
** compiled for Windows with the [SQLITE_WIN32_MALLOC] pre-processor macro
** defined. ^SQLITE_CONFIG_WIN32_HEAPSIZE takes a 32-bit unsigned integer value
** that specifies the maximum size of the created heap.
**
** [[SQLITE_CONFIG_PCACHE_HDRSZ]]
** <dt>SQLITE_CONFIG_PCACHE_HDRSZ
** <dd>^The SQLITE_CONFIG_PCACHE_HDRSZ option takes a single parameter which
** is a pointer to an integer and writes into that integer the number of extra
** bytes per page required for each page in [SQLITE_CONFIG_PAGECACHE].
** The amount of extra space required can change depending on the compiler,
** target platform, and SQLite version.
**
** [[SQLITE_CONFIG_PMASZ]]
** <dt>SQLITE_CONFIG_PMASZ
** <dd>^The SQLITE_CONFIG_PMASZ option takes a single parameter which
** is an unsigned integer and sets the "Minimum PMA Size" for the multithreaded
** sorter to that integer.  The default minimum PMA Size is set by the
** [SQLITE_SORTER_PMASZ] compile-time option.  New threads are launched
** to help with sort operations when multithreaded sorting
** is enabled (using the [PRAGMA threads] command) and the amount of content
** to be sorted exceeds the page size times the minimum of the
** [PRAGMA cache_size] setting and this value.
**
** [[SQLITE_CONFIG_STMTJRNL_SPILL]]
** <dt>SQLITE_CONFIG_STMTJRNL_SPILL
** <dd>^The SQLITE_CONFIG_STMTJRNL_SPILL option takes a single parameter which
** becomes the [statement journal] spill-to-disk threshold.  
** [Statement journals] are held in memory until their size (in bytes)
** exceeds this threshold, at which point they are written to disk.
** Or if the threshold is -1, statement journals are always held
** exclusively in memory.
** Since many statement journals never become large, setting the spill
** threshold to a value such as 64KiB can greatly reduce the amount of
** I/O required to support statement rollback.
** The default value for this setting is controlled by the
** [SQLITE_STMTJRNL_SPILL] compile-time option.
** </dl>
*/
#define SQLITE_CONFIG_SINGLETHREAD  1  /* nil */
#define SQLITE_CONFIG_MULTITHREAD   2  /* nil */
#define SQLITE_CONFIG_SERIALIZED    3  /* nil */
#define SQLITE_CONFIG_MALLOC        4  /* sqlite3_mem_methods* */
#define SQLITE_CONFIG_GETMALLOC     5  /* sqlite3_mem_methods* */
#define SQLITE_CONFIG_SCRATCH       6  /* void*, int sz, int N */
#define SQLITE_CONFIG_PAGECACHE     7  /* void*, int sz, int N */
#define SQLITE_CONFIG_HEAP          8  /* void*, int nByte, int min */
#define SQLITE_CONFIG_MEMSTATUS     9  /* boolean */
#define SQLITE_CONFIG_MUTEX        10  /* sqlite3_mutex_methods* */
#define SQLITE_CONFIG_GETMUTEX     11  /* sqlite3_mutex_methods* */
/* previously SQLITE_CONFIG_CHUNKALLOC 12 which is now unused. */ 
#define SQLITE_CONFIG_LOOKASIDE    13  /* int int */
#define SQLITE_CONFIG_PCACHE       14  /* no-op */
#define SQLITE_CONFIG_GETPCACHE    15  /* no-op */
#define SQLITE_CONFIG_LOG          16  /* xFunc, void* */
#define SQLITE_CONFIG_URI          17  /* int */
#define SQLITE_CONFIG_PCACHE2      18  /* sqlite3_pcache_methods2* */
#define SQLITE_CONFIG_GETPCACHE2   19  /* sqlite3_pcache_methods2* */
#define SQLITE_CONFIG_COVERING_INDEX_SCAN 20  /* int */
#define SQLITE_CONFIG_SQLLOG       21  /* xSqllog, void* */
#define SQLITE_CONFIG_MMAP_SIZE    22  /* sqlite3_int64, sqlite3_int64 */
#define SQLITE_CONFIG_WIN32_HEAPSIZE      23  /* int nByte */
#define SQLITE_CONFIG_PCACHE_HDRSZ        24  /* int *psz */
#define SQLITE_CONFIG_PMASZ               25  /* unsigned int szPma */
#define SQLITE_CONFIG_STMTJRNL_SPILL      26  /* int nByte */

/*
** CAPI3REF: Database Connection Configuration Options
**
** These constants are the available integer configuration options that
** can be passed as the second argument to the [sqlite3_db_config()] interface.
**
** New configuration options may be added in future releases of SQLite.
** Existing configuration options might be discontinued.  Applications
** should check the return code from [sqlite3_db_config()] to make sure that
** the call worked.  ^The [sqlite3_db_config()] interface will return a
** non-zero [error code] if a discontinued or unsupported configuration option
** is invoked.
**
** <dl>
** <dt>SQLITE_DBCONFIG_LOOKASIDE</dt>
** <dd> ^This option takes three additional arguments that determine the 
** [lookaside memory allocator] configuration for the [database connection].
** ^The first argument (the third parameter to [sqlite3_db_config()] is a
** pointer to a memory buffer to use for lookaside memory.
** ^The first argument after the SQLITE_DBCONFIG_LOOKASIDE verb
** may be NULL in which case SQLite will allocate the
** lookaside buffer itself using [sqlite3_malloc()]. ^The second argument is the
** size of each lookaside buffer slot.  ^The third argument is the number of
** slots.  The size of the buffer in the first argument must be greater than
** or equal to the product of the second and third arguments.  The buffer
** must be aligned to an 8-byte boundary.  ^If the second argument to
** SQLITE_DBCONFIG_LOOKASIDE is not a multiple of 8, it is internally
** rounded down to the next smaller multiple of 8.  ^(The lookaside memory
** configuration for a database connection can only be changed when that
** connection is not currently using lookaside memory, or in other words
** when the "current value" returned by
** [sqlite3_db_status](D,[SQLITE_CONFIG_LOOKASIDE],...) is zero.
** Any attempt to change the lookaside memory configuration when lookaside
** memory is in use leaves the configuration unchanged and returns 
** [SQLITE_BUSY].)^</dd>
**
** <dt>SQLITE_DBCONFIG_ENABLE_FKEY</dt>
** <dd> ^This option is used to enable or disable the enforcement of
** [foreign key constraints].  There should be two additional arguments.
** The first argument is an integer which is 0 to disable FK enforcement,
** positive to enable FK enforcement or negative to leave FK enforcement
** unchanged.  The second parameter is a pointer to an integer into which
** is written 0 or 1 to indicate whether FK enforcement is off or on
** following this call.  The second parameter may be a NULL pointer, in
** which case the FK enforcement setting is not reported back. </dd>
**
** <dt>SQLITE_DBCONFIG_ENABLE_TRIGGER</dt>
** <dd> ^This option is used to enable or disable [CREATE TRIGGER | triggers].
** There should be two additional arguments.
** The first argument is an integer which is 0 to disable triggers,
** positive to enable triggers or negative to leave the setting unchanged.
** The second parameter is a pointer to an integer into which
** is written 0 or 1 to indicate whether triggers are disabled or enabled
** following this call.  The second parameter may be a NULL pointer, in
** which case the trigger setting is not reported back. </dd>
**
** <dt>SQLITE_DBCONFIG_ENABLE_FTS3_TOKENIZER</dt>
** <dd> ^This option is used to enable or disable the two-argument
** version of the [fts3_tokenizer()] function which is part of the
** [FTS3] full-text search engine extension.
** There should be two additional arguments.
** The first argument is an integer which is 0 to disable fts3_tokenizer() or
** positive to enable fts3_tokenizer() or negative to leave the setting
** unchanged.
** The second parameter is a pointer to an integer into which
** is written 0 or 1 to indicate whether fts3_tokenizer is disabled or enabled
** following this call.  The second parameter may be a NULL pointer, in
** which case the new setting is not reported back. </dd>
**
** <dt>SQLITE_DBCONFIG_ENABLE_LOAD_EXTENSION</dt>
** <dd> ^This option is used to enable or disable the [sqlite3_load_extension()]
** interface independently of the [load_extension()] SQL function.
** The [sqlite3_enable_load_extension()] API enables or disables both the
** C-API [sqlite3_load_extension()] and the SQL function [load_extension()].
** There should be two additional arguments.
** When the first argument to this interface is 1, then only the C-API is
** enabled and the SQL function remains disabled.  If the first argment to
** this interface is 0, then both the C-API and the SQL function are disabled.
** If the first argument is -1, then no changes are made to state of either the
** C-API or the SQL function.
** The second parameter is a pointer to an integer into which
** is written 0 or 1 to indicate whether [sqlite3_load_extension()] interface
** is disabled or enabled following this call.  The second parameter may
** be a NULL pointer, in which case the new setting is not reported back.
** </dd>
**
** </dl>
*/
#define SQLITE_DBCONFIG_LOOKASIDE             1001 /* void* int int */
#define SQLITE_DBCONFIG_ENABLE_FKEY           1002 /* int int* */
#define SQLITE_DBCONFIG_ENABLE_TRIGGER        1003 /* int int* */
#define SQLITE_DBCONFIG_ENABLE_FTS3_TOKENIZER 1004 /* int int* */
#define SQLITE_DBCONFIG_ENABLE_LOAD_EXTENSION 1005 /* int int* */


/*
** CAPI3REF: Enable Or Disable Extended Result Codes
** METHOD: sqlite3
**
** ^The sqlite3_extended_result_codes() routine enables or disables the
** [extended result codes] feature of SQLite. ^The extended result
** codes are disabled by default for historical compatibility.
*/
SQLITE_API int SQLITE_STDCALL sqlite3_extended_result_codes(sqlite3*, int onoff);

/*
** CAPI3REF: Last Insert Rowid
** METHOD: sqlite3
**
** ^Each entry in most SQLite tables (except for [WITHOUT ROWID] tables)
** has a unique 64-bit signed
** integer key called the [ROWID | "rowid"]. ^The rowid is always available
** as an undeclared column named ROWID, OID, or _ROWID_ as long as those
** names are not also used by explicitly declared columns. ^If
** the table has a column of type [INTEGER PRIMARY KEY] then that column
** is another alias for the rowid.
**
** ^The sqlite3_last_insert_rowid(D) interface returns the [rowid] of the 
** most recent successful [INSERT] into a rowid table or [virtual table]
** on database connection D.
** ^Inserts into [WITHOUT ROWID] tables are not recorded.
** ^If no successful [INSERT]s into rowid tables
** have ever occurred on the database connection D, 
** then sqlite3_last_insert_rowid(D) returns zero.
**
** ^(If an [INSERT] occurs within a trigger or within a [virtual table]
** method, then this routine will return the [rowid] of the inserted
** row as long as the trigger or virtual table method is running.
** But once the trigger or virtual table method ends, the value returned 
** by this routine reverts to what it was before the trigger or virtual
** table method began.)^
**
** ^An [INSERT] that fails due to a constraint violation is not a
** successful [INSERT] and does not change the value returned by this
** routine.  ^Thus INSERT OR FAIL, INSERT OR IGNORE, INSERT OR ROLLBACK,
** and INSERT OR ABORT make no changes to the return value of this
** routine when their insertion fails.  ^(When INSERT OR REPLACE
** encounters a constraint violation, it does not fail.  The
** INSERT continues to completion after deleting rows that caused
** the constraint problem so INSERT OR REPLACE will always change
** the return value of this interface.)^
**
** ^For the purposes of this routine, an [INSERT] is considered to
** be successful even if it is subsequently rolled back.
**
** This function is accessible to SQL statements via the
** [last_insert_rowid() SQL function].
**
** If a separate thread performs a new [INSERT] on the same
** database connection while the [sqlite3_last_insert_rowid()]
** function is running and thus changes the last insert [rowid],
** then the value returned by [sqlite3_last_insert_rowid()] is
** unpredictable and might not equal either the old or the new
** last insert [rowid].
*/
SQLITE_API sqlite3_int64 SQLITE_STDCALL sqlite3_last_insert_rowid(sqlite3*);

/*
** CAPI3REF: Count The Number Of Rows Modified
** METHOD: sqlite3
**
** ^This function returns the number of rows modified, inserted or
** deleted by the most recently completed INSERT, UPDATE or DELETE
** statement on the database connection specified by the only parameter.
** ^Executing any other type of SQL statement does not modify the value
** returned by this function.
**
** ^Only changes made directly by the INSERT, UPDATE or DELETE statement are
** considered - auxiliary changes caused by [CREATE TRIGGER | triggers], 
** [foreign key actions] or [REPLACE] constraint resolution are not counted.
** 
** Changes to a view that are intercepted by 
** [INSTEAD OF trigger | INSTEAD OF triggers] are not counted. ^The value 
** returned by sqlite3_changes() immediately after an INSERT, UPDATE or 
** DELETE statement run on a view is always zero. Only changes made to real 
** tables are counted.
**
** Things are more complicated if the sqlite3_changes() function is
** executed while a trigger program is running. This may happen if the
** program uses the [changes() SQL function], or if some other callback
** function invokes sqlite3_changes() directly. Essentially:
** 
** <ul>
**   <li> ^(Before entering a trigger program the value returned by
**        sqlite3_changes() function is saved. After the trigger program 
**        has finished, the original value is restored.)^
** 
**   <li> ^(Within a trigger program each INSERT, UPDATE and DELETE 
**        statement sets the value returned by sqlite3_changes() 
**        upon completion as normal. Of course, this value will not include 
**        any changes performed by sub-triggers, as the sqlite3_changes() 
**        value will be saved and restored after each sub-trigger has run.)^
** </ul>
** 
** ^This means that if the changes() SQL function (or similar) is used
** by the first INSERT, UPDATE or DELETE statement within a trigger, it 
** returns the value as set when the calling statement began executing.
** ^If it is used by the second or subsequent such statement within a trigger 
** program, the value returned reflects the number of rows modified by the 
** previous INSERT, UPDATE or DELETE statement within the same trigger.
**
** See also the [sqlite3_total_changes()] interface, the
** [count_changes pragma], and the [changes() SQL function].
**
** If a separate thread makes changes on the same database connection
** while [sqlite3_changes()] is running then the value returned
** is unpredictable and not meaningful.
*/
SQLITE_API int SQLITE_STDCALL sqlite3_changes(sqlite3*);

/*
** CAPI3REF: Total Number Of Rows Modified
** METHOD: sqlite3
**
** ^This function returns the total number of rows inserted, modified or
** deleted by all [INSERT], [UPDATE] or [DELETE] statements completed
** since the database connection was opened, including those executed as
** part of trigger programs. ^Executing any other type of SQL statement
** does not affect the value returned by sqlite3_total_changes().
** 
** ^Changes made as part of [foreign key actions] are included in the
** count, but those made as part of REPLACE constraint resolution are
** not. ^Changes to a view that are intercepted by INSTEAD OF triggers 
** are not counted.
** 
** See also the [sqlite3_changes()] interface, the
** [count_changes pragma], and the [total_changes() SQL function].
**
** If a separate thread makes changes on the same database connection
** while [sqlite3_total_changes()] is running then the value
** returned is unpredictable and not meaningful.
*/
SQLITE_API int SQLITE_STDCALL sqlite3_total_changes(sqlite3*);

/*
** CAPI3REF: Interrupt A Long-Running Query
** METHOD: sqlite3
**
** ^This function causes any pending database operation to abort and
** return at its earliest opportunity. This routine is typically
** called in response to a user action such as pressing "Cancel"
** or Ctrl-C where the user wants a long query operation to halt
** immediately.
**
** ^It is safe to call this routine from a thread different from the
** thread that is currently running the database operation.  But it
** is not safe to call this routine with a [database connection] that
** is closed or might close before sqlite3_interrupt() returns.
**
** ^If an SQL operation is very nearly finished at the time when
** sqlite3_interrupt() is called, then it might not have an opportunity
** to be interrupted and might continue to completion.
**
** ^An SQL operation that is interrupted will return [SQLITE_INTERRUPT].
** ^If the interrupted SQL operation is an INSERT, UPDATE, or DELETE
** that is inside an explicit transaction, then the entire transaction
** will be rolled back automatically.
**
** ^The sqlite3_interrupt(D) call is in effect until all currently running
** SQL statements on [database connection] D complete.  ^Any new SQL statements
** that are started after the sqlite3_interrupt() call and before the 
** running statements reaches zero are interrupted as if they had been
** running prior to the sqlite3_interrupt() call.  ^New SQL statements
** that are started after the running statement count reaches zero are
** not effected by the sqlite3_interrupt().
** ^A call to sqlite3_interrupt(D) that occurs when there are no running
** SQL statements is a no-op and has no effect on SQL statements
** that are started after the sqlite3_interrupt() call returns.
**
** If the database connection closes while [sqlite3_interrupt()]
** is running then bad things will likely happen.
*/
SQLITE_API void SQLITE_STDCALL sqlite3_interrupt(sqlite3*);

/*
** CAPI3REF: Determine If An SQL Statement Is Complete
**
** These routines are useful during command-line input to determine if the
** currently entered text seems to form a complete SQL statement or
** if additional input is needed before sending the text into
** SQLite for parsing.  ^These routines return 1 if the input string
** appears to be a complete SQL statement.  ^A statement is judged to be
** complete if it ends with a semicolon token and is not a prefix of a
** well-formed CREATE TRIGGER statement.  ^Semicolons that are embedded within
** string literals or quoted identifier names or comments are not
** independent tokens (they are part of the token in which they are
** embedded) and thus do not count as a statement terminator.  ^Whitespace
** and comments that follow the final semicolon are ignored.
**
** ^These routines return 0 if the statement is incomplete.  ^If a
** memory allocation fails, then SQLITE_NOMEM is returned.
**
** ^These routines do not parse the SQL statements thus
** will not detect syntactically incorrect SQL.
**
** ^(If SQLite has not been initialized using [sqlite3_initialize()] prior 
** to invoking sqlite3_complete16() then sqlite3_initialize() is invoked
** automatically by sqlite3_complete16().  If that initialization fails,
** then the return value from sqlite3_complete16() will be non-zero
** regardless of whether or not the input SQL is complete.)^
**
** The input to [sqlite3_complete()] must be a zero-terminated
** UTF-8 string.
**
** The input to [sqlite3_complete16()] must be a zero-terminated
** UTF-16 string in native byte order.
*/
SQLITE_API int SQLITE_STDCALL sqlite3_complete(const char *sql);
SQLITE_API int SQLITE_STDCALL sqlite3_complete16(const void *sql);

/*
** CAPI3REF: Register A Callback To Handle SQLITE_BUSY Errors
** KEYWORDS: {busy-handler callback} {busy handler}
** METHOD: sqlite3
**
** ^The sqlite3_busy_handler(D,X,P) routine sets a callback function X
** that might be invoked with argument P whenever
** an attempt is made to access a database table associated with
** [database connection] D when another thread
** or process has the table locked.
** The sqlite3_busy_handler() interface is used to implement
** [sqlite3_busy_timeout()] and [PRAGMA busy_timeout].
**
** ^If the busy callback is NULL, then [SQLITE_BUSY]
** is returned immediately upon encountering the lock.  ^If the busy callback
** is not NULL, then the callback might be invoked with two arguments.
**
** ^The first argument to the busy handler is a copy of the void* pointer which
** is the third argument to sqlite3_busy_handler().  ^The second argument to
** the busy handler callback is the number of times that the busy handler has
** been invoked previously for the same locking event.  ^If the
** busy callback returns 0, then no additional attempts are made to
** access the database and [SQLITE_BUSY] is returned
** to the application.
** ^If the callback returns non-zero, then another attempt
** is made to access the database and the cycle repeats.
**
** The presence of a busy handler does not guarantee that it will be invoked
** when there is lock contention. ^If SQLite determines that invoking the busy
** handler could result in a deadlock, it will go ahead and return [SQLITE_BUSY]
** to the application instead of invoking the 
** busy handler.
** Consider a scenario where one process is holding a read lock that
** it is trying to promote to a reserved lock and
** a second process is holding a reserved lock that it is trying
** to promote to an exclusive lock.  The first process cannot proceed
** because it is blocked by the second and the second process cannot
** proceed because it is blocked by the first.  If both processes
** invoke the busy handlers, neither will make any progress.  Therefore,
** SQLite returns [SQLITE_BUSY] for the first process, hoping that this
** will induce the first process to release its read lock and allow
** the second process to proceed.
**
** ^The default busy callback is NULL.
**
** ^(There can only be a single busy handler defined for each
** [database connection].  Setting a new busy handler clears any
** previously set handler.)^  ^Note that calling [sqlite3_busy_timeout()]
** or evaluating [PRAGMA busy_timeout=N] will change the
** busy handler and thus clear any previously set busy handler.
**
** The busy callback should not take any actions which modify the
** database connection that invoked the busy handler.  In other words,
** the busy handler is not reentrant.  Any such actions
** result in undefined behavior.
** 
** A busy handler must not close the database connection
** or [prepared statement] that invoked the busy handler.
*/
SQLITE_API int SQLITE_STDCALL sqlite3_busy_handler(sqlite3*, int(*)(void*,int), void*);

/*
** CAPI3REF: Set A Busy Timeout
** METHOD: sqlite3
**
** ^This routine sets a [sqlite3_busy_handler | busy handler] that sleeps
** for a specified amount of time when a table is locked.  ^The handler
** will sleep multiple times until at least "ms" milliseconds of sleeping
** have accumulated.  ^After at least "ms" milliseconds of sleeping,
** the handler returns 0 which causes [sqlite3_step()] to return
** [SQLITE_BUSY].
**
** ^Calling this routine with an argument less than or equal to zero
** turns off all busy handlers.
**
** ^(There can only be a single busy handler for a particular
** [database connection] at any given moment.  If another busy handler
** was defined  (using [sqlite3_busy_handler()]) prior to calling
** this routine, that other busy handler is cleared.)^
**
** See also:  [PRAGMA busy_timeout]
*/
SQLITE_API int SQLITE_STDCALL sqlite3_busy_timeout(sqlite3*, int ms);

/*
** CAPI3REF: Convenience Routines For Running Queries
** METHOD: sqlite3
**
** This is a legacy interface that is preserved for backwards compatibility.
** Use of this interface is not recommended.
**
** Definition: A <b>result table</b> is memory data structure created by the
** [sqlite3_get_table()] interface.  A result table records the
** complete query results from one or more queries.
**
** The table conceptually has a number of rows and columns.  But
** these numbers are not part of the result table itself.  These
** numbers are obtained separately.  Let N be the number of rows
** and M be the number of columns.
**
** A result table is an array of pointers to zero-terminated UTF-8 strings.
** There are (N+1)*M elements in the array.  The first M pointers point
** to zero-terminated strings that  contain the names of the columns.
** The remaining entries all point to query results.  NULL values result
** in NULL pointers.  All other values are in their UTF-8 zero-terminated
** string representation as returned by [sqlite3_column_text()].
**
** A result table might consist of one or more memory allocations.
** It is not safe to pass a result table directly to [sqlite3_free()].
** A result table should be deallocated using [sqlite3_free_table()].
**
** ^(As an example of the result table format, suppose a query result
** is as follows:
**
** <blockquote><pre>
**        Name        | Age
**        -----------------------
**        Alice       | 43
**        Bob         | 28
**        Cindy       | 21
** </pre></blockquote>
**
** There are two column (M==2) and three rows (N==3).  Thus the
** result table has 8 entries.  Suppose the result table is stored
** in an array names azResult.  Then azResult holds this content:
**
** <blockquote><pre>
**        azResult&#91;0] = "Name";
**        azResult&#91;1] = "Age";
**        azResult&#91;2] = "Alice";
**        azResult&#91;3] = "43";
**        azResult&#91;4] = "Bob";
**        azResult&#91;5] = "28";
**        azResult&#91;6] = "Cindy";
**        azResult&#91;7] = "21";
** </pre></blockquote>)^
**
** ^The sqlite3_get_table() function evaluates one or more
** semicolon-separated SQL statements in the zero-terminated UTF-8
** string of its 2nd parameter and returns a result table to the
** pointer given in its 3rd parameter.
**
** After the application has finished with the result from sqlite3_get_table(),
** it must pass the result table pointer to sqlite3_free_table() in order to
** release the memory that was malloced.  Because of the way the
** [sqlite3_malloc()] happens within sqlite3_get_table(), the calling
** function must not try to call [sqlite3_free()] directly.  Only
** [sqlite3_free_table()] is able to release the memory properly and safely.
**
** The sqlite3_get_table() interface is implemented as a wrapper around
** [sqlite3_exec()].  The sqlite3_get_table() routine does not have access
** to any internal data structures of SQLite.  It uses only the public
** interface defined here.  As a consequence, errors that occur in the
** wrapper layer outside of the internal [sqlite3_exec()] call are not
** reflected in subsequent calls to [sqlite3_errcode()] or
** [sqlite3_errmsg()].
*/
SQLITE_API int SQLITE_STDCALL sqlite3_get_table(
  sqlite3 *db,          /* An open database */
  const char *zSql,     /* SQL to be evaluated */
  char ***pazResult,    /* Results of the query */
  int *pnRow,           /* Number of result rows written here */
  int *pnColumn,        /* Number of result columns written here */
  char **pzErrmsg       /* Error msg written here */
);
SQLITE_API void SQLITE_STDCALL sqlite3_free_table(char **result);

/*
** CAPI3REF: Formatted String Printing Functions
**
** These routines are work-alikes of the "printf()" family of functions
** from the standard C library.
** These routines understand most of the common K&R formatting options,
** plus some additional non-standard formats, detailed below.
** Note that some of the more obscure formatting options from recent
** C-library standards are omitted from this implementation.
**
** ^The sqlite3_mprintf() and sqlite3_vmprintf() routines write their
** results into memory obtained from [sqlite3_malloc()].
** The strings returned by these two routines should be
** released by [sqlite3_free()].  ^Both routines return a
** NULL pointer if [sqlite3_malloc()] is unable to allocate enough
** memory to hold the resulting string.
**
** ^(The sqlite3_snprintf() routine is similar to "snprintf()" from
** the standard C library.  The result is written into the
** buffer supplied as the second parameter whose size is given by
** the first parameter. Note that the order of the
** first two parameters is reversed from snprintf().)^  This is an
** historical accident that cannot be fixed without breaking
** backwards compatibility.  ^(Note also that sqlite3_snprintf()
** returns a pointer to its buffer instead of the number of
** characters actually written into the buffer.)^  We admit that
** the number of characters written would be a more useful return
** value but we cannot change the implementation of sqlite3_snprintf()
** now without breaking compatibility.
**
** ^As long as the buffer size is greater than zero, sqlite3_snprintf()
** guarantees that the buffer is always zero-terminated.  ^The first
** parameter "n" is the total size of the buffer, including space for
** the zero terminator.  So the longest string that can be completely
** written will be n-1 characters.
**
** ^The sqlite3_vsnprintf() routine is a varargs version of sqlite3_snprintf().
**
** These routines all implement some additional formatting
** options that are useful for constructing SQL statements.
** All of the usual printf() formatting options apply.  In addition, there
** is are "%q", "%Q", "%w" and "%z" options.
**
** ^(The %q option works like %s in that it substitutes a nul-terminated
** string from the argument list.  But %q also doubles every '\'' character.
** %q is designed for use inside a string literal.)^  By doubling each '\''
** character it escapes that character and allows it to be inserted into
** the string.
**
** For example, assume the string variable zText contains text as follows:
**
** <blockquote><pre>
**  char *zText = "It's a happy day!";
** </pre></blockquote>
**
** One can use this text in an SQL statement as follows:
**
** <blockquote><pre>
**  char *zSQL = sqlite3_mprintf("INSERT INTO table VALUES('%q')", zText);
**  sqlite3_exec(db, zSQL, 0, 0, 0);
**  sqlite3_free(zSQL);
** </pre></blockquote>
**
** Because the %q format string is used, the '\'' character in zText
** is escaped and the SQL generated is as follows:
**
** <blockquote><pre>
**  INSERT INTO table1 VALUES('It''s a happy day!')
** </pre></blockquote>
**
** This is correct.  Had we used %s instead of %q, the generated SQL
** would have looked like this:
**
** <blockquote><pre>
**  INSERT INTO table1 VALUES('It's a happy day!');
** </pre></blockquote>
**
** This second example is an SQL syntax error.  As a general rule you should
** always use %q instead of %s when inserting text into a string literal.
**
** ^(The %Q option works like %q except it also adds single quotes around
** the outside of the total string.  Additionally, if the parameter in the
** argument list is a NULL pointer, %Q substitutes the text "NULL" (without
** single quotes).)^  So, for example, one could say:
**
** <blockquote><pre>
**  char *zSQL = sqlite3_mprintf("INSERT INTO table VALUES(%Q)", zText);
**  sqlite3_exec(db, zSQL, 0, 0, 0);
**  sqlite3_free(zSQL);
** </pre></blockquote>
**
** The code above will render a correct SQL statement in the zSQL
** variable even if the zText variable is a NULL pointer.
**
** ^(The "%w" formatting option is like "%q" except that it expects to
** be contained within double-quotes instead of single quotes, and it
** escapes the double-quote character instead of the single-quote
** character.)^  The "%w" formatting option is intended for safely inserting
** table and column names into a constructed SQL statement.
**
** ^(The "%z" formatting option works like "%s" but with the
** addition that after the string has been read and copied into
** the result, [sqlite3_free()] is called on the input string.)^
*/
SQLITE_API char *SQLITE_CDECL sqlite3_mprintf(const char*,...);
SQLITE_API char *SQLITE_STDCALL sqlite3_vmprintf(const char*, va_list);
SQLITE_API char *SQLITE_CDECL sqlite3_snprintf(int,char*,const char*, ...);
SQLITE_API char *SQLITE_STDCALL sqlite3_vsnprintf(int,char*,const char*, va_list);

/*
** CAPI3REF: Memory Allocation Subsystem
**
** The SQLite core uses these three routines for all of its own
** internal memory allocation needs. "Core" in the previous sentence
** does not include operating-system specific VFS implementation.  The
** Windows VFS uses native malloc() and free() for some operations.
**
** ^The sqlite3_malloc() routine returns a pointer to a block
** of memory at least N bytes in length, where N is the parameter.
** ^If sqlite3_malloc() is unable to obtain sufficient free
** memory, it returns a NULL pointer.  ^If the parameter N to
** sqlite3_malloc() is zero or negative then sqlite3_malloc() returns
** a NULL pointer.
**
** ^The sqlite3_malloc64(N) routine works just like
** sqlite3_malloc(N) except that N is an unsigned 64-bit integer instead
** of a signed 32-bit integer.
**
** ^Calling sqlite3_free() with a pointer previously returned
** by sqlite3_malloc() or sqlite3_realloc() releases that memory so
** that it might be reused.  ^The sqlite3_free() routine is
** a no-op if is called with a NULL pointer.  Passing a NULL pointer
** to sqlite3_free() is harmless.  After being freed, memory
** should neither be read nor written.  Even reading previously freed
** memory might result in a segmentation fault or other severe error.
** Memory corruption, a segmentation fault, or other severe error
** might result if sqlite3_free() is called with a non-NULL pointer that
** was not obtained from sqlite3_malloc() or sqlite3_realloc().
**
** ^The sqlite3_realloc(X,N) interface attempts to resize a
** prior memory allocation X to be at least N bytes.
** ^If the X parameter to sqlite3_realloc(X,N)
** is a NULL pointer then its behavior is identical to calling
** sqlite3_malloc(N).
** ^If the N parameter to sqlite3_realloc(X,N) is zero or
** negative then the behavior is exactly the same as calling
** sqlite3_free(X).
** ^sqlite3_realloc(X,N) returns a pointer to a memory allocation
** of at least N bytes in size or NULL if insufficient memory is available.
** ^If M is the size of the prior allocation, then min(N,M) bytes
** of the prior allocation are copied into the beginning of buffer returned
** by sqlite3_realloc(X,N) and the prior allocation is freed.
** ^If sqlite3_realloc(X,N) returns NULL and N is positive, then the
** prior allocation is not freed.
**
** ^The sqlite3_realloc64(X,N) interfaces works the same as
** sqlite3_realloc(X,N) except that N is a 64-bit unsigned integer instead
** of a 32-bit signed integer.
**
** ^If X is a memory allocation previously obtained from sqlite3_malloc(),
** sqlite3_malloc64(), sqlite3_realloc(), or sqlite3_realloc64(), then
** sqlite3_msize(X) returns the size of that memory allocation in bytes.
** ^The value returned by sqlite3_msize(X) might be larger than the number
** of bytes requested when X was allocated.  ^If X is a NULL pointer then
** sqlite3_msize(X) returns zero.  If X points to something that is not
** the beginning of memory allocation, or if it points to a formerly
** valid memory allocation that has now been freed, then the behavior
** of sqlite3_msize(X) is undefined and possibly harmful.
**
** ^The memory returned by sqlite3_malloc(), sqlite3_realloc(),
** sqlite3_malloc64(), and sqlite3_realloc64()
** is always aligned to at least an 8 byte boundary, or to a
** 4 byte boundary if the [SQLITE_4_BYTE_ALIGNED_MALLOC] compile-time
** option is used.
**
** In SQLite version 3.5.0 and 3.5.1, it was possible to define
** the SQLITE_OMIT_MEMORY_ALLOCATION which would cause the built-in
** implementation of these routines to be omitted.  That capability
** is no longer provided.  Only built-in memory allocators can be used.
**
** Prior to SQLite version 3.7.10, the Windows OS interface layer called
** the system malloc() and free() directly when converting
** filenames between the UTF-8 encoding used by SQLite
** and whatever filename encoding is used by the particular Windows
** installation.  Memory allocation errors were detected, but
** they were reported back as [SQLITE_CANTOPEN] or
** [SQLITE_IOERR] rather than [SQLITE_NOMEM].
**
** The pointer arguments to [sqlite3_free()] and [sqlite3_realloc()]
** must be either NULL or else pointers obtained from a prior
** invocation of [sqlite3_malloc()] or [sqlite3_realloc()] that have
** not yet been released.
**
** The application must not read or write any part of
** a block of memory after it has been released using
** [sqlite3_free()] or [sqlite3_realloc()].
*/
SQLITE_API void *SQLITE_STDCALL sqlite3_malloc(int);
SQLITE_API void *SQLITE_STDCALL sqlite3_malloc64(sqlite3_uint64);
SQLITE_API void *SQLITE_STDCALL sqlite3_realloc(void*, int);
SQLITE_API void *SQLITE_STDCALL sqlite3_realloc64(void*, sqlite3_uint64);
SQLITE_API void SQLITE_STDCALL sqlite3_free(void*);
SQLITE_API sqlite3_uint64 SQLITE_STDCALL sqlite3_msize(void*);

/*
** CAPI3REF: Memory Allocator Statistics
**
** SQLite provides these two interfaces for reporting on the status
** of the [sqlite3_malloc()], [sqlite3_free()], and [sqlite3_realloc()]
** routines, which form the built-in memory allocation subsystem.
**
** ^The [sqlite3_memory_used()] routine returns the number of bytes
** of memory currently outstanding (malloced but not freed).
** ^The [sqlite3_memory_highwater()] routine returns the maximum
** value of [sqlite3_memory_used()] since the high-water mark
** was last reset.  ^The values returned by [sqlite3_memory_used()] and
** [sqlite3_memory_highwater()] include any overhead
** added by SQLite in its implementation of [sqlite3_malloc()],
** but not overhead added by the any underlying system library
** routines that [sqlite3_malloc()] may call.
**
** ^The memory high-water mark is reset to the current value of
** [sqlite3_memory_used()] if and only if the parameter to
** [sqlite3_memory_highwater()] is true.  ^The value returned
** by [sqlite3_memory_highwater(1)] is the high-water mark
** prior to the reset.
*/
SQLITE_API sqlite3_int64 SQLITE_STDCALL sqlite3_memory_used(void);
SQLITE_API sqlite3_int64 SQLITE_STDCALL sqlite3_memory_highwater(int resetFlag);

/*
** CAPI3REF: Pseudo-Random Number Generator
**
** SQLite contains a high-quality pseudo-random number generator (PRNG) used to
** select random [ROWID | ROWIDs] when inserting new records into a table that
** already uses the largest possible [ROWID].  The PRNG is also used for
** the build-in random() and randomblob() SQL functions.  This interface allows
** applications to access the same PRNG for other purposes.
**
** ^A call to this routine stores N bytes of randomness into buffer P.
** ^The P parameter can be a NULL pointer.
**
** ^If this routine has not been previously called or if the previous
** call had N less than one or a NULL pointer for P, then the PRNG is
** seeded using randomness obtained from the xRandomness method of
** the default [sqlite3_vfs] object.
** ^If the previous call to this routine had an N of 1 or more and a
** non-NULL P then the pseudo-randomness is generated
** internally and without recourse to the [sqlite3_vfs] xRandomness
** method.
*/
SQLITE_API void SQLITE_STDCALL sqlite3_randomness(int N, void *P);

/*
** CAPI3REF: Compile-Time Authorization Callbacks
** METHOD: sqlite3
**
** ^This routine registers an authorizer callback with a particular
** [database connection], supplied in the first argument.
** ^The authorizer callback is invoked as SQL statements are being compiled
** by [sqlite3_prepare()] or its variants [sqlite3_prepare_v2()],
** [sqlite3_prepare16()] and [sqlite3_prepare16_v2()].  ^At various
** points during the compilation process, as logic is being created
** to perform various actions, the authorizer callback is invoked to
** see if those actions are allowed.  ^The authorizer callback should
** return [SQLITE_OK] to allow the action, [SQLITE_IGNORE] to disallow the
** specific action but allow the SQL statement to continue to be
** compiled, or [SQLITE_DENY] to cause the entire SQL statement to be
** rejected with an error.  ^If the authorizer callback returns
** any value other than [SQLITE_IGNORE], [SQLITE_OK], or [SQLITE_DENY]
** then the [sqlite3_prepare_v2()] or equivalent call that triggered
** the authorizer will fail with an error message.
**
** When the callback returns [SQLITE_OK], that means the operation
** requested is ok.  ^When the callback returns [SQLITE_DENY], the
** [sqlite3_prepare_v2()] or equivalent call that triggered the
** authorizer will fail with an error message explaining that
** access is denied. 
**
** ^The first parameter to the authorizer callback is a copy of the third
** parameter to the sqlite3_set_authorizer() interface. ^The second parameter
** to the callback is an integer [SQLITE_COPY | action code] that specifies
** the particular action to be authorized. ^The third through sixth parameters
** to the callback are zero-terminated strings that contain additional
** details about the action to be authorized.
**
** ^If the action code is [SQLITE_READ]
** and the callback returns [SQLITE_IGNORE] then the
** [prepared statement] statement is constructed to substitute
** a NULL value in place of the table column that would have
** been read if [SQLITE_OK] had been returned.  The [SQLITE_IGNORE]
** return can be used to deny an untrusted user access to individual
** columns of a table.
** ^If the action code is [SQLITE_DELETE] and the callback returns
** [SQLITE_IGNORE] then the [DELETE] operation proceeds but the
** [truncate optimization] is disabled and all rows are deleted individually.
**
** An authorizer is used when [sqlite3_prepare | preparing]
** SQL statements from an untrusted source, to ensure that the SQL statements
** do not try to access data they are not allowed to see, or that they do not
** try to execute malicious statements that damage the database.  For
** example, an application may allow a user to enter arbitrary
** SQL queries for evaluation by a database.  But the application does
** not want the user to be able to make arbitrary changes to the
** database.  An authorizer could then be put in place while the
** user-entered SQL is being [sqlite3_prepare | prepared] that
** disallows everything except [SELECT] statements.
**
** Applications that need to process SQL from untrusted sources
** might also consider lowering resource limits using [sqlite3_limit()]
** and limiting database size using the [max_page_count] [PRAGMA]
** in addition to using an authorizer.
**
** ^(Only a single authorizer can be in place on a database connection
** at a time.  Each call to sqlite3_set_authorizer overrides the
** previous call.)^  ^Disable the authorizer by installing a NULL callback.
** The authorizer is disabled by default.
**
** The authorizer callback must not do anything that will modify
** the database connection that invoked the authorizer callback.
** Note that [sqlite3_prepare_v2()] and [sqlite3_step()] both modify their
** database connections for the meaning of "modify" in this paragraph.
**
** ^When [sqlite3_prepare_v2()] is used to prepare a statement, the
** statement might be re-prepared during [sqlite3_step()] due to a 
** schema change.  Hence, the application should ensure that the
** correct authorizer callback remains in place during the [sqlite3_step()].
**
** ^Note that the authorizer callback is invoked only during
** [sqlite3_prepare()] or its variants.  Authorization is not
** performed during statement evaluation in [sqlite3_step()], unless
** as stated in the previous paragraph, sqlite3_step() invokes
** sqlite3_prepare_v2() to reprepare a statement after a schema change.
*/
SQLITE_API int SQLITE_STDCALL sqlite3_set_authorizer(
  sqlite3*,
  int (*xAuth)(void*,int,const char*,const char*,const char*,const char*),
  void *pUserData
);

/*
** CAPI3REF: Authorizer Return Codes
**
** The [sqlite3_set_authorizer | authorizer callback function] must
** return either [SQLITE_OK] or one of these two constants in order
** to signal SQLite whether or not the action is permitted.  See the
** [sqlite3_set_authorizer | authorizer documentation] for additional
** information.
**
** Note that SQLITE_IGNORE is also used as a [conflict resolution mode]
** returned from the [sqlite3_vtab_on_conflict()] interface.
*/
#define SQLITE_DENY   1   /* Abort the SQL statement with an error */
#define SQLITE_IGNORE 2   /* Don't allow access, but don't generate an error */

/*
** CAPI3REF: Authorizer Action Codes
**
** The [sqlite3_set_authorizer()] interface registers a callback function
** that is invoked to authorize certain SQL statement actions.  The
** second parameter to the callback is an integer code that specifies
** what action is being authorized.  These are the integer action codes that
** the authorizer callback may be passed.
**
** These action code values signify what kind of operation is to be
** authorized.  The 3rd and 4th parameters to the authorization
** callback function will be parameters or NULL depending on which of these
** codes is used as the second parameter.  ^(The 5th parameter to the
** authorizer callback is the name of the database ("main", "temp",
** etc.) if applicable.)^  ^The 6th parameter to the authorizer callback
** is the name of the inner-most trigger or view that is responsible for
** the access attempt or NULL if this access attempt is directly from
** top-level SQL code.
*/
/******************************************* 3rd ************ 4th ***********/
#define SQLITE_CREATE_INDEX          1   /* Index Name      Table Name      */
#define SQLITE_CREATE_TABLE          2   /* Table Name      NULL            */
#define SQLITE_CREATE_TEMP_INDEX     3   /* Index Name      Table Name      */
#define SQLITE_CREATE_TEMP_TABLE     4   /* Table Name      NULL            */
#define SQLITE_CREATE_TEMP_TRIGGER   5   /* Trigger Name    Table Name      */
#define SQLITE_CREATE_TEMP_VIEW      6   /* View Name       NULL            */
#define SQLITE_CREATE_TRIGGER        7   /* Trigger Name    Table Name      */
#define SQLITE_CREATE_VIEW           8   /* View Name       NULL            */
#define SQLITE_DELETE                9   /* Table Name      NULL            */
#define SQLITE_DROP_INDEX           10   /* Index Name      Table Name      */
#define SQLITE_DROP_TABLE           11   /* Table Name      NULL            */
#define SQLITE_DROP_TEMP_INDEX      12   /* Index Name      Table Name      */
#define SQLITE_DROP_TEMP_TABLE      13   /* Table Name      NULL            */
#define SQLITE_DROP_TEMP_TRIGGER    14   /* Trigger Name    Table Name      */
#define SQLITE_DROP_TEMP_VIEW       15   /* View Name       NULL            */
#define SQLITE_DROP_TRIGGER         16   /* Trigger Name    Table Name      */
#define SQLITE_DROP_VIEW            17   /* View Name       NULL            */
#define SQLITE_INSERT               18   /* Table Name      NULL            */
#define SQLITE_PRAGMA               19   /* Pragma Name     1st arg or NULL */
#define SQLITE_READ                 20   /* Table Name      Column Name     */
#define SQLITE_SELECT               21   /* NULL            NULL            */
#define SQLITE_TRANSACTION          22   /* Operation       NULL            */
#define SQLITE_UPDATE               23   /* Table Name      Column Name     */
#define SQLITE_ATTACH               24   /* Filename        NULL            */
#define SQLITE_DETACH               25   /* Database Name   NULL            */
#define SQLITE_ALTER_TABLE          26   /* Database Name   Table Name      */
#define SQLITE_REINDEX              27   /* Index Name      NULL            */
#define SQLITE_ANALYZE              28   /* Table Name      NULL            */
#define SQLITE_CREATE_VTABLE        29   /* Table Name      Module Name     */
#define SQLITE_DROP_VTABLE          30   /* Table Name      Module Name     */
#define SQLITE_FUNCTION             31   /* NULL            Function Name   */
#define SQLITE_SAVEPOINT            32   /* Operation       Savepoint Name  */
#define SQLITE_COPY                  0   /* No longer used */
#define SQLITE_RECURSIVE            33   /* NULL            NULL            */

/*
** CAPI3REF: Tracing And Profiling Functions
** METHOD: sqlite3
**
** These routines register callback functions that can be used for
** tracing and profiling the execution of SQL statements.
**
** ^The callback function registered by sqlite3_trace() is invoked at
** various times when an SQL statement is being run by [sqlite3_step()].
** ^The sqlite3_trace() callback is invoked with a UTF-8 rendering of the
** SQL statement text as the statement first begins executing.
** ^(Additional sqlite3_trace() callbacks might occur
** as each triggered subprogram is entered.  The callbacks for triggers
** contain a UTF-8 SQL comment that identifies the trigger.)^
**
** The [SQLITE_TRACE_SIZE_LIMIT] compile-time option can be used to limit
** the length of [bound parameter] expansion in the output of sqlite3_trace().
**
** ^The callback function registered by sqlite3_profile() is invoked
** as each SQL statement finishes.  ^The profile callback contains
** the original statement text and an estimate of wall-clock time
** of how long that statement took to run.  ^The profile callback
** time is in units of nanoseconds, however the current implementation
** is only capable of millisecond resolution so the six least significant
** digits in the time are meaningless.  Future versions of SQLite
** might provide greater resolution on the profiler callback.  The
** sqlite3_profile() function is considered experimental and is
** subject to change in future versions of SQLite.
*/
SQLITE_API void *SQLITE_STDCALL sqlite3_trace(sqlite3*, void(*xTrace)(void*,const char*), void*);
SQLITE_API SQLITE_EXPERIMENTAL void *SQLITE_STDCALL sqlite3_profile(sqlite3*,
   void(*xProfile)(void*,const char*,sqlite3_uint64), void*);

/*
** CAPI3REF: Query Progress Callbacks
** METHOD: sqlite3
**
** ^The sqlite3_progress_handler(D,N,X,P) interface causes the callback
** function X to be invoked periodically during long running calls to
** [sqlite3_exec()], [sqlite3_step()] and [sqlite3_get_table()] for
** database connection D.  An example use for this
** interface is to keep a GUI updated during a large query.
**
** ^The parameter P is passed through as the only parameter to the 
** callback function X.  ^The parameter N is the approximate number of 
** [virtual machine instructions] that are evaluated between successive
** invocations of the callback X.  ^If N is less than one then the progress
** handler is disabled.
**
** ^Only a single progress handler may be defined at one time per
** [database connection]; setting a new progress handler cancels the
** old one.  ^Setting parameter X to NULL disables the progress handler.
** ^The progress handler is also disabled by setting N to a value less
** than 1.
**
** ^If the progress callback returns non-zero, the operation is
** interrupted.  This feature can be used to implement a
** "Cancel" button on a GUI progress dialog box.
**
** The progress handler callback must not do anything that will modify
** the database connection that invoked the progress handler.
** Note that [sqlite3_prepare_v2()] and [sqlite3_step()] both modify their
** database connections for the meaning of "modify" in this paragraph.
**
*/
SQLITE_API void SQLITE_STDCALL sqlite3_progress_handler(sqlite3*, int, int(*)(void*), void*);

/*
** CAPI3REF: Opening A New Database Connection
** CONSTRUCTOR: sqlite3
**
** ^These routines open an SQLite database file as specified by the 
** filename argument. ^The filename argument is interpreted as UTF-8 for
** sqlite3_open() and sqlite3_open_v2() and as UTF-16 in the native byte
** order for sqlite3_open16(). ^(A [database connection] handle is usually
** returned in *ppDb, even if an error occurs.  The only exception is that
** if SQLite is unable to allocate memory to hold the [sqlite3] object,
** a NULL will be written into *ppDb instead of a pointer to the [sqlite3]
** object.)^ ^(If the database is opened (and/or created) successfully, then
** [SQLITE_OK] is returned.  Otherwise an [error code] is returned.)^ ^The
** [sqlite3_errmsg()] or [sqlite3_errmsg16()] routines can be used to obtain
** an English language description of the error following a failure of any
** of the sqlite3_open() routines.
**
** ^The default encoding will be UTF-8 for databases created using
** sqlite3_open() or sqlite3_open_v2().  ^The default encoding for databases
** created using sqlite3_open16() will be UTF-16 in the native byte order.
**
** Whether or not an error occurs when it is opened, resources
** associated with the [database connection] handle should be released by
** passing it to [sqlite3_close()] when it is no longer required.
**
** The sqlite3_open_v2() interface works like sqlite3_open()
** except that it accepts two additional parameters for additional control
** over the new database connection.  ^(The flags parameter to
** sqlite3_open_v2() can take one of
** the following three values, optionally combined with the 
** [SQLITE_OPEN_NOMUTEX], [SQLITE_OPEN_FULLMUTEX], [SQLITE_OPEN_SHAREDCACHE],
** [SQLITE_OPEN_PRIVATECACHE], and/or [SQLITE_OPEN_URI] flags:)^
**
** <dl>
** ^(<dt>[SQLITE_OPEN_READONLY]</dt>
** <dd>The database is opened in read-only mode.  If the database does not
** already exist, an error is returned.</dd>)^
**
** ^(<dt>[SQLITE_OPEN_READWRITE]</dt>
** <dd>The database is opened for reading and writing if possible, or reading
** only if the file is write protected by the operating system.  In either
** case the database must already exist, otherwise an error is returned.</dd>)^
**
** ^(<dt>[SQLITE_OPEN_READWRITE] | [SQLITE_OPEN_CREATE]</dt>
** <dd>The database is opened for reading and writing, and is created if
** it does not already exist. This is the behavior that is always used for
** sqlite3_open() and sqlite3_open16().</dd>)^
** </dl>
**
** If the 3rd parameter to sqlite3_open_v2() is not one of the
** combinations shown above optionally combined with other
** [SQLITE_OPEN_READONLY | SQLITE_OPEN_* bits]
** then the behavior is undefined.
**
** ^If the [SQLITE_OPEN_NOMUTEX] flag is set, then the database connection
** opens in the multi-thread [threading mode] as long as the single-thread
** mode has not been set at compile-time or start-time.  ^If the
** [SQLITE_OPEN_FULLMUTEX] flag is set then the database connection opens
** in the serialized [threading mode] unless single-thread was
** previously selected at compile-time or start-time.
** ^The [SQLITE_OPEN_SHAREDCACHE] flag causes the database connection to be
** eligible to use [shared cache mode], regardless of whether or not shared
** cache is enabled using [sqlite3_enable_shared_cache()].  ^The
** [SQLITE_OPEN_PRIVATECACHE] flag causes the database connection to not
** participate in [shared cache mode] even if it is enabled.
**
** ^The fourth parameter to sqlite3_open_v2() is the name of the
** [sqlite3_vfs] object that defines the operating system interface that
** the new database connection should use.  ^If the fourth parameter is
** a NULL pointer then the default [sqlite3_vfs] object is used.
**
** ^If the filename is ":memory:", then a private, temporary in-memory database
** is created for the connection.  ^This in-memory database will vanish when
** the database connection is closed.  Future versions of SQLite might
** make use of additional special filenames that begin with the ":" character.
** It is recommended that when a database filename actually does begin with
** a ":" character you should prefix the filename with a pathname such as
** "./" to avoid ambiguity.
**
** ^If the filename is an empty string, then a private, temporary
** on-disk database will be created.  ^This private database will be
** automatically deleted as soon as the database connection is closed.
**
** [[URI filenames in sqlite3_open()]] <h3>URI Filenames</h3>
**
** ^If [URI filename] interpretation is enabled, and the filename argument
** begins with "file:", then the filename is interpreted as a URI. ^URI
** filename interpretation is enabled if the [SQLITE_OPEN_URI] flag is
** set in the fourth argument to sqlite3_open_v2(), or if it has
** been enabled globally using the [SQLITE_CONFIG_URI] option with the
** [sqlite3_config()] method or by the [SQLITE_USE_URI] compile-time option.
** As of SQLite version 3.7.7, URI filename interpretation is turned off
** by default, but future releases of SQLite might enable URI filename
** interpretation by default.  See "[URI filenames]" for additional
** information.
**
** URI filenames are parsed according to RFC 3986. ^If the URI contains an
** authority, then it must be either an empty string or the string 
** "localhost". ^If the authority is not an empty string or "localhost", an 
** error is returned to the caller. ^The fragment component of a URI, if 
** present, is ignored.
**
** ^SQLite uses the path component of the URI as the name of the disk file
** which contains the database. ^If the path begins with a '/' character, 
** then it is interpreted as an absolute path. ^If the path does not begin 
** with a '/' (meaning that the authority section is omitted from the URI)
** then the path is interpreted as a relative path. 
** ^(On windows, the first component of an absolute path 
** is a drive specification (e.g. "C:").)^
**
** [[core URI query parameters]]
** The query component of a URI may contain parameters that are interpreted
** either by SQLite itself, or by a [VFS | custom VFS implementation].
** SQLite and its built-in [VFSes] interpret the
** following query parameters:
**
** <ul>
**   <li> <b>vfs</b>: ^The "vfs" parameter may be used to specify the name of
**     a VFS object that provides the operating system interface that should
**     be used to access the database file on disk. ^If this option is set to
**     an empty string the default VFS object is used. ^Specifying an unknown
**     VFS is an error. ^If sqlite3_open_v2() is used and the vfs option is
**     present, then the VFS specified by the option takes precedence over
**     the value passed as the fourth parameter to sqlite3_open_v2().
**
**   <li> <b>mode</b>: ^(The mode parameter may be set to either "ro", "rw",
**     "rwc", or "memory". Attempting to set it to any other value is
**     an error)^. 
**     ^If "ro" is specified, then the database is opened for read-only 
**     access, just as if the [SQLITE_OPEN_READONLY] flag had been set in the 
**     third argument to sqlite3_open_v2(). ^If the mode option is set to 
**     "rw", then the database is opened for read-write (but not create) 
**     access, as if SQLITE_OPEN_READWRITE (but not SQLITE_OPEN_CREATE) had 
**     been set. ^Value "rwc" is equivalent to setting both 
**     SQLITE_OPEN_READWRITE and SQLITE_OPEN_CREATE.  ^If the mode option is
**     set to "memory" then a pure [in-memory database] that never reads
**     or writes from disk is used. ^It is an error to specify a value for
**     the mode parameter that is less restrictive than that specified by
**     the flags passed in the third parameter to sqlite3_open_v2().
**
**   <li> <b>cache</b>: ^The cache parameter may be set to either "shared" or
**     "private". ^Setting it to "shared" is equivalent to setting the
**     SQLITE_OPEN_SHAREDCACHE bit in the flags argument passed to
**     sqlite3_open_v2(). ^Setting the cache parameter to "private" is 
**     equivalent to setting the SQLITE_OPEN_PRIVATECACHE bit.
**     ^If sqlite3_open_v2() is used and the "cache" parameter is present in
**     a URI filename, its value overrides any behavior requested by setting
**     SQLITE_OPEN_PRIVATECACHE or SQLITE_OPEN_SHAREDCACHE flag.
**
**  <li> <b>psow</b>: ^The psow parameter indicates whether or not the
**     [powersafe overwrite] property does or does not apply to the
**     storage media on which the database file resides.
**
**  <li> <b>nolock</b>: ^The nolock parameter is a boolean query parameter
**     which if set disables file locking in rollback journal modes.  This
**     is useful for accessing a database on a filesystem that does not
**     support locking.  Caution:  Database corruption might result if two
**     or more processes write to the same database and any one of those
**     processes uses nolock=1.
**
**  <li> <b>immutable</b>: ^The immutable parameter is a boolean query
**     parameter that indicates that the database file is stored on
**     read-only media.  ^When immutable is set, SQLite assumes that the
**     database file cannot be changed, even by a process with higher
**     privilege, and so the database is opened read-only and all locking
**     and change detection is disabled.  Caution: Setting the immutable
**     property on a database file that does in fact change can result
**     in incorrect query results and/or [SQLITE_CORRUPT] errors.
**     See also: [SQLITE_IOCAP_IMMUTABLE].
**       
** </ul>
**
** ^Specifying an unknown parameter in the query component of a URI is not an
** error.  Future versions of SQLite might understand additional query
** parameters.  See "[query parameters with special meaning to SQLite]" for
** additional information.
**
** [[URI filename examples]] <h3>URI filename examples</h3>
**
** <table border="1" align=center cellpadding=5>
** <tr><th> URI filenames <th> Results
** <tr><td> file:data.db <td> 
**          Open the file "data.db" in the current directory.
** <tr><td> file:/home/fred/data.db<br>
**          file:///home/fred/data.db <br> 
**          file://localhost/home/fred/data.db <br> <td> 
**          Open the database file "/home/fred/data.db".
** <tr><td> file://darkstar/home/fred/data.db <td> 
**          An error. "darkstar" is not a recognized authority.
** <tr><td style="white-space:nowrap"> 
**          file:///C:/Documents%20and%20Settings/fred/Desktop/data.db
**     <td> Windows only: Open the file "data.db" on fred's desktop on drive
**          C:. Note that the %20 escaping in this example is not strictly 
**          necessary - space characters can be used literally
**          in URI filenames.
** <tr><td> file:data.db?mode=ro&cache=private <td> 
**          Open file "data.db" in the current directory for read-only access.
**          Regardless of whether or not shared-cache mode is enabled by
**          default, use a private cache.
** <tr><td> file:/home/fred/data.db?vfs=unix-dotfile <td>
**          Open file "/home/fred/data.db". Use the special VFS "unix-dotfile"
**          that uses dot-files in place of posix advisory locking.
** <tr><td> file:data.db?mode=readonly <td> 
**          An error. "readonly" is not a valid option for the "mode" parameter.
** </table>
**
** ^URI hexadecimal escape sequences (%HH) are supported within the path and
** query components of a URI. A hexadecimal escape sequence consists of a
** percent sign - "%" - followed by exactly two hexadecimal digits 
** specifying an octet value. ^Before the path or query components of a
** URI filename are interpreted, they are encoded using UTF-8 and all 
** hexadecimal escape sequences replaced by a single byte containing the
** corresponding octet. If this process generates an invalid UTF-8 encoding,
** the results are undefined.
**
** <b>Note to Windows users:</b>  The encoding used for the filename argument
** of sqlite3_open() and sqlite3_open_v2() must be UTF-8, not whatever
** codepage is currently defined.  Filenames containing international
** characters must be converted to UTF-8 prior to passing them into
** sqlite3_open() or sqlite3_open_v2().
**
** <b>Note to Windows Runtime users:</b>  The temporary directory must be set
** prior to calling sqlite3_open() or sqlite3_open_v2().  Otherwise, various
** features that require the use of temporary files may fail.
**
** See also: [sqlite3_temp_directory]
*/
SQLITE_API int SQLITE_STDCALL sqlite3_open(
  const char *filename,   /* Database filename (UTF-8) */
  sqlite3 **ppDb          /* OUT: SQLite db handle */
);
SQLITE_API int SQLITE_STDCALL sqlite3_open16(
  const void *filename,   /* Database filename (UTF-16) */
  sqlite3 **ppDb          /* OUT: SQLite db handle */
);
SQLITE_API int SQLITE_STDCALL sqlite3_open_v2(
  const char *filename,   /* Database filename (UTF-8) */
  sqlite3 **ppDb,         /* OUT: SQLite db handle */
  int flags,              /* Flags */
  const char *zVfs        /* Name of VFS module to use */
);

/*
** CAPI3REF: Obtain Values For URI Parameters
**
** These are utility routines, useful to VFS implementations, that check
** to see if a database file was a URI that contained a specific query 
** parameter, and if so obtains the value of that query parameter.
**
** If F is the database filename pointer passed into the xOpen() method of 
** a VFS implementation when the flags parameter to xOpen() has one or 
** more of the [SQLITE_OPEN_URI] or [SQLITE_OPEN_MAIN_DB] bits set and
** P is the name of the query parameter, then
** sqlite3_uri_parameter(F,P) returns the value of the P
** parameter if it exists or a NULL pointer if P does not appear as a 
** query parameter on F.  If P is a query parameter of F
** has no explicit value, then sqlite3_uri_parameter(F,P) returns
** a pointer to an empty string.
**
** The sqlite3_uri_boolean(F,P,B) routine assumes that P is a boolean
** parameter and returns true (1) or false (0) according to the value
** of P.  The sqlite3_uri_boolean(F,P,B) routine returns true (1) if the
** value of query parameter P is one of "yes", "true", or "on" in any
** case or if the value begins with a non-zero number.  The 
** sqlite3_uri_boolean(F,P,B) routines returns false (0) if the value of
** query parameter P is one of "no", "false", or "off" in any case or
** if the value begins with a numeric zero.  If P is not a query
** parameter on F or if the value of P is does not match any of the
** above, then sqlite3_uri_boolean(F,P,B) returns (B!=0).
**
** The sqlite3_uri_int64(F,P,D) routine converts the value of P into a
** 64-bit signed integer and returns that integer, or D if P does not
** exist.  If the value of P is something other than an integer, then
** zero is returned.
** 
** If F is a NULL pointer, then sqlite3_uri_parameter(F,P) returns NULL and
** sqlite3_uri_boolean(F,P,B) returns B.  If F is not a NULL pointer and
** is not a database file pathname pointer that SQLite passed into the xOpen
** VFS method, then the behavior of this routine is undefined and probably
** undesirable.
*/
SQLITE_API const char *SQLITE_STDCALL sqlite3_uri_parameter(const char *zFilename, const char *zParam);
SQLITE_API int SQLITE_STDCALL sqlite3_uri_boolean(const char *zFile, const char *zParam, int bDefault);
SQLITE_API sqlite3_int64 SQLITE_STDCALL sqlite3_uri_int64(const char*, const char*, sqlite3_int64);


/*
** CAPI3REF: Error Codes And Messages
** METHOD: sqlite3
**
** ^If the most recent sqlite3_* API call associated with 
** [database connection] D failed, then the sqlite3_errcode(D) interface
** returns the numeric [result code] or [extended result code] for that
** API call.
** If the most recent API call was successful,
** then the return value from sqlite3_errcode() is undefined.
** ^The sqlite3_extended_errcode()
** interface is the same except that it always returns the 
** [extended result code] even when extended result codes are
** disabled.
**
** ^The sqlite3_errmsg() and sqlite3_errmsg16() return English-language
** text that describes the error, as either UTF-8 or UTF-16 respectively.
** ^(Memory to hold the error message string is managed internally.
** The application does not need to worry about freeing the result.
** However, the error string might be overwritten or deallocated by
** subsequent calls to other SQLite interface functions.)^
**
** ^The sqlite3_errstr() interface returns the English-language text
** that describes the [result code], as UTF-8.
** ^(Memory to hold the error message string is managed internally
** and must not be freed by the application)^.
**
** When the serialized [threading mode] is in use, it might be the
** case that a second error occurs on a separate thread in between
** the time of the first error and the call to these interfaces.
** When that happens, the second error will be reported since these
** interfaces always report the most recent result.  To avoid
** this, each thread can obtain exclusive use of the [database connection] D
** by invoking [sqlite3_mutex_enter]([sqlite3_db_mutex](D)) before beginning
** to use D and invoking [sqlite3_mutex_leave]([sqlite3_db_mutex](D)) after
** all calls to the interfaces listed here are completed.
**
** If an interface fails with SQLITE_MISUSE, that means the interface
** was invoked incorrectly by the application.  In that case, the
** error code and message may or may not be set.
*/
SQLITE_API int SQLITE_STDCALL sqlite3_errcode(sqlite3 *db);
SQLITE_API int SQLITE_STDCALL sqlite3_extended_errcode(sqlite3 *db);
SQLITE_API const char *SQLITE_STDCALL sqlite3_errmsg(sqlite3*);
SQLITE_API const void *SQLITE_STDCALL sqlite3_errmsg16(sqlite3*);
SQLITE_API const char *SQLITE_STDCALL sqlite3_errstr(int);

/*
** CAPI3REF: Prepared Statement Object
** KEYWORDS: {prepared statement} {prepared statements}
**
** An instance of this object represents a single SQL statement that
** has been compiled into binary form and is ready to be evaluated.
**
** Think of each SQL statement as a separate computer program.  The
** original SQL text is source code.  A prepared statement object 
** is the compiled object code.  All SQL must be converted into a
** prepared statement before it can be run.
**
** The life-cycle of a prepared statement object usually goes like this:
**
** <ol>
** <li> Create the prepared statement object using [sqlite3_prepare_v2()].
** <li> Bind values to [parameters] using the sqlite3_bind_*()
**      interfaces.
** <li> Run the SQL by calling [sqlite3_step()] one or more times.
** <li> Reset the prepared statement using [sqlite3_reset()] then go back
**      to step 2.  Do this zero or more times.
** <li> Destroy the object using [sqlite3_finalize()].
** </ol>
*/
typedef struct sqlite3_stmt sqlite3_stmt;

/*
** CAPI3REF: Run-time Limits
** METHOD: sqlite3
**
** ^(This interface allows the size of various constructs to be limited
** on a connection by connection basis.  The first parameter is the
** [database connection] whose limit is to be set or queried.  The
** second parameter is one of the [limit categories] that define a
** class of constructs to be size limited.  The third parameter is the
** new limit for that construct.)^
**
** ^If the new limit is a negative number, the limit is unchanged.
** ^(For each limit category SQLITE_LIMIT_<i>NAME</i> there is a 
** [limits | hard upper bound]
** set at compile-time by a C preprocessor macro called
** [limits | SQLITE_MAX_<i>NAME</i>].
** (The "_LIMIT_" in the name is changed to "_MAX_".))^
** ^Attempts to increase a limit above its hard upper bound are
** silently truncated to the hard upper bound.
**
** ^Regardless of whether or not the limit was changed, the 
** [sqlite3_limit()] interface returns the prior value of the limit.
** ^Hence, to find the current value of a limit without changing it,
** simply invoke this interface with the third parameter set to -1.
**
** Run-time limits are intended for use in applications that manage
** both their own internal database and also databases that are controlled
** by untrusted external sources.  An example application might be a
** web browser that has its own databases for storing history and
** separate databases controlled by JavaScript applications downloaded
** off the Internet.  The internal databases can be given the
** large, default limits.  Databases managed by external sources can
** be given much smaller limits designed to prevent a denial of service
** attack.  Developers might also want to use the [sqlite3_set_authorizer()]
** interface to further control untrusted SQL.  The size of the database
** created by an untrusted script can be contained using the
** [max_page_count] [PRAGMA].
**
** New run-time limit categories may be added in future releases.
*/
SQLITE_API int SQLITE_STDCALL sqlite3_limit(sqlite3*, int id, int newVal);

/*
** CAPI3REF: Run-Time Limit Categories
** KEYWORDS: {limit category} {*limit categories}
**
** These constants define various performance limits
** that can be lowered at run-time using [sqlite3_limit()].
** The synopsis of the meanings of the various limits is shown below.
** Additional information is available at [limits | Limits in SQLite].
**
** <dl>
** [[SQLITE_LIMIT_LENGTH]] ^(<dt>SQLITE_LIMIT_LENGTH</dt>
** <dd>The maximum size of any string or BLOB or table row, in bytes.<dd>)^
**
** [[SQLITE_LIMIT_SQL_LENGTH]] ^(<dt>SQLITE_LIMIT_SQL_LENGTH</dt>
** <dd>The maximum length of an SQL statement, in bytes.</dd>)^
**
** [[SQLITE_LIMIT_COLUMN]] ^(<dt>SQLITE_LIMIT_COLUMN</dt>
** <dd>The maximum number of columns in a table definition or in the
** result set of a [SELECT] or the maximum number of columns in an index
** or in an ORDER BY or GROUP BY clause.</dd>)^
**
** [[SQLITE_LIMIT_EXPR_DEPTH]] ^(<dt>SQLITE_LIMIT_EXPR_DEPTH</dt>
** <dd>The maximum depth of the parse tree on any expression.</dd>)^
**
** [[SQLITE_LIMIT_COMPOUND_SELECT]] ^(<dt>SQLITE_LIMIT_COMPOUND_SELECT</dt>
** <dd>The maximum number of terms in a compound SELECT statement.</dd>)^
**
** [[SQLITE_LIMIT_VDBE_OP]] ^(<dt>SQLITE_LIMIT_VDBE_OP</dt>
** <dd>The maximum number of instructions in a virtual machine program
** used to implement an SQL statement.  This limit is not currently
** enforced, though that might be added in some future release of
** SQLite.</dd>)^
**
** [[SQLITE_LIMIT_FUNCTION_ARG]] ^(<dt>SQLITE_LIMIT_FUNCTION_ARG</dt>
** <dd>The maximum number of arguments on a function.</dd>)^
**
** [[SQLITE_LIMIT_ATTACHED]] ^(<dt>SQLITE_LIMIT_ATTACHED</dt>
** <dd>The maximum number of [ATTACH | attached databases].)^</dd>
**
** [[SQLITE_LIMIT_LIKE_PATTERN_LENGTH]]
** ^(<dt>SQLITE_LIMIT_LIKE_PATTERN_LENGTH</dt>
** <dd>The maximum length of the pattern argument to the [LIKE] or
** [GLOB] operators.</dd>)^
**
** [[SQLITE_LIMIT_VARIABLE_NUMBER]]
** ^(<dt>SQLITE_LIMIT_VARIABLE_NUMBER</dt>
** <dd>The maximum index number of any [parameter] in an SQL statement.)^
**
** [[SQLITE_LIMIT_TRIGGER_DEPTH]] ^(<dt>SQLITE_LIMIT_TRIGGER_DEPTH</dt>
** <dd>The maximum depth of recursion for triggers.</dd>)^
**
** [[SQLITE_LIMIT_WORKER_THREADS]] ^(<dt>SQLITE_LIMIT_WORKER_THREADS</dt>
** <dd>The maximum number of auxiliary worker threads that a single
** [prepared statement] may start.</dd>)^
** </dl>
*/
#define SQLITE_LIMIT_LENGTH                    0
#define SQLITE_LIMIT_SQL_LENGTH                1
#define SQLITE_LIMIT_COLUMN                    2
#define SQLITE_LIMIT_EXPR_DEPTH                3
#define SQLITE_LIMIT_COMPOUND_SELECT           4
#define SQLITE_LIMIT_VDBE_OP                   5
#define SQLITE_LIMIT_FUNCTION_ARG              6
#define SQLITE_LIMIT_ATTACHED                  7
#define SQLITE_LIMIT_LIKE_PATTERN_LENGTH       8
#define SQLITE_LIMIT_VARIABLE_NUMBER           9
#define SQLITE_LIMIT_TRIGGER_DEPTH            10
#define SQLITE_LIMIT_WORKER_THREADS           11

/*
** CAPI3REF: Compiling An SQL Statement
** KEYWORDS: {SQL statement compiler}
** METHOD: sqlite3
** CONSTRUCTOR: sqlite3_stmt
**
** To execute an SQL query, it must first be compiled into a byte-code
** program using one of these routines.
**
** The first argument, "db", is a [database connection] obtained from a
** prior successful call to [sqlite3_open()], [sqlite3_open_v2()] or
** [sqlite3_open16()].  The database connection must not have been closed.
**
** The second argument, "zSql", is the statement to be compiled, encoded
** as either UTF-8 or UTF-16.  The sqlite3_prepare() and sqlite3_prepare_v2()
** interfaces use UTF-8, and sqlite3_prepare16() and sqlite3_prepare16_v2()
** use UTF-16.
**
** ^If the nByte argument is negative, then zSql is read up to the
** first zero terminator. ^If nByte is positive, then it is the
** number of bytes read from zSql.  ^If nByte is zero, then no prepared
** statement is generated.
** If the caller knows that the supplied string is nul-terminated, then
** there is a small performance advantage to passing an nByte parameter that
** is the number of bytes in the input string <i>including</i>
** the nul-terminator.
**
** ^If pzTail is not NULL then *pzTail is made to point to the first byte
** past the end of the first SQL statement in zSql.  These routines only
** compile the first statement in zSql, so *pzTail is left pointing to
** what remains uncompiled.
**
** ^*ppStmt is left pointing to a compiled [prepared statement] that can be
** executed using [sqlite3_step()].  ^If there is an error, *ppStmt is set
** to NULL.  ^If the input text contains no SQL (if the input is an empty
** string or a comment) then *ppStmt is set to NULL.
** The calling procedure is responsible for deleting the compiled
** SQL statement using [sqlite3_finalize()] after it has finished with it.
** ppStmt may not be NULL.
**
** ^On success, the sqlite3_prepare() family of routines return [SQLITE_OK];
** otherwise an [error code] is returned.
**
** The sqlite3_prepare_v2() and sqlite3_prepare16_v2() interfaces are
** recommended for all new programs. The two older interfaces are retained
** for backwards compatibility, but their use is discouraged.
** ^In the "v2" interfaces, the prepared statement
** that is returned (the [sqlite3_stmt] object) contains a copy of the
** original SQL text. This causes the [sqlite3_step()] interface to
** behave differently in three ways:
**
** <ol>
** <li>
** ^If the database schema changes, instead of returning [SQLITE_SCHEMA] as it
** always used to do, [sqlite3_step()] will automatically recompile the SQL
** statement and try to run it again. As many as [SQLITE_MAX_SCHEMA_RETRY]
** retries will occur before sqlite3_step() gives up and returns an error.
** </li>
**
** <li>
** ^When an error occurs, [sqlite3_step()] will return one of the detailed
** [error codes] or [extended error codes].  ^The legacy behavior was that
** [sqlite3_step()] would only return a generic [SQLITE_ERROR] result code
** and the application would have to make a second call to [sqlite3_reset()]
** in order to find the underlying cause of the problem. With the "v2" prepare
** interfaces, the underlying reason for the error is returned immediately.
** </li>
**
** <li>
** ^If the specific value bound to [parameter | host parameter] in the 
** WHERE clause might influence the choice of query plan for a statement,
** then the statement will be automatically recompiled, as if there had been 
** a schema change, on the first  [sqlite3_step()] call following any change
** to the [sqlite3_bind_text | bindings] of that [parameter]. 
** ^The specific value of WHERE-clause [parameter] might influence the 
** choice of query plan if the parameter is the left-hand side of a [LIKE]
** or [GLOB] operator or if the parameter is compared to an indexed column
** and the [SQLITE_ENABLE_STAT3] compile-time option is enabled.
** </li>
** </ol>
*/
SQLITE_API int SQLITE_STDCALL sqlite3_prepare(
  sqlite3 *db,            /* Database handle */
  const char *zSql,       /* SQL statement, UTF-8 encoded */
  int nByte,              /* Maximum length of zSql in bytes. */
  sqlite3_stmt **ppStmt,  /* OUT: Statement handle */
  const char **pzTail     /* OUT: Pointer to unused portion of zSql */
);
SQLITE_API int SQLITE_STDCALL sqlite3_prepare_v2(
  sqlite3 *db,            /* Database handle */
  const char *zSql,       /* SQL statement, UTF-8 encoded */
  int nByte,              /* Maximum length of zSql in bytes. */
  sqlite3_stmt **ppStmt,  /* OUT: Statement handle */
  const char **pzTail     /* OUT: Pointer to unused portion of zSql */
);
SQLITE_API int SQLITE_STDCALL sqlite3_prepare16(
  sqlite3 *db,            /* Database handle */
  const void *zSql,       /* SQL statement, UTF-16 encoded */
  int nByte,              /* Maximum length of zSql in bytes. */
  sqlite3_stmt **ppStmt,  /* OUT: Statement handle */
  const void **pzTail     /* OUT: Pointer to unused portion of zSql */
);
SQLITE_API int SQLITE_STDCALL sqlite3_prepare16_v2(
  sqlite3 *db,            /* Database handle */
  const void *zSql,       /* SQL statement, UTF-16 encoded */
  int nByte,              /* Maximum length of zSql in bytes. */
  sqlite3_stmt **ppStmt,  /* OUT: Statement handle */
  const void **pzTail     /* OUT: Pointer to unused portion of zSql */
);

/*
** CAPI3REF: Retrieving Statement SQL
** METHOD: sqlite3_stmt
**
** ^This interface can be used to retrieve a saved copy of the original
** SQL text used to create a [prepared statement] if that statement was
** compiled using either [sqlite3_prepare_v2()] or [sqlite3_prepare16_v2()].
*/
SQLITE_API const char *SQLITE_STDCALL sqlite3_sql(sqlite3_stmt *pStmt);

/*
** CAPI3REF: Determine If An SQL Statement Writes The Database
** METHOD: sqlite3_stmt
**
** ^The sqlite3_stmt_readonly(X) interface returns true (non-zero) if
** and only if the [prepared statement] X makes no direct changes to
** the content of the database file.
**
** Note that [application-defined SQL functions] or
** [virtual tables] might change the database indirectly as a side effect.  
** ^(For example, if an application defines a function "eval()" that 
** calls [sqlite3_exec()], then the following SQL statement would
** change the database file through side-effects:
**
** <blockquote><pre>
**    SELECT eval('DELETE FROM t1') FROM t2;
** </pre></blockquote>
**
** But because the [SELECT] statement does not change the database file
** directly, sqlite3_stmt_readonly() would still return true.)^
**
** ^Transaction control statements such as [BEGIN], [COMMIT], [ROLLBACK],
** [SAVEPOINT], and [RELEASE] cause sqlite3_stmt_readonly() to return true,
** since the statements themselves do not actually modify the database but
** rather they control the timing of when other statements modify the 
** database.  ^The [ATTACH] and [DETACH] statements also cause
** sqlite3_stmt_readonly() to return true since, while those statements
** change the configuration of a database connection, they do not make 
** changes to the content of the database files on disk.
*/
SQLITE_API int SQLITE_STDCALL sqlite3_stmt_readonly(sqlite3_stmt *pStmt);

/*
** CAPI3REF: Determine If A Prepared Statement Has Been Reset
** METHOD: sqlite3_stmt
**
** ^The sqlite3_stmt_busy(S) interface returns true (non-zero) if the
** [prepared statement] S has been stepped at least once using 
** [sqlite3_step(S)] but has neither run to completion (returned
** [SQLITE_DONE] from [sqlite3_step(S)]) nor
** been reset using [sqlite3_reset(S)].  ^The sqlite3_stmt_busy(S)
** interface returns false if S is a NULL pointer.  If S is not a 
** NULL pointer and is not a pointer to a valid [prepared statement]
** object, then the behavior is undefined and probably undesirable.
**
** This interface can be used in combination [sqlite3_next_stmt()]
** to locate all prepared statements associated with a database 
** connection that are in need of being reset.  This can be used,
** for example, in diagnostic routines to search for prepared 
** statements that are holding a transaction open.
*/
SQLITE_API int SQLITE_STDCALL sqlite3_stmt_busy(sqlite3_stmt*);

/*
** CAPI3REF: Dynamically Typed Value Object
** KEYWORDS: {protected sqlite3_value} {unprotected sqlite3_value}
**
** SQLite uses the sqlite3_value object to represent all values
** that can be stored in a database table. SQLite uses dynamic typing
** for the values it stores.  ^Values stored in sqlite3_value objects
** can be integers, floating point values, strings, BLOBs, or NULL.
**
** An sqlite3_value object may be either "protected" or "unprotected".
** Some interfaces require a protected sqlite3_value.  Other interfaces
** will accept either a protected or an unprotected sqlite3_value.
** Every interface that accepts sqlite3_value arguments specifies
** whether or not it requires a protected sqlite3_value.  The
** [sqlite3_value_dup()] interface can be used to construct a new 
** protected sqlite3_value from an unprotected sqlite3_value.
**
** The terms "protected" and "unprotected" refer to whether or not
** a mutex is held.  An internal mutex is held for a protected
** sqlite3_value object but no mutex is held for an unprotected
** sqlite3_value object.  If SQLite is compiled to be single-threaded
** (with [SQLITE_THREADSAFE=0] and with [sqlite3_threadsafe()] returning 0)
** or if SQLite is run in one of reduced mutex modes 
** [SQLITE_CONFIG_SINGLETHREAD] or [SQLITE_CONFIG_MULTITHREAD]
** then there is no distinction between protected and unprotected
** sqlite3_value objects and they can be used interchangeably.  However,
** for maximum code portability it is recommended that applications
** still make the distinction between protected and unprotected
** sqlite3_value objects even when not strictly required.
**
** ^The sqlite3_value objects that are passed as parameters into the
** implementation of [application-defined SQL functions] are protected.
** ^The sqlite3_value object returned by
** [sqlite3_column_value()] is unprotected.
** Unprotected sqlite3_value objects may only be used with
** [sqlite3_result_value()] and [sqlite3_bind_value()].
** The [sqlite3_value_blob | sqlite3_value_type()] family of
** interfaces require protected sqlite3_value objects.
*/
typedef struct Mem sqlite3_value;

/*
** CAPI3REF: SQL Function Context Object
**
** The context in which an SQL function executes is stored in an
** sqlite3_context object.  ^A pointer to an sqlite3_context object
** is always first parameter to [application-defined SQL functions].
** The application-defined SQL function implementation will pass this
** pointer through into calls to [sqlite3_result_int | sqlite3_result()],
** [sqlite3_aggregate_context()], [sqlite3_user_data()],
** [sqlite3_context_db_handle()], [sqlite3_get_auxdata()],
** and/or [sqlite3_set_auxdata()].
*/
typedef struct sqlite3_context sqlite3_context;

/*
** CAPI3REF: Binding Values To Prepared Statements
** KEYWORDS: {host parameter} {host parameters} {host parameter name}
** KEYWORDS: {SQL parameter} {SQL parameters} {parameter binding}
** METHOD: sqlite3_stmt
**
** ^(In the SQL statement text input to [sqlite3_prepare_v2()] and its variants,
** literals may be replaced by a [parameter] that matches one of following
** templates:
**
** <ul>
** <li>  ?
** <li>  ?NNN
** <li>  :VVV
** <li>  @VVV
** <li>  $VVV
** </ul>
**
** In the templates above, NNN represents an integer literal,
** and VVV represents an alphanumeric identifier.)^  ^The values of these
** parameters (also called "host parameter names" or "SQL parameters")
** can be set using the sqlite3_bind_*() routines defined here.
**
** ^The first argument to the sqlite3_bind_*() routines is always
** a pointer to the [sqlite3_stmt] object returned from
** [sqlite3_prepare_v2()] or its variants.
**
** ^The second argument is the index of the SQL parameter to be set.
** ^The leftmost SQL parameter has an index of 1.  ^When the same named
** SQL parameter is used more than once, second and subsequent
** occurrences have the same index as the first occurrence.
** ^The index for named parameters can be looked up using the
** [sqlite3_bind_parameter_index()] API if desired.  ^The index
** for "?NNN" parameters is the value of NNN.
** ^The NNN value must be between 1 and the [sqlite3_limit()]
** parameter [SQLITE_LIMIT_VARIABLE_NUMBER] (default value: 999).
**
** ^The third argument is the value to bind to the parameter.
** ^If the third parameter to sqlite3_bind_text() or sqlite3_bind_text16()
** or sqlite3_bind_blob() is a NULL pointer then the fourth parameter
** is ignored and the end result is the same as sqlite3_bind_null().
**
** ^(In those routines that have a fourth argument, its value is the
** number of bytes in the parameter.  To be clear: the value is the
** number of <u>bytes</u> in the value, not the number of characters.)^
** ^If the fourth parameter to sqlite3_bind_text() or sqlite3_bind_text16()
** is negative, then the length of the string is
** the number of bytes up to the first zero terminator.
** If the fourth parameter to sqlite3_bind_blob() is negative, then
** the behavior is undefined.
** If a non-negative fourth parameter is provided to sqlite3_bind_text()
** or sqlite3_bind_text16() or sqlite3_bind_text64() then
** that parameter must be the byte offset
** where the NUL terminator would occur assuming the string were NUL
** terminated.  If any NUL characters occur at byte offsets less than 
** the value of the fourth parameter then the resulting string value will
** contain embedded NULs.  The result of expressions involving strings
** with embedded NULs is undefined.
**
** ^The fifth argument to the BLOB and string binding interfaces
** is a destructor used to dispose of the BLOB or
** string after SQLite has finished with it.  ^The destructor is called
** to dispose of the BLOB or string even if the call to bind API fails.
** ^If the fifth argument is
** the special value [SQLITE_STATIC], then SQLite assumes that the
** information is in static, unmanaged space and does not need to be freed.
** ^If the fifth argument has the value [SQLITE_TRANSIENT], then
** SQLite makes its own private copy of the data immediately, before
** the sqlite3_bind_*() routine returns.
**
** ^The sixth argument to sqlite3_bind_text64() must be one of
** [SQLITE_UTF8], [SQLITE_UTF16], [SQLITE_UTF16BE], or [SQLITE_UTF16LE]
** to specify the encoding of the text in the third parameter.  If
** the sixth argument to sqlite3_bind_text64() is not one of the
** allowed values shown above, or if the text encoding is different
** from the encoding specified by the sixth parameter, then the behavior
** is undefined.
**
** ^The sqlite3_bind_zeroblob() routine binds a BLOB of length N that
** is filled with zeroes.  ^A zeroblob uses a fixed amount of memory
** (just an integer to hold its size) while it is being processed.
** Zeroblobs are intended to serve as placeholders for BLOBs whose
** content is later written using
** [sqlite3_blob_open | incremental BLOB I/O] routines.
** ^A negative value for the zeroblob results in a zero-length BLOB.
**
** ^If any of the sqlite3_bind_*() routines are called with a NULL pointer
** for the [prepared statement] or with a prepared statement for which
** [sqlite3_step()] has been called more recently than [sqlite3_reset()],
** then the call will return [SQLITE_MISUSE].  If any sqlite3_bind_()
** routine is passed a [prepared statement] that has been finalized, the
** result is undefined and probably harmful.
**
** ^Bindings are not cleared by the [sqlite3_reset()] routine.
** ^Unbound parameters are interpreted as NULL.
**
** ^The sqlite3_bind_* routines return [SQLITE_OK] on success or an
** [error code] if anything goes wrong.
** ^[SQLITE_TOOBIG] might be returned if the size of a string or BLOB
** exceeds limits imposed by [sqlite3_limit]([SQLITE_LIMIT_LENGTH]) or
** [SQLITE_MAX_LENGTH].
** ^[SQLITE_RANGE] is returned if the parameter
** index is out of range.  ^[SQLITE_NOMEM] is returned if malloc() fails.
**
** See also: [sqlite3_bind_parameter_count()],
** [sqlite3_bind_parameter_name()], and [sqlite3_bind_parameter_index()].
*/
SQLITE_API int SQLITE_STDCALL sqlite3_bind_blob(sqlite3_stmt*, int, const void*, int n, void(*)(void*));
SQLITE_API int SQLITE_STDCALL sqlite3_bind_blob64(sqlite3_stmt*, int, const void*, sqlite3_uint64,
                        void(*)(void*));
SQLITE_API int SQLITE_STDCALL sqlite3_bind_double(sqlite3_stmt*, int, double);
SQLITE_API int SQLITE_STDCALL sqlite3_bind_int(sqlite3_stmt*, int, int);
SQLITE_API int SQLITE_STDCALL sqlite3_bind_int64(sqlite3_stmt*, int, sqlite3_int64);
SQLITE_API int SQLITE_STDCALL sqlite3_bind_null(sqlite3_stmt*, int);
SQLITE_API int SQLITE_STDCALL sqlite3_bind_text(sqlite3_stmt*,int,const char*,int,void(*)(void*));
SQLITE_API int SQLITE_STDCALL sqlite3_bind_text16(sqlite3_stmt*, int, const void*, int, void(*)(void*));
SQLITE_API int SQLITE_STDCALL sqlite3_bind_text64(sqlite3_stmt*, int, const char*, sqlite3_uint64,
                         void(*)(void*), unsigned char encoding);
SQLITE_API int SQLITE_STDCALL sqlite3_bind_value(sqlite3_stmt*, int, const sqlite3_value*);
SQLITE_API int SQLITE_STDCALL sqlite3_bind_zeroblob(sqlite3_stmt*, int, int n);
SQLITE_API int SQLITE_STDCALL sqlite3_bind_zeroblob64(sqlite3_stmt*, int, sqlite3_uint64);

/*
** CAPI3REF: Number Of SQL Parameters
** METHOD: sqlite3_stmt
**
** ^This routine can be used to find the number of [SQL parameters]
** in a [prepared statement].  SQL parameters are tokens of the
** form "?", "?NNN", ":AAA", "$AAA", or "@AAA" that serve as
** placeholders for values that are [sqlite3_bind_blob | bound]
** to the parameters at a later time.
**
** ^(This routine actually returns the index of the largest (rightmost)
** parameter. For all forms except ?NNN, this will correspond to the
** number of unique parameters.  If parameters of the ?NNN form are used,
** there may be gaps in the list.)^
**
** See also: [sqlite3_bind_blob|sqlite3_bind()],
** [sqlite3_bind_parameter_name()], and
** [sqlite3_bind_parameter_index()].
*/
SQLITE_API int SQLITE_STDCALL sqlite3_bind_parameter_count(sqlite3_stmt*);

/*
** CAPI3REF: Name Of A Host Parameter
** METHOD: sqlite3_stmt
**
** ^The sqlite3_bind_parameter_name(P,N) interface returns
** the name of the N-th [SQL parameter] in the [prepared statement] P.
** ^(SQL parameters of the form "?NNN" or ":AAA" or "@AAA" or "$AAA"
** have a name which is the string "?NNN" or ":AAA" or "@AAA" or "$AAA"
** respectively.
** In other words, the initial ":" or "$" or "@" or "?"
** is included as part of the name.)^
** ^Parameters of the form "?" without a following integer have no name
** and are referred to as "nameless" or "anonymous parameters".
**
** ^The first host parameter has an index of 1, not 0.
**
** ^If the value N is out of range or if the N-th parameter is
** nameless, then NULL is returned.  ^The returned string is
** always in UTF-8 encoding even if the named parameter was
** originally specified as UTF-16 in [sqlite3_prepare16()] or
** [sqlite3_prepare16_v2()].
**
** See also: [sqlite3_bind_blob|sqlite3_bind()],
** [sqlite3_bind_parameter_count()], and
** [sqlite3_bind_parameter_index()].
*/
SQLITE_API const char *SQLITE_STDCALL sqlite3_bind_parameter_name(sqlite3_stmt*, int);

/*
** CAPI3REF: Index Of A Parameter With A Given Name
** METHOD: sqlite3_stmt
**
** ^Return the index of an SQL parameter given its name.  ^The
** index value returned is suitable for use as the second
** parameter to [sqlite3_bind_blob|sqlite3_bind()].  ^A zero
** is returned if no matching parameter is found.  ^The parameter
** name must be given in UTF-8 even if the original statement
** was prepared from UTF-16 text using [sqlite3_prepare16_v2()].
**
** See also: [sqlite3_bind_blob|sqlite3_bind()],
** [sqlite3_bind_parameter_count()], and
** [sqlite3_bind_parameter_name()].
*/
SQLITE_API int SQLITE_STDCALL sqlite3_bind_parameter_index(sqlite3_stmt*, const char *zName);

/*
** CAPI3REF: Reset All Bindings On A Prepared Statement
** METHOD: sqlite3_stmt
**
** ^Contrary to the intuition of many, [sqlite3_reset()] does not reset
** the [sqlite3_bind_blob | bindings] on a [prepared statement].
** ^Use this routine to reset all host parameters to NULL.
*/
SQLITE_API int SQLITE_STDCALL sqlite3_clear_bindings(sqlite3_stmt*);

/*
** CAPI3REF: Number Of Columns In A Result Set
** METHOD: sqlite3_stmt
**
** ^Return the number of columns in the result set returned by the
** [prepared statement]. ^This routine returns 0 if pStmt is an SQL
** statement that does not return data (for example an [UPDATE]).
**
** See also: [sqlite3_data_count()]
*/
SQLITE_API int SQLITE_STDCALL sqlite3_column_count(sqlite3_stmt *pStmt);

/*
** CAPI3REF: Column Names In A Result Set
** METHOD: sqlite3_stmt
**
** ^These routines return the name assigned to a particular column
** in the result set of a [SELECT] statement.  ^The sqlite3_column_name()
** interface returns a pointer to a zero-terminated UTF-8 string
** and sqlite3_column_name16() returns a pointer to a zero-terminated
** UTF-16 string.  ^The first parameter is the [prepared statement]
** that implements the [SELECT] statement. ^The second parameter is the
** column number.  ^The leftmost column is number 0.
**
** ^The returned string pointer is valid until either the [prepared statement]
** is destroyed by [sqlite3_finalize()] or until the statement is automatically
** reprepared by the first call to [sqlite3_step()] for a particular run
** or until the next call to
** sqlite3_column_name() or sqlite3_column_name16() on the same column.
**
** ^If sqlite3_malloc() fails during the processing of either routine
** (for example during a conversion from UTF-8 to UTF-16) then a
** NULL pointer is returned.
**
** ^The name of a result column is the value of the "AS" clause for
** that column, if there is an AS clause.  If there is no AS clause
** then the name of the column is unspecified and may change from
** one release of SQLite to the next.
*/
SQLITE_API const char *SQLITE_STDCALL sqlite3_column_name(sqlite3_stmt*, int N);
SQLITE_API const void *SQLITE_STDCALL sqlite3_column_name16(sqlite3_stmt*, int N);

/*
** CAPI3REF: Source Of Data In A Query Result
** METHOD: sqlite3_stmt
**
** ^These routines provide a means to determine the database, table, and
** table column that is the origin of a particular result column in
** [SELECT] statement.
** ^The name of the database or table or column can be returned as
** either a UTF-8 or UTF-16 string.  ^The _database_ routines return
** the database name, the _table_ routines return the table name, and
** the origin_ routines return the column name.
** ^The returned string is valid until the [prepared statement] is destroyed
** using [sqlite3_finalize()] or until the statement is automatically
** reprepared by the first call to [sqlite3_step()] for a particular run
** or until the same information is requested
** again in a different encoding.
**
** ^The names returned are the original un-aliased names of the
** database, table, and column.
**
** ^The first argument to these interfaces is a [prepared statement].
** ^These functions return information about the Nth result column returned by
** the statement, where N is the second function argument.
** ^The left-most column is column 0 for these routines.
**
** ^If the Nth column returned by the statement is an expression or
** subquery and is not a column value, then all of these functions return
** NULL.  ^These routine might also return NULL if a memory allocation error
** occurs.  ^Otherwise, they return the name of the attached database, table,
** or column that query result column was extracted from.
**
** ^As with all other SQLite APIs, those whose names end with "16" return
** UTF-16 encoded strings and the other functions return UTF-8.
**
** ^These APIs are only available if the library was compiled with the
** [SQLITE_ENABLE_COLUMN_METADATA] C-preprocessor symbol.
**
** If two or more threads call one or more of these routines against the same
** prepared statement and column at the same time then the results are
** undefined.
**
** If two or more threads call one or more
** [sqlite3_column_database_name | column metadata interfaces]
** for the same [prepared statement] and result column
** at the same time then the results are undefined.
*/
SQLITE_API const char *SQLITE_STDCALL sqlite3_column_database_name(sqlite3_stmt*,int);
SQLITE_API const void *SQLITE_STDCALL sqlite3_column_database_name16(sqlite3_stmt*,int);
SQLITE_API const char *SQLITE_STDCALL sqlite3_column_table_name(sqlite3_stmt*,int);
SQLITE_API const void *SQLITE_STDCALL sqlite3_column_table_name16(sqlite3_stmt*,int);
SQLITE_API const char *SQLITE_STDCALL sqlite3_column_origin_name(sqlite3_stmt*,int);
SQLITE_API const void *SQLITE_STDCALL sqlite3_column_origin_name16(sqlite3_stmt*,int);

/*
** CAPI3REF: Declared Datatype Of A Query Result
** METHOD: sqlite3_stmt
**
** ^(The first parameter is a [prepared statement].
** If this statement is a [SELECT] statement and the Nth column of the
** returned result set of that [SELECT] is a table column (not an
** expression or subquery) then the declared type of the table
** column is returned.)^  ^If the Nth column of the result set is an
** expression or subquery, then a NULL pointer is returned.
** ^The returned string is always UTF-8 encoded.
**
** ^(For example, given the database schema:
**
** CREATE TABLE t1(c1 VARIANT);
**
** and the following statement to be compiled:
**
** SELECT c1 + 1, c1 FROM t1;
**
** this routine would return the string "VARIANT" for the second result
** column (i==1), and a NULL pointer for the first result column (i==0).)^
**
** ^SQLite uses dynamic run-time typing.  ^So just because a column
** is declared to contain a particular type does not mean that the
** data stored in that column is of the declared type.  SQLite is
** strongly typed, but the typing is dynamic not static.  ^Type
** is associated with individual values, not with the containers
** used to hold those values.
*/
SQLITE_API const char *SQLITE_STDCALL sqlite3_column_decltype(sqlite3_stmt*,int);
SQLITE_API const void *SQLITE_STDCALL sqlite3_column_decltype16(sqlite3_stmt*,int);

/*
** CAPI3REF: Evaluate An SQL Statement
** METHOD: sqlite3_stmt
**
** After a [prepared statement] has been prepared using either
** [sqlite3_prepare_v2()] or [sqlite3_prepare16_v2()] or one of the legacy
** interfaces [sqlite3_prepare()] or [sqlite3_prepare16()], this function
** must be called one or more times to evaluate the statement.
**
** The details of the behavior of the sqlite3_step() interface depend
** on whether the statement was prepared using the newer "v2" interface
** [sqlite3_prepare_v2()] and [sqlite3_prepare16_v2()] or the older legacy
** interface [sqlite3_prepare()] and [sqlite3_prepare16()].  The use of the
** new "v2" interface is recommended for new applications but the legacy
** interface will continue to be supported.
**
** ^In the legacy interface, the return value will be either [SQLITE_BUSY],
** [SQLITE_DONE], [SQLITE_ROW], [SQLITE_ERROR], or [SQLITE_MISUSE].
** ^With the "v2" interface, any of the other [result codes] or
** [extended result codes] might be returned as well.
**
** ^[SQLITE_BUSY] means that the database engine was unable to acquire the
** database locks it needs to do its job.  ^If the statement is a [COMMIT]
** or occurs outside of an explicit transaction, then you can retry the
** statement.  If the statement is not a [COMMIT] and occurs within an
** explicit transaction then you should rollback the transaction before
** continuing.
**
** ^[SQLITE_DONE] means that the statement has finished executing
** successfully.  sqlite3_step() should not be called again on this virtual
** machine without first calling [sqlite3_reset()] to reset the virtual
** machine back to its initial state.
**
** ^If the SQL statement being executed returns any data, then [SQLITE_ROW]
** is returned each time a new row of data is ready for processing by the
** caller. The values may be accessed using the [column access functions].
** sqlite3_step() is called again to retrieve the next row of data.
**
** ^[SQLITE_ERROR] means that a run-time error (such as a constraint
** violation) has occurred.  sqlite3_step() should not be called again on
** the VM. More information may be found by calling [sqlite3_errmsg()].
** ^With the legacy interface, a more specific error code (for example,
** [SQLITE_INTERRUPT], [SQLITE_SCHEMA], [SQLITE_CORRUPT], and so forth)
** can be obtained by calling [sqlite3_reset()] on the
** [prepared statement].  ^In the "v2" interface,
** the more specific error code is returned directly by sqlite3_step().
**
** [SQLITE_MISUSE] means that the this routine was called inappropriately.
** Perhaps it was called on a [prepared statement] that has
** already been [sqlite3_finalize | finalized] or on one that had
** previously returned [SQLITE_ERROR] or [SQLITE_DONE].  Or it could
** be the case that the same database connection is being used by two or
** more threads at the same moment in time.
**
** For all versions of SQLite up to and including 3.6.23.1, a call to
** [sqlite3_reset()] was required after sqlite3_step() returned anything
** other than [SQLITE_ROW] before any subsequent invocation of
** sqlite3_step().  Failure to reset the prepared statement using 
** [sqlite3_reset()] would result in an [SQLITE_MISUSE] return from
** sqlite3_step().  But after version 3.6.23.1, sqlite3_step() began
** calling [sqlite3_reset()] automatically in this circumstance rather
** than returning [SQLITE_MISUSE].  This is not considered a compatibility
** break because any application that ever receives an SQLITE_MISUSE error
** is broken by definition.  The [SQLITE_OMIT_AUTORESET] compile-time option
** can be used to restore the legacy behavior.
**
** <b>Goofy Interface Alert:</b> In the legacy interface, the sqlite3_step()
** API always returns a generic error code, [SQLITE_ERROR], following any
** error other than [SQLITE_BUSY] and [SQLITE_MISUSE].  You must call
** [sqlite3_reset()] or [sqlite3_finalize()] in order to find one of the
** specific [error codes] that better describes the error.
** We admit that this is a goofy design.  The problem has been fixed
** with the "v2" interface.  If you prepare all of your SQL statements
** using either [sqlite3_prepare_v2()] or [sqlite3_prepare16_v2()] instead
** of the legacy [sqlite3_prepare()] and [sqlite3_prepare16()] interfaces,
** then the more specific [error codes] are returned directly
** by sqlite3_step().  The use of the "v2" interface is recommended.
*/
SQLITE_API int SQLITE_STDCALL sqlite3_step(sqlite3_stmt*);

/*
** CAPI3REF: Number of columns in a result set
** METHOD: sqlite3_stmt
**
** ^The sqlite3_data_count(P) interface returns the number of columns in the
** current row of the result set of [prepared statement] P.
** ^If prepared statement P does not have results ready to return
** (via calls to the [sqlite3_column_int | sqlite3_column_*()] of
** interfaces) then sqlite3_data_count(P) returns 0.
** ^The sqlite3_data_count(P) routine also returns 0 if P is a NULL pointer.
** ^The sqlite3_data_count(P) routine returns 0 if the previous call to
** [sqlite3_step](P) returned [SQLITE_DONE].  ^The sqlite3_data_count(P)
** will return non-zero if previous call to [sqlite3_step](P) returned
** [SQLITE_ROW], except in the case of the [PRAGMA incremental_vacuum]
** where it always returns zero since each step of that multi-step
** pragma returns 0 columns of data.
**
** See also: [sqlite3_column_count()]
*/
SQLITE_API int SQLITE_STDCALL sqlite3_data_count(sqlite3_stmt *pStmt);

/*
** CAPI3REF: Fundamental Datatypes
** KEYWORDS: SQLITE_TEXT
**
** ^(Every value in SQLite has one of five fundamental datatypes:
**
** <ul>
** <li> 64-bit signed integer
** <li> 64-bit IEEE floating point number
** <li> string
** <li> BLOB
** <li> NULL
** </ul>)^
**
** These constants are codes for each of those types.
**
** Note that the SQLITE_TEXT constant was also used in SQLite version 2
** for a completely different meaning.  Software that links against both
** SQLite version 2 and SQLite version 3 should use SQLITE3_TEXT, not
** SQLITE_TEXT.
*/
#define SQLITE_INTEGER  1
#define SQLITE_FLOAT    2
#define SQLITE_BLOB     4
#define SQLITE_NULL     5
#ifdef SQLITE_TEXT
# undef SQLITE_TEXT
#else
# define SQLITE_TEXT     3
#endif
#define SQLITE3_TEXT     3

/*
** CAPI3REF: Result Values From A Query
** KEYWORDS: {column access functions}
** METHOD: sqlite3_stmt
**
** ^These routines return information about a single column of the current
** result row of a query.  ^In every case the first argument is a pointer
** to the [prepared statement] that is being evaluated (the [sqlite3_stmt*]
** that was returned from [sqlite3_prepare_v2()] or one of its variants)
** and the second argument is the index of the column for which information
** should be returned. ^The leftmost column of the result set has the index 0.
** ^The number of columns in the result can be determined using
** [sqlite3_column_count()].
**
** If the SQL statement does not currently point to a valid row, or if the
** column index is out of range, the result is undefined.
** These routines may only be called when the most recent call to
** [sqlite3_step()] has returned [SQLITE_ROW] and neither
** [sqlite3_reset()] nor [sqlite3_finalize()] have been called subsequently.
** If any of these routines are called after [sqlite3_reset()] or
** [sqlite3_finalize()] or after [sqlite3_step()] has returned
** something other than [SQLITE_ROW], the results are undefined.
** If [sqlite3_step()] or [sqlite3_reset()] or [sqlite3_finalize()]
** are called from a different thread while any of these routines
** are pending, then the results are undefined.
**
** ^The sqlite3_column_type() routine returns the
** [SQLITE_INTEGER | datatype code] for the initial data type
** of the result column.  ^The returned value is one of [SQLITE_INTEGER],
** [SQLITE_FLOAT], [SQLITE_TEXT], [SQLITE_BLOB], or [SQLITE_NULL].  The value
** returned by sqlite3_column_type() is only meaningful if no type
** conversions have occurred as described below.  After a type conversion,
** the value returned by sqlite3_column_type() is undefined.  Future
** versions of SQLite may change the behavior of sqlite3_column_type()
** following a type conversion.
**
** ^If the result is a BLOB or UTF-8 string then the sqlite3_column_bytes()
** routine returns the number of bytes in that BLOB or string.
** ^If the result is a UTF-16 string, then sqlite3_column_bytes() converts
** the string to UTF-8 and then returns the number of bytes.
** ^If the result is a numeric value then sqlite3_column_bytes() uses
** [sqlite3_snprintf()] to convert that value to a UTF-8 string and returns
** the number of bytes in that string.
** ^If the result is NULL, then sqlite3_column_bytes() returns zero.
**
** ^If the result is a BLOB or UTF-16 string then the sqlite3_column_bytes16()
** routine returns the number of bytes in that BLOB or string.
** ^If the result is a UTF-8 string, then sqlite3_column_bytes16() converts
** the string to UTF-16 and then returns the number of bytes.
** ^If the result is a numeric value then sqlite3_column_bytes16() uses
** [sqlite3_snprintf()] to convert that value to a UTF-16 string and returns
** the number of bytes in that string.
** ^If the result is NULL, then sqlite3_column_bytes16() returns zero.
**
** ^The values returned by [sqlite3_column_bytes()] and 
** [sqlite3_column_bytes16()] do not include the zero terminators at the end
** of the string.  ^For clarity: the values returned by
** [sqlite3_column_bytes()] and [sqlite3_column_bytes16()] are the number of
** bytes in the string, not the number of characters.
**
** ^Strings returned by sqlite3_column_text() and sqlite3_column_text16(),
** even empty strings, are always zero-terminated.  ^The return
** value from sqlite3_column_blob() for a zero-length BLOB is a NULL pointer.
**
** <b>Warning:</b> ^The object returned by [sqlite3_column_value()] is an
** [unprotected sqlite3_value] object.  In a multithreaded environment,
** an unprotected sqlite3_value object may only be used safely with
** [sqlite3_bind_value()] and [sqlite3_result_value()].
** If the [unprotected sqlite3_value] object returned by
** [sqlite3_column_value()] is used in any other way, including calls
** to routines like [sqlite3_value_int()], [sqlite3_value_text()],
** or [sqlite3_value_bytes()], the behavior is not threadsafe.
**
** These routines attempt to convert the value where appropriate.  ^For
** example, if the internal representation is FLOAT and a text result
** is requested, [sqlite3_snprintf()] is used internally to perform the
** conversion automatically.  ^(The following table details the conversions
** that are applied:
**
** <blockquote>
** <table border="1">
** <tr><th> Internal<br>Type <th> Requested<br>Type <th>  Conversion
**
** <tr><td>  NULL    <td> INTEGER   <td> Result is 0
** <tr><td>  NULL    <td>  FLOAT    <td> Result is 0.0
** <tr><td>  NULL    <td>   TEXT    <td> Result is a NULL pointer
** <tr><td>  NULL    <td>   BLOB    <td> Result is a NULL pointer
** <tr><td> INTEGER  <td>  FLOAT    <td> Convert from integer to float
** <tr><td> INTEGER  <td>   TEXT    <td> ASCII rendering of the integer
** <tr><td> INTEGER  <td>   BLOB    <td> Same as INTEGER->TEXT
** <tr><td>  FLOAT   <td> INTEGER   <td> [CAST] to INTEGER
** <tr><td>  FLOAT   <td>   TEXT    <td> ASCII rendering of the float
** <tr><td>  FLOAT   <td>   BLOB    <td> [CAST] to BLOB
** <tr><td>  TEXT    <td> INTEGER   <td> [CAST] to INTEGER
** <tr><td>  TEXT    <td>  FLOAT    <td> [CAST] to REAL
** <tr><td>  TEXT    <td>   BLOB    <td> No change
** <tr><td>  BLOB    <td> INTEGER   <td> [CAST] to INTEGER
** <tr><td>  BLOB    <td>  FLOAT    <td> [CAST] to REAL
** <tr><td>  BLOB    <td>   TEXT    <td> Add a zero terminator if needed
** </table>
** </blockquote>)^
**
** Note that when type conversions occur, pointers returned by prior
** calls to sqlite3_column_blob(), sqlite3_column_text(), and/or
** sqlite3_column_text16() may be invalidated.
** Type conversions and pointer invalidations might occur
** in the following cases:
**
** <ul>
** <li> The initial content is a BLOB and sqlite3_column_text() or
**      sqlite3_column_text16() is called.  A zero-terminator might
**      need to be added to the string.</li>
** <li> The initial content is UTF-8 text and sqlite3_column_bytes16() or
**      sqlite3_column_text16() is called.  The content must be converted
**      to UTF-16.</li>
** <li> The initial content is UTF-16 text and sqlite3_column_bytes() or
**      sqlite3_column_text() is called.  The content must be converted
**      to UTF-8.</li>
** </ul>
**
** ^Conversions between UTF-16be and UTF-16le are always done in place and do
** not invalidate a prior pointer, though of course the content of the buffer
** that the prior pointer references will have been modified.  Other kinds
** of conversion are done in place when it is possible, but sometimes they
** are not possible and in those cases prior pointers are invalidated.
**
** The safest policy is to invoke these routines
** in one of the following ways:
**
** <ul>
**  <li>sqlite3_column_text() followed by sqlite3_column_bytes()</li>
**  <li>sqlite3_column_blob() followed by sqlite3_column_bytes()</li>
**  <li>sqlite3_column_text16() followed by sqlite3_column_bytes16()</li>
** </ul>
**
** In other words, you should call sqlite3_column_text(),
** sqlite3_column_blob(), or sqlite3_column_text16() first to force the result
** into the desired format, then invoke sqlite3_column_bytes() or
** sqlite3_column_bytes16() to find the size of the result.  Do not mix calls
** to sqlite3_column_text() or sqlite3_column_blob() with calls to
** sqlite3_column_bytes16(), and do not mix calls to sqlite3_column_text16()
** with calls to sqlite3_column_bytes().
**
** ^The pointers returned are valid until a type conversion occurs as
** described above, or until [sqlite3_step()] or [sqlite3_reset()] or
** [sqlite3_finalize()] is called.  ^The memory space used to hold strings
** and BLOBs is freed automatically.  Do <em>not</em> pass the pointers returned
** from [sqlite3_column_blob()], [sqlite3_column_text()], etc. into
** [sqlite3_free()].
**
** ^(If a memory allocation error occurs during the evaluation of any
** of these routines, a default value is returned.  The default value
** is either the integer 0, the floating point number 0.0, or a NULL
** pointer.  Subsequent calls to [sqlite3_errcode()] will return
** [SQLITE_NOMEM].)^
*/
SQLITE_API const void *SQLITE_STDCALL sqlite3_column_blob(sqlite3_stmt*, int iCol);
SQLITE_API int SQLITE_STDCALL sqlite3_column_bytes(sqlite3_stmt*, int iCol);
SQLITE_API int SQLITE_STDCALL sqlite3_column_bytes16(sqlite3_stmt*, int iCol);
SQLITE_API double SQLITE_STDCALL sqlite3_column_double(sqlite3_stmt*, int iCol);
SQLITE_API int SQLITE_STDCALL sqlite3_column_int(sqlite3_stmt*, int iCol);
SQLITE_API sqlite3_int64 SQLITE_STDCALL sqlite3_column_int64(sqlite3_stmt*, int iCol);
SQLITE_API const unsigned char *SQLITE_STDCALL sqlite3_column_text(sqlite3_stmt*, int iCol);
SQLITE_API const void *SQLITE_STDCALL sqlite3_column_text16(sqlite3_stmt*, int iCol);
SQLITE_API int SQLITE_STDCALL sqlite3_column_type(sqlite3_stmt*, int iCol);
SQLITE_API sqlite3_value *SQLITE_STDCALL sqlite3_column_value(sqlite3_stmt*, int iCol);

/*
** CAPI3REF: Destroy A Prepared Statement Object
** DESTRUCTOR: sqlite3_stmt
**
** ^The sqlite3_finalize() function is called to delete a [prepared statement].
** ^If the most recent evaluation of the statement encountered no errors
** or if the statement is never been evaluated, then sqlite3_finalize() returns
** SQLITE_OK.  ^If the most recent evaluation of statement S failed, then
** sqlite3_finalize(S) returns the appropriate [error code] or
** [extended error code].
**
** ^The sqlite3_finalize(S) routine can be called at any point during
** the life cycle of [prepared statement] S:
** before statement S is ever evaluated, after
** one or more calls to [sqlite3_reset()], or after any call
** to [sqlite3_step()] regardless of whether or not the statement has
** completed execution.
**
** ^Invoking sqlite3_finalize() on a NULL pointer is a harmless no-op.
**
** The application must finalize every [prepared statement] in order to avoid
** resource leaks.  It is a grievous error for the application to try to use
** a prepared statement after it has been finalized.  Any use of a prepared
** statement after it has been finalized can result in undefined and
** undesirable behavior such as segfaults and heap corruption.
*/
SQLITE_API int SQLITE_STDCALL sqlite3_finalize(sqlite3_stmt *pStmt);

/*
** CAPI3REF: Reset A Prepared Statement Object
** METHOD: sqlite3_stmt
**
** The sqlite3_reset() function is called to reset a [prepared statement]
** object back to its initial state, ready to be re-executed.
** ^Any SQL statement variables that had values bound to them using
** the [sqlite3_bind_blob | sqlite3_bind_*() API] retain their values.
** Use [sqlite3_clear_bindings()] to reset the bindings.
**
** ^The [sqlite3_reset(S)] interface resets the [prepared statement] S
** back to the beginning of its program.
**
** ^If the most recent call to [sqlite3_step(S)] for the
** [prepared statement] S returned [SQLITE_ROW] or [SQLITE_DONE],
** or if [sqlite3_step(S)] has never before been called on S,
** then [sqlite3_reset(S)] returns [SQLITE_OK].
**
** ^If the most recent call to [sqlite3_step(S)] for the
** [prepared statement] S indicated an error, then
** [sqlite3_reset(S)] returns an appropriate [error code].
**
** ^The [sqlite3_reset(S)] interface does not change the values
** of any [sqlite3_bind_blob|bindings] on the [prepared statement] S.
*/
SQLITE_API int SQLITE_STDCALL sqlite3_reset(sqlite3_stmt *pStmt);

/*
** CAPI3REF: Create Or Redefine SQL Functions
** KEYWORDS: {function creation routines}
** KEYWORDS: {application-defined SQL function}
** KEYWORDS: {application-defined SQL functions}
** METHOD: sqlite3
**
** ^These functions (collectively known as "function creation routines")
** are used to add SQL functions or aggregates or to redefine the behavior
** of existing SQL functions or aggregates.  The only differences between
** these routines are the text encoding expected for
** the second parameter (the name of the function being created)
** and the presence or absence of a destructor callback for
** the application data pointer.
**
** ^The first parameter is the [database connection] to which the SQL
** function is to be added.  ^If an application uses more than one database
** connection then application-defined SQL functions must be added
** to each database connection separately.
**
** ^The second parameter is the name of the SQL function to be created or
** redefined.  ^The length of the name is limited to 255 bytes in a UTF-8
** representation, exclusive of the zero-terminator.  ^Note that the name
** length limit is in UTF-8 bytes, not characters nor UTF-16 bytes.  
** ^Any attempt to create a function with a longer name
** will result in [SQLITE_MISUSE] being returned.
**
** ^The third parameter (nArg)
** is the number of arguments that the SQL function or
** aggregate takes. ^If this parameter is -1, then the SQL function or
** aggregate may take any number of arguments between 0 and the limit
** set by [sqlite3_limit]([SQLITE_LIMIT_FUNCTION_ARG]).  If the third
** parameter is less than -1 or greater than 127 then the behavior is
** undefined.
**
** ^The fourth parameter, eTextRep, specifies what
** [SQLITE_UTF8 | text encoding] this SQL function prefers for
** its parameters.  The application should set this parameter to
** [SQLITE_UTF16LE] if the function implementation invokes 
** [sqlite3_value_text16le()] on an input, or [SQLITE_UTF16BE] if the
** implementation invokes [sqlite3_value_text16be()] on an input, or
** [SQLITE_UTF16] if [sqlite3_value_text16()] is used, or [SQLITE_UTF8]
** otherwise.  ^The same SQL function may be registered multiple times using
** different preferred text encodings, with different implementations for
** each encoding.
** ^When multiple implementations of the same function are available, SQLite
** will pick the one that involves the least amount of data conversion.
**
** ^The fourth parameter may optionally be ORed with [SQLITE_DETERMINISTIC]
** to signal that the function will always return the same result given
** the same inputs within a single SQL statement.  Most SQL functions are
** deterministic.  The built-in [random()] SQL function is an example of a
** function that is not deterministic.  The SQLite query planner is able to
** perform additional optimizations on deterministic functions, so use
** of the [SQLITE_DETERMINISTIC] flag is recommended where possible.
**
** ^(The fifth parameter is an arbitrary pointer.  The implementation of the
** function can gain access to this pointer using [sqlite3_user_data()].)^
**
** ^The sixth, seventh and eighth parameters, xFunc, xStep and xFinal, are
** pointers to C-language functions that implement the SQL function or
** aggregate. ^A scalar SQL function requires an implementation of the xFunc
** callback only; NULL pointers must be passed as the xStep and xFinal
** parameters. ^An aggregate SQL function requires an implementation of xStep
** and xFinal and NULL pointer must be passed for xFunc. ^To delete an existing
** SQL function or aggregate, pass NULL pointers for all three function
** callbacks.
**
** ^(If the ninth parameter to sqlite3_create_function_v2() is not NULL,
** then it is destructor for the application data pointer. 
** The destructor is invoked when the function is deleted, either by being
** overloaded or when the database connection closes.)^
** ^The destructor is also invoked if the call to
** sqlite3_create_function_v2() fails.
** ^When the destructor callback of the tenth parameter is invoked, it
** is passed a single argument which is a copy of the application data 
** pointer which was the fifth parameter to sqlite3_create_function_v2().
**
** ^It is permitted to register multiple implementations of the same
** functions with the same name but with either differing numbers of
** arguments or differing preferred text encodings.  ^SQLite will use
** the implementation that most closely matches the way in which the
** SQL function is used.  ^A function implementation with a non-negative
** nArg parameter is a better match than a function implementation with
** a negative nArg.  ^A function where the preferred text encoding
** matches the database encoding is a better
** match than a function where the encoding is different.  
** ^A function where the encoding difference is between UTF16le and UTF16be
** is a closer match than a function where the encoding difference is
** between UTF8 and UTF16.
**
** ^Built-in functions may be overloaded by new application-defined functions.
**
** ^An application-defined function is permitted to call other
** SQLite interfaces.  However, such calls must not
** close the database connection nor finalize or reset the prepared
** statement in which the function is running.
*/
SQLITE_API int SQLITE_STDCALL sqlite3_create_function(
  sqlite3 *db,
  const char *zFunctionName,
  int nArg,
  int eTextRep,
  void *pApp,
  void (*xFunc)(sqlite3_context*,int,sqlite3_value**),
  void (*xStep)(sqlite3_context*,int,sqlite3_value**),
  void (*xFinal)(sqlite3_context*)
);
SQLITE_API int SQLITE_STDCALL sqlite3_create_function16(
  sqlite3 *db,
  const void *zFunctionName,
  int nArg,
  int eTextRep,
  void *pApp,
  void (*xFunc)(sqlite3_context*,int,sqlite3_value**),
  void (*xStep)(sqlite3_context*,int,sqlite3_value**),
  void (*xFinal)(sqlite3_context*)
);
SQLITE_API int SQLITE_STDCALL sqlite3_create_function_v2(
  sqlite3 *db,
  const char *zFunctionName,
  int nArg,
  int eTextRep,
  void *pApp,
  void (*xFunc)(sqlite3_context*,int,sqlite3_value**),
  void (*xStep)(sqlite3_context*,int,sqlite3_value**),
  void (*xFinal)(sqlite3_context*),
  void(*xDestroy)(void*)
);

/*
** CAPI3REF: Text Encodings
**
** These constant define integer codes that represent the various
** text encodings supported by SQLite.
*/
#define SQLITE_UTF8           1    /* IMP: R-37514-35566 */
#define SQLITE_UTF16LE        2    /* IMP: R-03371-37637 */
#define SQLITE_UTF16BE        3    /* IMP: R-51971-34154 */
#define SQLITE_UTF16          4    /* Use native byte order */
#define SQLITE_ANY            5    /* Deprecated */
#define SQLITE_UTF16_ALIGNED  8    /* sqlite3_create_collation only */

/*
** CAPI3REF: Function Flags
**
** These constants may be ORed together with the 
** [SQLITE_UTF8 | preferred text encoding] as the fourth argument
** to [sqlite3_create_function()], [sqlite3_create_function16()], or
** [sqlite3_create_function_v2()].
*/
#define SQLITE_DETERMINISTIC    0x800

/*
** CAPI3REF: Deprecated Functions
** DEPRECATED
**
** These functions are [deprecated].  In order to maintain
** backwards compatibility with older code, these functions continue 
** to be supported.  However, new applications should avoid
** the use of these functions.  To encourage programmers to avoid
** these functions, we will not explain what they do.
*/
#ifndef SQLITE_OMIT_DEPRECATED
SQLITE_API SQLITE_DEPRECATED int SQLITE_STDCALL sqlite3_aggregate_count(sqlite3_context*);
SQLITE_API SQLITE_DEPRECATED int SQLITE_STDCALL sqlite3_expired(sqlite3_stmt*);
SQLITE_API SQLITE_DEPRECATED int SQLITE_STDCALL sqlite3_transfer_bindings(sqlite3_stmt*, sqlite3_stmt*);
SQLITE_API SQLITE_DEPRECATED int SQLITE_STDCALL sqlite3_global_recover(void);
SQLITE_API SQLITE_DEPRECATED void SQLITE_STDCALL sqlite3_thread_cleanup(void);
SQLITE_API SQLITE_DEPRECATED int SQLITE_STDCALL sqlite3_memory_alarm(void(*)(void*,sqlite3_int64,int),
                      void*,sqlite3_int64);
#endif

/*
** CAPI3REF: Obtaining SQL Values
** METHOD: sqlite3_value
**
** The C-language implementation of SQL functions and aggregates uses
** this set of interface routines to access the parameter values on
** the function or aggregate.  
**
** The xFunc (for scalar functions) or xStep (for aggregates) parameters
** to [sqlite3_create_function()] and [sqlite3_create_function16()]
** define callbacks that implement the SQL functions and aggregates.
** The 3rd parameter to these callbacks is an array of pointers to
** [protected sqlite3_value] objects.  There is one [sqlite3_value] object for
** each parameter to the SQL function.  These routines are used to
** extract values from the [sqlite3_value] objects.
**
** These routines work only with [protected sqlite3_value] objects.
** Any attempt to use these routines on an [unprotected sqlite3_value]
** object results in undefined behavior.
**
** ^These routines work just like the corresponding [column access functions]
** except that these routines take a single [protected sqlite3_value] object
** pointer instead of a [sqlite3_stmt*] pointer and an integer column number.
**
** ^The sqlite3_value_text16() interface extracts a UTF-16 string
** in the native byte-order of the host machine.  ^The
** sqlite3_value_text16be() and sqlite3_value_text16le() interfaces
** extract UTF-16 strings as big-endian and little-endian respectively.
**
** ^(The sqlite3_value_numeric_type() interface attempts to apply
** numeric affinity to the value.  This means that an attempt is
** made to convert the value to an integer or floating point.  If
** such a conversion is possible without loss of information (in other
** words, if the value is a string that looks like a number)
** then the conversion is performed.  Otherwise no conversion occurs.
** The [SQLITE_INTEGER | datatype] after conversion is returned.)^
**
** Please pay particular attention to the fact that the pointer returned
** from [sqlite3_value_blob()], [sqlite3_value_text()], or
** [sqlite3_value_text16()] can be invalidated by a subsequent call to
** [sqlite3_value_bytes()], [sqlite3_value_bytes16()], [sqlite3_value_text()],
** or [sqlite3_value_text16()].
**
** These routines must be called from the same thread as
** the SQL function that supplied the [sqlite3_value*] parameters.
*/
SQLITE_API const void *SQLITE_STDCALL sqlite3_value_blob(sqlite3_value*);
SQLITE_API int SQLITE_STDCALL sqlite3_value_bytes(sqlite3_value*);
SQLITE_API int SQLITE_STDCALL sqlite3_value_bytes16(sqlite3_value*);
SQLITE_API double SQLITE_STDCALL sqlite3_value_double(sqlite3_value*);
SQLITE_API int SQLITE_STDCALL sqlite3_value_int(sqlite3_value*);
SQLITE_API sqlite3_int64 SQLITE_STDCALL sqlite3_value_int64(sqlite3_value*);
SQLITE_API const unsigned char *SQLITE_STDCALL sqlite3_value_text(sqlite3_value*);
SQLITE_API const void *SQLITE_STDCALL sqlite3_value_text16(sqlite3_value*);
SQLITE_API const void *SQLITE_STDCALL sqlite3_value_text16le(sqlite3_value*);
SQLITE_API const void *SQLITE_STDCALL sqlite3_value_text16be(sqlite3_value*);
SQLITE_API int SQLITE_STDCALL sqlite3_value_type(sqlite3_value*);
SQLITE_API int SQLITE_STDCALL sqlite3_value_numeric_type(sqlite3_value*);

/*
** CAPI3REF: Finding The Subtype Of SQL Values
** METHOD: sqlite3_value
**
** The sqlite3_value_subtype(V) function returns the subtype for
** an [application-defined SQL function] argument V.  The subtype
** information can be used to pass a limited amount of context from
** one SQL function to another.  Use the [sqlite3_result_subtype()]
** routine to set the subtype for the return value of an SQL function.
**
** SQLite makes no use of subtype itself.  It merely passes the subtype
** from the result of one [application-defined SQL function] into the
** input of another.
*/
SQLITE_API unsigned int SQLITE_STDCALL sqlite3_value_subtype(sqlite3_value*);

/*
** CAPI3REF: Copy And Free SQL Values
** METHOD: sqlite3_value
**
** ^The sqlite3_value_dup(V) interface makes a copy of the [sqlite3_value]
** object D and returns a pointer to that copy.  ^The [sqlite3_value] returned
** is a [protected sqlite3_value] object even if the input is not.
** ^The sqlite3_value_dup(V) interface returns NULL if V is NULL or if a
** memory allocation fails.
**
** ^The sqlite3_value_free(V) interface frees an [sqlite3_value] object
** previously obtained from [sqlite3_value_dup()].  ^If V is a NULL pointer
** then sqlite3_value_free(V) is a harmless no-op.
*/
SQLITE_API sqlite3_value *SQLITE_STDCALL sqlite3_value_dup(const sqlite3_value*);
SQLITE_API void SQLITE_STDCALL sqlite3_value_free(sqlite3_value*);

/*
** CAPI3REF: Obtain Aggregate Function Context
** METHOD: sqlite3_context
**
** Implementations of aggregate SQL functions use this
** routine to allocate memory for storing their state.
**
** ^The first time the sqlite3_aggregate_context(C,N) routine is called 
** for a particular aggregate function, SQLite
** allocates N of memory, zeroes out that memory, and returns a pointer
** to the new memory. ^On second and subsequent calls to
** sqlite3_aggregate_context() for the same aggregate function instance,
** the same buffer is returned.  Sqlite3_aggregate_context() is normally
** called once for each invocation of the xStep callback and then one
** last time when the xFinal callback is invoked.  ^(When no rows match
** an aggregate query, the xStep() callback of the aggregate function
** implementation is never called and xFinal() is called exactly once.
** In those cases, sqlite3_aggregate_context() might be called for the
** first time from within xFinal().)^
**
** ^The sqlite3_aggregate_context(C,N) routine returns a NULL pointer 
** when first called if N is less than or equal to zero or if a memory
** allocate error occurs.
**
** ^(The amount of space allocated by sqlite3_aggregate_context(C,N) is
** determined by the N parameter on first successful call.  Changing the
** value of N in subsequent call to sqlite3_aggregate_context() within
** the same aggregate function instance will not resize the memory
** allocation.)^  Within the xFinal callback, it is customary to set
** N=0 in calls to sqlite3_aggregate_context(C,N) so that no 
** pointless memory allocations occur.
**
** ^SQLite automatically frees the memory allocated by 
** sqlite3_aggregate_context() when the aggregate query concludes.
**
** The first parameter must be a copy of the
** [sqlite3_context | SQL function context] that is the first parameter
** to the xStep or xFinal callback routine that implements the aggregate
** function.
**
** This routine must be called from the same thread in which
** the aggregate SQL function is running.
*/
SQLITE_API void *SQLITE_STDCALL sqlite3_aggregate_context(sqlite3_context*, int nBytes);

/*
** CAPI3REF: User Data For Functions
** METHOD: sqlite3_context
**
** ^The sqlite3_user_data() interface returns a copy of
** the pointer that was the pUserData parameter (the 5th parameter)
** of the [sqlite3_create_function()]
** and [sqlite3_create_function16()] routines that originally
** registered the application defined function.
**
** This routine must be called from the same thread in which
** the application-defined function is running.
*/
SQLITE_API void *SQLITE_STDCALL sqlite3_user_data(sqlite3_context*);

/*
** CAPI3REF: Database Connection For Functions
** METHOD: sqlite3_context
**
** ^The sqlite3_context_db_handle() interface returns a copy of
** the pointer to the [database connection] (the 1st parameter)
** of the [sqlite3_create_function()]
** and [sqlite3_create_function16()] routines that originally
** registered the application defined function.
*/
SQLITE_API sqlite3 *SQLITE_STDCALL sqlite3_context_db_handle(sqlite3_context*);

/*
** CAPI3REF: Function Auxiliary Data
** METHOD: sqlite3_context
**
** These functions may be used by (non-aggregate) SQL functions to
** associate metadata with argument values. If the same value is passed to
** multiple invocations of the same SQL function during query execution, under
** some circumstances the associated metadata may be preserved.  An example
** of where this might be useful is in a regular-expression matching
** function. The compiled version of the regular expression can be stored as
** metadata associated with the pattern string.  
** Then as long as the pattern string remains the same,
** the compiled regular expression can be reused on multiple
** invocations of the same function.
**
** ^The sqlite3_get_auxdata() interface returns a pointer to the metadata
** associated by the sqlite3_set_auxdata() function with the Nth argument
** value to the application-defined function. ^If there is no metadata
** associated with the function argument, this sqlite3_get_auxdata() interface
** returns a NULL pointer.
**
** ^The sqlite3_set_auxdata(C,N,P,X) interface saves P as metadata for the N-th
** argument of the application-defined function.  ^Subsequent
** calls to sqlite3_get_auxdata(C,N) return P from the most recent
** sqlite3_set_auxdata(C,N,P,X) call if the metadata is still valid or
** NULL if the metadata has been discarded.
** ^After each call to sqlite3_set_auxdata(C,N,P,X) where X is not NULL,
** SQLite will invoke the destructor function X with parameter P exactly
** once, when the metadata is discarded.
** SQLite is free to discard the metadata at any time, including: <ul>
** <li> when the corresponding function parameter changes, or
** <li> when [sqlite3_reset()] or [sqlite3_finalize()] is called for the
**      SQL statement, or
** <li> when sqlite3_set_auxdata() is invoked again on the same parameter, or
** <li> during the original sqlite3_set_auxdata() call when a memory 
**      allocation error occurs. </ul>)^
**
** Note the last bullet in particular.  The destructor X in 
** sqlite3_set_auxdata(C,N,P,X) might be called immediately, before the
** sqlite3_set_auxdata() interface even returns.  Hence sqlite3_set_auxdata()
** should be called near the end of the function implementation and the
** function implementation should not make any use of P after
** sqlite3_set_auxdata() has been called.
**
** ^(In practice, metadata is preserved between function calls for
** function parameters that are compile-time constants, including literal
** values and [parameters] and expressions composed from the same.)^
**
** These routines must be called from the same thread in which
** the SQL function is running.
*/
SQLITE_API void *SQLITE_STDCALL sqlite3_get_auxdata(sqlite3_context*, int N);
SQLITE_API void SQLITE_STDCALL sqlite3_set_auxdata(sqlite3_context*, int N, void*, void (*)(void*));


/*
** CAPI3REF: Constants Defining Special Destructor Behavior
**
** These are special values for the destructor that is passed in as the
** final argument to routines like [sqlite3_result_blob()].  ^If the destructor
** argument is SQLITE_STATIC, it means that the content pointer is constant
** and will never change.  It does not need to be destroyed.  ^The
** SQLITE_TRANSIENT value means that the content will likely change in
** the near future and that SQLite should make its own private copy of
** the content before returning.
**
** The typedef is necessary to work around problems in certain
** C++ compilers.
*/
typedef void (*sqlite3_destructor_type)(void*);
#define SQLITE_STATIC      ((sqlite3_destructor_type)0)
#define SQLITE_TRANSIENT   ((sqlite3_destructor_type)-1)

/*
** CAPI3REF: Setting The Result Of An SQL Function
** METHOD: sqlite3_context
**
** These routines are used by the xFunc or xFinal callbacks that
** implement SQL functions and aggregates.  See
** [sqlite3_create_function()] and [sqlite3_create_function16()]
** for additional information.
**
** These functions work very much like the [parameter binding] family of
** functions used to bind values to host parameters in prepared statements.
** Refer to the [SQL parameter] documentation for additional information.
**
** ^The sqlite3_result_blob() interface sets the result from
** an application-defined function to be the BLOB whose content is pointed
** to by the second parameter and which is N bytes long where N is the
** third parameter.
**
** ^The sqlite3_result_zeroblob(C,N) and sqlite3_result_zeroblob64(C,N)
** interfaces set the result of the application-defined function to be
** a BLOB containing all zero bytes and N bytes in size.
**
** ^The sqlite3_result_double() interface sets the result from
** an application-defined function to be a floating point value specified
** by its 2nd argument.
**
** ^The sqlite3_result_error() and sqlite3_result_error16() functions
** cause the implemented SQL function to throw an exception.
** ^SQLite uses the string pointed to by the
** 2nd parameter of sqlite3_result_error() or sqlite3_result_error16()
** as the text of an error message.  ^SQLite interprets the error
** message string from sqlite3_result_error() as UTF-8. ^SQLite
** interprets the string from sqlite3_result_error16() as UTF-16 in native
** byte order.  ^If the third parameter to sqlite3_result_error()
** or sqlite3_result_error16() is negative then SQLite takes as the error
** message all text up through the first zero character.
** ^If the third parameter to sqlite3_result_error() or
** sqlite3_result_error16() is non-negative then SQLite takes that many
** bytes (not characters) from the 2nd parameter as the error message.
** ^The sqlite3_result_error() and sqlite3_result_error16()
** routines make a private copy of the error message text before
** they return.  Hence, the calling function can deallocate or
** modify the text after they return without harm.
** ^The sqlite3_result_error_code() function changes the error code
** returned by SQLite as a result of an error in a function.  ^By default,
** the error code is SQLITE_ERROR.  ^A subsequent call to sqlite3_result_error()
** or sqlite3_result_error16() resets the error code to SQLITE_ERROR.
**
** ^The sqlite3_result_error_toobig() interface causes SQLite to throw an
** error indicating that a string or BLOB is too long to represent.
**
** ^The sqlite3_result_error_nomem() interface causes SQLite to throw an
** error indicating that a memory allocation failed.
**
** ^The sqlite3_result_int() interface sets the return value
** of the application-defined function to be the 32-bit signed integer
** value given in the 2nd argument.
** ^The sqlite3_result_int64() interface sets the return value
** of the application-defined function to be the 64-bit signed integer
** value given in the 2nd argument.
**
** ^The sqlite3_result_null() interface sets the return value
** of the application-defined function to be NULL.
**
** ^The sqlite3_result_text(), sqlite3_result_text16(),
** sqlite3_result_text16le(), and sqlite3_result_text16be() interfaces
** set the return value of the application-defined function to be
** a text string which is represented as UTF-8, UTF-16 native byte order,
** UTF-16 little endian, or UTF-16 big endian, respectively.
** ^The sqlite3_result_text64() interface sets the return value of an
** application-defined function to be a text string in an encoding
** specified by the fifth (and last) parameter, which must be one
** of [SQLITE_UTF8], [SQLITE_UTF16], [SQLITE_UTF16BE], or [SQLITE_UTF16LE].
** ^SQLite takes the text result from the application from
** the 2nd parameter of the sqlite3_result_text* interfaces.
** ^If the 3rd parameter to the sqlite3_result_text* interfaces
** is negative, then SQLite takes result text from the 2nd parameter
** through the first zero character.
** ^If the 3rd parameter to the sqlite3_result_text* interfaces
** is non-negative, then as many bytes (not characters) of the text
** pointed to by the 2nd parameter are taken as the application-defined
** function result.  If the 3rd parameter is non-negative, then it
** must be the byte offset into the string where the NUL terminator would
** appear if the string where NUL terminated.  If any NUL characters occur
** in the string at a byte offset that is less than the value of the 3rd
** parameter, then the resulting string will contain embedded NULs and the
** result of expressions operating on strings with embedded NULs is undefined.
** ^If the 4th parameter to the sqlite3_result_text* interfaces
** or sqlite3_result_blob is a non-NULL pointer, then SQLite calls that
** function as the destructor on the text or BLOB result when it has
** finished using that result.
** ^If the 4th parameter to the sqlite3_result_text* interfaces or to
** sqlite3_result_blob is the special constant SQLITE_STATIC, then SQLite
** assumes that the text or BLOB result is in constant space and does not
** copy the content of the parameter nor call a destructor on the content
** when it has finished using that result.
** ^If the 4th parameter to the sqlite3_result_text* interfaces
** or sqlite3_result_blob is the special constant SQLITE_TRANSIENT
** then SQLite makes a copy of the result into space obtained from
** from [sqlite3_malloc()] before it returns.
**
** ^The sqlite3_result_value() interface sets the result of
** the application-defined function to be a copy of the
** [unprotected sqlite3_value] object specified by the 2nd parameter.  ^The
** sqlite3_result_value() interface makes a copy of the [sqlite3_value]
** so that the [sqlite3_value] specified in the parameter may change or
** be deallocated after sqlite3_result_value() returns without harm.
** ^A [protected sqlite3_value] object may always be used where an
** [unprotected sqlite3_value] object is required, so either
** kind of [sqlite3_value] object can be used with this interface.
**
** If these routines are called from within the different thread
** than the one containing the application-defined function that received
** the [sqlite3_context] pointer, the results are undefined.
*/
SQLITE_API void SQLITE_STDCALL sqlite3_result_blob(sqlite3_context*, const void*, int, void(*)(void*));
SQLITE_API void SQLITE_STDCALL sqlite3_result_blob64(sqlite3_context*,const void*,
                           sqlite3_uint64,void(*)(void*));
SQLITE_API void SQLITE_STDCALL sqlite3_result_double(sqlite3_context*, double);
SQLITE_API void SQLITE_STDCALL sqlite3_result_error(sqlite3_context*, const char*, int);
SQLITE_API void SQLITE_STDCALL sqlite3_result_error16(sqlite3_context*, const void*, int);
SQLITE_API void SQLITE_STDCALL sqlite3_result_error_toobig(sqlite3_context*);
SQLITE_API void SQLITE_STDCALL sqlite3_result_error_nomem(sqlite3_context*);
SQLITE_API void SQLITE_STDCALL sqlite3_result_error_code(sqlite3_context*, int);
SQLITE_API void SQLITE_STDCALL sqlite3_result_int(sqlite3_context*, int);
SQLITE_API void SQLITE_STDCALL sqlite3_result_int64(sqlite3_context*, sqlite3_int64);
SQLITE_API void SQLITE_STDCALL sqlite3_result_null(sqlite3_context*);
SQLITE_API void SQLITE_STDCALL sqlite3_result_text(sqlite3_context*, const char*, int, void(*)(void*));
SQLITE_API void SQLITE_STDCALL sqlite3_result_text64(sqlite3_context*, const char*,sqlite3_uint64,
                           void(*)(void*), unsigned char encoding);
SQLITE_API void SQLITE_STDCALL sqlite3_result_text16(sqlite3_context*, const void*, int, void(*)(void*));
SQLITE_API void SQLITE_STDCALL sqlite3_result_text16le(sqlite3_context*, const void*, int,void(*)(void*));
SQLITE_API void SQLITE_STDCALL sqlite3_result_text16be(sqlite3_context*, const void*, int,void(*)(void*));
SQLITE_API void SQLITE_STDCALL sqlite3_result_value(sqlite3_context*, sqlite3_value*);
SQLITE_API void SQLITE_STDCALL sqlite3_result_zeroblob(sqlite3_context*, int n);
SQLITE_API int SQLITE_STDCALL sqlite3_result_zeroblob64(sqlite3_context*, sqlite3_uint64 n);


/*
** CAPI3REF: Setting The Subtype Of An SQL Function
** METHOD: sqlite3_context
**
** The sqlite3_result_subtype(C,T) function causes the subtype of
** the result from the [application-defined SQL function] with 
** [sqlite3_context] C to be the value T.  Only the lower 8 bits 
** of the subtype T are preserved in current versions of SQLite;
** higher order bits are discarded.
** The number of subtype bytes preserved by SQLite might increase
** in future releases of SQLite.
*/
SQLITE_API void SQLITE_STDCALL sqlite3_result_subtype(sqlite3_context*,unsigned int);

/*
** CAPI3REF: Define New Collating Sequences
** METHOD: sqlite3
**
** ^These functions add, remove, or modify a [collation] associated
** with the [database connection] specified as the first argument.
**
** ^The name of the collation is a UTF-8 string
** for sqlite3_create_collation() and sqlite3_create_collation_v2()
** and a UTF-16 string in native byte order for sqlite3_create_collation16().
** ^Collation names that compare equal according to [sqlite3_strnicmp()] are
** considered to be the same name.
**
** ^(The third argument (eTextRep) must be one of the constants:
** <ul>
** <li> [SQLITE_UTF8],
** <li> [SQLITE_UTF16LE],
** <li> [SQLITE_UTF16BE],
** <li> [SQLITE_UTF16], or
** <li> [SQLITE_UTF16_ALIGNED].
** </ul>)^
** ^The eTextRep argument determines the encoding of strings passed
** to the collating function callback, xCallback.
** ^The [SQLITE_UTF16] and [SQLITE_UTF16_ALIGNED] values for eTextRep
** force strings to be UTF16 with native byte order.
** ^The [SQLITE_UTF16_ALIGNED] value for eTextRep forces strings to begin
** on an even byte address.
**
** ^The fourth argument, pArg, is an application data pointer that is passed
** through as the first argument to the collating function callback.
**
** ^The fifth argument, xCallback, is a pointer to the collating function.
** ^Multiple collating functions can be registered using the same name but
** with different eTextRep parameters and SQLite will use whichever
** function requires the least amount of data transformation.
** ^If the xCallback argument is NULL then the collating function is
** deleted.  ^When all collating functions having the same name are deleted,
** that collation is no longer usable.
**
** ^The collating function callback is invoked with a copy of the pArg 
** application data pointer and with two strings in the encoding specified
** by the eTextRep argument.  The collating function must return an
** integer that is negative, zero, or positive
** if the first string is less than, equal to, or greater than the second,
** respectively.  A collating function must always return the same answer
** given the same inputs.  If two or more collating functions are registered
** to the same collation name (using different eTextRep values) then all
** must give an equivalent answer when invoked with equivalent strings.
** The collating function must obey the following properties for all
** strings A, B, and C:
**
** <ol>
** <li> If A==B then B==A.
** <li> If A==B and B==C then A==C.
** <li> If A&lt;B THEN B&gt;A.
** <li> If A&lt;B and B&lt;C then A&lt;C.
** </ol>
**
** If a collating function fails any of the above constraints and that
** collating function is  registered and used, then the behavior of SQLite
** is undefined.
**
** ^The sqlite3_create_collation_v2() works like sqlite3_create_collation()
** with the addition that the xDestroy callback is invoked on pArg when
** the collating function is deleted.
** ^Collating functions are deleted when they are overridden by later
** calls to the collation creation functions or when the
** [database connection] is closed using [sqlite3_close()].
**
** ^The xDestroy callback is <u>not</u> called if the 
** sqlite3_create_collation_v2() function fails.  Applications that invoke
** sqlite3_create_collation_v2() with a non-NULL xDestroy argument should 
** check the return code and dispose of the application data pointer
** themselves rather than expecting SQLite to deal with it for them.
** This is different from every other SQLite interface.  The inconsistency 
** is unfortunate but cannot be changed without breaking backwards 
** compatibility.
**
** See also:  [sqlite3_collation_needed()] and [sqlite3_collation_needed16()].
*/
SQLITE_API int SQLITE_STDCALL sqlite3_create_collation(
  sqlite3*, 
  const char *zName, 
  int eTextRep, 
  void *pArg,
  int(*xCompare)(void*,int,const void*,int,const void*)
);
SQLITE_API int SQLITE_STDCALL sqlite3_create_collation_v2(
  sqlite3*, 
  const char *zName, 
  int eTextRep, 
  void *pArg,
  int(*xCompare)(void*,int,const void*,int,const void*),
  void(*xDestroy)(void*)
);
SQLITE_API int SQLITE_STDCALL sqlite3_create_collation16(
  sqlite3*, 
  const void *zName,
  int eTextRep, 
  void *pArg,
  int(*xCompare)(void*,int,const void*,int,const void*)
);

/*
** CAPI3REF: Collation Needed Callbacks
** METHOD: sqlite3
**
** ^To avoid having to register all collation sequences before a database
** can be used, a single callback function may be registered with the
** [database connection] to be invoked whenever an undefined collation
** sequence is required.
**
** ^If the function is registered using the sqlite3_collation_needed() API,
** then it is passed the names of undefined collation sequences as strings
** encoded in UTF-8. ^If sqlite3_collation_needed16() is used,
** the names are passed as UTF-16 in machine native byte order.
** ^A call to either function replaces the existing collation-needed callback.
**
** ^(When the callback is invoked, the first argument passed is a copy
** of the second argument to sqlite3_collation_needed() or
** sqlite3_collation_needed16().  The second argument is the database
** connection.  The third argument is one of [SQLITE_UTF8], [SQLITE_UTF16BE],
** or [SQLITE_UTF16LE], indicating the most desirable form of the collation
** sequence function required.  The fourth parameter is the name of the
** required collation sequence.)^
**
** The callback function should register the desired collation using
** [sqlite3_create_collation()], [sqlite3_create_collation16()], or
** [sqlite3_create_collation_v2()].
*/
SQLITE_API int SQLITE_STDCALL sqlite3_collation_needed(
  sqlite3*, 
  void*, 
  void(*)(void*,sqlite3*,int eTextRep,const char*)
);
SQLITE_API int SQLITE_STDCALL sqlite3_collation_needed16(
  sqlite3*, 
  void*,
  void(*)(void*,sqlite3*,int eTextRep,const void*)
);

#ifdef SQLITE_HAS_CODEC
/*
** Specify the key for an encrypted database.  This routine should be
** called right after sqlite3_open().
**
** The code to implement this API is not available in the public release
** of SQLite.
*/
SQLITE_API int SQLITE_STDCALL sqlite3_key(
  sqlite3 *db,                   /* Database to be rekeyed */
  const void *pKey, int nKey     /* The key */
);
SQLITE_API int SQLITE_STDCALL sqlite3_key_v2(
  sqlite3 *db,                   /* Database to be rekeyed */
  const char *zDbName,           /* Name of the database */
  const void *pKey, int nKey     /* The key */
);

/*
** Change the key on an open database.  If the current database is not
** encrypted, this routine will encrypt it.  If pNew==0 or nNew==0, the
** database is decrypted.
**
** The code to implement this API is not available in the public release
** of SQLite.
*/
SQLITE_API int SQLITE_STDCALL sqlite3_rekey(
  sqlite3 *db,                   /* Database to be rekeyed */
  const void *pKey, int nKey     /* The new key */
);
SQLITE_API int SQLITE_STDCALL sqlite3_rekey_v2(
  sqlite3 *db,                   /* Database to be rekeyed */
  const char *zDbName,           /* Name of the database */
  const void *pKey, int nKey     /* The new key */
);

/*
** Specify the activation key for a SEE database.  Unless 
** activated, none of the SEE routines will work.
*/
SQLITE_API void SQLITE_STDCALL sqlite3_activate_see(
  const char *zPassPhrase        /* Activation phrase */
);
#endif

#ifdef SQLITE_ENABLE_CEROD
/*
** Specify the activation key for a CEROD database.  Unless 
** activated, none of the CEROD routines will work.
*/
SQLITE_API void SQLITE_STDCALL sqlite3_activate_cerod(
  const char *zPassPhrase        /* Activation phrase */
);
#endif

/*
** CAPI3REF: Suspend Execution For A Short Time
**
** The sqlite3_sleep() function causes the current thread to suspend execution
** for at least a number of milliseconds specified in its parameter.
**
** If the operating system does not support sleep requests with
** millisecond time resolution, then the time will be rounded up to
** the nearest second. The number of milliseconds of sleep actually
** requested from the operating system is returned.
**
** ^SQLite implements this interface by calling the xSleep()
** method of the default [sqlite3_vfs] object.  If the xSleep() method
** of the default VFS is not implemented correctly, or not implemented at
** all, then the behavior of sqlite3_sleep() may deviate from the description
** in the previous paragraphs.
*/
SQLITE_API int SQLITE_STDCALL sqlite3_sleep(int);

/*
** CAPI3REF: Name Of The Folder Holding Temporary Files
**
** ^(If this global variable is made to point to a string which is
** the name of a folder (a.k.a. directory), then all temporary files
** created by SQLite when using a built-in [sqlite3_vfs | VFS]
** will be placed in that directory.)^  ^If this variable
** is a NULL pointer, then SQLite performs a search for an appropriate
** temporary file directory.
**
** Applications are strongly discouraged from using this global variable.
** It is required to set a temporary folder on Windows Runtime (WinRT).
** But for all other platforms, it is highly recommended that applications
** neither read nor write this variable.  This global variable is a relic
** that exists for backwards compatibility of legacy applications and should
** be avoided in new projects.
**
** It is not safe to read or modify this variable in more than one
** thread at a time.  It is not safe to read or modify this variable
** if a [database connection] is being used at the same time in a separate
** thread.
** It is intended that this variable be set once
** as part of process initialization and before any SQLite interface
** routines have been called and that this variable remain unchanged
** thereafter.
**
** ^The [temp_store_directory pragma] may modify this variable and cause
** it to point to memory obtained from [sqlite3_malloc].  ^Furthermore,
** the [temp_store_directory pragma] always assumes that any string
** that this variable points to is held in memory obtained from 
** [sqlite3_malloc] and the pragma may attempt to free that memory
** using [sqlite3_free].
** Hence, if this variable is modified directly, either it should be
** made NULL or made to point to memory obtained from [sqlite3_malloc]
** or else the use of the [temp_store_directory pragma] should be avoided.
** Except when requested by the [temp_store_directory pragma], SQLite
** does not free the memory that sqlite3_temp_directory points to.  If
** the application wants that memory to be freed, it must do
** so itself, taking care to only do so after all [database connection]
** objects have been destroyed.
**
** <b>Note to Windows Runtime users:</b>  The temporary directory must be set
** prior to calling [sqlite3_open] or [sqlite3_open_v2].  Otherwise, various
** features that require the use of temporary files may fail.  Here is an
** example of how to do this using C++ with the Windows Runtime:
**
** <blockquote><pre>
** LPCWSTR zPath = Windows::Storage::ApplicationData::Current->
** &nbsp;     TemporaryFolder->Path->Data();
** char zPathBuf&#91;MAX_PATH + 1&#93;;
** memset(zPathBuf, 0, sizeof(zPathBuf));
** WideCharToMultiByte(CP_UTF8, 0, zPath, -1, zPathBuf, sizeof(zPathBuf),
** &nbsp;     NULL, NULL);
** sqlite3_temp_directory = sqlite3_mprintf("%s", zPathBuf);
** </pre></blockquote>
*/
SQLITE_API char *sqlite3_temp_directory;

/*
** CAPI3REF: Name Of The Folder Holding Database Files
**
** ^(If this global variable is made to point to a string which is
** the name of a folder (a.k.a. directory), then all database files
** specified with a relative pathname and created or accessed by
** SQLite when using a built-in windows [sqlite3_vfs | VFS] will be assumed
** to be relative to that directory.)^ ^If this variable is a NULL
** pointer, then SQLite assumes that all database files specified
** with a relative pathname are relative to the current directory
** for the process.  Only the windows VFS makes use of this global
** variable; it is ignored by the unix VFS.
**
** Changing the value of this variable while a database connection is
** open can result in a corrupt database.
**
** It is not safe to read or modify this variable in more than one
** thread at a time.  It is not safe to read or modify this variable
** if a [database connection] is being used at the same time in a separate
** thread.
** It is intended that this variable be set once
** as part of process initialization and before any SQLite interface
** routines have been called and that this variable remain unchanged
** thereafter.
**
** ^The [data_store_directory pragma] may modify this variable and cause
** it to point to memory obtained from [sqlite3_malloc].  ^Furthermore,
** the [data_store_directory pragma] always assumes that any string
** that this variable points to is held in memory obtained from 
** [sqlite3_malloc] and the pragma may attempt to free that memory
** using [sqlite3_free].
** Hence, if this variable is modified directly, either it should be
** made NULL or made to point to memory obtained from [sqlite3_malloc]
** or else the use of the [data_store_directory pragma] should be avoided.
*/
SQLITE_API char *sqlite3_data_directory;

/*
** CAPI3REF: Test For Auto-Commit Mode
** KEYWORDS: {autocommit mode}
** METHOD: sqlite3
**
** ^The sqlite3_get_autocommit() interface returns non-zero or
** zero if the given database connection is or is not in autocommit mode,
** respectively.  ^Autocommit mode is on by default.
** ^Autocommit mode is disabled by a [BEGIN] statement.
** ^Autocommit mode is re-enabled by a [COMMIT] or [ROLLBACK].
**
** If certain kinds of errors occur on a statement within a multi-statement
** transaction (errors including [SQLITE_FULL], [SQLITE_IOERR],
** [SQLITE_NOMEM], [SQLITE_BUSY], and [SQLITE_INTERRUPT]) then the
** transaction might be rolled back automatically.  The only way to
** find out whether SQLite automatically rolled back the transaction after
** an error is to use this function.
**
** If another thread changes the autocommit status of the database
** connection while this routine is running, then the return value
** is undefined.
*/
SQLITE_API int SQLITE_STDCALL sqlite3_get_autocommit(sqlite3*);

/*
** CAPI3REF: Find The Database Handle Of A Prepared Statement
** METHOD: sqlite3_stmt
**
** ^The sqlite3_db_handle interface returns the [database connection] handle
** to which a [prepared statement] belongs.  ^The [database connection]
** returned by sqlite3_db_handle is the same [database connection]
** that was the first argument
** to the [sqlite3_prepare_v2()] call (or its variants) that was used to
** create the statement in the first place.
*/
SQLITE_API sqlite3 *SQLITE_STDCALL sqlite3_db_handle(sqlite3_stmt*);

/*
** CAPI3REF: Return The Filename For A Database Connection
** METHOD: sqlite3
**
** ^The sqlite3_db_filename(D,N) interface returns a pointer to a filename
** associated with database N of connection D.  ^The main database file
** has the name "main".  If there is no attached database N on the database
** connection D, or if database N is a temporary or in-memory database, then
** a NULL pointer is returned.
**
** ^The filename returned by this function is the output of the
** xFullPathname method of the [VFS].  ^In other words, the filename
** will be an absolute pathname, even if the filename used
** to open the database originally was a URI or relative pathname.
*/
SQLITE_API const char *SQLITE_STDCALL sqlite3_db_filename(sqlite3 *db, const char *zDbName);

/*
** CAPI3REF: Determine if a database is read-only
** METHOD: sqlite3
**
** ^The sqlite3_db_readonly(D,N) interface returns 1 if the database N
** of connection D is read-only, 0 if it is read/write, or -1 if N is not
** the name of a database on connection D.
*/
SQLITE_API int SQLITE_STDCALL sqlite3_db_readonly(sqlite3 *db, const char *zDbName);

/*
** CAPI3REF: Find the next prepared statement
** METHOD: sqlite3
**
** ^This interface returns a pointer to the next [prepared statement] after
** pStmt associated with the [database connection] pDb.  ^If pStmt is NULL
** then this interface returns a pointer to the first prepared statement
** associated with the database connection pDb.  ^If no prepared statement
** satisfies the conditions of this routine, it returns NULL.
**
** The [database connection] pointer D in a call to
** [sqlite3_next_stmt(D,S)] must refer to an open database
** connection and in particular must not be a NULL pointer.
*/
SQLITE_API sqlite3_stmt *SQLITE_STDCALL sqlite3_next_stmt(sqlite3 *pDb, sqlite3_stmt *pStmt);

/*
** CAPI3REF: Commit And Rollback Notification Callbacks
** METHOD: sqlite3
**
** ^The sqlite3_commit_hook() interface registers a callback
** function to be invoked whenever a transaction is [COMMIT | committed].
** ^Any callback set by a previous call to sqlite3_commit_hook()
** for the same database connection is overridden.
** ^The sqlite3_rollback_hook() interface registers a callback
** function to be invoked whenever a transaction is [ROLLBACK | rolled back].
** ^Any callback set by a previous call to sqlite3_rollback_hook()
** for the same database connection is overridden.
** ^The pArg argument is passed through to the callback.
** ^If the callback on a commit hook function returns non-zero,
** then the commit is converted into a rollback.
**
** ^The sqlite3_commit_hook(D,C,P) and sqlite3_rollback_hook(D,C,P) functions
** return the P argument from the previous call of the same function
** on the same [database connection] D, or NULL for
** the first call for each function on D.
**
** The commit and rollback hook callbacks are not reentrant.
** The callback implementation must not do anything that will modify
** the database connection that invoked the callback.  Any actions
** to modify the database connection must be deferred until after the
** completion of the [sqlite3_step()] call that triggered the commit
** or rollback hook in the first place.
** Note that running any other SQL statements, including SELECT statements,
** or merely calling [sqlite3_prepare_v2()] and [sqlite3_step()] will modify
** the database connections for the meaning of "modify" in this paragraph.
**
** ^Registering a NULL function disables the callback.
**
** ^When the commit hook callback routine returns zero, the [COMMIT]
** operation is allowed to continue normally.  ^If the commit hook
** returns non-zero, then the [COMMIT] is converted into a [ROLLBACK].
** ^The rollback hook is invoked on a rollback that results from a commit
** hook returning non-zero, just as it would be with any other rollback.
**
** ^For the purposes of this API, a transaction is said to have been
** rolled back if an explicit "ROLLBACK" statement is executed, or
** an error or constraint causes an implicit rollback to occur.
** ^The rollback callback is not invoked if a transaction is
** automatically rolled back because the database connection is closed.
**
** See also the [sqlite3_update_hook()] interface.
*/
SQLITE_API void *SQLITE_STDCALL sqlite3_commit_hook(sqlite3*, int(*)(void*), void*);
SQLITE_API void *SQLITE_STDCALL sqlite3_rollback_hook(sqlite3*, void(*)(void *), void*);

/*
** CAPI3REF: Data Change Notification Callbacks
** METHOD: sqlite3
**
** ^The sqlite3_update_hook() interface registers a callback function
** with the [database connection] identified by the first argument
** to be invoked whenever a row is updated, inserted or deleted in
** a [rowid table].
** ^Any callback set by a previous call to this function
** for the same database connection is overridden.
**
** ^The second argument is a pointer to the function to invoke when a
** row is updated, inserted or deleted in a rowid table.
** ^The first argument to the callback is a copy of the third argument
** to sqlite3_update_hook().
** ^The second callback argument is one of [SQLITE_INSERT], [SQLITE_DELETE],
** or [SQLITE_UPDATE], depending on the operation that caused the callback
** to be invoked.
** ^The third and fourth arguments to the callback contain pointers to the
** database and table name containing the affected row.
** ^The final callback parameter is the [rowid] of the row.
** ^In the case of an update, this is the [rowid] after the update takes place.
**
** ^(The update hook is not invoked when internal system tables are
** modified (i.e. sqlite_master and sqlite_sequence).)^
** ^The update hook is not invoked when [WITHOUT ROWID] tables are modified.
**
** ^In the current implementation, the update hook
** is not invoked when duplication rows are deleted because of an
** [ON CONFLICT | ON CONFLICT REPLACE] clause.  ^Nor is the update hook
** invoked when rows are deleted using the [truncate optimization].
** The exceptions defined in this paragraph might change in a future
** release of SQLite.
**
** The update hook implementation must not do anything that will modify
** the database connection that invoked the update hook.  Any actions
** to modify the database connection must be deferred until after the
** completion of the [sqlite3_step()] call that triggered the update hook.
** Note that [sqlite3_prepare_v2()] and [sqlite3_step()] both modify their
** database connections for the meaning of "modify" in this paragraph.
**
** ^The sqlite3_update_hook(D,C,P) function
** returns the P argument from the previous call
** on the same [database connection] D, or NULL for
** the first call on D.
**
** See also the [sqlite3_commit_hook()], [sqlite3_rollback_hook()],
** and [sqlite3_preupdate_hook()] interfaces.
*/
SQLITE_API void *SQLITE_STDCALL sqlite3_update_hook(
  sqlite3*, 
  void(*)(void *,int ,char const *,char const *,sqlite3_int64),
  void*
);

/*
** CAPI3REF: Enable Or Disable Shared Pager Cache
**
** ^(This routine enables or disables the sharing of the database cache
** and schema data structures between [database connection | connections]
** to the same database. Sharing is enabled if the argument is true
** and disabled if the argument is false.)^
**
** ^Cache sharing is enabled and disabled for an entire process.
** This is a change as of SQLite version 3.5.0. In prior versions of SQLite,
** sharing was enabled or disabled for each thread separately.
**
** ^(The cache sharing mode set by this interface effects all subsequent
** calls to [sqlite3_open()], [sqlite3_open_v2()], and [sqlite3_open16()].
** Existing database connections continue use the sharing mode
** that was in effect at the time they were opened.)^
**
** ^(This routine returns [SQLITE_OK] if shared cache was enabled or disabled
** successfully.  An [error code] is returned otherwise.)^
**
** ^Shared cache is disabled by default. But this might change in
** future releases of SQLite.  Applications that care about shared
** cache setting should set it explicitly.
**
** Note: This method is disabled on MacOS X 10.7 and iOS version 5.0
** and will always return SQLITE_MISUSE. On those systems, 
** shared cache mode should be enabled per-database connection via 
** [sqlite3_open_v2()] with [SQLITE_OPEN_SHAREDCACHE].
**
** This interface is threadsafe on processors where writing a
** 32-bit integer is atomic.
**
** See Also:  [SQLite Shared-Cache Mode]
*/
SQLITE_API int SQLITE_STDCALL sqlite3_enable_shared_cache(int);

/*
** CAPI3REF: Attempt To Free Heap Memory
**
** ^The sqlite3_release_memory() interface attempts to free N bytes
** of heap memory by deallocating non-essential memory allocations
** held by the database library.   Memory used to cache database
** pages to improve performance is an example of non-essential memory.
** ^sqlite3_release_memory() returns the number of bytes actually freed,
** which might be more or less than the amount requested.
** ^The sqlite3_release_memory() routine is a no-op returning zero
** if SQLite is not compiled with [SQLITE_ENABLE_MEMORY_MANAGEMENT].
**
** See also: [sqlite3_db_release_memory()]
*/
SQLITE_API int SQLITE_STDCALL sqlite3_release_memory(int);

/*
** CAPI3REF: Free Memory Used By A Database Connection
** METHOD: sqlite3
**
** ^The sqlite3_db_release_memory(D) interface attempts to free as much heap
** memory as possible from database connection D. Unlike the
** [sqlite3_release_memory()] interface, this interface is in effect even
** when the [SQLITE_ENABLE_MEMORY_MANAGEMENT] compile-time option is
** omitted.
**
** See also: [sqlite3_release_memory()]
*/
SQLITE_API int SQLITE_STDCALL sqlite3_db_release_memory(sqlite3*);

/*
** CAPI3REF: Impose A Limit On Heap Size
**
** ^The sqlite3_soft_heap_limit64() interface sets and/or queries the
** soft limit on the amount of heap memory that may be allocated by SQLite.
** ^SQLite strives to keep heap memory utilization below the soft heap
** limit by reducing the number of pages held in the page cache
** as heap memory usages approaches the limit.
** ^The soft heap limit is "soft" because even though SQLite strives to stay
** below the limit, it will exceed the limit rather than generate
** an [SQLITE_NOMEM] error.  In other words, the soft heap limit 
** is advisory only.
**
** ^The return value from sqlite3_soft_heap_limit64() is the size of
** the soft heap limit prior to the call, or negative in the case of an
** error.  ^If the argument N is negative
** then no change is made to the soft heap limit.  Hence, the current
** size of the soft heap limit can be determined by invoking
** sqlite3_soft_heap_limit64() with a negative argument.
**
** ^If the argument N is zero then the soft heap limit is disabled.
**
** ^(The soft heap limit is not enforced in the current implementation
** if one or more of following conditions are true:
**
** <ul>
** <li> The soft heap limit is set to zero.
** <li> Memory accounting is disabled using a combination of the
**      [sqlite3_config]([SQLITE_CONFIG_MEMSTATUS],...) start-time option and
**      the [SQLITE_DEFAULT_MEMSTATUS] compile-time option.
** <li> An alternative page cache implementation is specified using
**      [sqlite3_config]([SQLITE_CONFIG_PCACHE2],...).
** <li> The page cache allocates from its own memory pool supplied
**      by [sqlite3_config]([SQLITE_CONFIG_PAGECACHE],...) rather than
**      from the heap.
** </ul>)^
**
** Beginning with SQLite version 3.7.3, the soft heap limit is enforced
** regardless of whether or not the [SQLITE_ENABLE_MEMORY_MANAGEMENT]
** compile-time option is invoked.  With [SQLITE_ENABLE_MEMORY_MANAGEMENT],
** the soft heap limit is enforced on every memory allocation.  Without
** [SQLITE_ENABLE_MEMORY_MANAGEMENT], the soft heap limit is only enforced
** when memory is allocated by the page cache.  Testing suggests that because
** the page cache is the predominate memory user in SQLite, most
** applications will achieve adequate soft heap limit enforcement without
** the use of [SQLITE_ENABLE_MEMORY_MANAGEMENT].
**
** The circumstances under which SQLite will enforce the soft heap limit may
** changes in future releases of SQLite.
*/
SQLITE_API sqlite3_int64 SQLITE_STDCALL sqlite3_soft_heap_limit64(sqlite3_int64 N);

/*
** CAPI3REF: Deprecated Soft Heap Limit Interface
** DEPRECATED
**
** This is a deprecated version of the [sqlite3_soft_heap_limit64()]
** interface.  This routine is provided for historical compatibility
** only.  All new applications should use the
** [sqlite3_soft_heap_limit64()] interface rather than this one.
*/
SQLITE_API SQLITE_DEPRECATED void SQLITE_STDCALL sqlite3_soft_heap_limit(int N);


/*
** CAPI3REF: Extract Metadata About A Column Of A Table
** METHOD: sqlite3
**
** ^(The sqlite3_table_column_metadata(X,D,T,C,....) routine returns
** information about column C of table T in database D
** on [database connection] X.)^  ^The sqlite3_table_column_metadata()
** interface returns SQLITE_OK and fills in the non-NULL pointers in
** the final five arguments with appropriate values if the specified
** column exists.  ^The sqlite3_table_column_metadata() interface returns
** SQLITE_ERROR and if the specified column does not exist.
** ^If the column-name parameter to sqlite3_table_column_metadata() is a
** NULL pointer, then this routine simply checks for the existance of the
** table and returns SQLITE_OK if the table exists and SQLITE_ERROR if it
** does not.
**
** ^The column is identified by the second, third and fourth parameters to
** this function. ^(The second parameter is either the name of the database
** (i.e. "main", "temp", or an attached database) containing the specified
** table or NULL.)^ ^If it is NULL, then all attached databases are searched
** for the table using the same algorithm used by the database engine to
** resolve unqualified table references.
**
** ^The third and fourth parameters to this function are the table and column
** name of the desired column, respectively.
**
** ^Metadata is returned by writing to the memory locations passed as the 5th
** and subsequent parameters to this function. ^Any of these arguments may be
** NULL, in which case the corresponding element of metadata is omitted.
**
** ^(<blockquote>
** <table border="1">
** <tr><th> Parameter <th> Output<br>Type <th>  Description
**
** <tr><td> 5th <td> const char* <td> Data type
** <tr><td> 6th <td> const char* <td> Name of default collation sequence
** <tr><td> 7th <td> int         <td> True if column has a NOT NULL constraint
** <tr><td> 8th <td> int         <td> True if column is part of the PRIMARY KEY
** <tr><td> 9th <td> int         <td> True if column is [AUTOINCREMENT]
** </table>
** </blockquote>)^
**
** ^The memory pointed to by the character pointers returned for the
** declaration type and collation sequence is valid until the next
** call to any SQLite API function.
**
** ^If the specified table is actually a view, an [error code] is returned.
**
** ^If the specified column is "rowid", "oid" or "_rowid_" and the table 
** is not a [WITHOUT ROWID] table and an
** [INTEGER PRIMARY KEY] column has been explicitly declared, then the output
** parameters are set for the explicitly declared column. ^(If there is no
** [INTEGER PRIMARY KEY] column, then the outputs
** for the [rowid] are set as follows:
**
** <pre>
**     data type: "INTEGER"
**     collation sequence: "BINARY"
**     not null: 0
**     primary key: 1
**     auto increment: 0
** </pre>)^
**
** ^This function causes all database schemas to be read from disk and
** parsed, if that has not already been done, and returns an error if
** any errors are encountered while loading the schema.
*/
SQLITE_API int SQLITE_STDCALL sqlite3_table_column_metadata(
  sqlite3 *db,                /* Connection handle */
  const char *zDbName,        /* Database name or NULL */
  const char *zTableName,     /* Table name */
  const char *zColumnName,    /* Column name */
  char const **pzDataType,    /* OUTPUT: Declared data type */
  char const **pzCollSeq,     /* OUTPUT: Collation sequence name */
  int *pNotNull,              /* OUTPUT: True if NOT NULL constraint exists */
  int *pPrimaryKey,           /* OUTPUT: True if column part of PK */
  int *pAutoinc               /* OUTPUT: True if column is auto-increment */
);

/*
** CAPI3REF: Load An Extension
** METHOD: sqlite3
**
** ^This interface loads an SQLite extension library from the named file.
**
** ^The sqlite3_load_extension() interface attempts to load an
** [SQLite extension] library contained in the file zFile.  If
** the file cannot be loaded directly, attempts are made to load
** with various operating-system specific extensions added.
** So for example, if "samplelib" cannot be loaded, then names like
** "samplelib.so" or "samplelib.dylib" or "samplelib.dll" might
** be tried also.
**
** ^The entry point is zProc.
** ^(zProc may be 0, in which case SQLite will try to come up with an
** entry point name on its own.  It first tries "sqlite3_extension_init".
** If that does not work, it constructs a name "sqlite3_X_init" where the
** X is consists of the lower-case equivalent of all ASCII alphabetic
** characters in the filename from the last "/" to the first following
** "." and omitting any initial "lib".)^
** ^The sqlite3_load_extension() interface returns
** [SQLITE_OK] on success and [SQLITE_ERROR] if something goes wrong.
** ^If an error occurs and pzErrMsg is not 0, then the
** [sqlite3_load_extension()] interface shall attempt to
** fill *pzErrMsg with error message text stored in memory
** obtained from [sqlite3_malloc()]. The calling function
** should free this memory by calling [sqlite3_free()].
**
** ^Extension loading must be enabled using
** [sqlite3_enable_load_extension()] or
** [sqlite3_db_config](db,[SQLITE_DBCONFIG_ENABLE_LOAD_EXTENSION],1,NULL)
** prior to calling this API,
** otherwise an error will be returned.
**
** <b>Security warning:</b> It is recommended that the 
** [SQLITE_DBCONFIG_ENABLE_LOAD_EXTENSION] method be used to enable only this
** interface.  The use of the [sqlite3_enable_load_extension()] interface
** should be avoided.  This will keep the SQL function [load_extension()]
** disabled and prevent SQL injections from giving attackers
** access to extension loading capabilities.
**
** See also the [load_extension() SQL function].
*/
SQLITE_API int SQLITE_STDCALL sqlite3_load_extension(
  sqlite3 *db,          /* Load the extension into this database connection */
  const char *zFile,    /* Name of the shared library containing extension */
  const char *zProc,    /* Entry point.  Derived from zFile if 0 */
  char **pzErrMsg       /* Put error message here if not 0 */
);

/*
** CAPI3REF: Enable Or Disable Extension Loading
** METHOD: sqlite3
**
** ^So as not to open security holes in older applications that are
** unprepared to deal with [extension loading], and as a means of disabling
** [extension loading] while evaluating user-entered SQL, the following API
** is provided to turn the [sqlite3_load_extension()] mechanism on and off.
**
** ^Extension loading is off by default.
** ^Call the sqlite3_enable_load_extension() routine with onoff==1
** to turn extension loading on and call it with onoff==0 to turn
** it back off again.
**
** ^This interface enables or disables both the C-API
** [sqlite3_load_extension()] and the SQL function [load_extension()].
** Use [sqlite3_db_config](db,[SQLITE_DBCONFIG_ENABLE_LOAD_EXTENSION],..)
** to enable or disable only the C-API.
**
** <b>Security warning:</b> It is recommended that extension loading
** be disabled using the [SQLITE_DBCONFIG_ENABLE_LOAD_EXTENSION] method
** rather than this interface, so the [load_extension()] SQL function
** remains disabled. This will prevent SQL injections from giving attackers
** access to extension loading capabilities.
*/
SQLITE_API int SQLITE_STDCALL sqlite3_enable_load_extension(sqlite3 *db, int onoff);

/*
** CAPI3REF: Automatically Load Statically Linked Extensions
**
** ^This interface causes the xEntryPoint() function to be invoked for
** each new [database connection] that is created.  The idea here is that
** xEntryPoint() is the entry point for a statically linked [SQLite extension]
** that is to be automatically loaded into all new database connections.
**
** ^(Even though the function prototype shows that xEntryPoint() takes
** no arguments and returns void, SQLite invokes xEntryPoint() with three
** arguments and expects and integer result as if the signature of the
** entry point where as follows:
**
** <blockquote><pre>
** &nbsp;  int xEntryPoint(
** &nbsp;    sqlite3 *db,
** &nbsp;    const char **pzErrMsg,
** &nbsp;    const struct sqlite3_api_routines *pThunk
** &nbsp;  );
** </pre></blockquote>)^
**
** If the xEntryPoint routine encounters an error, it should make *pzErrMsg
** point to an appropriate error message (obtained from [sqlite3_mprintf()])
** and return an appropriate [error code].  ^SQLite ensures that *pzErrMsg
** is NULL before calling the xEntryPoint().  ^SQLite will invoke
** [sqlite3_free()] on *pzErrMsg after xEntryPoint() returns.  ^If any
** xEntryPoint() returns an error, the [sqlite3_open()], [sqlite3_open16()],
** or [sqlite3_open_v2()] call that provoked the xEntryPoint() will fail.
**
** ^Calling sqlite3_auto_extension(X) with an entry point X that is already
** on the list of automatic extensions is a harmless no-op. ^No entry point
** will be called more than once for each database connection that is opened.
**
** See also: [sqlite3_reset_auto_extension()]
** and [sqlite3_cancel_auto_extension()]
*/
SQLITE_API int SQLITE_STDCALL sqlite3_auto_extension(void (*xEntryPoint)(void));

/*
** CAPI3REF: Cancel Automatic Extension Loading
**
** ^The [sqlite3_cancel_auto_extension(X)] interface unregisters the
** initialization routine X that was registered using a prior call to
** [sqlite3_auto_extension(X)].  ^The [sqlite3_cancel_auto_extension(X)]
** routine returns 1 if initialization routine X was successfully 
** unregistered and it returns 0 if X was not on the list of initialization
** routines.
*/
SQLITE_API int SQLITE_STDCALL sqlite3_cancel_auto_extension(void (*xEntryPoint)(void));

/*
** CAPI3REF: Reset Automatic Extension Loading
**
** ^This interface disables all automatic extensions previously
** registered using [sqlite3_auto_extension()].
*/
SQLITE_API void SQLITE_STDCALL sqlite3_reset_auto_extension(void);

/*
** The interface to the virtual-table mechanism is currently considered
** to be experimental.  The interface might change in incompatible ways.
** If this is a problem for you, do not use the interface at this time.
**
** When the virtual-table mechanism stabilizes, we will declare the
** interface fixed, support it indefinitely, and remove this comment.
*/

/*
** Structures used by the virtual table interface
*/
typedef struct sqlite3_vtab sqlite3_vtab;
typedef struct sqlite3_index_info sqlite3_index_info;
typedef struct sqlite3_vtab_cursor sqlite3_vtab_cursor;
typedef struct sqlite3_module sqlite3_module;

/*
** CAPI3REF: Virtual Table Object
** KEYWORDS: sqlite3_module {virtual table module}
**
** This structure, sometimes called a "virtual table module", 
** defines the implementation of a [virtual tables].  
** This structure consists mostly of methods for the module.
**
** ^A virtual table module is created by filling in a persistent
** instance of this structure and passing a pointer to that instance
** to [sqlite3_create_module()] or [sqlite3_create_module_v2()].
** ^The registration remains valid until it is replaced by a different
** module or until the [database connection] closes.  The content
** of this structure must not change while it is registered with
** any database connection.
*/
struct sqlite3_module {
  int iVersion;
  int (*xCreate)(sqlite3*, void *pAux,
               int argc, const char *const*argv,
               sqlite3_vtab **ppVTab, char**);
  int (*xConnect)(sqlite3*, void *pAux,
               int argc, const char *const*argv,
               sqlite3_vtab **ppVTab, char**);
  int (*xBestIndex)(sqlite3_vtab *pVTab, sqlite3_index_info*);
  int (*xDisconnect)(sqlite3_vtab *pVTab);
  int (*xDestroy)(sqlite3_vtab *pVTab);
  int (*xOpen)(sqlite3_vtab *pVTab, sqlite3_vtab_cursor **ppCursor);
  int (*xClose)(sqlite3_vtab_cursor*);
  int (*xFilter)(sqlite3_vtab_cursor*, int idxNum, const char *idxStr,
                int argc, sqlite3_value **argv);
  int (*xNext)(sqlite3_vtab_cursor*);
  int (*xEof)(sqlite3_vtab_cursor*);
  int (*xColumn)(sqlite3_vtab_cursor*, sqlite3_context*, int);
  int (*xRowid)(sqlite3_vtab_cursor*, sqlite3_int64 *pRowid);
  int (*xUpdate)(sqlite3_vtab *, int, sqlite3_value **, sqlite3_int64 *);
  int (*xBegin)(sqlite3_vtab *pVTab);
  int (*xSync)(sqlite3_vtab *pVTab);
  int (*xCommit)(sqlite3_vtab *pVTab);
  int (*xRollback)(sqlite3_vtab *pVTab);
  int (*xFindFunction)(sqlite3_vtab *pVtab, int nArg, const char *zName,
                       void (**pxFunc)(sqlite3_context*,int,sqlite3_value**),
                       void **ppArg);
  int (*xRename)(sqlite3_vtab *pVtab, const char *zNew);
  /* The methods above are in version 1 of the sqlite_module object. Those 
  ** below are for version 2 and greater. */
  int (*xSavepoint)(sqlite3_vtab *pVTab, int);
  int (*xRelease)(sqlite3_vtab *pVTab, int);
  int (*xRollbackTo)(sqlite3_vtab *pVTab, int);
};

/*
** CAPI3REF: Virtual Table Indexing Information
** KEYWORDS: sqlite3_index_info
**
** The sqlite3_index_info structure and its substructures is used as part
** of the [virtual table] interface to
** pass information into and receive the reply from the [xBestIndex]
** method of a [virtual table module].  The fields under **Inputs** are the
** inputs to xBestIndex and are read-only.  xBestIndex inserts its
** results into the **Outputs** fields.
**
** ^(The aConstraint[] array records WHERE clause constraints of the form:
**
** <blockquote>column OP expr</blockquote>
**
** where OP is =, &lt;, &lt;=, &gt;, or &gt;=.)^  ^(The particular operator is
** stored in aConstraint[].op using one of the
** [SQLITE_INDEX_CONSTRAINT_EQ | SQLITE_INDEX_CONSTRAINT_ values].)^
** ^(The index of the column is stored in
** aConstraint[].iColumn.)^  ^(aConstraint[].usable is TRUE if the
** expr on the right-hand side can be evaluated (and thus the constraint
** is usable) and false if it cannot.)^
**
** ^The optimizer automatically inverts terms of the form "expr OP column"
** and makes other simplifications to the WHERE clause in an attempt to
** get as many WHERE clause terms into the form shown above as possible.
** ^The aConstraint[] array only reports WHERE clause terms that are
** relevant to the particular virtual table being queried.
**
** ^Information about the ORDER BY clause is stored in aOrderBy[].
** ^Each term of aOrderBy records a column of the ORDER BY clause.
**
** The colUsed field indicates which columns of the virtual table may be
** required by the current scan. Virtual table columns are numbered from
** zero in the order in which they appear within the CREATE TABLE statement
** passed to sqlite3_declare_vtab(). For the first 63 columns (columns 0-62),
** the corresponding bit is set within the colUsed mask if the column may be
** required by SQLite. If the table has at least 64 columns and any column
** to the right of the first 63 is required, then bit 63 of colUsed is also
** set. In other words, column iCol may be required if the expression
** (colUsed & ((sqlite3_uint64)1 << (iCol>=63 ? 63 : iCol))) evaluates to 
** non-zero.
**
** The [xBestIndex] method must fill aConstraintUsage[] with information
** about what parameters to pass to xFilter.  ^If argvIndex>0 then
** the right-hand side of the corresponding aConstraint[] is evaluated
** and becomes the argvIndex-th entry in argv.  ^(If aConstraintUsage[].omit
** is true, then the constraint is assumed to be fully handled by the
** virtual table and is not checked again by SQLite.)^
**
** ^The idxNum and idxPtr values are recorded and passed into the
** [xFilter] method.
** ^[sqlite3_free()] is used to free idxPtr if and only if
** needToFreeIdxPtr is true.
**
** ^The orderByConsumed means that output from [xFilter]/[xNext] will occur in
** the correct order to satisfy the ORDER BY clause so that no separate
** sorting step is required.
**
** ^The estimatedCost value is an estimate of the cost of a particular
** strategy. A cost of N indicates that the cost of the strategy is similar
** to a linear scan of an SQLite table with N rows. A cost of log(N) 
** indicates that the expense of the operation is similar to that of a
** binary search on a unique indexed field of an SQLite table with N rows.
**
** ^The estimatedRows value is an estimate of the number of rows that
** will be returned by the strategy.
**
** The xBestIndex method may optionally populate the idxFlags field with a 
** mask of SQLITE_INDEX_SCAN_* flags. Currently there is only one such flag -
** SQLITE_INDEX_SCAN_UNIQUE. If the xBestIndex method sets this flag, SQLite
** assumes that the strategy may visit at most one row. 
**
** Additionally, if xBestIndex sets the SQLITE_INDEX_SCAN_UNIQUE flag, then
** SQLite also assumes that if a call to the xUpdate() method is made as
** part of the same statement to delete or update a virtual table row and the
** implementation returns SQLITE_CONSTRAINT, then there is no need to rollback
** any database changes. In other words, if the xUpdate() returns
** SQLITE_CONSTRAINT, the database contents must be exactly as they were
** before xUpdate was called. By contrast, if SQLITE_INDEX_SCAN_UNIQUE is not
** set and xUpdate returns SQLITE_CONSTRAINT, any database changes made by
** the xUpdate method are automatically rolled back by SQLite.
**
** IMPORTANT: The estimatedRows field was added to the sqlite3_index_info
** structure for SQLite version 3.8.2. If a virtual table extension is
** used with an SQLite version earlier than 3.8.2, the results of attempting 
** to read or write the estimatedRows field are undefined (but are likely 
** to included crashing the application). The estimatedRows field should
** therefore only be used if [sqlite3_libversion_number()] returns a
** value greater than or equal to 3008002. Similarly, the idxFlags field
** was added for version 3.9.0. It may therefore only be used if
** sqlite3_libversion_number() returns a value greater than or equal to
** 3009000.
*/
struct sqlite3_index_info {
  /* Inputs */
  int nConstraint;           /* Number of entries in aConstraint */
  struct sqlite3_index_constraint {
     int iColumn;              /* Column constrained.  -1 for ROWID */
     unsigned char op;         /* Constraint operator */
     unsigned char usable;     /* True if this constraint is usable */
     int iTermOffset;          /* Used internally - xBestIndex should ignore */
  } *aConstraint;            /* Table of WHERE clause constraints */
  int nOrderBy;              /* Number of terms in the ORDER BY clause */
  struct sqlite3_index_orderby {
     int iColumn;              /* Column number */
     unsigned char desc;       /* True for DESC.  False for ASC. */
  } *aOrderBy;               /* The ORDER BY clause */
  /* Outputs */
  struct sqlite3_index_constraint_usage {
    int argvIndex;           /* if >0, constraint is part of argv to xFilter */
    unsigned char omit;      /* Do not code a test for this constraint */
  } *aConstraintUsage;
  int idxNum;                /* Number used to identify the index */
  char *idxStr;              /* String, possibly obtained from sqlite3_malloc */
  int needToFreeIdxStr;      /* Free idxStr using sqlite3_free() if true */
  int orderByConsumed;       /* True if output is already ordered */
  double estimatedCost;           /* Estimated cost of using this index */
  /* Fields below are only available in SQLite 3.8.2 and later */
  sqlite3_int64 estimatedRows;    /* Estimated number of rows returned */
  /* Fields below are only available in SQLite 3.9.0 and later */
  int idxFlags;              /* Mask of SQLITE_INDEX_SCAN_* flags */
  /* Fields below are only available in SQLite 3.10.0 and later */
  sqlite3_uint64 colUsed;    /* Input: Mask of columns used by statement */
};

/*
** CAPI3REF: Virtual Table Scan Flags
*/
#define SQLITE_INDEX_SCAN_UNIQUE      1     /* Scan visits at most 1 row */

/*
** CAPI3REF: Virtual Table Constraint Operator Codes
**
** These macros defined the allowed values for the
** [sqlite3_index_info].aConstraint[].op field.  Each value represents
** an operator that is part of a constraint term in the wHERE clause of
** a query that uses a [virtual table].
*/
#define SQLITE_INDEX_CONSTRAINT_EQ      2
#define SQLITE_INDEX_CONSTRAINT_GT      4
#define SQLITE_INDEX_CONSTRAINT_LE      8
#define SQLITE_INDEX_CONSTRAINT_LT     16
#define SQLITE_INDEX_CONSTRAINT_GE     32
#define SQLITE_INDEX_CONSTRAINT_MATCH  64
#define SQLITE_INDEX_CONSTRAINT_LIKE   65
#define SQLITE_INDEX_CONSTRAINT_GLOB   66
#define SQLITE_INDEX_CONSTRAINT_REGEXP 67

/*
** CAPI3REF: Register A Virtual Table Implementation
** METHOD: sqlite3
**
** ^These routines are used to register a new [virtual table module] name.
** ^Module names must be registered before
** creating a new [virtual table] using the module and before using a
** preexisting [virtual table] for the module.
**
** ^The module name is registered on the [database connection] specified
** by the first parameter.  ^The name of the module is given by the 
** second parameter.  ^The third parameter is a pointer to
** the implementation of the [virtual table module].   ^The fourth
** parameter is an arbitrary client data pointer that is passed through
** into the [xCreate] and [xConnect] methods of the virtual table module
** when a new virtual table is be being created or reinitialized.
**
** ^The sqlite3_create_module_v2() interface has a fifth parameter which
** is a pointer to a destructor for the pClientData.  ^SQLite will
** invoke the destructor function (if it is not NULL) when SQLite
** no longer needs the pClientData pointer.  ^The destructor will also
** be invoked if the call to sqlite3_create_module_v2() fails.
** ^The sqlite3_create_module()
** interface is equivalent to sqlite3_create_module_v2() with a NULL
** destructor.
*/
SQLITE_API int SQLITE_STDCALL sqlite3_create_module(
  sqlite3 *db,               /* SQLite connection to register module with */
  const char *zName,         /* Name of the module */
  const sqlite3_module *p,   /* Methods for the module */
  void *pClientData          /* Client data for xCreate/xConnect */
);
SQLITE_API int SQLITE_STDCALL sqlite3_create_module_v2(
  sqlite3 *db,               /* SQLite connection to register module with */
  const char *zName,         /* Name of the module */
  const sqlite3_module *p,   /* Methods for the module */
  void *pClientData,         /* Client data for xCreate/xConnect */
  void(*xDestroy)(void*)     /* Module destructor function */
);

/*
** CAPI3REF: Virtual Table Instance Object
** KEYWORDS: sqlite3_vtab
**
** Every [virtual table module] implementation uses a subclass
** of this object to describe a particular instance
** of the [virtual table].  Each subclass will
** be tailored to the specific needs of the module implementation.
** The purpose of this superclass is to define certain fields that are
** common to all module implementations.
**
** ^Virtual tables methods can set an error message by assigning a
** string obtained from [sqlite3_mprintf()] to zErrMsg.  The method should
** take care that any prior string is freed by a call to [sqlite3_free()]
** prior to assigning a new string to zErrMsg.  ^After the error message
** is delivered up to the client application, the string will be automatically
** freed by sqlite3_free() and the zErrMsg field will be zeroed.
*/
struct sqlite3_vtab {
  const sqlite3_module *pModule;  /* The module for this virtual table */
  int nRef;                       /* Number of open cursors */
  char *zErrMsg;                  /* Error message from sqlite3_mprintf() */
  /* Virtual table implementations will typically add additional fields */
};

/*
** CAPI3REF: Virtual Table Cursor Object
** KEYWORDS: sqlite3_vtab_cursor {virtual table cursor}
**
** Every [virtual table module] implementation uses a subclass of the
** following structure to describe cursors that point into the
** [virtual table] and are used
** to loop through the virtual table.  Cursors are created using the
** [sqlite3_module.xOpen | xOpen] method of the module and are destroyed
** by the [sqlite3_module.xClose | xClose] method.  Cursors are used
** by the [xFilter], [xNext], [xEof], [xColumn], and [xRowid] methods
** of the module.  Each module implementation will define
** the content of a cursor structure to suit its own needs.
**
** This superclass exists in order to define fields of the cursor that
** are common to all implementations.
*/
struct sqlite3_vtab_cursor {
  sqlite3_vtab *pVtab;      /* Virtual table of this cursor */
  /* Virtual table implementations will typically add additional fields */
};

/*
** CAPI3REF: Declare The Schema Of A Virtual Table
**
** ^The [xCreate] and [xConnect] methods of a
** [virtual table module] call this interface
** to declare the format (the names and datatypes of the columns) of
** the virtual tables they implement.
*/
SQLITE_API int SQLITE_STDCALL sqlite3_declare_vtab(sqlite3*, const char *zSQL);

/*
** CAPI3REF: Overload A Function For A Virtual Table
** METHOD: sqlite3
**
** ^(Virtual tables can provide alternative implementations of functions
** using the [xFindFunction] method of the [virtual table module].  
** But global versions of those functions
** must exist in order to be overloaded.)^
**
** ^(This API makes sure a global version of a function with a particular
** name and number of parameters exists.  If no such function exists
** before this API is called, a new function is created.)^  ^The implementation
** of the new function always causes an exception to be thrown.  So
** the new function is not good for anything by itself.  Its only
** purpose is to be a placeholder function that can be overloaded
** by a [virtual table].
*/
SQLITE_API int SQLITE_STDCALL sqlite3_overload_function(sqlite3*, const char *zFuncName, int nArg);

/*
** The interface to the virtual-table mechanism defined above (back up
** to a comment remarkably similar to this one) is currently considered
** to be experimental.  The interface might change in incompatible ways.
** If this is a problem for you, do not use the interface at this time.
**
** When the virtual-table mechanism stabilizes, we will declare the
** interface fixed, support it indefinitely, and remove this comment.
*/

/*
** CAPI3REF: A Handle To An Open BLOB
** KEYWORDS: {BLOB handle} {BLOB handles}
**
** An instance of this object represents an open BLOB on which
** [sqlite3_blob_open | incremental BLOB I/O] can be performed.
** ^Objects of this type are created by [sqlite3_blob_open()]
** and destroyed by [sqlite3_blob_close()].
** ^The [sqlite3_blob_read()] and [sqlite3_blob_write()] interfaces
** can be used to read or write small subsections of the BLOB.
** ^The [sqlite3_blob_bytes()] interface returns the size of the BLOB in bytes.
*/
typedef struct sqlite3_blob sqlite3_blob;

/*
** CAPI3REF: Open A BLOB For Incremental I/O
** METHOD: sqlite3
** CONSTRUCTOR: sqlite3_blob
**
** ^(This interfaces opens a [BLOB handle | handle] to the BLOB located
** in row iRow, column zColumn, table zTable in database zDb;
** in other words, the same BLOB that would be selected by:
**
** <pre>
**     SELECT zColumn FROM zDb.zTable WHERE [rowid] = iRow;
** </pre>)^
**
** ^(Parameter zDb is not the filename that contains the database, but 
** rather the symbolic name of the database. For attached databases, this is
** the name that appears after the AS keyword in the [ATTACH] statement.
** For the main database file, the database name is "main". For TEMP
** tables, the database name is "temp".)^
**
** ^If the flags parameter is non-zero, then the BLOB is opened for read
** and write access. ^If the flags parameter is zero, the BLOB is opened for
** read-only access.
**
** ^(On success, [SQLITE_OK] is returned and the new [BLOB handle] is stored
** in *ppBlob. Otherwise an [error code] is returned and, unless the error
** code is SQLITE_MISUSE, *ppBlob is set to NULL.)^ ^This means that, provided
** the API is not misused, it is always safe to call [sqlite3_blob_close()] 
** on *ppBlob after this function it returns.
**
** This function fails with SQLITE_ERROR if any of the following are true:
** <ul>
**   <li> ^(Database zDb does not exist)^, 
**   <li> ^(Table zTable does not exist within database zDb)^, 
**   <li> ^(Table zTable is a WITHOUT ROWID table)^, 
**   <li> ^(Column zColumn does not exist)^,
**   <li> ^(Row iRow is not present in the table)^,
**   <li> ^(The specified column of row iRow contains a value that is not
**         a TEXT or BLOB value)^,
**   <li> ^(Column zColumn is part of an index, PRIMARY KEY or UNIQUE 
**         constraint and the blob is being opened for read/write access)^,
**   <li> ^([foreign key constraints | Foreign key constraints] are enabled, 
**         column zColumn is part of a [child key] definition and the blob is
**         being opened for read/write access)^.
** </ul>
**
** ^Unless it returns SQLITE_MISUSE, this function sets the 
** [database connection] error code and message accessible via 
** [sqlite3_errcode()] and [sqlite3_errmsg()] and related functions. 
**
**
** ^(If the row that a BLOB handle points to is modified by an
** [UPDATE], [DELETE], or by [ON CONFLICT] side-effects
** then the BLOB handle is marked as "expired".
** This is true if any column of the row is changed, even a column
** other than the one the BLOB handle is open on.)^
** ^Calls to [sqlite3_blob_read()] and [sqlite3_blob_write()] for
** an expired BLOB handle fail with a return code of [SQLITE_ABORT].
** ^(Changes written into a BLOB prior to the BLOB expiring are not
** rolled back by the expiration of the BLOB.  Such changes will eventually
** commit if the transaction continues to completion.)^
**
** ^Use the [sqlite3_blob_bytes()] interface to determine the size of
** the opened blob.  ^The size of a blob may not be changed by this
** interface.  Use the [UPDATE] SQL command to change the size of a
** blob.
**
** ^The [sqlite3_bind_zeroblob()] and [sqlite3_result_zeroblob()] interfaces
** and the built-in [zeroblob] SQL function may be used to create a 
** zero-filled blob to read or write using the incremental-blob interface.
**
** To avoid a resource leak, every open [BLOB handle] should eventually
** be released by a call to [sqlite3_blob_close()].
*/
SQLITE_API int SQLITE_STDCALL sqlite3_blob_open(
  sqlite3*,
  const char *zDb,
  const char *zTable,
  const char *zColumn,
  sqlite3_int64 iRow,
  int flags,
  sqlite3_blob **ppBlob
);

/*
** CAPI3REF: Move a BLOB Handle to a New Row
** METHOD: sqlite3_blob
**
** ^This function is used to move an existing blob handle so that it points
** to a different row of the same database table. ^The new row is identified
** by the rowid value passed as the second argument. Only the row can be
** changed. ^The database, table and column on which the blob handle is open
** remain the same. Moving an existing blob handle to a new row can be
** faster than closing the existing handle and opening a new one.
**
** ^(The new row must meet the same criteria as for [sqlite3_blob_open()] -
** it must exist and there must be either a blob or text value stored in
** the nominated column.)^ ^If the new row is not present in the table, or if
** it does not contain a blob or text value, or if another error occurs, an
** SQLite error code is returned and the blob handle is considered aborted.
** ^All subsequent calls to [sqlite3_blob_read()], [sqlite3_blob_write()] or
** [sqlite3_blob_reopen()] on an aborted blob handle immediately return
** SQLITE_ABORT. ^Calling [sqlite3_blob_bytes()] on an aborted blob handle
** always returns zero.
**
** ^This function sets the database handle error code and message.
*/
SQLITE_API int SQLITE_STDCALL sqlite3_blob_reopen(sqlite3_blob *, sqlite3_int64);

/*
** CAPI3REF: Close A BLOB Handle
** DESTRUCTOR: sqlite3_blob
**
** ^This function closes an open [BLOB handle]. ^(The BLOB handle is closed
** unconditionally.  Even if this routine returns an error code, the 
** handle is still closed.)^
**
** ^If the blob handle being closed was opened for read-write access, and if
** the database is in auto-commit mode and there are no other open read-write
** blob handles or active write statements, the current transaction is
** committed. ^If an error occurs while committing the transaction, an error
** code is returned and the transaction rolled back.
**
** Calling this function with an argument that is not a NULL pointer or an
** open blob handle results in undefined behaviour. ^Calling this routine 
** with a null pointer (such as would be returned by a failed call to 
** [sqlite3_blob_open()]) is a harmless no-op. ^Otherwise, if this function
** is passed a valid open blob handle, the values returned by the 
** sqlite3_errcode() and sqlite3_errmsg() functions are set before returning.
*/
SQLITE_API int SQLITE_STDCALL sqlite3_blob_close(sqlite3_blob *);

/*
** CAPI3REF: Return The Size Of An Open BLOB
** METHOD: sqlite3_blob
**
** ^Returns the size in bytes of the BLOB accessible via the 
** successfully opened [BLOB handle] in its only argument.  ^The
** incremental blob I/O routines can only read or overwriting existing
** blob content; they cannot change the size of a blob.
**
** This routine only works on a [BLOB handle] which has been created
** by a prior successful call to [sqlite3_blob_open()] and which has not
** been closed by [sqlite3_blob_close()].  Passing any other pointer in
** to this routine results in undefined and probably undesirable behavior.
*/
SQLITE_API int SQLITE_STDCALL sqlite3_blob_bytes(sqlite3_blob *);

/*
** CAPI3REF: Read Data From A BLOB Incrementally
** METHOD: sqlite3_blob
**
** ^(This function is used to read data from an open [BLOB handle] into a
** caller-supplied buffer. N bytes of data are copied into buffer Z
** from the open BLOB, starting at offset iOffset.)^
**
** ^If offset iOffset is less than N bytes from the end of the BLOB,
** [SQLITE_ERROR] is returned and no data is read.  ^If N or iOffset is
** less than zero, [SQLITE_ERROR] is returned and no data is read.
** ^The size of the blob (and hence the maximum value of N+iOffset)
** can be determined using the [sqlite3_blob_bytes()] interface.
**
** ^An attempt to read from an expired [BLOB handle] fails with an
** error code of [SQLITE_ABORT].
**
** ^(On success, sqlite3_blob_read() returns SQLITE_OK.
** Otherwise, an [error code] or an [extended error code] is returned.)^
**
** This routine only works on a [BLOB handle] which has been created
** by a prior successful call to [sqlite3_blob_open()] and which has not
** been closed by [sqlite3_blob_close()].  Passing any other pointer in
** to this routine results in undefined and probably undesirable behavior.
**
** See also: [sqlite3_blob_write()].
*/
SQLITE_API int SQLITE_STDCALL sqlite3_blob_read(sqlite3_blob *, void *Z, int N, int iOffset);

/*
** CAPI3REF: Write Data Into A BLOB Incrementally
** METHOD: sqlite3_blob
**
** ^(This function is used to write data into an open [BLOB handle] from a
** caller-supplied buffer. N bytes of data are copied from the buffer Z
** into the open BLOB, starting at offset iOffset.)^
**
** ^(On success, sqlite3_blob_write() returns SQLITE_OK.
** Otherwise, an  [error code] or an [extended error code] is returned.)^
** ^Unless SQLITE_MISUSE is returned, this function sets the 
** [database connection] error code and message accessible via 
** [sqlite3_errcode()] and [sqlite3_errmsg()] and related functions. 
**
** ^If the [BLOB handle] passed as the first argument was not opened for
** writing (the flags parameter to [sqlite3_blob_open()] was zero),
** this function returns [SQLITE_READONLY].
**
** This function may only modify the contents of the BLOB; it is
** not possible to increase the size of a BLOB using this API.
** ^If offset iOffset is less than N bytes from the end of the BLOB,
** [SQLITE_ERROR] is returned and no data is written. The size of the 
** BLOB (and hence the maximum value of N+iOffset) can be determined 
** using the [sqlite3_blob_bytes()] interface. ^If N or iOffset are less 
** than zero [SQLITE_ERROR] is returned and no data is written.
**
** ^An attempt to write to an expired [BLOB handle] fails with an
** error code of [SQLITE_ABORT].  ^Writes to the BLOB that occurred
** before the [BLOB handle] expired are not rolled back by the
** expiration of the handle, though of course those changes might
** have been overwritten by the statement that expired the BLOB handle
** or by other independent statements.
**
** This routine only works on a [BLOB handle] which has been created
** by a prior successful call to [sqlite3_blob_open()] and which has not
** been closed by [sqlite3_blob_close()].  Passing any other pointer in
** to this routine results in undefined and probably undesirable behavior.
**
** See also: [sqlite3_blob_read()].
*/
SQLITE_API int SQLITE_STDCALL sqlite3_blob_write(sqlite3_blob *, const void *z, int n, int iOffset);

/*
** CAPI3REF: Virtual File System Objects
**
** A virtual filesystem (VFS) is an [sqlite3_vfs] object
** that SQLite uses to interact
** with the underlying operating system.  Most SQLite builds come with a
** single default VFS that is appropriate for the host computer.
** New VFSes can be registered and existing VFSes can be unregistered.
** The following interfaces are provided.
**
** ^The sqlite3_vfs_find() interface returns a pointer to a VFS given its name.
** ^Names are case sensitive.
** ^Names are zero-terminated UTF-8 strings.
** ^If there is no match, a NULL pointer is returned.
** ^If zVfsName is NULL then the default VFS is returned.
**
** ^New VFSes are registered with sqlite3_vfs_register().
** ^Each new VFS becomes the default VFS if the makeDflt flag is set.
** ^The same VFS can be registered multiple times without injury.
** ^To make an existing VFS into the default VFS, register it again
** with the makeDflt flag set.  If two different VFSes with the
** same name are registered, the behavior is undefined.  If a
** VFS is registered with a name that is NULL or an empty string,
** then the behavior is undefined.
**
** ^Unregister a VFS with the sqlite3_vfs_unregister() interface.
** ^(If the default VFS is unregistered, another VFS is chosen as
** the default.  The choice for the new VFS is arbitrary.)^
*/
SQLITE_API sqlite3_vfs *SQLITE_STDCALL sqlite3_vfs_find(const char *zVfsName);
SQLITE_API int SQLITE_STDCALL sqlite3_vfs_register(sqlite3_vfs*, int makeDflt);
SQLITE_API int SQLITE_STDCALL sqlite3_vfs_unregister(sqlite3_vfs*);

/*
** CAPI3REF: Mutexes
**
** The SQLite core uses these routines for thread
** synchronization. Though they are intended for internal
** use by SQLite, code that links against SQLite is
** permitted to use any of these routines.
**
** The SQLite source code contains multiple implementations
** of these mutex routines.  An appropriate implementation
** is selected automatically at compile-time.  The following
** implementations are available in the SQLite core:
**
** <ul>
** <li>   SQLITE_MUTEX_PTHREADS
** <li>   SQLITE_MUTEX_W32
** <li>   SQLITE_MUTEX_NOOP
** </ul>
**
** The SQLITE_MUTEX_NOOP implementation is a set of routines
** that does no real locking and is appropriate for use in
** a single-threaded application.  The SQLITE_MUTEX_PTHREADS and
** SQLITE_MUTEX_W32 implementations are appropriate for use on Unix
** and Windows.
**
** If SQLite is compiled with the SQLITE_MUTEX_APPDEF preprocessor
** macro defined (with "-DSQLITE_MUTEX_APPDEF=1"), then no mutex
** implementation is included with the library. In this case the
** application must supply a custom mutex implementation using the
** [SQLITE_CONFIG_MUTEX] option of the sqlite3_config() function
** before calling sqlite3_initialize() or any other public sqlite3_
** function that calls sqlite3_initialize().
**
** ^The sqlite3_mutex_alloc() routine allocates a new
** mutex and returns a pointer to it. ^The sqlite3_mutex_alloc()
** routine returns NULL if it is unable to allocate the requested
** mutex.  The argument to sqlite3_mutex_alloc() must one of these
** integer constants:
**
** <ul>
** <li>  SQLITE_MUTEX_FAST
** <li>  SQLITE_MUTEX_RECURSIVE
** <li>  SQLITE_MUTEX_STATIC_MASTER
** <li>  SQLITE_MUTEX_STATIC_MEM
** <li>  SQLITE_MUTEX_STATIC_OPEN
** <li>  SQLITE_MUTEX_STATIC_PRNG
** <li>  SQLITE_MUTEX_STATIC_LRU
** <li>  SQLITE_MUTEX_STATIC_PMEM
** <li>  SQLITE_MUTEX_STATIC_APP1
** <li>  SQLITE_MUTEX_STATIC_APP2
** <li>  SQLITE_MUTEX_STATIC_APP3
** <li>  SQLITE_MUTEX_STATIC_VFS1
** <li>  SQLITE_MUTEX_STATIC_VFS2
** <li>  SQLITE_MUTEX_STATIC_VFS3
** </ul>
**
** ^The first two constants (SQLITE_MUTEX_FAST and SQLITE_MUTEX_RECURSIVE)
** cause sqlite3_mutex_alloc() to create
** a new mutex.  ^The new mutex is recursive when SQLITE_MUTEX_RECURSIVE
** is used but not necessarily so when SQLITE_MUTEX_FAST is used.
** The mutex implementation does not need to make a distinction
** between SQLITE_MUTEX_RECURSIVE and SQLITE_MUTEX_FAST if it does
** not want to.  SQLite will only request a recursive mutex in
** cases where it really needs one.  If a faster non-recursive mutex
** implementation is available on the host platform, the mutex subsystem
** might return such a mutex in response to SQLITE_MUTEX_FAST.
**
** ^The other allowed parameters to sqlite3_mutex_alloc() (anything other
** than SQLITE_MUTEX_FAST and SQLITE_MUTEX_RECURSIVE) each return
** a pointer to a static preexisting mutex.  ^Nine static mutexes are
** used by the current version of SQLite.  Future versions of SQLite
** may add additional static mutexes.  Static mutexes are for internal
** use by SQLite only.  Applications that use SQLite mutexes should
** use only the dynamic mutexes returned by SQLITE_MUTEX_FAST or
** SQLITE_MUTEX_RECURSIVE.
**
** ^Note that if one of the dynamic mutex parameters (SQLITE_MUTEX_FAST
** or SQLITE_MUTEX_RECURSIVE) is used then sqlite3_mutex_alloc()
** returns a different mutex on every call.  ^For the static
** mutex types, the same mutex is returned on every call that has
** the same type number.
**
** ^The sqlite3_mutex_free() routine deallocates a previously
** allocated dynamic mutex.  Attempting to deallocate a static
** mutex results in undefined behavior.
**
** ^The sqlite3_mutex_enter() and sqlite3_mutex_try() routines attempt
** to enter a mutex.  ^If another thread is already within the mutex,
** sqlite3_mutex_enter() will block and sqlite3_mutex_try() will return
** SQLITE_BUSY.  ^The sqlite3_mutex_try() interface returns [SQLITE_OK]
** upon successful entry.  ^(Mutexes created using
** SQLITE_MUTEX_RECURSIVE can be entered multiple times by the same thread.
** In such cases, the
** mutex must be exited an equal number of times before another thread
** can enter.)^  If the same thread tries to enter any mutex other
** than an SQLITE_MUTEX_RECURSIVE more than once, the behavior is undefined.
**
** ^(Some systems (for example, Windows 95) do not support the operation
** implemented by sqlite3_mutex_try().  On those systems, sqlite3_mutex_try()
** will always return SQLITE_BUSY. The SQLite core only ever uses
** sqlite3_mutex_try() as an optimization so this is acceptable 
** behavior.)^
**
** ^The sqlite3_mutex_leave() routine exits a mutex that was
** previously entered by the same thread.   The behavior
** is undefined if the mutex is not currently entered by the
** calling thread or is not currently allocated.
**
** ^If the argument to sqlite3_mutex_enter(), sqlite3_mutex_try(), or
** sqlite3_mutex_leave() is a NULL pointer, then all three routines
** behave as no-ops.
**
** See also: [sqlite3_mutex_held()] and [sqlite3_mutex_notheld()].
*/
SQLITE_API sqlite3_mutex *SQLITE_STDCALL sqlite3_mutex_alloc(int);
SQLITE_API void SQLITE_STDCALL sqlite3_mutex_free(sqlite3_mutex*);
SQLITE_API void SQLITE_STDCALL sqlite3_mutex_enter(sqlite3_mutex*);
SQLITE_API int SQLITE_STDCALL sqlite3_mutex_try(sqlite3_mutex*);
SQLITE_API void SQLITE_STDCALL sqlite3_mutex_leave(sqlite3_mutex*);

/*
** CAPI3REF: Mutex Methods Object
**
** An instance of this structure defines the low-level routines
** used to allocate and use mutexes.
**
** Usually, the default mutex implementations provided by SQLite are
** sufficient, however the application has the option of substituting a custom
** implementation for specialized deployments or systems for which SQLite
** does not provide a suitable implementation. In this case, the application
** creates and populates an instance of this structure to pass
** to sqlite3_config() along with the [SQLITE_CONFIG_MUTEX] option.
** Additionally, an instance of this structure can be used as an
** output variable when querying the system for the current mutex
** implementation, using the [SQLITE_CONFIG_GETMUTEX] option.
**
** ^The xMutexInit method defined by this structure is invoked as
** part of system initialization by the sqlite3_initialize() function.
** ^The xMutexInit routine is called by SQLite exactly once for each
** effective call to [sqlite3_initialize()].
**
** ^The xMutexEnd method defined by this structure is invoked as
** part of system shutdown by the sqlite3_shutdown() function. The
** implementation of this method is expected to release all outstanding
** resources obtained by the mutex methods implementation, especially
** those obtained by the xMutexInit method.  ^The xMutexEnd()
** interface is invoked exactly once for each call to [sqlite3_shutdown()].
**
** ^(The remaining seven methods defined by this structure (xMutexAlloc,
** xMutexFree, xMutexEnter, xMutexTry, xMutexLeave, xMutexHeld and
** xMutexNotheld) implement the following interfaces (respectively):
**
** <ul>
**   <li>  [sqlite3_mutex_alloc()] </li>
**   <li>  [sqlite3_mutex_free()] </li>
**   <li>  [sqlite3_mutex_enter()] </li>
**   <li>  [sqlite3_mutex_try()] </li>
**   <li>  [sqlite3_mutex_leave()] </li>
**   <li>  [sqlite3_mutex_held()] </li>
**   <li>  [sqlite3_mutex_notheld()] </li>
** </ul>)^
**
** The only difference is that the public sqlite3_XXX functions enumerated
** above silently ignore any invocations that pass a NULL pointer instead
** of a valid mutex handle. The implementations of the methods defined
** by this structure are not required to handle this case, the results
** of passing a NULL pointer instead of a valid mutex handle are undefined
** (i.e. it is acceptable to provide an implementation that segfaults if
** it is passed a NULL pointer).
**
** The xMutexInit() method must be threadsafe.  It must be harmless to
** invoke xMutexInit() multiple times within the same process and without
** intervening calls to xMutexEnd().  Second and subsequent calls to
** xMutexInit() must be no-ops.
**
** xMutexInit() must not use SQLite memory allocation ([sqlite3_malloc()]
** and its associates).  Similarly, xMutexAlloc() must not use SQLite memory
** allocation for a static mutex.  ^However xMutexAlloc() may use SQLite
** memory allocation for a fast or recursive mutex.
**
** ^SQLite will invoke the xMutexEnd() method when [sqlite3_shutdown()] is
** called, but only if the prior call to xMutexInit returned SQLITE_OK.
** If xMutexInit fails in any way, it is expected to clean up after itself
** prior to returning.
*/
typedef struct sqlite3_mutex_methods sqlite3_mutex_methods;
struct sqlite3_mutex_methods {
  int (*xMutexInit)(void);
  int (*xMutexEnd)(void);
  sqlite3_mutex *(*xMutexAlloc)(int);
  void (*xMutexFree)(sqlite3_mutex *);
  void (*xMutexEnter)(sqlite3_mutex *);
  int (*xMutexTry)(sqlite3_mutex *);
  void (*xMutexLeave)(sqlite3_mutex *);
  int (*xMutexHeld)(sqlite3_mutex *);
  int (*xMutexNotheld)(sqlite3_mutex *);
};

/*
** CAPI3REF: Mutex Verification Routines
**
** The sqlite3_mutex_held() and sqlite3_mutex_notheld() routines
** are intended for use inside assert() statements.  The SQLite core
** never uses these routines except inside an assert() and applications
** are advised to follow the lead of the core.  The SQLite core only
** provides implementations for these routines when it is compiled
** with the SQLITE_DEBUG flag.  External mutex implementations
** are only required to provide these routines if SQLITE_DEBUG is
** defined and if NDEBUG is not defined.
**
** These routines should return true if the mutex in their argument
** is held or not held, respectively, by the calling thread.
**
** The implementation is not required to provide versions of these
** routines that actually work. If the implementation does not provide working
** versions of these routines, it should at least provide stubs that always
** return true so that one does not get spurious assertion failures.
**
** If the argument to sqlite3_mutex_held() is a NULL pointer then
** the routine should return 1.   This seems counter-intuitive since
** clearly the mutex cannot be held if it does not exist.  But
** the reason the mutex does not exist is because the build is not
** using mutexes.  And we do not want the assert() containing the
** call to sqlite3_mutex_held() to fail, so a non-zero return is
** the appropriate thing to do.  The sqlite3_mutex_notheld()
** interface should also return 1 when given a NULL pointer.
*/
#ifndef NDEBUG
SQLITE_API int SQLITE_STDCALL sqlite3_mutex_held(sqlite3_mutex*);
SQLITE_API int SQLITE_STDCALL sqlite3_mutex_notheld(sqlite3_mutex*);
#endif

/*
** CAPI3REF: Mutex Types
**
** The [sqlite3_mutex_alloc()] interface takes a single argument
** which is one of these integer constants.
**
** The set of static mutexes may change from one SQLite release to the
** next.  Applications that override the built-in mutex logic must be
** prepared to accommodate additional static mutexes.
*/
#define SQLITE_MUTEX_FAST             0
#define SQLITE_MUTEX_RECURSIVE        1
#define SQLITE_MUTEX_STATIC_MASTER    2
#define SQLITE_MUTEX_STATIC_MEM       3  /* sqlite3_malloc() */
#define SQLITE_MUTEX_STATIC_MEM2      4  /* NOT USED */
#define SQLITE_MUTEX_STATIC_OPEN      4  /* sqlite3BtreeOpen() */
#define SQLITE_MUTEX_STATIC_PRNG      5  /* sqlite3_random() */
#define SQLITE_MUTEX_STATIC_LRU       6  /* lru page list */
#define SQLITE_MUTEX_STATIC_LRU2      7  /* NOT USED */
#define SQLITE_MUTEX_STATIC_PMEM      7  /* sqlite3PageMalloc() */
#define SQLITE_MUTEX_STATIC_APP1      8  /* For use by application */
#define SQLITE_MUTEX_STATIC_APP2      9  /* For use by application */
#define SQLITE_MUTEX_STATIC_APP3     10  /* For use by application */
#define SQLITE_MUTEX_STATIC_VFS1     11  /* For use by built-in VFS */
#define SQLITE_MUTEX_STATIC_VFS2     12  /* For use by extension VFS */
#define SQLITE_MUTEX_STATIC_VFS3     13  /* For use by application VFS */

/*
** CAPI3REF: Retrieve the mutex for a database connection
** METHOD: sqlite3
**
** ^This interface returns a pointer the [sqlite3_mutex] object that 
** serializes access to the [database connection] given in the argument
** when the [threading mode] is Serialized.
** ^If the [threading mode] is Single-thread or Multi-thread then this
** routine returns a NULL pointer.
*/
SQLITE_API sqlite3_mutex *SQLITE_STDCALL sqlite3_db_mutex(sqlite3*);

/*
** CAPI3REF: Low-Level Control Of Database Files
** METHOD: sqlite3
**
** ^The [sqlite3_file_control()] interface makes a direct call to the
** xFileControl method for the [sqlite3_io_methods] object associated
** with a particular database identified by the second argument. ^The
** name of the database is "main" for the main database or "temp" for the
** TEMP database, or the name that appears after the AS keyword for
** databases that are added using the [ATTACH] SQL command.
** ^A NULL pointer can be used in place of "main" to refer to the
** main database file.
** ^The third and fourth parameters to this routine
** are passed directly through to the second and third parameters of
** the xFileControl method.  ^The return value of the xFileControl
** method becomes the return value of this routine.
**
** ^The SQLITE_FCNTL_FILE_POINTER value for the op parameter causes
** a pointer to the underlying [sqlite3_file] object to be written into
** the space pointed to by the 4th parameter.  ^The SQLITE_FCNTL_FILE_POINTER
** case is a short-circuit path which does not actually invoke the
** underlying sqlite3_io_methods.xFileControl method.
**
** ^If the second parameter (zDbName) does not match the name of any
** open database file, then SQLITE_ERROR is returned.  ^This error
** code is not remembered and will not be recalled by [sqlite3_errcode()]
** or [sqlite3_errmsg()].  The underlying xFileControl method might
** also return SQLITE_ERROR.  There is no way to distinguish between
** an incorrect zDbName and an SQLITE_ERROR return from the underlying
** xFileControl method.
**
** See also: [SQLITE_FCNTL_LOCKSTATE]
*/
SQLITE_API int SQLITE_STDCALL sqlite3_file_control(sqlite3*, const char *zDbName, int op, void*);

/*
** CAPI3REF: Testing Interface
**
** ^The sqlite3_test_control() interface is used to read out internal
** state of SQLite and to inject faults into SQLite for testing
** purposes.  ^The first parameter is an operation code that determines
** the number, meaning, and operation of all subsequent parameters.
**
** This interface is not for use by applications.  It exists solely
** for verifying the correct operation of the SQLite library.  Depending
** on how the SQLite library is compiled, this interface might not exist.
**
** The details of the operation codes, their meanings, the parameters
** they take, and what they do are all subject to change without notice.
** Unlike most of the SQLite API, this function is not guaranteed to
** operate consistently from one release to the next.
*/
SQLITE_API int SQLITE_CDECL sqlite3_test_control(int op, ...);

/*
** CAPI3REF: Testing Interface Operation Codes
**
** These constants are the valid operation code parameters used
** as the first argument to [sqlite3_test_control()].
**
** These parameters and their meanings are subject to change
** without notice.  These values are for testing purposes only.
** Applications should not use any of these parameters or the
** [sqlite3_test_control()] interface.
*/
#define SQLITE_TESTCTRL_FIRST                    5
#define SQLITE_TESTCTRL_PRNG_SAVE                5
#define SQLITE_TESTCTRL_PRNG_RESTORE             6
#define SQLITE_TESTCTRL_PRNG_RESET               7
#define SQLITE_TESTCTRL_BITVEC_TEST              8
#define SQLITE_TESTCTRL_FAULT_INSTALL            9
#define SQLITE_TESTCTRL_BENIGN_MALLOC_HOOKS     10
#define SQLITE_TESTCTRL_PENDING_BYTE            11
#define SQLITE_TESTCTRL_ASSERT                  12
#define SQLITE_TESTCTRL_ALWAYS                  13
#define SQLITE_TESTCTRL_RESERVE                 14
#define SQLITE_TESTCTRL_OPTIMIZATIONS           15
#define SQLITE_TESTCTRL_ISKEYWORD               16
#define SQLITE_TESTCTRL_SCRATCHMALLOC           17
#define SQLITE_TESTCTRL_LOCALTIME_FAULT         18
#define SQLITE_TESTCTRL_EXPLAIN_STMT            19  /* NOT USED */
#define SQLITE_TESTCTRL_NEVER_CORRUPT           20
#define SQLITE_TESTCTRL_VDBE_COVERAGE           21
#define SQLITE_TESTCTRL_BYTEORDER               22
#define SQLITE_TESTCTRL_ISINIT                  23
#define SQLITE_TESTCTRL_SORTER_MMAP             24
#define SQLITE_TESTCTRL_IMPOSTER                25
#define SQLITE_TESTCTRL_LAST                    25

/*
** CAPI3REF: SQLite Runtime Status
**
** ^These interfaces are used to retrieve runtime status information
** about the performance of SQLite, and optionally to reset various
** highwater marks.  ^The first argument is an integer code for
** the specific parameter to measure.  ^(Recognized integer codes
** are of the form [status parameters | SQLITE_STATUS_...].)^
** ^The current value of the parameter is returned into *pCurrent.
** ^The highest recorded value is returned in *pHighwater.  ^If the
** resetFlag is true, then the highest record value is reset after
** *pHighwater is written.  ^(Some parameters do not record the highest
** value.  For those parameters
** nothing is written into *pHighwater and the resetFlag is ignored.)^
** ^(Other parameters record only the highwater mark and not the current
** value.  For these latter parameters nothing is written into *pCurrent.)^
**
** ^The sqlite3_status() and sqlite3_status64() routines return
** SQLITE_OK on success and a non-zero [error code] on failure.
**
** If either the current value or the highwater mark is too large to
** be represented by a 32-bit integer, then the values returned by
** sqlite3_status() are undefined.
**
** See also: [sqlite3_db_status()]
*/
SQLITE_API int SQLITE_STDCALL sqlite3_status(int op, int *pCurrent, int *pHighwater, int resetFlag);
SQLITE_API int SQLITE_STDCALL sqlite3_status64(
  int op,
  sqlite3_int64 *pCurrent,
  sqlite3_int64 *pHighwater,
  int resetFlag
);


/*
** CAPI3REF: Status Parameters
** KEYWORDS: {status parameters}
**
** These integer constants designate various run-time status parameters
** that can be returned by [sqlite3_status()].
**
** <dl>
** [[SQLITE_STATUS_MEMORY_USED]] ^(<dt>SQLITE_STATUS_MEMORY_USED</dt>
** <dd>This parameter is the current amount of memory checked out
** using [sqlite3_malloc()], either directly or indirectly.  The
** figure includes calls made to [sqlite3_malloc()] by the application
** and internal memory usage by the SQLite library.  Scratch memory
** controlled by [SQLITE_CONFIG_SCRATCH] and auxiliary page-cache
** memory controlled by [SQLITE_CONFIG_PAGECACHE] is not included in
** this parameter.  The amount returned is the sum of the allocation
** sizes as reported by the xSize method in [sqlite3_mem_methods].</dd>)^
**
** [[SQLITE_STATUS_MALLOC_SIZE]] ^(<dt>SQLITE_STATUS_MALLOC_SIZE</dt>
** <dd>This parameter records the largest memory allocation request
** handed to [sqlite3_malloc()] or [sqlite3_realloc()] (or their
** internal equivalents).  Only the value returned in the
** *pHighwater parameter to [sqlite3_status()] is of interest.  
** The value written into the *pCurrent parameter is undefined.</dd>)^
**
** [[SQLITE_STATUS_MALLOC_COUNT]] ^(<dt>SQLITE_STATUS_MALLOC_COUNT</dt>
** <dd>This parameter records the number of separate memory allocations
** currently checked out.</dd>)^
**
** [[SQLITE_STATUS_PAGECACHE_USED]] ^(<dt>SQLITE_STATUS_PAGECACHE_USED</dt>
** <dd>This parameter returns the number of pages used out of the
** [pagecache memory allocator] that was configured using 
** [SQLITE_CONFIG_PAGECACHE].  The
** value returned is in pages, not in bytes.</dd>)^
**
** [[SQLITE_STATUS_PAGECACHE_OVERFLOW]] 
** ^(<dt>SQLITE_STATUS_PAGECACHE_OVERFLOW</dt>
** <dd>This parameter returns the number of bytes of page cache
** allocation which could not be satisfied by the [SQLITE_CONFIG_PAGECACHE]
** buffer and where forced to overflow to [sqlite3_malloc()].  The
** returned value includes allocations that overflowed because they
** where too large (they were larger than the "sz" parameter to
** [SQLITE_CONFIG_PAGECACHE]) and allocations that overflowed because
** no space was left in the page cache.</dd>)^
**
** [[SQLITE_STATUS_PAGECACHE_SIZE]] ^(<dt>SQLITE_STATUS_PAGECACHE_SIZE</dt>
** <dd>This parameter records the largest memory allocation request
** handed to [pagecache memory allocator].  Only the value returned in the
** *pHighwater parameter to [sqlite3_status()] is of interest.  
** The value written into the *pCurrent parameter is undefined.</dd>)^
**
** [[SQLITE_STATUS_SCRATCH_USED]] ^(<dt>SQLITE_STATUS_SCRATCH_USED</dt>
** <dd>This parameter returns the number of allocations used out of the
** [scratch memory allocator] configured using
** [SQLITE_CONFIG_SCRATCH].  The value returned is in allocations, not
** in bytes.  Since a single thread may only have one scratch allocation
** outstanding at time, this parameter also reports the number of threads
** using scratch memory at the same time.</dd>)^
**
** [[SQLITE_STATUS_SCRATCH_OVERFLOW]] ^(<dt>SQLITE_STATUS_SCRATCH_OVERFLOW</dt>
** <dd>This parameter returns the number of bytes of scratch memory
** allocation which could not be satisfied by the [SQLITE_CONFIG_SCRATCH]
** buffer and where forced to overflow to [sqlite3_malloc()].  The values
** returned include overflows because the requested allocation was too
** larger (that is, because the requested allocation was larger than the
** "sz" parameter to [SQLITE_CONFIG_SCRATCH]) and because no scratch buffer
** slots were available.
** </dd>)^
**
** [[SQLITE_STATUS_SCRATCH_SIZE]] ^(<dt>SQLITE_STATUS_SCRATCH_SIZE</dt>
** <dd>This parameter records the largest memory allocation request
** handed to [scratch memory allocator].  Only the value returned in the
** *pHighwater parameter to [sqlite3_status()] is of interest.  
** The value written into the *pCurrent parameter is undefined.</dd>)^
**
** [[SQLITE_STATUS_PARSER_STACK]] ^(<dt>SQLITE_STATUS_PARSER_STACK</dt>
** <dd>The *pHighwater parameter records the deepest parser stack. 
** The *pCurrent value is undefined.  The *pHighwater value is only
** meaningful if SQLite is compiled with [YYTRACKMAXSTACKDEPTH].</dd>)^
** </dl>
**
** New status parameters may be added from time to time.
*/
#define SQLITE_STATUS_MEMORY_USED          0
#define SQLITE_STATUS_PAGECACHE_USED       1
#define SQLITE_STATUS_PAGECACHE_OVERFLOW   2
#define SQLITE_STATUS_SCRATCH_USED         3
#define SQLITE_STATUS_SCRATCH_OVERFLOW     4
#define SQLITE_STATUS_MALLOC_SIZE          5
#define SQLITE_STATUS_PARSER_STACK         6
#define SQLITE_STATUS_PAGECACHE_SIZE       7
#define SQLITE_STATUS_SCRATCH_SIZE         8
#define SQLITE_STATUS_MALLOC_COUNT         9

/*
** CAPI3REF: Database Connection Status
** METHOD: sqlite3
**
** ^This interface is used to retrieve runtime status information 
** about a single [database connection].  ^The first argument is the
** database connection object to be interrogated.  ^The second argument
** is an integer constant, taken from the set of
** [SQLITE_DBSTATUS options], that
** determines the parameter to interrogate.  The set of 
** [SQLITE_DBSTATUS options] is likely
** to grow in future releases of SQLite.
**
** ^The current value of the requested parameter is written into *pCur
** and the highest instantaneous value is written into *pHiwtr.  ^If
** the resetFlg is true, then the highest instantaneous value is
** reset back down to the current value.
**
** ^The sqlite3_db_status() routine returns SQLITE_OK on success and a
** non-zero [error code] on failure.
**
** See also: [sqlite3_status()] and [sqlite3_stmt_status()].
*/
SQLITE_API int SQLITE_STDCALL sqlite3_db_status(sqlite3*, int op, int *pCur, int *pHiwtr, int resetFlg);

/*
** CAPI3REF: Status Parameters for database connections
** KEYWORDS: {SQLITE_DBSTATUS options}
**
** These constants are the available integer "verbs" that can be passed as
** the second argument to the [sqlite3_db_status()] interface.
**
** New verbs may be added in future releases of SQLite. Existing verbs
** might be discontinued. Applications should check the return code from
** [sqlite3_db_status()] to make sure that the call worked.
** The [sqlite3_db_status()] interface will return a non-zero error code
** if a discontinued or unsupported verb is invoked.
**
** <dl>
** [[SQLITE_DBSTATUS_LOOKASIDE_USED]] ^(<dt>SQLITE_DBSTATUS_LOOKASIDE_USED</dt>
** <dd>This parameter returns the number of lookaside memory slots currently
** checked out.</dd>)^
**
** [[SQLITE_DBSTATUS_LOOKASIDE_HIT]] ^(<dt>SQLITE_DBSTATUS_LOOKASIDE_HIT</dt>
** <dd>This parameter returns the number malloc attempts that were 
** satisfied using lookaside memory. Only the high-water value is meaningful;
** the current value is always zero.)^
**
** [[SQLITE_DBSTATUS_LOOKASIDE_MISS_SIZE]]
** ^(<dt>SQLITE_DBSTATUS_LOOKASIDE_MISS_SIZE</dt>
** <dd>This parameter returns the number malloc attempts that might have
** been satisfied using lookaside memory but failed due to the amount of
** memory requested being larger than the lookaside slot size.
** Only the high-water value is meaningful;
** the current value is always zero.)^
**
** [[SQLITE_DBSTATUS_LOOKASIDE_MISS_FULL]]
** ^(<dt>SQLITE_DBSTATUS_LOOKASIDE_MISS_FULL</dt>
** <dd>This parameter returns the number malloc attempts that might have
** been satisfied using lookaside memory but failed due to all lookaside
** memory already being in use.
** Only the high-water value is meaningful;
** the current value is always zero.)^
**
** [[SQLITE_DBSTATUS_CACHE_USED]] ^(<dt>SQLITE_DBSTATUS_CACHE_USED</dt>
** <dd>This parameter returns the approximate number of bytes of heap
** memory used by all pager caches associated with the database connection.)^
** ^The highwater mark associated with SQLITE_DBSTATUS_CACHE_USED is always 0.
**
** [[SQLITE_DBSTATUS_SCHEMA_USED]] ^(<dt>SQLITE_DBSTATUS_SCHEMA_USED</dt>
** <dd>This parameter returns the approximate number of bytes of heap
** memory used to store the schema for all databases associated
** with the connection - main, temp, and any [ATTACH]-ed databases.)^ 
** ^The full amount of memory used by the schemas is reported, even if the
** schema memory is shared with other database connections due to
** [shared cache mode] being enabled.
** ^The highwater mark associated with SQLITE_DBSTATUS_SCHEMA_USED is always 0.
**
** [[SQLITE_DBSTATUS_STMT_USED]] ^(<dt>SQLITE_DBSTATUS_STMT_USED</dt>
** <dd>This parameter returns the approximate number of bytes of heap
** and lookaside memory used by all prepared statements associated with
** the database connection.)^
** ^The highwater mark associated with SQLITE_DBSTATUS_STMT_USED is always 0.
** </dd>
**
** [[SQLITE_DBSTATUS_CACHE_HIT]] ^(<dt>SQLITE_DBSTATUS_CACHE_HIT</dt>
** <dd>This parameter returns the number of pager cache hits that have
** occurred.)^ ^The highwater mark associated with SQLITE_DBSTATUS_CACHE_HIT 
** is always 0.
** </dd>
**
** [[SQLITE_DBSTATUS_CACHE_MISS]] ^(<dt>SQLITE_DBSTATUS_CACHE_MISS</dt>
** <dd>This parameter returns the number of pager cache misses that have
** occurred.)^ ^The highwater mark associated with SQLITE_DBSTATUS_CACHE_MISS 
** is always 0.
** </dd>
**
** [[SQLITE_DBSTATUS_CACHE_WRITE]] ^(<dt>SQLITE_DBSTATUS_CACHE_WRITE</dt>
** <dd>This parameter returns the number of dirty cache entries that have
** been written to disk. Specifically, the number of pages written to the
** wal file in wal mode databases, or the number of pages written to the
** database file in rollback mode databases. Any pages written as part of
** transaction rollback or database recovery operations are not included.
** If an IO or other error occurs while writing a page to disk, the effect
** on subsequent SQLITE_DBSTATUS_CACHE_WRITE requests is undefined.)^ ^The
** highwater mark associated with SQLITE_DBSTATUS_CACHE_WRITE is always 0.
** </dd>
**
** [[SQLITE_DBSTATUS_DEFERRED_FKS]] ^(<dt>SQLITE_DBSTATUS_DEFERRED_FKS</dt>
** <dd>This parameter returns zero for the current value if and only if
** all foreign key constraints (deferred or immediate) have been
** resolved.)^  ^The highwater mark is always 0.
** </dd>
** </dl>
*/
#define SQLITE_DBSTATUS_LOOKASIDE_USED       0
#define SQLITE_DBSTATUS_CACHE_USED           1
#define SQLITE_DBSTATUS_SCHEMA_USED          2
#define SQLITE_DBSTATUS_STMT_USED            3
#define SQLITE_DBSTATUS_LOOKASIDE_HIT        4
#define SQLITE_DBSTATUS_LOOKASIDE_MISS_SIZE  5
#define SQLITE_DBSTATUS_LOOKASIDE_MISS_FULL  6
#define SQLITE_DBSTATUS_CACHE_HIT            7
#define SQLITE_DBSTATUS_CACHE_MISS           8
#define SQLITE_DBSTATUS_CACHE_WRITE          9
#define SQLITE_DBSTATUS_DEFERRED_FKS        10
#define SQLITE_DBSTATUS_MAX                 10   /* Largest defined DBSTATUS */


/*
** CAPI3REF: Prepared Statement Status
** METHOD: sqlite3_stmt
**
** ^(Each prepared statement maintains various
** [SQLITE_STMTSTATUS counters] that measure the number
** of times it has performed specific operations.)^  These counters can
** be used to monitor the performance characteristics of the prepared
** statements.  For example, if the number of table steps greatly exceeds
** the number of table searches or result rows, that would tend to indicate
** that the prepared statement is using a full table scan rather than
** an index.  
**
** ^(This interface is used to retrieve and reset counter values from
** a [prepared statement].  The first argument is the prepared statement
** object to be interrogated.  The second argument
** is an integer code for a specific [SQLITE_STMTSTATUS counter]
** to be interrogated.)^
** ^The current value of the requested counter is returned.
** ^If the resetFlg is true, then the counter is reset to zero after this
** interface call returns.
**
** See also: [sqlite3_status()] and [sqlite3_db_status()].
*/
SQLITE_API int SQLITE_STDCALL sqlite3_stmt_status(sqlite3_stmt*, int op,int resetFlg);

/*
** CAPI3REF: Status Parameters for prepared statements
** KEYWORDS: {SQLITE_STMTSTATUS counter} {SQLITE_STMTSTATUS counters}
**
** These preprocessor macros define integer codes that name counter
** values associated with the [sqlite3_stmt_status()] interface.
** The meanings of the various counters are as follows:
**
** <dl>
** [[SQLITE_STMTSTATUS_FULLSCAN_STEP]] <dt>SQLITE_STMTSTATUS_FULLSCAN_STEP</dt>
** <dd>^This is the number of times that SQLite has stepped forward in
** a table as part of a full table scan.  Large numbers for this counter
** may indicate opportunities for performance improvement through 
** careful use of indices.</dd>
**
** [[SQLITE_STMTSTATUS_SORT]] <dt>SQLITE_STMTSTATUS_SORT</dt>
** <dd>^This is the number of sort operations that have occurred.
** A non-zero value in this counter may indicate an opportunity to
** improvement performance through careful use of indices.</dd>
**
** [[SQLITE_STMTSTATUS_AUTOINDEX]] <dt>SQLITE_STMTSTATUS_AUTOINDEX</dt>
** <dd>^This is the number of rows inserted into transient indices that
** were created automatically in order to help joins run faster.
** A non-zero value in this counter may indicate an opportunity to
** improvement performance by adding permanent indices that do not
** need to be reinitialized each time the statement is run.</dd>
**
** [[SQLITE_STMTSTATUS_VM_STEP]] <dt>SQLITE_STMTSTATUS_VM_STEP</dt>
** <dd>^This is the number of virtual machine operations executed
** by the prepared statement if that number is less than or equal
** to 2147483647.  The number of virtual machine operations can be 
** used as a proxy for the total work done by the prepared statement.
** If the number of virtual machine operations exceeds 2147483647
** then the value returned by this statement status code is undefined.
** </dd>
** </dl>
*/
#define SQLITE_STMTSTATUS_FULLSCAN_STEP     1
#define SQLITE_STMTSTATUS_SORT              2
#define SQLITE_STMTSTATUS_AUTOINDEX         3
#define SQLITE_STMTSTATUS_VM_STEP           4

/*
** CAPI3REF: Custom Page Cache Object
**
** The sqlite3_pcache type is opaque.  It is implemented by
** the pluggable module.  The SQLite core has no knowledge of
** its size or internal structure and never deals with the
** sqlite3_pcache object except by holding and passing pointers
** to the object.
**
** See [sqlite3_pcache_methods2] for additional information.
*/
typedef struct sqlite3_pcache sqlite3_pcache;

/*
** CAPI3REF: Custom Page Cache Object
**
** The sqlite3_pcache_page object represents a single page in the
** page cache.  The page cache will allocate instances of this
** object.  Various methods of the page cache use pointers to instances
** of this object as parameters or as their return value.
**
** See [sqlite3_pcache_methods2] for additional information.
*/
typedef struct sqlite3_pcache_page sqlite3_pcache_page;
struct sqlite3_pcache_page {
  void *pBuf;        /* The content of the page */
  void *pExtra;      /* Extra information associated with the page */
};

/*
** CAPI3REF: Application Defined Page Cache.
** KEYWORDS: {page cache}
**
** ^(The [sqlite3_config]([SQLITE_CONFIG_PCACHE2], ...) interface can
** register an alternative page cache implementation by passing in an 
** instance of the sqlite3_pcache_methods2 structure.)^
** In many applications, most of the heap memory allocated by 
** SQLite is used for the page cache.
** By implementing a 
** custom page cache using this API, an application can better control
** the amount of memory consumed by SQLite, the way in which 
** that memory is allocated and released, and the policies used to 
** determine exactly which parts of a database file are cached and for 
** how long.
**
** The alternative page cache mechanism is an
** extreme measure that is only needed by the most demanding applications.
** The built-in page cache is recommended for most uses.
**
** ^(The contents of the sqlite3_pcache_methods2 structure are copied to an
** internal buffer by SQLite within the call to [sqlite3_config].  Hence
** the application may discard the parameter after the call to
** [sqlite3_config()] returns.)^
**
** [[the xInit() page cache method]]
** ^(The xInit() method is called once for each effective 
** call to [sqlite3_initialize()])^
** (usually only once during the lifetime of the process). ^(The xInit()
** method is passed a copy of the sqlite3_pcache_methods2.pArg value.)^
** The intent of the xInit() method is to set up global data structures 
** required by the custom page cache implementation. 
** ^(If the xInit() method is NULL, then the 
** built-in default page cache is used instead of the application defined
** page cache.)^
**
** [[the xShutdown() page cache method]]
** ^The xShutdown() method is called by [sqlite3_shutdown()].
** It can be used to clean up 
** any outstanding resources before process shutdown, if required.
** ^The xShutdown() method may be NULL.
**
** ^SQLite automatically serializes calls to the xInit method,
** so the xInit method need not be threadsafe.  ^The
** xShutdown method is only called from [sqlite3_shutdown()] so it does
** not need to be threadsafe either.  All other methods must be threadsafe
** in multithreaded applications.
**
** ^SQLite will never invoke xInit() more than once without an intervening
** call to xShutdown().
**
** [[the xCreate() page cache methods]]
** ^SQLite invokes the xCreate() method to construct a new cache instance.
** SQLite will typically create one cache instance for each open database file,
** though this is not guaranteed. ^The
** first parameter, szPage, is the size in bytes of the pages that must
** be allocated by the cache.  ^szPage will always a power of two.  ^The
** second parameter szExtra is a number of bytes of extra storage 
** associated with each page cache entry.  ^The szExtra parameter will
** a number less than 250.  SQLite will use the
** extra szExtra bytes on each page to store metadata about the underlying
** database page on disk.  The value passed into szExtra depends
** on the SQLite version, the target platform, and how SQLite was compiled.
** ^The third argument to xCreate(), bPurgeable, is true if the cache being
** created will be used to cache database pages of a file stored on disk, or
** false if it is used for an in-memory database. The cache implementation
** does not have to do anything special based with the value of bPurgeable;
** it is purely advisory.  ^On a cache where bPurgeable is false, SQLite will
** never invoke xUnpin() except to deliberately delete a page.
** ^In other words, calls to xUnpin() on a cache with bPurgeable set to
** false will always have the "discard" flag set to true.  
** ^Hence, a cache created with bPurgeable false will
** never contain any unpinned pages.
**
** [[the xCachesize() page cache method]]
** ^(The xCachesize() method may be called at any time by SQLite to set the
** suggested maximum cache-size (number of pages stored by) the cache
** instance passed as the first argument. This is the value configured using
** the SQLite "[PRAGMA cache_size]" command.)^  As with the bPurgeable
** parameter, the implementation is not required to do anything with this
** value; it is advisory only.
**
** [[the xPagecount() page cache methods]]
** The xPagecount() method must return the number of pages currently
** stored in the cache, both pinned and unpinned.
** 
** [[the xFetch() page cache methods]]
** The xFetch() method locates a page in the cache and returns a pointer to 
** an sqlite3_pcache_page object associated with that page, or a NULL pointer.
** The pBuf element of the returned sqlite3_pcache_page object will be a
** pointer to a buffer of szPage bytes used to store the content of a 
** single database page.  The pExtra element of sqlite3_pcache_page will be
** a pointer to the szExtra bytes of extra storage that SQLite has requested
** for each entry in the page cache.
**
** The page to be fetched is determined by the key. ^The minimum key value
** is 1.  After it has been retrieved using xFetch, the page is considered
** to be "pinned".
**
** If the requested page is already in the page cache, then the page cache
** implementation must return a pointer to the page buffer with its content
** intact.  If the requested page is not already in the cache, then the
** cache implementation should use the value of the createFlag
** parameter to help it determined what action to take:
**
** <table border=1 width=85% align=center>
** <tr><th> createFlag <th> Behavior when page is not already in cache
** <tr><td> 0 <td> Do not allocate a new page.  Return NULL.
** <tr><td> 1 <td> Allocate a new page if it easy and convenient to do so.
**                 Otherwise return NULL.
** <tr><td> 2 <td> Make every effort to allocate a new page.  Only return
**                 NULL if allocating a new page is effectively impossible.
** </table>
**
** ^(SQLite will normally invoke xFetch() with a createFlag of 0 or 1.  SQLite
** will only use a createFlag of 2 after a prior call with a createFlag of 1
** failed.)^  In between the to xFetch() calls, SQLite may
** attempt to unpin one or more cache pages by spilling the content of
** pinned pages to disk and synching the operating system disk cache.
**
** [[the xUnpin() page cache method]]
** ^xUnpin() is called by SQLite with a pointer to a currently pinned page
** as its second argument.  If the third parameter, discard, is non-zero,
** then the page must be evicted from the cache.
** ^If the discard parameter is
** zero, then the page may be discarded or retained at the discretion of
** page cache implementation. ^The page cache implementation
** may choose to evict unpinned pages at any time.
**
** The cache must not perform any reference counting. A single 
** call to xUnpin() unpins the page regardless of the number of prior calls 
** to xFetch().
**
** [[the xRekey() page cache methods]]
** The xRekey() method is used to change the key value associated with the
** page passed as the second argument. If the cache
** previously contains an entry associated with newKey, it must be
** discarded. ^Any prior cache entry associated with newKey is guaranteed not
** to be pinned.
**
** When SQLite calls the xTruncate() method, the cache must discard all
** existing cache entries with page numbers (keys) greater than or equal
** to the value of the iLimit parameter passed to xTruncate(). If any
** of these pages are pinned, they are implicitly unpinned, meaning that
** they can be safely discarded.
**
** [[the xDestroy() page cache method]]
** ^The xDestroy() method is used to delete a cache allocated by xCreate().
** All resources associated with the specified cache should be freed. ^After
** calling the xDestroy() method, SQLite considers the [sqlite3_pcache*]
** handle invalid, and will not use it with any other sqlite3_pcache_methods2
** functions.
**
** [[the xShrink() page cache method]]
** ^SQLite invokes the xShrink() method when it wants the page cache to
** free up as much of heap memory as possible.  The page cache implementation
** is not obligated to free any memory, but well-behaved implementations should
** do their best.
*/
typedef struct sqlite3_pcache_methods2 sqlite3_pcache_methods2;
struct sqlite3_pcache_methods2 {
  int iVersion;
  void *pArg;
  int (*xInit)(void*);
  void (*xShutdown)(void*);
  sqlite3_pcache *(*xCreate)(int szPage, int szExtra, int bPurgeable);
  void (*xCachesize)(sqlite3_pcache*, int nCachesize);
  int (*xPagecount)(sqlite3_pcache*);
  sqlite3_pcache_page *(*xFetch)(sqlite3_pcache*, unsigned key, int createFlag);
  void (*xUnpin)(sqlite3_pcache*, sqlite3_pcache_page*, int discard);
  void (*xRekey)(sqlite3_pcache*, sqlite3_pcache_page*, 
      unsigned oldKey, unsigned newKey);
  void (*xTruncate)(sqlite3_pcache*, unsigned iLimit);
  void (*xDestroy)(sqlite3_pcache*);
  void (*xShrink)(sqlite3_pcache*);
};

/*
** This is the obsolete pcache_methods object that has now been replaced
** by sqlite3_pcache_methods2.  This object is not used by SQLite.  It is
** retained in the header file for backwards compatibility only.
*/
typedef struct sqlite3_pcache_methods sqlite3_pcache_methods;
struct sqlite3_pcache_methods {
  void *pArg;
  int (*xInit)(void*);
  void (*xShutdown)(void*);
  sqlite3_pcache *(*xCreate)(int szPage, int bPurgeable);
  void (*xCachesize)(sqlite3_pcache*, int nCachesize);
  int (*xPagecount)(sqlite3_pcache*);
  void *(*xFetch)(sqlite3_pcache*, unsigned key, int createFlag);
  void (*xUnpin)(sqlite3_pcache*, void*, int discard);
  void (*xRekey)(sqlite3_pcache*, void*, unsigned oldKey, unsigned newKey);
  void (*xTruncate)(sqlite3_pcache*, unsigned iLimit);
  void (*xDestroy)(sqlite3_pcache*);
};


/*
** CAPI3REF: Online Backup Object
**
** The sqlite3_backup object records state information about an ongoing
** online backup operation.  ^The sqlite3_backup object is created by
** a call to [sqlite3_backup_init()] and is destroyed by a call to
** [sqlite3_backup_finish()].
**
** See Also: [Using the SQLite Online Backup API]
*/
typedef struct sqlite3_backup sqlite3_backup;

/*
** CAPI3REF: Online Backup API.
**
** The backup API copies the content of one database into another.
** It is useful either for creating backups of databases or
** for copying in-memory databases to or from persistent files. 
**
** See Also: [Using the SQLite Online Backup API]
**
** ^SQLite holds a write transaction open on the destination database file
** for the duration of the backup operation.
** ^The source database is read-locked only while it is being read;
** it is not locked continuously for the entire backup operation.
** ^Thus, the backup may be performed on a live source database without
** preventing other database connections from
** reading or writing to the source database while the backup is underway.
** 
** ^(To perform a backup operation: 
**   <ol>
**     <li><b>sqlite3_backup_init()</b> is called once to initialize the
**         backup, 
**     <li><b>sqlite3_backup_step()</b> is called one or more times to transfer 
**         the data between the two databases, and finally
**     <li><b>sqlite3_backup_finish()</b> is called to release all resources 
**         associated with the backup operation. 
**   </ol>)^
** There should be exactly one call to sqlite3_backup_finish() for each
** successful call to sqlite3_backup_init().
**
** [[sqlite3_backup_init()]] <b>sqlite3_backup_init()</b>
**
** ^The D and N arguments to sqlite3_backup_init(D,N,S,M) are the 
** [database connection] associated with the destination database 
** and the database name, respectively.
** ^The database name is "main" for the main database, "temp" for the
** temporary database, or the name specified after the AS keyword in
** an [ATTACH] statement for an attached database.
** ^The S and M arguments passed to 
** sqlite3_backup_init(D,N,S,M) identify the [database connection]
** and database name of the source database, respectively.
** ^The source and destination [database connections] (parameters S and D)
** must be different or else sqlite3_backup_init(D,N,S,M) will fail with
** an error.
**
** ^A call to sqlite3_backup_init() will fail, returning NULL, if 
** there is already a read or read-write transaction open on the 
** destination database.
**
** ^If an error occurs within sqlite3_backup_init(D,N,S,M), then NULL is
** returned and an error code and error message are stored in the
** destination [database connection] D.
** ^The error code and message for the failed call to sqlite3_backup_init()
** can be retrieved using the [sqlite3_errcode()], [sqlite3_errmsg()], and/or
** [sqlite3_errmsg16()] functions.
** ^A successful call to sqlite3_backup_init() returns a pointer to an
** [sqlite3_backup] object.
** ^The [sqlite3_backup] object may be used with the sqlite3_backup_step() and
** sqlite3_backup_finish() functions to perform the specified backup 
** operation.
**
** [[sqlite3_backup_step()]] <b>sqlite3_backup_step()</b>
**
** ^Function sqlite3_backup_step(B,N) will copy up to N pages between 
** the source and destination databases specified by [sqlite3_backup] object B.
** ^If N is negative, all remaining source pages are copied. 
** ^If sqlite3_backup_step(B,N) successfully copies N pages and there
** are still more pages to be copied, then the function returns [SQLITE_OK].
** ^If sqlite3_backup_step(B,N) successfully finishes copying all pages
** from source to destination, then it returns [SQLITE_DONE].
** ^If an error occurs while running sqlite3_backup_step(B,N),
** then an [error code] is returned. ^As well as [SQLITE_OK] and
** [SQLITE_DONE], a call to sqlite3_backup_step() may return [SQLITE_READONLY],
** [SQLITE_NOMEM], [SQLITE_BUSY], [SQLITE_LOCKED], or an
** [SQLITE_IOERR_ACCESS | SQLITE_IOERR_XXX] extended error code.
**
** ^(The sqlite3_backup_step() might return [SQLITE_READONLY] if
** <ol>
** <li> the destination database was opened read-only, or
** <li> the destination database is using write-ahead-log journaling
** and the destination and source page sizes differ, or
** <li> the destination database is an in-memory database and the
** destination and source page sizes differ.
** </ol>)^
**
** ^If sqlite3_backup_step() cannot obtain a required file-system lock, then
** the [sqlite3_busy_handler | busy-handler function]
** is invoked (if one is specified). ^If the 
** busy-handler returns non-zero before the lock is available, then 
** [SQLITE_BUSY] is returned to the caller. ^In this case the call to
** sqlite3_backup_step() can be retried later. ^If the source
** [database connection]
** is being used to write to the source database when sqlite3_backup_step()
** is called, then [SQLITE_LOCKED] is returned immediately. ^Again, in this
** case the call to sqlite3_backup_step() can be retried later on. ^(If
** [SQLITE_IOERR_ACCESS | SQLITE_IOERR_XXX], [SQLITE_NOMEM], or
** [SQLITE_READONLY] is returned, then 
** there is no point in retrying the call to sqlite3_backup_step(). These 
** errors are considered fatal.)^  The application must accept 
** that the backup operation has failed and pass the backup operation handle 
** to the sqlite3_backup_finish() to release associated resources.
**
** ^The first call to sqlite3_backup_step() obtains an exclusive lock
** on the destination file. ^The exclusive lock is not released until either 
** sqlite3_backup_finish() is called or the backup operation is complete 
** and sqlite3_backup_step() returns [SQLITE_DONE].  ^Every call to
** sqlite3_backup_step() obtains a [shared lock] on the source database that
** lasts for the duration of the sqlite3_backup_step() call.
** ^Because the source database is not locked between calls to
** sqlite3_backup_step(), the source database may be modified mid-way
** through the backup process.  ^If the source database is modified by an
** external process or via a database connection other than the one being
** used by the backup operation, then the backup will be automatically
** restarted by the next call to sqlite3_backup_step(). ^If the source 
** database is modified by the using the same database connection as is used
** by the backup operation, then the backup database is automatically
** updated at the same time.
**
** [[sqlite3_backup_finish()]] <b>sqlite3_backup_finish()</b>
**
** When sqlite3_backup_step() has returned [SQLITE_DONE], or when the 
** application wishes to abandon the backup operation, the application
** should destroy the [sqlite3_backup] by passing it to sqlite3_backup_finish().
** ^The sqlite3_backup_finish() interfaces releases all
** resources associated with the [sqlite3_backup] object. 
** ^If sqlite3_backup_step() has not yet returned [SQLITE_DONE], then any
** active write-transaction on the destination database is rolled back.
** The [sqlite3_backup] object is invalid
** and may not be used following a call to sqlite3_backup_finish().
**
** ^The value returned by sqlite3_backup_finish is [SQLITE_OK] if no
** sqlite3_backup_step() errors occurred, regardless or whether or not
** sqlite3_backup_step() completed.
** ^If an out-of-memory condition or IO error occurred during any prior
** sqlite3_backup_step() call on the same [sqlite3_backup] object, then
** sqlite3_backup_finish() returns the corresponding [error code].
**
** ^A return of [SQLITE_BUSY] or [SQLITE_LOCKED] from sqlite3_backup_step()
** is not a permanent error and does not affect the return value of
** sqlite3_backup_finish().
**
** [[sqlite3_backup_remaining()]] [[sqlite3_backup_pagecount()]]
** <b>sqlite3_backup_remaining() and sqlite3_backup_pagecount()</b>
**
** ^The sqlite3_backup_remaining() routine returns the number of pages still
** to be backed up at the conclusion of the most recent sqlite3_backup_step().
** ^The sqlite3_backup_pagecount() routine returns the total number of pages
** in the source database at the conclusion of the most recent
** sqlite3_backup_step().
** ^(The values returned by these functions are only updated by
** sqlite3_backup_step(). If the source database is modified in a way that
** changes the size of the source database or the number of pages remaining,
** those changes are not reflected in the output of sqlite3_backup_pagecount()
** and sqlite3_backup_remaining() until after the next
** sqlite3_backup_step().)^
**
** <b>Concurrent Usage of Database Handles</b>
**
** ^The source [database connection] may be used by the application for other
** purposes while a backup operation is underway or being initialized.
** ^If SQLite is compiled and configured to support threadsafe database
** connections, then the source database connection may be used concurrently
** from within other threads.
**
** However, the application must guarantee that the destination 
** [database connection] is not passed to any other API (by any thread) after 
** sqlite3_backup_init() is called and before the corresponding call to
** sqlite3_backup_finish().  SQLite does not currently check to see
** if the application incorrectly accesses the destination [database connection]
** and so no error code is reported, but the operations may malfunction
** nevertheless.  Use of the destination database connection while a
** backup is in progress might also also cause a mutex deadlock.
**
** If running in [shared cache mode], the application must
** guarantee that the shared cache used by the destination database
** is not accessed while the backup is running. In practice this means
** that the application must guarantee that the disk file being 
** backed up to is not accessed by any connection within the process,
** not just the specific connection that was passed to sqlite3_backup_init().
**
** The [sqlite3_backup] object itself is partially threadsafe. Multiple 
** threads may safely make multiple concurrent calls to sqlite3_backup_step().
** However, the sqlite3_backup_remaining() and sqlite3_backup_pagecount()
** APIs are not strictly speaking threadsafe. If they are invoked at the
** same time as another thread is invoking sqlite3_backup_step() it is
** possible that they return invalid values.
*/
SQLITE_API sqlite3_backup *SQLITE_STDCALL sqlite3_backup_init(
  sqlite3 *pDest,                        /* Destination database handle */
  const char *zDestName,                 /* Destination database name */
  sqlite3 *pSource,                      /* Source database handle */
  const char *zSourceName                /* Source database name */
);
SQLITE_API int SQLITE_STDCALL sqlite3_backup_step(sqlite3_backup *p, int nPage);
SQLITE_API int SQLITE_STDCALL sqlite3_backup_finish(sqlite3_backup *p);
SQLITE_API int SQLITE_STDCALL sqlite3_backup_remaining(sqlite3_backup *p);
SQLITE_API int SQLITE_STDCALL sqlite3_backup_pagecount(sqlite3_backup *p);

/*
** CAPI3REF: Unlock Notification
** METHOD: sqlite3
**
** ^When running in shared-cache mode, a database operation may fail with
** an [SQLITE_LOCKED] error if the required locks on the shared-cache or
** individual tables within the shared-cache cannot be obtained. See
** [SQLite Shared-Cache Mode] for a description of shared-cache locking. 
** ^This API may be used to register a callback that SQLite will invoke 
** when the connection currently holding the required lock relinquishes it.
** ^This API is only available if the library was compiled with the
** [SQLITE_ENABLE_UNLOCK_NOTIFY] C-preprocessor symbol defined.
**
** See Also: [Using the SQLite Unlock Notification Feature].
**
** ^Shared-cache locks are released when a database connection concludes
** its current transaction, either by committing it or rolling it back. 
**
** ^When a connection (known as the blocked connection) fails to obtain a
** shared-cache lock and SQLITE_LOCKED is returned to the caller, the
** identity of the database connection (the blocking connection) that
** has locked the required resource is stored internally. ^After an 
** application receives an SQLITE_LOCKED error, it may call the
** sqlite3_unlock_notify() method with the blocked connection handle as 
** the first argument to register for a callback that will be invoked
** when the blocking connections current transaction is concluded. ^The
** callback is invoked from within the [sqlite3_step] or [sqlite3_close]
** call that concludes the blocking connections transaction.
**
** ^(If sqlite3_unlock_notify() is called in a multi-threaded application,
** there is a chance that the blocking connection will have already
** concluded its transaction by the time sqlite3_unlock_notify() is invoked.
** If this happens, then the specified callback is invoked immediately,
** from within the call to sqlite3_unlock_notify().)^
**
** ^If the blocked connection is attempting to obtain a write-lock on a
** shared-cache table, and more than one other connection currently holds
** a read-lock on the same table, then SQLite arbitrarily selects one of 
** the other connections to use as the blocking connection.
**
** ^(There may be at most one unlock-notify callback registered by a 
** blocked connection. If sqlite3_unlock_notify() is called when the
** blocked connection already has a registered unlock-notify callback,
** then the new callback replaces the old.)^ ^If sqlite3_unlock_notify() is
** called with a NULL pointer as its second argument, then any existing
** unlock-notify callback is canceled. ^The blocked connections 
** unlock-notify callback may also be canceled by closing the blocked
** connection using [sqlite3_close()].
**
** The unlock-notify callback is not reentrant. If an application invokes
** any sqlite3_xxx API functions from within an unlock-notify callback, a
** crash or deadlock may be the result.
**
** ^Unless deadlock is detected (see below), sqlite3_unlock_notify() always
** returns SQLITE_OK.
**
** <b>Callback Invocation Details</b>
**
** When an unlock-notify callback is registered, the application provides a 
** single void* pointer that is passed to the callback when it is invoked.
** However, the signature of the callback function allows SQLite to pass
** it an array of void* context pointers. The first argument passed to
** an unlock-notify callback is a pointer to an array of void* pointers,
** and the second is the number of entries in the array.
**
** When a blocking connections transaction is concluded, there may be
** more than one blocked connection that has registered for an unlock-notify
** callback. ^If two or more such blocked connections have specified the
** same callback function, then instead of invoking the callback function
** multiple times, it is invoked once with the set of void* context pointers
** specified by the blocked connections bundled together into an array.
** This gives the application an opportunity to prioritize any actions 
** related to the set of unblocked database connections.
**
** <b>Deadlock Detection</b>
**
** Assuming that after registering for an unlock-notify callback a 
** database waits for the callback to be issued before taking any further
** action (a reasonable assumption), then using this API may cause the
** application to deadlock. For example, if connection X is waiting for
** connection Y's transaction to be concluded, and similarly connection
** Y is waiting on connection X's transaction, then neither connection
** will proceed and the system may remain deadlocked indefinitely.
**
** To avoid this scenario, the sqlite3_unlock_notify() performs deadlock
** detection. ^If a given call to sqlite3_unlock_notify() would put the
** system in a deadlocked state, then SQLITE_LOCKED is returned and no
** unlock-notify callback is registered. The system is said to be in
** a deadlocked state if connection A has registered for an unlock-notify
** callback on the conclusion of connection B's transaction, and connection
** B has itself registered for an unlock-notify callback when connection
** A's transaction is concluded. ^Indirect deadlock is also detected, so
** the system is also considered to be deadlocked if connection B has
** registered for an unlock-notify callback on the conclusion of connection
** C's transaction, where connection C is waiting on connection A. ^Any
** number of levels of indirection are allowed.
**
** <b>The "DROP TABLE" Exception</b>
**
** When a call to [sqlite3_step()] returns SQLITE_LOCKED, it is almost 
** always appropriate to call sqlite3_unlock_notify(). There is however,
** one exception. When executing a "DROP TABLE" or "DROP INDEX" statement,
** SQLite checks if there are any currently executing SELECT statements
** that belong to the same connection. If there are, SQLITE_LOCKED is
** returned. In this case there is no "blocking connection", so invoking
** sqlite3_unlock_notify() results in the unlock-notify callback being
** invoked immediately. If the application then re-attempts the "DROP TABLE"
** or "DROP INDEX" query, an infinite loop might be the result.
**
** One way around this problem is to check the extended error code returned
** by an sqlite3_step() call. ^(If there is a blocking connection, then the
** extended error code is set to SQLITE_LOCKED_SHAREDCACHE. Otherwise, in
** the special "DROP TABLE/INDEX" case, the extended error code is just 
** SQLITE_LOCKED.)^
*/
SQLITE_API int SQLITE_STDCALL sqlite3_unlock_notify(
  sqlite3 *pBlocked,                          /* Waiting connection */
  void (*xNotify)(void **apArg, int nArg),    /* Callback function to invoke */
  void *pNotifyArg                            /* Argument to pass to xNotify */
);


/*
** CAPI3REF: String Comparison
**
** ^The [sqlite3_stricmp()] and [sqlite3_strnicmp()] APIs allow applications
** and extensions to compare the contents of two buffers containing UTF-8
** strings in a case-independent fashion, using the same definition of "case
** independence" that SQLite uses internally when comparing identifiers.
*/
SQLITE_API int SQLITE_STDCALL sqlite3_stricmp(const char *, const char *);
SQLITE_API int SQLITE_STDCALL sqlite3_strnicmp(const char *, const char *, int);

/*
** CAPI3REF: String Globbing
*
** ^The [sqlite3_strglob(P,X)] interface returns zero if and only if
** string X matches the [GLOB] pattern P.
** ^The definition of [GLOB] pattern matching used in
** [sqlite3_strglob(P,X)] is the same as for the "X GLOB P" operator in the
** SQL dialect understood by SQLite.  ^The [sqlite3_strglob(P,X)] function
** is case sensitive.
**
** Note that this routine returns zero on a match and non-zero if the strings
** do not match, the same as [sqlite3_stricmp()] and [sqlite3_strnicmp()].
**
** See also: [sqlite3_strlike()].
*/
SQLITE_API int SQLITE_STDCALL sqlite3_strglob(const char *zGlob, const char *zStr);

/*
** CAPI3REF: String LIKE Matching
*
** ^The [sqlite3_strlike(P,X,E)] interface returns zero if and only if
** string X matches the [LIKE] pattern P with escape character E.
** ^The definition of [LIKE] pattern matching used in
** [sqlite3_strlike(P,X,E)] is the same as for the "X LIKE P ESCAPE E"
** operator in the SQL dialect understood by SQLite.  ^For "X LIKE P" without
** the ESCAPE clause, set the E parameter of [sqlite3_strlike(P,X,E)] to 0.
** ^As with the LIKE operator, the [sqlite3_strlike(P,X,E)] function is case
** insensitive - equivalent upper and lower case ASCII characters match
** one another.
**
** ^The [sqlite3_strlike(P,X,E)] function matches Unicode characters, though
** only ASCII characters are case folded.
**
** Note that this routine returns zero on a match and non-zero if the strings
** do not match, the same as [sqlite3_stricmp()] and [sqlite3_strnicmp()].
**
** See also: [sqlite3_strglob()].
*/
SQLITE_API int SQLITE_STDCALL sqlite3_strlike(const char *zGlob, const char *zStr, unsigned int cEsc);

/*
** CAPI3REF: Error Logging Interface
**
** ^The [sqlite3_log()] interface writes a message into the [error log]
** established by the [SQLITE_CONFIG_LOG] option to [sqlite3_config()].
** ^If logging is enabled, the zFormat string and subsequent arguments are
** used with [sqlite3_snprintf()] to generate the final output string.
**
** The sqlite3_log() interface is intended for use by extensions such as
** virtual tables, collating functions, and SQL functions.  While there is
** nothing to prevent an application from calling sqlite3_log(), doing so
** is considered bad form.
**
** The zFormat string must not be NULL.
**
** To avoid deadlocks and other threading problems, the sqlite3_log() routine
** will not use dynamically allocated memory.  The log message is stored in
** a fixed-length buffer on the stack.  If the log message is longer than
** a few hundred characters, it will be truncated to the length of the
** buffer.
*/
SQLITE_API void SQLITE_CDECL sqlite3_log(int iErrCode, const char *zFormat, ...);

/*
** CAPI3REF: Write-Ahead Log Commit Hook
** METHOD: sqlite3
**
** ^The [sqlite3_wal_hook()] function is used to register a callback that
** is invoked each time data is committed to a database in wal mode.
**
** ^(The callback is invoked by SQLite after the commit has taken place and 
** the associated write-lock on the database released)^, so the implementation 
** may read, write or [checkpoint] the database as required.
**
** ^The first parameter passed to the callback function when it is invoked
** is a copy of the third parameter passed to sqlite3_wal_hook() when
** registering the callback. ^The second is a copy of the database handle.
** ^The third parameter is the name of the database that was written to -
** either "main" or the name of an [ATTACH]-ed database. ^The fourth parameter
** is the number of pages currently in the write-ahead log file,
** including those that were just committed.
**
** The callback function should normally return [SQLITE_OK].  ^If an error
** code is returned, that error will propagate back up through the
** SQLite code base to cause the statement that provoked the callback
** to report an error, though the commit will have still occurred. If the
** callback returns [SQLITE_ROW] or [SQLITE_DONE], or if it returns a value
** that does not correspond to any valid SQLite error code, the results
** are undefined.
**
** A single database handle may have at most a single write-ahead log callback 
** registered at one time. ^Calling [sqlite3_wal_hook()] replaces any
** previously registered write-ahead log callback. ^Note that the
** [sqlite3_wal_autocheckpoint()] interface and the
** [wal_autocheckpoint pragma] both invoke [sqlite3_wal_hook()] and will
** overwrite any prior [sqlite3_wal_hook()] settings.
*/
SQLITE_API void *SQLITE_STDCALL sqlite3_wal_hook(
  sqlite3*, 
  int(*)(void *,sqlite3*,const char*,int),
  void*
);

/*
** CAPI3REF: Configure an auto-checkpoint
** METHOD: sqlite3
**
** ^The [sqlite3_wal_autocheckpoint(D,N)] is a wrapper around
** [sqlite3_wal_hook()] that causes any database on [database connection] D
** to automatically [checkpoint]
** after committing a transaction if there are N or
** more frames in the [write-ahead log] file.  ^Passing zero or 
** a negative value as the nFrame parameter disables automatic
** checkpoints entirely.
**
** ^The callback registered by this function replaces any existing callback
** registered using [sqlite3_wal_hook()].  ^Likewise, registering a callback
** using [sqlite3_wal_hook()] disables the automatic checkpoint mechanism
** configured by this function.
**
** ^The [wal_autocheckpoint pragma] can be used to invoke this interface
** from SQL.
**
** ^Checkpoints initiated by this mechanism are
** [sqlite3_wal_checkpoint_v2|PASSIVE].
**
** ^Every new [database connection] defaults to having the auto-checkpoint
** enabled with a threshold of 1000 or [SQLITE_DEFAULT_WAL_AUTOCHECKPOINT]
** pages.  The use of this interface
** is only necessary if the default setting is found to be suboptimal
** for a particular application.
*/
SQLITE_API int SQLITE_STDCALL sqlite3_wal_autocheckpoint(sqlite3 *db, int N);

/*
** CAPI3REF: Checkpoint a database
** METHOD: sqlite3
**
** ^(The sqlite3_wal_checkpoint(D,X) is equivalent to
** [sqlite3_wal_checkpoint_v2](D,X,[SQLITE_CHECKPOINT_PASSIVE],0,0).)^
**
** In brief, sqlite3_wal_checkpoint(D,X) causes the content in the 
** [write-ahead log] for database X on [database connection] D to be
** transferred into the database file and for the write-ahead log to
** be reset.  See the [checkpointing] documentation for addition
** information.
**
** This interface used to be the only way to cause a checkpoint to
** occur.  But then the newer and more powerful [sqlite3_wal_checkpoint_v2()]
** interface was added.  This interface is retained for backwards
** compatibility and as a convenience for applications that need to manually
** start a callback but which do not need the full power (and corresponding
** complication) of [sqlite3_wal_checkpoint_v2()].
*/
SQLITE_API int SQLITE_STDCALL sqlite3_wal_checkpoint(sqlite3 *db, const char *zDb);

/*
** CAPI3REF: Checkpoint a database
** METHOD: sqlite3
**
** ^(The sqlite3_wal_checkpoint_v2(D,X,M,L,C) interface runs a checkpoint
** operation on database X of [database connection] D in mode M.  Status
** information is written back into integers pointed to by L and C.)^
** ^(The M parameter must be a valid [checkpoint mode]:)^
**
** <dl>
** <dt>SQLITE_CHECKPOINT_PASSIVE<dd>
**   ^Checkpoint as many frames as possible without waiting for any database 
**   readers or writers to finish, then sync the database file if all frames 
**   in the log were checkpointed. ^The [busy-handler callback]
**   is never invoked in the SQLITE_CHECKPOINT_PASSIVE mode.  
**   ^On the other hand, passive mode might leave the checkpoint unfinished
**   if there are concurrent readers or writers.
**
** <dt>SQLITE_CHECKPOINT_FULL<dd>
**   ^This mode blocks (it invokes the
**   [sqlite3_busy_handler|busy-handler callback]) until there is no
**   database writer and all readers are reading from the most recent database
**   snapshot. ^It then checkpoints all frames in the log file and syncs the
**   database file. ^This mode blocks new database writers while it is pending,
**   but new database readers are allowed to continue unimpeded.
**
** <dt>SQLITE_CHECKPOINT_RESTART<dd>
**   ^This mode works the same way as SQLITE_CHECKPOINT_FULL with the addition
**   that after checkpointing the log file it blocks (calls the 
**   [busy-handler callback])
**   until all readers are reading from the database file only. ^This ensures 
**   that the next writer will restart the log file from the beginning.
**   ^Like SQLITE_CHECKPOINT_FULL, this mode blocks new
**   database writer attempts while it is pending, but does not impede readers.
**
** <dt>SQLITE_CHECKPOINT_TRUNCATE<dd>
**   ^This mode works the same way as SQLITE_CHECKPOINT_RESTART with the
**   addition that it also truncates the log file to zero bytes just prior
**   to a successful return.
** </dl>
**
** ^If pnLog is not NULL, then *pnLog is set to the total number of frames in
** the log file or to -1 if the checkpoint could not run because
** of an error or because the database is not in [WAL mode]. ^If pnCkpt is not
** NULL,then *pnCkpt is set to the total number of checkpointed frames in the
** log file (including any that were already checkpointed before the function
** was called) or to -1 if the checkpoint could not run due to an error or
** because the database is not in WAL mode. ^Note that upon successful
** completion of an SQLITE_CHECKPOINT_TRUNCATE, the log file will have been
** truncated to zero bytes and so both *pnLog and *pnCkpt will be set to zero.
**
** ^All calls obtain an exclusive "checkpoint" lock on the database file. ^If
** any other process is running a checkpoint operation at the same time, the 
** lock cannot be obtained and SQLITE_BUSY is returned. ^Even if there is a 
** busy-handler configured, it will not be invoked in this case.
**
** ^The SQLITE_CHECKPOINT_FULL, RESTART and TRUNCATE modes also obtain the 
** exclusive "writer" lock on the database file. ^If the writer lock cannot be
** obtained immediately, and a busy-handler is configured, it is invoked and
** the writer lock retried until either the busy-handler returns 0 or the lock
** is successfully obtained. ^The busy-handler is also invoked while waiting for
** database readers as described above. ^If the busy-handler returns 0 before
** the writer lock is obtained or while waiting for database readers, the
** checkpoint operation proceeds from that point in the same way as 
** SQLITE_CHECKPOINT_PASSIVE - checkpointing as many frames as possible 
** without blocking any further. ^SQLITE_BUSY is returned in this case.
**
** ^If parameter zDb is NULL or points to a zero length string, then the
** specified operation is attempted on all WAL databases [attached] to 
** [database connection] db.  In this case the
** values written to output parameters *pnLog and *pnCkpt are undefined. ^If 
** an SQLITE_BUSY error is encountered when processing one or more of the 
** attached WAL databases, the operation is still attempted on any remaining 
** attached databases and SQLITE_BUSY is returned at the end. ^If any other 
** error occurs while processing an attached database, processing is abandoned 
** and the error code is returned to the caller immediately. ^If no error 
** (SQLITE_BUSY or otherwise) is encountered while processing the attached 
** databases, SQLITE_OK is returned.
**
** ^If database zDb is the name of an attached database that is not in WAL
** mode, SQLITE_OK is returned and both *pnLog and *pnCkpt set to -1. ^If
** zDb is not NULL (or a zero length string) and is not the name of any
** attached database, SQLITE_ERROR is returned to the caller.
**
** ^Unless it returns SQLITE_MISUSE,
** the sqlite3_wal_checkpoint_v2() interface
** sets the error information that is queried by
** [sqlite3_errcode()] and [sqlite3_errmsg()].
**
** ^The [PRAGMA wal_checkpoint] command can be used to invoke this interface
** from SQL.
*/
SQLITE_API int SQLITE_STDCALL sqlite3_wal_checkpoint_v2(
  sqlite3 *db,                    /* Database handle */
  const char *zDb,                /* Name of attached database (or NULL) */
  int eMode,                      /* SQLITE_CHECKPOINT_* value */
  int *pnLog,                     /* OUT: Size of WAL log in frames */
  int *pnCkpt                     /* OUT: Total number of frames checkpointed */
);

/*
** CAPI3REF: Checkpoint Mode Values
** KEYWORDS: {checkpoint mode}
**
** These constants define all valid values for the "checkpoint mode" passed
** as the third parameter to the [sqlite3_wal_checkpoint_v2()] interface.
** See the [sqlite3_wal_checkpoint_v2()] documentation for details on the
** meaning of each of these checkpoint modes.
*/
#define SQLITE_CHECKPOINT_PASSIVE  0  /* Do as much as possible w/o blocking */
#define SQLITE_CHECKPOINT_FULL     1  /* Wait for writers, then checkpoint */
#define SQLITE_CHECKPOINT_RESTART  2  /* Like FULL but wait for for readers */
#define SQLITE_CHECKPOINT_TRUNCATE 3  /* Like RESTART but also truncate WAL */

/*
** CAPI3REF: Virtual Table Interface Configuration
**
** This function may be called by either the [xConnect] or [xCreate] method
** of a [virtual table] implementation to configure
** various facets of the virtual table interface.
**
** If this interface is invoked outside the context of an xConnect or
** xCreate virtual table method then the behavior is undefined.
**
** At present, there is only one option that may be configured using
** this function. (See [SQLITE_VTAB_CONSTRAINT_SUPPORT].)  Further options
** may be added in the future.
*/
SQLITE_API int SQLITE_CDECL sqlite3_vtab_config(sqlite3*, int op, ...);

/*
** CAPI3REF: Virtual Table Configuration Options
**
** These macros define the various options to the
** [sqlite3_vtab_config()] interface that [virtual table] implementations
** can use to customize and optimize their behavior.
**
** <dl>
** <dt>SQLITE_VTAB_CONSTRAINT_SUPPORT
** <dd>Calls of the form
** [sqlite3_vtab_config](db,SQLITE_VTAB_CONSTRAINT_SUPPORT,X) are supported,
** where X is an integer.  If X is zero, then the [virtual table] whose
** [xCreate] or [xConnect] method invoked [sqlite3_vtab_config()] does not
** support constraints.  In this configuration (which is the default) if
** a call to the [xUpdate] method returns [SQLITE_CONSTRAINT], then the entire
** statement is rolled back as if [ON CONFLICT | OR ABORT] had been
** specified as part of the users SQL statement, regardless of the actual
** ON CONFLICT mode specified.
**
** If X is non-zero, then the virtual table implementation guarantees
** that if [xUpdate] returns [SQLITE_CONSTRAINT], it will do so before
** any modifications to internal or persistent data structures have been made.
** If the [ON CONFLICT] mode is ABORT, FAIL, IGNORE or ROLLBACK, SQLite 
** is able to roll back a statement or database transaction, and abandon
** or continue processing the current SQL statement as appropriate. 
** If the ON CONFLICT mode is REPLACE and the [xUpdate] method returns
** [SQLITE_CONSTRAINT], SQLite handles this as if the ON CONFLICT mode
** had been ABORT.
**
** Virtual table implementations that are required to handle OR REPLACE
** must do so within the [xUpdate] method. If a call to the 
** [sqlite3_vtab_on_conflict()] function indicates that the current ON 
** CONFLICT policy is REPLACE, the virtual table implementation should 
** silently replace the appropriate rows within the xUpdate callback and
** return SQLITE_OK. Or, if this is not possible, it may return
** SQLITE_CONSTRAINT, in which case SQLite falls back to OR ABORT 
** constraint handling.
** </dl>
*/
#define SQLITE_VTAB_CONSTRAINT_SUPPORT 1

/*
** CAPI3REF: Determine The Virtual Table Conflict Policy
**
** This function may only be called from within a call to the [xUpdate] method
** of a [virtual table] implementation for an INSERT or UPDATE operation. ^The
** value returned is one of [SQLITE_ROLLBACK], [SQLITE_IGNORE], [SQLITE_FAIL],
** [SQLITE_ABORT], or [SQLITE_REPLACE], according to the [ON CONFLICT] mode
** of the SQL statement that triggered the call to the [xUpdate] method of the
** [virtual table].
*/
SQLITE_API int SQLITE_STDCALL sqlite3_vtab_on_conflict(sqlite3 *);

/*
** CAPI3REF: Conflict resolution modes
** KEYWORDS: {conflict resolution mode}
**
** These constants are returned by [sqlite3_vtab_on_conflict()] to
** inform a [virtual table] implementation what the [ON CONFLICT] mode
** is for the SQL statement being evaluated.
**
** Note that the [SQLITE_IGNORE] constant is also used as a potential
** return value from the [sqlite3_set_authorizer()] callback and that
** [SQLITE_ABORT] is also a [result code].
*/
#define SQLITE_ROLLBACK 1
/* #define SQLITE_IGNORE 2 // Also used by sqlite3_authorizer() callback */
#define SQLITE_FAIL     3
/* #define SQLITE_ABORT 4  // Also an error code */
#define SQLITE_REPLACE  5

/*
** CAPI3REF: Prepared Statement Scan Status Opcodes
** KEYWORDS: {scanstatus options}
**
** The following constants can be used for the T parameter to the
** [sqlite3_stmt_scanstatus(S,X,T,V)] interface.  Each constant designates a
** different metric for sqlite3_stmt_scanstatus() to return.
**
** When the value returned to V is a string, space to hold that string is
** managed by the prepared statement S and will be automatically freed when
** S is finalized.
**
** <dl>
** [[SQLITE_SCANSTAT_NLOOP]] <dt>SQLITE_SCANSTAT_NLOOP</dt>
** <dd>^The [sqlite3_int64] variable pointed to by the T parameter will be
** set to the total number of times that the X-th loop has run.</dd>
**
** [[SQLITE_SCANSTAT_NVISIT]] <dt>SQLITE_SCANSTAT_NVISIT</dt>
** <dd>^The [sqlite3_int64] variable pointed to by the T parameter will be set
** to the total number of rows examined by all iterations of the X-th loop.</dd>
**
** [[SQLITE_SCANSTAT_EST]] <dt>SQLITE_SCANSTAT_EST</dt>
** <dd>^The "double" variable pointed to by the T parameter will be set to the
** query planner's estimate for the average number of rows output from each
** iteration of the X-th loop.  If the query planner's estimates was accurate,
** then this value will approximate the quotient NVISIT/NLOOP and the
** product of this value for all prior loops with the same SELECTID will
** be the NLOOP value for the current loop.
**
** [[SQLITE_SCANSTAT_NAME]] <dt>SQLITE_SCANSTAT_NAME</dt>
** <dd>^The "const char *" variable pointed to by the T parameter will be set
** to a zero-terminated UTF-8 string containing the name of the index or table
** used for the X-th loop.
**
** [[SQLITE_SCANSTAT_EXPLAIN]] <dt>SQLITE_SCANSTAT_EXPLAIN</dt>
** <dd>^The "const char *" variable pointed to by the T parameter will be set
** to a zero-terminated UTF-8 string containing the [EXPLAIN QUERY PLAN]
** description for the X-th loop.
**
** [[SQLITE_SCANSTAT_SELECTID]] <dt>SQLITE_SCANSTAT_SELECT</dt>
** <dd>^The "int" variable pointed to by the T parameter will be set to the
** "select-id" for the X-th loop.  The select-id identifies which query or
** subquery the loop is part of.  The main query has a select-id of zero.
** The select-id is the same value as is output in the first column
** of an [EXPLAIN QUERY PLAN] query.
** </dl>
*/
#define SQLITE_SCANSTAT_NLOOP    0
#define SQLITE_SCANSTAT_NVISIT   1
#define SQLITE_SCANSTAT_EST      2
#define SQLITE_SCANSTAT_NAME     3
#define SQLITE_SCANSTAT_EXPLAIN  4
#define SQLITE_SCANSTAT_SELECTID 5

/*
** CAPI3REF: Prepared Statement Scan Status
** METHOD: sqlite3_stmt
**
** This interface returns information about the predicted and measured
** performance for pStmt.  Advanced applications can use this
** interface to compare the predicted and the measured performance and
** issue warnings and/or rerun [ANALYZE] if discrepancies are found.
**
** Since this interface is expected to be rarely used, it is only
** available if SQLite is compiled using the [SQLITE_ENABLE_STMT_SCANSTATUS]
** compile-time option.
**
** The "iScanStatusOp" parameter determines which status information to return.
** The "iScanStatusOp" must be one of the [scanstatus options] or the behavior
** of this interface is undefined.
** ^The requested measurement is written into a variable pointed to by
** the "pOut" parameter.
** Parameter "idx" identifies the specific loop to retrieve statistics for.
** Loops are numbered starting from zero. ^If idx is out of range - less than
** zero or greater than or equal to the total number of loops used to implement
** the statement - a non-zero value is returned and the variable that pOut
** points to is unchanged.
**
** ^Statistics might not be available for all loops in all statements. ^In cases
** where there exist loops with no available statistics, this function behaves
** as if the loop did not exist - it returns non-zero and leave the variable
** that pOut points to unchanged.
**
** See also: [sqlite3_stmt_scanstatus_reset()]
*/
SQLITE_API int SQLITE_STDCALL sqlite3_stmt_scanstatus(
  sqlite3_stmt *pStmt,      /* Prepared statement for which info desired */
  int idx,                  /* Index of loop to report on */
  int iScanStatusOp,        /* Information desired.  SQLITE_SCANSTAT_* */
  void *pOut                /* Result written here */
);     

/*
** CAPI3REF: Zero Scan-Status Counters
** METHOD: sqlite3_stmt
**
** ^Zero all [sqlite3_stmt_scanstatus()] related event counters.
**
** This API is only available if the library is built with pre-processor
** symbol [SQLITE_ENABLE_STMT_SCANSTATUS] defined.
*/
SQLITE_API void SQLITE_STDCALL sqlite3_stmt_scanstatus_reset(sqlite3_stmt*);

/*
** CAPI3REF: Flush caches to disk mid-transaction
**
** ^If a write-transaction is open on [database connection] D when the
** [sqlite3_db_cacheflush(D)] interface invoked, any dirty
** pages in the pager-cache that are not currently in use are written out 
** to disk. A dirty page may be in use if a database cursor created by an
** active SQL statement is reading from it, or if it is page 1 of a database
** file (page 1 is always "in use").  ^The [sqlite3_db_cacheflush(D)]
** interface flushes caches for all schemas - "main", "temp", and
** any [attached] databases.
**
** ^If this function needs to obtain extra database locks before dirty pages 
** can be flushed to disk, it does so. ^If those locks cannot be obtained 
** immediately and there is a busy-handler callback configured, it is invoked
** in the usual manner. ^If the required lock still cannot be obtained, then
** the database is skipped and an attempt made to flush any dirty pages
** belonging to the next (if any) database. ^If any databases are skipped
** because locks cannot be obtained, but no other error occurs, this
** function returns SQLITE_BUSY.
**
** ^If any other error occurs while flushing dirty pages to disk (for
** example an IO error or out-of-memory condition), then processing is
** abandoned and an SQLite [error code] is returned to the caller immediately.
**
** ^Otherwise, if no error occurs, [sqlite3_db_cacheflush()] returns SQLITE_OK.
**
** ^This function does not set the database handle error code or message
** returned by the [sqlite3_errcode()] and [sqlite3_errmsg()] functions.
*/
SQLITE_API int SQLITE_STDCALL sqlite3_db_cacheflush(sqlite3*);

/*
** CAPI3REF: The pre-update hook.
**
** ^These interfaces are only available if SQLite is compiled using the
** [SQLITE_ENABLE_PREUPDATE_HOOK] compile-time option.
**
** ^The [sqlite3_preupdate_hook()] interface registers a callback function
** that is invoked prior to each [INSERT], [UPDATE], and [DELETE] operation
** on a [rowid table].
** ^At most one preupdate hook may be registered at a time on a single
** [database connection]; each call to [sqlite3_preupdate_hook()] overrides
** the previous setting.
** ^The preupdate hook is disabled by invoking [sqlite3_preupdate_hook()]
** with a NULL pointer as the second parameter.
** ^The third parameter to [sqlite3_preupdate_hook()] is passed through as
** the first parameter to callbacks.
**
** ^The preupdate hook only fires for changes to [rowid tables]; the preupdate
** hook is not invoked for changes to [virtual tables] or [WITHOUT ROWID]
** tables.
**
** ^The second parameter to the preupdate callback is a pointer to
** the [database connection] that registered the preupdate hook.
** ^The third parameter to the preupdate callback is one of the constants
** [SQLITE_INSERT], [SQLITE_DELETE], or [SQLITE_UPDATE] to indentify the
** kind of update operation that is about to occur.
** ^(The fourth parameter to the preupdate callback is the name of the
** database within the database connection that is being modified.  This
** will be "main" for the main database or "temp" for TEMP tables or 
** the name given after the AS keyword in the [ATTACH] statement for attached
** databases.)^
** ^The fifth parameter to the preupdate callback is the name of the
** table that is being modified.
** ^The sixth parameter to the preupdate callback is the initial [rowid] of the
** row being changes for SQLITE_UPDATE and SQLITE_DELETE changes and is
** undefined for SQLITE_INSERT changes.
** ^The seventh parameter to the preupdate callback is the final [rowid] of
** the row being changed for SQLITE_UPDATE and SQLITE_INSERT changes and is
** undefined for SQLITE_DELETE changes.
**
** The [sqlite3_preupdate_old()], [sqlite3_preupdate_new()],
** [sqlite3_preupdate_count()], and [sqlite3_preupdate_depth()] interfaces
** provide additional information about a preupdate event. These routines
** may only be called from within a preupdate callback.  Invoking any of
** these routines from outside of a preupdate callback or with a
** [database connection] pointer that is different from the one supplied
** to the preupdate callback results in undefined and probably undesirable
** behavior.
**
** ^The [sqlite3_preupdate_count(D)] interface returns the number of columns
** in the row that is being inserted, updated, or deleted.
**
** ^The [sqlite3_preupdate_old(D,N,P)] interface writes into P a pointer to
** a [protected sqlite3_value] that contains the value of the Nth column of
** the table row before it is updated.  The N parameter must be between 0
** and one less than the number of columns or the behavior will be
** undefined. This must only be used within SQLITE_UPDATE and SQLITE_DELETE
** preupdate callbacks; if it is used by an SQLITE_INSERT callback then the
** behavior is undefined.  The [sqlite3_value] that P points to
** will be destroyed when the preupdate callback returns.
**
** ^The [sqlite3_preupdate_new(D,N,P)] interface writes into P a pointer to
** a [protected sqlite3_value] that contains the value of the Nth column of
** the table row after it is updated.  The N parameter must be between 0
** and one less than the number of columns or the behavior will be
** undefined. This must only be used within SQLITE_INSERT and SQLITE_UPDATE
** preupdate callbacks; if it is used by an SQLITE_DELETE callback then the
** behavior is undefined.  The [sqlite3_value] that P points to
** will be destroyed when the preupdate callback returns.
**
** ^The [sqlite3_preupdate_depth(D)] interface returns 0 if the preupdate
** callback was invoked as a result of a direct insert, update, or delete
** operation; or 1 for inserts, updates, or deletes invoked by top-level 
** triggers; or 2 for changes resulting from triggers called by top-level
** triggers; and so forth.
**
** See also:  [sqlite3_update_hook()]
*/
SQLITE_API SQLITE_EXPERIMENTAL void *SQLITE_STDCALL sqlite3_preupdate_hook(
  sqlite3 *db,
  void(*xPreUpdate)(
    void *pCtx,                   /* Copy of third arg to preupdate_hook() */
    sqlite3 *db,                  /* Database handle */
    int op,                       /* SQLITE_UPDATE, DELETE or INSERT */
    char const *zDb,              /* Database name */
    char const *zName,            /* Table name */
    sqlite3_int64 iKey1,          /* Rowid of row about to be deleted/updated */
    sqlite3_int64 iKey2           /* New rowid value (for a rowid UPDATE) */
  ),
  void*
);
SQLITE_API SQLITE_EXPERIMENTAL int SQLITE_STDCALL sqlite3_preupdate_old(sqlite3 *, int, sqlite3_value **);
SQLITE_API SQLITE_EXPERIMENTAL int SQLITE_STDCALL sqlite3_preupdate_count(sqlite3 *);
SQLITE_API SQLITE_EXPERIMENTAL int SQLITE_STDCALL sqlite3_preupdate_depth(sqlite3 *);
SQLITE_API SQLITE_EXPERIMENTAL int SQLITE_STDCALL sqlite3_preupdate_new(sqlite3 *, int, sqlite3_value **);

/*
** CAPI3REF: Low-level system error code
**
** ^Attempt to return the underlying operating system error code or error
** number that caused the most recent I/O error or failure to open a file.
** The return value is OS-dependent.  For example, on unix systems, after
** [sqlite3_open_v2()] returns [SQLITE_CANTOPEN], this interface could be
** called to get back the underlying "errno" that caused the problem, such
** as ENOSPC, EAUTH, EISDIR, and so forth.  
*/
SQLITE_API int SQLITE_STDCALL sqlite3_system_errno(sqlite3*);

/*
** CAPI3REF: Database Snapshot
** KEYWORDS: {snapshot}
** EXPERIMENTAL
**
** An instance of the snapshot object records the state of a [WAL mode]
** database for some specific point in history.
**
** In [WAL mode], multiple [database connections] that are open on the
** same database file can each be reading a different historical version
** of the database file.  When a [database connection] begins a read
** transaction, that connection sees an unchanging copy of the database
** as it existed for the point in time when the transaction first started.
** Subsequent changes to the database from other connections are not seen
** by the reader until a new read transaction is started.
**
** The sqlite3_snapshot object records state information about an historical
** version of the database file so that it is possible to later open a new read
** transaction that sees that historical version of the database rather than
** the most recent version.
**
** The constructor for this object is [sqlite3_snapshot_get()].  The
** [sqlite3_snapshot_open()] method causes a fresh read transaction to refer
** to an historical snapshot (if possible).  The destructor for 
** sqlite3_snapshot objects is [sqlite3_snapshot_free()].
*/
typedef struct sqlite3_snapshot sqlite3_snapshot;

/*
** CAPI3REF: Record A Database Snapshot
** EXPERIMENTAL
**
** ^The [sqlite3_snapshot_get(D,S,P)] interface attempts to make a
** new [sqlite3_snapshot] object that records the current state of
** schema S in database connection D.  ^On success, the
** [sqlite3_snapshot_get(D,S,P)] interface writes a pointer to the newly
** created [sqlite3_snapshot] object into *P and returns SQLITE_OK.
** ^If schema S of [database connection] D is not a [WAL mode] database
** that is in a read transaction, then [sqlite3_snapshot_get(D,S,P)]
** leaves the *P value unchanged and returns an appropriate [error code].
**
** The [sqlite3_snapshot] object returned from a successful call to
** [sqlite3_snapshot_get()] must be freed using [sqlite3_snapshot_free()]
** to avoid a memory leak.
**
** The [sqlite3_snapshot_get()] interface is only available when the
** SQLITE_ENABLE_SNAPSHOT compile-time option is used.
*/
SQLITE_API SQLITE_EXPERIMENTAL int SQLITE_STDCALL sqlite3_snapshot_get(
  sqlite3 *db,
  const char *zSchema,
  sqlite3_snapshot **ppSnapshot
);

/*
** CAPI3REF: Start a read transaction on an historical snapshot
** EXPERIMENTAL
**
** ^The [sqlite3_snapshot_open(D,S,P)] interface starts a
** read transaction for schema S of
** [database connection] D such that the read transaction
** refers to historical [snapshot] P, rather than the most
** recent change to the database.
** ^The [sqlite3_snapshot_open()] interface returns SQLITE_OK on success
** or an appropriate [error code] if it fails.
**
** ^In order to succeed, a call to [sqlite3_snapshot_open(D,S,P)] must be
** the first operation following the [BEGIN] that takes the schema S
** out of [autocommit mode].
** ^In other words, schema S must not currently be in
** a transaction for [sqlite3_snapshot_open(D,S,P)] to work, but the
** database connection D must be out of [autocommit mode].
** ^A [snapshot] will fail to open if it has been overwritten by a
** [checkpoint].
** ^(A call to [sqlite3_snapshot_open(D,S,P)] will fail if the
** database connection D does not know that the database file for
** schema S is in [WAL mode].  A database connection might not know
** that the database file is in [WAL mode] if there has been no prior
** I/O on that database connection, or if the database entered [WAL mode] 
** after the most recent I/O on the database connection.)^
** (Hint: Run "[PRAGMA application_id]" against a newly opened
** database connection in order to make it ready to use snapshots.)
**
** The [sqlite3_snapshot_open()] interface is only available when the
** SQLITE_ENABLE_SNAPSHOT compile-time option is used.
*/
SQLITE_API SQLITE_EXPERIMENTAL int SQLITE_STDCALL sqlite3_snapshot_open(
  sqlite3 *db,
  const char *zSchema,
  sqlite3_snapshot *pSnapshot
);

/*
** CAPI3REF: Destroy a snapshot
** EXPERIMENTAL
**
** ^The [sqlite3_snapshot_free(P)] interface destroys [sqlite3_snapshot] P.
** The application must eventually free every [sqlite3_snapshot] object
** using this routine to avoid a memory leak.
**
** The [sqlite3_snapshot_free()] interface is only available when the
** SQLITE_ENABLE_SNAPSHOT compile-time option is used.
*/
SQLITE_API SQLITE_EXPERIMENTAL void SQLITE_STDCALL sqlite3_snapshot_free(sqlite3_snapshot*);

/*
** CAPI3REF: Compare the ages of two snapshot handles.
** EXPERIMENTAL
**
** The sqlite3_snapshot_cmp(P1, P2) interface is used to compare the ages
** of two valid snapshot handles. 
**
** If the two snapshot handles are not associated with the same database 
** file, the result of the comparison is undefined. 
**
** Additionally, the result of the comparison is only valid if both of the
** snapshot handles were obtained by calling sqlite3_snapshot_get() since the
** last time the wal file was deleted. The wal file is deleted when the
** database is changed back to rollback mode or when the number of database
** clients drops to zero. If either snapshot handle was obtained before the 
** wal file was last deleted, the value returned by this function 
** is undefined.
**
** Otherwise, this API returns a negative value if P1 refers to an older
** snapshot than P2, zero if the two handles refer to the same database
** snapshot, and a positive value if P1 is a newer snapshot than P2.
*/
SQLITE_API SQLITE_EXPERIMENTAL int SQLITE_STDCALL sqlite3_snapshot_cmp(
  sqlite3_snapshot *p1,
  sqlite3_snapshot *p2
);

/*
** Undo the hack that converts floating point types to integer for
** builds on processors without floating point support.
*/
#ifdef SQLITE_OMIT_FLOATING_POINT
# undef double
#endif

#if 0
}  /* End of the 'extern "C"' block */
#endif
#endif /* _SQLITE3_H_ */

/******** Begin file sqlite3rtree.h *********/
/*
** 2010 August 30
**
** The author disclaims copyright to this source code.  In place of
** a legal notice, here is a blessing:
**
**    May you do good and not evil.
**    May you find forgiveness for yourself and forgive others.
**    May you share freely, never taking more than you give.
**
*************************************************************************
*/

#ifndef _SQLITE3RTREE_H_
#define _SQLITE3RTREE_H_


#if 0
extern "C" {
#endif

typedef struct sqlite3_rtree_geometry sqlite3_rtree_geometry;
typedef struct sqlite3_rtree_query_info sqlite3_rtree_query_info;

/* The double-precision datatype used by RTree depends on the
** SQLITE_RTREE_INT_ONLY compile-time option.
*/
#ifdef SQLITE_RTREE_INT_ONLY
  typedef sqlite3_int64 sqlite3_rtree_dbl;
#else
  typedef double sqlite3_rtree_dbl;
#endif

/*
** Register a geometry callback named zGeom that can be used as part of an
** R-Tree geometry query as follows:
**
**   SELECT ... FROM <rtree> WHERE <rtree col> MATCH $zGeom(... params ...)
*/
SQLITE_API int SQLITE_STDCALL sqlite3_rtree_geometry_callback(
  sqlite3 *db,
  const char *zGeom,
  int (*xGeom)(sqlite3_rtree_geometry*, int, sqlite3_rtree_dbl*,int*),
  void *pContext
);


/*
** A pointer to a structure of the following type is passed as the first
** argument to callbacks registered using rtree_geometry_callback().
*/
struct sqlite3_rtree_geometry {
  void *pContext;                 /* Copy of pContext passed to s_r_g_c() */
  int nParam;                     /* Size of array aParam[] */
  sqlite3_rtree_dbl *aParam;      /* Parameters passed to SQL geom function */
  void *pUser;                    /* Callback implementation user data */
  void (*xDelUser)(void *);       /* Called by SQLite to clean up pUser */
};

/*
** Register a 2nd-generation geometry callback named zScore that can be 
** used as part of an R-Tree geometry query as follows:
**
**   SELECT ... FROM <rtree> WHERE <rtree col> MATCH $zQueryFunc(... params ...)
*/
SQLITE_API int SQLITE_STDCALL sqlite3_rtree_query_callback(
  sqlite3 *db,
  const char *zQueryFunc,
  int (*xQueryFunc)(sqlite3_rtree_query_info*),
  void *pContext,
  void (*xDestructor)(void*)
);


/*
** A pointer to a structure of the following type is passed as the 
** argument to scored geometry callback registered using
** sqlite3_rtree_query_callback().
**
** Note that the first 5 fields of this structure are identical to
** sqlite3_rtree_geometry.  This structure is a subclass of
** sqlite3_rtree_geometry.
*/
struct sqlite3_rtree_query_info {
  void *pContext;                   /* pContext from when function registered */
  int nParam;                       /* Number of function parameters */
  sqlite3_rtree_dbl *aParam;        /* value of function parameters */
  void *pUser;                      /* callback can use this, if desired */
  void (*xDelUser)(void*);          /* function to free pUser */
  sqlite3_rtree_dbl *aCoord;        /* Coordinates of node or entry to check */
  unsigned int *anQueue;            /* Number of pending entries in the queue */
  int nCoord;                       /* Number of coordinates */
  int iLevel;                       /* Level of current node or entry */
  int mxLevel;                      /* The largest iLevel value in the tree */
  sqlite3_int64 iRowid;             /* Rowid for current entry */
  sqlite3_rtree_dbl rParentScore;   /* Score of parent node */
  int eParentWithin;                /* Visibility of parent node */
  int eWithin;                      /* OUT: Visiblity */
  sqlite3_rtree_dbl rScore;         /* OUT: Write the score here */
  /* The following fields are only available in 3.8.11 and later */
  sqlite3_value **apSqlParam;       /* Original SQL values of parameters */
};

/*
** Allowed values for sqlite3_rtree_query.eWithin and .eParentWithin.
*/
#define NOT_WITHIN       0   /* Object completely outside of query region */
#define PARTLY_WITHIN    1   /* Object partially overlaps query region */
#define FULLY_WITHIN     2   /* Object fully contained within query region */


#if 0
}  /* end of the 'extern "C"' block */
#endif

#endif  /* ifndef _SQLITE3RTREE_H_ */

/******** End of sqlite3rtree.h *********/
/******** Begin file sqlite3session.h *********/

#if !defined(__SQLITESESSION_H_) && defined(SQLITE_ENABLE_SESSION)
#define __SQLITESESSION_H_ 1

/*
** Make sure we can call this stuff from C++.
*/
#if 0
extern "C" {
#endif


/*
** CAPI3REF: Session Object Handle
*/
typedef struct sqlite3_session sqlite3_session;

/*
** CAPI3REF: Changeset Iterator Handle
*/
typedef struct sqlite3_changeset_iter sqlite3_changeset_iter;

/*
** CAPI3REF: Create A New Session Object
**
** Create a new session object attached to database handle db. If successful,
** a pointer to the new object is written to *ppSession and SQLITE_OK is
** returned. If an error occurs, *ppSession is set to NULL and an SQLite
** error code (e.g. SQLITE_NOMEM) is returned.
**
** It is possible to create multiple session objects attached to a single
** database handle.
**
** Session objects created using this function should be deleted using the
** [sqlite3session_delete()] function before the database handle that they
** are attached to is itself closed. If the database handle is closed before
** the session object is deleted, then the results of calling any session
** module function, including [sqlite3session_delete()] on the session object
** are undefined.
**
** Because the session module uses the [sqlite3_preupdate_hook()] API, it
** is not possible for an application to register a pre-update hook on a
** database handle that has one or more session objects attached. Nor is
** it possible to create a session object attached to a database handle for
** which a pre-update hook is already defined. The results of attempting 
** either of these things are undefined.
**
** The session object will be used to create changesets for tables in
** database zDb, where zDb is either "main", or "temp", or the name of an
** attached database. It is not an error if database zDb is not attached
** to the database when the session object is created.
*/
int sqlite3session_create(
  sqlite3 *db,                    /* Database handle */
  const char *zDb,                /* Name of db (e.g. "main") */
  sqlite3_session **ppSession     /* OUT: New session object */
);

/*
** CAPI3REF: Delete A Session Object
**
** Delete a session object previously allocated using 
** [sqlite3session_create()]. Once a session object has been deleted, the
** results of attempting to use pSession with any other session module
** function are undefined.
**
** Session objects must be deleted before the database handle to which they
** are attached is closed. Refer to the documentation for 
** [sqlite3session_create()] for details.
*/
void sqlite3session_delete(sqlite3_session *pSession);


/*
** CAPI3REF: Enable Or Disable A Session Object
**
** Enable or disable the recording of changes by a session object. When
** enabled, a session object records changes made to the database. When
** disabled - it does not. A newly created session object is enabled.
** Refer to the documentation for [sqlite3session_changeset()] for further
** details regarding how enabling and disabling a session object affects
** the eventual changesets.
**
** Passing zero to this function disables the session. Passing a value
** greater than zero enables it. Passing a value less than zero is a 
** no-op, and may be used to query the current state of the session.
**
** The return value indicates the final state of the session object: 0 if 
** the session is disabled, or 1 if it is enabled.
*/
int sqlite3session_enable(sqlite3_session *pSession, int bEnable);

/*
** CAPI3REF: Set Or Clear the Indirect Change Flag
**
** Each change recorded by a session object is marked as either direct or
** indirect. A change is marked as indirect if either:
**
** <ul>
**   <li> The session object "indirect" flag is set when the change is
**        made, or
**   <li> The change is made by an SQL trigger or foreign key action 
**        instead of directly as a result of a users SQL statement.
** </ul>
**
** If a single row is affected by more than one operation within a session,
** then the change is considered indirect if all operations meet the criteria
** for an indirect change above, or direct otherwise.
**
** This function is used to set, clear or query the session object indirect
** flag.  If the second argument passed to this function is zero, then the
** indirect flag is cleared. If it is greater than zero, the indirect flag
** is set. Passing a value less than zero does not modify the current value
** of the indirect flag, and may be used to query the current state of the 
** indirect flag for the specified session object.
**
** The return value indicates the final state of the indirect flag: 0 if 
** it is clear, or 1 if it is set.
*/
int sqlite3session_indirect(sqlite3_session *pSession, int bIndirect);

/*
** CAPI3REF: Attach A Table To A Session Object
**
** If argument zTab is not NULL, then it is the name of a table to attach
** to the session object passed as the first argument. All subsequent changes 
** made to the table while the session object is enabled will be recorded. See 
** documentation for [sqlite3session_changeset()] for further details.
**
** Or, if argument zTab is NULL, then changes are recorded for all tables
** in the database. If additional tables are added to the database (by 
** executing "CREATE TABLE" statements) after this call is made, changes for 
** the new tables are also recorded.
**
** Changes can only be recorded for tables that have a PRIMARY KEY explicitly
** defined as part of their CREATE TABLE statement. It does not matter if the 
** PRIMARY KEY is an "INTEGER PRIMARY KEY" (rowid alias) or not. The PRIMARY
** KEY may consist of a single column, or may be a composite key.
** 
** It is not an error if the named table does not exist in the database. Nor
** is it an error if the named table does not have a PRIMARY KEY. However,
** no changes will be recorded in either of these scenarios.
**
** Changes are not recorded for individual rows that have NULL values stored
** in one or more of their PRIMARY KEY columns.
**
** SQLITE_OK is returned if the call completes without error. Or, if an error 
** occurs, an SQLite error code (e.g. SQLITE_NOMEM) is returned.
*/
int sqlite3session_attach(
  sqlite3_session *pSession,      /* Session object */
  const char *zTab                /* Table name */
);

/*
** CAPI3REF: Set a table filter on a Session Object.
**
** The second argument (xFilter) is the "filter callback". For changes to rows 
** in tables that are not attached to the Session oject, the filter is called
** to determine whether changes to the table's rows should be tracked or not. 
** If xFilter returns 0, changes is not tracked. Note that once a table is 
** attached, xFilter will not be called again.
*/
void sqlite3session_table_filter(
  sqlite3_session *pSession,      /* Session object */
  int(*xFilter)(
    void *pCtx,                   /* Copy of third arg to _filter_table() */
    const char *zTab              /* Table name */
  ),
  void *pCtx                      /* First argument passed to xFilter */
);

/*
** CAPI3REF: Generate A Changeset From A Session Object
**
** Obtain a changeset containing changes to the tables attached to the 
** session object passed as the first argument. If successful, 
** set *ppChangeset to point to a buffer containing the changeset 
** and *pnChangeset to the size of the changeset in bytes before returning
** SQLITE_OK. If an error occurs, set both *ppChangeset and *pnChangeset to
** zero and return an SQLite error code.
**
** A changeset consists of zero or more INSERT, UPDATE and/or DELETE changes,
** each representing a change to a single row of an attached table. An INSERT
** change contains the values of each field of a new database row. A DELETE
** contains the original values of each field of a deleted database row. An
** UPDATE change contains the original values of each field of an updated
** database row along with the updated values for each updated non-primary-key
** column. It is not possible for an UPDATE change to represent a change that
** modifies the values of primary key columns. If such a change is made, it
** is represented in a changeset as a DELETE followed by an INSERT.
**
** Changes are not recorded for rows that have NULL values stored in one or 
** more of their PRIMARY KEY columns. If such a row is inserted or deleted,
** no corresponding change is present in the changesets returned by this
** function. If an existing row with one or more NULL values stored in
** PRIMARY KEY columns is updated so that all PRIMARY KEY columns are non-NULL,
** only an INSERT is appears in the changeset. Similarly, if an existing row
** with non-NULL PRIMARY KEY values is updated so that one or more of its
** PRIMARY KEY columns are set to NULL, the resulting changeset contains a
** DELETE change only.
**
** The contents of a changeset may be traversed using an iterator created
** using the [sqlite3changeset_start()] API. A changeset may be applied to
** a database with a compatible schema using the [sqlite3changeset_apply()]
** API.
**
** Within a changeset generated by this function, all changes related to a
** single table are grouped together. In other words, when iterating through
** a changeset or when applying a changeset to a database, all changes related
** to a single table are processed before moving on to the next table. Tables
** are sorted in the same order in which they were attached (or auto-attached)
** to the sqlite3_session object. The order in which the changes related to
** a single table are stored is undefined.
**
** Following a successful call to this function, it is the responsibility of
** the caller to eventually free the buffer that *ppChangeset points to using
** [sqlite3_free()].
**
** <h3>Changeset Generation</h3>
**
** Once a table has been attached to a session object, the session object
** records the primary key values of all new rows inserted into the table.
** It also records the original primary key and other column values of any
** deleted or updated rows. For each unique primary key value, data is only
** recorded once - the first time a row with said primary key is inserted,
** updated or deleted in the lifetime of the session.
**
** There is one exception to the previous paragraph: when a row is inserted,
** updated or deleted, if one or more of its primary key columns contain a
** NULL value, no record of the change is made.
**
** The session object therefore accumulates two types of records - those
** that consist of primary key values only (created when the user inserts
** a new record) and those that consist of the primary key values and the
** original values of other table columns (created when the users deletes
** or updates a record).
**
** When this function is called, the requested changeset is created using
** both the accumulated records and the current contents of the database
** file. Specifically:
**
** <ul>
**   <li> For each record generated by an insert, the database is queried
**        for a row with a matching primary key. If one is found, an INSERT
**        change is added to the changeset. If no such row is found, no change 
**        is added to the changeset.
**
**   <li> For each record generated by an update or delete, the database is 
**        queried for a row with a matching primary key. If such a row is
**        found and one or more of the non-primary key fields have been
**        modified from their original values, an UPDATE change is added to 
**        the changeset. Or, if no such row is found in the table, a DELETE 
**        change is added to the changeset. If there is a row with a matching
**        primary key in the database, but all fields contain their original
**        values, no change is added to the changeset.
** </ul>
**
** This means, amongst other things, that if a row is inserted and then later
** deleted while a session object is active, neither the insert nor the delete
** will be present in the changeset. Or if a row is deleted and then later a 
** row with the same primary key values inserted while a session object is
** active, the resulting changeset will contain an UPDATE change instead of
** a DELETE and an INSERT.
**
** When a session object is disabled (see the [sqlite3session_enable()] API),
** it does not accumulate records when rows are inserted, updated or deleted.
** This may appear to have some counter-intuitive effects if a single row
** is written to more than once during a session. For example, if a row
** is inserted while a session object is enabled, then later deleted while 
** the same session object is disabled, no INSERT record will appear in the
** changeset, even though the delete took place while the session was disabled.
** Or, if one field of a row is updated while a session is disabled, and 
** another field of the same row is updated while the session is enabled, the
** resulting changeset will contain an UPDATE change that updates both fields.
*/
int sqlite3session_changeset(
  sqlite3_session *pSession,      /* Session object */
  int *pnChangeset,               /* OUT: Size of buffer at *ppChangeset */
  void **ppChangeset              /* OUT: Buffer containing changeset */
);

/*
** CAPI3REF: Load The Difference Between Tables Into A Session 
**
** If it is not already attached to the session object passed as the first
** argument, this function attaches table zTbl in the same manner as the
** [sqlite3session_attach()] function. If zTbl does not exist, or if it
** does not have a primary key, this function is a no-op (but does not return
** an error).
**
** Argument zFromDb must be the name of a database ("main", "temp" etc.)
** attached to the same database handle as the session object that contains 
** a table compatible with the table attached to the session by this function.
** A table is considered compatible if it:
**
** <ul>
**   <li> Has the same name,
**   <li> Has the same set of columns declared in the same order, and
**   <li> Has the same PRIMARY KEY definition.
** </ul>
**
** If the tables are not compatible, SQLITE_SCHEMA is returned. If the tables
** are compatible but do not have any PRIMARY KEY columns, it is not an error
** but no changes are added to the session object. As with other session
** APIs, tables without PRIMARY KEYs are simply ignored.
**
** This function adds a set of changes to the session object that could be
** used to update the table in database zFrom (call this the "from-table") 
** so that its content is the same as the table attached to the session 
** object (call this the "to-table"). Specifically:
**
** <ul>
**   <li> For each row (primary key) that exists in the to-table but not in 
**     the from-table, an INSERT record is added to the session object.
**
**   <li> For each row (primary key) that exists in the to-table but not in 
**     the from-table, a DELETE record is added to the session object.
**
**   <li> For each row (primary key) that exists in both tables, but features 
**     different in each, an UPDATE record is added to the session.
** </ul>
**
** To clarify, if this function is called and then a changeset constructed
** using [sqlite3session_changeset()], then after applying that changeset to 
** database zFrom the contents of the two compatible tables would be 
** identical.
**
** It an error if database zFrom does not exist or does not contain the
** required compatible table.
**
** If the operation successful, SQLITE_OK is returned. Otherwise, an SQLite
** error code. In this case, if argument pzErrMsg is not NULL, *pzErrMsg
** may be set to point to a buffer containing an English language error 
** message. It is the responsibility of the caller to free this buffer using
** sqlite3_free().
*/
int sqlite3session_diff(
  sqlite3_session *pSession,
  const char *zFromDb,
  const char *zTbl,
  char **pzErrMsg
);


/*
** CAPI3REF: Generate A Patchset From A Session Object
**
** The differences between a patchset and a changeset are that:
**
** <ul>
**   <li> DELETE records consist of the primary key fields only. The 
**        original values of other fields are omitted.
**   <li> The original values of any modified fields are omitted from 
**        UPDATE records.
** </ul>
**
** A patchset blob may be used with up to date versions of all 
** sqlite3changeset_xxx API functions except for sqlite3changeset_invert(), 
** which returns SQLITE_CORRUPT if it is passed a patchset. Similarly,
** attempting to use a patchset blob with old versions of the
** sqlite3changeset_xxx APIs also provokes an SQLITE_CORRUPT error. 
**
** Because the non-primary key "old.*" fields are omitted, no 
** SQLITE_CHANGESET_DATA conflicts can be detected or reported if a patchset
** is passed to the sqlite3changeset_apply() API. Other conflict types work
** in the same way as for changesets.
**
** Changes within a patchset are ordered in the same way as for changesets
** generated by the sqlite3session_changeset() function (i.e. all changes for
** a single table are grouped together, tables appear in the order in which
** they were attached to the session object).
*/
int sqlite3session_patchset(
  sqlite3_session *pSession,      /* Session object */
  int *pnPatchset,                /* OUT: Size of buffer at *ppChangeset */
  void **ppPatchset               /* OUT: Buffer containing changeset */
);

/*
** CAPI3REF: Test if a changeset has recorded any changes.
**
** Return non-zero if no changes to attached tables have been recorded by 
** the session object passed as the first argument. Otherwise, if one or 
** more changes have been recorded, return zero.
**
** Even if this function returns zero, it is possible that calling
** [sqlite3session_changeset()] on the session handle may still return a
** changeset that contains no changes. This can happen when a row in 
** an attached table is modified and then later on the original values 
** are restored. However, if this function returns non-zero, then it is
** guaranteed that a call to sqlite3session_changeset() will return a 
** changeset containing zero changes.
*/
int sqlite3session_isempty(sqlite3_session *pSession);

/*
** CAPI3REF: Create An Iterator To Traverse A Changeset 
**
** Create an iterator used to iterate through the contents of a changeset.
** If successful, *pp is set to point to the iterator handle and SQLITE_OK
** is returned. Otherwise, if an error occurs, *pp is set to zero and an
** SQLite error code is returned.
**
** The following functions can be used to advance and query a changeset 
** iterator created by this function:
**
** <ul>
**   <li> [sqlite3changeset_next()]
**   <li> [sqlite3changeset_op()]
**   <li> [sqlite3changeset_new()]
**   <li> [sqlite3changeset_old()]
** </ul>
**
** It is the responsibility of the caller to eventually destroy the iterator
** by passing it to [sqlite3changeset_finalize()]. The buffer containing the
** changeset (pChangeset) must remain valid until after the iterator is
** destroyed.
**
** Assuming the changeset blob was created by one of the
** [sqlite3session_changeset()], [sqlite3changeset_concat()] or
** [sqlite3changeset_invert()] functions, all changes within the changeset 
** that apply to a single table are grouped together. This means that when 
** an application iterates through a changeset using an iterator created by 
** this function, all changes that relate to a single table are visted 
** consecutively. There is no chance that the iterator will visit a change 
** the applies to table X, then one for table Y, and then later on visit 
** another change for table X.
*/
int sqlite3changeset_start(
  sqlite3_changeset_iter **pp,    /* OUT: New changeset iterator handle */
  int nChangeset,                 /* Size of changeset blob in bytes */
  void *pChangeset                /* Pointer to blob containing changeset */
);


/*
** CAPI3REF: Advance A Changeset Iterator
**
** This function may only be used with iterators created by function
** [sqlite3changeset_start()]. If it is called on an iterator passed to
** a conflict-handler callback by [sqlite3changeset_apply()], SQLITE_MISUSE
** is returned and the call has no effect.
**
** Immediately after an iterator is created by sqlite3changeset_start(), it
** does not point to any change in the changeset. Assuming the changeset
** is not empty, the first call to this function advances the iterator to
** point to the first change in the changeset. Each subsequent call advances
** the iterator to point to the next change in the changeset (if any). If
** no error occurs and the iterator points to a valid change after a call
** to sqlite3changeset_next() has advanced it, SQLITE_ROW is returned. 
** Otherwise, if all changes in the changeset have already been visited,
** SQLITE_DONE is returned.
**
** If an error occurs, an SQLite error code is returned. Possible error 
** codes include SQLITE_CORRUPT (if the changeset buffer is corrupt) or 
** SQLITE_NOMEM.
*/
int sqlite3changeset_next(sqlite3_changeset_iter *pIter);

/*
** CAPI3REF: Obtain The Current Operation From A Changeset Iterator
**
** The pIter argument passed to this function may either be an iterator
** passed to a conflict-handler by [sqlite3changeset_apply()], or an iterator
** created by [sqlite3changeset_start()]. In the latter case, the most recent
** call to [sqlite3changeset_next()] must have returned [SQLITE_ROW]. If this
** is not the case, this function returns [SQLITE_MISUSE].
**
** If argument pzTab is not NULL, then *pzTab is set to point to a
** nul-terminated utf-8 encoded string containing the name of the table
** affected by the current change. The buffer remains valid until either
** sqlite3changeset_next() is called on the iterator or until the 
** conflict-handler function returns. If pnCol is not NULL, then *pnCol is 
** set to the number of columns in the table affected by the change. If
** pbIncorrect is not NULL, then *pbIndirect is set to true (1) if the change
** is an indirect change, or false (0) otherwise. See the documentation for
** [sqlite3session_indirect()] for a description of direct and indirect
** changes. Finally, if pOp is not NULL, then *pOp is set to one of 
** [SQLITE_INSERT], [SQLITE_DELETE] or [SQLITE_UPDATE], depending on the 
** type of change that the iterator currently points to.
**
** If no error occurs, SQLITE_OK is returned. If an error does occur, an
** SQLite error code is returned. The values of the output variables may not
** be trusted in this case.
*/
int sqlite3changeset_op(
  sqlite3_changeset_iter *pIter,  /* Iterator object */
  const char **pzTab,             /* OUT: Pointer to table name */
  int *pnCol,                     /* OUT: Number of columns in table */
  int *pOp,                       /* OUT: SQLITE_INSERT, DELETE or UPDATE */
  int *pbIndirect                 /* OUT: True for an 'indirect' change */
);

/*
** CAPI3REF: Obtain The Primary Key Definition Of A Table
**
** For each modified table, a changeset includes the following:
**
** <ul>
**   <li> The number of columns in the table, and
**   <li> Which of those columns make up the tables PRIMARY KEY.
** </ul>
**
** This function is used to find which columns comprise the PRIMARY KEY of
** the table modified by the change that iterator pIter currently points to.
** If successful, *pabPK is set to point to an array of nCol entries, where
** nCol is the number of columns in the table. Elements of *pabPK are set to
** 0x01 if the corresponding column is part of the tables primary key, or
** 0x00 if it is not.
**
** If argumet pnCol is not NULL, then *pnCol is set to the number of columns
** in the table.
**
** If this function is called when the iterator does not point to a valid
** entry, SQLITE_MISUSE is returned and the output variables zeroed. Otherwise,
** SQLITE_OK is returned and the output variables populated as described
** above.
*/
int sqlite3changeset_pk(
  sqlite3_changeset_iter *pIter,  /* Iterator object */
  unsigned char **pabPK,          /* OUT: Array of boolean - true for PK cols */
  int *pnCol                      /* OUT: Number of entries in output array */
);

/*
** CAPI3REF: Obtain old.* Values From A Changeset Iterator
**
** The pIter argument passed to this function may either be an iterator
** passed to a conflict-handler by [sqlite3changeset_apply()], or an iterator
** created by [sqlite3changeset_start()]. In the latter case, the most recent
** call to [sqlite3changeset_next()] must have returned SQLITE_ROW. 
** Furthermore, it may only be called if the type of change that the iterator
** currently points to is either [SQLITE_DELETE] or [SQLITE_UPDATE]. Otherwise,
** this function returns [SQLITE_MISUSE] and sets *ppValue to NULL.
**
** Argument iVal must be greater than or equal to 0, and less than the number
** of columns in the table affected by the current change. Otherwise,
** [SQLITE_RANGE] is returned and *ppValue is set to NULL.
**
** If successful, this function sets *ppValue to point to a protected
** sqlite3_value object containing the iVal'th value from the vector of 
** original row values stored as part of the UPDATE or DELETE change and
** returns SQLITE_OK. The name of the function comes from the fact that this 
** is similar to the "old.*" columns available to update or delete triggers.
**
** If some other error occurs (e.g. an OOM condition), an SQLite error code
** is returned and *ppValue is set to NULL.
*/
int sqlite3changeset_old(
  sqlite3_changeset_iter *pIter,  /* Changeset iterator */
  int iVal,                       /* Column number */
  sqlite3_value **ppValue         /* OUT: Old value (or NULL pointer) */
);

/*
** CAPI3REF: Obtain new.* Values From A Changeset Iterator
**
** The pIter argument passed to this function may either be an iterator
** passed to a conflict-handler by [sqlite3changeset_apply()], or an iterator
** created by [sqlite3changeset_start()]. In the latter case, the most recent
** call to [sqlite3changeset_next()] must have returned SQLITE_ROW. 
** Furthermore, it may only be called if the type of change that the iterator
** currently points to is either [SQLITE_UPDATE] or [SQLITE_INSERT]. Otherwise,
** this function returns [SQLITE_MISUSE] and sets *ppValue to NULL.
**
** Argument iVal must be greater than or equal to 0, and less than the number
** of columns in the table affected by the current change. Otherwise,
** [SQLITE_RANGE] is returned and *ppValue is set to NULL.
**
** If successful, this function sets *ppValue to point to a protected
** sqlite3_value object containing the iVal'th value from the vector of 
** new row values stored as part of the UPDATE or INSERT change and
** returns SQLITE_OK. If the change is an UPDATE and does not include
** a new value for the requested column, *ppValue is set to NULL and 
** SQLITE_OK returned. The name of the function comes from the fact that 
** this is similar to the "new.*" columns available to update or delete 
** triggers.
**
** If some other error occurs (e.g. an OOM condition), an SQLite error code
** is returned and *ppValue is set to NULL.
*/
int sqlite3changeset_new(
  sqlite3_changeset_iter *pIter,  /* Changeset iterator */
  int iVal,                       /* Column number */
  sqlite3_value **ppValue         /* OUT: New value (or NULL pointer) */
);

/*
** CAPI3REF: Obtain Conflicting Row Values From A Changeset Iterator
**
** This function should only be used with iterator objects passed to a
** conflict-handler callback by [sqlite3changeset_apply()] with either
** [SQLITE_CHANGESET_DATA] or [SQLITE_CHANGESET_CONFLICT]. If this function
** is called on any other iterator, [SQLITE_MISUSE] is returned and *ppValue
** is set to NULL.
**
** Argument iVal must be greater than or equal to 0, and less than the number
** of columns in the table affected by the current change. Otherwise,
** [SQLITE_RANGE] is returned and *ppValue is set to NULL.
**
** If successful, this function sets *ppValue to point to a protected
** sqlite3_value object containing the iVal'th value from the 
** "conflicting row" associated with the current conflict-handler callback
** and returns SQLITE_OK.
**
** If some other error occurs (e.g. an OOM condition), an SQLite error code
** is returned and *ppValue is set to NULL.
*/
int sqlite3changeset_conflict(
  sqlite3_changeset_iter *pIter,  /* Changeset iterator */
  int iVal,                       /* Column number */
  sqlite3_value **ppValue         /* OUT: Value from conflicting row */
);

/*
** CAPI3REF: Determine The Number Of Foreign Key Constraint Violations
**
** This function may only be called with an iterator passed to an
** SQLITE_CHANGESET_FOREIGN_KEY conflict handler callback. In this case
** it sets the output variable to the total number of known foreign key
** violations in the destination database and returns SQLITE_OK.
**
** In all other cases this function returns SQLITE_MISUSE.
*/
int sqlite3changeset_fk_conflicts(
  sqlite3_changeset_iter *pIter,  /* Changeset iterator */
  int *pnOut                      /* OUT: Number of FK violations */
);


/*
** CAPI3REF: Finalize A Changeset Iterator
**
** This function is used to finalize an iterator allocated with
** [sqlite3changeset_start()].
**
** This function should only be called on iterators created using the
** [sqlite3changeset_start()] function. If an application calls this
** function with an iterator passed to a conflict-handler by
** [sqlite3changeset_apply()], [SQLITE_MISUSE] is immediately returned and the
** call has no effect.
**
** If an error was encountered within a call to an sqlite3changeset_xxx()
** function (for example an [SQLITE_CORRUPT] in [sqlite3changeset_next()] or an 
** [SQLITE_NOMEM] in [sqlite3changeset_new()]) then an error code corresponding
** to that error is returned by this function. Otherwise, SQLITE_OK is
** returned. This is to allow the following pattern (pseudo-code):
**
**   sqlite3changeset_start();
**   while( SQLITE_ROW==sqlite3changeset_next() ){
**     // Do something with change.
**   }
**   rc = sqlite3changeset_finalize();
**   if( rc!=SQLITE_OK ){
**     // An error has occurred 
**   }
*/
int sqlite3changeset_finalize(sqlite3_changeset_iter *pIter);

/*
** CAPI3REF: Invert A Changeset
**
** This function is used to "invert" a changeset object. Applying an inverted
** changeset to a database reverses the effects of applying the uninverted
** changeset. Specifically:
**
** <ul>
**   <li> Each DELETE change is changed to an INSERT, and
**   <li> Each INSERT change is changed to a DELETE, and
**   <li> For each UPDATE change, the old.* and new.* values are exchanged.
** </ul>
**
** This function does not change the order in which changes appear within
** the changeset. It merely reverses the sense of each individual change.
**
** If successful, a pointer to a buffer containing the inverted changeset
** is stored in *ppOut, the size of the same buffer is stored in *pnOut, and
** SQLITE_OK is returned. If an error occurs, both *pnOut and *ppOut are
** zeroed and an SQLite error code returned.
**
** It is the responsibility of the caller to eventually call sqlite3_free()
** on the *ppOut pointer to free the buffer allocation following a successful 
** call to this function.
**
** WARNING/TODO: This function currently assumes that the input is a valid
** changeset. If it is not, the results are undefined.
*/
int sqlite3changeset_invert(
  int nIn, const void *pIn,       /* Input changeset */
  int *pnOut, void **ppOut        /* OUT: Inverse of input */
);

/*
** CAPI3REF: Concatenate Two Changeset Objects
**
** This function is used to concatenate two changesets, A and B, into a 
** single changeset. The result is a changeset equivalent to applying
** changeset A followed by changeset B. 
**
** This function combines the two input changesets using an 
** sqlite3_changegroup object. Calling it produces similar results as the
** following code fragment:
**
**   sqlite3_changegroup *pGrp;
**   rc = sqlite3_changegroup_new(&pGrp);
**   if( rc==SQLITE_OK ) rc = sqlite3changegroup_add(pGrp, nA, pA);
**   if( rc==SQLITE_OK ) rc = sqlite3changegroup_add(pGrp, nB, pB);
**   if( rc==SQLITE_OK ){
**     rc = sqlite3changegroup_output(pGrp, pnOut, ppOut);
**   }else{
**     *ppOut = 0;
**     *pnOut = 0;
**   }
**
** Refer to the sqlite3_changegroup documentation below for details.
*/
int sqlite3changeset_concat(
  int nA,                         /* Number of bytes in buffer pA */
  void *pA,                       /* Pointer to buffer containing changeset A */
  int nB,                         /* Number of bytes in buffer pB */
  void *pB,                       /* Pointer to buffer containing changeset B */
  int *pnOut,                     /* OUT: Number of bytes in output changeset */
  void **ppOut                    /* OUT: Buffer containing output changeset */
);


/*
** Changegroup handle.
*/
typedef struct sqlite3_changegroup sqlite3_changegroup;

/*
** CAPI3REF: Combine two or more changesets into a single changeset.
**
** An sqlite3_changegroup object is used to combine two or more changesets
** (or patchsets) into a single changeset (or patchset). A single changegroup
** object may combine changesets or patchsets, but not both. The output is
** always in the same format as the input.
**
** If successful, this function returns SQLITE_OK and populates (*pp) with
** a pointer to a new sqlite3_changegroup object before returning. The caller
** should eventually free the returned object using a call to 
** sqlite3changegroup_delete(). If an error occurs, an SQLite error code
** (i.e. SQLITE_NOMEM) is returned and *pp is set to NULL.
**
** The usual usage pattern for an sqlite3_changegroup object is as follows:
**
** <ul>
**   <li> It is created using a call to sqlite3changegroup_new().
**
**   <li> Zero or more changesets (or patchsets) are added to the object
**        by calling sqlite3changegroup_add().
**
**   <li> The result of combining all input changesets together is obtained 
**        by the application via a call to sqlite3changegroup_output().
**
**   <li> The object is deleted using a call to sqlite3changegroup_delete().
** </ul>
**
** Any number of calls to add() and output() may be made between the calls to
** new() and delete(), and in any order.
**
** As well as the regular sqlite3changegroup_add() and 
** sqlite3changegroup_output() functions, also available are the streaming
** versions sqlite3changegroup_add_strm() and sqlite3changegroup_output_strm().
*/
int sqlite3changegroup_new(sqlite3_changegroup **pp);

/*
** Add all changes within the changeset (or patchset) in buffer pData (size
** nData bytes) to the changegroup. 
**
** If the buffer contains a patchset, then all prior calls to this function
** on the same changegroup object must also have specified patchsets. Or, if
** the buffer contains a changeset, so must have the earlier calls to this
** function. Otherwise, SQLITE_ERROR is returned and no changes are added
** to the changegroup.
**
** Rows within the changeset and changegroup are identified by the values in
** their PRIMARY KEY columns. A change in the changeset is considered to
** apply to the same row as a change already present in the changegroup if
** the two rows have the same primary key.
**
** Changes to rows that that do not already appear in the changegroup are
** simply copied into it. Or, if both the new changeset and the changegroup
** contain changes that apply to a single row, the final contents of the
** changegroup depends on the type of each change, as follows:
**
** <table border=1 style="margin-left:8ex;margin-right:8ex">
**   <tr><th style="white-space:pre">Existing Change  </th>
**       <th style="white-space:pre">New Change       </th>
**       <th>Output Change
**   <tr><td>INSERT <td>INSERT <td>
**       The new change is ignored. This case does not occur if the new
**       changeset was recorded immediately after the changesets already
**       added to the changegroup.
**   <tr><td>INSERT <td>UPDATE <td>
**       The INSERT change remains in the changegroup. The values in the 
**       INSERT change are modified as if the row was inserted by the
**       existing change and then updated according to the new change.
**   <tr><td>INSERT <td>DELETE <td>
**       The existing INSERT is removed from the changegroup. The DELETE is
**       not added.
**   <tr><td>UPDATE <td>INSERT <td>
**       The new change is ignored. This case does not occur if the new
**       changeset was recorded immediately after the changesets already
**       added to the changegroup.
**   <tr><td>UPDATE <td>UPDATE <td>
**       The existing UPDATE remains within the changegroup. It is amended 
**       so that the accompanying values are as if the row was updated once 
**       by the existing change and then again by the new change.
**   <tr><td>UPDATE <td>DELETE <td>
**       The existing UPDATE is replaced by the new DELETE within the
**       changegroup.
**   <tr><td>DELETE <td>INSERT <td>
**       If one or more of the column values in the row inserted by the
**       new change differ from those in the row deleted by the existing 
**       change, the existing DELETE is replaced by an UPDATE within the
**       changegroup. Otherwise, if the inserted row is exactly the same 
**       as the deleted row, the existing DELETE is simply discarded.
**   <tr><td>DELETE <td>UPDATE <td>
**       The new change is ignored. This case does not occur if the new
**       changeset was recorded immediately after the changesets already
**       added to the changegroup.
**   <tr><td>DELETE <td>DELETE <td>
**       The new change is ignored. This case does not occur if the new
**       changeset was recorded immediately after the changesets already
**       added to the changegroup.
** </table>
**
** If the new changeset contains changes to a table that is already present
** in the changegroup, then the number of columns and the position of the
** primary key columns for the table must be consistent. If this is not the
** case, this function fails with SQLITE_SCHEMA. If the input changeset
** appears to be corrupt and the corruption is detected, SQLITE_CORRUPT is
** returned. Or, if an out-of-memory condition occurs during processing, this
** function returns SQLITE_NOMEM. In all cases, if an error occurs the
** final contents of the changegroup is undefined.
**
** If no error occurs, SQLITE_OK is returned.
*/
int sqlite3changegroup_add(sqlite3_changegroup*, int nData, void *pData);

/*
** Obtain a buffer containing a changeset (or patchset) representing the
** current contents of the changegroup. If the inputs to the changegroup
** were themselves changesets, the output is a changeset. Or, if the
** inputs were patchsets, the output is also a patchset.
**
** As with the output of the sqlite3session_changeset() and
** sqlite3session_patchset() functions, all changes related to a single
** table are grouped together in the output of this function. Tables appear
** in the same order as for the very first changeset added to the changegroup.
** If the second or subsequent changesets added to the changegroup contain
** changes for tables that do not appear in the first changeset, they are
** appended onto the end of the output changeset, again in the order in
** which they are first encountered.
**
** If an error occurs, an SQLite error code is returned and the output
** variables (*pnData) and (*ppData) are set to 0. Otherwise, SQLITE_OK
** is returned and the output variables are set to the size of and a 
** pointer to the output buffer, respectively. In this case it is the
** responsibility of the caller to eventually free the buffer using a
** call to sqlite3_free().
*/
int sqlite3changegroup_output(
  sqlite3_changegroup*,
  int *pnData,                    /* OUT: Size of output buffer in bytes */
  void **ppData                   /* OUT: Pointer to output buffer */
);

/*
** Delete a changegroup object.
*/
void sqlite3changegroup_delete(sqlite3_changegroup*);

/*
** CAPI3REF: Apply A Changeset To A Database
**
** Apply a changeset to a database. This function attempts to update the
** "main" database attached to handle db with the changes found in the
** changeset passed via the second and third arguments.
**
** The fourth argument (xFilter) passed to this function is the "filter
** callback". If it is not NULL, then for each table affected by at least one
** change in the changeset, the filter callback is invoked with
** the table name as the second argument, and a copy of the context pointer
** passed as the sixth argument to this function as the first. If the "filter
** callback" returns zero, then no attempt is made to apply any changes to 
** the table. Otherwise, if the return value is non-zero or the xFilter
** argument to this function is NULL, all changes related to the table are
** attempted.
**
** For each table that is not excluded by the filter callback, this function 
** tests that the target database contains a compatible table. A table is 
** considered compatible if all of the following are true:
**
** <ul>
**   <li> The table has the same name as the name recorded in the 
**        changeset, and
**   <li> The table has the same number of columns as recorded in the 
**        changeset, and
**   <li> The table has primary key columns in the same position as 
**        recorded in the changeset.
** </ul>
**
** If there is no compatible table, it is not an error, but none of the
** changes associated with the table are applied. A warning message is issued
** via the sqlite3_log() mechanism with the error code SQLITE_SCHEMA. At most
** one such warning is issued for each table in the changeset.
**
** For each change for which there is a compatible table, an attempt is made 
** to modify the table contents according to the UPDATE, INSERT or DELETE 
** change. If a change cannot be applied cleanly, the conflict handler 
** function passed as the fifth argument to sqlite3changeset_apply() may be 
** invoked. A description of exactly when the conflict handler is invoked for 
** each type of change is below.
**
** Unlike the xFilter argument, xConflict may not be passed NULL. The results
** of passing anything other than a valid function pointer as the xConflict
** argument are undefined.
**
** Each time the conflict handler function is invoked, it must return one
** of [SQLITE_CHANGESET_OMIT], [SQLITE_CHANGESET_ABORT] or 
** [SQLITE_CHANGESET_REPLACE]. SQLITE_CHANGESET_REPLACE may only be returned
** if the second argument passed to the conflict handler is either
** SQLITE_CHANGESET_DATA or SQLITE_CHANGESET_CONFLICT. If the conflict-handler
** returns an illegal value, any changes already made are rolled back and
** the call to sqlite3changeset_apply() returns SQLITE_MISUSE. Different 
** actions are taken by sqlite3changeset_apply() depending on the value
** returned by each invocation of the conflict-handler function. Refer to
** the documentation for the three 
** [SQLITE_CHANGESET_OMIT|available return values] for details.
**
** <dl>
** <dt>DELETE Changes<dd>
**   For each DELETE change, this function checks if the target database 
**   contains a row with the same primary key value (or values) as the 
**   original row values stored in the changeset. If it does, and the values 
**   stored in all non-primary key columns also match the values stored in 
**   the changeset the row is deleted from the target database.
**
**   If a row with matching primary key values is found, but one or more of
**   the non-primary key fields contains a value different from the original
**   row value stored in the changeset, the conflict-handler function is
**   invoked with [SQLITE_CHANGESET_DATA] as the second argument.
**
**   If no row with matching primary key values is found in the database,
**   the conflict-handler function is invoked with [SQLITE_CHANGESET_NOTFOUND]
**   passed as the second argument.
**
**   If the DELETE operation is attempted, but SQLite returns SQLITE_CONSTRAINT
**   (which can only happen if a foreign key constraint is violated), the
**   conflict-handler function is invoked with [SQLITE_CHANGESET_CONSTRAINT]
**   passed as the second argument. This includes the case where the DELETE
**   operation is attempted because an earlier call to the conflict handler
**   function returned [SQLITE_CHANGESET_REPLACE].
**
** <dt>INSERT Changes<dd>
**   For each INSERT change, an attempt is made to insert the new row into
**   the database.
**
**   If the attempt to insert the row fails because the database already 
**   contains a row with the same primary key values, the conflict handler
**   function is invoked with the second argument set to 
**   [SQLITE_CHANGESET_CONFLICT].
**
**   If the attempt to insert the row fails because of some other constraint
**   violation (e.g. NOT NULL or UNIQUE), the conflict handler function is 
**   invoked with the second argument set to [SQLITE_CHANGESET_CONSTRAINT].
**   This includes the case where the INSERT operation is re-attempted because 
**   an earlier call to the conflict handler function returned 
**   [SQLITE_CHANGESET_REPLACE].
**
** <dt>UPDATE Changes<dd>
**   For each UPDATE change, this function checks if the target database 
**   contains a row with the same primary key value (or values) as the 
**   original row values stored in the changeset. If it does, and the values 
**   stored in all non-primary key columns also match the values stored in 
**   the changeset the row is updated within the target database.
**
**   If a row with matching primary key values is found, but one or more of
**   the non-primary key fields contains a value different from an original
**   row value stored in the changeset, the conflict-handler function is
**   invoked with [SQLITE_CHANGESET_DATA] as the second argument. Since
**   UPDATE changes only contain values for non-primary key fields that are
**   to be modified, only those fields need to match the original values to
**   avoid the SQLITE_CHANGESET_DATA conflict-handler callback.
**
**   If no row with matching primary key values is found in the database,
**   the conflict-handler function is invoked with [SQLITE_CHANGESET_NOTFOUND]
**   passed as the second argument.
**
**   If the UPDATE operation is attempted, but SQLite returns 
**   SQLITE_CONSTRAINT, the conflict-handler function is invoked with 
**   [SQLITE_CHANGESET_CONSTRAINT] passed as the second argument.
**   This includes the case where the UPDATE operation is attempted after 
**   an earlier call to the conflict handler function returned
**   [SQLITE_CHANGESET_REPLACE].  
** </dl>
**
** It is safe to execute SQL statements, including those that write to the
** table that the callback related to, from within the xConflict callback.
** This can be used to further customize the applications conflict
** resolution strategy.
**
** All changes made by this function are enclosed in a savepoint transaction.
** If any other error (aside from a constraint failure when attempting to
** write to the target database) occurs, then the savepoint transaction is
** rolled back, restoring the target database to its original state, and an 
** SQLite error code returned.
*/
int sqlite3changeset_apply(
  sqlite3 *db,                    /* Apply change to "main" db of this handle */
  int nChangeset,                 /* Size of changeset in bytes */
  void *pChangeset,               /* Changeset blob */
  int(*xFilter)(
    void *pCtx,                   /* Copy of sixth arg to _apply() */
    const char *zTab              /* Table name */
  ),
  int(*xConflict)(
    void *pCtx,                   /* Copy of sixth arg to _apply() */
    int eConflict,                /* DATA, MISSING, CONFLICT, CONSTRAINT */
    sqlite3_changeset_iter *p     /* Handle describing change and conflict */
  ),
  void *pCtx                      /* First argument passed to xConflict */
);

/* 
** CAPI3REF: Constants Passed To The Conflict Handler
**
** Values that may be passed as the second argument to a conflict-handler.
**
** <dl>
** <dt>SQLITE_CHANGESET_DATA<dd>
**   The conflict handler is invoked with CHANGESET_DATA as the second argument
**   when processing a DELETE or UPDATE change if a row with the required
**   PRIMARY KEY fields is present in the database, but one or more other 
**   (non primary-key) fields modified by the update do not contain the 
**   expected "before" values.
** 
**   The conflicting row, in this case, is the database row with the matching
**   primary key.
** 
** <dt>SQLITE_CHANGESET_NOTFOUND<dd>
**   The conflict handler is invoked with CHANGESET_NOTFOUND as the second
**   argument when processing a DELETE or UPDATE change if a row with the
**   required PRIMARY KEY fields is not present in the database.
** 
**   There is no conflicting row in this case. The results of invoking the
**   sqlite3changeset_conflict() API are undefined.
** 
** <dt>SQLITE_CHANGESET_CONFLICT<dd>
**   CHANGESET_CONFLICT is passed as the second argument to the conflict
**   handler while processing an INSERT change if the operation would result 
**   in duplicate primary key values.
** 
**   The conflicting row in this case is the database row with the matching
**   primary key.
**
** <dt>SQLITE_CHANGESET_FOREIGN_KEY<dd>
**   If foreign key handling is enabled, and applying a changeset leaves the
**   database in a state containing foreign key violations, the conflict 
**   handler is invoked with CHANGESET_FOREIGN_KEY as the second argument
**   exactly once before the changeset is committed. If the conflict handler
**   returns CHANGESET_OMIT, the changes, including those that caused the
**   foreign key constraint violation, are committed. Or, if it returns
**   CHANGESET_ABORT, the changeset is rolled back.
**
**   No current or conflicting row information is provided. The only function
**   it is possible to call on the supplied sqlite3_changeset_iter handle
**   is sqlite3changeset_fk_conflicts().
** 
** <dt>SQLITE_CHANGESET_CONSTRAINT<dd>
**   If any other constraint violation occurs while applying a change (i.e. 
**   a UNIQUE, CHECK or NOT NULL constraint), the conflict handler is 
**   invoked with CHANGESET_CONSTRAINT as the second argument.
** 
**   There is no conflicting row in this case. The results of invoking the
**   sqlite3changeset_conflict() API are undefined.
**
** </dl>
*/
#define SQLITE_CHANGESET_DATA        1
#define SQLITE_CHANGESET_NOTFOUND    2
#define SQLITE_CHANGESET_CONFLICT    3
#define SQLITE_CHANGESET_CONSTRAINT  4
#define SQLITE_CHANGESET_FOREIGN_KEY 5

/* 
** CAPI3REF: Constants Returned By The Conflict Handler
**
** A conflict handler callback must return one of the following three values.
**
** <dl>
** <dt>SQLITE_CHANGESET_OMIT<dd>
**   If a conflict handler returns this value no special action is taken. The
**   change that caused the conflict is not applied. The session module 
**   continues to the next change in the changeset.
**
** <dt>SQLITE_CHANGESET_REPLACE<dd>
**   This value may only be returned if the second argument to the conflict
**   handler was SQLITE_CHANGESET_DATA or SQLITE_CHANGESET_CONFLICT. If this
**   is not the case, any changes applied so far are rolled back and the 
**   call to sqlite3changeset_apply() returns SQLITE_MISUSE.
**
**   If CHANGESET_REPLACE is returned by an SQLITE_CHANGESET_DATA conflict
**   handler, then the conflicting row is either updated or deleted, depending
**   on the type of change.
**
**   If CHANGESET_REPLACE is returned by an SQLITE_CHANGESET_CONFLICT conflict
**   handler, then the conflicting row is removed from the database and a
**   second attempt to apply the change is made. If this second attempt fails,
**   the original row is restored to the database before continuing.
**
** <dt>SQLITE_CHANGESET_ABORT<dd>
**   If this value is returned, any changes applied so far are rolled back 
**   and the call to sqlite3changeset_apply() returns SQLITE_ABORT.
** </dl>
*/
#define SQLITE_CHANGESET_OMIT       0
#define SQLITE_CHANGESET_REPLACE    1
#define SQLITE_CHANGESET_ABORT      2

/*
** CAPI3REF: Streaming Versions of API functions.
**
** The six streaming API xxx_strm() functions serve similar purposes to the 
** corresponding non-streaming API functions:
**
** <table border=1 style="margin-left:8ex;margin-right:8ex">
**   <tr><th>Streaming function<th>Non-streaming equivalent</th>
**   <tr><td>sqlite3changeset_apply_str<td>[sqlite3changeset_apply] 
**   <tr><td>sqlite3changeset_concat_str<td>[sqlite3changeset_concat] 
**   <tr><td>sqlite3changeset_invert_str<td>[sqlite3changeset_invert] 
**   <tr><td>sqlite3changeset_start_str<td>[sqlite3changeset_start] 
**   <tr><td>sqlite3session_changeset_str<td>[sqlite3session_changeset] 
**   <tr><td>sqlite3session_patchset_str<td>[sqlite3session_patchset] 
** </table>
**
** Non-streaming functions that accept changesets (or patchsets) as input
** require that the entire changeset be stored in a single buffer in memory. 
** Similarly, those that return a changeset or patchset do so by returning 
** a pointer to a single large buffer allocated using sqlite3_malloc(). 
** Normally this is convenient. However, if an application running in a 
** low-memory environment is required to handle very large changesets, the
** large contiguous memory allocations required can become onerous.
**
** In order to avoid this problem, instead of a single large buffer, input
** is passed to a streaming API functions by way of a callback function that
** the sessions module invokes to incrementally request input data as it is
** required. In all cases, a pair of API function parameters such as
**
**  <pre>
**  &nbsp;     int nChangeset,
**  &nbsp;     void *pChangeset,
**  </pre>
**
** Is replaced by:
**
**  <pre>
**  &nbsp;     int (*xInput)(void *pIn, void *pData, int *pnData),
**  &nbsp;     void *pIn,
**  </pre>
**
** Each time the xInput callback is invoked by the sessions module, the first
** argument passed is a copy of the supplied pIn context pointer. The second 
** argument, pData, points to a buffer (*pnData) bytes in size. Assuming no 
** error occurs the xInput method should copy up to (*pnData) bytes of data 
** into the buffer and set (*pnData) to the actual number of bytes copied 
** before returning SQLITE_OK. If the input is completely exhausted, (*pnData) 
** should be set to zero to indicate this. Or, if an error occurs, an SQLite 
** error code should be returned. In all cases, if an xInput callback returns
** an error, all processing is abandoned and the streaming API function
** returns a copy of the error code to the caller.
**
** In the case of sqlite3changeset_start_strm(), the xInput callback may be
** invoked by the sessions module at any point during the lifetime of the
** iterator. If such an xInput callback returns an error, the iterator enters
** an error state, whereby all subsequent calls to iterator functions 
** immediately fail with the same error code as returned by xInput.
**
** Similarly, streaming API functions that return changesets (or patchsets)
** return them in chunks by way of a callback function instead of via a
** pointer to a single large buffer. In this case, a pair of parameters such
** as:
**
**  <pre>
**  &nbsp;     int *pnChangeset,
**  &nbsp;     void **ppChangeset,
**  </pre>
**
** Is replaced by:
**
**  <pre>
**  &nbsp;     int (*xOutput)(void *pOut, const void *pData, int nData),
**  &nbsp;     void *pOut
**  </pre>
**
** The xOutput callback is invoked zero or more times to return data to
** the application. The first parameter passed to each call is a copy of the
** pOut pointer supplied by the application. The second parameter, pData,
** points to a buffer nData bytes in size containing the chunk of output
** data being returned. If the xOutput callback successfully processes the
** supplied data, it should return SQLITE_OK to indicate success. Otherwise,
** it should return some other SQLite error code. In this case processing
** is immediately abandoned and the streaming API function returns a copy
** of the xOutput error code to the application.
**
** The sessions module never invokes an xOutput callback with the third 
** parameter set to a value less than or equal to zero. Other than this,
** no guarantees are made as to the size of the chunks of data returned.
*/
int sqlite3changeset_apply_strm(
  sqlite3 *db,                    /* Apply change to "main" db of this handle */
  int (*xInput)(void *pIn, void *pData, int *pnData), /* Input function */
  void *pIn,                                          /* First arg for xInput */
  int(*xFilter)(
    void *pCtx,                   /* Copy of sixth arg to _apply() */
    const char *zTab              /* Table name */
  ),
  int(*xConflict)(
    void *pCtx,                   /* Copy of sixth arg to _apply() */
    int eConflict,                /* DATA, MISSING, CONFLICT, CONSTRAINT */
    sqlite3_changeset_iter *p     /* Handle describing change and conflict */
  ),
  void *pCtx                      /* First argument passed to xConflict */
);
int sqlite3changeset_concat_strm(
  int (*xInputA)(void *pIn, void *pData, int *pnData),
  void *pInA,
  int (*xInputB)(void *pIn, void *pData, int *pnData),
  void *pInB,
  int (*xOutput)(void *pOut, const void *pData, int nData),
  void *pOut
);
int sqlite3changeset_invert_strm(
  int (*xInput)(void *pIn, void *pData, int *pnData),
  void *pIn,
  int (*xOutput)(void *pOut, const void *pData, int nData),
  void *pOut
);
int sqlite3changeset_start_strm(
  sqlite3_changeset_iter **pp,
  int (*xInput)(void *pIn, void *pData, int *pnData),
  void *pIn
);
int sqlite3session_changeset_strm(
  sqlite3_session *pSession,
  int (*xOutput)(void *pOut, const void *pData, int nData),
  void *pOut
);
int sqlite3session_patchset_strm(
  sqlite3_session *pSession,
  int (*xOutput)(void *pOut, const void *pData, int nData),
  void *pOut
);
int sqlite3changegroup_add_strm(sqlite3_changegroup*, 
    int (*xInput)(void *pIn, void *pData, int *pnData),
    void *pIn
);
int sqlite3changegroup_output_strm(sqlite3_changegroup*,
    int (*xOutput)(void *pOut, const void *pData, int nData), 
    void *pOut
);


/*
** Make sure we can call this stuff from C++.
*/
#if 0
}
#endif

#endif  /* !defined(__SQLITESESSION_H_) && defined(SQLITE_ENABLE_SESSION) */

/******** End of sqlite3session.h *********/
/******** Begin file fts5.h *********/
/*
** 2014 May 31
**
** The author disclaims copyright to this source code.  In place of
** a legal notice, here is a blessing:
**
**    May you do good and not evil.
**    May you find forgiveness for yourself and forgive others.
**    May you share freely, never taking more than you give.
**
******************************************************************************
**
** Interfaces to extend FTS5. Using the interfaces defined in this file, 
** FTS5 may be extended with:
**
**     * custom tokenizers, and
**     * custom auxiliary functions.
*/


#ifndef _FTS5_H
#define _FTS5_H


#if 0
extern "C" {
#endif

/*************************************************************************
** CUSTOM AUXILIARY FUNCTIONS
**
** Virtual table implementations may overload SQL functions by implementing
** the sqlite3_module.xFindFunction() method.
*/

typedef struct Fts5ExtensionApi Fts5ExtensionApi;
typedef struct Fts5Context Fts5Context;
typedef struct Fts5PhraseIter Fts5PhraseIter;

typedef void (*fts5_extension_function)(
  const Fts5ExtensionApi *pApi,   /* API offered by current FTS version */
  Fts5Context *pFts,              /* First arg to pass to pApi functions */
  sqlite3_context *pCtx,          /* Context for returning result/error */
  int nVal,                       /* Number of values in apVal[] array */
  sqlite3_value **apVal           /* Array of trailing arguments */
);

struct Fts5PhraseIter {
  const unsigned char *a;
  const unsigned char *b;
};

/*
** EXTENSION API FUNCTIONS
**
** xUserData(pFts):
**   Return a copy of the context pointer the extension function was 
**   registered with.
**
** xColumnTotalSize(pFts, iCol, pnToken):
**   If parameter iCol is less than zero, set output variable *pnToken
**   to the total number of tokens in the FTS5 table. Or, if iCol is
**   non-negative but less than the number of columns in the table, return
**   the total number of tokens in column iCol, considering all rows in 
**   the FTS5 table.
**
**   If parameter iCol is greater than or equal to the number of columns
**   in the table, SQLITE_RANGE is returned. Or, if an error occurs (e.g.
**   an OOM condition or IO error), an appropriate SQLite error code is 
**   returned.
**
** xColumnCount(pFts):
**   Return the number of columns in the table.
**
** xColumnSize(pFts, iCol, pnToken):
**   If parameter iCol is less than zero, set output variable *pnToken
**   to the total number of tokens in the current row. Or, if iCol is
**   non-negative but less than the number of columns in the table, set
**   *pnToken to the number of tokens in column iCol of the current row.
**
**   If parameter iCol is greater than or equal to the number of columns
**   in the table, SQLITE_RANGE is returned. Or, if an error occurs (e.g.
**   an OOM condition or IO error), an appropriate SQLite error code is 
**   returned.
**
**   This function may be quite inefficient if used with an FTS5 table
**   created with the "columnsize=0" option.
**
** xColumnText:
**   This function attempts to retrieve the text of column iCol of the
**   current document. If successful, (*pz) is set to point to a buffer
**   containing the text in utf-8 encoding, (*pn) is set to the size in bytes
**   (not characters) of the buffer and SQLITE_OK is returned. Otherwise,
**   if an error occurs, an SQLite error code is returned and the final values
**   of (*pz) and (*pn) are undefined.
**
** xPhraseCount:
**   Returns the number of phrases in the current query expression.
**
** xPhraseSize:
**   Returns the number of tokens in phrase iPhrase of the query. Phrases
**   are numbered starting from zero.
**
** xInstCount:
**   Set *pnInst to the total number of occurrences of all phrases within
**   the query within the current row. Return SQLITE_OK if successful, or
**   an error code (i.e. SQLITE_NOMEM) if an error occurs.
**
**   This API can be quite slow if used with an FTS5 table created with the
**   "detail=none" or "detail=column" option. If the FTS5 table is created 
**   with either "detail=none" or "detail=column" and "content=" option 
**   (i.e. if it is a contentless table), then this API always returns 0.
**
** xInst:
**   Query for the details of phrase match iIdx within the current row.
**   Phrase matches are numbered starting from zero, so the iIdx argument
**   should be greater than or equal to zero and smaller than the value
**   output by xInstCount().
**
**   Usually, output parameter *piPhrase is set to the phrase number, *piCol
**   to the column in which it occurs and *piOff the token offset of the
**   first token of the phrase. The exception is if the table was created
**   with the offsets=0 option specified. In this case *piOff is always
**   set to -1.
**
**   Returns SQLITE_OK if successful, or an error code (i.e. SQLITE_NOMEM) 
**   if an error occurs.
**
**   This API can be quite slow if used with an FTS5 table created with the
**   "detail=none" or "detail=column" option. 
**
** xRowid:
**   Returns the rowid of the current row.
**
** xTokenize:
**   Tokenize text using the tokenizer belonging to the FTS5 table.
**
** xQueryPhrase(pFts5, iPhrase, pUserData, xCallback):
**   This API function is used to query the FTS table for phrase iPhrase
**   of the current query. Specifically, a query equivalent to:
**
**       ... FROM ftstable WHERE ftstable MATCH $p ORDER BY rowid
**
**   with $p set to a phrase equivalent to the phrase iPhrase of the
**   current query is executed. Any column filter that applies to
**   phrase iPhrase of the current query is included in $p. For each 
**   row visited, the callback function passed as the fourth argument 
**   is invoked. The context and API objects passed to the callback 
**   function may be used to access the properties of each matched row.
**   Invoking Api.xUserData() returns a copy of the pointer passed as 
**   the third argument to pUserData.
**
**   If the callback function returns any value other than SQLITE_OK, the
**   query is abandoned and the xQueryPhrase function returns immediately.
**   If the returned value is SQLITE_DONE, xQueryPhrase returns SQLITE_OK.
**   Otherwise, the error code is propagated upwards.
**
**   If the query runs to completion without incident, SQLITE_OK is returned.
**   Or, if some error occurs before the query completes or is aborted by
**   the callback, an SQLite error code is returned.
**
**
** xSetAuxdata(pFts5, pAux, xDelete)
**
**   Save the pointer passed as the second argument as the extension functions 
**   "auxiliary data". The pointer may then be retrieved by the current or any
**   future invocation of the same fts5 extension function made as part of
**   of the same MATCH query using the xGetAuxdata() API.
**
**   Each extension function is allocated a single auxiliary data slot for
**   each FTS query (MATCH expression). If the extension function is invoked 
**   more than once for a single FTS query, then all invocations share a 
**   single auxiliary data context.
**
**   If there is already an auxiliary data pointer when this function is
**   invoked, then it is replaced by the new pointer. If an xDelete callback
**   was specified along with the original pointer, it is invoked at this
**   point.
**
**   The xDelete callback, if one is specified, is also invoked on the
**   auxiliary data pointer after the FTS5 query has finished.
**
**   If an error (e.g. an OOM condition) occurs within this function, an
**   the auxiliary data is set to NULL and an error code returned. If the
**   xDelete parameter was not NULL, it is invoked on the auxiliary data
**   pointer before returning.
**
**
** xGetAuxdata(pFts5, bClear)
**
**   Returns the current auxiliary data pointer for the fts5 extension 
**   function. See the xSetAuxdata() method for details.
**
**   If the bClear argument is non-zero, then the auxiliary data is cleared
**   (set to NULL) before this function returns. In this case the xDelete,
**   if any, is not invoked.
**
**
** xRowCount(pFts5, pnRow)
**
**   This function is used to retrieve the total number of rows in the table.
**   In other words, the same value that would be returned by:
**
**        SELECT count(*) FROM ftstable;
**
** xPhraseFirst()
**   This function is used, along with type Fts5PhraseIter and the xPhraseNext
**   method, to iterate through all instances of a single query phrase within
**   the current row. This is the same information as is accessible via the
**   xInstCount/xInst APIs. While the xInstCount/xInst APIs are more convenient
**   to use, this API may be faster under some circumstances. To iterate 
**   through instances of phrase iPhrase, use the following code:
**
**       Fts5PhraseIter iter;
**       int iCol, iOff;
**       for(pApi->xPhraseFirst(pFts, iPhrase, &iter, &iCol, &iOff);
**           iCol>=0;
**           pApi->xPhraseNext(pFts, &iter, &iCol, &iOff)
**       ){
**         // An instance of phrase iPhrase at offset iOff of column iCol
**       }
**
**   The Fts5PhraseIter structure is defined above. Applications should not
**   modify this structure directly - it should only be used as shown above
**   with the xPhraseFirst() and xPhraseNext() API methods (and by
**   xPhraseFirstColumn() and xPhraseNextColumn() as illustrated below).
**
**   This API can be quite slow if used with an FTS5 table created with the
**   "detail=none" or "detail=column" option. If the FTS5 table is created 
**   with either "detail=none" or "detail=column" and "content=" option 
**   (i.e. if it is a contentless table), then this API always iterates
**   through an empty set (all calls to xPhraseFirst() set iCol to -1).
**
** xPhraseNext()
**   See xPhraseFirst above.
**
** xPhraseFirstColumn()
**   This function and xPhraseNextColumn() are similar to the xPhraseFirst()
**   and xPhraseNext() APIs described above. The difference is that instead
**   of iterating through all instances of a phrase in the current row, these
**   APIs are used to iterate through the set of columns in the current row
**   that contain one or more instances of a specified phrase. For example:
**
**       Fts5PhraseIter iter;
**       int iCol;
**       for(pApi->xPhraseFirstColumn(pFts, iPhrase, &iter, &iCol);
**           iCol>=0;
**           pApi->xPhraseNextColumn(pFts, &iter, &iCol)
**       ){
**         // Column iCol contains at least one instance of phrase iPhrase
**       }
**
**   This API can be quite slow if used with an FTS5 table created with the
**   "detail=none" option. If the FTS5 table is created with either 
**   "detail=none" "content=" option (i.e. if it is a contentless table), 
**   then this API always iterates through an empty set (all calls to 
**   xPhraseFirstColumn() set iCol to -1).
**
**   The information accessed using this API and its companion
**   xPhraseFirstColumn() may also be obtained using xPhraseFirst/xPhraseNext
**   (or xInst/xInstCount). The chief advantage of this API is that it is
**   significantly more efficient than those alternatives when used with
**   "detail=column" tables.  
**
** xPhraseNextColumn()
**   See xPhraseFirstColumn above.
*/
struct Fts5ExtensionApi {
  int iVersion;                   /* Currently always set to 3 */

  void *(*xUserData)(Fts5Context*);

  int (*xColumnCount)(Fts5Context*);
  int (*xRowCount)(Fts5Context*, sqlite3_int64 *pnRow);
  int (*xColumnTotalSize)(Fts5Context*, int iCol, sqlite3_int64 *pnToken);

  int (*xTokenize)(Fts5Context*, 
    const char *pText, int nText, /* Text to tokenize */
    void *pCtx,                   /* Context passed to xToken() */
    int (*xToken)(void*, int, const char*, int, int, int)       /* Callback */
  );

  int (*xPhraseCount)(Fts5Context*);
  int (*xPhraseSize)(Fts5Context*, int iPhrase);

  int (*xInstCount)(Fts5Context*, int *pnInst);
  int (*xInst)(Fts5Context*, int iIdx, int *piPhrase, int *piCol, int *piOff);

  sqlite3_int64 (*xRowid)(Fts5Context*);
  int (*xColumnText)(Fts5Context*, int iCol, const char **pz, int *pn);
  int (*xColumnSize)(Fts5Context*, int iCol, int *pnToken);

  int (*xQueryPhrase)(Fts5Context*, int iPhrase, void *pUserData,
    int(*)(const Fts5ExtensionApi*,Fts5Context*,void*)
  );
  int (*xSetAuxdata)(Fts5Context*, void *pAux, void(*xDelete)(void*));
  void *(*xGetAuxdata)(Fts5Context*, int bClear);

  int (*xPhraseFirst)(Fts5Context*, int iPhrase, Fts5PhraseIter*, int*, int*);
  void (*xPhraseNext)(Fts5Context*, Fts5PhraseIter*, int *piCol, int *piOff);

  int (*xPhraseFirstColumn)(Fts5Context*, int iPhrase, Fts5PhraseIter*, int*);
  void (*xPhraseNextColumn)(Fts5Context*, Fts5PhraseIter*, int *piCol);
};

/* 
** CUSTOM AUXILIARY FUNCTIONS
*************************************************************************/

/*************************************************************************
** CUSTOM TOKENIZERS
**
** Applications may also register custom tokenizer types. A tokenizer 
** is registered by providing fts5 with a populated instance of the 
** following structure. All structure methods must be defined, setting
** any member of the fts5_tokenizer struct to NULL leads to undefined
** behaviour. The structure methods are expected to function as follows:
**
** xCreate:
**   This function is used to allocate and inititalize a tokenizer instance.
**   A tokenizer instance is required to actually tokenize text.
**
**   The first argument passed to this function is a copy of the (void*)
**   pointer provided by the application when the fts5_tokenizer object
**   was registered with FTS5 (the third argument to xCreateTokenizer()). 
**   The second and third arguments are an array of nul-terminated strings
**   containing the tokenizer arguments, if any, specified following the
**   tokenizer name as part of the CREATE VIRTUAL TABLE statement used
**   to create the FTS5 table.
**
**   The final argument is an output variable. If successful, (*ppOut) 
**   should be set to point to the new tokenizer handle and SQLITE_OK
**   returned. If an error occurs, some value other than SQLITE_OK should
**   be returned. In this case, fts5 assumes that the final value of *ppOut 
**   is undefined.
**
** xDelete:
**   This function is invoked to delete a tokenizer handle previously
**   allocated using xCreate(). Fts5 guarantees that this function will
**   be invoked exactly once for each successful call to xCreate().
**
** xTokenize:
**   This function is expected to tokenize the nText byte string indicated 
**   by argument pText. pText may or may not be nul-terminated. The first
**   argument passed to this function is a pointer to an Fts5Tokenizer object
**   returned by an earlier call to xCreate().
**
**   The second argument indicates the reason that FTS5 is requesting
**   tokenization of the supplied text. This is always one of the following
**   four values:
**
**   <ul><li> <b>FTS5_TOKENIZE_DOCUMENT</b> - A document is being inserted into
**            or removed from the FTS table. The tokenizer is being invoked to
**            determine the set of tokens to add to (or delete from) the
**            FTS index.
**
**       <li> <b>FTS5_TOKENIZE_QUERY</b> - A MATCH query is being executed 
**            against the FTS index. The tokenizer is being called to tokenize 
**            a bareword or quoted string specified as part of the query.
**
**       <li> <b>(FTS5_TOKENIZE_QUERY | FTS5_TOKENIZE_PREFIX)</b> - Same as
**            FTS5_TOKENIZE_QUERY, except that the bareword or quoted string is
**            followed by a "*" character, indicating that the last token
**            returned by the tokenizer will be treated as a token prefix.
**
**       <li> <b>FTS5_TOKENIZE_AUX</b> - The tokenizer is being invoked to 
**            satisfy an fts5_api.xTokenize() request made by an auxiliary
**            function. Or an fts5_api.xColumnSize() request made by the same
**            on a columnsize=0 database.  
**   </ul>
**
**   For each token in the input string, the supplied callback xToken() must
**   be invoked. The first argument to it should be a copy of the pointer
**   passed as the second argument to xTokenize(). The third and fourth
**   arguments are a pointer to a buffer containing the token text, and the
**   size of the token in bytes. The 4th and 5th arguments are the byte offsets
**   of the first byte of and first byte immediately following the text from
**   which the token is derived within the input.
**
**   The second argument passed to the xToken() callback ("tflags") should
**   normally be set to 0. The exception is if the tokenizer supports 
**   synonyms. In this case see the discussion below for details.
**
**   FTS5 assumes the xToken() callback is invoked for each token in the 
**   order that they occur within the input text.
**
**   If an xToken() callback returns any value other than SQLITE_OK, then
**   the tokenization should be abandoned and the xTokenize() method should
**   immediately return a copy of the xToken() return value. Or, if the
**   input buffer is exhausted, xTokenize() should return SQLITE_OK. Finally,
**   if an error occurs with the xTokenize() implementation itself, it
**   may abandon the tokenization and return any error code other than
**   SQLITE_OK or SQLITE_DONE.
**
** SYNONYM SUPPORT
**
**   Custom tokenizers may also support synonyms. Consider a case in which a
**   user wishes to query for a phrase such as "first place". Using the 
**   built-in tokenizers, the FTS5 query 'first + place' will match instances
**   of "first place" within the document set, but not alternative forms
**   such as "1st place". In some applications, it would be better to match
**   all instances of "first place" or "1st place" regardless of which form
**   the user specified in the MATCH query text.
**
**   There are several ways to approach this in FTS5:
**
**   <ol><li> By mapping all synonyms to a single token. In this case, the 
**            In the above example, this means that the tokenizer returns the
**            same token for inputs "first" and "1st". Say that token is in
**            fact "first", so that when the user inserts the document "I won
**            1st place" entries are added to the index for tokens "i", "won",
**            "first" and "place". If the user then queries for '1st + place',
**            the tokenizer substitutes "first" for "1st" and the query works
**            as expected.
**
**       <li> By adding multiple synonyms for a single term to the FTS index.
**            In this case, when tokenizing query text, the tokenizer may 
**            provide multiple synonyms for a single term within the document.
**            FTS5 then queries the index for each synonym individually. For
**            example, faced with the query:
**
**   <codeblock>
**     ... MATCH 'first place'</codeblock>
**
**            the tokenizer offers both "1st" and "first" as synonyms for the
**            first token in the MATCH query and FTS5 effectively runs a query 
**            similar to:
**
**   <codeblock>
**     ... MATCH '(first OR 1st) place'</codeblock>
**
**            except that, for the purposes of auxiliary functions, the query
**            still appears to contain just two phrases - "(first OR 1st)" 
**            being treated as a single phrase.
**
**       <li> By adding multiple synonyms for a single term to the FTS index.
**            Using this method, when tokenizing document text, the tokenizer
**            provides multiple synonyms for each token. So that when a 
**            document such as "I won first place" is tokenized, entries are
**            added to the FTS index for "i", "won", "first", "1st" and
**            "place".
**
**            This way, even if the tokenizer does not provide synonyms
**            when tokenizing query text (it should not - to do would be
**            inefficient), it doesn't matter if the user queries for 
**            'first + place' or '1st + place', as there are entires in the
**            FTS index corresponding to both forms of the first token.
**   </ol>
**
**   Whether it is parsing document or query text, any call to xToken that
**   specifies a <i>tflags</i> argument with the FTS5_TOKEN_COLOCATED bit
**   is considered to supply a synonym for the previous token. For example,
**   when parsing the document "I won first place", a tokenizer that supports
**   synonyms would call xToken() 5 times, as follows:
**
**   <codeblock>
**       xToken(pCtx, 0, "i",                      1,  0,  1);
**       xToken(pCtx, 0, "won",                    3,  2,  5);
**       xToken(pCtx, 0, "first",                  5,  6, 11);
**       xToken(pCtx, FTS5_TOKEN_COLOCATED, "1st", 3,  6, 11);
**       xToken(pCtx, 0, "place",                  5, 12, 17);
**</codeblock>
**
**   It is an error to specify the FTS5_TOKEN_COLOCATED flag the first time
**   xToken() is called. Multiple synonyms may be specified for a single token
**   by making multiple calls to xToken(FTS5_TOKEN_COLOCATED) in sequence. 
**   There is no limit to the number of synonyms that may be provided for a
**   single token.
**
**   In many cases, method (1) above is the best approach. It does not add 
**   extra data to the FTS index or require FTS5 to query for multiple terms,
**   so it is efficient in terms of disk space and query speed. However, it
**   does not support prefix queries very well. If, as suggested above, the
**   token "first" is subsituted for "1st" by the tokenizer, then the query:
**
**   <codeblock>
**     ... MATCH '1s*'</codeblock>
**
**   will not match documents that contain the token "1st" (as the tokenizer
**   will probably not map "1s" to any prefix of "first").
**
**   For full prefix support, method (3) may be preferred. In this case, 
**   because the index contains entries for both "first" and "1st", prefix
**   queries such as 'fi*' or '1s*' will match correctly. However, because
**   extra entries are added to the FTS index, this method uses more space
**   within the database.
**
**   Method (2) offers a midpoint between (1) and (3). Using this method,
**   a query such as '1s*' will match documents that contain the literal 
**   token "1st", but not "first" (assuming the tokenizer is not able to
**   provide synonyms for prefixes). However, a non-prefix query like '1st'
**   will match against "1st" and "first". This method does not require
**   extra disk space, as no extra entries are added to the FTS index. 
**   On the other hand, it may require more CPU cycles to run MATCH queries,
**   as separate queries of the FTS index are required for each synonym.
**
**   When using methods (2) or (3), it is important that the tokenizer only
**   provide synonyms when tokenizing document text (method (2)) or query
**   text (method (3)), not both. Doing so will not cause any errors, but is
**   inefficient.
*/
typedef struct Fts5Tokenizer Fts5Tokenizer;
typedef struct fts5_tokenizer fts5_tokenizer;
struct fts5_tokenizer {
  int (*xCreate)(void*, const char **azArg, int nArg, Fts5Tokenizer **ppOut);
  void (*xDelete)(Fts5Tokenizer*);
  int (*xTokenize)(Fts5Tokenizer*, 
      void *pCtx,
      int flags,            /* Mask of FTS5_TOKENIZE_* flags */
      const char *pText, int nText, 
      int (*xToken)(
        void *pCtx,         /* Copy of 2nd argument to xTokenize() */
        int tflags,         /* Mask of FTS5_TOKEN_* flags */
        const char *pToken, /* Pointer to buffer containing token */
        int nToken,         /* Size of token in bytes */
        int iStart,         /* Byte offset of token within input text */
        int iEnd            /* Byte offset of end of token within input text */
      )
  );
};

/* Flags that may be passed as the third argument to xTokenize() */
#define FTS5_TOKENIZE_QUERY     0x0001
#define FTS5_TOKENIZE_PREFIX    0x0002
#define FTS5_TOKENIZE_DOCUMENT  0x0004
#define FTS5_TOKENIZE_AUX       0x0008

/* Flags that may be passed by the tokenizer implementation back to FTS5
** as the third argument to the supplied xToken callback. */
#define FTS5_TOKEN_COLOCATED    0x0001      /* Same position as prev. token */

/*
** END OF CUSTOM TOKENIZERS
*************************************************************************/

/*************************************************************************
** FTS5 EXTENSION REGISTRATION API
*/
typedef struct fts5_api fts5_api;
struct fts5_api {
  int iVersion;                   /* Currently always set to 2 */

  /* Create a new tokenizer */
  int (*xCreateTokenizer)(
    fts5_api *pApi,
    const char *zName,
    void *pContext,
    fts5_tokenizer *pTokenizer,
    void (*xDestroy)(void*)
  );

  /* Find an existing tokenizer */
  int (*xFindTokenizer)(
    fts5_api *pApi,
    const char *zName,
    void **ppContext,
    fts5_tokenizer *pTokenizer
  );

  /* Create a new auxiliary function */
  int (*xCreateFunction)(
    fts5_api *pApi,
    const char *zName,
    void *pContext,
    fts5_extension_function xFunction,
    void (*xDestroy)(void*)
  );
};

/*
** END OF REGISTRATION API
*************************************************************************/

#if 0
}  /* end of the 'extern "C"' block */
#endif

#endif /* _FTS5_H */


/******** End of fts5.h *********/

/************** End of sqlite3.h *********************************************/
/************** Continuing where we left off in sqliteInt.h ******************/

/*
** Include the configuration header output by 'configure' if we're using the
** autoconf-based build
*/
#ifdef _HAVE_SQLITE_CONFIG_H
#include "config.h"
#endif

/************** Include sqliteLimit.h in the middle of sqliteInt.h ***********/
/************** Begin file sqliteLimit.h *************************************/
/*
** 2007 May 7
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
** This file defines various limits of what SQLite can process.
*/

/*
** The maximum length of a TEXT or BLOB in bytes.   This also
** limits the size of a row in a table or index.
**
** The hard limit is the ability of a 32-bit signed integer
** to count the size: 2^31-1 or 2147483647.
*/
#ifndef SQLITE_MAX_LENGTH
# define SQLITE_MAX_LENGTH 1000000000
#endif

/*
** This is the maximum number of
**
**    * Columns in a table
**    * Columns in an index
**    * Columns in a view
**    * Terms in the SET clause of an UPDATE statement
**    * Terms in the result set of a SELECT statement
**    * Terms in the GROUP BY or ORDER BY clauses of a SELECT statement.
**    * Terms in the VALUES clause of an INSERT statement
**
** The hard upper limit here is 32676.  Most database people will
** tell you that in a well-normalized database, you usually should
** not have more than a dozen or so columns in any table.  And if
** that is the case, there is no point in having more than a few
** dozen values in any of the other situations described above.
*/
#ifndef SQLITE_MAX_COLUMN
# define SQLITE_MAX_COLUMN 2000
#endif

/*
** The maximum length of a single SQL statement in bytes.
**
** It used to be the case that setting this value to zero would
** turn the limit off.  That is no longer true.  It is not possible
** to turn this limit off.
*/
#ifndef SQLITE_MAX_SQL_LENGTH
# define SQLITE_MAX_SQL_LENGTH 1000000000
#endif

/*
** The maximum depth of an expression tree. This is limited to 
** some extent by SQLITE_MAX_SQL_LENGTH. But sometime you might 
** want to place more severe limits on the complexity of an 
** expression.
**
** A value of 0 used to mean that the limit was not enforced.
** But that is no longer true.  The limit is now strictly enforced
** at all times.
*/
#ifndef SQLITE_MAX_EXPR_DEPTH
# define SQLITE_MAX_EXPR_DEPTH 1000
#endif

/*
** The maximum number of terms in a compound SELECT statement.
** The code generator for compound SELECT statements does one
** level of recursion for each term.  A stack overflow can result
** if the number of terms is too large.  In practice, most SQL
** never has more than 3 or 4 terms.  Use a value of 0 to disable
** any limit on the number of terms in a compount SELECT.
*/
#ifndef SQLITE_MAX_COMPOUND_SELECT
# define SQLITE_MAX_COMPOUND_SELECT 500
#endif

/*
** The maximum number of opcodes in a VDBE program.
** Not currently enforced.
*/
#ifndef SQLITE_MAX_VDBE_OP
# define SQLITE_MAX_VDBE_OP 25000
#endif

/*
** The maximum number of arguments to an SQL function.
*/
#ifndef SQLITE_MAX_FUNCTION_ARG
# define SQLITE_MAX_FUNCTION_ARG 127
#endif

/*
** The suggested maximum number of in-memory pages to use for
** the main database table and for temporary tables.
**
** IMPLEMENTATION-OF: R-30185-15359 The default suggested cache size is -2000,
** which means the cache size is limited to 2048000 bytes of memory.
** IMPLEMENTATION-OF: R-48205-43578 The default suggested cache size can be
** altered using the SQLITE_DEFAULT_CACHE_SIZE compile-time options.
*/
#ifndef SQLITE_DEFAULT_CACHE_SIZE
# define SQLITE_DEFAULT_CACHE_SIZE  -2000
#endif

/*
** The default number of frames to accumulate in the log file before
** checkpointing the database in WAL mode.
*/
#ifndef SQLITE_DEFAULT_WAL_AUTOCHECKPOINT
# define SQLITE_DEFAULT_WAL_AUTOCHECKPOINT  1000
#endif

/*
** The maximum number of attached databases.  This must be between 0
** and 125.  The upper bound of 125 is because the attached databases are
** counted using a signed 8-bit integer which has a maximum value of 127
** and we have to allow 2 extra counts for the "main" and "temp" databases.
*/
#ifndef SQLITE_MAX_ATTACHED
# define SQLITE_MAX_ATTACHED 10
#endif


/*
** The maximum value of a ?nnn wildcard that the parser will accept.
*/
#ifndef SQLITE_MAX_VARIABLE_NUMBER
# define SQLITE_MAX_VARIABLE_NUMBER 999
#endif

/* Maximum page size.  The upper bound on this value is 65536.  This a limit
** imposed by the use of 16-bit offsets within each page.
**
** Earlier versions of SQLite allowed the user to change this value at
** compile time. This is no longer permitted, on the grounds that it creates
** a library that is technically incompatible with an SQLite library 
** compiled with a different limit. If a process operating on a database 
** with a page-size of 65536 bytes crashes, then an instance of SQLite 
** compiled with the default page-size limit will not be able to rollback 
** the aborted transaction. This could lead to database corruption.
*/
#ifdef SQLITE_MAX_PAGE_SIZE
# undef SQLITE_MAX_PAGE_SIZE
#endif
#define SQLITE_MAX_PAGE_SIZE 65536


/*
** The default size of a database page.
*/
#ifndef SQLITE_DEFAULT_PAGE_SIZE
# define SQLITE_DEFAULT_PAGE_SIZE 4096
#endif
#if SQLITE_DEFAULT_PAGE_SIZE>SQLITE_MAX_PAGE_SIZE
# undef SQLITE_DEFAULT_PAGE_SIZE
# define SQLITE_DEFAULT_PAGE_SIZE SQLITE_MAX_PAGE_SIZE
#endif

/*
** Ordinarily, if no value is explicitly provided, SQLite creates databases
** with page size SQLITE_DEFAULT_PAGE_SIZE. However, based on certain
** device characteristics (sector-size and atomic write() support),
** SQLite may choose a larger value. This constant is the maximum value
** SQLite will choose on its own.
*/
#ifndef SQLITE_MAX_DEFAULT_PAGE_SIZE
# define SQLITE_MAX_DEFAULT_PAGE_SIZE 8192
#endif
#if SQLITE_MAX_DEFAULT_PAGE_SIZE>SQLITE_MAX_PAGE_SIZE
# undef SQLITE_MAX_DEFAULT_PAGE_SIZE
# define SQLITE_MAX_DEFAULT_PAGE_SIZE SQLITE_MAX_PAGE_SIZE
#endif


/*
** Maximum number of pages in one database file.
**
** This is really just the default value for the max_page_count pragma.
** This value can be lowered (or raised) at run-time using that the
** max_page_count macro.
*/
#ifndef SQLITE_MAX_PAGE_COUNT
# define SQLITE_MAX_PAGE_COUNT 1073741823
#endif

/*
** Maximum length (in bytes) of the pattern in a LIKE or GLOB
** operator.
*/
#ifndef SQLITE_MAX_LIKE_PATTERN_LENGTH
# define SQLITE_MAX_LIKE_PATTERN_LENGTH 50000
#endif

/*
** Maximum depth of recursion for triggers.
**
** A value of 1 means that a trigger program will not be able to itself
** fire any triggers. A value of 0 means that no trigger programs at all 
** may be executed.
*/
#ifndef SQLITE_MAX_TRIGGER_DEPTH
# define SQLITE_MAX_TRIGGER_DEPTH 1000
#endif

/************** End of sqliteLimit.h *****************************************/
/************** Continuing where we left off in sqliteInt.h ******************/

/* Disable nuisance warnings on Borland compilers */
#if defined(__BORLANDC__)
#pragma warn -rch /* unreachable code */
#pragma warn -ccc /* Condition is always true or false */
#pragma warn -aus /* Assigned value is never used */
#pragma warn -csu /* Comparing signed and unsigned */
#pragma warn -spa /* Suspicious pointer arithmetic */
#endif

/*
** Include standard header files as necessary
*/
#ifdef HAVE_STDINT_H
#include <stdint.h>
#endif
#ifdef HAVE_INTTYPES_H
#include <inttypes.h>
#endif

/*
** The following macros are used to cast pointers to integers and
** integers to pointers.  The way you do this varies from one compiler
** to the next, so we have developed the following set of #if statements
** to generate appropriate macros for a wide range of compilers.
**
** The correct "ANSI" way to do this is to use the intptr_t type.
** Unfortunately, that typedef is not available on all compilers, or
** if it is available, it requires an #include of specific headers
** that vary from one machine to the next.
**
** Ticket #3860:  The llvm-gcc-4.2 compiler from Apple chokes on
** the ((void*)&((char*)0)[X]) construct.  But MSVC chokes on ((void*)(X)).
** So we have to define the macros in different ways depending on the
** compiler.
*/
#if defined(__PTRDIFF_TYPE__)  /* This case should work for GCC */
# define SQLITE_INT_TO_PTR(X)  ((void*)(__PTRDIFF_TYPE__)(X))
# define SQLITE_PTR_TO_INT(X)  ((int)(__PTRDIFF_TYPE__)(X))
#elif !defined(__GNUC__)       /* Works for compilers other than LLVM */
# define SQLITE_INT_TO_PTR(X)  ((void*)&((char*)0)[X])
# define SQLITE_PTR_TO_INT(X)  ((int)(((char*)X)-(char*)0))
#elif defined(HAVE_STDINT_H)   /* Use this case if we have ANSI headers */
# define SQLITE_INT_TO_PTR(X)  ((void*)(intptr_t)(X))
# define SQLITE_PTR_TO_INT(X)  ((int)(intptr_t)(X))
#else                          /* Generates a warning - but it always works */
# define SQLITE_INT_TO_PTR(X)  ((void*)(X))
# define SQLITE_PTR_TO_INT(X)  ((int)(X))
#endif

/*
** A macro to hint to the compiler that a function should not be
** inlined.
*/
#if defined(__GNUC__)
#  define SQLITE_NOINLINE  __attribute__((noinline))
#elif defined(_MSC_VER) && _MSC_VER>=1310
#  define SQLITE_NOINLINE  __declspec(noinline)
#else
#  define SQLITE_NOINLINE
#endif

/*
** Make sure that the compiler intrinsics we desire are enabled when
** compiling with an appropriate version of MSVC unless prevented by
** the SQLITE_DISABLE_INTRINSIC define.
*/
#if !defined(SQLITE_DISABLE_INTRINSIC)
#  if defined(_MSC_VER) && _MSC_VER>=1300
#    if !defined(_WIN32_WCE)
#      include <intrin.h>
#      pragma intrinsic(_byteswap_ushort)
#      pragma intrinsic(_byteswap_ulong)
#      pragma intrinsic(_ReadWriteBarrier)
#    else
#      include <cmnintrin.h>
#    endif
#  endif
#endif

/*
** The SQLITE_THREADSAFE macro must be defined as 0, 1, or 2.
** 0 means mutexes are permanently disable and the library is never
** threadsafe.  1 means the library is serialized which is the highest
** level of threadsafety.  2 means the library is multithreaded - multiple
** threads can use SQLite as long as no two threads try to use the same
** database connection at the same time.
**
** Older versions of SQLite used an optional THREADSAFE macro.
** We support that for legacy.
*/
#if !defined(SQLITE_THREADSAFE)
# if defined(THREADSAFE)
#   define SQLITE_THREADSAFE THREADSAFE
# else
#   define SQLITE_THREADSAFE 1 /* IMP: R-07272-22309 */
# endif
#endif

/*
** Powersafe overwrite is on by default.  But can be turned off using
** the -DSQLITE_POWERSAFE_OVERWRITE=0 command-line option.
*/
#ifndef SQLITE_POWERSAFE_OVERWRITE
# define SQLITE_POWERSAFE_OVERWRITE 1
#endif

/*
** EVIDENCE-OF: R-25715-37072 Memory allocation statistics are enabled by
** default unless SQLite is compiled with SQLITE_DEFAULT_MEMSTATUS=0 in
** which case memory allocation statistics are disabled by default.
*/
#if !defined(SQLITE_DEFAULT_MEMSTATUS)
# define SQLITE_DEFAULT_MEMSTATUS 1
#endif

/*
** Exactly one of the following macros must be defined in order to
** specify which memory allocation subsystem to use.
**
**     SQLITE_SYSTEM_MALLOC          // Use normal system malloc()
**     SQLITE_WIN32_MALLOC           // Use Win32 native heap API
**     SQLITE_ZERO_MALLOC            // Use a stub allocator that always fails
**     SQLITE_MEMDEBUG               // Debugging version of system malloc()
**
** On Windows, if the SQLITE_WIN32_MALLOC_VALIDATE macro is defined and the
** assert() macro is enabled, each call into the Win32 native heap subsystem
** will cause HeapValidate to be called.  If heap validation should fail, an
** assertion will be triggered.
**
** If none of the above are defined, then set SQLITE_SYSTEM_MALLOC as
** the default.
*/
#if defined(SQLITE_SYSTEM_MALLOC) \
  + defined(SQLITE_WIN32_MALLOC) \
  + defined(SQLITE_ZERO_MALLOC) \
  + defined(SQLITE_MEMDEBUG)>1
# error "Two or more of the following compile-time configuration options\
 are defined but at most one is allowed:\
 SQLITE_SYSTEM_MALLOC, SQLITE_WIN32_MALLOC, SQLITE_MEMDEBUG,\
 SQLITE_ZERO_MALLOC"
#endif
#if defined(SQLITE_SYSTEM_MALLOC) \
  + defined(SQLITE_WIN32_MALLOC) \
  + defined(SQLITE_ZERO_MALLOC) \
  + defined(SQLITE_MEMDEBUG)==0
# define SQLITE_SYSTEM_MALLOC 1
#endif

/*
** If SQLITE_MALLOC_SOFT_LIMIT is not zero, then try to keep the
** sizes of memory allocations below this value where possible.
*/
#if !defined(SQLITE_MALLOC_SOFT_LIMIT)
# define SQLITE_MALLOC_SOFT_LIMIT 1024
#endif

/*
** We need to define _XOPEN_SOURCE as follows in order to enable
** recursive mutexes on most Unix systems and fchmod() on OpenBSD.
** But _XOPEN_SOURCE define causes problems for Mac OS X, so omit
** it.
*/
#if !defined(_XOPEN_SOURCE) && !defined(__DARWIN__) && !defined(__APPLE__)
#  define _XOPEN_SOURCE 600
#endif

/*
** NDEBUG and SQLITE_DEBUG are opposites.  It should always be true that
** defined(NDEBUG)==!defined(SQLITE_DEBUG).  If this is not currently true,
** make it true by defining or undefining NDEBUG.
**
** Setting NDEBUG makes the code smaller and faster by disabling the
** assert() statements in the code.  So we want the default action
** to be for NDEBUG to be set and NDEBUG to be undefined only if SQLITE_DEBUG
** is set.  Thus NDEBUG becomes an opt-in rather than an opt-out
** feature.
*/
#if !defined(NDEBUG) && !defined(SQLITE_DEBUG)
# define NDEBUG 1
#endif
#if defined(NDEBUG) && defined(SQLITE_DEBUG)
# undef NDEBUG
#endif

/*
** Enable SQLITE_ENABLE_EXPLAIN_COMMENTS if SQLITE_DEBUG is turned on.
*/
#if !defined(SQLITE_ENABLE_EXPLAIN_COMMENTS) && defined(SQLITE_DEBUG)
# define SQLITE_ENABLE_EXPLAIN_COMMENTS 1
#endif

/*
** The testcase() macro is used to aid in coverage testing.  When
** doing coverage testing, the condition inside the argument to
** testcase() must be evaluated both true and false in order to
** get full branch coverage.  The testcase() macro is inserted
** to help ensure adequate test coverage in places where simple
** condition/decision coverage is inadequate.  For example, testcase()
** can be used to make sure boundary values are tested.  For
** bitmask tests, testcase() can be used to make sure each bit
** is significant and used at least once.  On switch statements
** where multiple cases go to the same block of code, testcase()
** can insure that all cases are evaluated.
**
*/
#ifdef SQLITE_COVERAGE_TEST
SQLITE_PRIVATE   void sqlite3Coverage(int);
# define testcase(X)  if( X ){ sqlite3Coverage(__LINE__); }
#else
# define testcase(X)
#endif

/*
** The TESTONLY macro is used to enclose variable declarations or
** other bits of code that are needed to support the arguments
** within testcase() and assert() macros.
*/
#if !defined(NDEBUG) || defined(SQLITE_COVERAGE_TEST)
# define TESTONLY(X)  X
#else
# define TESTONLY(X)
#endif

/*
** Sometimes we need a small amount of code such as a variable initialization
** to setup for a later assert() statement.  We do not want this code to
** appear when assert() is disabled.  The following macro is therefore
** used to contain that setup code.  The "VVA" acronym stands for
** "Verification, Validation, and Accreditation".  In other words, the
** code within VVA_ONLY() will only run during verification processes.
*/
#ifndef NDEBUG
# define VVA_ONLY(X)  X
#else
# define VVA_ONLY(X)
#endif

/*
** The ALWAYS and NEVER macros surround boolean expressions which
** are intended to always be true or false, respectively.  Such
** expressions could be omitted from the code completely.  But they
** are included in a few cases in order to enhance the resilience
** of SQLite to unexpected behavior - to make the code "self-healing"
** or "ductile" rather than being "brittle" and crashing at the first
** hint of unplanned behavior.
**
** In other words, ALWAYS and NEVER are added for defensive code.
**
** When doing coverage testing ALWAYS and NEVER are hard-coded to
** be true and false so that the unreachable code they specify will
** not be counted as untested code.
*/
#if defined(SQLITE_COVERAGE_TEST) || defined(SQLITE_MUTATION_TEST)
# define ALWAYS(X)      (1)
# define NEVER(X)       (0)
#elif !defined(NDEBUG)
# define ALWAYS(X)      ((X)?1:(assert(0),0))
# define NEVER(X)       ((X)?(assert(0),1):0)
#else
# define ALWAYS(X)      (X)
# define NEVER(X)       (X)
#endif

/*
** Some malloc failures are only possible if SQLITE_TEST_REALLOC_STRESS is
** defined.  We need to defend against those failures when testing with
** SQLITE_TEST_REALLOC_STRESS, but we don't want the unreachable branches
** during a normal build.  The following macro can be used to disable tests
** that are always false except when SQLITE_TEST_REALLOC_STRESS is set.
*/
#if defined(SQLITE_TEST_REALLOC_STRESS)
# define ONLY_IF_REALLOC_STRESS(X)  (X)
#elif !defined(NDEBUG)
# define ONLY_IF_REALLOC_STRESS(X)  ((X)?(assert(0),1):0)
#else
# define ONLY_IF_REALLOC_STRESS(X)  (0)
#endif

/*
** Declarations used for tracing the operating system interfaces.
*/
#if defined(SQLITE_FORCE_OS_TRACE) || defined(SQLITE_TEST) || \
    (defined(SQLITE_DEBUG) && SQLITE_OS_WIN)
  extern int sqlite3OSTrace;
# define OSTRACE(X)          if( sqlite3OSTrace ) sqlite3DebugPrintf X
# define SQLITE_HAVE_OS_TRACE
#else
# define OSTRACE(X)
# undef  SQLITE_HAVE_OS_TRACE
#endif

/*
** Is the sqlite3ErrName() function needed in the build?  Currently,
** it is needed by "mutex_w32.c" (when debugging), "os_win.c" (when
** OSTRACE is enabled), and by several "test*.c" files (which are
** compiled using SQLITE_TEST).
*/
#if defined(SQLITE_HAVE_OS_TRACE) || defined(SQLITE_TEST) || \
    (defined(SQLITE_DEBUG) && SQLITE_OS_WIN)
# define SQLITE_NEED_ERR_NAME
#else
# undef  SQLITE_NEED_ERR_NAME
#endif

/*
** SQLITE_ENABLE_EXPLAIN_COMMENTS is incompatible with SQLITE_OMIT_EXPLAIN
*/
#ifdef SQLITE_OMIT_EXPLAIN
# undef SQLITE_ENABLE_EXPLAIN_COMMENTS
#endif

/*
** Return true (non-zero) if the input is an integer that is too large
** to fit in 32-bits.  This macro is used inside of various testcase()
** macros to verify that we have tested SQLite for large-file support.
*/
#define IS_BIG_INT(X)  (((X)&~(i64)0xffffffff)!=0)

/*
** The macro unlikely() is a hint that surrounds a boolean
** expression that is usually false.  Macro likely() surrounds
** a boolean expression that is usually true.  These hints could,
** in theory, be used by the compiler to generate better code, but
** currently they are just comments for human readers.
*/
#define likely(X)    (X)
#define unlikely(X)  (X)

/************** Include hash.h in the middle of sqliteInt.h ******************/
/************** Begin file hash.h ********************************************/
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
** This is the header file for the generic hash-table implementation
** used in SQLite.
*/
#ifndef _SQLITE_HASH_H_
#define _SQLITE_HASH_H_

/* Forward declarations of structures. */
typedef struct Hash Hash;
typedef struct HashElem HashElem;

/* A complete hash table is an instance of the following structure.
** The internals of this structure are intended to be opaque -- client
** code should not attempt to access or modify the fields of this structure
** directly.  Change this structure only by using the routines below.
** However, some of the "procedures" and "functions" for modifying and
** accessing this structure are really macros, so we can't really make
** this structure opaque.
**
** All elements of the hash table are on a single doubly-linked list.
** Hash.first points to the head of this list.
**
** There are Hash.htsize buckets.  Each bucket points to a spot in
** the global doubly-linked list.  The contents of the bucket are the
** element pointed to plus the next _ht.count-1 elements in the list.
**
** Hash.htsize and Hash.ht may be zero.  In that case lookup is done
** by a linear search of the global list.  For small tables, the 
** Hash.ht table is never allocated because if there are few elements
** in the table, it is faster to do a linear search than to manage
** the hash table.
*/
struct Hash {
  unsigned int htsize;      /* Number of buckets in the hash table */
  unsigned int count;       /* Number of entries in this table */
  HashElem *first;          /* The first element of the array */
  struct _ht {              /* the hash table */
    int count;                 /* Number of entries with this hash */
    HashElem *chain;           /* Pointer to first entry with this hash */
  } *ht;
};

/* Each element in the hash table is an instance of the following 
** structure.  All elements are stored on a single doubly-linked list.
**
** Again, this structure is intended to be opaque, but it can't really
** be opaque because it is used by macros.
*/
struct HashElem {
  HashElem *next, *prev;       /* Next and previous elements in the table */
  void *data;                  /* Data associated with this element */
  const char *pKey;            /* Key associated with this element */
};

/*
** Access routines.  To delete, insert a NULL pointer.
*/
SQLITE_PRIVATE void sqlite3HashInit(Hash*);
SQLITE_PRIVATE void *sqlite3HashInsert(Hash*, const char *pKey, void *pData);
SQLITE_PRIVATE void *sqlite3HashFind(const Hash*, const char *pKey);
SQLITE_PRIVATE void sqlite3HashClear(Hash*);

/*
** Macros for looping over all elements of a hash table.  The idiom is
** like this:
**
**   Hash h;
**   HashElem *p;
**   ...
**   for(p=sqliteHashFirst(&h); p; p=sqliteHashNext(p)){
**     SomeStructure *pData = sqliteHashData(p);
**     // do something with pData
**   }
*/
#define sqliteHashFirst(H)  ((H)->first)
#define sqliteHashNext(E)   ((E)->next)
#define sqliteHashData(E)   ((E)->data)
/* #define sqliteHashKey(E)    ((E)->pKey) // NOT USED */
/* #define sqliteHashKeysize(E) ((E)->nKey)  // NOT USED */

/*
** Number of entries in a hash table
*/
/* #define sqliteHashCount(H)  ((H)->count) // NOT USED */

#endif /* _SQLITE_HASH_H_ */

/************** End of hash.h ************************************************/
/************** Continuing where we left off in sqliteInt.h ******************/
/************** Include parse.h in the middle of sqliteInt.h *****************/
/************** Begin file parse.h *******************************************/
#define TK_SEMI                             1
#define TK_EXPLAIN                          2
#define TK_QUERY                            3
#define TK_PLAN                             4
#define TK_BEGIN                            5
#define TK_TRANSACTION                      6
#define TK_DEFERRED                         7
#define TK_IMMEDIATE                        8
#define TK_EXCLUSIVE                        9
#define TK_COMMIT                          10
#define TK_END                             11
#define TK_ROLLBACK                        12
#define TK_SAVEPOINT                       13
#define TK_RELEASE                         14
#define TK_TO                              15
#define TK_TABLE                           16
#define TK_CREATE                          17
#define TK_IF                              18
#define TK_NOT                             19
#define TK_EXISTS                          20
#define TK_TEMP                            21
#define TK_LP                              22
#define TK_RP                              23
#define TK_AS                              24
#define TK_WITHOUT                         25
#define TK_COMMA                           26
#define TK_OR                              27
#define TK_AND                             28
#define TK_IS                              29
#define TK_MATCH                           30
#define TK_LIKE_KW                         31
#define TK_BETWEEN                         32
#define TK_IN                              33
#define TK_ISNULL                          34
#define TK_NOTNULL                         35
#define TK_NE                              36
#define TK_EQ                              37
#define TK_GT                              38
#define TK_LE                              39
#define TK_LT                              40
#define TK_GE                              41
#define TK_ESCAPE                          42
#define TK_BITAND                          43
#define TK_BITOR                           44
#define TK_LSHIFT                          45
#define TK_RSHIFT                          46
#define TK_PLUS                            47
#define TK_MINUS                           48
#define TK_STAR                            49
#define TK_SLASH                           50
#define TK_REM                             51
#define TK_CONCAT                          52
#define TK_COLLATE                         53
#define TK_BITNOT                          54
#define TK_ID                              55
#define TK_INDEXED                         56
#define TK_ABORT                           57
#define TK_ACTION                          58
#define TK_AFTER                           59
#define TK_ANALYZE                         60
#define TK_ASC                             61
#define TK_ATTACH                          62
#define TK_BEFORE                          63
#define TK_BY                              64
#define TK_CASCADE                         65
#define TK_CAST                            66
#define TK_COLUMNKW                        67
#define TK_CONFLICT                        68
#define TK_DATABASE                        69
#define TK_DESC                            70
#define TK_DETACH                          71
#define TK_EACH                            72
#define TK_FAIL                            73
#define TK_FOR                             74
#define TK_IGNORE                          75
#define TK_INITIALLY                       76
#define TK_INSTEAD                         77
#define TK_NO                              78
#define TK_KEY                             79
#define TK_OF                              80
#define TK_OFFSET                          81
#define TK_PRAGMA                          82
#define TK_RAISE                           83
#define TK_RECURSIVE                       84
#define TK_REPLACE                         85
#define TK_RESTRICT                        86
#define TK_ROW                             87
#define TK_TRIGGER                         88
#define TK_VACUUM                          89
#define TK_VIEW                            90
#define TK_VIRTUAL                         91
#define TK_WITH                            92
#define TK_REINDEX                         93
#define TK_RENAME                          94
#define TK_CTIME_KW                        95
#define TK_ANY                             96
#define TK_STRING                          97
#define TK_JOIN_KW                         98
#define TK_CONSTRAINT                      99
#define TK_DEFAULT                        100
#define TK_NULL                           101
#define TK_PRIMARY                        102
#define TK_UNIQUE                         103
#define TK_CHECK                          104
#define TK_REFERENCES                     105
#define TK_AUTOINCR                       106
#define TK_ON                             107
#define TK_INSERT                         108
#define TK_DELETE                         109
#define TK_UPDATE                         110
#define TK_SET                            111
#define TK_DEFERRABLE                     112
#define TK_FOREIGN                        113
#define TK_DROP                           114
#define TK_UNION                          115
#define TK_ALL                            116
#define TK_EXCEPT                         117
#define TK_INTERSECT                      118
#define TK_SELECT                         119
#define TK_VALUES                         120
#define TK_DISTINCT                       121
#define TK_DOT                            122
#define TK_FROM                           123
#define TK_JOIN                           124
#define TK_USING                          125
#define TK_ORDER                          126
#define TK_GROUP                          127
#define TK_HAVING                         128
#define TK_LIMIT                          129
#define TK_WHERE                          130
#define TK_INTO                           131
#define TK_INTEGER                        132
#define TK_FLOAT                          133
#define TK_BLOB                           134
#define TK_VARIABLE                       135
#define TK_CASE                           136
#define TK_WHEN                           137
#define TK_THEN                           138
#define TK_ELSE                           139
#define TK_INDEX                          140
#define TK_ALTER                          141
#define TK_ADD                            142
#define TK_TO_TEXT                        143
#define TK_TO_BLOB                        144
#define TK_TO_NUMERIC                     145
#define TK_TO_INT                         146
#define TK_TO_REAL                        147
#define TK_ISNOT                          148
#define TK_END_OF_FILE                    149
#define TK_UNCLOSED_STRING                150
#define TK_FUNCTION                       151
#define TK_COLUMN                         152
#define TK_AGG_FUNCTION                   153
#define TK_AGG_COLUMN                     154
#define TK_UMINUS                         155
#define TK_UPLUS                          156
#define TK_REGISTER                       157
#define TK_ASTERISK                       158
#define TK_SPAN                           159
#define TK_SPACE                          160
#define TK_ILLEGAL                        161

/* The token codes above must all fit in 8 bits */
#define TKFLG_MASK           0xff  

/* Flags that can be added to a token code when it is not
** being stored in a u8: */
#define TKFLG_DONTFOLD       0x100  /* Omit constant folding optimizations */

/************** End of parse.h ***********************************************/
/************** Continuing where we left off in sqliteInt.h ******************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <stddef.h>

/*
** If compiling for a processor that lacks floating point support,
** substitute integer for floating-point
*/
#ifdef SQLITE_OMIT_FLOATING_POINT
# define double sqlite_int64
# define float sqlite_int64
# define LONGDOUBLE_TYPE sqlite_int64
# ifndef SQLITE_BIG_DBL
#   define SQLITE_BIG_DBL (((sqlite3_int64)1)<<50)
# endif
# define SQLITE_OMIT_DATETIME_FUNCS 1
# define SQLITE_OMIT_TRACE 1
# undef SQLITE_MIXED_ENDIAN_64BIT_FLOAT
# undef SQLITE_HAVE_ISNAN
#endif
#ifndef SQLITE_BIG_DBL
# define SQLITE_BIG_DBL (1e99)
#endif

/*
** OMIT_TEMPDB is set to 1 if SQLITE_OMIT_TEMPDB is defined, or 0
** afterward. Having this macro allows us to cause the C compiler
** to omit code used by TEMP tables without messy #ifndef statements.
*/
#ifdef SQLITE_OMIT_TEMPDB
#define OMIT_TEMPDB 1
#else
#define OMIT_TEMPDB 0
#endif

/*
** The "file format" number is an integer that is incremented whenever
** the VDBE-level file format changes.  The following macros define the
** the default file format for new databases and the maximum file format
** that the library can read.
*/
#define SQLITE_MAX_FILE_FORMAT 4
#ifndef SQLITE_DEFAULT_FILE_FORMAT
# define SQLITE_DEFAULT_FILE_FORMAT 4
#endif

/*
** Determine whether triggers are recursive by default.  This can be
** changed at run-time using a pragma.
*/
#ifndef SQLITE_DEFAULT_RECURSIVE_TRIGGERS
# define SQLITE_DEFAULT_RECURSIVE_TRIGGERS 0
#endif

/*
** Provide a default value for SQLITE_TEMP_STORE in case it is not specified
** on the command-line
*/
#ifndef SQLITE_TEMP_STORE
# define SQLITE_TEMP_STORE 1
# define SQLITE_TEMP_STORE_xc 1  /* Exclude from ctime.c */
#endif

/*
** If no value has been provided for SQLITE_MAX_WORKER_THREADS, or if
** SQLITE_TEMP_STORE is set to 3 (never use temporary files), set it
** to zero.
*/
#if SQLITE_TEMP_STORE==3 || SQLITE_THREADSAFE==0
# undef SQLITE_MAX_WORKER_THREADS
# define SQLITE_MAX_WORKER_THREADS 0
#endif
#ifndef SQLITE_MAX_WORKER_THREADS
# define SQLITE_MAX_WORKER_THREADS 8
#endif
#ifndef SQLITE_DEFAULT_WORKER_THREADS
# define SQLITE_DEFAULT_WORKER_THREADS 0
#endif
#if SQLITE_DEFAULT_WORKER_THREADS>SQLITE_MAX_WORKER_THREADS
# undef SQLITE_MAX_WORKER_THREADS
# define SQLITE_MAX_WORKER_THREADS SQLITE_DEFAULT_WORKER_THREADS
#endif

/*
** The default initial allocation for the pagecache when using separate
** pagecaches for each database connection.  A positive number is the
** number of pages.  A negative number N translations means that a buffer
** of -1024*N bytes is allocated and used for as many pages as it will hold.
*/
#ifndef SQLITE_DEFAULT_PCACHE_INITSZ
# define SQLITE_DEFAULT_PCACHE_INITSZ 100
#endif

/*
** GCC does not define the offsetof() macro so we'll have to do it
** ourselves.
*/
#ifndef offsetof
#define offsetof(STRUCTURE,FIELD) ((int)((char*)&((STRUCTURE*)0)->FIELD))
#endif

/*
** Macros to compute minimum and maximum of two numbers.
*/
#ifndef MIN
# define MIN(A,B) ((A)<(B)?(A):(B))
#endif
#ifndef MAX
# define MAX(A,B) ((A)>(B)?(A):(B))
#endif

/*
** Swap two objects of type TYPE.
*/
#define SWAP(TYPE,A,B) {TYPE t=A; A=B; B=t;}

/*
** Check to see if this machine uses EBCDIC.  (Yes, believe it or
** not, there are still machines out there that use EBCDIC.)
*/
#if 'A' == '\301'
# define SQLITE_EBCDIC 1
#else
# define SQLITE_ASCII 1
#endif

/*
** Integers of known sizes.  These typedefs might change for architectures
** where the sizes very.  Preprocessor macros are available so that the
** types can be conveniently redefined at compile-type.  Like this:
**
**         cc '-DUINTPTR_TYPE=long long int' ...
*/
#ifndef UINT32_TYPE
# ifdef HAVE_UINT32_T
#  define UINT32_TYPE uint32_t
# else
#  define UINT32_TYPE unsigned int
# endif
#endif
#ifndef UINT16_TYPE
# ifdef HAVE_UINT16_T
#  define UINT16_TYPE uint16_t
# else
#  define UINT16_TYPE unsigned short int
# endif
#endif
#ifndef INT16_TYPE
# ifdef HAVE_INT16_T
#  define INT16_TYPE int16_t
# else
#  define INT16_TYPE short int
# endif
#endif
#ifndef UINT8_TYPE
# ifdef HAVE_UINT8_T
#  define UINT8_TYPE uint8_t
# else
#  define UINT8_TYPE unsigned char
# endif
#endif
#ifndef INT8_TYPE
# ifdef HAVE_INT8_T
#  define INT8_TYPE int8_t
# else
#  define INT8_TYPE signed char
# endif
#endif
#ifndef LONGDOUBLE_TYPE
# define LONGDOUBLE_TYPE long double
#endif
typedef sqlite_int64 i64;          /* 8-byte signed integer */
typedef sqlite_uint64 u64;         /* 8-byte unsigned integer */
typedef UINT32_TYPE u32;           /* 4-byte unsigned integer */
typedef UINT16_TYPE u16;           /* 2-byte unsigned integer */
typedef INT16_TYPE i16;            /* 2-byte signed integer */
typedef UINT8_TYPE u8;             /* 1-byte unsigned integer */
typedef INT8_TYPE i8;              /* 1-byte signed integer */

/*
** SQLITE_MAX_U32 is a u64 constant that is the maximum u64 value
** that can be stored in a u32 without loss of data.  The value
** is 0x00000000ffffffff.  But because of quirks of some compilers, we
** have to specify the value in the less intuitive manner shown:
*/
#define SQLITE_MAX_U32  ((((u64)1)<<32)-1)

/*
** The datatype used to store estimates of the number of rows in a
** table or index.  This is an unsigned integer type.  For 99.9% of
** the world, a 32-bit integer is sufficient.  But a 64-bit integer
** can be used at compile-time if desired.
*/
#ifdef SQLITE_64BIT_STATS
 typedef u64 tRowcnt;    /* 64-bit only if requested at compile-time */
#else
 typedef u32 tRowcnt;    /* 32-bit is the default */
#endif

/*
** Estimated quantities used for query planning are stored as 16-bit
** logarithms.  For quantity X, the value stored is 10*log2(X).  This
** gives a possible range of values of approximately 1.0e986 to 1e-986.
** But the allowed values are "grainy".  Not every value is representable.
** For example, quantities 16 and 17 are both represented by a LogEst
** of 40.  However, since LogEst quantities are suppose to be estimates,
** not exact values, this imprecision is not a problem.
**
** "LogEst" is short for "Logarithmic Estimate".
**
** Examples:
**      1 -> 0              20 -> 43          10000 -> 132
**      2 -> 10             25 -> 46          25000 -> 146
**      3 -> 16            100 -> 66        1000000 -> 199
**      4 -> 20           1000 -> 99        1048576 -> 200
**     10 -> 33           1024 -> 100    4294967296 -> 320
**
** The LogEst can be negative to indicate fractional values.
** Examples:
**
**    0.5 -> -10           0.1 -> -33        0.0625 -> -40
*/
typedef INT16_TYPE LogEst;

/*
** Set the SQLITE_PTRSIZE macro to the number of bytes in a pointer
*/
#ifndef SQLITE_PTRSIZE
# if defined(__SIZEOF_POINTER__)
#   define SQLITE_PTRSIZE __SIZEOF_POINTER__
# elif defined(i386)     || defined(__i386__)   || defined(_M_IX86) ||    \
       defined(_M_ARM)   || defined(__arm__)    || defined(__x86)
#   define SQLITE_PTRSIZE 4
# else
#   define SQLITE_PTRSIZE 8
# endif
#endif

/* The uptr type is an unsigned integer large enough to hold a pointer
*/
#if defined(HAVE_STDINT_H)
  typedef uintptr_t uptr;
#elif SQLITE_PTRSIZE==4
  typedef u32 uptr;
#else
  typedef u64 uptr;
#endif

/*
** The SQLITE_WITHIN(P,S,E) macro checks to see if pointer P points to
** something between S (inclusive) and E (exclusive).
**
** In other words, S is a buffer and E is a pointer to the first byte after
** the end of buffer S.  This macro returns true if P points to something
** contained within the buffer S.
*/
#define SQLITE_WITHIN(P,S,E) (((uptr)(P)>=(uptr)(S))&&((uptr)(P)<(uptr)(E)))


/*
** Macros to determine whether the machine is big or little endian,
** and whether or not that determination is run-time or compile-time.
**
** For best performance, an attempt is made to guess at the byte-order
** using C-preprocessor macros.  If that is unsuccessful, or if
** -DSQLITE_RUNTIME_BYTEORDER=1 is set, then byte-order is determined
** at run-time.
*/
#if (defined(i386)     || defined(__i386__)   || defined(_M_IX86) ||    \
     defined(__x86_64) || defined(__x86_64__) || defined(_M_X64)  ||    \
     defined(_M_AMD64) || defined(_M_ARM)     || defined(__x86)   ||    \
     defined(__arm__)) && !defined(SQLITE_RUNTIME_BYTEORDER)
# define SQLITE_BYTEORDER    1234
# define SQLITE_BIGENDIAN    0
# define SQLITE_LITTLEENDIAN 1
# define SQLITE_UTF16NATIVE  SQLITE_UTF16LE
#endif
#if (defined(sparc)    || defined(__ppc__))  \
    && !defined(SQLITE_RUNTIME_BYTEORDER)
# define SQLITE_BYTEORDER    4321
# define SQLITE_BIGENDIAN    1
# define SQLITE_LITTLEENDIAN 0
# define SQLITE_UTF16NATIVE  SQLITE_UTF16BE
#endif
#if !defined(SQLITE_BYTEORDER)
# ifdef SQLITE_AMALGAMATION
  const int sqlite3one = 1;
# else
  extern const int sqlite3one;
# endif
# define SQLITE_BYTEORDER    0     /* 0 means "unknown at compile-time" */
# define SQLITE_BIGENDIAN    (*(char *)(&sqlite3one)==0)
# define SQLITE_LITTLEENDIAN (*(char *)(&sqlite3one)==1)
# define SQLITE_UTF16NATIVE  (SQLITE_BIGENDIAN?SQLITE_UTF16BE:SQLITE_UTF16LE)
#endif

/*
** Constants for the largest and smallest possible 64-bit signed integers.
** These macros are designed to work correctly on both 32-bit and 64-bit
** compilers.
*/
#define LARGEST_INT64  (0xffffffff|(((i64)0x7fffffff)<<32))
#define SMALLEST_INT64 (((i64)-1) - LARGEST_INT64)

/*
** Round up a number to the next larger multiple of 8.  This is used
** to force 8-byte alignment on 64-bit architectures.
*/
#define ROUND8(x)     (((x)+7)&~7)

/*
** Round down to the nearest multiple of 8
*/
#define ROUNDDOWN8(x) ((x)&~7)

/*
** Assert that the pointer X is aligned to an 8-byte boundary.  This
** macro is used only within assert() to verify that the code gets
** all alignment restrictions correct.
**
** Except, if SQLITE_4_BYTE_ALIGNED_MALLOC is defined, then the
** underlying malloc() implementation might return us 4-byte aligned
** pointers.  In that case, only verify 4-byte alignment.
*/
#ifdef SQLITE_4_BYTE_ALIGNED_MALLOC
# define EIGHT_BYTE_ALIGNMENT(X)   ((((char*)(X) - (char*)0)&3)==0)
#else
# define EIGHT_BYTE_ALIGNMENT(X)   ((((char*)(X) - (char*)0)&7)==0)
#endif

/*
** Disable MMAP on platforms where it is known to not work
*/
#if defined(__OpenBSD__) || defined(__QNXNTO__)
# undef SQLITE_MAX_MMAP_SIZE
# define SQLITE_MAX_MMAP_SIZE 0
#endif

/*
** Default maximum size of memory used by memory-mapped I/O in the VFS
*/
#ifdef __APPLE__
# include <TargetConditionals.h>
#endif
#ifndef SQLITE_MAX_MMAP_SIZE
# if defined(__linux__) \
  || defined(_WIN32) \
  || (defined(__APPLE__) && defined(__MACH__)) \
  || defined(__sun) \
  || defined(__FreeBSD__) \
  || defined(__DragonFly__)
#   define SQLITE_MAX_MMAP_SIZE 0x7fff0000  /* 2147418112 */
# else
#   define SQLITE_MAX_MMAP_SIZE 0
# endif
# define SQLITE_MAX_MMAP_SIZE_xc 1 /* exclude from ctime.c */
#endif

/*
** The default MMAP_SIZE is zero on all platforms.  Or, even if a larger
** default MMAP_SIZE is specified at compile-time, make sure that it does
** not exceed the maximum mmap size.
*/
#ifndef SQLITE_DEFAULT_MMAP_SIZE
# define SQLITE_DEFAULT_MMAP_SIZE 0
# define SQLITE_DEFAULT_MMAP_SIZE_xc 1  /* Exclude from ctime.c */
#endif
#if SQLITE_DEFAULT_MMAP_SIZE>SQLITE_MAX_MMAP_SIZE
# undef SQLITE_DEFAULT_MMAP_SIZE
# define SQLITE_DEFAULT_MMAP_SIZE SQLITE_MAX_MMAP_SIZE
#endif

/*
** Only one of SQLITE_ENABLE_STAT3 or SQLITE_ENABLE_STAT4 can be defined.
** Priority is given to SQLITE_ENABLE_STAT4.  If either are defined, also
** define SQLITE_ENABLE_STAT3_OR_STAT4
*/
#ifdef SQLITE_ENABLE_STAT4
# undef SQLITE_ENABLE_STAT3
# define SQLITE_ENABLE_STAT3_OR_STAT4 1
#elif SQLITE_ENABLE_STAT3
# define SQLITE_ENABLE_STAT3_OR_STAT4 1
#elif SQLITE_ENABLE_STAT3_OR_STAT4
# undef SQLITE_ENABLE_STAT3_OR_STAT4
#endif

/*
** SELECTTRACE_ENABLED will be either 1 or 0 depending on whether or not
** the Select query generator tracing logic is turned on.
*/
#if defined(SQLITE_DEBUG) || defined(SQLITE_ENABLE_SELECTTRACE)
# define SELECTTRACE_ENABLED 1
#else
# define SELECTTRACE_ENABLED 0
#endif

/*
** An instance of the following structure is used to store the busy-handler
** callback for a given sqlite handle.
**
** The sqlite.busyHandler member of the sqlite struct contains the busy
** callback for the database handle. Each pager opened via the sqlite
** handle is passed a pointer to sqlite.busyHandler. The busy-handler
** callback is currently invoked only from within pager.c.
*/
typedef struct BusyHandler BusyHandler;
struct BusyHandler {
  int (*xFunc)(void *,int);  /* The busy callback */
  void *pArg;                /* First arg to busy callback */
  int nBusy;                 /* Incremented with each busy call */
};

/*
** Name of the master database table.  The master database table
** is a special table that holds the names and attributes of all
** user tables and indices.
*/
#define MASTER_NAME       "sqlite_master"
#define TEMP_MASTER_NAME  "sqlite_temp_master"

/*
** The root-page of the master database table.
*/
#define MASTER_ROOT       1

/*
** The name of the schema table.
*/
#define SCHEMA_TABLE(x)  ((!OMIT_TEMPDB)&&(x==1)?TEMP_MASTER_NAME:MASTER_NAME)

/*
** A convenience macro that returns the number of elements in
** an array.
*/
#define ArraySize(X)    ((int)(sizeof(X)/sizeof(X[0])))

/*
** Determine if the argument is a power of two
*/
#define IsPowerOfTwo(X) (((X)&((X)-1))==0)

/*
** The following value as a destructor means to use sqlite3DbFree().
** The sqlite3DbFree() routine requires two parameters instead of the
** one parameter that destructors normally want.  So we have to introduce
** this magic value that the code knows to handle differently.  Any
** pointer will work here as long as it is distinct from SQLITE_STATIC
** and SQLITE_TRANSIENT.
*/
#define SQLITE_DYNAMIC   ((sqlite3_destructor_type)sqlite3MallocSize)

/*
** When SQLITE_OMIT_WSD is defined, it means that the target platform does
** not support Writable Static Data (WSD) such as global and static variables.
** All variables must either be on the stack or dynamically allocated from
** the heap.  When WSD is unsupported, the variable declarations scattered
** throughout the SQLite code must become constants instead.  The SQLITE_WSD
** macro is used for this purpose.  And instead of referencing the variable
** directly, we use its constant as a key to lookup the run-time allocated
** buffer that holds real variable.  The constant is also the initializer
** for the run-time allocated buffer.
**
** In the usual case where WSD is supported, the SQLITE_WSD and GLOBAL
** macros become no-ops and have zero performance impact.
*/
#ifdef SQLITE_OMIT_WSD
  #define SQLITE_WSD const
  #define GLOBAL(t,v) (*(t*)sqlite3_wsd_find((void*)&(v), sizeof(v)))
  #define sqlite3GlobalConfig GLOBAL(struct Sqlite3Config, sqlite3Config)
SQLITE_API int SQLITE_STDCALL sqlite3_wsd_init(int N, int J);
SQLITE_API void *SQLITE_STDCALL sqlite3_wsd_find(void *K, int L);
#else
  #define SQLITE_WSD
  #define GLOBAL(t,v) v
  #define sqlite3GlobalConfig sqlite3Config
#endif

/*
** The following macros are used to suppress compiler warnings and to
** make it clear to human readers when a function parameter is deliberately
** left unused within the body of a function. This usually happens when
** a function is called via a function pointer. For example the
** implementation of an SQL aggregate step callback may not use the
** parameter indicating the number of arguments passed to the aggregate,
** if it knows that this is enforced elsewhere.
**
** When a function parameter is not used at all within the body of a function,
** it is generally named "NotUsed" or "NotUsed2" to make things even clearer.
** However, these macros may also be used to suppress warnings related to
** parameters that may or may not be used depending on compilation options.
** For example those parameters only used in assert() statements. In these
** cases the parameters are named as per the usual conventions.
*/
#define UNUSED_PARAMETER(x) (void)(x)
#define UNUSED_PARAMETER2(x,y) UNUSED_PARAMETER(x),UNUSED_PARAMETER(y)

/*
** Forward references to structures
*/
typedef struct AggInfo AggInfo;
typedef struct AuthContext AuthContext;
typedef struct AutoincInfo AutoincInfo;
typedef struct Bitvec Bitvec;
typedef struct CollSeq CollSeq;
typedef struct Column Column;
typedef struct Db Db;
typedef struct Schema Schema;
typedef struct Expr Expr;
typedef struct ExprList ExprList;
typedef struct ExprSpan ExprSpan;
typedef struct FKey FKey;
typedef struct FuncDestructor FuncDestructor;
typedef struct FuncDef FuncDef;
typedef struct FuncDefHash FuncDefHash;
typedef struct IdList IdList;
typedef struct Index Index;
typedef struct IndexSample IndexSample;
typedef struct KeyClass KeyClass;
typedef struct KeyInfo KeyInfo;
typedef struct Lookaside Lookaside;
typedef struct LookasideSlot LookasideSlot;
typedef struct Module Module;
typedef struct NameContext NameContext;
typedef struct Parse Parse;
typedef struct PreUpdate PreUpdate;
typedef struct PrintfArguments PrintfArguments;
typedef struct RowSet RowSet;
typedef struct Savepoint Savepoint;
typedef struct Select Select;
typedef struct SQLiteThread SQLiteThread;
typedef struct SelectDest SelectDest;
typedef struct SrcList SrcList;
typedef struct StrAccum StrAccum;
typedef struct Table Table;
typedef struct TableLock TableLock;
typedef struct Token Token;
typedef struct TreeView TreeView;
typedef struct Trigger Trigger;
typedef struct TriggerPrg TriggerPrg;
typedef struct TriggerStep TriggerStep;
typedef struct UnpackedRecord UnpackedRecord;
typedef struct VTable VTable;
typedef struct VtabCtx VtabCtx;
typedef struct Walker Walker;
typedef struct WhereInfo WhereInfo;
typedef struct With With;

/*
** Defer sourcing vdbe.h and btree.h until after the "u8" and
** "BusyHandler" typedefs. vdbe.h also requires a few of the opaque
** pointer types (i.e. FuncDef) defined above.
*/
/************** Include btree.h in the middle of sqliteInt.h *****************/
/************** Begin file btree.h *******************************************/
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
** This header file defines the interface that the sqlite B-Tree file
** subsystem.  See comments in the source code for a detailed description
** of what each interface routine does.
*/
#ifndef _BTREE_H_
#define _BTREE_H_

/* TODO: This definition is just included so other modules compile. It
** needs to be revisited.
*/
#define SQLITE_N_BTREE_META 16

/*
** If defined as non-zero, auto-vacuum is enabled by default. Otherwise
** it must be turned on for each database using "PRAGMA auto_vacuum = 1".
*/
#ifndef SQLITE_DEFAULT_AUTOVACUUM
  #define SQLITE_DEFAULT_AUTOVACUUM 0
#endif

#define BTREE_AUTOVACUUM_NONE 0        /* Do not do auto-vacuum */
#define BTREE_AUTOVACUUM_FULL 1        /* Do full auto-vacuum */
#define BTREE_AUTOVACUUM_INCR 2        /* Incremental vacuum */

/*
** Forward declarations of structure
*/
typedef struct Btree Btree;
typedef struct BtCursor BtCursor;
typedef struct BtShared BtShared;


SQLITE_PRIVATE int sqlite3BtreeOpen(
  sqlite3_vfs *pVfs,       /* VFS to use with this b-tree */
  const char *zFilename,   /* Name of database file to open */
  sqlite3 *db,             /* Associated database connection */
  Btree **ppBtree,         /* Return open Btree* here */
  int flags,               /* Flags */
  int vfsFlags             /* Flags passed through to VFS open */
);

/* The flags parameter to sqlite3BtreeOpen can be the bitwise or of the
** following values.
**
** NOTE:  These values must match the corresponding PAGER_ values in
** pager.h.
*/
#define BTREE_OMIT_JOURNAL  1  /* Do not create or use a rollback journal */
#define BTREE_MEMORY        2  /* This is an in-memory DB */
#define BTREE_SINGLE        4  /* The file contains at most 1 b-tree */
#define BTREE_UNORDERED     8  /* Use of a hash implementation is OK */

SQLITE_PRIVATE int sqlite3BtreeClose(Btree*);
SQLITE_PRIVATE int sqlite3BtreeSetCacheSize(Btree*,int);
SQLITE_PRIVATE int sqlite3BtreeSetSpillSize(Btree*,int);
#if SQLITE_MAX_MMAP_SIZE>0
SQLITE_PRIVATE   int sqlite3BtreeSetMmapLimit(Btree*,sqlite3_int64);
#endif
SQLITE_PRIVATE int sqlite3BtreeSetPagerFlags(Btree*,unsigned);
SQLITE_PRIVATE int sqlite3BtreeSetPageSize(Btree *p, int nPagesize, int nReserve, int eFix);
SQLITE_PRIVATE int sqlite3BtreeGetPageSize(Btree*);
SQLITE_PRIVATE int sqlite3BtreeMaxPageCount(Btree*,int);
SQLITE_PRIVATE u32 sqlite3BtreeLastPage(Btree*);
SQLITE_PRIVATE int sqlite3BtreeSecureDelete(Btree*,int);
SQLITE_PRIVATE int sqlite3BtreeGetOptimalReserve(Btree*);
SQLITE_PRIVATE int sqlite3BtreeGetReserveNoMutex(Btree *p);
SQLITE_PRIVATE int sqlite3BtreeSetAutoVacuum(Btree *, int);
SQLITE_PRIVATE int sqlite3BtreeGetAutoVacuum(Btree *);
SQLITE_PRIVATE int sqlite3BtreeBeginTrans(Btree*,int);
SQLITE_PRIVATE int sqlite3BtreeCommitPhaseOne(Btree*, const char *zMaster);
SQLITE_PRIVATE int sqlite3BtreeCommitPhaseTwo(Btree*, int);
SQLITE_PRIVATE int sqlite3BtreeCommit(Btree*);
SQLITE_PRIVATE int sqlite3BtreeRollback(Btree*,int,int);
SQLITE_PRIVATE int sqlite3BtreeBeginStmt(Btree*,int);
SQLITE_PRIVATE int sqlite3BtreeCreateTable(Btree*, int*, int flags);
SQLITE_PRIVATE int sqlite3BtreeIsInTrans(Btree*);
SQLITE_PRIVATE int sqlite3BtreeIsInReadTrans(Btree*);
SQLITE_PRIVATE int sqlite3BtreeIsInBackup(Btree*);
SQLITE_PRIVATE void *sqlite3BtreeSchema(Btree *, int, void(*)(void *));
SQLITE_PRIVATE int sqlite3BtreeSchemaLocked(Btree *pBtree);
SQLITE_PRIVATE int sqlite3BtreeLockTable(Btree *pBtree, int iTab, u8 isWriteLock);
SQLITE_PRIVATE int sqlite3BtreeSavepoint(Btree *, int, int);

SQLITE_PRIVATE const char *sqlite3BtreeGetFilename(Btree *);
SQLITE_PRIVATE const char *sqlite3BtreeGetJournalname(Btree *);
SQLITE_PRIVATE int sqlite3BtreeCopyFile(Btree *, Btree *);

SQLITE_PRIVATE int sqlite3BtreeIncrVacuum(Btree *);

/* The flags parameter to sqlite3BtreeCreateTable can be the bitwise OR
** of the flags shown below.
**
** Every SQLite table must have either BTREE_INTKEY or BTREE_BLOBKEY set.
** With BTREE_INTKEY, the table key is a 64-bit integer and arbitrary data
** is stored in the leaves.  (BTREE_INTKEY is used for SQL tables.)  With
** BTREE_BLOBKEY, the key is an arbitrary BLOB and no content is stored
** anywhere - the key is the content.  (BTREE_BLOBKEY is used for SQL
** indices.)
*/
#define BTREE_INTKEY     1    /* Table has only 64-bit signed integer keys */
#define BTREE_BLOBKEY    2    /* Table has keys only - no data */

SQLITE_PRIVATE int sqlite3BtreeDropTable(Btree*, int, int*);
SQLITE_PRIVATE int sqlite3BtreeClearTable(Btree*, int, int*);
SQLITE_PRIVATE int sqlite3BtreeClearTableOfCursor(BtCursor*);
SQLITE_PRIVATE int sqlite3BtreeTripAllCursors(Btree*, int, int);

SQLITE_PRIVATE void sqlite3BtreeGetMeta(Btree *pBtree, int idx, u32 *pValue);
SQLITE_PRIVATE int sqlite3BtreeUpdateMeta(Btree*, int idx, u32 value);

SQLITE_PRIVATE int sqlite3BtreeNewDb(Btree *p);

/*
** The second parameter to sqlite3BtreeGetMeta or sqlite3BtreeUpdateMeta
** should be one of the following values. The integer values are assigned 
** to constants so that the offset of the corresponding field in an
** SQLite database header may be found using the following formula:
**
**   offset = 36 + (idx * 4)
**
** For example, the free-page-count field is located at byte offset 36 of
** the database file header. The incr-vacuum-flag field is located at
** byte offset 64 (== 36+4*7).
**
** The BTREE_DATA_VERSION value is not really a value stored in the header.
** It is a read-only number computed by the pager.  But we merge it with
** the header value access routines since its access pattern is the same.
** Call it a "virtual meta value".
*/
#define BTREE_FREE_PAGE_COUNT     0
#define BTREE_SCHEMA_VERSION      1
#define BTREE_FILE_FORMAT         2
#define BTREE_DEFAULT_CACHE_SIZE  3
#define BTREE_LARGEST_ROOT_PAGE   4
#define BTREE_TEXT_ENCODING       5
#define BTREE_USER_VERSION        6
#define BTREE_INCR_VACUUM         7
#define BTREE_APPLICATION_ID      8
#define BTREE_DATA_VERSION        15  /* A virtual meta-value */

/*
** Kinds of hints that can be passed into the sqlite3BtreeCursorHint()
** interface.
**
** BTREE_HINT_RANGE  (arguments: Expr*, Mem*)
**
**     The first argument is an Expr* (which is guaranteed to be constant for
**     the lifetime of the cursor) that defines constraints on which rows
**     might be fetched with this cursor.  The Expr* tree may contain
**     TK_REGISTER nodes that refer to values stored in the array of registers
**     passed as the second parameter.  In other words, if Expr.op==TK_REGISTER
**     then the value of the node is the value in Mem[pExpr.iTable].  Any
**     TK_COLUMN node in the expression tree refers to the Expr.iColumn-th
**     column of the b-tree of the cursor.  The Expr tree will not contain
**     any function calls nor subqueries nor references to b-trees other than
**     the cursor being hinted.
**
**     The design of the _RANGE hint is aid b-tree implementations that try
**     to prefetch content from remote machines - to provide those
**     implementations with limits on what needs to be prefetched and thereby
**     reduce network bandwidth.
**
** Note that BTREE_HINT_FLAGS with BTREE_BULKLOAD is the only hint used by
** standard SQLite.  The other hints are provided for extentions that use
** the SQLite parser and code generator but substitute their own storage
** engine.
*/
#define BTREE_HINT_RANGE 0       /* Range constraints on queries */

/*
** Values that may be OR'd together to form the argument to the
** BTREE_HINT_FLAGS hint for sqlite3BtreeCursorHint():
**
** The BTREE_BULKLOAD flag is set on index cursors when the index is going
** to be filled with content that is already in sorted order.
**
** The BTREE_SEEK_EQ flag is set on cursors that will get OP_SeekGE or
** OP_SeekLE opcodes for a range search, but where the range of entries
** selected will all have the same key.  In other words, the cursor will
** be used only for equality key searches.
**
*/
#define BTREE_BULKLOAD 0x00000001  /* Used to full index in sorted order */
#define BTREE_SEEK_EQ  0x00000002  /* EQ seeks only - no range seeks */

/* 
** Flags passed as the third argument to sqlite3BtreeCursor().
**
** For read-only cursors the wrFlag argument is always zero. For read-write
** cursors it may be set to either (BTREE_WRCSR|BTREE_FORDELETE) or just
** (BTREE_WRCSR). If the BTREE_FORDELETE bit is set, then the cursor will
** only be used by SQLite for the following:
**
**   * to seek to and then delete specific entries, and/or
**
**   * to read values that will be used to create keys that other
**     BTREE_FORDELETE cursors will seek to and delete.
**
** The BTREE_FORDELETE flag is an optimization hint.  It is not used by
** by this, the native b-tree engine of SQLite, but it is available to
** alternative storage engines that might be substituted in place of this
** b-tree system.  For alternative storage engines in which a delete of
** the main table row automatically deletes corresponding index rows,
** the FORDELETE flag hint allows those alternative storage engines to
** skip a lot of work.  Namely:  FORDELETE cursors may treat all SEEK
** and DELETE operations as no-ops, and any READ operation against a
** FORDELETE cursor may return a null row: 0x01 0x00.
*/
#define BTREE_WRCSR     0x00000004     /* read-write cursor */
#define BTREE_FORDELETE 0x00000008     /* Cursor is for seek/delete only */

SQLITE_PRIVATE int sqlite3BtreeCursor(
  Btree*,                              /* BTree containing table to open */
  int iTable,                          /* Index of root page */
  int wrFlag,                          /* 1 for writing.  0 for read-only */
  struct KeyInfo*,                     /* First argument to compare function */
  BtCursor *pCursor                    /* Space to write cursor structure */
);
SQLITE_PRIVATE int sqlite3BtreeCursorSize(void);
SQLITE_PRIVATE void sqlite3BtreeCursorZero(BtCursor*);
SQLITE_PRIVATE void sqlite3BtreeCursorHintFlags(BtCursor*, unsigned);
#ifdef SQLITE_ENABLE_CURSOR_HINTS
SQLITE_PRIVATE void sqlite3BtreeCursorHint(BtCursor*, int, ...);
#endif

SQLITE_PRIVATE int sqlite3BtreeCloseCursor(BtCursor*);
SQLITE_PRIVATE int sqlite3BtreeMovetoUnpacked(
  BtCursor*,
  UnpackedRecord *pUnKey,
  i64 intKey,
  int bias,
  int *pRes
);
SQLITE_PRIVATE int sqlite3BtreeCursorHasMoved(BtCursor*);
SQLITE_PRIVATE int sqlite3BtreeCursorRestore(BtCursor*, int*);
SQLITE_PRIVATE int sqlite3BtreeDelete(BtCursor*, u8 flags);

/* Allowed flags for the 2nd argument to sqlite3BtreeDelete() */
#define BTREE_SAVEPOSITION 0x02  /* Leave cursor pointing at NEXT or PREV */
#define BTREE_AUXDELETE    0x04  /* not the primary delete operation */

SQLITE_PRIVATE int sqlite3BtreeInsert(BtCursor*, const void *pKey, i64 nKey,
                                  const void *pData, int nData,
                                  int nZero, int bias, int seekResult);
SQLITE_PRIVATE int sqlite3BtreeFirst(BtCursor*, int *pRes);
SQLITE_PRIVATE int sqlite3BtreeLast(BtCursor*, int *pRes);
SQLITE_PRIVATE int sqlite3BtreeNext(BtCursor*, int *pRes);
SQLITE_PRIVATE int sqlite3BtreeEof(BtCursor*);
SQLITE_PRIVATE int sqlite3BtreePrevious(BtCursor*, int *pRes);
SQLITE_PRIVATE int sqlite3BtreeKeySize(BtCursor*, i64 *pSize);
SQLITE_PRIVATE int sqlite3BtreeKey(BtCursor*, u32 offset, u32 amt, void*);
SQLITE_PRIVATE const void *sqlite3BtreeKeyFetch(BtCursor*, u32 *pAmt);
SQLITE_PRIVATE const void *sqlite3BtreeDataFetch(BtCursor*, u32 *pAmt);
SQLITE_PRIVATE int sqlite3BtreeDataSize(BtCursor*, u32 *pSize);
SQLITE_PRIVATE int sqlite3BtreeData(BtCursor*, u32 offset, u32 amt, void*);

SQLITE_PRIVATE char *sqlite3BtreeIntegrityCheck(Btree*, int *aRoot, int nRoot, int, int*);
SQLITE_PRIVATE struct Pager *sqlite3BtreePager(Btree*);

SQLITE_PRIVATE int sqlite3BtreePutData(BtCursor*, u32 offset, u32 amt, void*);
SQLITE_PRIVATE void sqlite3BtreeIncrblobCursor(BtCursor *);
SQLITE_PRIVATE void sqlite3BtreeClearCursor(BtCursor *);
SQLITE_PRIVATE int sqlite3BtreeSetVersion(Btree *pBt, int iVersion);
SQLITE_PRIVATE int sqlite3BtreeCursorHasHint(BtCursor*, unsigned int mask);
SQLITE_PRIVATE int sqlite3BtreeIsReadonly(Btree *pBt);
SQLITE_PRIVATE int sqlite3HeaderSizeBtree(void);

#ifndef NDEBUG
SQLITE_PRIVATE int sqlite3BtreeCursorIsValid(BtCursor*);
#endif

#ifndef SQLITE_OMIT_BTREECOUNT
SQLITE_PRIVATE int sqlite3BtreeCount(BtCursor *, i64 *);
#endif

#ifdef SQLITE_TEST
SQLITE_PRIVATE int sqlite3BtreeCursorInfo(BtCursor*, int*, int);
SQLITE_PRIVATE void sqlite3BtreeCursorList(Btree*);
#endif

#ifndef SQLITE_OMIT_WAL
SQLITE_PRIVATE   int sqlite3BtreeCheckpoint(Btree*, int, int *, int *);
#endif

/*
** If we are not using shared cache, then there is no need to
** use mutexes to access the BtShared structures.  So make the
** Enter and Leave procedures no-ops.
*/
#ifndef SQLITE_OMIT_SHARED_CACHE
SQLITE_PRIVATE   void sqlite3BtreeEnter(Btree*);
SQLITE_PRIVATE   void sqlite3BtreeEnterAll(sqlite3*);
SQLITE_PRIVATE   int sqlite3BtreeSharable(Btree*);
SQLITE_PRIVATE   void sqlite3BtreeEnterCursor(BtCursor*);
#else
# define sqlite3BtreeEnter(X) 
# define sqlite3BtreeEnterAll(X)
# define sqlite3BtreeSharable(X) 0
# define sqlite3BtreeEnterCursor(X)
#endif

#if !defined(SQLITE_OMIT_SHARED_CACHE) && SQLITE_THREADSAFE
SQLITE_PRIVATE   void sqlite3BtreeLeave(Btree*);
SQLITE_PRIVATE   void sqlite3BtreeLeaveCursor(BtCursor*);
SQLITE_PRIVATE   void sqlite3BtreeLeaveAll(sqlite3*);
#ifndef NDEBUG
  /* These routines are used inside assert() statements only. */
SQLITE_PRIVATE   int sqlite3BtreeHoldsMutex(Btree*);
SQLITE_PRIVATE   int sqlite3BtreeHoldsAllMutexes(sqlite3*);
SQLITE_PRIVATE   int sqlite3SchemaMutexHeld(sqlite3*,int,Schema*);
#endif
#else

# define sqlite3BtreeLeave(X)
# define sqlite3BtreeLeaveCursor(X)
# define sqlite3BtreeLeaveAll(X)

# define sqlite3BtreeHoldsMutex(X) 1
# define sqlite3BtreeHoldsAllMutexes(X) 1
# define sqlite3SchemaMutexHeld(X,Y,Z) 1
#endif


#endif /* _BTREE_H_ */

/************** End of btree.h ***********************************************/
/************** Continuing where we left off in sqliteInt.h ******************/
/************** Include vdbe.h in the middle of sqliteInt.h ******************/
/************** Begin file vdbe.h ********************************************/
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
** Header file for the Virtual DataBase Engine (VDBE)
**
** This header defines the interface to the virtual database engine
** or VDBE.  The VDBE implements an abstract machine that runs a
** simple program to access and modify the underlying database.
*/
#ifndef _SQLITE_VDBE_H_
#define _SQLITE_VDBE_H_
/* #include <stdio.h> */

/*
** A single VDBE is an opaque structure named "Vdbe".  Only routines
** in the source file sqliteVdbe.c are allowed to see the insides
** of this structure.
*/
typedef struct Vdbe Vdbe;

/*
** The names of the following types declared in vdbeInt.h are required
** for the VdbeOp definition.
*/
typedef struct Mem Mem;
typedef struct SubProgram SubProgram;

/*
** A single instruction of the virtual machine has an opcode
** and as many as three operands.  The instruction is recorded
** as an instance of the following structure:
*/
struct VdbeOp {
  u8 opcode;          /* What operation to perform */
  signed char p4type; /* One of the P4_xxx constants for p4 */
  u8 notUsed1;
  u8 p5;              /* Fifth parameter is an unsigned character */
  int p1;             /* First operand */
  int p2;             /* Second parameter (often the jump destination) */
  int p3;             /* The third parameter */
  union p4union {     /* fourth parameter */
    int i;                 /* Integer value if p4type==P4_INT32 */
    void *p;               /* Generic pointer */
    char *z;               /* Pointer to data for string (char array) types */
    i64 *pI64;             /* Used when p4type is P4_INT64 */
    double *pReal;         /* Used when p4type is P4_REAL */
    FuncDef *pFunc;        /* Used when p4type is P4_FUNCDEF */
    sqlite3_context *pCtx; /* Used when p4type is P4_FUNCCTX */
    CollSeq *pColl;        /* Used when p4type is P4_COLLSEQ */
    Mem *pMem;             /* Used when p4type is P4_MEM */
    VTable *pVtab;         /* Used when p4type is P4_VTAB */
    KeyInfo *pKeyInfo;     /* Used when p4type is P4_KEYINFO */
    int *ai;               /* Used when p4type is P4_INTARRAY */
    SubProgram *pProgram;  /* Used when p4type is P4_SUBPROGRAM */
    Table *pTab;           /* Used when p4type is P4_TABLE */
#ifdef SQLITE_ENABLE_CURSOR_HINTS
    Expr *pExpr;           /* Used when p4type is P4_EXPR */
#endif
    int (*xAdvance)(BtCursor *, int *);
  } p4;
#ifdef SQLITE_ENABLE_EXPLAIN_COMMENTS
  char *zComment;          /* Comment to improve readability */
#endif
#ifdef VDBE_PROFILE
  u32 cnt;                 /* Number of times this instruction was executed */
  u64 cycles;              /* Total time spent executing this instruction */
#endif
#ifdef SQLITE_VDBE_COVERAGE
  int iSrcLine;            /* Source-code line that generated this opcode */
#endif
};
typedef struct VdbeOp VdbeOp;


/*
** A sub-routine used to implement a trigger program.
*/
struct SubProgram {
  VdbeOp *aOp;                  /* Array of opcodes for sub-program */
  int nOp;                      /* Elements in aOp[] */
  int nMem;                     /* Number of memory cells required */
  int nCsr;                     /* Number of cursors required */
  int nOnce;                    /* Number of OP_Once instructions */
  void *token;                  /* id that may be used to recursive triggers */
  SubProgram *pNext;            /* Next sub-program already visited */
};

/*
** A smaller version of VdbeOp used for the VdbeAddOpList() function because
** it takes up less space.
*/
struct VdbeOpList {
  u8 opcode;          /* What operation to perform */
  signed char p1;     /* First operand */
  signed char p2;     /* Second parameter (often the jump destination) */
  signed char p3;     /* Third parameter */
};
typedef struct VdbeOpList VdbeOpList;

/*
** Allowed values of VdbeOp.p4type
*/
#define P4_NOTUSED    0   /* The P4 parameter is not used */
#define P4_DYNAMIC  (-1)  /* Pointer to a string obtained from sqliteMalloc() */
#define P4_STATIC   (-2)  /* Pointer to a static string */
#define P4_COLLSEQ  (-4)  /* P4 is a pointer to a CollSeq structure */
#define P4_FUNCDEF  (-5)  /* P4 is a pointer to a FuncDef structure */
#define P4_KEYINFO  (-6)  /* P4 is a pointer to a KeyInfo structure */
#define P4_EXPR     (-7)  /* P4 is a pointer to an Expr tree */
#define P4_MEM      (-8)  /* P4 is a pointer to a Mem*    structure */
#define P4_TRANSIENT  0   /* P4 is a pointer to a transient string */
#define P4_VTAB     (-10) /* P4 is a pointer to an sqlite3_vtab structure */
#define P4_MPRINTF  (-11) /* P4 is a string obtained from sqlite3_mprintf() */
#define P4_REAL     (-12) /* P4 is a 64-bit floating point value */
#define P4_INT64    (-13) /* P4 is a 64-bit signed integer */
#define P4_INT32    (-14) /* P4 is a 32-bit signed integer */
#define P4_INTARRAY (-15) /* P4 is a vector of 32-bit integers */
#define P4_SUBPROGRAM  (-18) /* P4 is a pointer to a SubProgram structure */
#define P4_ADVANCE  (-19) /* P4 is a pointer to BtreeNext() or BtreePrev() */
#define P4_TABLE    (-20) /* P4 is a pointer to a Table structure */
#define P4_FUNCCTX  (-21) /* P4 is a pointer to an sqlite3_context object */

/* Error message codes for OP_Halt */
#define P5_ConstraintNotNull 1
#define P5_ConstraintUnique  2
#define P5_ConstraintCheck   3
#define P5_ConstraintFK      4

/*
** The Vdbe.aColName array contains 5n Mem structures, where n is the 
** number of columns of data returned by the statement.
*/
#define COLNAME_NAME     0
#define COLNAME_DECLTYPE 1
#define COLNAME_DATABASE 2
#define COLNAME_TABLE    3
#define COLNAME_COLUMN   4
#ifdef SQLITE_ENABLE_COLUMN_METADATA
# define COLNAME_N        5      /* Number of COLNAME_xxx symbols */
#else
# ifdef SQLITE_OMIT_DECLTYPE
#   define COLNAME_N      1      /* Store only the name */
# else
#   define COLNAME_N      2      /* Store the name and decltype */
# endif
#endif

/*
** The following macro converts a relative address in the p2 field
** of a VdbeOp structure into a negative number so that 
** sqlite3VdbeAddOpList() knows that the address is relative.  Calling
** the macro again restores the address.
*/
#define ADDR(X)  (-1-(X))

/*
** The makefile scans the vdbe.c source file and creates the "opcodes.h"
** header file that defines a number for each opcode used by the VDBE.
*/
/************** Include opcodes.h in the middle of vdbe.h ********************/
/************** Begin file opcodes.h *****************************************/
/* Automatically generated.  Do not edit */
/* See the tool/mkopcodeh.tcl script for details */
#define OP_Savepoint       0
#define OP_AutoCommit      1
#define OP_Transaction     2
#define OP_SorterNext      3
#define OP_PrevIfOpen      4
#define OP_NextIfOpen      5
#define OP_Prev            6
#define OP_Next            7
#define OP_Checkpoint      8
#define OP_JournalMode     9
#define OP_Vacuum         10
#define OP_VFilter        11 /* synopsis: iplan=r[P3] zplan='P4'           */
#define OP_VUpdate        12 /* synopsis: data=r[P3@P2]                    */
#define OP_Goto           13
#define OP_Gosub          14
#define OP_InitCoroutine  15
#define OP_Yield          16
#define OP_MustBeInt      17
#define OP_Jump           18
#define OP_Not            19 /* same as TK_NOT, synopsis: r[P2]= !r[P1]    */
#define OP_Once           20
#define OP_If             21
#define OP_IfNot          22
#define OP_SeekLT         23 /* synopsis: key=r[P3@P4]                     */
#define OP_SeekLE         24 /* synopsis: key=r[P3@P4]                     */
#define OP_SeekGE         25 /* synopsis: key=r[P3@P4]                     */
#define OP_SeekGT         26 /* synopsis: key=r[P3@P4]                     */
#define OP_Or             27 /* same as TK_OR, synopsis: r[P3]=(r[P1] || r[P2]) */
#define OP_And            28 /* same as TK_AND, synopsis: r[P3]=(r[P1] && r[P2]) */
#define OP_NoConflict     29 /* synopsis: key=r[P3@P4]                     */
#define OP_NotFound       30 /* synopsis: key=r[P3@P4]                     */
#define OP_Found          31 /* synopsis: key=r[P3@P4]                     */
#define OP_NotExists      32 /* synopsis: intkey=r[P3]                     */
#define OP_Last           33
#define OP_IsNull         34 /* same as TK_ISNULL, synopsis: if r[P1]==NULL goto P2 */
#define OP_NotNull        35 /* same as TK_NOTNULL, synopsis: if r[P1]!=NULL goto P2 */
#define OP_Ne             36 /* same as TK_NE, synopsis: if r[P1]!=r[P3] goto P2 */
#define OP_Eq             37 /* same as TK_EQ, synopsis: if r[P1]==r[P3] goto P2 */
#define OP_Gt             38 /* same as TK_GT, synopsis: if r[P1]>r[P3] goto P2 */
#define OP_Le             39 /* same as TK_LE, synopsis: if r[P1]<=r[P3] goto P2 */
#define OP_Lt             40 /* same as TK_LT, synopsis: if r[P1]<r[P3] goto P2 */
#define OP_Ge             41 /* same as TK_GE, synopsis: if r[P1]>=r[P3] goto P2 */
#define OP_SorterSort     42
#define OP_BitAnd         43 /* same as TK_BITAND, synopsis: r[P3]=r[P1]&r[P2] */
#define OP_BitOr          44 /* same as TK_BITOR, synopsis: r[P3]=r[P1]|r[P2] */
#define OP_ShiftLeft      45 /* same as TK_LSHIFT, synopsis: r[P3]=r[P2]<<r[P1] */
#define OP_ShiftRight     46 /* same as TK_RSHIFT, synopsis: r[P3]=r[P2]>>r[P1] */
#define OP_Add            47 /* same as TK_PLUS, synopsis: r[P3]=r[P1]+r[P2] */
#define OP_Subtract       48 /* same as TK_MINUS, synopsis: r[P3]=r[P2]-r[P1] */
#define OP_Multiply       49 /* same as TK_STAR, synopsis: r[P3]=r[P1]*r[P2] */
#define OP_Divide         50 /* same as TK_SLASH, synopsis: r[P3]=r[P2]/r[P1] */
#define OP_Remainder      51 /* same as TK_REM, synopsis: r[P3]=r[P2]%r[P1] */
#define OP_Concat         52 /* same as TK_CONCAT, synopsis: r[P3]=r[P2]+r[P1] */
#define OP_Sort           53
#define OP_BitNot         54 /* same as TK_BITNOT, synopsis: r[P1]= ~r[P1] */
#define OP_Rewind         55
#define OP_IdxLE          56 /* synopsis: key=r[P3@P4]                     */
#define OP_IdxGT          57 /* synopsis: key=r[P3@P4]                     */
#define OP_IdxLT          58 /* synopsis: key=r[P3@P4]                     */
#define OP_IdxGE          59 /* synopsis: key=r[P3@P4]                     */
#define OP_RowSetRead     60 /* synopsis: r[P3]=rowset(P1)                 */
#define OP_RowSetTest     61 /* synopsis: if r[P3] in rowset(P1) goto P2   */
#define OP_Program        62
#define OP_FkIfZero       63 /* synopsis: if fkctr[P1]==0 goto P2          */
#define OP_IfPos          64 /* synopsis: if r[P1]>0 then r[P1]-=P3, goto P2 */
#define OP_IfNotZero      65 /* synopsis: if r[P1]!=0 then r[P1]-=P3, goto P2 */
#define OP_DecrJumpZero   66 /* synopsis: if (--r[P1])==0 goto P2          */
#define OP_IncrVacuum     67
#define OP_VNext          68
#define OP_Init           69 /* synopsis: Start at P2                      */
#define OP_Return         70
#define OP_EndCoroutine   71
#define OP_HaltIfNull     72 /* synopsis: if r[P3]=null halt               */
#define OP_Halt           73
#define OP_Integer        74 /* synopsis: r[P2]=P1                         */
#define OP_Int64          75 /* synopsis: r[P2]=P4                         */
#define OP_String         76 /* synopsis: r[P2]='P4' (len=P1)              */
#define OP_Null           77 /* synopsis: r[P2..P3]=NULL                   */
#define OP_SoftNull       78 /* synopsis: r[P1]=NULL                       */
#define OP_Blob           79 /* synopsis: r[P2]=P4 (len=P1)                */
#define OP_Variable       80 /* synopsis: r[P2]=parameter(P1,P4)           */
#define OP_Move           81 /* synopsis: r[P2@P3]=r[P1@P3]                */
#define OP_Copy           82 /* synopsis: r[P2@P3+1]=r[P1@P3+1]            */
#define OP_SCopy          83 /* synopsis: r[P2]=r[P1]                      */
#define OP_IntCopy        84 /* synopsis: r[P2]=r[P1]                      */
#define OP_ResultRow      85 /* synopsis: output=r[P1@P2]                  */
#define OP_CollSeq        86
#define OP_Function0      87 /* synopsis: r[P3]=func(r[P2@P5])             */
#define OP_Function       88 /* synopsis: r[P3]=func(r[P2@P5])             */
#define OP_AddImm         89 /* synopsis: r[P1]=r[P1]+P2                   */
#define OP_RealAffinity   90
#define OP_Cast           91 /* synopsis: affinity(r[P1])                  */
#define OP_Permutation    92
#define OP_Compare        93 /* synopsis: r[P1@P3] <-> r[P2@P3]            */
#define OP_Column         94 /* synopsis: r[P3]=PX                         */
#define OP_Affinity       95 /* synopsis: affinity(r[P1@P2])               */
#define OP_MakeRecord     96 /* synopsis: r[P3]=mkrec(r[P1@P2])            */
#define OP_String8        97 /* same as TK_STRING, synopsis: r[P2]='P4'    */
#define OP_Count          98 /* synopsis: r[P2]=count()                    */
#define OP_ReadCookie     99
#define OP_SetCookie     100
#define OP_ReopenIdx     101 /* synopsis: root=P2 iDb=P3                   */
#define OP_OpenRead      102 /* synopsis: root=P2 iDb=P3                   */
#define OP_OpenWrite     103 /* synopsis: root=P2 iDb=P3                   */
#define OP_OpenAutoindex 104 /* synopsis: nColumn=P2                       */
#define OP_OpenEphemeral 105 /* synopsis: nColumn=P2                       */
#define OP_SorterOpen    106
#define OP_SequenceTest  107 /* synopsis: if( cursor[P1].ctr++ ) pc = P2   */
#define OP_OpenPseudo    108 /* synopsis: P3 columns in r[P2]              */
#define OP_Close         109
#define OP_ColumnsUsed   110
#define OP_Sequence      111 /* synopsis: r[P2]=cursor[P1].ctr++           */
#define OP_NewRowid      112 /* synopsis: r[P2]=rowid                      */
#define OP_Insert        113 /* synopsis: intkey=r[P3] data=r[P2]          */
#define OP_InsertInt     114 /* synopsis: intkey=P3 data=r[P2]             */
#define OP_Delete        115
#define OP_ResetCount    116
#define OP_SorterCompare 117 /* synopsis: if key(P1)!=trim(r[P3],P4) goto P2 */
#define OP_SorterData    118 /* synopsis: r[P2]=data                       */
#define OP_RowKey        119 /* synopsis: r[P2]=key                        */
#define OP_RowData       120 /* synopsis: r[P2]=data                       */
#define OP_Rowid         121 /* synopsis: r[P2]=rowid                      */
#define OP_NullRow       122
#define OP_SorterInsert  123
#define OP_IdxInsert     124 /* synopsis: key=r[P2]                        */
#define OP_IdxDelete     125 /* synopsis: key=r[P2@P3]                     */
#define OP_Seek          126 /* synopsis: Move P3 to P1.rowid              */
#define OP_IdxRowid      127 /* synopsis: r[P2]=rowid                      */
#define OP_Destroy       128
#define OP_Clear         129
#define OP_ResetSorter   130
#define OP_CreateIndex   131 /* synopsis: r[P2]=root iDb=P1                */
#define OP_CreateTable   132 /* synopsis: r[P2]=root iDb=P1                */
#define OP_Real          133 /* same as TK_FLOAT, synopsis: r[P2]=P4       */
#define OP_ParseSchema   134
#define OP_LoadAnalysis  135
#define OP_DropTable     136
#define OP_DropIndex     137
#define OP_DropTrigger   138
#define OP_IntegrityCk   139
#define OP_RowSetAdd     140 /* synopsis: rowset(P1)=r[P2]                 */
#define OP_Param         141
#define OP_FkCounter     142 /* synopsis: fkctr[P1]+=P2                    */
#define OP_MemMax        143 /* synopsis: r[P1]=max(r[P1],r[P2])           */
#define OP_OffsetLimit   144 /* synopsis: if r[P1]>0 then r[P2]=r[P1]+max(0,r[P3]) else r[P2]=(-1) */
#define OP_AggStep0      145 /* synopsis: accum=r[P3] step(r[P2@P5])       */
#define OP_AggStep       146 /* synopsis: accum=r[P3] step(r[P2@P5])       */
#define OP_AggFinal      147 /* synopsis: accum=r[P1] N=P2                 */
#define OP_Expire        148
#define OP_TableLock     149 /* synopsis: iDb=P1 root=P2 write=P3          */
#define OP_VBegin        150
#define OP_VCreate       151
#define OP_VDestroy      152
#define OP_VOpen         153
#define OP_VColumn       154 /* synopsis: r[P3]=vcolumn(P2)                */
#define OP_VRename       155
#define OP_Pagecount     156
#define OP_MaxPgcnt      157
#define OP_CursorHint    158
#define OP_Noop          159
#define OP_Explain       160

/* Properties such as "out2" or "jump" that are specified in
** comments following the "case" for each opcode in the vdbe.c
** are encoded into bitvectors as follows:
*/
#define OPFLG_JUMP        0x01  /* jump:  P2 holds jmp target */
#define OPFLG_IN1         0x02  /* in1:   P1 is an input */
#define OPFLG_IN2         0x04  /* in2:   P2 is an input */
#define OPFLG_IN3         0x08  /* in3:   P3 is an input */
#define OPFLG_OUT2        0x10  /* out2:  P2 is an output */
#define OPFLG_OUT3        0x20  /* out3:  P3 is an output */
#define OPFLG_INITIALIZER {\
/*   0 */ 0x00, 0x00, 0x00, 0x01, 0x01, 0x01, 0x01, 0x01,\
/*   8 */ 0x00, 0x10, 0x00, 0x01, 0x00, 0x01, 0x01, 0x01,\
/*  16 */ 0x03, 0x03, 0x01, 0x12, 0x01, 0x03, 0x03, 0x09,\
/*  24 */ 0x09, 0x09, 0x09, 0x26, 0x26, 0x09, 0x09, 0x09,\
/*  32 */ 0x09, 0x01, 0x03, 0x03, 0x0b, 0x0b, 0x0b, 0x0b,\
/*  40 */ 0x0b, 0x0b, 0x01, 0x26, 0x26, 0x26, 0x26, 0x26,\
/*  48 */ 0x26, 0x26, 0x26, 0x26, 0x26, 0x01, 0x12, 0x01,\
/*  56 */ 0x01, 0x01, 0x01, 0x01, 0x23, 0x0b, 0x01, 0x01,\
/*  64 */ 0x03, 0x03, 0x03, 0x01, 0x01, 0x01, 0x02, 0x02,\
/*  72 */ 0x08, 0x00, 0x10, 0x10, 0x10, 0x10, 0x00, 0x10,\
/*  80 */ 0x10, 0x00, 0x00, 0x10, 0x10, 0x00, 0x00, 0x00,\
/*  88 */ 0x00, 0x02, 0x02, 0x02, 0x00, 0x00, 0x00, 0x00,\
/*  96 */ 0x00, 0x10, 0x10, 0x10, 0x00, 0x00, 0x00, 0x00,\
/* 104 */ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x10,\
/* 112 */ 0x10, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,\
/* 120 */ 0x00, 0x10, 0x00, 0x04, 0x04, 0x00, 0x00, 0x10,\
/* 128 */ 0x10, 0x00, 0x00, 0x10, 0x10, 0x10, 0x00, 0x00,\
/* 136 */ 0x00, 0x00, 0x00, 0x00, 0x06, 0x10, 0x00, 0x04,\
/* 144 */ 0x1a, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,\
/* 152 */ 0x00, 0x00, 0x00, 0x00, 0x10, 0x10, 0x00, 0x00,\
/* 160 */ 0x00,}

/* The sqlite3P2Values() routine is able to run faster if it knows
** the value of the largest JUMP opcode.  The smaller the maximum
** JUMP opcode the better, so the mkopcodeh.tcl script that
** generated this include file strives to group all JUMP opcodes
** together near the beginning of the list.
*/
#define SQLITE_MX_JUMP_OPCODE  69  /* Maximum JUMP opcode */

/************** End of opcodes.h *********************************************/
/************** Continuing where we left off in vdbe.h ***********************/

/*
** Prototypes for the VDBE interface.  See comments on the implementation
** for a description of what each of these routines does.
*/
SQLITE_PRIVATE Vdbe *sqlite3VdbeCreate(Parse*);
SQLITE_PRIVATE int sqlite3VdbeAddOp0(Vdbe*,int);
SQLITE_PRIVATE int sqlite3VdbeAddOp1(Vdbe*,int,int);
SQLITE_PRIVATE int sqlite3VdbeAddOp2(Vdbe*,int,int,int);
SQLITE_PRIVATE int sqlite3VdbeGoto(Vdbe*,int);
SQLITE_PRIVATE int sqlite3VdbeLoadString(Vdbe*,int,const char*);
SQLITE_PRIVATE void sqlite3VdbeMultiLoad(Vdbe*,int,const char*,...);
SQLITE_PRIVATE int sqlite3VdbeAddOp3(Vdbe*,int,int,int,int);
SQLITE_PRIVATE int sqlite3VdbeAddOp4(Vdbe*,int,int,int,int,const char *zP4,int);
SQLITE_PRIVATE int sqlite3VdbeAddOp4Dup8(Vdbe*,int,int,int,int,const u8*,int);
SQLITE_PRIVATE int sqlite3VdbeAddOp4Int(Vdbe*,int,int,int,int,int);
SQLITE_PRIVATE void sqlite3VdbeEndCoroutine(Vdbe*,int);
#if defined(SQLITE_DEBUG) && !defined(SQLITE_TEST_REALLOC_STRESS)
SQLITE_PRIVATE   void sqlite3VdbeVerifyNoMallocRequired(Vdbe *p, int N);
#else
# define sqlite3VdbeVerifyNoMallocRequired(A,B)
#endif
SQLITE_PRIVATE VdbeOp *sqlite3VdbeAddOpList(Vdbe*, int nOp, VdbeOpList const *aOp, int iLineno);
SQLITE_PRIVATE void sqlite3VdbeAddParseSchemaOp(Vdbe*,int,char*);
SQLITE_PRIVATE void sqlite3VdbeChangeOpcode(Vdbe*, u32 addr, u8);
SQLITE_PRIVATE void sqlite3VdbeChangeP1(Vdbe*, u32 addr, int P1);
SQLITE_PRIVATE void sqlite3VdbeChangeP2(Vdbe*, u32 addr, int P2);
SQLITE_PRIVATE void sqlite3VdbeChangeP3(Vdbe*, u32 addr, int P3);
SQLITE_PRIVATE void sqlite3VdbeChangeP5(Vdbe*, u8 P5);
SQLITE_PRIVATE void sqlite3VdbeJumpHere(Vdbe*, int addr);
SQLITE_PRIVATE int sqlite3VdbeChangeToNoop(Vdbe*, int addr);
SQLITE_PRIVATE int sqlite3VdbeDeletePriorOpcode(Vdbe*, u8 op);
SQLITE_PRIVATE void sqlite3VdbeChangeP4(Vdbe*, int addr, const char *zP4, int N);
SQLITE_PRIVATE void sqlite3VdbeSetP4KeyInfo(Parse*, Index*);
SQLITE_PRIVATE void sqlite3VdbeUsesBtree(Vdbe*, int);
SQLITE_PRIVATE VdbeOp *sqlite3VdbeGetOp(Vdbe*, int);
SQLITE_PRIVATE int sqlite3VdbeMakeLabel(Vdbe*);
SQLITE_PRIVATE void sqlite3VdbeRunOnlyOnce(Vdbe*);
SQLITE_PRIVATE void sqlite3VdbeReusable(Vdbe*);
SQLITE_PRIVATE void sqlite3VdbeDelete(Vdbe*);
SQLITE_PRIVATE void sqlite3VdbeClearObject(sqlite3*,Vdbe*);
SQLITE_PRIVATE void sqlite3VdbeMakeReady(Vdbe*,Parse*);
SQLITE_PRIVATE int sqlite3VdbeFinalize(Vdbe*);
SQLITE_PRIVATE void sqlite3VdbeResolveLabel(Vdbe*, int);
SQLITE_PRIVATE int sqlite3VdbeCurrentAddr(Vdbe*);
#ifdef SQLITE_DEBUG
SQLITE_PRIVATE   int sqlite3VdbeAssertMayAbort(Vdbe *, int);
#endif
SQLITE_PRIVATE void sqlite3VdbeResetStepResult(Vdbe*);
SQLITE_PRIVATE void sqlite3VdbeRewind(Vdbe*);
SQLITE_PRIVATE int sqlite3VdbeReset(Vdbe*);
SQLITE_PRIVATE void sqlite3VdbeSetNumCols(Vdbe*,int);
SQLITE_PRIVATE int sqlite3VdbeSetColName(Vdbe*, int, int, const char *, void(*)(void*));
SQLITE_PRIVATE void sqlite3VdbeCountChanges(Vdbe*);
SQLITE_PRIVATE sqlite3 *sqlite3VdbeDb(Vdbe*);
SQLITE_PRIVATE void sqlite3VdbeSetSql(Vdbe*, const char *z, int n, int);
SQLITE_PRIVATE void sqlite3VdbeSwap(Vdbe*,Vdbe*);
SQLITE_PRIVATE VdbeOp *sqlite3VdbeTakeOpArray(Vdbe*, int*, int*);
SQLITE_PRIVATE sqlite3_value *sqlite3VdbeGetBoundValue(Vdbe*, int, u8);
SQLITE_PRIVATE void sqlite3VdbeSetVarmask(Vdbe*, int);
#ifndef SQLITE_OMIT_TRACE
SQLITE_PRIVATE   char *sqlite3VdbeExpandSql(Vdbe*, const char*);
#endif
SQLITE_PRIVATE int sqlite3MemCompare(const Mem*, const Mem*, const CollSeq*);

SQLITE_PRIVATE void sqlite3VdbeRecordUnpack(KeyInfo*,int,const void*,UnpackedRecord*);
SQLITE_PRIVATE int sqlite3VdbeRecordCompare(int,const void*,UnpackedRecord*);
SQLITE_PRIVATE int sqlite3VdbeRecordCompareWithSkip(int, const void *, UnpackedRecord *, int);
SQLITE_PRIVATE UnpackedRecord *sqlite3VdbeAllocUnpackedRecord(KeyInfo *, char *, int, char **);

typedef int (*RecordCompare)(int,const void*,UnpackedRecord*);
SQLITE_PRIVATE RecordCompare sqlite3VdbeFindCompare(UnpackedRecord*);

#ifndef SQLITE_OMIT_TRIGGER
SQLITE_PRIVATE void sqlite3VdbeLinkSubProgram(Vdbe *, SubProgram *);
#endif

/* Use SQLITE_ENABLE_COMMENTS to enable generation of extra comments on
** each VDBE opcode.
**
** Use the SQLITE_ENABLE_MODULE_COMMENTS macro to see some extra no-op
** comments in VDBE programs that show key decision points in the code
** generator.
*/
#ifdef SQLITE_ENABLE_EXPLAIN_COMMENTS
SQLITE_PRIVATE   void sqlite3VdbeComment(Vdbe*, const char*, ...);
# define VdbeComment(X)  sqlite3VdbeComment X
SQLITE_PRIVATE   void sqlite3VdbeNoopComment(Vdbe*, const char*, ...);
# define VdbeNoopComment(X)  sqlite3VdbeNoopComment X
# ifdef SQLITE_ENABLE_MODULE_COMMENTS
#   define VdbeModuleComment(X)  sqlite3VdbeNoopComment X
# else
#   define VdbeModuleComment(X)
# endif
#else
# define VdbeComment(X)
# define VdbeNoopComment(X)
# define VdbeModuleComment(X)
#endif

/*
** The VdbeCoverage macros are used to set a coverage testing point
** for VDBE branch instructions.  The coverage testing points are line
** numbers in the sqlite3.c source file.  VDBE branch coverage testing
** only works with an amalagmation build.  That's ok since a VDBE branch
** coverage build designed for testing the test suite only.  No application
** should ever ship with VDBE branch coverage measuring turned on.
**
**    VdbeCoverage(v)                  // Mark the previously coded instruction
**                                     // as a branch
**
**    VdbeCoverageIf(v, conditional)   // Mark previous if conditional true
**
**    VdbeCoverageAlwaysTaken(v)       // Previous branch is always taken
**
**    VdbeCoverageNeverTaken(v)        // Previous branch is never taken
**
** Every VDBE branch operation must be tagged with one of the macros above.
** If not, then when "make test" is run with -DSQLITE_VDBE_COVERAGE and
** -DSQLITE_DEBUG then an ALWAYS() will fail in the vdbeTakeBranch()
** routine in vdbe.c, alerting the developer to the missed tag.
*/
#ifdef SQLITE_VDBE_COVERAGE
SQLITE_PRIVATE   void sqlite3VdbeSetLineNumber(Vdbe*,int);
# define VdbeCoverage(v) sqlite3VdbeSetLineNumber(v,__LINE__)
# define VdbeCoverageIf(v,x) if(x)sqlite3VdbeSetLineNumber(v,__LINE__)
# define VdbeCoverageAlwaysTaken(v) sqlite3VdbeSetLineNumber(v,2);
# define VdbeCoverageNeverTaken(v) sqlite3VdbeSetLineNumber(v,1);
# define VDBE_OFFSET_LINENO(x) (__LINE__+x)
#else
# define VdbeCoverage(v)
# define VdbeCoverageIf(v,x)
# define VdbeCoverageAlwaysTaken(v)
# define VdbeCoverageNeverTaken(v)
# define VDBE_OFFSET_LINENO(x) 0
#endif

#ifdef SQLITE_ENABLE_STMT_SCANSTATUS
SQLITE_PRIVATE void sqlite3VdbeScanStatus(Vdbe*, int, int, int, LogEst, const char*);
#else
# define sqlite3VdbeScanStatus(a,b,c,d,e)
#endif

#endif

/************** End of vdbe.h ************************************************/
/************** Continuing where we left off in sqliteInt.h ******************/
/************** Include pager.h in the middle of sqliteInt.h *****************/
/************** Begin file pager.h *******************************************/
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
** This header file defines the interface that the sqlite page cache
** subsystem.  The page cache subsystem reads and writes a file a page
** at a time and provides a journal for rollback.
*/

#ifndef _PAGER_H_
#define _PAGER_H_

/*
** Default maximum size for persistent journal files. A negative 
** value means no limit. This value may be overridden using the 
** sqlite3PagerJournalSizeLimit() API. See also "PRAGMA journal_size_limit".
*/
#ifndef SQLITE_DEFAULT_JOURNAL_SIZE_LIMIT
  #define SQLITE_DEFAULT_JOURNAL_SIZE_LIMIT -1
#endif

/*
** The type used to represent a page number.  The first page in a file
** is called page 1.  0 is used to represent "not a page".
*/
typedef u32 Pgno;

/*
** Each open file is managed by a separate instance of the "Pager" structure.
*/
typedef struct Pager Pager;

/*
** Handle type for pages.
*/
typedef struct PgHdr DbPage;

/*
** Page number PAGER_MJ_PGNO is never used in an SQLite database (it is
** reserved for working around a windows/posix incompatibility). It is
** used in the journal to signify that the remainder of the journal file 
** is devoted to storing a master journal name - there are no more pages to
** roll back. See comments for function writeMasterJournal() in pager.c 
** for details.
*/
#define PAGER_MJ_PGNO(x) ((Pgno)((PENDING_BYTE/((x)->pageSize))+1))

/*
** Allowed values for the flags parameter to sqlite3PagerOpen().
**
** NOTE: These values must match the corresponding BTREE_ values in btree.h.
*/
#define PAGER_OMIT_JOURNAL  0x0001    /* Do not use a rollback journal */
#define PAGER_MEMORY        0x0002    /* In-memory database */

/*
** Valid values for the second argument to sqlite3PagerLockingMode().
*/
#define PAGER_LOCKINGMODE_QUERY      -1
#define PAGER_LOCKINGMODE_NORMAL      0
#define PAGER_LOCKINGMODE_EXCLUSIVE   1

/*
** Numeric constants that encode the journalmode.
**
** The numeric values encoded here (other than PAGER_JOURNALMODE_QUERY)
** are exposed in the API via the "PRAGMA journal_mode" command and
** therefore cannot be changed without a compatibility break.
*/
#define PAGER_JOURNALMODE_QUERY     (-1)  /* Query the value of journalmode */
#define PAGER_JOURNALMODE_DELETE      0   /* Commit by deleting journal file */
#define PAGER_JOURNALMODE_PERSIST     1   /* Commit by zeroing journal header */
#define PAGER_JOURNALMODE_OFF         2   /* Journal omitted.  */
#define PAGER_JOURNALMODE_TRUNCATE    3   /* Commit by truncating journal */
#define PAGER_JOURNALMODE_MEMORY      4   /* In-memory journal file */
#define PAGER_JOURNALMODE_WAL         5   /* Use write-ahead logging */

/*
** Flags that make up the mask passed to sqlite3PagerGet().
*/
#define PAGER_GET_NOCONTENT     0x01  /* Do not load data from disk */
#define PAGER_GET_READONLY      0x02  /* Read-only page is acceptable */

/*
** Flags for sqlite3PagerSetFlags()
**
** Value constraints (enforced via assert()):
**    PAGER_FULLFSYNC      == SQLITE_FullFSync
**    PAGER_CKPT_FULLFSYNC == SQLITE_CkptFullFSync
**    PAGER_CACHE_SPILL    == SQLITE_CacheSpill
*/
#define PAGER_SYNCHRONOUS_OFF       0x01  /* PRAGMA synchronous=OFF */
#define PAGER_SYNCHRONOUS_NORMAL    0x02  /* PRAGMA synchronous=NORMAL */
#define PAGER_SYNCHRONOUS_FULL      0x03  /* PRAGMA synchronous=FULL */
#define PAGER_SYNCHRONOUS_EXTRA     0x04  /* PRAGMA synchronous=EXTRA */
#define PAGER_SYNCHRONOUS_MASK      0x07  /* Mask for four values above */
#define PAGER_FULLFSYNC             0x08  /* PRAGMA fullfsync=ON */
#define PAGER_CKPT_FULLFSYNC        0x10  /* PRAGMA checkpoint_fullfsync=ON */
#define PAGER_CACHESPILL            0x20  /* PRAGMA cache_spill=ON */
#define PAGER_FLAGS_MASK            0x38  /* All above except SYNCHRONOUS */

/*
** The remainder of this file contains the declarations of the functions
** that make up the Pager sub-system API. See source code comments for 
** a detailed description of each routine.
*/

/* Open and close a Pager connection. */ 
SQLITE_PRIVATE int sqlite3PagerOpen(
  sqlite3_vfs*,
  Pager **ppPager,
  const char*,
  int,
  int,
  int,
  void(*)(DbPage*)
);
SQLITE_PRIVATE int sqlite3PagerClose(Pager *pPager);
SQLITE_PRIVATE int sqlite3PagerReadFileheader(Pager*, int, unsigned char*);

/* Functions used to configure a Pager object. */
SQLITE_PRIVATE void sqlite3PagerSetBusyhandler(Pager*, int(*)(void *), void *);
SQLITE_PRIVATE int sqlite3PagerSetPagesize(Pager*, u32*, int);
#ifdef SQLITE_HAS_CODEC
SQLITE_PRIVATE void sqlite3PagerAlignReserve(Pager*,Pager*);
#endif
SQLITE_PRIVATE int sqlite3PagerMaxPageCount(Pager*, int);
SQLITE_PRIVATE void sqlite3PagerSetCachesize(Pager*, int);
SQLITE_PRIVATE int sqlite3PagerSetSpillsize(Pager*, int);
SQLITE_PRIVATE void sqlite3PagerSetMmapLimit(Pager *, sqlite3_int64);
SQLITE_PRIVATE void sqlite3PagerShrink(Pager*);
SQLITE_PRIVATE void sqlite3PagerSetFlags(Pager*,unsigned);
SQLITE_PRIVATE int sqlite3PagerLockingMode(Pager *, int);
SQLITE_PRIVATE int sqlite3PagerSetJournalMode(Pager *, int);
SQLITE_PRIVATE int sqlite3PagerGetJournalMode(Pager*);
SQLITE_PRIVATE int sqlite3PagerOkToChangeJournalMode(Pager*);
SQLITE_PRIVATE i64 sqlite3PagerJournalSizeLimit(Pager *, i64);
SQLITE_PRIVATE sqlite3_backup **sqlite3PagerBackupPtr(Pager*);
SQLITE_PRIVATE int sqlite3PagerFlush(Pager*);

/* Functions used to obtain and release page references. */ 
SQLITE_PRIVATE int sqlite3PagerGet(Pager *pPager, Pgno pgno, DbPage **ppPage, int clrFlag);
SQLITE_PRIVATE DbPage *sqlite3PagerLookup(Pager *pPager, Pgno pgno);
SQLITE_PRIVATE void sqlite3PagerRef(DbPage*);
SQLITE_PRIVATE void sqlite3PagerUnref(DbPage*);
SQLITE_PRIVATE void sqlite3PagerUnrefNotNull(DbPage*);

/* Operations on page references. */
SQLITE_PRIVATE int sqlite3PagerWrite(DbPage*);
SQLITE_PRIVATE void sqlite3PagerDontWrite(DbPage*);
SQLITE_PRIVATE int sqlite3PagerMovepage(Pager*,DbPage*,Pgno,int);
SQLITE_PRIVATE int sqlite3PagerPageRefcount(DbPage*);
SQLITE_PRIVATE void *sqlite3PagerGetData(DbPage *); 
SQLITE_PRIVATE void *sqlite3PagerGetExtra(DbPage *); 

/* Functions used to manage pager transactions and savepoints. */
SQLITE_PRIVATE void sqlite3PagerPagecount(Pager*, int*);
SQLITE_PRIVATE int sqlite3PagerBegin(Pager*, int exFlag, int);
SQLITE_PRIVATE int sqlite3PagerCommitPhaseOne(Pager*,const char *zMaster, int);
SQLITE_PRIVATE int sqlite3PagerExclusiveLock(Pager*);
SQLITE_PRIVATE int sqlite3PagerSync(Pager *pPager, const char *zMaster);
SQLITE_PRIVATE int sqlite3PagerCommitPhaseTwo(Pager*);
SQLITE_PRIVATE int sqlite3PagerRollback(Pager*);
SQLITE_PRIVATE int sqlite3PagerOpenSavepoint(Pager *pPager, int n);
SQLITE_PRIVATE int sqlite3PagerSavepoint(Pager *pPager, int op, int iSavepoint);
SQLITE_PRIVATE int sqlite3PagerSharedLock(Pager *pPager);

#ifndef SQLITE_OMIT_WAL
SQLITE_PRIVATE   int sqlite3PagerCheckpoint(Pager *pPager, int, int*, int*);
SQLITE_PRIVATE   int sqlite3PagerWalSupported(Pager *pPager);
SQLITE_PRIVATE   int sqlite3PagerWalCallback(Pager *pPager);
SQLITE_PRIVATE   int sqlite3PagerOpenWal(Pager *pPager, int *pisOpen);
SQLITE_PRIVATE   int sqlite3PagerCloseWal(Pager *pPager);
# ifdef SQLITE_ENABLE_SNAPSHOT
SQLITE_PRIVATE   int sqlite3PagerSnapshotGet(Pager *pPager, sqlite3_snapshot **ppSnapshot);
SQLITE_PRIVATE   int sqlite3PagerSnapshotOpen(Pager *pPager, sqlite3_snapshot *pSnapshot);
# endif
#endif

#ifdef SQLITE_ENABLE_ZIPVFS
SQLITE_PRIVATE   int sqlite3PagerWalFramesize(Pager *pPager);
#endif

/* Functions used to query pager state and configuration. */
SQLITE_PRIVATE u8 sqlite3PagerIsreadonly(Pager*);
SQLITE_PRIVATE u32 sqlite3PagerDataVersion(Pager*);
#ifdef SQLITE_DEBUG
SQLITE_PRIVATE   int sqlite3PagerRefcount(Pager*);
#endif
SQLITE_PRIVATE int sqlite3PagerMemUsed(Pager*);
SQLITE_PRIVATE const char *sqlite3PagerFilename(Pager*, int);
SQLITE_PRIVATE sqlite3_vfs *sqlite3PagerVfs(Pager*);
SQLITE_PRIVATE sqlite3_file *sqlite3PagerFile(Pager*);
SQLITE_PRIVATE sqlite3_file *sqlite3PagerJrnlFile(Pager*);
SQLITE_PRIVATE const char *sqlite3PagerJournalname(Pager*);
SQLITE_PRIVATE void *sqlite3PagerTempSpace(Pager*);
SQLITE_PRIVATE int sqlite3PagerIsMemdb(Pager*);
SQLITE_PRIVATE void sqlite3PagerCacheStat(Pager *, int, int, int *);
SQLITE_PRIVATE void sqlite3PagerClearCache(Pager *);
SQLITE_PRIVATE int sqlite3SectorSize(sqlite3_file *);

/* Functions used to truncate the database file. */
SQLITE_PRIVATE void sqlite3PagerTruncateImage(Pager*,Pgno);

SQLITE_PRIVATE void sqlite3PagerRekey(DbPage*, Pgno, u16);

#if defined(SQLITE_HAS_CODEC) && !defined(SQLITE_OMIT_WAL)
SQLITE_PRIVATE void *sqlite3PagerCodec(DbPage *);
#endif

/* Functions to support testing and debugging. */
#if !defined(NDEBUG) || defined(SQLITE_TEST)
SQLITE_PRIVATE   Pgno sqlite3PagerPagenumber(DbPage*);
SQLITE_PRIVATE   int sqlite3PagerIswriteable(DbPage*);
#endif
#ifdef SQLITE_TEST
SQLITE_PRIVATE   int *sqlite3PagerStats(Pager*);
SQLITE_PRIVATE   void sqlite3PagerRefdump(Pager*);
  void disable_simulated_io_errors(void);
  void enable_simulated_io_errors(void);
#else
# define disable_simulated_io_errors()
# define enable_simulated_io_errors()
#endif

#endif /* _PAGER_H_ */

/************** End of pager.h ***********************************************/
/************** Continuing where we left off in sqliteInt.h ******************/
/************** Include pcache.h in the middle of sqliteInt.h ****************/
/************** Begin file pcache.h ******************************************/
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
** This header file defines the interface that the sqlite page cache
** subsystem. 
*/

#ifndef _PCACHE_H_

typedef struct PgHdr PgHdr;
typedef struct PCache PCache;

/*
** Every page in the cache is controlled by an instance of the following
** structure.
*/
struct PgHdr {
  sqlite3_pcache_page *pPage;    /* Pcache object page handle */
  void *pData;                   /* Page data */
  void *pExtra;                  /* Extra content */
  PgHdr *pDirty;                 /* Transient list of dirty sorted by pgno */
  Pager *pPager;                 /* The pager this page is part of */
  Pgno pgno;                     /* Page number for this page */
#ifdef SQLITE_CHECK_PAGES
  u32 pageHash;                  /* Hash of page content */
#endif
  u16 flags;                     /* PGHDR flags defined below */

  /**********************************************************************
  ** Elements above are public.  All that follows is private to pcache.c
  ** and should not be accessed by other modules.
  */
  i16 nRef;                      /* Number of users of this page */
  PCache *pCache;                /* Cache that owns this page */

  PgHdr *pDirtyNext;             /* Next element in list of dirty pages */
  PgHdr *pDirtyPrev;             /* Previous element in list of dirty pages */
};

/* Bit values for PgHdr.flags */
#define PGHDR_CLEAN           0x001  /* Page not on the PCache.pDirty list */
#define PGHDR_DIRTY           0x002  /* Page is on the PCache.pDirty list */
#define PGHDR_WRITEABLE       0x004  /* Journaled and ready to modify */
#define PGHDR_NEED_SYNC       0x008  /* Fsync the rollback journal before
                                     ** writing this page to the database */
#define PGHDR_DONT_WRITE      0x010  /* Do not write content to disk */
#define PGHDR_MMAP            0x020  /* This is an mmap page object */

#define PGHDR_WAL_APPEND      0x040  /* Appended to wal file */

/* Initialize and shutdown the page cache subsystem */
SQLITE_PRIVATE int sqlite3PcacheInitialize(void);
SQLITE_PRIVATE void sqlite3PcacheShutdown(void);

/* Page cache buffer management:
** These routines implement SQLITE_CONFIG_PAGECACHE.
*/
SQLITE_PRIVATE void sqlite3PCacheBufferSetup(void *, int sz, int n);

/* Create a new pager cache.
** Under memory stress, invoke xStress to try to make pages clean.
** Only clean and unpinned pages can be reclaimed.
*/
SQLITE_PRIVATE int sqlite3PcacheOpen(
  int szPage,                    /* Size of every page */
  int szExtra,                   /* Extra space associated with each page */
  int bPurgeable,                /* True if pages are on backing store */
  int (*xStress)(void*, PgHdr*), /* Call to try to make pages clean */
  void *pStress,                 /* Argument to xStress */
  PCache *pToInit                /* Preallocated space for the PCache */
);

/* Modify the page-size after the cache has been created. */
SQLITE_PRIVATE int sqlite3PcacheSetPageSize(PCache *, int);

/* Return the size in bytes of a PCache object.  Used to preallocate
** storage space.
*/
SQLITE_PRIVATE int sqlite3PcacheSize(void);

/* One release per successful fetch.  Page is pinned until released.
** Reference counted. 
*/
SQLITE_PRIVATE sqlite3_pcache_page *sqlite3PcacheFetch(PCache*, Pgno, int createFlag);
SQLITE_PRIVATE int sqlite3PcacheFetchStress(PCache*, Pgno, sqlite3_pcache_page**);
SQLITE_PRIVATE PgHdr *sqlite3PcacheFetchFinish(PCache*, Pgno, sqlite3_pcache_page *pPage);
SQLITE_PRIVATE void sqlite3PcacheRelease(PgHdr*);

SQLITE_PRIVATE void sqlite3PcacheDrop(PgHdr*);         /* Remove page from cache */
SQLITE_PRIVATE void sqlite3PcacheMakeDirty(PgHdr*);    /* Make sure page is marked dirty */
SQLITE_PRIVATE void sqlite3PcacheMakeClean(PgHdr*);    /* Mark a single page as clean */
SQLITE_PRIVATE void sqlite3PcacheCleanAll(PCache*);    /* Mark all dirty list pages as clean */
SQLITE_PRIVATE void sqlite3PcacheClearWritable(PCache*);

/* Change a page number.  Used by incr-vacuum. */
SQLITE_PRIVATE void sqlite3PcacheMove(PgHdr*, Pgno);

/* Remove all pages with pgno>x.  Reset the cache if x==0 */
SQLITE_PRIVATE void sqlite3PcacheTruncate(PCache*, Pgno x);

/* Get a list of all dirty pages in the cache, sorted by page number */
SQLITE_PRIVATE PgHdr *sqlite3PcacheDirtyList(PCache*);

/* Reset and close the cache object */
SQLITE_PRIVATE void sqlite3PcacheClose(PCache*);

/* Clear flags from pages of the page cache */
SQLITE_PRIVATE void sqlite3PcacheClearSyncFlags(PCache *);

/* Discard the contents of the cache */
SQLITE_PRIVATE void sqlite3PcacheClear(PCache*);

/* Return the total number of outstanding page references */
SQLITE_PRIVATE int sqlite3PcacheRefCount(PCache*);

/* Increment the reference count of an existing page */
SQLITE_PRIVATE void sqlite3PcacheRef(PgHdr*);

SQLITE_PRIVATE int sqlite3PcachePageRefcount(PgHdr*);

/* Return the total number of pages stored in the cache */
SQLITE_PRIVATE int sqlite3PcachePagecount(PCache*);

#if defined(SQLITE_CHECK_PAGES) || defined(SQLITE_DEBUG)
/* Iterate through all dirty pages currently stored in the cache. This
** interface is only available if SQLITE_CHECK_PAGES is defined when the 
** library is built.
*/
SQLITE_PRIVATE void sqlite3PcacheIterateDirty(PCache *pCache, void (*xIter)(PgHdr *));
#endif

#if defined(SQLITE_DEBUG)
/* Check invariants on a PgHdr object */
SQLITE_PRIVATE int sqlite3PcachePageSanity(PgHdr*);
#endif

/* Set and get the suggested cache-size for the specified pager-cache.
**
** If no global maximum is configured, then the system attempts to limit
** the total number of pages cached by purgeable pager-caches to the sum
** of the suggested cache-sizes.
*/
SQLITE_PRIVATE void sqlite3PcacheSetCachesize(PCache *, int);
#ifdef SQLITE_TEST
SQLITE_PRIVATE int sqlite3PcacheGetCachesize(PCache *);
#endif

/* Set or get the suggested spill-size for the specified pager-cache.
**
** The spill-size is the minimum number of pages in cache before the cache
** will attempt to spill dirty pages by calling xStress.
*/
SQLITE_PRIVATE int sqlite3PcacheSetSpillsize(PCache *, int);

/* Free up as much memory as possible from the page cache */
SQLITE_PRIVATE void sqlite3PcacheShrink(PCache*);

#ifdef SQLITE_ENABLE_MEMORY_MANAGEMENT
/* Try to return memory used by the pcache module to the main memory heap */
SQLITE_PRIVATE int sqlite3PcacheReleaseMemory(int);
#endif

#ifdef SQLITE_TEST
SQLITE_PRIVATE void sqlite3PcacheStats(int*,int*,int*,int*);
#endif

SQLITE_PRIVATE void sqlite3PCacheSetDefault(void);

/* Return the header size */
SQLITE_PRIVATE int sqlite3HeaderSizePcache(void);
SQLITE_PRIVATE int sqlite3HeaderSizePcache1(void);

/* Number of dirty pages as a percentage of the configured cache size */
SQLITE_PRIVATE int sqlite3PCachePercentDirty(PCache*);

#endif /* _PCACHE_H_ */

/************** End of pcache.h **********************************************/
/************** Continuing where we left off in sqliteInt.h ******************/
/************** Include os.h in the middle of sqliteInt.h ********************/
/************** Begin file os.h **********************************************/
/*
** 2001 September 16
**
** The author disclaims copyright to this source code.  In place of
** a legal notice, here is a blessing:
**
**    May you do good and not evil.
**    May you find forgiveness for yourself and forgive others.
**    May you share freely, never taking more than you give.
**
******************************************************************************
**
** This header file (together with is companion C source-code file
** "os.c") attempt to abstract the underlying operating system so that
** the SQLite library will work on both POSIX and windows systems.
**
** This header file is #include-ed by sqliteInt.h and thus ends up
** being included by every source file.
*/
#ifndef _SQLITE_OS_H_
#define _SQLITE_OS_H_

/*
** Attempt to automatically detect the operating system and setup the
** necessary pre-processor macros for it.
*/
/************** Include os_setup.h in the middle of os.h *********************/
/************** Begin file os_setup.h ****************************************/
/*
** 2013 November 25
**
** The author disclaims copyright to this source code.  In place of
** a legal notice, here is a blessing:
**
**    May you do good and not evil.
**    May you find forgiveness for yourself and forgive others.
**    May you share freely, never taking more than you give.
**
******************************************************************************
**
** This file contains pre-processor directives related to operating system
** detection and/or setup.
*/
#ifndef _OS_SETUP_H_
#define _OS_SETUP_H_

/*
** Figure out if we are dealing with Unix, Windows, or some other operating
** system.
**
** After the following block of preprocess macros, all of SQLITE_OS_UNIX,
** SQLITE_OS_WIN, and SQLITE_OS_OTHER will defined to either 1 or 0.  One of
** the three will be 1.  The other two will be 0.
*/
#if defined(SQLITE_OS_OTHER)
#  if SQLITE_OS_OTHER==1
#    undef SQLITE_OS_UNIX
#    define SQLITE_OS_UNIX 0
#    undef SQLITE_OS_WIN
#    define SQLITE_OS_WIN 0
#  else
#    undef SQLITE_OS_OTHER
#  endif
#endif
#if !defined(SQLITE_OS_UNIX) && !defined(SQLITE_OS_OTHER)
#  define SQLITE_OS_OTHER 0
#  ifndef SQLITE_OS_WIN
#    if defined(_WIN32) || defined(WIN32) || defined(__CYGWIN__) || \
        defined(__MINGW32__) || defined(__BORLANDC__)
#      define SQLITE_OS_WIN 1
#      define SQLITE_OS_UNIX 0
#    else
#      define SQLITE_OS_WIN 0
#      define SQLITE_OS_UNIX 1
#    endif
#  else
#    define SQLITE_OS_UNIX 0
#  endif
#else
#  ifndef SQLITE_OS_WIN
#    define SQLITE_OS_WIN 0
#  endif
#endif

#endif /* _OS_SETUP_H_ */

/************** End of os_setup.h ********************************************/
/************** Continuing where we left off in os.h *************************/

/* If the SET_FULLSYNC macro is not defined above, then make it
** a no-op
*/
#ifndef SET_FULLSYNC
# define SET_FULLSYNC(x,y)
#endif

/*
** The default size of a disk sector
*/
#ifndef SQLITE_DEFAULT_SECTOR_SIZE
# define SQLITE_DEFAULT_SECTOR_SIZE 4096
#endif

/*
** Temporary files are named starting with this prefix followed by 16 random
** alphanumeric characters, and no file extension. They are stored in the
** OS's standard temporary file directory, and are deleted prior to exit.
** If sqlite is being embedded in another program, you may wish to change the
** prefix to reflect your program's name, so that if your program exits
** prematurely, old temporary files can be easily identified. This can be done
** using -DSQLITE_TEMP_FILE_PREFIX=myprefix_ on the compiler command line.
**
** 2006-10-31:  The default prefix used to be "sqlite_".  But then
** Mcafee started using SQLite in their anti-virus product and it
** started putting files with the "sqlite" name in the c:/temp folder.
** This annoyed many windows users.  Those users would then do a 
** Google search for "sqlite", find the telephone numbers of the
** developers and call to wake them up at night and complain.
** For this reason, the default name prefix is changed to be "sqlite" 
** spelled backwards.  So the temp files are still identified, but
** anybody smart enough to figure out the code is also likely smart
** enough to know that calling the developer will not help get rid
** of the file.
*/
#ifndef SQLITE_TEMP_FILE_PREFIX
# define SQLITE_TEMP_FILE_PREFIX "etilqs_"
#endif

/*
** The following values may be passed as the second argument to
** sqlite3OsLock(). The various locks exhibit the following semantics:
**
** SHARED:    Any number of processes may hold a SHARED lock simultaneously.
** RESERVED:  A single process may hold a RESERVED lock on a file at
**            any time. Other processes may hold and obtain new SHARED locks.
** PENDING:   A single process may hold a PENDING lock on a file at
**            any one time. Existing SHARED locks may persist, but no new
**            SHARED locks may be obtained by other processes.
** EXCLUSIVE: An EXCLUSIVE lock precludes all other locks.
**
** PENDING_LOCK may not be passed directly to sqlite3OsLock(). Instead, a
** process that requests an EXCLUSIVE lock may actually obtain a PENDING
** lock. This can be upgraded to an EXCLUSIVE lock by a subsequent call to
** sqlite3OsLock().
*/
#define NO_LOCK         0
#define SHARED_LOCK     1
#define RESERVED_LOCK   2
#define PENDING_LOCK    3
#define EXCLUSIVE_LOCK  4

/*
** File Locking Notes:  (Mostly about windows but also some info for Unix)
**
** We cannot use LockFileEx() or UnlockFileEx() on Win95/98/ME because
** those functions are not available.  So we use only LockFile() and
** UnlockFile().
**
** LockFile() prevents not just writing but also reading by other processes.
** A SHARED_LOCK is obtained by locking a single randomly-chosen 
** byte out of a specific range of bytes. The lock byte is obtained at 
** random so two separate readers can probably access the file at the 
** same time, unless they are unlucky and choose the same lock byte.
** An EXCLUSIVE_LOCK is obtained by locking all bytes in the range.
** There can only be one writer.  A RESERVED_LOCK is obtained by locking
** a single byte of the file that is designated as the reserved lock byte.
** A PENDING_LOCK is obtained by locking a designated byte different from
** the RESERVED_LOCK byte.
**
** On WinNT/2K/XP systems, LockFileEx() and UnlockFileEx() are available,
** which means we can use reader/writer locks.  When reader/writer locks
** are used, the lock is placed on the same range of bytes that is used
** for probabilistic locking in Win95/98/ME.  Hence, the locking scheme
** will support two or more Win95 readers or two or more WinNT readers.
** But a single Win95 reader will lock out all WinNT readers and a single
** WinNT reader will lock out all other Win95 readers.
**
** The following #defines specify the range of bytes used for locking.
** SHARED_SIZE is the number of bytes available in the pool from which
** a random byte is selected for a shared lock.  The pool of bytes for
** shared locks begins at SHARED_FIRST. 
**
** The same locking strategy and
** byte ranges are used for Unix.  This leaves open the possibility of having
** clients on win95, winNT, and unix all talking to the same shared file
** and all locking correctly.  To do so would require that samba (or whatever
** tool is being used for file sharing) implements locks correctly between
** windows and unix.  I'm guessing that isn't likely to happen, but by
** using the same locking range we are at least open to the possibility.
**
** Locking in windows is manditory.  For this reason, we cannot store
** actual data in the bytes used for locking.  The pager never allocates
** the pages involved in locking therefore.  SHARED_SIZE is selected so
** that all locks will fit on a single page even at the minimum page size.
** PENDING_BYTE defines the beginning of the locks.  By default PENDING_BYTE
** is set high so that we don't have to allocate an unused page except
** for very large databases.  But one should test the page skipping logic 
** by setting PENDING_BYTE low and running the entire regression suite.
**
** Changing the value of PENDING_BYTE results in a subtly incompatible
** file format.  Depending on how it is changed, you might not notice
** the incompatibility right away, even running a full regression test.
** The default location of PENDING_BYTE is the first byte past the
** 1GB boundary.
**
*/
#ifdef SQLITE_OMIT_WSD
# define PENDING_BYTE     (0x40000000)
#else
# define PENDING_BYTE      sqlite3PendingByte
#endif
#define RESERVED_BYTE     (PENDING_BYTE+1)
#define SHARED_FIRST      (PENDING_BYTE+2)
#define SHARED_SIZE       510

/*
** Wrapper around OS specific sqlite3_os_init() function.
*/
SQLITE_PRIVATE int sqlite3OsInit(void);

/* 
** Functions for accessing sqlite3_file methods 
*/
SQLITE_PRIVATE void sqlite3OsClose(sqlite3_file*);
SQLITE_PRIVATE int sqlite3OsRead(sqlite3_file*, void*, int amt, i64 offset);
SQLITE_PRIVATE int sqlite3OsWrite(sqlite3_file*, const void*, int amt, i64 offset);
SQLITE_PRIVATE int sqlite3OsTruncate(sqlite3_file*, i64 size);
SQLITE_PRIVATE int sqlite3OsSync(sqlite3_file*, int);
SQLITE_PRIVATE int sqlite3OsFileSize(sqlite3_file*, i64 *pSize);
SQLITE_PRIVATE int sqlite3OsLock(sqlite3_file*, int);
SQLITE_PRIVATE int sqlite3OsUnlock(sqlite3_file*, int);
SQLITE_PRIVATE int sqlite3OsCheckReservedLock(sqlite3_file *id, int *pResOut);
SQLITE_PRIVATE int sqlite3OsFileControl(sqlite3_file*,int,void*);
SQLITE_PRIVATE void sqlite3OsFileControlHint(sqlite3_file*,int,void*);
#define SQLITE_FCNTL_DB_UNCHANGED 0xca093fa0
SQLITE_PRIVATE int sqlite3OsSectorSize(sqlite3_file *id);
SQLITE_PRIVATE int sqlite3OsDeviceCharacteristics(sqlite3_file *id);
SQLITE_PRIVATE int sqlite3OsShmMap(sqlite3_file *,int,int,int,void volatile **);
SQLITE_PRIVATE int sqlite3OsShmLock(sqlite3_file *id, int, int, int);
SQLITE_PRIVATE void sqlite3OsShmBarrier(sqlite3_file *id);
SQLITE_PRIVATE int sqlite3OsShmUnmap(sqlite3_file *id, int);
SQLITE_PRIVATE int sqlite3OsFetch(sqlite3_file *id, i64, int, void **);
SQLITE_PRIVATE int sqlite3OsUnfetch(sqlite3_file *, i64, void *);


/* 
** Functions for accessing sqlite3_vfs methods 
*/
SQLITE_PRIVATE int sqlite3OsOpen(sqlite3_vfs *, const char *, sqlite3_file*, int, int *);
SQLITE_PRIVATE int sqlite3OsDelete(sqlite3_vfs *, const char *, int);
SQLITE_PRIVATE int sqlite3OsAccess(sqlite3_vfs *, const char *, int, int *pResOut);
SQLITE_PRIVATE int sqlite3OsFullPathname(sqlite3_vfs *, const char *, int, char *);
#ifndef SQLITE_OMIT_LOAD_EXTENSION
SQLITE_PRIVATE void *sqlite3OsDlOpen(sqlite3_vfs *, const char *);
SQLITE_PRIVATE void sqlite3OsDlError(sqlite3_vfs *, int, char *);
SQLITE_PRIVATE void (*sqlite3OsDlSym(sqlite3_vfs *, void *, const char *))(void);
SQLITE_PRIVATE void sqlite3OsDlClose(sqlite3_vfs *, void *);
#endif /* SQLITE_OMIT_LOAD_EXTENSION */
SQLITE_PRIVATE int sqlite3OsRandomness(sqlite3_vfs *, int, char *);
SQLITE_PRIVATE int sqlite3OsSleep(sqlite3_vfs *, int);
SQLITE_PRIVATE int sqlite3OsGetLastError(sqlite3_vfs*);
SQLITE_PRIVATE int sqlite3OsCurrentTimeInt64(sqlite3_vfs *, sqlite3_int64*);

/*
** Convenience functions for opening and closing files using 
** sqlite3_malloc() to obtain space for the file-handle structure.
*/
SQLITE_PRIVATE int sqlite3OsOpenMalloc(sqlite3_vfs *, const char *, sqlite3_file **, int,int*);
SQLITE_PRIVATE void sqlite3OsCloseFree(sqlite3_file *);

#endif /* _SQLITE_OS_H_ */

/************** End of os.h **************************************************/
/************** Continuing where we left off in sqliteInt.h ******************/
/************** Include mutex.h in the middle of sqliteInt.h *****************/
/************** Begin file mutex.h *******************************************/
/*
** 2007 August 28
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
** This file contains the common header for all mutex implementations.
** The sqliteInt.h header #includes this file so that it is available
** to all source files.  We break it out in an effort to keep the code
** better organized.
**
** NOTE:  source files should *not* #include this header file directly.
** Source files should #include the sqliteInt.h file and let that file
** include this one indirectly.
*/


/*
** Figure out what version of the code to use.  The choices are
**
**   SQLITE_MUTEX_OMIT         No mutex logic.  Not even stubs.  The
**                             mutexes implementation cannot be overridden
**                             at start-time.
**
**   SQLITE_MUTEX_NOOP         For single-threaded applications.  No
**                             mutual exclusion is provided.  But this
**                             implementation can be overridden at
**                             start-time.
**
**   SQLITE_MUTEX_PTHREADS     For multi-threaded applications on Unix.
**
**   SQLITE_MUTEX_W32          For multi-threaded applications on Win32.
*/
#if !SQLITE_THREADSAFE
# define SQLITE_MUTEX_OMIT
#endif
#if SQLITE_THREADSAFE && !defined(SQLITE_MUTEX_NOOP)
#  if SQLITE_OS_UNIX
#    define SQLITE_MUTEX_PTHREADS
#  elif SQLITE_OS_WIN
#    define SQLITE_MUTEX_W32
#  else
#    define SQLITE_MUTEX_NOOP
#  endif
#endif

#ifdef SQLITE_MUTEX_OMIT
/*
** If this is a no-op implementation, implement everything as macros.
*/
#define sqlite3_mutex_alloc(X)    ((sqlite3_mutex*)8)
#define sqlite3_mutex_free(X)
#define sqlite3_mutex_enter(X)    
#define sqlite3_mutex_try(X)      SQLITE_OK
#define sqlite3_mutex_leave(X)    
#define sqlite3_mutex_held(X)     ((void)(X),1)
#define sqlite3_mutex_notheld(X)  ((void)(X),1)
#define sqlite3MutexAlloc(X)      ((sqlite3_mutex*)8)
#define sqlite3MutexInit()        SQLITE_OK
#define sqlite3MutexEnd()
#define MUTEX_LOGIC(X)
#else
#define MUTEX_LOGIC(X)            X
#endif /* defined(SQLITE_MUTEX_OMIT) */

/************** End of mutex.h ***********************************************/
/************** Continuing where we left off in sqliteInt.h ******************/

/* The SQLITE_EXTRA_DURABLE compile-time option used to set the default
** synchronous setting to EXTRA.  It is no longer supported.
*/
#ifdef SQLITE_EXTRA_DURABLE
# warning Use SQLITE_DEFAULT_SYNCHRONOUS=3 instead of SQLITE_EXTRA_DURABLE
# define SQLITE_DEFAULT_SYNCHRONOUS 3
#endif

/*
** Default synchronous levels.
**
** Note that (for historcal reasons) the PAGER_SYNCHRONOUS_* macros differ
** from the SQLITE_DEFAULT_SYNCHRONOUS value by 1.
**
**           PAGER_SYNCHRONOUS       DEFAULT_SYNCHRONOUS
**   OFF           1                         0
**   NORMAL        2                         1
**   FULL          3                         2
**   EXTRA         4                         3
**
** The "PRAGMA synchronous" statement also uses the zero-based numbers.
** In other words, the zero-based numbers are used for all external interfaces
** and the one-based values are used internally.
*/
#ifndef SQLITE_DEFAULT_SYNCHRONOUS
# define SQLITE_DEFAULT_SYNCHRONOUS (PAGER_SYNCHRONOUS_FULL-1)
#endif
#ifndef SQLITE_DEFAULT_WAL_SYNCHRONOUS
# define SQLITE_DEFAULT_WAL_SYNCHRONOUS SQLITE_DEFAULT_SYNCHRONOUS
#endif

/*
** Each database file to be accessed by the system is an instance
** of the following structure.  There are normally two of these structures
** in the sqlite.aDb[] array.  aDb[0] is the main database file and
** aDb[1] is the database file used to hold temporary tables.  Additional
** databases may be attached.
*/
struct Db {
  char *zName;         /* Name of this database */
  Btree *pBt;          /* The B*Tree structure for this database file */
  u8 safety_level;     /* How aggressive at syncing data to disk */
  u8 bSyncSet;         /* True if "PRAGMA synchronous=N" has been run */
  Schema *pSchema;     /* Pointer to database schema (possibly shared) */
};

/*
** An instance of the following structure stores a database schema.
**
** Most Schema objects are associated with a Btree.  The exception is
** the Schema for the TEMP databaes (sqlite3.aDb[1]) which is free-standing.
** In shared cache mode, a single Schema object can be shared by multiple
** Btrees that refer to the same underlying BtShared object.
**
** Schema objects are automatically deallocated when the last Btree that
** references them is destroyed.   The TEMP Schema is manually freed by
** sqlite3_close().
*
** A thread must be holding a mutex on the corresponding Btree in order
** to access Schema content.  This implies that the thread must also be
** holding a mutex on the sqlite3 connection pointer that owns the Btree.
** For a TEMP Schema, only the connection mutex is required.
*/
struct Schema {
  int schema_cookie;   /* Database schema version number for this file */
  int iGeneration;     /* Generation counter.  Incremented with each change */
  Hash tblHash;        /* All tables indexed by name */
  Hash idxHash;        /* All (named) indices indexed by name */
  Hash trigHash;       /* All triggers indexed by name */
  Hash fkeyHash;       /* All foreign keys by referenced table name */
  Table *pSeqTab;      /* The sqlite_sequence table used by AUTOINCREMENT */
  u8 file_format;      /* Schema format version for this file */
  u8 enc;              /* Text encoding used by this database */
  u16 schemaFlags;     /* Flags associated with this schema */
  int cache_size;      /* Number of pages to use in the cache */
};

/*
** These macros can be used to test, set, or clear bits in the
** Db.pSchema->flags field.
*/
#define DbHasProperty(D,I,P)     (((D)->aDb[I].pSchema->schemaFlags&(P))==(P))
#define DbHasAnyProperty(D,I,P)  (((D)->aDb[I].pSchema->schemaFlags&(P))!=0)
#define DbSetProperty(D,I,P)     (D)->aDb[I].pSchema->schemaFlags|=(P)
#define DbClearProperty(D,I,P)   (D)->aDb[I].pSchema->schemaFlags&=~(P)

/*
** Allowed values for the DB.pSchema->flags field.
**
** The DB_SchemaLoaded flag is set after the database schema has been
** read into internal hash tables.
**
** DB_UnresetViews means that one or more views have column names that
** have been filled out.  If the schema changes, these column names might
** changes and so the view will need to be reset.
*/
#define DB_SchemaLoaded    0x0001  /* The schema has been loaded */
#define DB_UnresetViews    0x0002  /* Some views have defined column names */
#define DB_Empty           0x0004  /* The file is empty (length 0 bytes) */

/*
** The number of different kinds of things that can be limited
** using the sqlite3_limit() interface.
*/
#define SQLITE_N_LIMIT (SQLITE_LIMIT_WORKER_THREADS+1)

/*
** Lookaside malloc is a set of fixed-size buffers that can be used
** to satisfy small transient memory allocation requests for objects
** associated with a particular database connection.  The use of
** lookaside malloc provides a significant performance enhancement
** (approx 10%) by avoiding numerous malloc/free requests while parsing
** SQL statements.
**
** The Lookaside structure holds configuration information about the
** lookaside malloc subsystem.  Each available memory allocation in
** the lookaside subsystem is stored on a linked list of LookasideSlot
** objects.
**
** Lookaside allocations are only allowed for objects that are associated
** with a particular database connection.  Hence, schema information cannot
** be stored in lookaside because in shared cache mode the schema information
** is shared by multiple database connections.  Therefore, while parsing
** schema information, the Lookaside.bEnabled flag is cleared so that
** lookaside allocations are not used to construct the schema objects.
*/
struct Lookaside {
  u32 bDisable;           /* Only operate the lookaside when zero */
  u16 sz;                 /* Size of each buffer in bytes */
  u8 bMalloced;           /* True if pStart obtained from sqlite3_malloc() */
  int nOut;               /* Number of buffers currently checked out */
  int mxOut;              /* Highwater mark for nOut */
  int anStat[3];          /* 0: hits.  1: size misses.  2: full misses */
  LookasideSlot *pFree;   /* List of available buffers */
  void *pStart;           /* First byte of available memory space */
  void *pEnd;             /* First byte past end of available space */
};
struct LookasideSlot {
  LookasideSlot *pNext;    /* Next buffer in the list of free buffers */
};

/*
** A hash table for built-in function definitions.  (Application-defined
** functions use a regular table table from hash.h.)
**
** Hash each FuncDef structure into one of the FuncDefHash.a[] slots.
** Collisions are on the FuncDef.u.pHash chain.
*/
#define SQLITE_FUNC_HASH_SZ 23
struct FuncDefHash {
  FuncDef *a[SQLITE_FUNC_HASH_SZ];       /* Hash table for functions */
};

#ifdef SQLITE_USER_AUTHENTICATION
/*
** Information held in the "sqlite3" database connection object and used
** to manage user authentication.
*/
typedef struct sqlite3_userauth sqlite3_userauth;
struct sqlite3_userauth {
  u8 authLevel;                 /* Current authentication level */
  int nAuthPW;                  /* Size of the zAuthPW in bytes */
  char *zAuthPW;                /* Password used to authenticate */
  char *zAuthUser;              /* User name used to authenticate */
};

/* Allowed values for sqlite3_userauth.authLevel */
#define UAUTH_Unknown     0     /* Authentication not yet checked */
#define UAUTH_Fail        1     /* User authentication failed */
#define UAUTH_User        2     /* Authenticated as a normal user */
#define UAUTH_Admin       3     /* Authenticated as an administrator */

/* Functions used only by user authorization logic */
SQLITE_PRIVATE int sqlite3UserAuthTable(const char*);
SQLITE_PRIVATE int sqlite3UserAuthCheckLogin(sqlite3*,const char*,u8*);
SQLITE_PRIVATE void sqlite3UserAuthInit(sqlite3*);
SQLITE_PRIVATE void sqlite3CryptFunc(sqlite3_context*,int,sqlite3_value**);

#endif /* SQLITE_USER_AUTHENTICATION */

/*
** typedef for the authorization callback function.
*/
#ifdef SQLITE_USER_AUTHENTICATION
  typedef int (*sqlite3_xauth)(void*,int,const char*,const char*,const char*,
                               const char*, const char*);
#else
  typedef int (*sqlite3_xauth)(void*,int,const char*,const char*,const char*,
                               const char*);
#endif


/*
** Each database connection is an instance of the following structure.
*/
struct sqlite3 {
  sqlite3_vfs *pVfs;            /* OS Interface */
  struct Vdbe *pVdbe;           /* List of active virtual machines */
  CollSeq *pDfltColl;           /* The default collating sequence (BINARY) */
  sqlite3_mutex *mutex;         /* Connection mutex */
  Db *aDb;                      /* All backends */
  int nDb;                      /* Number of backends currently in use */
  int flags;                    /* Miscellaneous flags. See below */
  i64 lastRowid;                /* ROWID of most recent insert (see above) */
  i64 szMmap;                   /* Default mmap_size setting */
  unsigned int openFlags;       /* Flags passed to sqlite3_vfs.xOpen() */
  int errCode;                  /* Most recent error code (SQLITE_*) */
  int errMask;                  /* & result codes with this before returning */
  int iSysErrno;                /* Errno value from last system error */
  u16 dbOptFlags;               /* Flags to enable/disable optimizations */
  u8 enc;                       /* Text encoding */
  u8 autoCommit;                /* The auto-commit flag. */
  u8 temp_store;                /* 1: file 2: memory 0: default */
  u8 mallocFailed;              /* True if we have seen a malloc failure */
  u8 bBenignMalloc;             /* Do not require OOMs if true */
  u8 dfltLockMode;              /* Default locking-mode for attached dbs */
  signed char nextAutovac;      /* Autovac setting after VACUUM if >=0 */
  u8 suppressErr;               /* Do not issue error messages if true */
  u8 vtabOnConflict;            /* Value to return for s3_vtab_on_conflict() */
  u8 isTransactionSavepoint;    /* True if the outermost savepoint is a TS */
  int nextPagesize;             /* Pagesize after VACUUM if >0 */
  u32 magic;                    /* Magic number for detect library misuse */
  int nChange;                  /* Value returned by sqlite3_changes() */
  int nTotalChange;             /* Value returned by sqlite3_total_changes() */
  int aLimit[SQLITE_N_LIMIT];   /* Limits */
  int nMaxSorterMmap;           /* Maximum size of regions mapped by sorter */
  struct sqlite3InitInfo {      /* Information used during initialization */
    int newTnum;                /* Rootpage of table being initialized */
    u8 iDb;                     /* Which db file is being initialized */
    u8 busy;                    /* TRUE if currently initializing */
    u8 orphanTrigger;           /* Last statement is orphaned TEMP trigger */
    u8 imposterTable;           /* Building an imposter table */
  } init;
  int nVdbeActive;              /* Number of VDBEs currently running */
  int nVdbeRead;                /* Number of active VDBEs that read or write */
  int nVdbeWrite;               /* Number of active VDBEs that read and write */
  int nVdbeExec;                /* Number of nested calls to VdbeExec() */
  int nVDestroy;                /* Number of active OP_VDestroy operations */
  int nExtension;               /* Number of loaded extensions */
  void **aExtension;            /* Array of shared library handles */
  void (*xTrace)(void*,const char*);        /* Trace function */
  void *pTraceArg;                          /* Argument to the trace function */
  void (*xProfile)(void*,const char*,u64);  /* Profiling function */
  void *pProfileArg;                        /* Argument to profile function */
  void *pCommitArg;                 /* Argument to xCommitCallback() */
  int (*xCommitCallback)(void*);    /* Invoked at every commit. */
  void *pRollbackArg;               /* Argument to xRollbackCallback() */
  void (*xRollbackCallback)(void*); /* Invoked at every commit. */
  void *pUpdateArg;
  void (*xUpdateCallback)(void*,int, const char*,const char*,sqlite_int64);
#ifdef SQLITE_ENABLE_PREUPDATE_HOOK
  void *pPreUpdateArg;          /* First argument to xPreUpdateCallback */
  void (*xPreUpdateCallback)(   /* Registered using sqlite3_preupdate_hook() */
    void*,sqlite3*,int,char const*,char const*,sqlite3_int64,sqlite3_int64
  );
  PreUpdate *pPreUpdate;        /* Context for active pre-update callback */
#endif /* SQLITE_ENABLE_PREUPDATE_HOOK */
#ifndef SQLITE_OMIT_WAL
  int (*xWalCallback)(void *, sqlite3 *, const char *, int);
  void *pWalArg;
#endif
  void(*xCollNeeded)(void*,sqlite3*,int eTextRep,const char*);
  void(*xCollNeeded16)(void*,sqlite3*,int eTextRep,const void*);
  void *pCollNeededArg;
  sqlite3_value *pErr;          /* Most recent error message */
  union {
    volatile int isInterrupted; /* True if sqlite3_interrupt has been called */
    double notUsed1;            /* Spacer */
  } u1;
  Lookaside lookaside;          /* Lookaside malloc configuration */
#ifndef SQLITE_OMIT_AUTHORIZATION
  sqlite3_xauth xAuth;          /* Access authorization function */
  void *pAuthArg;               /* 1st argument to the access auth function */
#endif
#ifndef SQLITE_OMIT_PROGRESS_CALLBACK
  int (*xProgress)(void *);     /* The progress callback */
  void *pProgressArg;           /* Argument to the progress callback */
  unsigned nProgressOps;        /* Number of opcodes for progress callback */
#endif
#ifndef SQLITE_OMIT_VIRTUALTABLE
  int nVTrans;                  /* Allocated size of aVTrans */
  Hash aModule;                 /* populated by sqlite3_create_module() */
  VtabCtx *pVtabCtx;            /* Context for active vtab connect/create */
  VTable **aVTrans;             /* Virtual tables with open transactions */
  VTable *pDisconnect;    /* Disconnect these in next sqlite3_prepare() */
#endif
  Hash aFunc;                   /* Hash table of connection functions */
  Hash aCollSeq;                /* All collating sequences */
  BusyHandler busyHandler;      /* Busy callback */
  Db aDbStatic[2];              /* Static space for the 2 default backends */
  Savepoint *pSavepoint;        /* List of active savepoints */
  int busyTimeout;              /* Busy handler timeout, in msec */
  int nSavepoint;               /* Number of non-transaction savepoints */
  int nStatement;               /* Number of nested statement-transactions  */
  i64 nDeferredCons;            /* Net deferred constraints this transaction. */
  i64 nDeferredImmCons;         /* Net deferred immediate constraints */
  int *pnBytesFreed;            /* If not NULL, increment this in DbFree() */
#ifdef SQLITE_ENABLE_UNLOCK_NOTIFY
  /* The following variables are all protected by the STATIC_MASTER
  ** mutex, not by sqlite3.mutex. They are used by code in notify.c.
  **
  ** When X.pUnlockConnection==Y, that means that X is waiting for Y to
  ** unlock so that it can proceed.
  **
  ** When X.pBlockingConnection==Y, that means that something that X tried
  ** tried to do recently failed with an SQLITE_LOCKED error due to locks
  ** held by Y.
  */
  sqlite3 *pBlockingConnection; /* Connection that caused SQLITE_LOCKED */
  sqlite3 *pUnlockConnection;           /* Connection to watch for unlock */
  void *pUnlockArg;                     /* Argument to xUnlockNotify */
  void (*xUnlockNotify)(void **, int);  /* Unlock notify callback */
  sqlite3 *pNextBlocked;        /* Next in list of all blocked connections */
#endif
#ifdef SQLITE_USER_AUTHENTICATION
  sqlite3_userauth auth;        /* User authentication information */
#endif
};

/*
** A macro to discover the encoding of a database.
*/
#define SCHEMA_ENC(db) ((db)->aDb[0].pSchema->enc)
#define ENC(db)        ((db)->enc)

/*
** Possible values for the sqlite3.flags.
**
** Value constraints (enforced via assert()):
**      SQLITE_FullFSync     == PAGER_FULLFSYNC
**      SQLITE_CkptFullFSync == PAGER_CKPT_FULLFSYNC
**      SQLITE_CacheSpill    == PAGER_CACHE_SPILL
*/
#define SQLITE_VdbeTrace      0x00000001  /* True to trace VDBE execution */
#define SQLITE_InternChanges  0x00000002  /* Uncommitted Hash table changes */
#define SQLITE_FullColNames   0x00000004  /* Show full column names on SELECT */
#define SQLITE_FullFSync      0x00000008  /* Use full fsync on the backend */
#define SQLITE_CkptFullFSync  0x00000010  /* Use full fsync for checkpoint */
#define SQLITE_CacheSpill     0x00000020  /* OK to spill pager cache */
#define SQLITE_ShortColNames  0x00000040  /* Show short columns names */
#define SQLITE_CountRows      0x00000080  /* Count rows changed by INSERT, */
                                          /*   DELETE, or UPDATE and return */
                                          /*   the count using a callback. */
#define SQLITE_NullCallback   0x00000100  /* Invoke the callback once if the */
                                          /*   result set is empty */
#define SQLITE_SqlTrace       0x00000200  /* Debug print SQL as it executes */
#define SQLITE_VdbeListing    0x00000400  /* Debug listings of VDBE programs */
#define SQLITE_WriteSchema    0x00000800  /* OK to update SQLITE_MASTER */
#define SQLITE_VdbeAddopTrace 0x00001000  /* Trace sqlite3VdbeAddOp() calls */
#define SQLITE_IgnoreChecks   0x00002000  /* Do not enforce check constraints */
#define SQLITE_ReadUncommitted 0x0004000  /* For shared-cache mode */
#define SQLITE_LegacyFileFmt  0x00008000  /* Create new databases in format 1 */
#define SQLITE_RecoveryMode   0x00010000  /* Ignore schema errors */
#define SQLITE_ReverseOrder   0x00020000  /* Reverse unordered SELECTs */
#define SQLITE_RecTriggers    0x00040000  /* Enable recursive triggers */
#define SQLITE_ForeignKeys    0x00080000  /* Enforce foreign key constraints  */
#define SQLITE_AutoIndex      0x00100000  /* Enable automatic indexes */
#define SQLITE_PreferBuiltin  0x00200000  /* Preference to built-in funcs */
#define SQLITE_LoadExtension  0x00400000  /* Enable load_extension */
#define SQLITE_LoadExtFunc    0x00800000  /* Enable load_extension() SQL func */
#define SQLITE_EnableTrigger  0x01000000  /* True to enable triggers */
#define SQLITE_DeferFKs       0x02000000  /* Defer all FK constraints */
#define SQLITE_QueryOnly      0x04000000  /* Disable database changes */
#define SQLITE_VdbeEQP        0x08000000  /* Debug EXPLAIN QUERY PLAN */
#define SQLITE_Vacuum         0x10000000  /* Currently in a VACUUM */
#define SQLITE_CellSizeCk     0x20000000  /* Check btree cell sizes on load */
#define SQLITE_Fts3Tokenizer  0x40000000  /* Enable fts3_tokenizer(2) */


/*
** Bits of the sqlite3.dbOptFlags field that are used by the
** sqlite3_test_control(SQLITE_TESTCTRL_OPTIMIZATIONS,...) interface to
** selectively disable various optimizations.
*/
#define SQLITE_QueryFlattener 0x0001   /* Query flattening */
#define SQLITE_ColumnCache    0x0002   /* Column cache */
#define SQLITE_GroupByOrder   0x0004   /* GROUPBY cover of ORDERBY */
#define SQLITE_FactorOutConst 0x0008   /* Constant factoring */
/*                not used    0x0010   // Was: SQLITE_IdxRealAsInt */
#define SQLITE_DistinctOpt    0x0020   /* DISTINCT using indexes */
#define SQLITE_CoverIdxScan   0x0040   /* Covering index scans */
#define SQLITE_OrderByIdxJoin 0x0080   /* ORDER BY of joins via index */
#define SQLITE_SubqCoroutine  0x0100   /* Evaluate subqueries as coroutines */
#define SQLITE_Transitive     0x0200   /* Transitive constraints */
#define SQLITE_OmitNoopJoin   0x0400   /* Omit unused tables in joins */
#define SQLITE_Stat34         0x0800   /* Use STAT3 or STAT4 data */
#define SQLITE_CursorHints    0x2000   /* Add OP_CursorHint opcodes */
#define SQLITE_AllOpts        0xffff   /* All optimizations */

/*
** Macros for testing whether or not optimizations are enabled or disabled.
*/
#ifndef SQLITE_OMIT_BUILTIN_TEST
#define OptimizationDisabled(db, mask)  (((db)->dbOptFlags&(mask))!=0)
#define OptimizationEnabled(db, mask)   (((db)->dbOptFlags&(mask))==0)
#else
#define OptimizationDisabled(db, mask)  0
#define OptimizationEnabled(db, mask)   1
#endif

/*
** Return true if it OK to factor constant expressions into the initialization
** code. The argument is a Parse object for the code generator.
*/
#define ConstFactorOk(P) ((P)->okConstFactor)

/*
** Possible values for the sqlite.magic field.
** The numbers are obtained at random and have no special meaning, other
** than being distinct from one another.
*/
#define SQLITE_MAGIC_OPEN     0xa029a697  /* Database is open */
#define SQLITE_MAGIC_CLOSED   0x9f3c2d33  /* Database is closed */
#define SQLITE_MAGIC_SICK     0x4b771290  /* Error and awaiting close */
#define SQLITE_MAGIC_BUSY     0xf03b7906  /* Database currently in use */
#define SQLITE_MAGIC_ERROR    0xb5357930  /* An SQLITE_MISUSE error occurred */
#define SQLITE_MAGIC_ZOMBIE   0x64cffc7f  /* Close with last statement close */

/*
** Each SQL function is defined by an instance of the following
** structure.  For global built-in functions (ex: substr(), max(), count())
** a pointer to this structure is held in the sqlite3BuiltinFunctions object.
** For per-connection application-defined functions, a pointer to this
** structure is held in the db->aHash hash table.
**
** The u.pHash field is used by the global built-ins.  The u.pDestructor
** field is used by per-connection app-def functions.
*/
struct FuncDef {
  i8 nArg;             /* Number of arguments.  -1 means unlimited */
  u16 funcFlags;       /* Some combination of SQLITE_FUNC_* */
  void *pUserData;     /* User data parameter */
  FuncDef *pNext;      /* Next function with same name */
  void (*xSFunc)(sqlite3_context*,int,sqlite3_value**); /* func or agg-step */
  void (*xFinalize)(sqlite3_context*);                  /* Agg finalizer */
  const char *zName;   /* SQL name of the function. */
  union {
    FuncDef *pHash;      /* Next with a different name but the same hash */
    FuncDestructor *pDestructor;   /* Reference counted destructor function */
  } u;
};

/*
** This structure encapsulates a user-function destructor callback (as
** configured using create_function_v2()) and a reference counter. When
** create_function_v2() is called to create a function with a destructor,
** a single object of this type is allocated. FuncDestructor.nRef is set to
** the number of FuncDef objects created (either 1 or 3, depending on whether
** or not the specified encoding is SQLITE_ANY). The FuncDef.pDestructor
** member of each of the new FuncDef objects is set to point to the allocated
** FuncDestructor.
**
** Thereafter, when one of the FuncDef objects is deleted, the reference
** count on this object is decremented. When it reaches 0, the destructor
** is invoked and the FuncDestructor structure freed.
*/
struct FuncDestructor {
  int nRef;
  void (*xDestroy)(void *);
  void *pUserData;
};

/*
** Possible values for FuncDef.flags.  Note that the _LENGTH and _TYPEOF
** values must correspond to OPFLAG_LENGTHARG and OPFLAG_TYPEOFARG.  And
** SQLITE_FUNC_CONSTANT must be the same as SQLITE_DETERMINISTIC.  There
** are assert() statements in the code to verify this.
**
** Value constraints (enforced via assert()):
**     SQLITE_FUNC_MINMAX    ==  NC_MinMaxAgg      == SF_MinMaxAgg
**     SQLITE_FUNC_LENGTH    ==  OPFLAG_LENGTHARG
**     SQLITE_FUNC_TYPEOF    ==  OPFLAG_TYPEOFARG
**     SQLITE_FUNC_CONSTANT  ==  SQLITE_DETERMINISTIC from the API
**     SQLITE_FUNC_ENCMASK   depends on SQLITE_UTF* macros in the API
*/
#define SQLITE_FUNC_ENCMASK  0x0003 /* SQLITE_UTF8, SQLITE_UTF16BE or UTF16LE */
#define SQLITE_FUNC_LIKE     0x0004 /* Candidate for the LIKE optimization */
#define SQLITE_FUNC_CASE     0x0008 /* Case-sensitive LIKE-type function */
#define SQLITE_FUNC_EPHEM    0x0010 /* Ephemeral.  Delete with VDBE */
#define SQLITE_FUNC_NEEDCOLL 0x0020 /* sqlite3GetFuncCollSeq() might be called*/
#define SQLITE_FUNC_LENGTH   0x0040 /* Built-in length() function */
#define SQLITE_FUNC_TYPEOF   0x0080 /* Built-in typeof() function */
#define SQLITE_FUNC_COUNT    0x0100 /* Built-in count(*) aggregate */
#define SQLITE_FUNC_COALESCE 0x0200 /* Built-in coalesce() or ifnull() */
#define SQLITE_FUNC_UNLIKELY 0x0400 /* Built-in unlikely() function */
#define SQLITE_FUNC_CONSTANT 0x0800 /* Constant inputs give a constant output */
#define SQLITE_FUNC_MINMAX   0x1000 /* True for min() and max() aggregates */
#define SQLITE_FUNC_SLOCHNG  0x2000 /* "Slow Change". Value constant during a
                                    ** single query - might change over time */

/*
** The following three macros, FUNCTION(), LIKEFUNC() and AGGREGATE() are
** used to create the initializers for the FuncDef structures.
**
**   FUNCTION(zName, nArg, iArg, bNC, xFunc)
**     Used to create a scalar function definition of a function zName
**     implemented by C function xFunc that accepts nArg arguments. The
**     value passed as iArg is cast to a (void*) and made available
**     as the user-data (sqlite3_user_data()) for the function. If
**     argument bNC is true, then the SQLITE_FUNC_NEEDCOLL flag is set.
**
**   VFUNCTION(zName, nArg, iArg, bNC, xFunc)
**     Like FUNCTION except it omits the SQLITE_FUNC_CONSTANT flag.
**
**   DFUNCTION(zName, nArg, iArg, bNC, xFunc)
**     Like FUNCTION except it omits the SQLITE_FUNC_CONSTANT flag and
**     adds the SQLITE_FUNC_SLOCHNG flag.  Used for date & time functions
**     and functions like sqlite_version() that can change, but not during
**     a single query.
**
**   AGGREGATE(zName, nArg, iArg, bNC, xStep, xFinal)
**     Used to create an aggregate function definition implemented by
**     the C functions xStep and xFinal. The first four parameters
**     are interpreted in the same way as the first 4 parameters to
**     FUNCTION().
**
**   LIKEFUNC(zName, nArg, pArg, flags)
**     Used to create a scalar function definition of a function zName
**     that accepts nArg arguments and is implemented by a call to C
**     function likeFunc. Argument pArg is cast to a (void *) and made
**     available as the function user-data (sqlite3_user_data()). The
**     FuncDef.flags variable is set to the value passed as the flags
**     parameter.
*/
#define FUNCTION(zName, nArg, iArg, bNC, xFunc) \
  {nArg, SQLITE_FUNC_CONSTANT|SQLITE_UTF8|(bNC*SQLITE_FUNC_NEEDCOLL), \
   SQLITE_INT_TO_PTR(iArg), 0, xFunc, 0, #zName, {0} }
#define VFUNCTION(zName, nArg, iArg, bNC, xFunc) \
  {nArg, SQLITE_UTF8|(bNC*SQLITE_FUNC_NEEDCOLL), \
   SQLITE_INT_TO_PTR(iArg), 0, xFunc, 0, #zName, {0} }
#define DFUNCTION(zName, nArg, iArg, bNC, xFunc) \
  {nArg, SQLITE_FUNC_SLOCHNG|SQLITE_UTF8|(bNC*SQLITE_FUNC_NEEDCOLL), \
   SQLITE_INT_TO_PTR(iArg), 0, xFunc, 0, #zName, {0} }
#define FUNCTION2(zName, nArg, iArg, bNC, xFunc, extraFlags) \
  {nArg,SQLITE_FUNC_CONSTANT|SQLITE_UTF8|(bNC*SQLITE_FUNC_NEEDCOLL)|extraFlags,\
   SQLITE_INT_TO_PTR(iArg), 0, xFunc, 0, #zName, {0} }
#define STR_FUNCTION(zName, nArg, pArg, bNC, xFunc) \
  {nArg, SQLITE_FUNC_SLOCHNG|SQLITE_UTF8|(bNC*SQLITE_FUNC_NEEDCOLL), \
   pArg, 0, xFunc, 0, #zName, }
#define LIKEFUNC(zName, nArg, arg, flags) \
  {nArg, SQLITE_FUNC_CONSTANT|SQLITE_UTF8|flags, \
   (void *)arg, 0, likeFunc, 0, #zName, {0} }
#define AGGREGATE(zName, nArg, arg, nc, xStep, xFinal) \
  {nArg, SQLITE_UTF8|(nc*SQLITE_FUNC_NEEDCOLL), \
   SQLITE_INT_TO_PTR(arg), 0, xStep,xFinal,#zName, {0}}
#define AGGREGATE2(zName, nArg, arg, nc, xStep, xFinal, extraFlags) \
  {nArg, SQLITE_UTF8|(nc*SQLITE_FUNC_NEEDCOLL)|extraFlags, \
   SQLITE_INT_TO_PTR(arg), 0, xStep,xFinal,#zName, {0}}

/*
** All current savepoints are stored in a linked list starting at
** sqlite3.pSavepoint. The first element in the list is the most recently
** opened savepoint. Savepoints are added to the list by the vdbe
** OP_Savepoint instruction.
*/
struct Savepoint {
  char *zName;                        /* Savepoint name (nul-terminated) */
  i64 nDeferredCons;                  /* Number of deferred fk violations */
  i64 nDeferredImmCons;               /* Number of deferred imm fk. */
  Savepoint *pNext;                   /* Parent savepoint (if any) */
};

/*
** The following are used as the second parameter to sqlite3Savepoint(),
** and as the P1 argument to the OP_Savepoint instruction.
*/
#define SAVEPOINT_BEGIN      0
#define SAVEPOINT_RELEASE    1
#define SAVEPOINT_ROLLBACK   2


/*
** Each SQLite module (virtual table definition) is defined by an
** instance of the following structure, stored in the sqlite3.aModule
** hash table.
*/
struct Module {
  const sqlite3_module *pModule;       /* Callback pointers */
  const char *zName;                   /* Name passed to create_module() */
  void *pAux;                          /* pAux passed to create_module() */
  void (*xDestroy)(void *);            /* Module destructor function */
  Table *pEpoTab;                      /* Eponymous table for this module */
};

/*
** information about each column of an SQL table is held in an instance
** of this structure.
*/
struct Column {
  char *zName;     /* Name of this column, \000, then the type */
  Expr *pDflt;     /* Default value of this column */
  char *zColl;     /* Collating sequence.  If NULL, use the default */
  u8 notNull;      /* An OE_ code for handling a NOT NULL constraint */
  char affinity;   /* One of the SQLITE_AFF_... values */
  u8 szEst;        /* Estimated size of value in this column. sizeof(INT)==1 */
  u8 colFlags;     /* Boolean properties.  See COLFLAG_ defines below */
};

/* Allowed values for Column.colFlags:
*/
#define COLFLAG_PRIMKEY  0x0001    /* Column is part of the primary key */
#define COLFLAG_HIDDEN   0x0002    /* A hidden column in a virtual table */
#define COLFLAG_HASTYPE  0x0004    /* Type name follows column name */

/*
** A "Collating Sequence" is defined by an instance of the following
** structure. Conceptually, a collating sequence consists of a name and
** a comparison routine that defines the order of that sequence.
**
** If CollSeq.xCmp is NULL, it means that the
** collating sequence is undefined.  Indices built on an undefined
** collating sequence may not be read or written.
*/
struct CollSeq {
  char *zName;          /* Name of the collating sequence, UTF-8 encoded */
  u8 enc;               /* Text encoding handled by xCmp() */
  void *pUser;          /* First argument to xCmp() */
  int (*xCmp)(void*,int, const void*, int, const void*);
  void (*xDel)(void*);  /* Destructor for pUser */
};

/*
** A sort order can be either ASC or DESC.
*/
#define SQLITE_SO_ASC       0  /* Sort in ascending order */
#define SQLITE_SO_DESC      1  /* Sort in ascending order */
#define SQLITE_SO_UNDEFINED -1 /* No sort order specified */

/*
** Column affinity types.
**
** These used to have mnemonic name like 'i' for SQLITE_AFF_INTEGER and
** 't' for SQLITE_AFF_TEXT.  But we can save a little space and improve
** the speed a little by numbering the values consecutively.
**
** But rather than start with 0 or 1, we begin with 'A'.  That way,
** when multiple affinity types are concatenated into a string and
** used as the P4 operand, they will be more readable.
**
** Note also that the numeric types are grouped together so that testing
** for a numeric type is a single comparison.  And the BLOB type is first.
*/
#define SQLITE_AFF_BLOB     'A'
#define SQLITE_AFF_TEXT     'B'
#define SQLITE_AFF_NUMERIC  'C'
#define SQLITE_AFF_INTEGER  'D'
#define SQLITE_AFF_REAL     'E'

#define sqlite3IsNumericAffinity(X)  ((X)>=SQLITE_AFF_NUMERIC)

/*
** The SQLITE_AFF_MASK values masks off the significant bits of an
** affinity value.
*/
#define SQLITE_AFF_MASK     0x47

/*
** Additional bit values that can be ORed with an affinity without
** changing the affinity.
**
** The SQLITE_NOTNULL flag is a combination of NULLEQ and JUMPIFNULL.
** It causes an assert() to fire if either operand to a comparison
** operator is NULL.  It is added to certain comparison operators to
** prove that the operands are always NOT NULL.
*/
#define SQLITE_JUMPIFNULL   0x10  /* jumps if either operand is NULL */
#define SQLITE_STOREP2      0x20  /* Store result in reg[P2] rather than jump */
#define SQLITE_NULLEQ       0x80  /* NULL=NULL */
#define SQLITE_NOTNULL      0x90  /* Assert that operands are never NULL */

/*
** An object of this type is created for each virtual table present in
** the database schema.
**
** If the database schema is shared, then there is one instance of this
** structure for each database connection (sqlite3*) that uses the shared
** schema. This is because each database connection requires its own unique
** instance of the sqlite3_vtab* handle used to access the virtual table
** implementation. sqlite3_vtab* handles can not be shared between
** database connections, even when the rest of the in-memory database
** schema is shared, as the implementation often stores the database
** connection handle passed to it via the xConnect() or xCreate() method
** during initialization internally. This database connection handle may
** then be used by the virtual table implementation to access real tables
** within the database. So that they appear as part of the callers
** transaction, these accesses need to be made via the same database
** connection as that used to execute SQL operations on the virtual table.
**
** All VTable objects that correspond to a single table in a shared
** database schema are initially stored in a linked-list pointed to by
** the Table.pVTable member variable of the corresponding Table object.
** When an sqlite3_prepare() operation is required to access the virtual
** table, it searches the list for the VTable that corresponds to the
** database connection doing the preparing so as to use the correct
** sqlite3_vtab* handle in the compiled query.
**
** When an in-memory Table object is deleted (for example when the
** schema is being reloaded for some reason), the VTable objects are not
** deleted and the sqlite3_vtab* handles are not xDisconnect()ed
** immediately. Instead, they are moved from the Table.pVTable list to
** another linked list headed by the sqlite3.pDisconnect member of the
** corresponding sqlite3 structure. They are then deleted/xDisconnected
** next time a statement is prepared using said sqlite3*. This is done
** to avoid deadlock issues involving multiple sqlite3.mutex mutexes.
** Refer to comments above function sqlite3VtabUnlockList() for an
** explanation as to why it is safe to add an entry to an sqlite3.pDisconnect
** list without holding the corresponding sqlite3.mutex mutex.
**
** The memory for objects of this type is always allocated by
** sqlite3DbMalloc(), using the connection handle stored in VTable.db as
** the first argument.
*/
struct VTable {
  sqlite3 *db;              /* Database connection associated with this table */
  Module *pMod;             /* Pointer to module implementation */
  sqlite3_vtab *pVtab;      /* Pointer to vtab instance */
  int nRef;                 /* Number of pointers to this structure */
  u8 bConstraint;           /* True if constraints are supported */
  int iSavepoint;           /* Depth of the SAVEPOINT stack */
  VTable *pNext;            /* Next in linked list (see above) */
};

/*
** The schema for each SQL table and view is represented in memory
** by an instance of the following structure.
*/
struct Table {
  char *zName;         /* Name of the table or view */
  Column *aCol;        /* Information about each column */
  Index *pIndex;       /* List of SQL indexes on this table. */
  Select *pSelect;     /* NULL for tables.  Points to definition if a view. */
  FKey *pFKey;         /* Linked list of all foreign keys in this table */
  char *zColAff;       /* String defining the affinity of each column */
  ExprList *pCheck;    /* All CHECK constraints */
                       /*   ... also used as column name list in a VIEW */
  int tnum;            /* Root BTree page for this table */
  i16 iPKey;           /* If not negative, use aCol[iPKey] as the rowid */
  i16 nCol;            /* Number of columns in this table */
  u16 nRef;            /* Number of pointers to this Table */
  LogEst nRowLogEst;   /* Estimated rows in table - from sqlite_stat1 table */
  LogEst szTabRow;     /* Estimated size of each table row in bytes */
#ifdef SQLITE_ENABLE_COSTMULT
  LogEst costMult;     /* Cost multiplier for using this table */
#endif
  u8 tabFlags;         /* Mask of TF_* values */
  u8 keyConf;          /* What to do in case of uniqueness conflict on iPKey */
#ifndef SQLITE_OMIT_ALTERTABLE
  int addColOffset;    /* Offset in CREATE TABLE stmt to add a new column */
#endif
#ifndef SQLITE_OMIT_VIRTUALTABLE
  int nModuleArg;      /* Number of arguments to the module */
  char **azModuleArg;  /* 0: module 1: schema 2: vtab name 3...: args */
  VTable *pVTable;     /* List of VTable objects. */
#endif
  Trigger *pTrigger;   /* List of triggers stored in pSchema */
  Schema *pSchema;     /* Schema that contains this table */
  Table *pNextZombie;  /* Next on the Parse.pZombieTab list */
};

/*
** Allowed values for Table.tabFlags.
**
** TF_OOOHidden applies to tables or view that have hidden columns that are
** followed by non-hidden columns.  Example:  "CREATE VIRTUAL TABLE x USING
** vtab1(a HIDDEN, b);".  Since "b" is a non-hidden column but "a" is hidden,
** the TF_OOOHidden attribute would apply in this case.  Such tables require
** special handling during INSERT processing.
*/
#define TF_Readonly        0x01    /* Read-only system table */
#define TF_Ephemeral       0x02    /* An ephemeral table */
#define TF_HasPrimaryKey   0x04    /* Table has a primary key */
#define TF_Autoincrement   0x08    /* Integer primary key is autoincrement */
#define TF_Virtual         0x10    /* Is a virtual table */
#define TF_WithoutRowid    0x20    /* No rowid.  PRIMARY KEY is the key */
#define TF_NoVisibleRowid  0x40    /* No user-visible "rowid" column */
#define TF_OOOHidden       0x80    /* Out-of-Order hidden columns */


/*
** Test to see whether or not a table is a virtual table.  This is
** done as a macro so that it will be optimized out when virtual
** table support is omitted from the build.
*/
#ifndef SQLITE_OMIT_VIRTUALTABLE
#  define IsVirtual(X)      (((X)->tabFlags & TF_Virtual)!=0)
#else
#  define IsVirtual(X)      0
#endif

/*
** Macros to determine if a column is hidden.  IsOrdinaryHiddenColumn()
** only works for non-virtual tables (ordinary tables and views) and is
** always false unless SQLITE_ENABLE_HIDDEN_COLUMNS is defined.  The
** IsHiddenColumn() macro is general purpose.
*/
#if defined(SQLITE_ENABLE_HIDDEN_COLUMNS)
#  define IsHiddenColumn(X)         (((X)->colFlags & COLFLAG_HIDDEN)!=0)
#  define IsOrdinaryHiddenColumn(X) (((X)->colFlags & COLFLAG_HIDDEN)!=0)
#elif !defined(SQLITE_OMIT_VIRTUALTABLE)
#  define IsHiddenColumn(X)         (((X)->colFlags & COLFLAG_HIDDEN)!=0)
#  define IsOrdinaryHiddenColumn(X) 0
#else
#  define IsHiddenColumn(X)         0
#  define IsOrdinaryHiddenColumn(X) 0
#endif


/* Does the table have a rowid */
#define HasRowid(X)     (((X)->tabFlags & TF_WithoutRowid)==0)
#define VisibleRowid(X) (((X)->tabFlags & TF_NoVisibleRowid)==0)

/*
** Each foreign key constraint is an instance of the following structure.
**
** A foreign key is associated with two tables.  The "from" table is
** the table that contains the REFERENCES clause that creates the foreign
** key.  The "to" table is the table that is named in the REFERENCES clause.
** Consider this example:
**
**     CREATE TABLE ex1(
**       a INTEGER PRIMARY KEY,
**       b INTEGER CONSTRAINT fk1 REFERENCES ex2(x)
**     );
**
** For foreign key "fk1", the from-table is "ex1" and the to-table is "ex2".
** Equivalent names:
**
**     from-table == child-table
**       to-table == parent-table
**
** Each REFERENCES clause generates an instance of the following structure
** which is attached to the from-table.  The to-table need not exist when
** the from-table is created.  The existence of the to-table is not checked.
**
** The list of all parents for child Table X is held at X.pFKey.
**
** A list of all children for a table named Z (which might not even exist)
** is held in Schema.fkeyHash with a hash key of Z.
*/
struct FKey {
  Table *pFrom;     /* Table containing the REFERENCES clause (aka: Child) */
  FKey *pNextFrom;  /* Next FKey with the same in pFrom. Next parent of pFrom */
  char *zTo;        /* Name of table that the key points to (aka: Parent) */
  FKey *pNextTo;    /* Next with the same zTo. Next child of zTo. */
  FKey *pPrevTo;    /* Previous with the same zTo */
  int nCol;         /* Number of columns in this key */
  /* EV: R-30323-21917 */
  u8 isDeferred;       /* True if constraint checking is deferred till COMMIT */
  u8 aAction[2];        /* ON DELETE and ON UPDATE actions, respectively */
  Trigger *apTrigger[2];/* Triggers for aAction[] actions */
  struct sColMap {      /* Mapping of columns in pFrom to columns in zTo */
    int iFrom;            /* Index of column in pFrom */
    char *zCol;           /* Name of column in zTo.  If NULL use PRIMARY KEY */
  } aCol[1];            /* One entry for each of nCol columns */
};

/*
** SQLite supports many different ways to resolve a constraint
** error.  ROLLBACK processing means that a constraint violation
** causes the operation in process to fail and for the current transaction
** to be rolled back.  ABORT processing means the operation in process
** fails and any prior changes from that one operation are backed out,
** but the transaction is not rolled back.  FAIL processing means that
** the operation in progress stops and returns an error code.  But prior
** changes due to the same operation are not backed out and no rollback
** occurs.  IGNORE means that the particular row that caused the constraint
** error is not inserted or updated.  Processing continues and no error
** is returned.  REPLACE means that preexisting database rows that caused
** a UNIQUE constraint violation are removed so that the new insert or
** update can proceed.  Processing continues and no error is reported.
**
** RESTRICT, SETNULL, and CASCADE actions apply only to foreign keys.
** RESTRICT is the same as ABORT for IMMEDIATE foreign keys and the
** same as ROLLBACK for DEFERRED keys.  SETNULL means that the foreign
** key is set to NULL.  CASCADE means that a DELETE or UPDATE of the
** referenced table row is propagated into the row that holds the
** foreign key.
**
** The following symbolic values are used to record which type
** of action to take.
*/
#define OE_None     0   /* There is no constraint to check */
#define OE_Rollback 1   /* Fail the operation and rollback the transaction */
#define OE_Abort    2   /* Back out changes but do no rollback transaction */
#define OE_Fail     3   /* Stop the operation but leave all prior changes */
#define OE_Ignore   4   /* Ignore the error. Do not do the INSERT or UPDATE */
#define OE_Replace  5   /* Delete existing record, then do INSERT or UPDATE */

#define OE_Restrict 6   /* OE_Abort for IMMEDIATE, OE_Rollback for DEFERRED */
#define OE_SetNull  7   /* Set the foreign key value to NULL */
#define OE_SetDflt  8   /* Set the foreign key value to its default */
#define OE_Cascade  9   /* Cascade the changes */

#define OE_Default  10  /* Do whatever the default action is */


/*
** An instance of the following structure is passed as the first
** argument to sqlite3VdbeKeyCompare and is used to control the
** comparison of the two index keys.
**
** Note that aSortOrder[] and aColl[] have nField+1 slots.  There
** are nField slots for the columns of an index then one extra slot
** for the rowid at the end.
*/
struct KeyInfo {
  u32 nRef;           /* Number of references to this KeyInfo object */
  u8 enc;             /* Text encoding - one of the SQLITE_UTF* values */
  u16 nField;         /* Number of key columns in the index */
  u16 nXField;        /* Number of columns beyond the key columns */
  sqlite3 *db;        /* The database connection */
  u8 *aSortOrder;     /* Sort order for each column. */
  CollSeq *aColl[1];  /* Collating sequence for each term of the key */
};

/*
** This object holds a record which has been parsed out into individual
** fields, for the purposes of doing a comparison.
**
** A record is an object that contains one or more fields of data.
** Records are used to store the content of a table row and to store
** the key of an index.  A blob encoding of a record is created by
** the OP_MakeRecord opcode of the VDBE and is disassembled by the
** OP_Column opcode.
**
** An instance of this object serves as a "key" for doing a search on
** an index b+tree. The goal of the search is to find the entry that
** is closed to the key described by this object.  This object might hold
** just a prefix of the key.  The number of fields is given by
** pKeyInfo->nField.
**
** The r1 and r2 fields are the values to return if this key is less than
** or greater than a key in the btree, respectively.  These are normally
** -1 and +1 respectively, but might be inverted to +1 and -1 if the b-tree
** is in DESC order.
**
** The key comparison functions actually return default_rc when they find
** an equals comparison.  default_rc can be -1, 0, or +1.  If there are
** multiple entries in the b-tree with the same key (when only looking
** at the first pKeyInfo->nFields,) then default_rc can be set to -1 to
** cause the search to find the last match, or +1 to cause the search to
** find the first match.
**
** The key comparison functions will set eqSeen to true if they ever
** get and equal results when comparing this structure to a b-tree record.
** When default_rc!=0, the search might end up on the record immediately
** before the first match or immediately after the last match.  The
** eqSeen field will indicate whether or not an exact match exists in the
** b-tree.
*/
struct UnpackedRecord {
  KeyInfo *pKeyInfo;  /* Collation and sort-order information */
  Mem *aMem;          /* Values */
  u16 nField;         /* Number of entries in apMem[] */
  i8 default_rc;      /* Comparison result if keys are equal */
  u8 errCode;         /* Error detected by xRecordCompare (CORRUPT or NOMEM) */
  i8 r1;              /* Value to return if (lhs > rhs) */
  i8 r2;              /* Value to return if (rhs < lhs) */
  u8 eqSeen;          /* True if an equality comparison has been seen */
};


/*
** Each SQL index is represented in memory by an
** instance of the following structure.
**
** The columns of the table that are to be indexed are described
** by the aiColumn[] field of this structure.  For example, suppose
** we have the following table and index:
**
**     CREATE TABLE Ex1(c1 int, c2 int, c3 text);
**     CREATE INDEX Ex2 ON Ex1(c3,c1);
**
** In the Table structure describing Ex1, nCol==3 because there are
** three columns in the table.  In the Index structure describing
** Ex2, nColumn==2 since 2 of the 3 columns of Ex1 are indexed.
** The value of aiColumn is {2, 0}.  aiColumn[0]==2 because the
** first column to be indexed (c3) has an index of 2 in Ex1.aCol[].
** The second column to be indexed (c1) has an index of 0 in
** Ex1.aCol[], hence Ex2.aiColumn[1]==0.
**
** The Index.onError field determines whether or not the indexed columns
** must be unique and what to do if they are not.  When Index.onError=OE_None,
** it means this is not a unique index.  Otherwise it is a unique index
** and the value of Index.onError indicate the which conflict resolution
** algorithm to employ whenever an attempt is made to insert a non-unique
** element.
**
** While parsing a CREATE TABLE or CREATE INDEX statement in order to
** generate VDBE code (as opposed to parsing one read from an sqlite_master
** table as part of parsing an existing database schema), transient instances
** of this structure may be created. In this case the Index.tnum variable is
** used to store the address of a VDBE instruction, not a database page
** number (it cannot - the database page is not allocated until the VDBE
** program is executed). See convertToWithoutRowidTable() for details.
*/
struct Index {
  char *zName;             /* Name of this index */
  i16 *aiColumn;           /* Which columns are used by this index.  1st is 0 */
  LogEst *aiRowLogEst;     /* From ANALYZE: Est. rows selected by each column */
  Table *pTable;           /* The SQL table being indexed */
  char *zColAff;           /* String defining the affinity of each column */
  Index *pNext;            /* The next index associated with the same table */
  Schema *pSchema;         /* Schema containing this index */
  u8 *aSortOrder;          /* for each column: True==DESC, False==ASC */
  const char **azColl;     /* Array of collation sequence names for index */
  Expr *pPartIdxWhere;     /* WHERE clause for partial indices */
  ExprList *aColExpr;      /* Column expressions */
  int tnum;                /* DB Page containing root of this index */
  LogEst szIdxRow;         /* Estimated average row size in bytes */
  u16 nKeyCol;             /* Number of columns forming the key */
  u16 nColumn;             /* Number of columns stored in the index */
  u8 onError;              /* OE_Abort, OE_Ignore, OE_Replace, or OE_None */
  unsigned idxType:2;      /* 1==UNIQUE, 2==PRIMARY KEY, 0==CREATE INDEX */
  unsigned bUnordered:1;   /* Use this index for == or IN queries only */
  unsigned uniqNotNull:1;  /* True if UNIQUE and NOT NULL for all columns */
  unsigned isResized:1;    /* True if resizeIndexObject() has been called */
  unsigned isCovering:1;   /* True if this is a covering index */
  unsigned noSkipScan:1;   /* Do not try to use skip-scan if true */
#ifdef SQLITE_ENABLE_STAT3_OR_STAT4
  int nSample;             /* Number of elements in aSample[] */
  int nSampleCol;          /* Size of IndexSample.anEq[] and so on */
  tRowcnt *aAvgEq;         /* Average nEq values for keys not in aSample */
  IndexSample *aSample;    /* Samples of the left-most key */
  tRowcnt *aiRowEst;       /* Non-logarithmic stat1 data for this index */
  tRowcnt nRowEst0;        /* Non-logarithmic number of rows in the index */
#endif
};

/*
** Allowed values for Index.idxType
*/
#define SQLITE_IDXTYPE_APPDEF      0   /* Created using CREATE INDEX */
#define SQLITE_IDXTYPE_UNIQUE      1   /* Implements a UNIQUE constraint */
#define SQLITE_IDXTYPE_PRIMARYKEY  2   /* Is the PRIMARY KEY for the table */

/* Return true if index X is a PRIMARY KEY index */
#define IsPrimaryKeyIndex(X)  ((X)->idxType==SQLITE_IDXTYPE_PRIMARYKEY)

/* Return true if index X is a UNIQUE index */
#define IsUniqueIndex(X)      ((X)->onError!=OE_None)

/* The Index.aiColumn[] values are normally positive integer.  But
** there are some negative values that have special meaning:
*/
#define XN_ROWID     (-1)     /* Indexed column is the rowid */
#define XN_EXPR      (-2)     /* Indexed column is an expression */

/*
** Each sample stored in the sqlite_stat3 table is represented in memory
** using a structure of this type.  See documentation at the top of the
** analyze.c source file for additional information.
*/
struct IndexSample {
  void *p;          /* Pointer to sampled record */
  int n;            /* Size of record in bytes */
  tRowcnt *anEq;    /* Est. number of rows where the key equals this sample */
  tRowcnt *anLt;    /* Est. number of rows where key is less than this sample */
  tRowcnt *anDLt;   /* Est. number of distinct keys less than this sample */
};

/*
** Each token coming out of the lexer is an instance of
** this structure.  Tokens are also used as part of an expression.
**
** Note if Token.z==0 then Token.dyn and Token.n are undefined and
** may contain random values.  Do not make any assumptions about Token.dyn
** and Token.n when Token.z==0.
*/
struct Token {
  const char *z;     /* Text of the token.  Not NULL-terminated! */
  unsigned int n;    /* Number of characters in this token */
};

/*
** An instance of this structure contains information needed to generate
** code for a SELECT that contains aggregate functions.
**
** If Expr.op==TK_AGG_COLUMN or TK_AGG_FUNCTION then Expr.pAggInfo is a
** pointer to this structure.  The Expr.iColumn field is the index in
** AggInfo.aCol[] or AggInfo.aFunc[] of information needed to generate
** code for that node.
**
** AggInfo.pGroupBy and AggInfo.aFunc.pExpr point to fields within the
** original Select structure that describes the SELECT statement.  These
** fields do not need to be freed when deallocating the AggInfo structure.
*/
struct AggInfo {
  u8 directMode;          /* Direct rendering mode means take data directly
                          ** from source tables rather than from accumulators */
  u8 useSortingIdx;       /* In direct mode, reference the sorting index rather
                          ** than the source table */
  int sortingIdx;         /* Cursor number of the sorting index */
  int sortingIdxPTab;     /* Cursor number of pseudo-table */
  int nSortingColumn;     /* Number of columns in the sorting index */
  int mnReg, mxReg;       /* Range of registers allocated for aCol and aFunc */
  ExprList *pGroupBy;     /* The group by clause */
  struct AggInfo_col {    /* For each column used in source tables */
    Table *pTab;             /* Source table */
    int iTable;              /* Cursor number of the source table */
    int iColumn;             /* Column number within the source table */
    int iSorterColumn;       /* Column number in the sorting index */
    int iMem;                /* Memory location that acts as accumulator */
    Expr *pExpr;             /* The original expression */
  } *aCol;
  int nColumn;            /* Number of used entries in aCol[] */
  int nAccumulator;       /* Number of columns that show through to the output.
                          ** Additional columns are used only as parameters to
                          ** aggregate functions */
  struct AggInfo_func {   /* For each aggregate function */
    Expr *pExpr;             /* Expression encoding the function */
    FuncDef *pFunc;          /* The aggregate function implementation */
    int iMem;                /* Memory location that acts as accumulator */
    int iDistinct;           /* Ephemeral table used to enforce DISTINCT */
  } *aFunc;
  int nFunc;              /* Number of entries in aFunc[] */
};

/*
** The datatype ynVar is a signed integer, either 16-bit or 32-bit.
** Usually it is 16-bits.  But if SQLITE_MAX_VARIABLE_NUMBER is greater
** than 32767 we have to make it 32-bit.  16-bit is preferred because
** it uses less memory in the Expr object, which is a big memory user
** in systems with lots of prepared statements.  And few applications
** need more than about 10 or 20 variables.  But some extreme users want
** to have prepared statements with over 32767 variables, and for them
** the option is available (at compile-time).
*/
#if SQLITE_MAX_VARIABLE_NUMBER<=32767
typedef i16 ynVar;
#else
typedef int ynVar;
#endif

/*
** Each node of an expression in the parse tree is an instance
** of this structure.
**
** Expr.op is the opcode. The integer parser token codes are reused
** as opcodes here. For example, the parser defines TK_GE to be an integer
** code representing the ">=" operator. This same integer code is reused
** to represent the greater-than-or-equal-to operator in the expression
** tree.
**
** If the expression is an SQL literal (TK_INTEGER, TK_FLOAT, TK_BLOB,
** or TK_STRING), then Expr.token contains the text of the SQL literal. If
** the expression is a variable (TK_VARIABLE), then Expr.token contains the
** variable name. Finally, if the expression is an SQL function (TK_FUNCTION),
** then Expr.token contains the name of the function.
**
** Expr.pRight and Expr.pLeft are the left and right subexpressions of a
** binary operator. Either or both may be NULL.
**
** Expr.x.pList is a list of arguments if the expression is an SQL function,
** a CASE expression or an IN expression of the form "<lhs> IN (<y>, <z>...)".
** Expr.x.pSelect is used if the expression is a sub-select or an expression of
** the form "<lhs> IN (SELECT ...)". If the EP_xIsSelect bit is set in the
** Expr.flags mask, then Expr.x.pSelect is valid. Otherwise, Expr.x.pList is
** valid.
**
** An expression of the form ID or ID.ID refers to a column in a table.
** For such expressions, Expr.op is set to TK_COLUMN and Expr.iTable is
** the integer cursor number of a VDBE cursor pointing to that table and
** Expr.iColumn is the column number for the specific column.  If the
** expression is used as a result in an aggregate SELECT, then the
** value is also stored in the Expr.iAgg column in the aggregate so that
** it can be accessed after all aggregates are computed.
**
** If the expression is an unbound variable marker (a question mark
** character '?' in the original SQL) then the Expr.iTable holds the index
** number for that variable.
**
** If the expression is a subquery then Expr.iColumn holds an integer
** register number containing the result of the subquery.  If the
** subquery gives a constant result, then iTable is -1.  If the subquery
** gives a different answer at different times during statement processing
** then iTable is the address of a subroutine that computes the subquery.
**
** If the Expr is of type OP_Column, and the table it is selecting from
** is a disk table or the "old.*" pseudo-table, then pTab points to the
** corresponding table definition.
**
** ALLOCATION NOTES:
**
** Expr objects can use a lot of memory space in database schema.  To
** help reduce memory requirements, sometimes an Expr object will be
** truncated.  And to reduce the number of memory allocations, sometimes
** two or more Expr objects will be stored in a single memory allocation,
** together with Expr.zToken strings.
**
** If the EP_Reduced and EP_TokenOnly flags are set when
** an Expr object is truncated.  When EP_Reduced is set, then all
** the child Expr objects in the Expr.pLeft and Expr.pRight subtrees
** are contained within the same memory allocation.  Note, however, that
** the subtrees in Expr.x.pList or Expr.x.pSelect are always separately
** allocated, regardless of whether or not EP_Reduced is set.
*/
struct Expr {
  u8 op;                 /* Operation performed by this node */
  char affinity;         /* The affinity of the column or 0 if not a column */
  u32 flags;             /* Various flags.  EP_* See below */
  union {
    char *zToken;          /* Token value. Zero terminated and dequoted */
    int iValue;            /* Non-negative integer value if EP_IntValue */
  } u;

  /* If the EP_TokenOnly flag is set in the Expr.flags mask, then no
  ** space is allocated for the fields below this point. An attempt to
  ** access them will result in a segfault or malfunction.
  *********************************************************************/

  Expr *pLeft;           /* Left subnode */
  Expr *pRight;          /* Right subnode */
  union {
    ExprList *pList;     /* op = IN, EXISTS, SELECT, CASE, FUNCTION, BETWEEN */
    Select *pSelect;     /* EP_xIsSelect and op = IN, EXISTS, SELECT */
  } x;

  /* If the EP_Reduced flag is set in the Expr.flags mask, then no
  ** space is allocated for the fields below this point. An attempt to
  ** access them will result in a segfault or malfunction.
  *********************************************************************/

#if SQLITE_MAX_EXPR_DEPTH>0
  int nHeight;           /* Height of the tree headed by this node */
#endif
  int iTable;            /* TK_COLUMN: cursor number of table holding column
                         ** TK_REGISTER: register number
                         ** TK_TRIGGER: 1 -> new, 0 -> old
                         ** EP_Unlikely:  134217728 times likelihood */
  ynVar iColumn;         /* TK_COLUMN: column index.  -1 for rowid.
                         ** TK_VARIABLE: variable number (always >= 1). */
  i16 iAgg;              /* Which entry in pAggInfo->aCol[] or ->aFunc[] */
  i16 iRightJoinTable;   /* If EP_FromJoin, the right table of the join */
  u8 op2;                /* TK_REGISTER: original value of Expr.op
                         ** TK_COLUMN: the value of p5 for OP_Column
                         ** TK_AGG_FUNCTION: nesting depth */
  AggInfo *pAggInfo;     /* Used by TK_AGG_COLUMN and TK_AGG_FUNCTION */
  Table *pTab;           /* Table for TK_COLUMN expressions. */
};

/*
** The following are the meanings of bits in the Expr.flags field.
*/
#define EP_FromJoin  0x000001 /* Originates in ON/USING clause of outer join */
#define EP_Agg       0x000002 /* Contains one or more aggregate functions */
#define EP_Resolved  0x000004 /* IDs have been resolved to COLUMNs */
#define EP_Error     0x000008 /* Expression contains one or more errors */
#define EP_Distinct  0x000010 /* Aggregate function with DISTINCT keyword */
#define EP_VarSelect 0x000020 /* pSelect is correlated, not constant */
#define EP_DblQuoted 0x000040 /* token.z was originally in "..." */
#define EP_InfixFunc 0x000080 /* True for an infix function: LIKE, GLOB, etc */
#define EP_Collate   0x000100 /* Tree contains a TK_COLLATE operator */
#define EP_Generic   0x000200 /* Ignore COLLATE or affinity on this tree */
#define EP_IntValue  0x000400 /* Integer value contained in u.iValue */
#define EP_xIsSelect 0x000800 /* x.pSelect is valid (otherwise x.pList is) */
#define EP_Skip      0x001000 /* COLLATE, AS, or UNLIKELY */
#define EP_Reduced   0x002000 /* Expr struct EXPR_REDUCEDSIZE bytes only */
#define EP_TokenOnly 0x004000 /* Expr struct EXPR_TOKENONLYSIZE bytes only */
#define EP_Static    0x008000 /* Held in memory not obtained from malloc() */
#define EP_MemToken  0x010000 /* Need to sqlite3DbFree() Expr.zToken */
#define EP_NoReduce  0x020000 /* Cannot EXPRDUP_REDUCE this Expr */
#define EP_Unlikely  0x040000 /* unlikely() or likelihood() function */
#define EP_ConstFunc 0x080000 /* A SQLITE_FUNC_CONSTANT or _SLOCHNG function */
#define EP_CanBeNull 0x100000 /* Can be null despite NOT NULL constraint */
#define EP_Subquery  0x200000 /* Tree contains a TK_SELECT operator */
#define EP_Alias     0x400000 /* Is an alias for a result set column */

/*
** Combinations of two or more EP_* flags
*/
#define EP_Propagate (EP_Collate|EP_Subquery) /* Propagate these bits up tree */

/*
** These macros can be used to test, set, or clear bits in the
** Expr.flags field.
*/
#define ExprHasProperty(E,P)     (((E)->flags&(P))!=0)
#define ExprHasAllProperty(E,P)  (((E)->flags&(P))==(P))
#define ExprSetProperty(E,P)     (E)->flags|=(P)
#define ExprClearProperty(E,P)   (E)->flags&=~(P)

/* The ExprSetVVAProperty() macro is used for Verification, Validation,
** and Accreditation only.  It works like ExprSetProperty() during VVA
** processes but is a no-op for delivery.
*/
#ifdef SQLITE_DEBUG
# define ExprSetVVAProperty(E,P)  (E)->flags|=(P)
#else
# define ExprSetVVAProperty(E,P)
#endif

/*
** Macros to determine the number of bytes required by a normal Expr
** struct, an Expr struct with the EP_Reduced flag set in Expr.flags
** and an Expr struct with the EP_TokenOnly flag set.
*/
#define EXPR_FULLSIZE           sizeof(Expr)           /* Full size */
#define EXPR_REDUCEDSIZE        offsetof(Expr,iTable)  /* Common features */
#define EXPR_TOKENONLYSIZE      offsetof(Expr,pLeft)   /* Fewer features */

/*
** Flags passed to the sqlite3ExprDup() function. See the header comment
** above sqlite3ExprDup() for details.
*/
#define EXPRDUP_REDUCE         0x0001  /* Used reduced-size Expr nodes */

/*
** A list of expressions.  Each expression may optionally have a
** name.  An expr/name combination can be used in several ways, such
** as the list of "expr AS ID" fields following a "SELECT" or in the
** list of "ID = expr" items in an UPDATE.  A list of expressions can
** also be used as the argument to a function, in which case the a.zName
** field is not used.
**
** By default the Expr.zSpan field holds a human-readable description of
** the expression that is used in the generation of error messages and
** column labels.  In this case, Expr.zSpan is typically the text of a
** column expression as it exists in a SELECT statement.  However, if
** the bSpanIsTab flag is set, then zSpan is overloaded to mean the name
** of the result column in the form: DATABASE.TABLE.COLUMN.  This later
** form is used for name resolution with nested FROM clauses.
*/
struct ExprList {
  int nExpr;             /* Number of expressions on the list */
  struct ExprList_item { /* For each expression in the list */
    Expr *pExpr;            /* The list of expressions */
    char *zName;            /* Token associated with this expression */
    char *zSpan;            /* Original text of the expression */
    u8 sortOrder;           /* 1 for DESC or 0 for ASC */
    unsigned done :1;       /* A flag to indicate when processing is finished */
    unsigned bSpanIsTab :1; /* zSpan holds DB.TABLE.COLUMN */
    unsigned reusable :1;   /* Constant expression is reusable */
    union {
      struct {
        u16 iOrderByCol;      /* For ORDER BY, column number in result set */
        u16 iAlias;           /* Index into Parse.aAlias[] for zName */
      } x;
      int iConstExprReg;      /* Register in which Expr value is cached */
    } u;
  } *a;                  /* Alloc a power of two greater or equal to nExpr */
};

/*
** An instance of this structure is used by the parser to record both
** the parse tree for an expression and the span of input text for an
** expression.
*/
struct ExprSpan {
  Expr *pExpr;          /* The expression parse tree */
  const char *zStart;   /* First character of input text */
  const char *zEnd;     /* One character past the end of input text */
};

/*
** An instance of this structure can hold a simple list of identifiers,
** such as the list "a,b,c" in the following statements:
**
**      INSERT INTO t(a,b,c) VALUES ...;
**      CREATE INDEX idx ON t(a,b,c);
**      CREATE TRIGGER trig BEFORE UPDATE ON t(a,b,c) ...;
**
** The IdList.a.idx field is used when the IdList represents the list of
** column names after a table name in an INSERT statement.  In the statement
**
**     INSERT INTO t(a,b,c) ...
**
** If "a" is the k-th column of table "t", then IdList.a[0].idx==k.
*/
struct IdList {
  struct IdList_item {
    char *zName;      /* Name of the identifier */
    int idx;          /* Index in some Table.aCol[] of a column named zName */
  } *a;
  int nId;         /* Number of identifiers on the list */
};

/*
** The bitmask datatype defined below is used for various optimizations.
**
** Changing this from a 64-bit to a 32-bit type limits the number of
** tables in a join to 32 instead of 64.  But it also reduces the size
** of the library by 738 bytes on ix86.
*/
#ifdef SQLITE_BITMASK_TYPE
  typedef SQLITE_BITMASK_TYPE Bitmask;
#else
  typedef u64 Bitmask;
#endif

/*
** The number of bits in a Bitmask.  "BMS" means "BitMask Size".
*/
#define BMS  ((int)(sizeof(Bitmask)*8))

/*
** A bit in a Bitmask
*/
#define MASKBIT(n)   (((Bitmask)1)<<(n))
#define MASKBIT32(n) (((unsigned int)1)<<(n))
#define ALLBITS      ((Bitmask)-1)

/*
** The following structure describes the FROM clause of a SELECT statement.
** Each table or subquery in the FROM clause is a separate element of
** the SrcList.a[] array.
**
** With the addition of multiple database support, the following structure
** can also be used to describe a particular table such as the table that
** is modified by an INSERT, DELETE, or UPDATE statement.  In standard SQL,
** such a table must be a simple name: ID.  But in SQLite, the table can
** now be identified by a database name, a dot, then the table name: ID.ID.
**
** The jointype starts out showing the join type between the current table
** and the next table on the list.  The parser builds the list this way.
** But sqlite3SrcListShiftJoinType() later shifts the jointypes so that each
** jointype expresses the join between the table and the previous table.
**
** In the colUsed field, the high-order bit (bit 63) is set if the table
** contains more than 63 columns and the 64-th or later column is used.
*/
struct SrcList {
  int nSrc;        /* Number of tables or subqueries in the FROM clause */
  u32 nAlloc;      /* Number of entries allocated in a[] below */
  struct SrcList_item {
    Schema *pSchema;  /* Schema to which this item is fixed */
    char *zDatabase;  /* Name of database holding this table */
    char *zName;      /* Name of the table */
    char *zAlias;     /* The "B" part of a "A AS B" phrase.  zName is the "A" */
    Table *pTab;      /* An SQL table corresponding to zName */
    Select *pSelect;  /* A SELECT statement used in place of a table name */
    int addrFillSub;  /* Address of subroutine to manifest a subquery */
    int regReturn;    /* Register holding return address of addrFillSub */
    int regResult;    /* Registers holding results of a co-routine */
    struct {
      u8 jointype;      /* Type of join between this able and the previous */
      unsigned notIndexed :1;    /* True if there is a NOT INDEXED clause */
      unsigned isIndexedBy :1;   /* True if there is an INDEXED BY clause */
      unsigned isTabFunc :1;     /* True if table-valued-function syntax */
      unsigned isCorrelated :1;  /* True if sub-query is correlated */
      unsigned viaCoroutine :1;  /* Implemented as a co-routine */
      unsigned isRecursive :1;   /* True for recursive reference in WITH */
    } fg;
#ifndef SQLITE_OMIT_EXPLAIN
    u8 iSelectId;     /* If pSelect!=0, the id of the sub-select in EQP */
#endif
    int iCursor;      /* The VDBE cursor number used to access this table */
    Expr *pOn;        /* The ON clause of a join */
    IdList *pUsing;   /* The USING clause of a join */
    Bitmask colUsed;  /* Bit N (1<<N) set if column N of pTab is used */
    union {
      char *zIndexedBy;    /* Identifier from "INDEXED BY <zIndex>" clause */
      ExprList *pFuncArg;  /* Arguments to table-valued-function */
    } u1;
    Index *pIBIndex;  /* Index structure corresponding to u1.zIndexedBy */
  } a[1];             /* One entry for each identifier on the list */
};

/*
** Permitted values of the SrcList.a.jointype field
*/
#define JT_INNER     0x0001    /* Any kind of inner or cross join */
#define JT_CROSS     0x0002    /* Explicit use of the CROSS keyword */
#define JT_NATURAL   0x0004    /* True for a "natural" join */
#define JT_LEFT      0x0008    /* Left outer join */
#define JT_RIGHT     0x0010    /* Right outer join */
#define JT_OUTER     0x0020    /* The "OUTER" keyword is present */
#define JT_ERROR     0x0040    /* unknown or unsupported join type */


/*
** Flags appropriate for the wctrlFlags parameter of sqlite3WhereBegin()
** and the WhereInfo.wctrlFlags member.
**
** Value constraints (enforced via assert()):
**     WHERE_USE_LIMIT  == SF_FixedLimit
*/
#define WHERE_ORDERBY_NORMAL   0x0000 /* No-op */
#define WHERE_ORDERBY_MIN      0x0001 /* ORDER BY processing for min() func */
#define WHERE_ORDERBY_MAX      0x0002 /* ORDER BY processing for max() func */
#define WHERE_ONEPASS_DESIRED  0x0004 /* Want to do one-pass UPDATE/DELETE */
#define WHERE_DUPLICATES_OK    0x0008 /* Ok to return a row more than once */
#define WHERE_OMIT_OPEN_CLOSE  0x0010 /* Table cursors are already open */
#define WHERE_FORCE_TABLE      0x0020 /* Do not use an index-only search */
#define WHERE_ONETABLE_ONLY    0x0040 /* Only code the 1st table in pTabList */
#define WHERE_NO_AUTOINDEX     0x0080 /* Disallow automatic indexes */
#define WHERE_GROUPBY          0x0100 /* pOrderBy is really a GROUP BY */
#define WHERE_DISTINCTBY       0x0200 /* pOrderby is really a DISTINCT clause */
#define WHERE_WANT_DISTINCT    0x0400 /* All output needs to be distinct */
#define WHERE_SORTBYGROUP      0x0800 /* Support sqlite3WhereIsSorted() */
#define WHERE_REOPEN_IDX       0x1000 /* Try to use OP_ReopenIdx */
#define WHERE_ONEPASS_MULTIROW 0x2000 /* ONEPASS is ok with multiple rows */
#define WHERE_USE_LIMIT        0x4000 /* There is a constant LIMIT clause */
#define WHERE_SEEK_TABLE       0x8000 /* Do not defer seeks on main table */

/* Allowed return values from sqlite3WhereIsDistinct()
*/
#define WHERE_DISTINCT_NOOP      0  /* DISTINCT keyword not used */
#define WHERE_DISTINCT_UNIQUE    1  /* No duplicates */
#define WHERE_DISTINCT_ORDERED   2  /* All duplicates are adjacent */
#define WHERE_DISTINCT_UNORDERED 3  /* Duplicates are scattered */

/*
** A NameContext defines a context in which to resolve table and column
** names.  The context consists of a list of tables (the pSrcList) field and
** a list of named expression (pEList).  The named expression list may
** be NULL.  The pSrc corresponds to the FROM clause of a SELECT or
** to the table being operated on by INSERT, UPDATE, or DELETE.  The
** pEList corresponds to the result set of a SELECT and is NULL for
** other statements.
**
** NameContexts can be nested.  When resolving names, the inner-most
** context is searched first.  If no match is found, the next outer
** context is checked.  If there is still no match, the next context
** is checked.  This process continues until either a match is found
** or all contexts are check.  When a match is found, the nRef member of
** the context containing the match is incremented.
**
** Each subquery gets a new NameContext.  The pNext field points to the
** NameContext in the parent query.  Thus the process of scanning the
** NameContext list corresponds to searching through successively outer
** subqueries looking for a match.
*/
struct NameContext {
  Parse *pParse;       /* The parser */
  SrcList *pSrcList;   /* One or more tables used to resolve names */
  ExprList *pEList;    /* Optional list of result-set columns */
  AggInfo *pAggInfo;   /* Information about aggregates at this level */
  NameContext *pNext;  /* Next outer name context.  NULL for outermost */
  int nRef;            /* Number of names resolved by this context */
  int nErr;            /* Number of errors encountered while resolving names */
  u16 ncFlags;         /* Zero or more NC_* flags defined below */
};

/*
** Allowed values for the NameContext, ncFlags field.
**
** Value constraints (all checked via assert()):
**    NC_HasAgg    == SF_HasAgg
**    NC_MinMaxAgg == SF_MinMaxAgg == SQLITE_FUNC_MINMAX
**
*/
#define NC_AllowAgg  0x0001  /* Aggregate functions are allowed here */
#define NC_PartIdx   0x0002  /* True if resolving a partial index WHERE */
#define NC_IsCheck   0x0004  /* True if resolving names in a CHECK constraint */
#define NC_InAggFunc 0x0008  /* True if analyzing arguments to an agg func */
#define NC_HasAgg    0x0010  /* One or more aggregate functions seen */
#define NC_IdxExpr   0x0020  /* True if resolving columns of CREATE INDEX */
#define NC_VarSelect 0x0040  /* A correlated subquery has been seen */
#define NC_MinMaxAgg 0x1000  /* min/max aggregates seen.  See note above */

/*
** An instance of the following structure contains all information
** needed to generate code for a single SELECT statement.
**
** nLimit is set to -1 if there is no LIMIT clause.  nOffset is set to 0.
** If there is a LIMIT clause, the parser sets nLimit to the value of the
** limit and nOffset to the value of the offset (or 0 if there is not
** offset).  But later on, nLimit and nOffset become the memory locations
** in the VDBE that record the limit and offset counters.
**
** addrOpenEphm[] entries contain the address of OP_OpenEphemeral opcodes.
** These addresses must be stored so that we can go back and fill in
** the P4_KEYINFO and P2 parameters later.  Neither the KeyInfo nor
** the number of columns in P2 can be computed at the same time
** as the OP_OpenEphm instruction is coded because not
** enough information about the compound query is known at that point.
** The KeyInfo for addrOpenTran[0] and [1] contains collating sequences
** for the result set.  The KeyInfo for addrOpenEphm[2] contains collating
** sequences for the ORDER BY clause.
*/
struct Select {
  ExprList *pEList;      /* The fields of the result */
  u8 op;                 /* One of: TK_UNION TK_ALL TK_INTERSECT TK_EXCEPT */
  LogEst nSelectRow;     /* Estimated number of result rows */
  u32 selFlags;          /* Various SF_* values */
  int iLimit, iOffset;   /* Memory registers holding LIMIT & OFFSET counters */
#if SELECTTRACE_ENABLED
  char zSelName[12];     /* Symbolic name of this SELECT use for debugging */
#endif
  int addrOpenEphm[2];   /* OP_OpenEphem opcodes related to this select */
  SrcList *pSrc;         /* The FROM clause */
  Expr *pWhere;          /* The WHERE clause */
  ExprList *pGroupBy;    /* The GROUP BY clause */
  Expr *pHaving;         /* The HAVING clause */
  ExprList *pOrderBy;    /* The ORDER BY clause */
  Select *pPrior;        /* Prior select in a compound select statement */
  Select *pNext;         /* Next select to the left in a compound */
  Expr *pLimit;          /* LIMIT expression. NULL means not used. */
  Expr *pOffset;         /* OFFSET expression. NULL means not used. */
  With *pWith;           /* WITH clause attached to this select. Or NULL. */
};

/*
** Allowed values for Select.selFlags.  The "SF" prefix stands for
** "Select Flag".
**
** Value constraints (all checked via assert())
**     SF_HasAgg     == NC_HasAgg
**     SF_MinMaxAgg  == NC_MinMaxAgg     == SQLITE_FUNC_MINMAX
**     SF_FixedLimit == WHERE_USE_LIMIT
*/
#define SF_Distinct       0x00001  /* Output should be DISTINCT */
#define SF_All            0x00002  /* Includes the ALL keyword */
#define SF_Resolved       0x00004  /* Identifiers have been resolved */
#define SF_Aggregate      0x00008  /* Contains agg functions or a GROUP BY */
#define SF_HasAgg         0x00010  /* Contains aggregate functions */
#define SF_UsesEphemeral  0x00020  /* Uses the OpenEphemeral opcode */
#define SF_Expanded       0x00040  /* sqlite3SelectExpand() called on this */
#define SF_HasTypeInfo    0x00080  /* FROM subqueries have Table metadata */
#define SF_Compound       0x00100  /* Part of a compound query */
#define SF_Values         0x00200  /* Synthesized from VALUES clause */
#define SF_MultiValue     0x00400  /* Single VALUES term with multiple rows */
#define SF_NestedFrom     0x00800  /* Part of a parenthesized FROM clause */
#define SF_MinMaxAgg      0x01000  /* Aggregate containing min() or max() */
#define SF_Recursive      0x02000  /* The recursive part of a recursive CTE */
#define SF_FixedLimit     0x04000  /* nSelectRow set by a constant LIMIT */
#define SF_MaybeConvert   0x08000  /* Need convertCompoundSelectToSubquery() */
#define SF_Converted      0x10000  /* By convertCompoundSelectToSubquery() */
#define SF_IncludeHidden  0x20000  /* Include hidden columns in output */


/*
** The results of a SELECT can be distributed in several ways, as defined
** by one of the following macros.  The "SRT" prefix means "SELECT Result
** Type".
**
**     SRT_Union       Store results as a key in a temporary index
**                     identified by pDest->iSDParm.
**
**     SRT_Except      Remove results from the temporary index pDest->iSDParm.
**
**     SRT_Exists      Store a 1 in memory cell pDest->iSDParm if the result
**                     set is not empty.
**
**     SRT_Discard     Throw the results away.  This is used by SELECT
**                     statements within triggers whose only purpose is
**                     the side-effects of functions.
**
** All of the above are free to ignore their ORDER BY clause. Those that
** follow must honor the ORDER BY clause.
**
**     SRT_Output      Generate a row of output (using the OP_ResultRow
**                     opcode) for each row in the result set.
**
**     SRT_Mem         Only valid if the result is a single column.
**                     Store the first column of the first result row
**                     in register pDest->iSDParm then abandon the rest
**                     of the query.  This destination implies "LIMIT 1".
**
**     SRT_Set         The result must be a single column.  Store each
**                     row of result as the key in table pDest->iSDParm.
**                     Apply the affinity pDest->affSdst before storing
**                     results.  Used to implement "IN (SELECT ...)".
**
**     SRT_EphemTab    Create an temporary table pDest->iSDParm and store
**                     the result there. The cursor is left open after
**                     returning.  This is like SRT_Table except that
**                     this destination uses OP_OpenEphemeral to create
**                     the table first.
**
**     SRT_Coroutine   Generate a co-routine that returns a new row of
**                     results each time it is invoked.  The entry point
**                     of the co-routine is stored in register pDest->iSDParm
**                     and the result row is stored in pDest->nDest registers
**                     starting with pDest->iSdst.
**
**     SRT_Table       Store results in temporary table pDest->iSDParm.
**     SRT_Fifo        This is like SRT_EphemTab except that the table
**                     is assumed to already be open.  SRT_Fifo has
**                     the additional property of being able to ignore
**                     the ORDER BY clause.
**
**     SRT_DistFifo    Store results in a temporary table pDest->iSDParm.
**                     But also use temporary table pDest->iSDParm+1 as
**                     a record of all prior results and ignore any duplicate
**                     rows.  Name means:  "Distinct Fifo".
**
**     SRT_Queue       Store results in priority queue pDest->iSDParm (really
**                     an index).  Append a sequence number so that all entries
**                     are distinct.
**
**     SRT_DistQueue   Store results in priority queue pDest->iSDParm only if
**                     the same record has never been stored before.  The
**                     index at pDest->iSDParm+1 hold all prior stores.
*/
#define SRT_Union        1  /* Store result as keys in an index */
#define SRT_Except       2  /* Remove result from a UNION index */
#define SRT_Exists       3  /* Store 1 if the result is not empty */
#define SRT_Discard      4  /* Do not save the results anywhere */
#define SRT_Fifo         5  /* Store result as data with an automatic rowid */
#define SRT_DistFifo     6  /* Like SRT_Fifo, but unique results only */
#define SRT_Queue        7  /* Store result in an queue */
#define SRT_DistQueue    8  /* Like SRT_Queue, but unique results only */

/* The ORDER BY clause is ignored for all of the above */
#define IgnorableOrderby(X) ((X->eDest)<=SRT_DistQueue)

#define SRT_Output       9  /* Output each row of result */
#define SRT_Mem         10  /* Store result in a memory cell */
#define SRT_Set         11  /* Store results as keys in an index */
#define SRT_EphemTab    12  /* Create transient tab and store like SRT_Table */
#define SRT_Coroutine   13  /* Generate a single row of result */
#define SRT_Table       14  /* Store result as data with an automatic rowid */

/*
** An instance of this object describes where to put of the results of
** a SELECT statement.
*/
struct SelectDest {
  u8 eDest;            /* How to dispose of the results.  On of SRT_* above. */
  char affSdst;        /* Affinity used when eDest==SRT_Set */
  int iSDParm;         /* A parameter used by the eDest disposal method */
  int iSdst;           /* Base register where results are written */
  int nSdst;           /* Number of registers allocated */
  ExprList *pOrderBy;  /* Key columns for SRT_Queue and SRT_DistQueue */
};

/*
** During code generation of statements that do inserts into AUTOINCREMENT
** tables, the following information is attached to the Table.u.autoInc.p
** pointer of each autoincrement table to record some side information that
** the code generator needs.  We have to keep per-table autoincrement
** information in case inserts are done within triggers.  Triggers do not
** normally coordinate their activities, but we do need to coordinate the
** loading and saving of autoincrement information.
*/
struct AutoincInfo {
  AutoincInfo *pNext;   /* Next info block in a list of them all */
  Table *pTab;          /* Table this info block refers to */
  int iDb;              /* Index in sqlite3.aDb[] of database holding pTab */
  int regCtr;           /* Memory register holding the rowid counter */
};

/*
** Size of the column cache
*/
#ifndef SQLITE_N_COLCACHE
# define SQLITE_N_COLCACHE 10
#endif

/*
** At least one instance of the following structure is created for each
** trigger that may be fired while parsing an INSERT, UPDATE or DELETE
** statement. All such objects are stored in the linked list headed at
** Parse.pTriggerPrg and deleted once statement compilation has been
** completed.
**
** A Vdbe sub-program that implements the body and WHEN clause of trigger
** TriggerPrg.pTrigger, assuming a default ON CONFLICT clause of
** TriggerPrg.orconf, is stored in the TriggerPrg.pProgram variable.
** The Parse.pTriggerPrg list never contains two entries with the same
** values for both pTrigger and orconf.
**
** The TriggerPrg.aColmask[0] variable is set to a mask of old.* columns
** accessed (or set to 0 for triggers fired as a result of INSERT
** statements). Similarly, the TriggerPrg.aColmask[1] variable is set to
** a mask of new.* columns used by the program.
*/
struct TriggerPrg {
  Trigger *pTrigger;      /* Trigger this program was coded from */
  TriggerPrg *pNext;      /* Next entry in Parse.pTriggerPrg list */
  SubProgram *pProgram;   /* Program implementing pTrigger/orconf */
  int orconf;             /* Default ON CONFLICT policy */
  u32 aColmask[2];        /* Masks of old.*, new.* columns accessed */
};

/*
** The yDbMask datatype for the bitmask of all attached databases.
*/
#if SQLITE_MAX_ATTACHED>30
  typedef unsigned char yDbMask[(SQLITE_MAX_ATTACHED+9)/8];
# define DbMaskTest(M,I)    (((M)[(I)/8]&(1<<((I)&7)))!=0)
# define DbMaskZero(M)      memset((M),0,sizeof(M))
# define DbMaskSet(M,I)     (M)[(I)/8]|=(1<<((I)&7))
# define DbMaskAllZero(M)   sqlite3DbMaskAllZero(M)
# define DbMaskNonZero(M)   (sqlite3DbMaskAllZero(M)==0)
#else
  typedef unsigned int yDbMask;
# define DbMaskTest(M,I)    (((M)&(((yDbMask)1)<<(I)))!=0)
# define DbMaskZero(M)      (M)=0
# define DbMaskSet(M,I)     (M)|=(((yDbMask)1)<<(I))
# define DbMaskAllZero(M)   (M)==0
# define DbMaskNonZero(M)   (M)!=0
#endif

/*
** An SQL parser context.  A copy of this structure is passed through
** the parser and down into all the parser action routine in order to
** carry around information that is global to the entire parse.
**
** The structure is divided into two parts.  When the parser and code
** generate call themselves recursively, the first part of the structure
** is constant but the second part is reset at the beginning and end of
** each recursion.
**
** The nTableLock and aTableLock variables are only used if the shared-cache
** feature is enabled (if sqlite3Tsd()->useSharedData is true). They are
** used to store the set of table-locks required by the statement being
** compiled. Function sqlite3TableLock() is used to add entries to the
** list.
*/
struct Parse {
  sqlite3 *db;         /* The main database structure */
  char *zErrMsg;       /* An error message */
  Vdbe *pVdbe;         /* An engine for executing database bytecode */
  int rc;              /* Return code from execution */
  u8 colNamesSet;      /* TRUE after OP_ColumnName has been issued to pVdbe */
  u8 checkSchema;      /* Causes schema cookie check after an error */
  u8 nested;           /* Number of nested calls to the parser/code generator */
  u8 nTempReg;         /* Number of temporary registers in aTempReg[] */
  u8 isMultiWrite;     /* True if statement may modify/insert multiple rows */
  u8 mayAbort;         /* True if statement may throw an ABORT exception */
  u8 hasCompound;      /* Need to invoke convertCompoundSelectToSubquery() */
  u8 okConstFactor;    /* OK to factor out constants */
  u8 disableLookaside; /* Number of times lookaside has been disabled */
  u8 nColCache;        /* Number of entries in aColCache[] */
  int aTempReg[8];     /* Holding area for temporary registers */
  int nRangeReg;       /* Size of the temporary register block */
  int iRangeReg;       /* First register in temporary register block */
  int nErr;            /* Number of errors seen */
  int nTab;            /* Number of previously allocated VDBE cursors */
  int nMem;            /* Number of memory cells used so far */
  int nSet;            /* Number of sets used so far */
  int nOnce;           /* Number of OP_Once instructions so far */
  int nOpAlloc;        /* Number of slots allocated for Vdbe.aOp[] */
  int szOpAlloc;       /* Bytes of memory space allocated for Vdbe.aOp[] */
  int iFixedOp;        /* Never back out opcodes iFixedOp-1 or earlier */
  int ckBase;          /* Base register of data during check constraints */
  int iSelfTab;        /* Table of an index whose exprs are being coded */
  int iCacheLevel;     /* ColCache valid when aColCache[].iLevel<=iCacheLevel */
  int iCacheCnt;       /* Counter used to generate aColCache[].lru values */
  int nLabel;          /* Number of labels used */
  int *aLabel;         /* Space to hold the labels */
  struct yColCache {
    int iTable;           /* Table cursor number */
    i16 iColumn;          /* Table column number */
    u8 tempReg;           /* iReg is a temp register that needs to be freed */
    int iLevel;           /* Nesting level */
    int iReg;             /* Reg with value of this column. 0 means none. */
    int lru;              /* Least recently used entry has the smallest value */
  } aColCache[SQLITE_N_COLCACHE];  /* One for each column cache entry */
  ExprList *pConstExpr;/* Constant expressions */
  Token constraintName;/* Name of the constraint currently being parsed */
  yDbMask writeMask;   /* Start a write transaction on these databases */
  yDbMask cookieMask;  /* Bitmask of schema verified databases */
  int cookieValue[SQLITE_MAX_ATTACHED+2];  /* Values of cookies to verify */
  int regRowid;        /* Register holding rowid of CREATE TABLE entry */
  int regRoot;         /* Register holding root page number for new objects */
  int nMaxArg;         /* Max args passed to user function by sub-program */
#if SELECTTRACE_ENABLED
  int nSelect;         /* Number of SELECT statements seen */
  int nSelectIndent;   /* How far to indent SELECTTRACE() output */
#endif
#ifndef SQLITE_OMIT_SHARED_CACHE
  int nTableLock;        /* Number of locks in aTableLock */
  TableLock *aTableLock; /* Required table locks for shared-cache mode */
#endif
  AutoincInfo *pAinc;  /* Information about AUTOINCREMENT counters */

  /* Information used while coding trigger programs. */
  Parse *pToplevel;    /* Parse structure for main program (or NULL) */
  Table *pTriggerTab;  /* Table triggers are being coded for */
  int addrCrTab;       /* Address of OP_CreateTable opcode on CREATE TABLE */
  u32 nQueryLoop;      /* Est number of iterations of a query (10*log2(N)) */
  u32 oldmask;         /* Mask of old.* columns referenced */
  u32 newmask;         /* Mask of new.* columns referenced */
  u8 eTriggerOp;       /* TK_UPDATE, TK_INSERT or TK_DELETE */
  u8 eOrconf;          /* Default ON CONFLICT policy for trigger steps */
  u8 disableTriggers;  /* True to disable triggers */

  /************************************************************************
  ** Above is constant between recursions.  Below is reset before and after
  ** each recursion.  The boundary between these two regions is determined
  ** using offsetof(Parse,nVar) so the nVar field must be the first field
  ** in the recursive region.
  ************************************************************************/

  ynVar nVar;               /* Number of '?' variables seen in the SQL so far */
  int nzVar;                /* Number of available slots in azVar[] */
  u8 iPkSortOrder;          /* ASC or DESC for INTEGER PRIMARY KEY */
  u8 explain;               /* True if the EXPLAIN flag is found on the query */
#ifndef SQLITE_OMIT_VIRTUALTABLE
  u8 declareVtab;           /* True if inside sqlite3_declare_vtab() */
  int nVtabLock;            /* Number of virtual tables to lock */
#endif
  int nAlias;               /* Number of aliased result set columns */
  int nHeight;              /* Expression tree height of current sub-select */
#ifndef SQLITE_OMIT_EXPLAIN
  int iSelectId;            /* ID of current select for EXPLAIN output */
  int iNextSelectId;        /* Next available select ID for EXPLAIN output */
#endif
  char **azVar;             /* Pointers to names of parameters */
  Vdbe *pReprepare;         /* VM being reprepared (sqlite3Reprepare()) */
  const char *zTail;        /* All SQL text past the last semicolon parsed */
  Table *pNewTable;         /* A table being constructed by CREATE TABLE */
  Trigger *pNewTrigger;     /* Trigger under construct by a CREATE TRIGGER */
  const char *zAuthContext; /* The 6th parameter to db->xAuth callbacks */
  Token sNameToken;         /* Token with unqualified schema object name */
  Token sLastToken;         /* The last token parsed */
#ifndef SQLITE_OMIT_VIRTUALTABLE
  Token sArg;               /* Complete text of a module argument */
  Table **apVtabLock;       /* Pointer to virtual tables needing locking */
#endif
  Table *pZombieTab;        /* List of Table objects to delete after code gen */
  TriggerPrg *pTriggerPrg;  /* Linked list of coded triggers */
  With *pWith;              /* Current WITH clause, or NULL */
  With *pWithToFree;        /* Free this WITH object at the end of the parse */
};

/*
** Return true if currently inside an sqlite3_declare_vtab() call.
*/
#ifdef SQLITE_OMIT_VIRTUALTABLE
  #define IN_DECLARE_VTAB 0
#else
  #define IN_DECLARE_VTAB (pParse->declareVtab)
#endif

/*
** An instance of the following structure can be declared on a stack and used
** to save the Parse.zAuthContext value so that it can be restored later.
*/
struct AuthContext {
  const char *zAuthContext;   /* Put saved Parse.zAuthContext here */
  Parse *pParse;              /* The Parse structure */
};

/*
** Bitfield flags for P5 value in various opcodes.
**
** Value constraints (enforced via assert()):
**    OPFLAG_LENGTHARG    == SQLITE_FUNC_LENGTH
**    OPFLAG_TYPEOFARG    == SQLITE_FUNC_TYPEOF
**    OPFLAG_BULKCSR      == BTREE_BULKLOAD
**    OPFLAG_SEEKEQ       == BTREE_SEEK_EQ
**    OPFLAG_FORDELETE    == BTREE_FORDELETE
**    OPFLAG_SAVEPOSITION == BTREE_SAVEPOSITION
**    OPFLAG_AUXDELETE    == BTREE_AUXDELETE
*/
#define OPFLAG_NCHANGE       0x01    /* OP_Insert: Set to update db->nChange */
                                     /* Also used in P2 (not P5) of OP_Delete */
#define OPFLAG_EPHEM         0x01    /* OP_Column: Ephemeral output is ok */
#define OPFLAG_LASTROWID     0x02    /* Set to update db->lastRowid */
#define OPFLAG_ISUPDATE      0x04    /* This OP_Insert is an sql UPDATE */
#define OPFLAG_APPEND        0x08    /* This is likely to be an append */
#define OPFLAG_USESEEKRESULT 0x10    /* Try to avoid a seek in BtreeInsert() */
#ifdef SQLITE_ENABLE_PREUPDATE_HOOK
#define OPFLAG_ISNOOP        0x40    /* OP_Delete does pre-update-hook only */
#endif
#define OPFLAG_LENGTHARG     0x40    /* OP_Column only used for length() */
#define OPFLAG_TYPEOFARG     0x80    /* OP_Column only used for typeof() */
#define OPFLAG_BULKCSR       0x01    /* OP_Open** used to open bulk cursor */
#define OPFLAG_SEEKEQ        0x02    /* OP_Open** cursor uses EQ seek only */
#define OPFLAG_FORDELETE     0x08    /* OP_Open should use BTREE_FORDELETE */
#define OPFLAG_P2ISREG       0x10    /* P2 to OP_Open** is a register number */
#define OPFLAG_PERMUTE       0x01    /* OP_Compare: use the permutation */
#define OPFLAG_SAVEPOSITION  0x02    /* OP_Delete: keep cursor position */
#define OPFLAG_AUXDELETE     0x04    /* OP_Delete: index in a DELETE op */

/*
 * Each trigger present in the database schema is stored as an instance of
 * struct Trigger.
 *
 * Pointers to instances of struct Trigger are stored in two ways.
 * 1. In the "trigHash" hash table (part of the sqlite3* that represents the
 *    database). This allows Trigger structures to be retrieved by name.
 * 2. All triggers associated with a single table form a linked list, using the
 *    pNext member of struct Trigger. A pointer to the first element of the
 *    linked list is stored as the "pTrigger" member of the associated
 *    struct Table.
 *
 * The "step_list" member points to the first element of a linked list
 * containing the SQL statements specified as the trigger program.
 */
struct Trigger {
  char *zName;            /* The name of the trigger                        */
  char *table;            /* The table or view to which the trigger applies */
  u8 op;                  /* One of TK_DELETE, TK_UPDATE, TK_INSERT         */
  u8 tr_tm;               /* One of TRIGGER_BEFORE, TRIGGER_AFTER */
  Expr *pWhen;            /* The WHEN clause of the expression (may be NULL) */
  IdList *pColumns;       /* If this is an UPDATE OF <column-list> trigger,
                             the <column-list> is stored here */
  Schema *pSchema;        /* Schema containing the trigger */
  Schema *pTabSchema;     /* Schema containing the table */
  TriggerStep *step_list; /* Link list of trigger program steps             */
  Trigger *pNext;         /* Next trigger associated with the table */
};

/*
** A trigger is either a BEFORE or an AFTER trigger.  The following constants
** determine which.
**
** If there are multiple triggers, you might of some BEFORE and some AFTER.
** In that cases, the constants below can be ORed together.
*/
#define TRIGGER_BEFORE  1
#define TRIGGER_AFTER   2

/*
 * An instance of struct TriggerStep is used to store a single SQL statement
 * that is a part of a trigger-program.
 *
 * Instances of struct TriggerStep are stored in a singly linked list (linked
 * using the "pNext" member) referenced by the "step_list" member of the
 * associated struct Trigger instance. The first element of the linked list is
 * the first step of the trigger-program.
 *
 * The "op" member indicates whether this is a "DELETE", "INSERT", "UPDATE" or
 * "SELECT" statement. The meanings of the other members is determined by the
 * value of "op" as follows:
 *
 * (op == TK_INSERT)
 * orconf    -> stores the ON CONFLICT algorithm
 * pSelect   -> If this is an INSERT INTO ... SELECT ... statement, then
 *              this stores a pointer to the SELECT statement. Otherwise NULL.
 * zTarget   -> Dequoted name of the table to insert into.
 * pExprList -> If this is an INSERT INTO ... VALUES ... statement, then
 *              this stores values to be inserted. Otherwise NULL.
 * pIdList   -> If this is an INSERT INTO ... (<column-names>) VALUES ...
 *              statement, then this stores the column-names to be
 *              inserted into.
 *
 * (op == TK_DELETE)
 * zTarget   -> Dequoted name of the table to delete from.
 * pWhere    -> The WHERE clause of the DELETE statement if one is specified.
 *              Otherwise NULL.
 *
 * (op == TK_UPDATE)
 * zTarget   -> Dequoted name of the table to update.
 * pWhere    -> The WHERE clause of the UPDATE statement if one is specified.
 *              Otherwise NULL.
 * pExprList -> A list of the columns to update and the expressions to update
 *              them to. See sqlite3Update() documentation of "pChanges"
 *              argument.
 *
 */
struct TriggerStep {
  u8 op;               /* One of TK_DELETE, TK_UPDATE, TK_INSERT, TK_SELECT */
  u8 orconf;           /* OE_Rollback etc. */
  Trigger *pTrig;      /* The trigger that this step is a part of */
  Select *pSelect;     /* SELECT statement or RHS of INSERT INTO SELECT ... */
  char *zTarget;       /* Target table for DELETE, UPDATE, INSERT */
  Expr *pWhere;        /* The WHERE clause for DELETE or UPDATE steps */
  ExprList *pExprList; /* SET clause for UPDATE. */
  IdList *pIdList;     /* Column names for INSERT */
  TriggerStep *pNext;  /* Next in the link-list */
  TriggerStep *pLast;  /* Last element in link-list. Valid for 1st elem only */
};

/*
** The following structure contains information used by the sqliteFix...
** routines as they walk the parse tree to make database references
** explicit.
*/
typedef struct DbFixer DbFixer;
struct DbFixer {
  Parse *pParse;      /* The parsing context.  Error messages written here */
  Schema *pSchema;    /* Fix items to this schema */
  int bVarOnly;       /* Check for variable references only */
  const char *zDb;    /* Make sure all objects are contained in this database */
  const char *zType;  /* Type of the container - used for error messages */
  const Token *pName; /* Name of the container - used for error messages */
};

/*
** An objected used to accumulate the text of a string where we
** do not necessarily know how big the string will be in the end.
*/
struct StrAccum {
  sqlite3 *db;         /* Optional database for lookaside.  Can be NULL */
  char *zBase;         /* A base allocation.  Not from malloc. */
  char *zText;         /* The string collected so far */
  u32  nChar;          /* Length of the string so far */
  u32  nAlloc;         /* Amount of space allocated in zText */
  u32  mxAlloc;        /* Maximum allowed allocation.  0 for no malloc usage */
  u8   accError;       /* STRACCUM_NOMEM or STRACCUM_TOOBIG */
  u8   printfFlags;    /* SQLITE_PRINTF flags below */
};
#define STRACCUM_NOMEM   1
#define STRACCUM_TOOBIG  2
#define SQLITE_PRINTF_INTERNAL 0x01  /* Internal-use-only converters allowed */
#define SQLITE_PRINTF_SQLFUNC  0x02  /* SQL function arguments to VXPrintf */
#define SQLITE_PRINTF_MALLOCED 0x04  /* True if xText is allocated space */

#define isMalloced(X)  (((X)->printfFlags & SQLITE_PRINTF_MALLOCED)!=0)


/*
** A pointer to this structure is used to communicate information
** from sqlite3Init and OP_ParseSchema into the sqlite3InitCallback.
*/
typedef struct {
  sqlite3 *db;        /* The database being initialized */
  char **pzErrMsg;    /* Error message stored here */
  int iDb;            /* 0 for main database.  1 for TEMP, 2.. for ATTACHed */
  int rc;             /* Result code stored here */
} InitData;

/*
** Structure containing global configuration data for the SQLite library.
**
** This structure also contains some state information.
*/
struct Sqlite3Config {
  int bMemstat;                     /* True to enable memory status */
  int bCoreMutex;                   /* True to enable core mutexing */
  int bFullMutex;                   /* True to enable full mutexing */
  int bOpenUri;                     /* True to interpret filenames as URIs */
  int bUseCis;                      /* Use covering indices for full-scans */
  int mxStrlen;                     /* Maximum string length */
  int neverCorrupt;                 /* Database is always well-formed */
  int szLookaside;                  /* Default lookaside buffer size */
  int nLookaside;                   /* Default lookaside buffer count */
  int nStmtSpill;                   /* Stmt-journal spill-to-disk threshold */
  sqlite3_mem_methods m;            /* Low-level memory allocation interface */
  sqlite3_mutex_methods mutex;      /* Low-level mutex interface */
  sqlite3_pcache_methods2 pcache2;  /* Low-level page-cache interface */
  void *pHeap;                      /* Heap storage space */
  int nHeap;                        /* Size of pHeap[] */
  int mnReq, mxReq;                 /* Min and max heap requests sizes */
  sqlite3_int64 szMmap;             /* mmap() space per open file */
  sqlite3_int64 mxMmap;             /* Maximum value for szMmap */
  void *pScratch;                   /* Scratch memory */
  int szScratch;                    /* Size of each scratch buffer */
  int nScratch;                     /* Number of scratch buffers */
  void *pPage;                      /* Page cache memory */
  int szPage;                       /* Size of each page in pPage[] */
  int nPage;                        /* Number of pages in pPage[] */
  int mxParserStack;                /* maximum depth of the parser stack */
  int sharedCacheEnabled;           /* true if shared-cache mode enabled */
  u32 szPma;                        /* Maximum Sorter PMA size */
  /* The above might be initialized to non-zero.  The following need to always
  ** initially be zero, however. */
  int isInit;                       /* True after initialization has finished */
  int inProgress;                   /* True while initialization in progress */
  int isMutexInit;                  /* True after mutexes are initialized */
  int isMallocInit;                 /* True after malloc is initialized */
  int isPCacheInit;                 /* True after malloc is initialized */
  int nRefInitMutex;                /* Number of users of pInitMutex */
  sqlite3_mutex *pInitMutex;        /* Mutex used by sqlite3_initialize() */
  void (*xLog)(void*,int,const char*); /* Function for logging */
  void *pLogArg;                       /* First argument to xLog() */
#ifdef SQLITE_ENABLE_SQLLOG
  void(*xSqllog)(void*,sqlite3*,const char*, int);
  void *pSqllogArg;
#endif
#ifdef SQLITE_VDBE_COVERAGE
  /* The following callback (if not NULL) is invoked on every VDBE branch
  ** operation.  Set the callback using SQLITE_TESTCTRL_VDBE_COVERAGE.
  */
  void (*xVdbeBranch)(void*,int iSrcLine,u8 eThis,u8 eMx);  /* Callback */
  void *pVdbeBranchArg;                                     /* 1st argument */
#endif
#ifndef SQLITE_OMIT_BUILTIN_TEST
  int (*xTestCallback)(int);        /* Invoked by sqlite3FaultSim() */
#endif
  int bLocaltimeFault;              /* True to fail localtime() calls */
};

/*
** This macro is used inside of assert() statements to indicate that
** the assert is only valid on a well-formed database.  Instead of:
**
**     assert( X );
**
** One writes:
**
**     assert( X || CORRUPT_DB );
**
** CORRUPT_DB is true during normal operation.  CORRUPT_DB does not indicate
** that the database is definitely corrupt, only that it might be corrupt.
** For most test cases, CORRUPT_DB is set to false using a special
** sqlite3_test_control().  This enables assert() statements to prove
** things that are always true for well-formed databases.
*/
#define CORRUPT_DB  (sqlite3Config.neverCorrupt==0)

/*
** Context pointer passed down through the tree-walk.
*/
struct Walker {
  Parse *pParse;                            /* Parser context.  */
  int (*xExprCallback)(Walker*, Expr*);     /* Callback for expressions */
  int (*xSelectCallback)(Walker*,Select*);  /* Callback for SELECTs */
  void (*xSelectCallback2)(Walker*,Select*);/* Second callback for SELECTs */
  int walkerDepth;                          /* Number of subqueries */
  u8 eCode;                                 /* A small processing code */
  union {                                   /* Extra data for callback */
    NameContext *pNC;                          /* Naming context */
    int n;                                     /* A counter */
    int iCur;                                  /* A cursor number */
    SrcList *pSrcList;                         /* FROM clause */
    struct SrcCount *pSrcCount;                /* Counting column references */
    struct CCurHint *pCCurHint;                /* Used by codeCursorHint() */
    int *aiCol;                                /* array of column indexes */
  } u;
};

/* Forward declarations */
SQLITE_PRIVATE int sqlite3WalkExpr(Walker*, Expr*);
SQLITE_PRIVATE int sqlite3WalkExprList(Walker*, ExprList*);
SQLITE_PRIVATE int sqlite3WalkSelect(Walker*, Select*);
SQLITE_PRIVATE int sqlite3WalkSelectExpr(Walker*, Select*);
SQLITE_PRIVATE int sqlite3WalkSelectFrom(Walker*, Select*);
SQLITE_PRIVATE int sqlite3ExprWalkNoop(Walker*, Expr*);

/*
** Return code from the parse-tree walking primitives and their
** callbacks.
*/
#define WRC_Continue    0   /* Continue down into children */
#define WRC_Prune       1   /* Omit children but continue walking siblings */
#define WRC_Abort       2   /* Abandon the tree walk */

/*
** An instance of this structure represents a set of one or more CTEs
** (common table expressions) created by a single WITH clause.
*/
struct With {
  int nCte;                       /* Number of CTEs in the WITH clause */
  With *pOuter;                   /* Containing WITH clause, or NULL */
  struct Cte {                    /* For each CTE in the WITH clause.... */
    char *zName;                    /* Name of this CTE */
    ExprList *pCols;                /* List of explicit column names, or NULL */
    Select *pSelect;                /* The definition of this CTE */
    const char *zCteErr;            /* Error message for circular references */
  } a[1];
};

#ifdef SQLITE_DEBUG
/*
** An instance of the TreeView object is used for printing the content of
** data structures on sqlite3DebugPrintf() using a tree-like view.
*/
struct TreeView {
  int iLevel;             /* Which level of the tree we are on */
  u8  bLine[100];         /* Draw vertical in column i if bLine[i] is true */
};
#endif /* SQLITE_DEBUG */

/*
** Assuming zIn points to the first byte of a UTF-8 character,
** advance zIn to point to the first byte of the next UTF-8 character.
*/
#define SQLITE_SKIP_UTF8(zIn) {                        \
  if( (*(zIn++))>=0xc0 ){                              \
    while( (*zIn & 0xc0)==0x80 ){ zIn++; }             \
  }                                                    \
}

/*
** The SQLITE_*_BKPT macros are substitutes for the error codes with
** the same name but without the _BKPT suffix.  These macros invoke
** routines that report the line-number on which the error originated
** using sqlite3_log().  The routines also provide a convenient place
** to set a debugger breakpoint.
*/
SQLITE_PRIVATE int sqlite3CorruptError(int);
SQLITE_PRIVATE int sqlite3MisuseError(int);
SQLITE_PRIVATE int sqlite3CantopenError(int);
#define SQLITE_CORRUPT_BKPT sqlite3CorruptError(__LINE__)
#define SQLITE_MISUSE_BKPT sqlite3MisuseError(__LINE__)
#define SQLITE_CANTOPEN_BKPT sqlite3CantopenError(__LINE__)
#ifdef SQLITE_DEBUG
SQLITE_PRIVATE   int sqlite3NomemError(int);
SQLITE_PRIVATE   int sqlite3IoerrnomemError(int);
# define SQLITE_NOMEM_BKPT sqlite3NomemError(__LINE__)
# define SQLITE_IOERR_NOMEM_BKPT sqlite3IoerrnomemError(__LINE__)
#else
# define SQLITE_NOMEM_BKPT SQLITE_NOMEM
# define SQLITE_IOERR_NOMEM_BKPT SQLITE_IOERR_NOMEM
#endif

/*
** FTS3 and FTS4 both require virtual table support
*/
#if defined(SQLITE_OMIT_VIRTUALTABLE)
# undef SQLITE_ENABLE_FTS3
# undef SQLITE_ENABLE_FTS4
#endif

/*
** FTS4 is really an extension for FTS3.  It is enabled using the
** SQLITE_ENABLE_FTS3 macro.  But to avoid confusion we also call
** the SQLITE_ENABLE_FTS4 macro to serve as an alias for SQLITE_ENABLE_FTS3.
*/
#if defined(SQLITE_ENABLE_FTS4) && !defined(SQLITE_ENABLE_FTS3)
# define SQLITE_ENABLE_FTS3 1
#endif

/*
** The ctype.h header is needed for non-ASCII systems.  It is also
** needed by FTS3 when FTS3 is included in the amalgamation.
*/
#if !defined(SQLITE_ASCII) || \
    (defined(SQLITE_ENABLE_FTS3) && defined(SQLITE_AMALGAMATION))
# include <ctype.h>
#endif

/*
** The following macros mimic the standard library functions toupper(),
** isspace(), isalnum(), isdigit() and isxdigit(), respectively. The
** sqlite versions only work for ASCII characters, regardless of locale.
*/
#ifdef SQLITE_ASCII
# define sqlite3Toupper(x)  ((x)&~(sqlite3CtypeMap[(unsigned char)(x)]&0x20))
# define sqlite3Isspace(x)   (sqlite3CtypeMap[(unsigned char)(x)]&0x01)
# define sqlite3Isalnum(x)   (sqlite3CtypeMap[(unsigned char)(x)]&0x06)
# define sqlite3Isalpha(x)   (sqlite3CtypeMap[(unsigned char)(x)]&0x02)
# define sqlite3Isdigit(x)   (sqlite3CtypeMap[(unsigned char)(x)]&0x04)
# define sqlite3Isxdigit(x)  (sqlite3CtypeMap[(unsigned char)(x)]&0x08)
# define sqlite3Tolower(x)   (sqlite3UpperToLower[(unsigned char)(x)])
# define sqlite3Isquote(x)   (sqlite3CtypeMap[(unsigned char)(x)]&0x80)
#else
# define sqlite3Toupper(x)   toupper((unsigned char)(x))
# define sqlite3Isspace(x)   isspace((unsigned char)(x))
# define sqlite3Isalnum(x)   isalnum((unsigned char)(x))
# define sqlite3Isalpha(x)   isalpha((unsigned char)(x))
# define sqlite3Isdigit(x)   isdigit((unsigned char)(x))
# define sqlite3Isxdigit(x)  isxdigit((unsigned char)(x))
# define sqlite3Tolower(x)   tolower((unsigned char)(x))
# define sqlite3Isquote(x)   ((x)=='"'||(x)=='\''||(x)=='['||(x)=='`')
#endif
#ifndef SQLITE_OMIT_COMPILEOPTION_DIAGS
SQLITE_PRIVATE int sqlite3IsIdChar(u8);
#endif

/*
** Internal function prototypes
*/
SQLITE_PRIVATE int sqlite3StrICmp(const char*,const char*);
SQLITE_PRIVATE int sqlite3Strlen30(const char*);
SQLITE_PRIVATE char *sqlite3ColumnType(Column*,char*);
#define sqlite3StrNICmp sqlite3_strnicmp

SQLITE_PRIVATE int sqlite3MallocInit(void);
SQLITE_PRIVATE void sqlite3MallocEnd(void);
SQLITE_PRIVATE void *sqlite3Malloc(u64);
SQLITE_PRIVATE void *sqlite3MallocZero(u64);
SQLITE_PRIVATE void *sqlite3DbMallocZero(sqlite3*, u64);
SQLITE_PRIVATE void *sqlite3DbMallocRaw(sqlite3*, u64);
SQLITE_PRIVATE void *sqlite3DbMallocRawNN(sqlite3*, u64);
SQLITE_PRIVATE char *sqlite3DbStrDup(sqlite3*,const char*);
SQLITE_PRIVATE char *sqlite3DbStrNDup(sqlite3*,const char*, u64);
SQLITE_PRIVATE void *sqlite3Realloc(void*, u64);
SQLITE_PRIVATE void *sqlite3DbReallocOrFree(sqlite3 *, void *, u64);
SQLITE_PRIVATE void *sqlite3DbRealloc(sqlite3 *, void *, u64);
SQLITE_PRIVATE void sqlite3DbFree(sqlite3*, void*);
SQLITE_PRIVATE int sqlite3MallocSize(void*);
SQLITE_PRIVATE int sqlite3DbMallocSize(sqlite3*, void*);
SQLITE_PRIVATE void *sqlite3ScratchMalloc(int);
SQLITE_PRIVATE void sqlite3ScratchFree(void*);
SQLITE_PRIVATE void *sqlite3PageMalloc(int);
SQLITE_PRIVATE void sqlite3PageFree(void*);
SQLITE_PRIVATE void sqlite3MemSetDefault(void);
#ifndef SQLITE_OMIT_BUILTIN_TEST
SQLITE_PRIVATE void sqlite3BenignMallocHooks(void (*)(void), void (*)(void));
#endif
SQLITE_PRIVATE int sqlite3HeapNearlyFull(void);

/*
** On systems with ample stack space and that support alloca(), make
** use of alloca() to obtain space for large automatic objects.  By default,
** obtain space from malloc().
**
** The alloca() routine never returns NULL.  This will cause code paths
** that deal with sqlite3StackAlloc() failures to be unreachable.
*/
#ifdef SQLITE_USE_ALLOCA
# define sqlite3StackAllocRaw(D,N)   alloca(N)
# define sqlite3StackAllocZero(D,N)  memset(alloca(N), 0, N)
# define sqlite3StackFree(D,P)
#else
# define sqlite3StackAllocRaw(D,N)   sqlite3DbMallocRaw(D,N)
# define sqlite3StackAllocZero(D,N)  sqlite3DbMallocZero(D,N)
# define sqlite3StackFree(D,P)       sqlite3DbFree(D,P)
#endif

#ifdef SQLITE_ENABLE_MEMSYS3
SQLITE_PRIVATE const sqlite3_mem_methods *sqlite3MemGetMemsys3(void);
#endif
#ifdef SQLITE_ENABLE_MEMSYS5
SQLITE_PRIVATE const sqlite3_mem_methods *sqlite3MemGetMemsys5(void);
#endif


#ifndef SQLITE_MUTEX_OMIT
SQLITE_PRIVATE   sqlite3_mutex_methods const *sqlite3DefaultMutex(void);
SQLITE_PRIVATE   sqlite3_mutex_methods const *sqlite3NoopMutex(void);
SQLITE_PRIVATE   sqlite3_mutex *sqlite3MutexAlloc(int);
SQLITE_PRIVATE   int sqlite3MutexInit(void);
SQLITE_PRIVATE   int sqlite3MutexEnd(void);
#endif
#if !defined(SQLITE_MUTEX_OMIT) && !defined(SQLITE_MUTEX_NOOP)
SQLITE_PRIVATE   void sqlite3MemoryBarrier(void);
#else
# define sqlite3MemoryBarrier()
#endif

SQLITE_PRIVATE sqlite3_int64 sqlite3StatusValue(int);
SQLITE_PRIVATE void sqlite3StatusUp(int, int);
SQLITE_PRIVATE void sqlite3StatusDown(int, int);
SQLITE_PRIVATE void sqlite3StatusHighwater(int, int);

/* Access to mutexes used by sqlite3_status() */
SQLITE_PRIVATE sqlite3_mutex *sqlite3Pcache1Mutex(void);
SQLITE_PRIVATE sqlite3_mutex *sqlite3MallocMutex(void);

#ifndef SQLITE_OMIT_FLOATING_POINT
SQLITE_PRIVATE   int sqlite3IsNaN(double);
#else
# define sqlite3IsNaN(X)  0
#endif

/*
** An instance of the following structure holds information about SQL
** functions arguments that are the parameters to the printf() function.
*/
struct PrintfArguments {
  int nArg;                /* Total number of arguments */
  int nUsed;               /* Number of arguments used so far */
  sqlite3_value **apArg;   /* The argument values */
};

SQLITE_PRIVATE void sqlite3VXPrintf(StrAccum*, const char*, va_list);
SQLITE_PRIVATE void sqlite3XPrintf(StrAccum*, const char*, ...);
SQLITE_PRIVATE char *sqlite3MPrintf(sqlite3*,const char*, ...);
SQLITE_PRIVATE char *sqlite3VMPrintf(sqlite3*,const char*, va_list);
#if defined(SQLITE_DEBUG) || defined(SQLITE_HAVE_OS_TRACE)
SQLITE_PRIVATE   void sqlite3DebugPrintf(const char*, ...);
#endif
#if defined(SQLITE_TEST)
SQLITE_PRIVATE   void *sqlite3TestTextToPtr(const char*);
#endif

#if defined(SQLITE_DEBUG)
SQLITE_PRIVATE   void sqlite3TreeViewExpr(TreeView*, const Expr*, u8);
SQLITE_PRIVATE   void sqlite3TreeViewExprList(TreeView*, const ExprList*, u8, const char*);
SQLITE_PRIVATE   void sqlite3TreeViewSelect(TreeView*, const Select*, u8);
SQLITE_PRIVATE   void sqlite3TreeViewWith(TreeView*, const With*, u8);
#endif


SQLITE_PRIVATE void sqlite3SetString(char **, sqlite3*, const char*);
SQLITE_PRIVATE void sqlite3ErrorMsg(Parse*, const char*, ...);
SQLITE_PRIVATE void sqlite3Dequote(char*);
SQLITE_PRIVATE void sqlite3TokenInit(Token*,char*);
SQLITE_PRIVATE int sqlite3KeywordCode(const unsigned char*, int);
SQLITE_PRIVATE int sqlite3RunParser(Parse*, const char*, char **);
SQLITE_PRIVATE void sqlite3FinishCoding(Parse*);
SQLITE_PRIVATE int sqlite3GetTempReg(Parse*);
SQLITE_PRIVATE void sqlite3ReleaseTempReg(Parse*,int);
SQLITE_PRIVATE int sqlite3GetTempRange(Parse*,int);
SQLITE_PRIVATE void sqlite3ReleaseTempRange(Parse*,int,int);
SQLITE_PRIVATE void sqlite3ClearTempRegCache(Parse*);
#ifdef SQLITE_DEBUG
SQLITE_PRIVATE int sqlite3NoTempsInRange(Parse*,int,int);
#endif
SQLITE_PRIVATE Expr *sqlite3ExprAlloc(sqlite3*,int,const Token*,int);
SQLITE_PRIVATE Expr *sqlite3Expr(sqlite3*,int,const char*);
SQLITE_PRIVATE void sqlite3ExprAttachSubtrees(sqlite3*,Expr*,Expr*,Expr*);
SQLITE_PRIVATE Expr *sqlite3PExpr(Parse*, int, Expr*, Expr*, const Token*);
SQLITE_PRIVATE void sqlite3PExprAddSelect(Parse*, Expr*, Select*);
SQLITE_PRIVATE Expr *sqlite3ExprAnd(sqlite3*,Expr*, Expr*);
SQLITE_PRIVATE Expr *sqlite3ExprFunction(Parse*,ExprList*, Token*);
SQLITE_PRIVATE void sqlite3ExprAssignVarNumber(Parse*, Expr*);
SQLITE_PRIVATE void sqlite3ExprDelete(sqlite3*, Expr*);
SQLITE_PRIVATE ExprList *sqlite3ExprListAppend(Parse*,ExprList*,Expr*);
SQLITE_PRIVATE void sqlite3ExprListSetSortOrder(ExprList*,int);
SQLITE_PRIVATE void sqlite3ExprListSetName(Parse*,ExprList*,Token*,int);
SQLITE_PRIVATE void sqlite3ExprListSetSpan(Parse*,ExprList*,ExprSpan*);
SQLITE_PRIVATE void sqlite3ExprListDelete(sqlite3*, ExprList*);
SQLITE_PRIVATE u32 sqlite3ExprListFlags(const ExprList*);
SQLITE_PRIVATE int sqlite3Init(sqlite3*, char**);
SQLITE_PRIVATE int sqlite3InitCallback(void*, int, char**, char**);
SQLITE_PRIVATE void sqlite3Pragma(Parse*,Token*,Token*,Token*,int);
SQLITE_PRIVATE void sqlite3ResetAllSchemasOfConnection(sqlite3*);
SQLITE_PRIVATE void sqlite3ResetOneSchema(sqlite3*,int);
SQLITE_PRIVATE void sqlite3CollapseDatabaseArray(sqlite3*);
SQLITE_PRIVATE void sqlite3CommitInternalChanges(sqlite3*);
SQLITE_PRIVATE void sqlite3DeleteColumnNames(sqlite3*,Table*);
SQLITE_PRIVATE int sqlite3ColumnsFromExprList(Parse*,ExprList*,i16*,Column**);
SQLITE_PRIVATE void sqlite3SelectAddColumnTypeAndCollation(Parse*,Table*,Select*);
SQLITE_PRIVATE Table *sqlite3ResultSetOfSelect(Parse*,Select*);
SQLITE_PRIVATE void sqlite3OpenMasterTable(Parse *, int);
SQLITE_PRIVATE Index *sqlite3PrimaryKeyIndex(Table*);
SQLITE_PRIVATE i16 sqlite3ColumnOfIndex(Index*, i16);
SQLITE_PRIVATE void sqlite3StartTable(Parse*,Token*,Token*,int,int,int,int);
#if SQLITE_ENABLE_HIDDEN_COLUMNS
SQLITE_PRIVATE   void sqlite3ColumnPropertiesFromName(Table*, Column*);
#else
# define sqlite3ColumnPropertiesFromName(T,C) /* no-op */
#endif
SQLITE_PRIVATE void sqlite3AddColumn(Parse*,Token*,Token*);
SQLITE_PRIVATE void sqlite3AddNotNull(Parse*, int);
SQLITE_PRIVATE void sqlite3AddPrimaryKey(Parse*, ExprList*, int, int, int);
SQLITE_PRIVATE void sqlite3AddCheckConstraint(Parse*, Expr*);
SQLITE_PRIVATE void sqlite3AddDefaultValue(Parse*,ExprSpan*);
SQLITE_PRIVATE void sqlite3AddCollateType(Parse*, Token*);
SQLITE_PRIVATE void sqlite3EndTable(Parse*,Token*,Token*,u8,Select*);
SQLITE_PRIVATE int sqlite3ParseUri(const char*,const char*,unsigned int*,
                    sqlite3_vfs**,char**,char **);
SQLITE_PRIVATE Btree *sqlite3DbNameToBtree(sqlite3*,const char*);
SQLITE_PRIVATE int sqlite3CodeOnce(Parse *);

#ifdef SQLITE_OMIT_BUILTIN_TEST
# define sqlite3FaultSim(X) SQLITE_OK
#else
SQLITE_PRIVATE   int sqlite3FaultSim(int);
#endif

SQLITE_PRIVATE Bitvec *sqlite3BitvecCreate(u32);
SQLITE_PRIVATE int sqlite3BitvecTest(Bitvec*, u32);
SQLITE_PRIVATE int sqlite3BitvecTestNotNull(Bitvec*, u32);
SQLITE_PRIVATE int sqlite3BitvecSet(Bitvec*, u32);
SQLITE_PRIVATE void sqlite3BitvecClear(Bitvec*, u32, void*);
SQLITE_PRIVATE void sqlite3BitvecDestroy(Bitvec*);
SQLITE_PRIVATE u32 sqlite3BitvecSize(Bitvec*);
#ifndef SQLITE_OMIT_BUILTIN_TEST
SQLITE_PRIVATE int sqlite3BitvecBuiltinTest(int,int*);
#endif

SQLITE_PRIVATE RowSet *sqlite3RowSetInit(sqlite3*, void*, unsigned int);
SQLITE_PRIVATE void sqlite3RowSetClear(RowSet*);
SQLITE_PRIVATE void sqlite3RowSetInsert(RowSet*, i64);
SQLITE_PRIVATE int sqlite3RowSetTest(RowSet*, int iBatch, i64);
SQLITE_PRIVATE int sqlite3RowSetNext(RowSet*, i64*);

SQLITE_PRIVATE void sqlite3CreateView(Parse*,Token*,Token*,Token*,ExprList*,Select*,int,int);

#if !defined(SQLITE_OMIT_VIEW) || !defined(SQLITE_OMIT_VIRTUALTABLE)
SQLITE_PRIVATE   int sqlite3ViewGetColumnNames(Parse*,Table*);
#else
# define sqlite3ViewGetColumnNames(A,B) 0
#endif

#if SQLITE_MAX_ATTACHED>30
SQLITE_PRIVATE   int sqlite3DbMaskAllZero(yDbMask);
#endif
SQLITE_PRIVATE void sqlite3DropTable(Parse*, SrcList*, int, int);
SQLITE_PRIVATE void sqlite3CodeDropTable(Parse*, Table*, int, int);
SQLITE_PRIVATE void sqlite3DeleteTable(sqlite3*, Table*);
#ifndef SQLITE_OMIT_AUTOINCREMENT
SQLITE_PRIVATE   void sqlite3AutoincrementBegin(Parse *pParse);
SQLITE_PRIVATE   void sqlite3AutoincrementEnd(Parse *pParse);
#else
# define sqlite3AutoincrementBegin(X)
# define sqlite3AutoincrementEnd(X)
#endif
SQLITE_PRIVATE void sqlite3Insert(Parse*, SrcList*, Select*, IdList*, int);
SQLITE_PRIVATE void *sqlite3ArrayAllocate(sqlite3*,void*,int,int*,int*);
SQLITE_PRIVATE IdList *sqlite3IdListAppend(sqlite3*, IdList*, Token*);
SQLITE_PRIVATE int sqlite3IdListIndex(IdList*,const char*);
SQLITE_PRIVATE SrcList *sqlite3SrcListEnlarge(sqlite3*, SrcList*, int, int);
SQLITE_PRIVATE SrcList *sqlite3SrcListAppend(sqlite3*, SrcList*, Token*, Token*);
SQLITE_PRIVATE SrcList *sqlite3SrcListAppendFromTerm(Parse*, SrcList*, Token*, Token*,
                                      Token*, Select*, Expr*, IdList*);
SQLITE_PRIVATE void sqlite3SrcListIndexedBy(Parse *, SrcList *, Token *);
SQLITE_PRIVATE void sqlite3SrcListFuncArgs(Parse*, SrcList*, ExprList*);
SQLITE_PRIVATE int sqlite3IndexedByLookup(Parse *, struct SrcList_item *);
SQLITE_PRIVATE void sqlite3SrcListShiftJoinType(SrcList*);
SQLITE_PRIVATE void sqlite3SrcListAssignCursors(Parse*, SrcList*);
SQLITE_PRIVATE void sqlite3IdListDelete(sqlite3*, IdList*);
SQLITE_PRIVATE void sqlite3SrcListDelete(sqlite3*, SrcList*);
SQLITE_PRIVATE Index *sqlite3AllocateIndexObject(sqlite3*,i16,int,char**);
SQLITE_PRIVATE Index *sqlite3CreateIndex(Parse*,Token*,Token*,SrcList*,ExprList*,int,Token*,
                          Expr*, int, int);
SQLITE_PRIVATE void sqlite3DropIndex(Parse*, SrcList*, int);
SQLITE_PRIVATE int sqlite3Select(Parse*, Select*, SelectDest*);
SQLITE_PRIVATE Select *sqlite3SelectNew(Parse*,ExprList*,SrcList*,Expr*,ExprList*,
                         Expr*,ExprList*,u32,Expr*,Expr*);
SQLITE_PRIVATE void sqlite3SelectDelete(sqlite3*, Select*);
SQLITE_PRIVATE Table *sqlite3SrcListLookup(Parse*, SrcList*);
SQLITE_PRIVATE int sqlite3IsReadOnly(Parse*, Table*, int);
SQLITE_PRIVATE void sqlite3OpenTable(Parse*, int iCur, int iDb, Table*, int);
#if defined(SQLITE_ENABLE_UPDATE_DELETE_LIMIT) && !defined(SQLITE_OMIT_SUBQUERY)
SQLITE_PRIVATE Expr *sqlite3LimitWhere(Parse*,SrcList*,Expr*,ExprList*,Expr*,Expr*,char*);
#endif
SQLITE_PRIVATE void sqlite3DeleteFrom(Parse*, SrcList*, Expr*);
SQLITE_PRIVATE void sqlite3Update(Parse*, SrcList*, ExprList*, Expr*, int);
SQLITE_PRIVATE WhereInfo *sqlite3WhereBegin(Parse*,SrcList*,Expr*,ExprList*,ExprList*,u16,int);
SQLITE_PRIVATE void sqlite3WhereEnd(WhereInfo*);
SQLITE_PRIVATE LogEst sqlite3WhereOutputRowCount(WhereInfo*);
SQLITE_PRIVATE int sqlite3WhereIsDistinct(WhereInfo*);
SQLITE_PRIVATE int sqlite3WhereIsOrdered(WhereInfo*);
SQLITE_PRIVATE int sqlite3WhereIsSorted(WhereInfo*);
SQLITE_PRIVATE int sqlite3WhereContinueLabel(WhereInfo*);
SQLITE_PRIVATE int sqlite3WhereBreakLabel(WhereInfo*);
SQLITE_PRIVATE int sqlite3WhereOkOnePass(WhereInfo*, int*);
#define ONEPASS_OFF      0        /* Use of ONEPASS not allowed */
#define ONEPASS_SINGLE   1        /* ONEPASS valid for a single row update */
#define ONEPASS_MULTI    2        /* ONEPASS is valid for multiple rows */
SQLITE_PRIVATE void sqlite3ExprCodeLoadIndexColumn(Parse*, Index*, int, int, int);
SQLITE_PRIVATE int sqlite3ExprCodeGetColumn(Parse*, Table*, int, int, int, u8);
SQLITE_PRIVATE void sqlite3ExprCodeGetColumnToReg(Parse*, Table*, int, int, int);
SQLITE_PRIVATE void sqlite3ExprCodeGetColumnOfTable(Vdbe*, Table*, int, int, int);
SQLITE_PRIVATE void sqlite3ExprCodeMove(Parse*, int, int, int);
SQLITE_PRIVATE void sqlite3ExprCacheStore(Parse*, int, int, int);
SQLITE_PRIVATE void sqlite3ExprCachePush(Parse*);
SQLITE_PRIVATE void sqlite3ExprCachePop(Parse*);
SQLITE_PRIVATE void sqlite3ExprCacheRemove(Parse*, int, int);
SQLITE_PRIVATE void sqlite3ExprCacheClear(Parse*);
SQLITE_PRIVATE void sqlite3ExprCacheAffinityChange(Parse*, int, int);
SQLITE_PRIVATE void sqlite3ExprCode(Parse*, Expr*, int);
SQLITE_PRIVATE void sqlite3ExprCodeCopy(Parse*, Expr*, int);
SQLITE_PRIVATE void sqlite3ExprCodeFactorable(Parse*, Expr*, int);
SQLITE_PRIVATE void sqlite3ExprCodeAtInit(Parse*, Expr*, int, u8);
SQLITE_PRIVATE int sqlite3ExprCodeTemp(Parse*, Expr*, int*);
SQLITE_PRIVATE int sqlite3ExprCodeTarget(Parse*, Expr*, int);
SQLITE_PRIVATE void sqlite3ExprCodeAndCache(Parse*, Expr*, int);
SQLITE_PRIVATE int sqlite3ExprCodeExprList(Parse*, ExprList*, int, int, u8);
#define SQLITE_ECEL_DUP      0x01  /* Deep, not shallow copies */
#define SQLITE_ECEL_FACTOR   0x02  /* Factor out constant terms */
#define SQLITE_ECEL_REF      0x04  /* Use ExprList.u.x.iOrderByCol */
SQLITE_PRIVATE void sqlite3ExprIfTrue(Parse*, Expr*, int, int);
SQLITE_PRIVATE void sqlite3ExprIfFalse(Parse*, Expr*, int, int);
SQLITE_PRIVATE void sqlite3ExprIfFalseDup(Parse*, Expr*, int, int);
SQLITE_PRIVATE Table *sqlite3FindTable(sqlite3*,const char*, const char*);
SQLITE_PRIVATE Table *sqlite3LocateTable(Parse*,int isView,const char*, const char*);
SQLITE_PRIVATE Table *sqlite3LocateTableItem(Parse*,int isView,struct SrcList_item *);
SQLITE_PRIVATE Index *sqlite3FindIndex(sqlite3*,const char*, const char*);
SQLITE_PRIVATE void sqlite3UnlinkAndDeleteTable(sqlite3*,int,const char*);
SQLITE_PRIVATE void sqlite3UnlinkAndDeleteIndex(sqlite3*,int,const char*);
SQLITE_PRIVATE void sqlite3Vacuum(Parse*);
SQLITE_PRIVATE int sqlite3RunVacuum(char**, sqlite3*);
SQLITE_PRIVATE char *sqlite3NameFromToken(sqlite3*, Token*);
SQLITE_PRIVATE int sqlite3ExprCompare(Expr*, Expr*, int);
SQLITE_PRIVATE int sqlite3ExprListCompare(ExprList*, ExprList*, int);
SQLITE_PRIVATE int sqlite3ExprImpliesExpr(Expr*, Expr*, int);
SQLITE_PRIVATE void sqlite3ExprAnalyzeAggregates(NameContext*, Expr*);
SQLITE_PRIVATE void sqlite3ExprAnalyzeAggList(NameContext*,ExprList*);
SQLITE_PRIVATE int sqlite3FunctionUsesThisSrc(Expr*, SrcList*);
SQLITE_PRIVATE Vdbe *sqlite3GetVdbe(Parse*);
#ifndef SQLITE_OMIT_BUILTIN_TEST
SQLITE_PRIVATE void sqlite3PrngSaveState(void);
SQLITE_PRIVATE void sqlite3PrngRestoreState(void);
#endif
SQLITE_PRIVATE void sqlite3RollbackAll(sqlite3*,int);
SQLITE_PRIVATE void sqlite3CodeVerifySchema(Parse*, int);
SQLITE_PRIVATE void sqlite3CodeVerifyNamedSchema(Parse*, const char *zDb);
SQLITE_PRIVATE void sqlite3BeginTransaction(Parse*, int);
SQLITE_PRIVATE void sqlite3CommitTransaction(Parse*);
SQLITE_PRIVATE void sqlite3RollbackTransaction(Parse*);
SQLITE_PRIVATE void sqlite3Savepoint(Parse*, int, Token*);
SQLITE_PRIVATE void sqlite3CloseSavepoints(sqlite3 *);
SQLITE_PRIVATE void sqlite3LeaveMutexAndCloseZombie(sqlite3*);
SQLITE_PRIVATE int sqlite3ExprIsConstant(Expr*);
SQLITE_PRIVATE int sqlite3ExprIsConstantNotJoin(Expr*);
SQLITE_PRIVATE int sqlite3ExprIsConstantOrFunction(Expr*, u8);
SQLITE_PRIVATE int sqlite3ExprIsTableConstant(Expr*,int);
#ifdef SQLITE_ENABLE_CURSOR_HINTS
SQLITE_PRIVATE int sqlite3ExprContainsSubquery(Expr*);
#endif
SQLITE_PRIVATE int sqlite3ExprIsInteger(Expr*, int*);
SQLITE_PRIVATE int sqlite3ExprCanBeNull(const Expr*);
SQLITE_PRIVATE int sqlite3ExprNeedsNoAffinityChange(const Expr*, char);
SQLITE_PRIVATE int sqlite3IsRowid(const char*);
SQLITE_PRIVATE void sqlite3GenerateRowDelete(
    Parse*,Table*,Trigger*,int,int,int,i16,u8,u8,u8,int);
SQLITE_PRIVATE void sqlite3GenerateRowIndexDelete(Parse*, Table*, int, int, int*, int);
SQLITE_PRIVATE int sqlite3GenerateIndexKey(Parse*, Index*, int, int, int, int*,Index*,int);
SQLITE_PRIVATE void sqlite3ResolvePartIdxLabel(Parse*,int);
SQLITE_PRIVATE void sqlite3GenerateConstraintChecks(Parse*,Table*,int*,int,int,int,int,
                                     u8,u8,int,int*,int*);
SQLITE_PRIVATE void sqlite3CompleteInsertion(Parse*,Table*,int,int,int,int*,int,int,int);
SQLITE_PRIVATE int sqlite3OpenTableAndIndices(Parse*, Table*, int, u8, int, u8*, int*, int*);
SQLITE_PRIVATE void sqlite3BeginWriteOperation(Parse*, int, int);
SQLITE_PRIVATE void sqlite3MultiWrite(Parse*);
SQLITE_PRIVATE void sqlite3MayAbort(Parse*);
SQLITE_PRIVATE void sqlite3HaltConstraint(Parse*, int, int, char*, i8, u8);
SQLITE_PRIVATE void sqlite3UniqueConstraint(Parse*, int, Index*);
SQLITE_PRIVATE void sqlite3RowidConstraint(Parse*, int, Table*);
SQLITE_PRIVATE Expr *sqlite3ExprDup(sqlite3*,Expr*,int);
SQLITE_PRIVATE ExprList *sqlite3ExprListDup(sqlite3*,ExprList*,int);
SQLITE_PRIVATE SrcList *sqlite3SrcListDup(sqlite3*,SrcList*,int);
SQLITE_PRIVATE IdList *sqlite3IdListDup(sqlite3*,IdList*);
SQLITE_PRIVATE Select *sqlite3SelectDup(sqlite3*,Select*,int);
#if SELECTTRACE_ENABLED
SQLITE_PRIVATE void sqlite3SelectSetName(Select*,const char*);
#else
# define sqlite3SelectSetName(A,B)
#endif
SQLITE_PRIVATE void sqlite3InsertBuiltinFuncs(FuncDef*,int);
SQLITE_PRIVATE FuncDef *sqlite3FindFunction(sqlite3*,const char*,int,u8,u8);
SQLITE_PRIVATE void sqlite3RegisterBuiltinFunctions(void);
SQLITE_PRIVATE void sqlite3RegisterDateTimeFunctions(void);
SQLITE_PRIVATE void sqlite3RegisterPerConnectionBuiltinFunctions(sqlite3*);
SQLITE_PRIVATE int sqlite3SafetyCheckOk(sqlite3*);
SQLITE_PRIVATE int sqlite3SafetyCheckSickOrOk(sqlite3*);
SQLITE_PRIVATE void sqlite3ChangeCookie(Parse*, int);

#if !defined(SQLITE_OMIT_VIEW) && !defined(SQLITE_OMIT_TRIGGER)
SQLITE_PRIVATE void sqlite3MaterializeView(Parse*, Table*, Expr*, int);
#endif

#ifndef SQLITE_OMIT_TRIGGER
SQLITE_PRIVATE   void sqlite3BeginTrigger(Parse*, Token*,Token*,int,int,IdList*,SrcList*,
                           Expr*,int, int);
SQLITE_PRIVATE   void sqlite3FinishTrigger(Parse*, TriggerStep*, Token*);
SQLITE_PRIVATE   void sqlite3DropTrigger(Parse*, SrcList*, int);
SQLITE_PRIVATE   void sqlite3DropTriggerPtr(Parse*, Trigger*);
SQLITE_PRIVATE   Trigger *sqlite3TriggersExist(Parse *, Table*, int, ExprList*, int *pMask);
SQLITE_PRIVATE   Trigger *sqlite3TriggerList(Parse *, Table *);
SQLITE_PRIVATE   void sqlite3CodeRowTrigger(Parse*, Trigger *, int, ExprList*, int, Table *,
                            int, int, int);
SQLITE_PRIVATE   void sqlite3CodeRowTriggerDirect(Parse *, Trigger *, Table *, int, int, int);
  void sqliteViewTriggers(Parse*, Table*, Expr*, int, ExprList*);
SQLITE_PRIVATE   void sqlite3DeleteTriggerStep(sqlite3*, TriggerStep*);
SQLITE_PRIVATE   TriggerStep *sqlite3TriggerSelectStep(sqlite3*,Select*);
SQLITE_PRIVATE   TriggerStep *sqlite3TriggerInsertStep(sqlite3*,Token*, IdList*,
                                        Select*,u8);
SQLITE_PRIVATE   TriggerStep *sqlite3TriggerUpdateStep(sqlite3*,Token*,ExprList*, Expr*, u8);
SQLITE_PRIVATE   TriggerStep *sqlite3TriggerDeleteStep(sqlite3*,Token*, Expr*);
SQLITE_PRIVATE   void sqlite3DeleteTrigger(sqlite3*, Trigger*);
SQLITE_PRIVATE   void sqlite3UnlinkAndDeleteTrigger(sqlite3*,int,const char*);
SQLITE_PRIVATE   u32 sqlite3TriggerColmask(Parse*,Trigger*,ExprList*,int,int,Table*,int);
# define sqlite3ParseToplevel(p) ((p)->pToplevel ? (p)->pToplevel : (p))
# define sqlite3IsToplevel(p) ((p)->pToplevel==0)
#else
# define sqlite3TriggersExist(B,C,D,E,F) 0
# define sqlite3DeleteTrigger(A,B)
# define sqlite3DropTriggerPtr(A,B)
# define sqlite3UnlinkAndDeleteTrigger(A,B,C)
# define sqlite3CodeRowTrigger(A,B,C,D,E,F,G,H,I)
# define sqlite3CodeRowTriggerDirect(A,B,C,D,E,F)
# define sqlite3TriggerList(X, Y) 0
# define sqlite3ParseToplevel(p) p
# define sqlite3IsToplevel(p) 1
# define sqlite3TriggerColmask(A,B,C,D,E,F,G) 0
#endif

SQLITE_PRIVATE int sqlite3JoinType(Parse*, Token*, Token*, Token*);
SQLITE_PRIVATE void sqlite3CreateForeignKey(Parse*, ExprList*, Token*, ExprList*, int);
SQLITE_PRIVATE void sqlite3DeferForeignKey(Parse*, int);
#ifndef SQLITE_OMIT_AUTHORIZATION
SQLITE_PRIVATE   void sqlite3AuthRead(Parse*,Expr*,Schema*,SrcList*);
SQLITE_PRIVATE   int sqlite3AuthCheck(Parse*,int, const char*, const char*, const char*);
SQLITE_PRIVATE   void sqlite3AuthContextPush(Parse*, AuthContext*, const char*);
SQLITE_PRIVATE   void sqlite3AuthContextPop(AuthContext*);
SQLITE_PRIVATE   int sqlite3AuthReadCol(Parse*, const char *, const char *, int);
#else
# define sqlite3AuthRead(a,b,c,d)
# define sqlite3AuthCheck(a,b,c,d,e)    SQLITE_OK
# define sqlite3AuthContextPush(a,b,c)
# define sqlite3AuthContextPop(a)  ((void)(a))
#endif
SQLITE_PRIVATE void sqlite3Attach(Parse*, Expr*, Expr*, Expr*);
SQLITE_PRIVATE void sqlite3Detach(Parse*, Expr*);
SQLITE_PRIVATE void sqlite3FixInit(DbFixer*, Parse*, int, const char*, const Token*);
SQLITE_PRIVATE int sqlite3FixSrcList(DbFixer*, SrcList*);
SQLITE_PRIVATE int sqlite3FixSelect(DbFixer*, Select*);
SQLITE_PRIVATE int sqlite3FixExpr(DbFixer*, Expr*);
SQLITE_PRIVATE int sqlite3FixExprList(DbFixer*, ExprList*);
SQLITE_PRIVATE int sqlite3FixTriggerStep(DbFixer*, TriggerStep*);
SQLITE_PRIVATE int sqlite3AtoF(const char *z, double*, int, u8);
SQLITE_PRIVATE int sqlite3GetInt32(const char *, int*);
SQLITE_PRIVATE int sqlite3Atoi(const char*);
SQLITE_PRIVATE int sqlite3Utf16ByteLen(const void *pData, int nChar);
SQLITE_PRIVATE int sqlite3Utf8CharLen(const char *pData, int nByte);
SQLITE_PRIVATE u32 sqlite3Utf8Read(const u8**);
SQLITE_PRIVATE LogEst sqlite3LogEst(u64);
SQLITE_PRIVATE LogEst sqlite3LogEstAdd(LogEst,LogEst);
#ifndef SQLITE_OMIT_VIRTUALTABLE
SQLITE_PRIVATE LogEst sqlite3LogEstFromDouble(double);
#endif
#if defined(SQLITE_ENABLE_STMT_SCANSTATUS) || \
    defined(SQLITE_ENABLE_STAT3_OR_STAT4) || \
    defined(SQLITE_EXPLAIN_ESTIMATED_ROWS)
SQLITE_PRIVATE u64 sqlite3LogEstToInt(LogEst);
#endif

/*
** Routines to read and write variable-length integers.  These used to
** be defined locally, but now we use the varint routines in the util.c
** file.
*/
SQLITE_PRIVATE int sqlite3PutVarint(unsigned char*, u64);
SQLITE_PRIVATE u8 sqlite3GetVarint(const unsigned char *, u64 *);
SQLITE_PRIVATE u8 sqlite3GetVarint32(const unsigned char *, u32 *);
SQLITE_PRIVATE int sqlite3VarintLen(u64 v);

/*
** The common case is for a varint to be a single byte.  They following
** macros handle the common case without a procedure call, but then call
** the procedure for larger varints.
*/
#define getVarint32(A,B)  \
  (u8)((*(A)<(u8)0x80)?((B)=(u32)*(A)),1:sqlite3GetVarint32((A),(u32 *)&(B)))
#define putVarint32(A,B)  \
  (u8)(((u32)(B)<(u32)0x80)?(*(A)=(unsigned char)(B)),1:\
  sqlite3PutVarint((A),(B)))
#define getVarint    sqlite3GetVarint
#define putVarint    sqlite3PutVarint


SQLITE_PRIVATE const char *sqlite3IndexAffinityStr(sqlite3*, Index*);
SQLITE_PRIVATE void sqlite3TableAffinity(Vdbe*, Table*, int);
SQLITE_PRIVATE char sqlite3CompareAffinity(Expr *pExpr, char aff2);
SQLITE_PRIVATE int sqlite3IndexAffinityOk(Expr *pExpr, char idx_affinity);
SQLITE_PRIVATE char sqlite3ExprAffinity(Expr *pExpr);
SQLITE_PRIVATE int sqlite3Atoi64(const char*, i64*, int, u8);
SQLITE_PRIVATE int sqlite3DecOrHexToI64(const char*, i64*);
SQLITE_PRIVATE void sqlite3ErrorWithMsg(sqlite3*, int, const char*,...);
SQLITE_PRIVATE void sqlite3Error(sqlite3*,int);
SQLITE_PRIVATE void sqlite3SystemError(sqlite3*,int);
SQLITE_PRIVATE void *sqlite3HexToBlob(sqlite3*, const char *z, int n);
SQLITE_PRIVATE u8 sqlite3HexToInt(int h);
SQLITE_PRIVATE int sqlite3TwoPartName(Parse *, Token *, Token *, Token **);

#if defined(SQLITE_NEED_ERR_NAME)
SQLITE_PRIVATE const char *sqlite3ErrName(int);
#endif

SQLITE_PRIVATE const char *sqlite3ErrStr(int);
SQLITE_PRIVATE int sqlite3ReadSchema(Parse *pParse);
SQLITE_PRIVATE CollSeq *sqlite3FindCollSeq(sqlite3*,u8 enc, const char*,int);
SQLITE_PRIVATE CollSeq *sqlite3LocateCollSeq(Parse *pParse, const char*zName);
SQLITE_PRIVATE CollSeq *sqlite3ExprCollSeq(Parse *pParse, Expr *pExpr);
SQLITE_PRIVATE Expr *sqlite3ExprAddCollateToken(Parse *pParse, Expr*, const Token*, int);
SQLITE_PRIVATE Expr *sqlite3ExprAddCollateString(Parse*,Expr*,const char*);
SQLITE_PRIVATE Expr *sqlite3ExprSkipCollate(Expr*);
SQLITE_PRIVATE int sqlite3CheckCollSeq(Parse *, CollSeq *);
SQLITE_PRIVATE int sqlite3CheckObjectName(Parse *, const char *);
SQLITE_PRIVATE void sqlite3VdbeSetChanges(sqlite3 *, int);
SQLITE_PRIVATE int sqlite3AddInt64(i64*,i64);
SQLITE_PRIVATE int sqlite3SubInt64(i64*,i64);
SQLITE_PRIVATE int sqlite3MulInt64(i64*,i64);
SQLITE_PRIVATE int sqlite3AbsInt32(int);
#ifdef SQLITE_ENABLE_8_3_NAMES
SQLITE_PRIVATE void sqlite3FileSuffix3(const char*, char*);
#else
# define sqlite3FileSuffix3(X,Y)
#endif
SQLITE_PRIVATE u8 sqlite3GetBoolean(const char *z,u8);

SQLITE_PRIVATE const void *sqlite3ValueText(sqlite3_value*, u8);
SQLITE_PRIVATE int sqlite3ValueBytes(sqlite3_value*, u8);
SQLITE_PRIVATE void sqlite3ValueSetStr(sqlite3_value*, int, const void *,u8,
                        void(*)(void*));
SQLITE_PRIVATE void sqlite3ValueSetNull(sqlite3_value*);
SQLITE_PRIVATE void sqlite3ValueFree(sqlite3_value*);
SQLITE_PRIVATE sqlite3_value *sqlite3ValueNew(sqlite3 *);
SQLITE_PRIVATE char *sqlite3Utf16to8(sqlite3 *, const void*, int, u8);
SQLITE_PRIVATE int sqlite3ValueFromExpr(sqlite3 *, Expr *, u8, u8, sqlite3_value **);
SQLITE_PRIVATE void sqlite3ValueApplyAffinity(sqlite3_value *, u8, u8);
#ifndef SQLITE_AMALGAMATION
SQLITE_PRIVATE const unsigned char sqlite3OpcodeProperty[];
SQLITE_PRIVATE const char sqlite3StrBINARY[];
SQLITE_PRIVATE const unsigned char sqlite3UpperToLower[];
SQLITE_PRIVATE const unsigned char sqlite3CtypeMap[];
SQLITE_PRIVATE const Token sqlite3IntTokens[];
SQLITE_PRIVATE SQLITE_WSD struct Sqlite3Config sqlite3Config;
SQLITE_PRIVATE FuncDefHash sqlite3BuiltinFunctions;
#ifndef SQLITE_OMIT_WSD
SQLITE_PRIVATE int sqlite3PendingByte;
#endif
#endif
SQLITE_PRIVATE void sqlite3RootPageMoved(sqlite3*, int, int, int);
SQLITE_PRIVATE void sqlite3Reindex(Parse*, Token*, Token*);
SQLITE_PRIVATE void sqlite3AlterFunctions(void);
SQLITE_PRIVATE void sqlite3AlterRenameTable(Parse*, SrcList*, Token*);
SQLITE_PRIVATE int sqlite3GetToken(const unsigned char *, int *);
SQLITE_PRIVATE void sqlite3NestedParse(Parse*, const char*, ...);
SQLITE_PRIVATE void sqlite3ExpirePreparedStatements(sqlite3*);
SQLITE_PRIVATE int sqlite3CodeSubselect(Parse *, Expr *, int, int);
SQLITE_PRIVATE void sqlite3SelectPrep(Parse*, Select*, NameContext*);
SQLITE_PRIVATE void sqlite3SelectWrongNumTermsError(Parse *pParse, Select *p);
SQLITE_PRIVATE int sqlite3MatchSpanName(const char*, const char*, const char*, const char*);
SQLITE_PRIVATE int sqlite3ResolveExprNames(NameContext*, Expr*);
SQLITE_PRIVATE int sqlite3ResolveExprListNames(NameContext*, ExprList*);
SQLITE_PRIVATE void sqlite3ResolveSelectNames(Parse*, Select*, NameContext*);
SQLITE_PRIVATE void sqlite3ResolveSelfReference(Parse*,Table*,int,Expr*,ExprList*);
SQLITE_PRIVATE int sqlite3ResolveOrderGroupBy(Parse*, Select*, ExprList*, const char*);
SQLITE_PRIVATE void sqlite3ColumnDefault(Vdbe *, Table *, int, int);
SQLITE_PRIVATE void sqlite3AlterFinishAddColumn(Parse *, Token *);
SQLITE_PRIVATE void sqlite3AlterBeginAddColumn(Parse *, SrcList *);
SQLITE_PRIVATE CollSeq *sqlite3GetCollSeq(Parse*, u8, CollSeq *, const char*);
SQLITE_PRIVATE char sqlite3AffinityType(const char*, u8*);
SQLITE_PRIVATE void sqlite3Analyze(Parse*, Token*, Token*);
SQLITE_PRIVATE int sqlite3InvokeBusyHandler(BusyHandler*);
SQLITE_PRIVATE int sqlite3FindDb(sqlite3*, Token*);
SQLITE_PRIVATE int sqlite3FindDbName(sqlite3 *, const char *);
SQLITE_PRIVATE int sqlite3AnalysisLoad(sqlite3*,int iDB);
SQLITE_PRIVATE void sqlite3DeleteIndexSamples(sqlite3*,Index*);
SQLITE_PRIVATE void sqlite3DefaultRowEst(Index*);
SQLITE_PRIVATE void sqlite3RegisterLikeFunctions(sqlite3*, int);
SQLITE_PRIVATE int sqlite3IsLikeFunction(sqlite3*,Expr*,int*,char*);
SQLITE_PRIVATE void sqlite3SchemaClear(void *);
SQLITE_PRIVATE Schema *sqlite3SchemaGet(sqlite3 *, Btree *);
SQLITE_PRIVATE int sqlite3SchemaToIndex(sqlite3 *db, Schema *);
SQLITE_PRIVATE KeyInfo *sqlite3KeyInfoAlloc(sqlite3*,int,int);
SQLITE_PRIVATE void sqlite3KeyInfoUnref(KeyInfo*);
SQLITE_PRIVATE KeyInfo *sqlite3KeyInfoRef(KeyInfo*);
SQLITE_PRIVATE KeyInfo *sqlite3KeyInfoOfIndex(Parse*, Index*);
#ifdef SQLITE_DEBUG
SQLITE_PRIVATE int sqlite3KeyInfoIsWriteable(KeyInfo*);
#endif
SQLITE_PRIVATE int sqlite3CreateFunc(sqlite3 *, const char *, int, int, void *,
  void (*)(sqlite3_context*,int,sqlite3_value **),
  void (*)(sqlite3_context*,int,sqlite3_value **), void (*)(sqlite3_context*),
  FuncDestructor *pDestructor
);
SQLITE_PRIVATE void sqlite3OomFault(sqlite3*);
SQLITE_PRIVATE void sqlite3OomClear(sqlite3*);
SQLITE_PRIVATE int sqlite3ApiExit(sqlite3 *db, int);
SQLITE_PRIVATE int sqlite3OpenTempDatabase(Parse *);

SQLITE_PRIVATE void sqlite3StrAccumInit(StrAccum*, sqlite3*, char*, int, int);
SQLITE_PRIVATE void sqlite3StrAccumAppend(StrAccum*,const char*,int);
SQLITE_PRIVATE void sqlite3StrAccumAppendAll(StrAccum*,const char*);
SQLITE_PRIVATE void sqlite3AppendChar(StrAccum*,int,char);
SQLITE_PRIVATE char *sqlite3StrAccumFinish(StrAccum*);
SQLITE_PRIVATE void sqlite3StrAccumReset(StrAccum*);
SQLITE_PRIVATE void sqlite3SelectDestInit(SelectDest*,int,int);
SQLITE_PRIVATE Expr *sqlite3CreateColumnExpr(sqlite3 *, SrcList *, int, int);

SQLITE_PRIVATE void sqlite3BackupRestart(sqlite3_backup *);
SQLITE_PRIVATE void sqlite3BackupUpdate(sqlite3_backup *, Pgno, const u8 *);

#ifdef SQLITE_ENABLE_STAT3_OR_STAT4
SQLITE_PRIVATE void sqlite3AnalyzeFunctions(void);
SQLITE_PRIVATE int sqlite3Stat4ProbeSetValue(Parse*,Index*,UnpackedRecord**,Expr*,u8,int,int*);
SQLITE_PRIVATE int sqlite3Stat4ValueFromExpr(Parse*, Expr*, u8, sqlite3_value**);
SQLITE_PRIVATE void sqlite3Stat4ProbeFree(UnpackedRecord*);
SQLITE_PRIVATE int sqlite3Stat4Column(sqlite3*, const void*, int, int, sqlite3_value**);
#endif

/*
** The interface to the LEMON-generated parser
*/
SQLITE_PRIVATE void *sqlite3ParserAlloc(void*(*)(u64));
SQLITE_PRIVATE void sqlite3ParserFree(void*, void(*)(void*));
SQLITE_PRIVATE void sqlite3Parser(void*, int, Token, Parse*);
#ifdef YYTRACKMAXSTACKDEPTH
SQLITE_PRIVATE   int sqlite3ParserStackPeak(void*);
#endif

SQLITE_PRIVATE void sqlite3AutoLoadExtensions(sqlite3*);
#ifndef SQLITE_OMIT_LOAD_EXTENSION
SQLITE_PRIVATE   void sqlite3CloseExtensions(sqlite3*);
#else
# define sqlite3CloseExtensions(X)
#endif

#ifndef SQLITE_OMIT_SHARED_CACHE
SQLITE_PRIVATE   void sqlite3TableLock(Parse *, int, int, u8, const char *);
#else
  #define sqlite3TableLock(v,w,x,y,z)
#endif

#ifdef SQLITE_TEST
SQLITE_PRIVATE   int sqlite3Utf8To8(unsigned char*);
#endif

#ifdef SQLITE_OMIT_VIRTUALTABLE
#  define sqlite3VtabClear(Y)
#  define sqlite3VtabSync(X,Y) SQLITE_OK
#  define sqlite3VtabRollback(X)
#  define sqlite3VtabCommit(X)
#  define sqlite3VtabInSync(db) 0
#  define sqlite3VtabLock(X)
#  define sqlite3VtabUnlock(X)
#  define sqlite3VtabUnlockList(X)
#  define sqlite3VtabSavepoint(X, Y, Z) SQLITE_OK
#  define sqlite3GetVTable(X,Y)  ((VTable*)0)
#else
SQLITE_PRIVATE    void sqlite3VtabClear(sqlite3 *db, Table*);
SQLITE_PRIVATE    void sqlite3VtabDisconnect(sqlite3 *db, Table *p);
SQLITE_PRIVATE    int sqlite3VtabSync(sqlite3 *db, Vdbe*);
SQLITE_PRIVATE    int sqlite3VtabRollback(sqlite3 *db);
SQLITE_PRIVATE    int sqlite3VtabCommit(sqlite3 *db);
SQLITE_PRIVATE    void sqlite3VtabLock(VTable *);
SQLITE_PRIVATE    void sqlite3VtabUnlock(VTable *);
SQLITE_PRIVATE    void sqlite3VtabUnlockList(sqlite3*);
SQLITE_PRIVATE    int sqlite3VtabSavepoint(sqlite3 *, int, int);
SQLITE_PRIVATE    void sqlite3VtabImportErrmsg(Vdbe*, sqlite3_vtab*);
SQLITE_PRIVATE    VTable *sqlite3GetVTable(sqlite3*, Table*);
#  define sqlite3VtabInSync(db) ((db)->nVTrans>0 && (db)->aVTrans==0)
#endif
SQLITE_PRIVATE int sqlite3VtabEponymousTableInit(Parse*,Module*);
SQLITE_PRIVATE void sqlite3VtabEponymousTableClear(sqlite3*,Module*);
SQLITE_PRIVATE void sqlite3VtabMakeWritable(Parse*,Table*);
SQLITE_PRIVATE void sqlite3VtabBeginParse(Parse*, Token*, Token*, Token*, int);
SQLITE_PRIVATE void sqlite3VtabFinishParse(Parse*, Token*);
SQLITE_PRIVATE void sqlite3VtabArgInit(Parse*);
SQLITE_PRIVATE void sqlite3VtabArgExtend(Parse*, Token*);
SQLITE_PRIVATE int sqlite3VtabCallCreate(sqlite3*, int, const char *, char **);
SQLITE_PRIVATE int sqlite3VtabCallConnect(Parse*, Table*);
SQLITE_PRIVATE int sqlite3VtabCallDestroy(sqlite3*, int, const char *);
SQLITE_PRIVATE int sqlite3VtabBegin(sqlite3 *, VTable *);
SQLITE_PRIVATE FuncDef *sqlite3VtabOverloadFunction(sqlite3 *,FuncDef*, int nArg, Expr*);
SQLITE_PRIVATE void sqlite3InvalidFunction(sqlite3_context*,int,sqlite3_value**);
SQLITE_PRIVATE sqlite3_int64 sqlite3StmtCurrentTime(sqlite3_context*);
SQLITE_PRIVATE int sqlite3VdbeParameterIndex(Vdbe*, const char*, int);
SQLITE_PRIVATE int sqlite3TransferBindings(sqlite3_stmt *, sqlite3_stmt *);
SQLITE_PRIVATE void sqlite3ParserReset(Parse*);
SQLITE_PRIVATE int sqlite3Reprepare(Vdbe*);
SQLITE_PRIVATE void sqlite3ExprListCheckLength(Parse*, ExprList*, const char*);
SQLITE_PRIVATE CollSeq *sqlite3BinaryCompareCollSeq(Parse *, Expr *, Expr *);
SQLITE_PRIVATE int sqlite3TempInMemory(const sqlite3*);
SQLITE_PRIVATE const char *sqlite3JournalModename(int);
#ifndef SQLITE_OMIT_WAL
SQLITE_PRIVATE   int sqlite3Checkpoint(sqlite3*, int, int, int*, int*);
SQLITE_PRIVATE   int sqlite3WalDefaultHook(void*,sqlite3*,const char*,int);
#endif
#ifndef SQLITE_OMIT_CTE
SQLITE_PRIVATE   With *sqlite3WithAdd(Parse*,With*,Token*,ExprList*,Select*);
SQLITE_PRIVATE   void sqlite3WithDelete(sqlite3*,With*);
SQLITE_PRIVATE   void sqlite3WithPush(Parse*, With*, u8);
#else
#define sqlite3WithPush(x,y,z)
#define sqlite3WithDelete(x,y)
#endif

/* Declarations for functions in fkey.c. All of these are replaced by
** no-op macros if OMIT_FOREIGN_KEY is defined. In this case no foreign
** key functionality is available. If OMIT_TRIGGER is defined but
** OMIT_FOREIGN_KEY is not, only some of the functions are no-oped. In
** this case foreign keys are parsed, but no other functionality is
** provided (enforcement of FK constraints requires the triggers sub-system).
*/
#if !defined(SQLITE_OMIT_FOREIGN_KEY) && !defined(SQLITE_OMIT_TRIGGER)
SQLITE_PRIVATE   void sqlite3FkCheck(Parse*, Table*, int, int, int*, int);
SQLITE_PRIVATE   void sqlite3FkDropTable(Parse*, SrcList *, Table*);
SQLITE_PRIVATE   void sqlite3FkActions(Parse*, Table*, ExprList*, int, int*, int);
SQLITE_PRIVATE   int sqlite3FkRequired(Parse*, Table*, int*, int);
SQLITE_PRIVATE   u32 sqlite3FkOldmask(Parse*, Table*);
SQLITE_PRIVATE   FKey *sqlite3FkReferences(Table *);
#else
  #define sqlite3FkActions(a,b,c,d,e,f)
  #define sqlite3FkCheck(a,b,c,d,e,f)
  #define sqlite3FkDropTable(a,b,c)
  #define sqlite3FkOldmask(a,b)         0
  #define sqlite3FkRequired(a,b,c,d)    0
#endif
#ifndef SQLITE_OMIT_FOREIGN_KEY
SQLITE_PRIVATE   void sqlite3FkDelete(sqlite3 *, Table*);
SQLITE_PRIVATE   int sqlite3FkLocateIndex(Parse*,Table*,FKey*,Index**,int**);
#else
  #define sqlite3FkDelete(a,b)
  #define sqlite3FkLocateIndex(a,b,c,d,e)
#endif


/*
** Available fault injectors.  Should be numbered beginning with 0.
*/
#define SQLITE_FAULTINJECTOR_MALLOC     0
#define SQLITE_FAULTINJECTOR_COUNT      1

/*
** The interface to the code in fault.c used for identifying "benign"
** malloc failures. This is only present if SQLITE_OMIT_BUILTIN_TEST
** is not defined.
*/
#ifndef SQLITE_OMIT_BUILTIN_TEST
SQLITE_PRIVATE   void sqlite3BeginBenignMalloc(void);
SQLITE_PRIVATE   void sqlite3EndBenignMalloc(void);
#else
  #define sqlite3BeginBenignMalloc()
  #define sqlite3EndBenignMalloc()
#endif

/*
** Allowed return values from sqlite3FindInIndex()
*/
#define IN_INDEX_ROWID        1   /* Search the rowid of the table */
#define IN_INDEX_EPH          2   /* Search an ephemeral b-tree */
#define IN_INDEX_INDEX_ASC    3   /* Existing index ASCENDING */
#define IN_INDEX_INDEX_DESC   4   /* Existing index DESCENDING */
#define IN_INDEX_NOOP         5   /* No table available. Use comparisons */
/*
** Allowed flags for the 3rd parameter to sqlite3FindInIndex().
*/
#define IN_INDEX_NOOP_OK     0x0001  /* OK to return IN_INDEX_NOOP */
#define IN_INDEX_MEMBERSHIP  0x0002  /* IN operator used for membership test */
#define IN_INDEX_LOOP        0x0004  /* IN operator used as a loop */
SQLITE_PRIVATE int sqlite3FindInIndex(Parse *, Expr *, u32, int*);

SQLITE_PRIVATE int sqlite3JournalOpen(sqlite3_vfs *, const char *, sqlite3_file *, int, int);
SQLITE_PRIVATE int sqlite3JournalSize(sqlite3_vfs *);
#ifdef SQLITE_ENABLE_ATOMIC_WRITE
SQLITE_PRIVATE   int sqlite3JournalCreate(sqlite3_file *);
#endif

SQLITE_PRIVATE int sqlite3JournalIsInMemory(sqlite3_file *p);
SQLITE_PRIVATE void sqlite3MemJournalOpen(sqlite3_file *);

SQLITE_PRIVATE void sqlite3ExprSetHeightAndFlags(Parse *pParse, Expr *p);
#if SQLITE_MAX_EXPR_DEPTH>0
SQLITE_PRIVATE   int sqlite3SelectExprHeight(Select *);
SQLITE_PRIVATE   int sqlite3ExprCheckHeight(Parse*, int);
#else
  #define sqlite3SelectExprHeight(x) 0
  #define sqlite3ExprCheckHeight(x,y)
#endif

SQLITE_PRIVATE u32 sqlite3Get4byte(const u8*);
SQLITE_PRIVATE void sqlite3Put4byte(u8*, u32);

#ifdef SQLITE_ENABLE_UNLOCK_NOTIFY
SQLITE_PRIVATE   void sqlite3ConnectionBlocked(sqlite3 *, sqlite3 *);
SQLITE_PRIVATE   void sqlite3ConnectionUnlocked(sqlite3 *db);
SQLITE_PRIVATE   void sqlite3ConnectionClosed(sqlite3 *db);
#else
  #define sqlite3ConnectionBlocked(x,y)
  #define sqlite3ConnectionUnlocked(x)
  #define sqlite3ConnectionClosed(x)
#endif

#ifdef SQLITE_DEBUG
SQLITE_PRIVATE   void sqlite3ParserTrace(FILE*, char *);
#endif

/*
** If the SQLITE_ENABLE IOTRACE exists then the global variable
** sqlite3IoTrace is a pointer to a printf-like routine used to
** print I/O tracing messages.
*/
#ifdef SQLITE_ENABLE_IOTRACE
# define IOTRACE(A)  if( sqlite3IoTrace ){ sqlite3IoTrace A; }
SQLITE_PRIVATE   void sqlite3VdbeIOTraceSql(Vdbe*);
SQLITE_API SQLITE_EXTERN void (SQLITE_CDECL *sqlite3IoTrace)(const char*,...);
#else
# define IOTRACE(A)
# define sqlite3VdbeIOTraceSql(X)
#endif

/*
** These routines are available for the mem2.c debugging memory allocator
** only.  They are used to verify that different "types" of memory
** allocations are properly tracked by the system.
**
** sqlite3MemdebugSetType() sets the "type" of an allocation to one of
** the MEMTYPE_* macros defined below.  The type must be a bitmask with
** a single bit set.
**
** sqlite3MemdebugHasType() returns true if any of the bits in its second
** argument match the type set by the previous sqlite3MemdebugSetType().
** sqlite3MemdebugHasType() is intended for use inside assert() statements.
**
** sqlite3MemdebugNoType() returns true if none of the bits in its second
** argument match the type set by the previous sqlite3MemdebugSetType().
**
** Perhaps the most important point is the difference between MEMTYPE_HEAP
** and MEMTYPE_LOOKASIDE.  If an allocation is MEMTYPE_LOOKASIDE, that means
** it might have been allocated by lookaside, except the allocation was
** too large or lookaside was already full.  It is important to verify
** that allocations that might have been satisfied by lookaside are not
** passed back to non-lookaside free() routines.  Asserts such as the
** example above are placed on the non-lookaside free() routines to verify
** this constraint.
**
** All of this is no-op for a production build.  It only comes into
** play when the SQLITE_MEMDEBUG compile-time option is used.
*/
#ifdef SQLITE_MEMDEBUG
SQLITE_PRIVATE   void sqlite3MemdebugSetType(void*,u8);
SQLITE_PRIVATE   int sqlite3MemdebugHasType(void*,u8);
SQLITE_PRIVATE   int sqlite3MemdebugNoType(void*,u8);
#else
# define sqlite3MemdebugSetType(X,Y)  /* no-op */
# define sqlite3MemdebugHasType(X,Y)  1
# define sqlite3MemdebugNoType(X,Y)   1
#endif
#define MEMTYPE_HEAP       0x01  /* General heap allocations */
#define MEMTYPE_LOOKASIDE  0x02  /* Heap that might have been lookaside */
#define MEMTYPE_SCRATCH    0x04  /* Scratch allocations */
#define MEMTYPE_PCACHE     0x08  /* Page cache allocations */

/*
** Threading interface
*/
#if SQLITE_MAX_WORKER_THREADS>0
SQLITE_PRIVATE int sqlite3ThreadCreate(SQLiteThread**,void*(*)(void*),void*);
SQLITE_PRIVATE int sqlite3ThreadJoin(SQLiteThread*, void**);
#endif

#if defined(SQLITE_ENABLE_DBSTAT_VTAB) || defined(SQLITE_TEST)
SQLITE_PRIVATE int sqlite3DbstatRegister(sqlite3*);
#endif

#endif /* _SQLITEINT_H_ */

/************** End of sqliteInt.h *******************************************/
