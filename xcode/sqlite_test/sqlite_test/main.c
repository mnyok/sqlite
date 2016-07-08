//
//  main.c
//  sqlite_test
//
//  Created by 김성민 on 2016. 7. 6..
//  Copyright © 2016년 Purple. All rights reserved.
//

#include "../../../common/common.h"


#define SRC_PATH  "/Users/Purple/Dropbox/ug/sqlite/xcode/sqlite_test/sqlite_test"


void change_directory_to_source_folder(){
    
    chdir(SRC_PATH);
}

#define nil NULL
int main(){
    
    change_directory_to_source_folder();
    
    int rc;
    int i;
    
    srand((unsigned)time(NULL) + (unsigned)getpid());
    
    sqlite3* db;
    rc = sqlite3_open_v2("test1-wal.db",&db,SQLITE_OPEN_READWRITE,nil);
    
    check();
    
    
    sql(db,"attach database 'test2-wal.db' as t2");
    
    for(i = 0; i < 1; i++){
        sql(db,"begin transaction");
    
        sql_insert_rand(db, "tb1");
        sql_insert_rand(db, "tb1");
        sql_insert_rand(db, "t2.tb2");
        sql_insert_rand(db, "t2.tb2");
        
        sql_update_rand(db, "tb1");

        sql(db,"commit transaction");
    }
    
    sql(db,"delete from tb1");
    sql(db,"delete from t2.tb2");
    
    sqlite3_close(db);
    return 0;
}

