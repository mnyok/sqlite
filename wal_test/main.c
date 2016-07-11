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

/* Handle callback from sql
** argc : number of data
** argv : data
** colName : name of data column
*/
static int callback_sum(void* sum, int argc, char** argv, char** colName)
{
    *(int*)sum += atoi(argv[0]);
	return 0;
}

int sql_insert(sqlite3* db, const char * table, int a, int b, int c){
    int i;
    int rc;
    char buffer[100];
    char sql[1000] = "insert into ";

    strcat(sql, table);
    strcat(sql, " values (");

    sprintf(buffer, "%d", a);
    strcat(sql, buffer);
    strcat(sql, ",");

    sprintf(buffer, "%d", b);
    strcat(sql, buffer);
    strcat(sql, ",");

    sprintf(buffer, "%d", c);
    strcat(sql, buffer);
    strcat(sql, ")");

    // printf("%s\n", sql);

    rc = sqlite3_exec(db, sql, nil, nil, nil);

    check();

    return rc;
}

//update column b to b + value where table.a = a
int sql_update(sqlite3* db, const char * table, int a, int value){
    int i;
    int rc;
    char buffer[100];
    char sql[1000] = "update ";

    strcat(sql, table);
    strcat(sql, " set b = b");

    sprintf(buffer, "%+d", value);
    strcat(sql, buffer);

    strcat(sql, " where a = ");
    sprintf(buffer, "%d", a);
    strcat(sql, buffer);

    // printf("%s\n", sql);

    rc = sqlite3_exec(db, sql, nil, nil, nil);

    check();

    return rc;
}

//insert random set
int sql_insert_rand(sqlite3* db, const char * table){
    int i;
    int rc;
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

//update every a to random number less than 100
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
    int rc;
    int i;
    int before = 0;
    int after = 0;

    srand((unsigned)time(NULL) + (unsigned)getpid());

    sqlite3* db;
    rc = sqlite3_open_v2("test1-wal.db",&db,SQLITE_OPEN_READWRITE,nil);

    check();

    sql(db,"attach database 'test2-wal.db' as t2");

    sql(db,"delete from tb1");
    sql(db,"delete from t2.tb2");

    //insert data and save sum of first column
    int insert_data;
    for(i = 0; i < 1000; i++){
        insert_data = rand() % 100;
        before += insert_data * 2;
        sql_insert(db, "tb1", i, insert_data, insert_data);
        sql_insert(db, "t2.tb2", i, insert_data, insert_data);
    }
    printf("Before sum = %d\n", before);

    int update_value;
    int update_row;
    int j = 0;
    for(i = 0; i < 10; i++){
        update_row = rand() % 1000;
        update_value = 1;
        sql(db,"begin transaction");

        for(j = 0; j < 5; j++){
            sql_update(db, "tb1", update_row, update_value);

            // if(rand() % 100 < 5){
            //     printf("%d %d\n", i, j);
            //     exit(-1);
            //     // sqlite3_interrupt(db);
            //     // continue;
            // }
            //
            // sql_update(db, "t2.tb2", update_row, update_value * -1);
        }

        sql(db,"commit transaction");
    }

    sqlite3_exec(db, "select SUM(b) as sum_b from tb1", callback_sum, &after, nil);
    sqlite3_exec(db, "select SUM(b) as sum_b from t2.tb2", callback_sum, &after, nil);
    printf("After sum = %d\n", after);

    // sql(db,"delete from tb1");
    // sql(db,"delete from t2.tb2");

    sqlite3_close(db);
    return 0;
}
