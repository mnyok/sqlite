

#ifndef __COMMON__
#define __COMMON__

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <math.h>
#include <time.h>
#include <signal.h>
#include "../sqlite3.h"




#define nil NULL
#define true 1
#define false 0

typedef unsigned char boolean;

double get_time_milisecond(void);

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
