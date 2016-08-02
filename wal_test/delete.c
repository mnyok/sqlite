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

    rc = sql_execute(db,"delete from tb1");

    check();

    rc = sqlite3_close(db);

    check();

    rc = sqlite3_open_v2("test2-wal.db",&db,SQLITE_OPEN_READWRITE,nil);

    check();

    sql_execute(db,"delete from tb2");

    sqlite3_close(db);
    return 0;
}
