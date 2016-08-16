sqlite3_crash_enable 1
sqlite3_crashparams   1 /Users/Purple/Dropbox/ug/sqlite/xcode/sqlite_test/sqlite_test/testdir/test2.db-wal
sqlite3_test_control_pending_byte 65536
 sqlite3 db test.db -vfs crash 
db eval {SELECT * FROM sqlite_master;}
set bt [btree_from_db db]
btree_set_cache_size $bt 10
db eval {

      ATTACH 'test2.db' AS aux;
      BEGIN;
        INSERT INTO t1 VALUES(randomblob(1024), randomblob(1024), randomblob(1024));
        INSERT INTO t2 VALUES(randomblob(1024), randomblob(1024), randomblob(1024));
        INSERT INTO t1 VALUES(randomblob(1024), randomblob(1024), randomblob(1024));
        INSERT INTO t2 VALUES(randomblob(1024), randomblob(1024), randomblob(1024));
        INSERT INTO t1 VALUES(randomblob(1024), randomblob(1024), randomblob(1024));
        INSERT INTO t2 VALUES(randomblob(1024), randomblob(1024), randomblob(1024));


      COMMIT;
    
}
