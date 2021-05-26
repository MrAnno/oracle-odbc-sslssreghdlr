#!/usr/bin/env bash
set -e
set -x

cd oracle-odbc-sslssreghdlr
make clean
make build

set +x
while true; do
  ./oracle-odbc-sslssreghdlr 'DRIVER=/opt/oracle/instantclient_21_1/libsqora.so.21.1;UID=test_user;PWD=test_passwd;DBQ=oracle-db:1521/TESTDB;APA=T'
done
