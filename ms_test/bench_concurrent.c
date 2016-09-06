#include <time.h>
#include <unistd.h>
#include <pthread.h>
#include "../common/common.h"

static int count = 0;

void *t_test(void* arg){
    int insert_data;
    int select_row;
    int i, j;
    while (i < 1000){
        i = __sync_fetch_and_add(&count, 1);
        if (i % 2) {
            for(j = 0; j < 5; j++){
                insert_data = rand() % 100;
                sql_execute(db, "begin transaction", 0);
                sql_insert(db, "tb1", (i*5 + j), insert_data, insert_data);
                sql_insert(db, "t2.tb2", (i*5 + j), insert_data, insert_data);
                sql_execute(db, "commit transaction", 0);
            }
        } else {
            for(j = 0; j < 50; j++){
                select_row = rand() % i; 
                sql_execute(db, "begin transaction", 0);
                sql_execute(db, "select * from tb1", 0);
                sql_execute(db, "select * from t2.tb2", 0);
                sql_execute(db, "commit transaction", 0);
            }
        }    
    }
}

int main(){
    int rc;
    int i;
    double time_start = 0;
    pthread_t* thread_test[2];

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

    //test start
    pthread_create(thread_test[0], NULL, t_test, 0);
    pthread_create(thread_test[1], NULL, t_test, 0);

    pthread_join(thread_test[0], NULL);
    pthread_join(thread_test[1], NULL);

    printf("Multi-database Insert Test..\n");
    time_start = get_time_milisecond();

    printf("running time: %f\n", get_time_milisecond() - time_start);

    sqlite3_close(db);
    return 0;
}
