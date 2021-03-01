#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h> /* :*/

#include <pthread.h>

#include <sql.h>
#include <sqlext.h>
#include <sqltypes.h>

#define NUM_OF_THREADS 2
#define CONNECTION_TIMEOUT 5
#define TEST_SQL_QUERY "SELECT * FROM \"test_table\""

static const char* connection_string;

typedef struct SQLHandles
{
    SQLHENV env;
    SQLHDBC conn;
} SQLHandles;

static void exit_error(const char* err)
{
    fprintf(stderr, "%s\n", err);
    exit(1);
}

static void print_sql_error(SQLRETURN result, SQLSMALLINT type, SQLHANDLE handle)
{
    SQLCHAR buffer[SQL_MAX_MESSAGE_LENGTH + 1];
    SQLCHAR sqlstate[SQL_SQLSTATE_SIZE + 1];
    SQLINTEGER sqlcode;

    for (SQLSMALLINT i = 1; SQLGetDiagRec(type, handle, i, sqlstate, &sqlcode, buffer, sizeof(buffer), NULL) == SQL_SUCCESS; ++i)
        fprintf(stderr, "state: %s, code: %d, str: %s\n", sqlstate, sqlcode, buffer);
}

static SQLHENV alloc_environment_handle(void)
{
    SQLHENV env;
    SQLRETURN result = SQLAllocHandle(SQL_HANDLE_ENV, SQL_NULL_HANDLE, &env);
    if (!SQL_SUCCEEDED(result))
        exit_error("Error allocating environment handle");

    return env;
}

static void set_odbc_version(SQLHENV env)
{
    SQLRETURN result = SQLSetEnvAttr(env, SQL_ATTR_ODBC_VERSION, (void *) SQL_OV_ODBC3, 0);
    if (!SQL_SUCCEEDED(result))
        exit_error("Error setting ODBC version");
}

static SQLHDBC connect_to_oracle(SQLHENV env)
{
    SQLHDBC conn;
    SQLRETURN result = SQLAllocHandle(SQL_HANDLE_DBC, env, &conn);
    if (!SQL_SUCCEEDED(result))
        exit_error("Error allocating connection handle");

    result = SQLSetConnectAttr(conn, SQL_ATTR_LOGIN_TIMEOUT, (SQLPOINTER) CONNECTION_TIMEOUT, SQL_IS_UINTEGER);
    if (!SQL_SUCCEEDED(result))
        exit_error("Error setting login timeout");

    result = SQLSetConnectAttr(conn, SQL_ATTR_CONNECTION_TIMEOUT, (SQLPOINTER) CONNECTION_TIMEOUT, SQL_IS_UINTEGER);
    if (!SQL_SUCCEEDED(result))
        exit_error("Error setting connection timeout");

    result = SQLDriverConnect(conn, NULL, (SQLCHAR *) connection_string, SQL_NTS, NULL, 0, NULL, SQL_DRIVER_NOPROMPT);
    if (!SQL_SUCCEEDED(result)) {
        print_sql_error(result, SQL_HANDLE_DBC, conn);
        exit_error("Error connecting to database");
    }

    printf("Connected to <%s>\n", connection_string);
    return conn;
}

static void exec_query(const SQLHandles* handles)
{
    SQLHSTMT statement;
    SQLRETURN result = SQLAllocHandle(SQL_HANDLE_STMT, handles->conn, &statement);
    if (!SQL_SUCCEEDED(result))
        exit_error("Error allocating statement handle");

    /* disabling SQL_ATTR_QUERY_TIMEOUT seems to be a workaround */
    result = SQLSetStmtAttr(statement, SQL_ATTR_QUERY_TIMEOUT, (SQLPOINTER) CONNECTION_TIMEOUT, SQL_IS_UINTEGER);
    if (!SQL_SUCCEEDED(result))
        exit_error("Error setting query timeout");

    result = SQLExecDirect(statement, (SQLCHAR *) TEST_SQL_QUERY, SQL_NTS);
    if(!SQL_SUCCEEDED(result)) {
        print_sql_error(result, SQL_HANDLE_STMT, statement);
        exit_error("Error executing query");
    }

    printf("Query executed (%s)\n", TEST_SQL_QUERY);

    SQLFreeHandle(SQL_HANDLE_STMT, statement);
}

static void* thread_run_sql_test(void* arg)
{
    SQLHandles handles;
    handles.env = alloc_environment_handle();
    set_odbc_version(handles.env);

    handles.conn = connect_to_oracle(handles.env);

    exec_query(&handles);

    SQLDisconnect(handles.conn);
    SQLFreeHandle(SQL_HANDLE_DBC, handles.conn);
    SQLFreeHandle(SQL_HANDLE_ENV, handles.env);
}

int main(int argc, char** argv)
{
    if (argc != 2) {
        fprintf(stderr, "usage: %s <connection-string>\n", argv[0]);
        fprintf(stderr, "example: %s 'DRIVER=/opt/instantclient_21_1/libsqora.so.21.1;UID=test_user;PWD=test_passwd;DBQ=localhost:1521/TESTDB;APA=T'\n", argv[0]);
        return 2;
    }

    assert(setenv("NLS_LANG", ".UTF8", 0) == 0);

    connection_string = argv[1];

    pthread_t threads[NUM_OF_THREADS];

    for (int t = 0; t < NUM_OF_THREADS; ++t) {
        int err = pthread_create(&threads[t], NULL, thread_run_sql_test, NULL);
        if (err != 0) {
            exit_error("Error creating pthread");
        }
    }

    for (int t = 0; t < NUM_OF_THREADS; ++t) {
        int err = pthread_join(threads[t], NULL);
        if (err != 0) {
            exit_error("Error joining pthread");
        }
    }

    return 0;
}
