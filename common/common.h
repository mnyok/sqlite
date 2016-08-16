

<<<<<<< HEAD
=======
#ifndef __COMMON__
#define __COMMON__

>>>>>>> cce5628904989b800e61a6a66d837c4c50025b0d
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <math.h>
#include <time.h>
<<<<<<< HEAD
#include "../sqlite3.h"


#define nil NULL

#define check() \
                \
    if(rc != SQLITE_OK){\
        puts(sqlite3_errmsg(db)); \
        puts("sqlite close"); \
        sqlite3_close(db); \
        exit(1);\
    }



int sql_execute(sqlite3* db, const char * sql);
int sql_insert(sqlite3* db, const char * table, int a, int b, int c);
int sql_update(sqlite3* db, const char * table, int key, int value);
int sql_insert_rand(sqlite3* db, const char * table);
int sql_update_rand(sqlite3* db, const char * table);
=======
#include <signal.h>
#include "../sqlite3.h"




#define nil NULL
#define true 1
#define false 0

typedef unsigned char boolean;


void sql_log(void* data, int resultCode, const char* msg);

int check(sqlite3* db,int rc);


int sql_callback(void *data, int argc, char **argv, char **azColName);



#ifdef __cplusplus
int sql_execute(sqlite3* db, const char * sql, boolean useCallback = false);
#else
int sql_execute(sqlite3* db, const char * sql, boolean useCallback);
#endif


//int sql_insert(sqlite3* db, const char * table, int a, int b, int c);
//int sql_update(sqlite3* db, const char * table, int key, int value);
//int sql_insert_rand(sqlite3* db, const char * table);
//int sql_update_rand(sqlite3* db, const char * table);

void sigkill();

#endif
>>>>>>> cce5628904989b800e61a6a66d837c4c50025b0d
