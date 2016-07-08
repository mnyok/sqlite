



#include "common.h"




int sql_execute(sqlite3* db, const char * sql){
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
    
    rc = sql_execute(db,sql);
    
    return rc;
}


int sql_update_rand(sqlite3* db, const char * table){
    int rc;
    //    int i;
    char buffer[100];
    char sql[1000] = "update ";
    
    strcat(sql, table);
    strcat(sql, " set a = ");
    
    sprintf(buffer, "%d", rand() % 100);
    strcat(sql, buffer);
    
    // printf("%s\n", sql);
    
    rc = sql_execute(db,sql);
    
    return rc;
}