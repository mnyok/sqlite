sqlite3_crash_enable 1
<<<<<<< HEAD
sqlite3_crashparams -s 512  3 /home/ubuntu/sqlite/test/testdir/test.db
=======
sqlite3_crashparams   1 /Users/Purple/Dropbox/ug/sqlite/test/testdir/test1.db-wal
>>>>>>> cce5628904989b800e61a6a66d837c4c50025b0d
sqlite3_test_control_pending_byte 65536
 sqlite3 db test.db -vfs crash 
db eval {SELECT * FROM sqlite_master;}
set bt [btree_from_db db]
btree_set_cache_size $bt 10
<<<<<<< HEAD
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
=======
db eval {

      PRAGMA journal_mode = WAL;
    --   pragma fullsync = off;
      ATTACH 'test2.db' AS aux;
      CREATE TABLE t1(a PRIMARY KEY, b);
      CREATE TABLE aux.t2(a PRIMARY KEY, b);
      BEGIN;
        INSERT INTO t1 VALUES(1, 2);
        INSERT INTO t2 VALUES(1, 2);
        INSERT INTO t1 VALUES(2, 3);
        INSERT INTO t2 VALUES(2, 3);
        INSERT INTO t1 VALUES(3, 4);
        INSERT INTO t2 VALUES(3, 4);
      COMMIT;
>>>>>>> cce5628904989b800e61a6a66d837c4c50025b0d
    
}
