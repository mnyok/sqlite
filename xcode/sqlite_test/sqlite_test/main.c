//
//  main.c
//  sqlite_test
//
//  Created by 김성민 on 2016. 7. 6..
//  Copyright © 2016년 Purple. All rights reserved.
//

#include <stdio.h>
#include "sqlite3.h"
#include <stdlib.h>
#include <string.h>
#include <unistd.h>


#define SRC_PATH  "/Users/Purple/Dropbox/ug/sqlite/xcode/sqlite_test/sqlite_test"


#define nil NULL

#define check() if(rc != SQLITE_OK){puts(sqlite3_errmsg(db)); puts("sqlite close"); sqlite3_close(db); exit(1);}

int sql(sqlite3* db, const char * sql){
    int rc;
    
    rc = sqlite3_exec(db, sql, nil, nil, nil);
    
    check();
    
    return rc;
}

void change_directory_to_source_folder(){
    
    chdir(SRC_PATH);
}

#define nil NULL
int main(){
    
    change_directory_to_source_folder();
    
    int rc;
    
    sqlite3* db;
    rc = sqlite3_open_v2("test1.db",&db,SQLITE_OPEN_READWRITE,nil);
    
    check();


    sql(db,"attach database 'test2.db' as t2");

    sql(db,"begin transaction");
    
    sql(db,"insert into tb1 values (1,2,3)");
    
    sql(db,"insert into t2.tb2 values (3,4,5)");
    
    
    sql(db,"commit transaction");
    
    sqlite3_close(db);
    return 0;
}
