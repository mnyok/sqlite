//
//  main.c
//  sqlite_test
//
//  Created by 김성민 on 2016. 7. 6..
//  Copyright © 2016년 Purple. All rights reserved.
//

#include "../common/common.c"


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


int main(){
    int rc;
    int i;
    int before = 0;
    int after = 0;

    srand((unsigned)time(NULL) + (unsigned)getpid());

    sqlite3* db;
    rc = sqlite3_open_v2("test1-wal.db",&db,SQLITE_OPEN_READWRITE,nil);

    check();

    sql_execute(db,"delete from tb1");

    //insert data and save sum of first column
    int insert_data;
    for(i = 0; i < 1000; i++){
        insert_data = rand() % 100;
        before += insert_data;
        sql_insert(db, "tb1", i, insert_data, insert_data);
    }
    printf("Before sum = %d\n", before);

    int update_value;
    int j = 0;
    for(i = 0; i < 200; i++){
        update_value = 1;
        sql_execute(db,"begin transaction");

        for(j = 0; j < 5; j++){
            sql_update(db, "tb1", rand() % 1000, update_value);
            sql_update(db, "tb1", rand() % 1000, update_value * -1);
        }

        sql_execute(db,"commit transaction");
    }

    sqlite3_exec(db, "select SUM(b) as sum_b from tb1", callback_sum, &after, nil);

    printf("After sum = %d\n", after);

    // sql_execute(db,"delete from tb1");
    // sql_execute(db,"delete from t2.tb2");

    sqlite3_close(db);
    return 0;
}
