/******************************************************************************
** This file is an amalgamation of many separate C source files from SQLite
** version 3.13.0.  By combining all the individual C code files into this 
** single large file, the entire code can be compiled as a single translation
** unit.  This allows many compilers to do optimizations that would not be
** possible if the files were compiled separately.  Performance improvements
** of 5% or more are commonly seen when SQLite is compiled as a single
** translation unit.
**
** This file is all you need to compile SQLite.  To use SQLite in other
** programs, you need this file and the "sqlite3.h" header file that defines
** the programming interface to the SQLite library.  (If you do not have 
** the "sqlite3.h" header file at hand, you will find a copy embedded within
** the text of this file.  Search for "Begin file sqlite3.h" to find the start
** of the embedded sqlite3.h header file.) Additional code files may be needed
** if you want a wrapper to interface SQLite with your choice of programming
** language. The code for the "sqlite3" command-line shell is also in a
** separate file. This file contains only code for the core SQLite library.
*/

#define SQLITE_DEBUG 1


#define SQLITE_CORE 1
#define SQLITE_AMALGAMATION 1
#ifndef SQLITE_PRIVATE
# define SQLITE_PRIVATE static
#endif
#include "sqlite3-1.c"
#include "sqlite3-2.c"
#include "sqlite3-3.c"
#include "sqlite3-4.c"
#include "sqlite3-5.c"
#include "sqlite3-6.c"
#include "sqlite3-7.c"
#include "sqlite3-8.c"
#include "sqlite3-9.c"
#include "sqlite3-10.c"
#include "sqlite3-11.c"
#include "sqlite3-12.c"
#include "sqlite3-13.c"
#include "sqlite3-14.c"
#include "sqlite3-15.c"
#include "sqlite3-16.c"
#include "sqlite3-17.c"
#include "sqlite3-18.c"
#include "sqlite3-19.c"
#include "sqlite3-20.c"
#include "sqlite3-21.c"
#include "sqlite3-22.c"
#include "sqlite3-23.c"
#include "sqlite3-24.c"
#include "sqlite3-25.c"
#include "sqlite3-26.c"
#include "sqlite3-27.c"
#include "sqlite3-28.c"
#include "sqlite3-29.c"
#include "sqlite3-30.c"
#include "sqlite3-31.c"
#include "sqlite3-32.c"
#include "sqlite3-33.c"
#include "sqlite3-34.c"
#include "sqlite3-35.c"
#include "sqlite3-36.c"
#include "sqlite3-37.c"
#include "sqlite3-38.c"
#include "sqlite3-39.c"
