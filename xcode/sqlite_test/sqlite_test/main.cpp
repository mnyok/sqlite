//
//  main.c
//  sqlite_test
//
//  Created by 김성민 on 2016. 7. 6..
//  Copyright © 2016년 Purple. All rights reserved.
//

#include "../../../common/common.h"
#include <time.h>


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

void delete_files(){
    
    system("/bin/rm *");
    
    
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

void read_both(){
    sqlite3* db;
    
    sqlite3_open_v2("test.db", &db, SQLITE_OPEN_READWRITE, nil);
    
    sql_execute(db,"attach 'test2.db' as aux");
    
    sql_execute(db, "select * from t1 except select * from t2",true);
    
    sqlite3_close(db);
}

void read_both_twice(){
  sqlite3* db;

  sqlite3_open_v2("test.db", &db, SQLITE_OPEN_READWRITE, nil);

  sql_execute(db,"attach 'test2.db' as aux");

  sql_execute(db, "select * from t1 except select * from t2",true);
  sql_execute(db, "select * from t1 except select * from t2",true);

  sqlite3_close(db);
}

void read_twice(){
    
    
    sqlite3* db;
    sqlite3_open_v2("test.db", &db, SQLITE_OPEN_READWRITE, nil);
    
    sql_execute(db, "select * from t1");
    sql_execute(db, "select * from t1");
    
    sqlite3_close(db);
    
    
}

void read(){
    
    sqlite3* db;
    sqlite3_open_v2("test.db", &db, SQLITE_OPEN_READWRITE, nil);
    
    sql_execute(db, "select * from t1");
    
    sqlite3_close(db);
}

void transaction(){
    
    sqlite3* db;
    sqlite3* db2;
    delete_files();
    
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

void checkpoint_transaction_test(){
    
    sqlite3* db;
    //    system("/bin/rm *");
    delete_files();
    
    create_db_and_make_table_and_set_journaling(&db, "test.db", "t1", WAL);
    
    sql_execute(db,"attach 'test2.db' as aux");
    
    sql_execute(db,"pragma aux.journal_mode = wal");
    sql_execute(db,"create table aux.t2(a, b, c)");
    sql_execute(db,"pragma wal_checkpoint(full)");
    
    //    sql_execute(db,"insert into t1 values (1,2,3)");
    
    sql_execute(db,"begin");
    
    for(int i = 0 ; i < 10 ; i ++){
        
        sql_execute(db, "insert into t1 values (randomblob(4),randomblob(4),randomblob(4))");
        sql_execute(db, "insert into aux.t2 values (randomblob(4),randomblob(4),randomblob(4))");
    }
    sql_execute(db,"commit");
    
    
    sqlite3_close(db);
}

void checkpoint_test(){
    
    sqlite3* db;
    
    delete_files();
    
    sqlite3_open_v2("test.db", &db, SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE, nil);
    
    sql_execute(db, "pragma journal_mode = wal;");
    sql_execute(db, "create table t1 (a primary key, b);");
    //    sql_execute(db, "pragma wal_checkpoint(full);");
    sql_execute(db, "pragma wal_checkpoint(restart);");
    
    sql_execute(db, "insert into t1 values (1,2);");
    
    sqlite3_close(db);
    
    
}

void checkpoint_only_test(){
    sqlite3* db;
    sqlite3_open_v2("test.db", &db, SQLITE_OPEN_READWRITE, nil);
    
    sql_execute(db, "pragma wal_checkpoint(full);");
    
}

static int count = 1000;

void insert(sqlite3* db){
    
    sqlite3_exec(db,"begin",0,0,0);
    
    
    for(int i = 0 ; i < count ; i ++){
        
        sqlite3_exec(db,"insert into t1 values (1,2,3);"
                     "insert into t2 values (2,3,4);",0,0,0);
        
    }
    
    
    sqlite3_exec(db,"commit",0,0,0);
}
void update(sqlite3* db){
    sqlite3_exec(db,"begin",0,0,0);
    
    
    for(int i = 0 ; i < count ; i ++){
        
        sqlite3_exec(db,"insert into t1 values (1,2,3);"
                     "insert into t2 values (2,3,4);",0,0,0);
        
    }
    
    
    sqlite3_exec(db,"commit",0,0,0);
}

int test(){
    
    sqlite3* db;
    sqlite3_open_v2("test.db", &db, SQLITE_OPEN_CREATE | SQLITE_OPEN_READWRITE, NULL);
    
    sqlite3_exec(db, "attach 'test2.db' as aux;"
                 "pragma main.journal_mode = wal;"
                 "pragma aux.journal_mode = wal;"
                 "create table main.t1(a,b,c);"
                 "create table aux.t2(a,b,c);", 0, 0, 0);
    
    
    
    
    sqlite3_close(db);
    
    return 0;
}

#define nil NULL
int main(){
    
    
    
    change_directory_to_source_folder();
    sqlite3_config(SQLITE_CONFIG_LOG,sql_log);
    
//    delete_files();
    
    //    checkpoint_only_test();
    //    checkpoint_transaction_test();
    //    checkpoint_test();
//        read_both_twice();
//    read();
//    read_twice();
//        read();
        transaction();

    
    
    
    
    
    puts("complete");
    
    
    
    
    
    return 0;
}

