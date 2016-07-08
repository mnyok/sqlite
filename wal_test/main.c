//
//  main.c
//  sqlite_test
//
//  Created by 김성민 on 2016. 7. 6..
//  Copyright © 2016년 Purple. All rights reserved.
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <math.h>
#include "../sqlite3.h"


#define SRC_PATH  "/Users/Purple/Dropbox/ug/sqlite/xcode/sqlite_test/sqlite_test"


#define nil NULL

#define check() if(rc != SQLITE_OK){puts(sqlite3_errmsg(db)); puts("sqlite close"); sqlite3_close(db); exit(1);}

int sql(sqlite3* db, const char * sql){
    int rc;

    rc = sqlite3_exec(db, sql, nil, nil, nil);

    check();

    return rc;
}

int sql_insert_rand(sqlite3* db, const char * table){
    int rc;
    int i;
    char buffer[100];
    char sql[1000] = "insert into ";

    strcat(sql, table);
    strcat(sql, " values (");

    for(i = 0; i < 3; i++){
        sprintf(buffer, "%d", rand() % 100);
        strcat(sql, buffer);
        if(i != 2){
            strcat(sql, ",");
        }
    }
    strcat(sql, ")");

    // printf("%s\n", sql);

    rc = sqlite3_exec(db, sql, nil, nil, nil);

    check();

    return rc;
}

int sql_update_rand(sqlite3* db, const char * table){
    int rc;
    int i;
    char buffer[100];
    char sql[1000] = "update ";

    strcat(sql, table);
    strcat(sql, " set a = ");

    sprintf(buffer, "%d", rand() % 100);
    strcat(sql, buffer);

    // printf("%s\n", sql);

    rc = sqlite3_exec(db, sql, nil, nil, nil);

    check();

    return rc;
}

void change_directory_to_source_folder(){

    chdir(SRC_PATH);
}

#define nil NULL
int main(){

    // change_directory_to_source_folder();

    int rc;
    int i;

    srand((unsigned)time(NULL) + (unsigned)getpid());

    sqlite3* db;
    rc = sqlite3_open_v2("test1-wal.db",&db,SQLITE_OPEN_READWRITE,nil);

    check();


    sql(db,"attach database 'test2-wal.db' as t2");

    for(i = 0; i < 1000; i++){
        sql(db,"begin transaction");

        // sql(db,"insert into tb1 values (1,2,3)");
        //
        // sql(db,"insert into t2.tb2 values (3,4,5)");
        // sql(db,"update tb1 set a = 2 where a = 1");

        sql_insert_rand(db, "tb1");
        sql_insert_rand(db, "tb1");
        sql_insert_rand(db, "t2.tb2");
        sql_insert_rand(db, "t2.tb2");

        sql_update_rand(db, "tb1");

        // exit(-1);

    sql(db,"commit transaction");
    }

    sql(db,"delete from tb1");
    sql(db,"delete from t2.tb2");

    sqlite3_close(db);
    return 0;
}
