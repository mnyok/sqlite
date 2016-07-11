



#include "common.h"




int sql_execute(sqlite3* db, const char * sql){
    int rc;

    rc = sqlite3_exec(db, sql, nil, nil, nil);

    check();

    return rc;
}

//insert into table values (a, b, c)
int sql_insert(sqlite3* db, const char * table, int a, int b, int c){
//    int i;
    int rc;
    char buffer[100];
    char sql[100] = "insert into ";

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

//update table set b = b + value where a = key
int sql_update(sqlite3* db, const char * table, int key, int value){
//    int i;
    int rc;
    char buffer[100];
    char sql[100] = "update ";

    strcat(sql, table);
    strcat(sql, " set b = b");

    sprintf(buffer, "%+d", value);
    strcat(sql, buffer);

    strcat(sql, " where a = ");
    sprintf(buffer, "%d", key);
    strcat(sql, buffer);

    // printf("%s\n", sql);

    rc = sqlite3_exec(db, sql, nil, nil, nil);

    check();

    return rc;
}

int sql_insert_rand(sqlite3* db, const char * table){
    int rc;
    int i;
    char buffer[100];
    char sql[100] = "insert into ";

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
    char sql[100] = "update ";

    strcat(sql, table);
    strcat(sql, " set a = ");

    sprintf(buffer, "%d", rand() % 100);
    strcat(sql, buffer);

    // printf("%s\n", sql);

    rc = sql_execute(db,sql);

    return rc;
}
