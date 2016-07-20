

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <math.h>
#include <time.h>
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
