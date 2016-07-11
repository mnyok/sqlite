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
    int i;
    int j;

    for(i = 0; i < argc; i++){
        printf("%s :%s ", colName[i], argv[i]);
        if( i % 4 == 3){
            (*(int*)sum)++;
            printf("\n");
        }
    }

	return 0;
}

#define nil NULL
int main(){
    int rc;
    int i;
    int result = 0;

    sqlite3* db;
    rc = sqlite3_open_v2("test1-wal.db",&db,SQLITE_OPEN_READWRITE,nil);

    check();

    sql(db,"attach database 'test2-wal.db' as t2");

    rc = sqlite3_exec(db, "select * from (select tb1.a as A, tb1.b as B1, tb1.c as C, t2.tb2.b as B2 from tb1 inner join t2.tb2 on tb1.a = t2.tb2.a) where (C * 2) != (B1 + B2)", callback_sum, &result, nil);
    check();
    printf("Number of unvalid column: %d\n", result);

    sqlite3_close(db);
    return rc;
}
