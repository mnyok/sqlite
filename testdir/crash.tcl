sqlite3_crash_enable 1
sqlite3_crashparams   2 /Users/Purple/Dropbox/ug/sqlite/testdir/test.db
sqlite3_test_control_pending_byte 65536
 sqlite3 db test.db -vfs crash 
db eval {SELECT * FROM sqlite_master;}
set bt [btree_from_db db]
btree_set_cache_size $bt 10
db eval {

        SELECT * FROM sqlite_master;
        INSERT INTO t1 VALUES(randomblob(900));
      
}
