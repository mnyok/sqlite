



#include "common.h"

//  void (*xLog)(void*,int,const char*);

void sql_log(void* data, int resultCode, const char* msg){
    
}

int sql_callback(void *data, int argc, char **argv, char **azColName)
{
    int i;
    fprintf(stderr, "%s: ", (const char*)data);
    for(i=0; i<argc; i++){
        printf("%s = %s, ", azColName[i], argv[i] ? argv[i] : "NULL");
    }
    printf("\n");
    return 0;
}

<<<<<<< HEAD
int check(sqlite3* db,int rc){
    if(rc != SQLITE_OK){
        puts(sqlite3_errmsg(db));
        puts("sqlite close");
        sqlite3_close(db);
        exit(1);
    }
    
    return rc;

}


int sql_execute(sqlite3* db, const char * sql, boolean useCallback){

    int rc;
    
    puts(sql);
    
//    rc = sqlite3_exec(db, sql, useCallback ? sql_callback : nil, nil, nil);

    rc = sqlite3_exec(db, sql, useCallback ? sql_callback : nil, (void*)"Callback function", nil);

    check(db,rc);
=======
int sql_execute(sqlite3* db, const char * sql){
    int rc;

    rc = sqlite3_exec(db, sql, nil, nil, nil);

    check();
>>>>>>> 3fd02fbe8ac3d421bdd94658831331c0abbd3697

    return rc;
}

//insert into table values (a, b, c)
int sql_insert(sqlite3* db, const char * table, int a, int b, int c){
<<<<<<< HEAD
//    int i;
    int rc;
    char buffer[100];
    char sql[100] = "insert into ";
=======
    int i;
    int rc;
    char buffer[100];
    char sql[1000] = "insert into ";
>>>>>>> 3fd02fbe8ac3d421bdd94658831331c0abbd3697

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

<<<<<<< HEAD
    rc = sql_execute(db, sql,false);

    check(db,rc);
=======
    rc = sqlite3_exec(db, sql, nil, nil, nil);

    check();
>>>>>>> 3fd02fbe8ac3d421bdd94658831331c0abbd3697

    return rc;
}

//update table set b = b + value where a = key
int sql_update(sqlite3* db, const char * table, int key, int value){
<<<<<<< HEAD
//    int i;
    int rc;
    char buffer[100];
    char sql[100] = "update ";
=======
    int i;
    int rc;
    char buffer[100];
    char sql[1000] = "update ";
>>>>>>> 3fd02fbe8ac3d421bdd94658831331c0abbd3697

    strcat(sql, table);
    strcat(sql, " set b = b");

    sprintf(buffer, "%+d", value);
    strcat(sql, buffer);

    strcat(sql, " where a = ");
    sprintf(buffer, "%d", key);
    strcat(sql, buffer);

    // printf("%s\n", sql);

    rc = sqlite3_exec(db, sql, nil, nil, nil);

<<<<<<< HEAD
    check(db,rc);
=======
    check();
>>>>>>> 3fd02fbe8ac3d421bdd94658831331c0abbd3697

    return rc;
}

int sql_insert_rand(sqlite3* db, const char * table){
    int rc;
    int i;
    char buffer[100];
<<<<<<< HEAD
    char sql[100] = "insert into ";
=======
    char sql[1000] = "insert into ";
>>>>>>> 3fd02fbe8ac3d421bdd94658831331c0abbd3697

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

<<<<<<< HEAD
    rc = sql_execute(db,sql,false);
=======
    rc = sql_execute(db,sql);
>>>>>>> 3fd02fbe8ac3d421bdd94658831331c0abbd3697

    return rc;
}


int sql_update_rand(sqlite3* db, const char * table){
    int rc;
    //    int i;
    char buffer[100];
<<<<<<< HEAD
    char sql[100] = "update ";
=======
    char sql[1000] = "update ";
>>>>>>> 3fd02fbe8ac3d421bdd94658831331c0abbd3697

    strcat(sql, table);
    strcat(sql, " set a = ");

    sprintf(buffer, "%d", rand() % 100);
    strcat(sql, buffer);

    // printf("%s\n", sql);

<<<<<<< HEAD
    rc = sql_execute(db,sql,false);

    return rc;
}

void sigkill(){
    kill(getpid(),SIGKILL);
}
=======
    rc = sql_execute(db,sql);

    return rc;
}
>>>>>>> 3fd02fbe8ac3d421bdd94658831331c0abbd3697
