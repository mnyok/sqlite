# Atomic Multi-database Transaction of WAL Journaling Mode in SQLite

[Read paper about it here](https://drive.google.com/file/d/0B5vadFDZKX2pbmtYY1NyMEx0c00/view)

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
