# Atomic Multi-database Transaction of WAL Journaling Mode in SQLite

[Read the paper about it here](https://drive.google.com/file/d/0B5vadFDZKX2pbmtYY1NyMEx0c00/view)

This project solves the problem of WAL(Write-Ahead Logging) that transactions involving changes with multiple attached databases do not guarantee atomicity across all databases as a set in recovery.

This project is based on SQLite 3.13.0. You can checkout the original source code [here](http://sqlite.org/download.html).

If you want to see the differences with the original source code, run `git diff f36212ee8a9af1f`


## How to run the tests

```
git clone https://github.com/purpleblues/sqlite

cd sqlite

./configure

make quicktest # for the quick test.

make test # for the normal test which usually takes minutes.

make fulltest # for the full test which takes HOURS.


# Note
# In the test 'oserror', a call to getcwd() may fail if there are no free file descriptors in macOS.
# This problem occurs with the original SQLite test script in the same way.
```


## Recovery
src/wal.c 에 대부분의 WAL 복구에 관한 함수들이 모여있습니다.

Multi-database transaction을 수행할때 WAL은 `sqlite3WalFrames()`를 호출하고, 트랜잭션을 시작하기 전 wal파일의 프레임 갯수, 즉 기존 상태를 mj-stored 파일에 `walWriteMasterStoreFile()`를 통해 저장하게 됩니다. transaction이 무사히 끝나게 되면 `walZeroMasterStore()`를 호출하여 다음번에 사용 되지 않게 하고, 중간에 크래쉬가 나게 된다면 다음번 db를 open 하여 처음으로 read를 시작할때 복구를 시작하게 됩니다.

WAL을 사용하는 DB가 read를 시작할때 `walIndexReadHdr()` 를 호출합니다.

이 함수에서 `walMxFrameFromMasterStore()` 를 통해 복구를 해야하는지 여부와, 복구를 해야한다면 WAL 파일에서 남겨야하는 프레임의 갯수와 마스터 저널 파일 이름을 읽어옵니다. 이 함수 안에서 mj-stored 파일을 읽어오는 일은 `walReadMasterJournal()` 에서 수행합니다.

그 후 `walIndexRecover()`를 호출하여 실제 WAL파일에서 이전 트랜잭션에서 추가된 obsolte한  프레임을 지워야하는 작업을 수행하고, `walZeroMasterStore()`를 통해 mj-stored 파일이 이미 사용 됐으니 다음번엔 사용하지 않아도 된다는 것을 표시합니다.

복구가 끝나면 `pager_delmaster()`를 호출하여 master journal file을 지워야 하는지 확인하고, 지워야 한다면 실제 지우는 작업을 수행합니다. 이 함수는 다른 저널링 모드에서도 공통적으로 사용합니다.

위의 함수들 중 `pager_delmaster()` 는 src/pager.c에, 나머지는 전부 src/wal.c에 있습니다.
