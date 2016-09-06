#include <time.h>
#include "../common/common.h"


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
    double time_start = 0;

    srand((unsigned)time(NULL) + (unsigned)getpid());

    sqlite3* db;

    rc = sqlite3_open_v2("test2-wal.db",&db,SQLITE_OPEN_READWRITE,nil);
    sqlite3_close(db);

    rc = sqlite3_open_v2("test1-wal.db",&db,SQLITE_OPEN_READWRITE,nil);

    sql_execute(db,"attach database 'test2-wal.db' as t2", 0);

    sql_execute(db,"delete from tb1", 0);
    sql_execute(db,"delete from t2.tb2", 0);

    sql_execute(db, "PRAGMA main.journal_mode = WAL", 0);
    sql_execute(db, "PRAGMA t2.journal_mode = WAL", 0);

    printf("Multi-database Insert Test..\n");
    time_start = get_time_milisecond();
    //insert data and save sum of first column
    int insert_data;
    for(i = 0; i < 10000; i++){
        insert_data = rand() % 100;
        sql_execute(db, "begin transaction", 0);
        sql_insert(db, "tb1", i, insert_data, insert_data);
        sql_insert(db, "t2.tb2", i, insert_data, insert_data);
        sql_execute(db, "commit transaction", 0);
    }

    printf("running time: %f\n", get_time_milisecond() - time_start);

    
    printf("Multi-database Update Test..\n");
    time_start = get_time_milisecond();
    //update
    int update_value;
    int update_row;
    int j = 0;
    for(i = 0; i < 2000; i++){
        update_row = rand() % 1000;
        update_value = 1;
        sql_execute(db,"begin transaction", 0);

        for(j = 0; j < 5; j++){
            sql_update(db, "tb1", update_row, update_value);
            sql_update(db, "t2.tb2", update_row, update_value * -1);
        }

        sql_execute(db,"commit transaction", 0);
    }
    printf("running time: %f\n", get_time_milisecond() - time_start);

    sqlite3_close(db);
    return 0;
}
