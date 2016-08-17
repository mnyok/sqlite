//
//  main.c
//  sqlite_test
//
//  Created by 김성민 on 2016. 7. 6..
//  Copyright © 2016년 Purple. All rights reserved.
//

#include "../../../common/common.h"



#define SRC_PATH  "/Users/Purple/Dropbox/ug/sqlite/xcode/sqlite_test/sqlite_test"


typedef enum {
    
   DELETE  ,
   PERSIST ,
   OFF     ,
   TRUNCATE,
   MEMORY  ,
   WAL
    
} JournalingMode;


char* fstring(const char * fmt,...){
    
    static char buffer[1000] = {0,};
    
    va_list va;
    va_start (va, fmt);
    vsprintf (buffer, fmt, va);
    va_end (va);
    
    return buffer;
    
}


void change_directory_to_source_folder(){
    
    chdir(SRC_PATH "/testdir");
}

void delete_files(const char* name){
    
    remove(name);
    remove(fstring("%s-wal",name));
    remove(fstring("%s-shm",name));
    remove(fstring("%s-journal",name));
    remove(fstring("%s-mj-stored",name));
    

}
void create_db_and_make_table_and_set_journaling(sqlite3** db,const char* name, const char* table_name, JournalingMode journaling_mode){
    
    char buf[10];
    
    
    sqlite3_open_v2(name,db,SQLITE_OPEN_CREATE | SQLITE_OPEN_READWRITE,nil);
    
    switch(journaling_mode){
        case  DELETE  : sprintf(buf,"delete"); break;
        case   PERSIST : sprintf(buf,"persist"); break;
        case OFF     :sprintf(buf,"off"); break;
        case TRUNCATE: sprintf(buf,"truncate"); break;
        case MEMORY  : sprintf(buf,"memory"); break;
        case WAL: sprintf(buf,"wal"); break;
        default: exit(-1);

            
    }
    
    sql_execute(*db,fstring("pragma journal_mode = %s",buf));
    
    
    sql_execute(*db,fstring("create table %s (a,b,c)",table_name));
    
    
    
}

void read_both(sqlite3* db){
    sqlite3_open_v2("test.db", &db, SQLITE_OPEN_READWRITE, nil);
    
    sql_execute(db,"attach 'test2.db' as aux");
    
    sql_execute(db, "select * from t1 except select * from t2",true);
    
    sqlite3_close(db);
}

void read(sqlite3* db){
    
    sqlite3_open_v2("test2.db", &db, SQLITE_OPEN_READWRITE, nil);
    
    sql_execute(db, "select * from t2");
    
    sqlite3_close(db);
}

void transaction(sqlite3* db,sqlite3* db2){
    delete_files("test.db");
    delete_files("test2.db");
    
    create_db_and_make_table_and_set_journaling(&db, "test.db", "t1", WAL);
    
    
    create_db_and_make_table_and_set_journaling(&db2, "test2.db", "t2", WAL);
    sqlite3_close(db2);
    
    sql_execute(db,"attach 'test2.db' as aux");
    sql_execute(db,"begin");
    
    for(int i = 0 ; i < 10 ; i ++){
        
        sql_execute(db, "insert into t1 values (randomblob(4),randomblob(4),randomblob(4))");
        sql_execute(db, "insert into aux.t2 values (randomblob(4),randomblob(4),randomblob(4))");
    }
    sql_execute(db,"commit");
    
    
    sqlite3_close(db);
}

#define nil NULL
int main(){
    
    sqlite3* db;
    sqlite3* db2;
    

    change_directory_to_source_folder();
    sqlite3_config(SQLITE_CONFIG_LOG,sql_log);
    
 
    read_both(db);
//    read(db);
//    transaction(db,db2);
    
    
    
    

    puts("complete");
    
    
    
    
    
    return 0;
}

