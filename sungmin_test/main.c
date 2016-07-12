//
//  main.c
//  sqlite_test
//
//  Created by 김성민 on 2016. 7. 6..
//  Copyright © 2016년 Purple. All rights reserved.
//

#include "../common/common.h"


#define nil NULL
int main(){
    
    
    int rc;
    int i;
    
    srand((unsigned)time(NULL) + (unsigned)getpid());
    
    sqlite3* db;
    rc = sqlite3_open_v2("test1-wal.db",&db,SQLITE_OPEN_READWRITE,nil);
    
    check();
    
    
    sql_execute(db,"attach database 'test2-wal.db' as t2");
    
    for(i = 0; i < 1000; i++){
        sql_execute(db,"begin transaction");
    
        sql_insert_rand(db, "tb1");
        sql_insert_rand(db, "tb1");
        sql_insert_rand(db, "t2.tb2");
        sql_insert_rand(db, "t2.tb2");
        
        sql_update_rand(db, "tb1");
        
        if(i == 500){
            printf("exit with 1\n");
            exit(1);
////            sigkill();
        }

        sql_execute(db,"commit transaction");
        
       
    }
    
    sql_execute(db,"delete from tb1");
    sql_execute(db,"delete from t2.tb2");
    
    sqlite3_close(db);
    return 0;
}

