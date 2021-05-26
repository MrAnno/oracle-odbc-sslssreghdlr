alter session set "_ORACLE_SCRIPT"=true;

CREATE USER test_user IDENTIFIED BY test_passwd;
GRANT CONNECT TO test_user;
GRANT CONNECT, RESOURCE, DBA TO test_user;
GRANT UNLIMITED TABLESPACE TO test_user;
ALTER USER test_user QUOTA UNLIMITED ON USERS;

CREATE TABLE test_user.test_table (testcol VARCHAR(255));

shutdown immediate;
startup mount;
alter database noarchivelog;
alter database open;
alter database flashback off;
shutdown;
startup;
exit;
