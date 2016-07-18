sqlite3_crash_enable 1
sqlite3_crashparams -s 512  3 /home/ubuntu/sqlite/test/testdir/test.db
sqlite3_test_control_pending_byte 65536
 sqlite3 db test.db -vfs crash 
db eval {SELECT * FROM sqlite_master;}
set bt [btree_from_db db]
btree_set_cache_size $bt 10
db eval {SELECT randomblob(793)}
db eval {

      PRAGMA page_size = 4096;
      PRAGMA journal_mode = wal;
      BEGIN;
        CREATE TABLE t1(a, b);
        INSERT INTO t1 VALUES(1, 2);
      COMMIT;
      PRAGMA wal_checkpoint;
      CREATE INDEX i1 ON t1(a);
      PRAGMA wal_checkpoint;
    
}
