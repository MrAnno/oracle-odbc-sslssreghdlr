# Oracle ODBC driver hangs in sslssrecursivemx_acquire()

When `SQL_ATTR_QUERY_TIMEOUT` is set, multi-threaded unixODBC applications may
hang on `sslssreghdlr`.

(Might be related to: Multi-threaded UNIX ODBC application hung on SSLSSREGHDLR. (Bug 21459317))

This repository contains an example application that can be used to reproduce
the issue.

## Prerequisites

- x86-64 Linux environment with GNU Make, pkg-config, gcc, gdb preinstalled
- unixODBC v2.3.7 or later (`pkg-config` is used for detection)
- Oracle Instant Client v21.1.0.0.0 or later with ODBC support
- A Oracle DBMS configured to be accessible, a table with a single row is required
  - table name: test_table

## Usage

1. Add the location of Oracle Instant Client to `LD_LIBRARY_PATH`, for example:

   `export LD_LIBRARY_PATH=/opt/instantclient_21_1/`

2. Build example application:

   `make build`

3. Run example repeatedly:

   `while true; do ./oracle-odbc-sslssreghdlr 'DRIVER=/opt/instantclient_21_1/libsqora.so.21.1;UID=test_user;PWD=test_passwd;DBQ=localhost:1521/TESTDB;APA=T'; done`

   The output should look like this:
   ```
   Connected to <...>
   Connected to <...>
   Query executed (SELECT * FROM "test_table")
   Query executed (SELECT * FROM "test_table")
   ```

4. Wait until it gets stuck

```
(gdb) thread apply all bt

Thread 3 (LWP 432284 "oracle-odbc-ssl"):
#0  0x00007fafcc734125 in clock_nanosleep@GLIBC_2.2.5 () from /usr/lib/libc.so.6
#1  0x00007fafcc739357 in nanosleep () from /usr/lib/libc.so.6
#2  0x00007fafb813c2c5 in kpucpincrtime () from /opt/instantclient_21_1/libclntsh.so.21.1
#3  0x00007fafcc843299 in start_thread () from /usr/lib/libpthread.so.0
#4  0x00007fafcc76c053 in clone () from /usr/lib/libc.so.6

Thread 2 (LWP 432280 "oracle-odbc-ssl"):
#0  0x00007fafcc84c6e0 in __lll_lock_wait () from /usr/lib/libpthread.so.0
#1  0x00007fafcc8455f0 in pthread_mutex_lock () from /usr/lib/libpthread.so.0
#2  0x00007fafcad2a010 in sslssrecursivemx_acquire () from /opt/instantclient_21_1/libclntshcore.so.21.1
#3  0x00007fafcad28749 in sslssreghdlr () from /opt/instantclient_21_1/libclntshcore.so.21.1
#4  0x00007fafcb404785 in bcoSQLExecute () from /opt/instantclient_21_1/libsqora.so.21.1
#5  0x00007fafcb41cc2f in SQLExecDirectW () from /opt/instantclient_21_1/libsqora.so.21.1
#6  0x00007fafcc8727c6 in SQLExecDirect () from /usr/lib/libodbc.so.2
#7  0x00005645537c8550 in exec_query ()
#8  0x00005645537c85fc in thread_run_sql_test ()
#9  0x00007fafcc843299 in start_thread () from /usr/lib/libpthread.so.0
#10 0x00007fafcc76c053 in clone () from /usr/lib/libc.so.6

Thread 1 (LWP 432278 "oracle-odbc-ssl"):
#0  0x00007fafcc84f9ba in __futex_abstimed_wait_common64 () from /usr/lib/libpthread.so.0
#1  0x00007fafcc8447a3 in __pthread_clockjoin_ex () from /usr/lib/libpthread.so.0
#2  0x00005645537c870b in main ()
```


Binary (requires glibc 2.17):

https://github.com/MrAnno/oracle-odbc-sslssreghdlr/releases/download/v1.0.0/oracle-odbc-sslssreghdlr

## Notes

- The application starts 2 threads, each executing a single SQL query.
- There is no shared resource between threads, only `connection_string` is shared.
- Using a shared `SQLHENV` across threads, or separate table names will make no difference.
- Removing `SQLSetStmtAttr(SQL_ATTR_QUERY_TIMEOUT)` from the application seems to be a workaround.
