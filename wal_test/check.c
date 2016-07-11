#include "../common/common.c"


/* Handle callback from sql
** argc : number of data
** argv : data
** colName : name of data column
*/
static int callback_sum(void* sum, int argc, char** argv, char** colName)
{
    int i;
    int j;

    for(i = 0; i < argc; i++){
        printf("%s :%s ", colName[i], argv[i]);
        if( i % 4 == 3){
            (*(int*)sum)++;
            printf("\n");
        }
    }

	return 0;
}

#define nil NULL
int main(){
    int rc;
    int i;
    int result = 0;

    sqlite3* db;
    rc = sqlite3_open_v2("test1-wal.db",&db,SQLITE_OPEN_READWRITE,nil);

    check();

    sql_execute(db,"attach database 'test2-wal.db' as t2");

    rc = sqlite3_exec(db, "select * from (select tb1.a as A, tb1.b as B1, tb1.c as C, t2.tb2.b as B2 from tb1 inner join t2.tb2 on tb1.a = t2.tb2.a) where (C * 2) != (B1 + B2)", callback_sum, &result, nil);
    check();
    printf("Number of unvalid column: %d\n", result);

    sqlite3_close(db);
    return rc;
}
