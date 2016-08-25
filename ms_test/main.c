//
//  main.c
//  sqlite_test
//
//  Created by 김성민 on 2016. 7. 6..
//  Copyright © 2016년 Purple. All rights reserved.
//

#include "../common/common.c"


<<<<<<< HEAD
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
=======
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

>>>>>>> 2a2f2dd... cherrypick common files

int main(){

    // change_directory_to_source_folder();

    int rc;
    int i;

    srand((unsigned)time(NULL) + (unsigned)getpid());

    sqlite3* db;
    rc = sqlite3_open_v2("test1-wal.db",&db,SQLITE_OPEN_READWRITE,nil);

    check();

<<<<<<< HEAD

    sql(db,"attach database 'test2-wal.db' as t2");
=======
    sql_execute(db,"attach database 'test2-wal.db' as t2");

    sql_execute(db,"delete from tb1");
    sql_execute(db,"delete from t2.tb2");
>>>>>>> 2a2f2dd... cherrypick common files

    for(i = 0; i < 1000; i++){
<<<<<<< HEAD
        sql(db,"begin transaction");

        // sql(db,"insert into tb1 values (1,2,3)");
        //
        // sql(db,"insert into t2.tb2 values (3,4,5)");
        // sql(db,"update tb1 set a = 2 where a = 1");

        sql_insert_rand(db, "tb1");
        sql_insert_rand(db, "tb1");
        sql_insert_rand(db, "t2.tb2");
        sql_insert_rand(db, "t2.tb2");
=======
        insert_data = rand() % 100;
        before += insert_data * 2;
        sql_insert(db, "tb1", i, insert_data, insert_data);
        sql_insert(db, "t2.tb2", i, insert_data, insert_data);
    }
    printf("Before sum = %d\n", before);

    int update_value;
    int update_row;
    int j = 0;
    for(i = 0; i < 200; i++){
        update_row = rand() % 1000;
        update_value = 1;
        sql_execute(db,"begin transaction");

        for(j = 0; j < 5; j++){
            sql_update(db, "tb1", update_row, update_value);

            // if(rand() % 100 < 5){
            //     printf("%d %d\n", i, j);
            //     exit(-1);
            //     // sqlite3_interrupt(db);
            //     // continue;
            // }

            sql_update(db, "t2.tb2", update_row, update_value * -1);
        }

        sql_execute(db,"commit transaction");
    }
>>>>>>> 2a2f2dd... cherrypick common files

        sql_update_rand(db, "tb1");

<<<<<<< HEAD
        // exit(-1);

    sql(db,"commit transaction");
    }

    sql(db,"delete from tb1");
    sql(db,"delete from t2.tb2");
=======
    // sql_execute(db,"delete from tb1");
    // sql_execute(db,"delete from t2.tb2");
>>>>>>> 2a2f2dd... cherrypick common files

    sqlite3_close(db);
    return 0;
}
