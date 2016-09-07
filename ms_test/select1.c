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

    //A: index B: updated value C: original(inserted) value
    rc = sqlite3_exec(db, "select * from (select tb1.a as A, tb1.c as C, tb1.b as B1, t2.tb2.b as B2 from tb1 inner join t2.tb2 on tb1.a = t2.tb2.a)", callback_sum, &result, nil);
    check();
    printf("Number of column: %d\n", result);

    sqlite3_close(db);
    return rc;
}
