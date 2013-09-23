
#ifndef incl_EXT_PGSQL_H_
#define incl_EXT_PGSQL_H_

#include "hphp/runtime/base/base-includes.h"

#ifdef HAVE_PGSQL
#include <libpq-fe.h>

#define PGSQL_ASSOC 1
#define PGSQL_NUM 2
#define PGSQL_BOTH (PGSQL_ASSOC | PGSQL_NUM)
#define k_PGSQL_BOTH PGSQL_BOTH
#define PGSQL_STATUS_LONG 1
#define PGSQL_STATUS_STRING 2

namespace HPHP {


///////////////////////////////////////////////////////////////////////////////

class PGSQL : public SweepableResourceData {

public:
    static bool AllowPersistent;
    static int MaxPersistent;
    static int MaxLinks;
    static bool AutoResetPersistent;
    static bool IgnoreNotice;
    static bool LogNotice;

    static PGSQL *Get(CVarRef conn_id);
    static PGconn *GetConn(CVarRef conn_id, PGSQL **rconn = NULL);


public:
    PGSQL(String conninfo, bool async);
    ~PGSQL();

    void close();

    static StaticString s_class_name;
    virtual CStrRef o_getClassNameHook() const { return s_class_name; }
    virtual bool isResource() const { return m_conn != NULL; }

    PGconn * get() { return m_conn; }

    bool isNonBlocking() { return PQisnonblocking(m_conn) == 1; }
    int  setNonBlocking(bool val=true) { return PQsetnonblocking(m_conn, (int)val); }

private:
    PGconn *m_conn;

public:
    std::string m_conn_string;

    std::string m_db;
    std::string m_user;
    std::string m_pass;
    std::string m_host;
    std::string m_port;
    std::string m_options;

    std::string m_last_notice;
};

class PGSQLResult : public SweepableResourceData {
public:
    static PGSQLResult *Get(CVarRef result);
    static PGresult *GetResult(CVarRef result, PGSQLResult **rres = NULL);
public:
    PGSQLResult(PGSQL* conn, PGresult *res);
    ~PGSQLResult();

    static StaticString s_class_name;
    virtual CStrRef o_getClassNameHook() const { return s_class_name; }
    virtual bool isResource() const { return m_res != NULL; }

    void close();

    PGresult *get() { return m_res; }

    int getFieldNumber(CVarRef field);
    int getNumFields();
    int getNumRows();

    bool convertFieldRow(CVarRef row, CVarRef field,
        int *out_row, int *out_field, const char *fn_name = NULL);

    Variant fieldIsNull(CVarRef row, CVarRef field, const char *fn_name = NULL);

    Variant getFieldVal(CVarRef row, CVarRef field, const char *fn_name = NULL);
    String getFieldVal(int row, int field, const char *fn_name = NULL);

    PGSQL * getConn() { return m_conn; }

public:
    int m_current_row;
private:
    PGresult *m_res;
    int m_num_fields;
    int m_num_rows;
    PGSQL * m_conn;
};

/////////////////////////////////////////////////////////////////////////////////////////////

int64_t HHVM_FUNCTION(pg_affected_rows, CResRef result);
bool    HHVM_FUNCTION(pg_cancel_query, CResRef connection);
Variant HHVM_FUNCTION(pg_client_encoding, CResRef connection);
bool    HHVM_FUNCTION(pg_close, CResRef connection);
Variant HHVM_FUNCTION(pg_connect, CStrRef connection_string, int connect_type = 0);
Variant HHVM_FUNCTION(pg_async_connect, CStrRef connection_string, int connect_type = 0);
bool    HHVM_FUNCTION(pg_connection_busy, CResRef connection);
bool    HHVM_FUNCTION(pg_connection_reset, CResRef connection);
int64_t HHVM_FUNCTION(pg_connection_status, CResRef connection);
Variant HHVM_FUNCTION(pg_convert, CResRef connection, CStrRef table_name, CArrRef assoc_array, int options = 0);
bool    HHVM_FUNCTION(pg_copy_from, CResRef connection, CStrRef table_name, CArrRef rows, CStrRef delimiter = "\t", CStrRef null_as = "\n");
Variant HHVM_FUNCTION(pg_copy_to, CResRef connection, CStrRef table_name, CStrRef delimiter = "\t", CStrRef null_as = "\n");
Variant HHVM_FUNCTION(pg_dbname, CResRef connection);
bool    HHVM_FUNCTION(pg_end_copy, CResRef connection);
String  HHVM_FUNCTION(pg_escape_bytea, CResRef connection, CStrRef data);
String  HHVM_FUNCTION(pg_escape_identifier, CResRef connection, CStrRef data);
String  HHVM_FUNCTION(pg_escape_literal, CResRef connection, CStrRef data);
String  HHVM_FUNCTION(pg_escape_string, CResRef connection, CStrRef data);
Variant HHVM_FUNCTION(pg_execute, CResRef connection, CStrRef stmtname, CArrRef params);
Variant HHVM_FUNCTION(pg_fetch_all_columns, CResRef result, int64_t column = 0);
Variant HHVM_FUNCTION(pg_fetch_all, CResRef result);
Variant HHVM_FUNCTION(pg_fetch_array, CResRef result, CVarRef row = null_variant, int64_t result_type = PGSQL_BOTH);
Variant HHVM_FUNCTION(pg_fetch_assoc, CResRef result, CVarRef row = null_variant);
Variant HHVM_FUNCTION(pg_fetch_result, CResRef result, CVarRef row = null_variant, CVarRef field = null_variant);
Variant HHVM_FUNCTION(pg_fetch_row, CResRef result, CVarRef row = null_variant);
Variant HHVM_FUNCTION(pg_field_is_null, CResRef result, CVarRef row, CVarRef field = null_variant);
Variant HHVM_FUNCTION(pg_field_name, CResRef result, int64_t field_number);
int64_t HHVM_FUNCTION(pg_field_num, CResRef result, CStrRef field_name);
Variant HHVM_FUNCTION(pg_field_prtlen, CResRef result, CVarRef row_number, CVarRef field = null_variant);
Variant HHVM_FUNCTION(pg_field_size, CResRef result, int64_t field_number);
Variant HHVM_FUNCTION(pg_field_table, CResRef result, int64_t field_number, bool oid_only = false);
Variant HHVM_FUNCTION(pg_field_type_oid, CResRef result, int64_t field_number);
Variant HHVM_FUNCTION(pg_field_type, CResRef result, int64_t field_number);
bool    HHVM_FUNCTION(pg_free_result, CResRef result);
Variant HHVM_FUNCTION(pg_get_notify, CResRef connection, int64_t result_type = PGSQL_BOTH);
int64_t HHVM_FUNCTION(pg_get_pid, CResRef connection);
Variant HHVM_FUNCTION(pg_get_result, CResRef connection);
Variant HHVM_FUNCTION(pg_host, CResRef connection);
Variant HHVM_FUNCTION(pg_last_error, CResRef connection);
Variant HHVM_FUNCTION(pg_last_notice, CResRef connection);
Variant HHVM_FUNCTION(pg_last_oid, CResRef result);
Variant HHVM_FUNCTION(pg_meta_data, CResRef connection, CStrRef table_name);
int64_t HHVM_FUNCTION(pg_num_fields, CResRef result);
int64_t HHVM_FUNCTION(pg_num_rows, CResRef result);
Variant HHVM_FUNCTION(pg_options, CResRef connection);
Variant HHVM_FUNCTION(pg_parameter_status, CResRef connection, CStrRef param_name);
Variant HHVM_FUNCTION(pg_pconnect, CStrRef connection_string, int64_t connect_type = 0);
bool    HHVM_FUNCTION(pg_ping, CResRef connection);
Variant HHVM_FUNCTION(pg_port, CResRef connection);
Variant HHVM_FUNCTION(pg_prepare, CResRef connection, CStrRef stmtname, CStrRef query);
bool    HHVM_FUNCTION(pg_put_line, CResRef connection, CStrRef data);
Variant HHVM_FUNCTION(pg_query_params, CResRef connection, CStrRef query, CArrRef params);
Variant HHVM_FUNCTION(pg_query, CResRef connection, CStrRef query);
Variant HHVM_FUNCTION(pg_result_error_field, CResRef result, int64_t fieldcode);
Variant HHVM_FUNCTION(pg_result_error, CResRef result);
bool    HHVM_FUNCTION(pg_result_seek, CResRef result, int64_t offset);
Variant HHVM_FUNCTION(pg_result_status, CResRef result, int64_t type = PGSQL_STATUS_LONG);
bool    HHVM_FUNCTION(pg_send_execute, CResRef connection, CStrRef stmtname, CArrRef params);
bool    HHVM_FUNCTION(pg_send_prepare, CResRef connection, CStrRef stmtname, CStrRef query = null_string);
bool    HHVM_FUNCTION(pg_send_query_params, CResRef connection, CStrRef query, CArrRef params);
bool    HHVM_FUNCTION(pg_send_query, CResRef connection, CStrRef query);
int64_t HHVM_FUNCTION(pg_set_client_encoding, CResRef connection, CStrRef encoding);
bool    HHVM_FUNCTION(pg_trace, CStrRef pathname, CStrRef mode, CResRef connection);
int64_t HHVM_FUNCTION(pg_transaction_status, CResRef connection);
String  HHVM_FUNCTION(pg_unescape_bytea, CStrRef data);
bool    HHVM_FUNCTION(pg_untrace, CResRef connection);
Variant HHVM_FUNCTION(pg_version, CResRef connection);

///////////////////////////////////////////////////////////////////////////////
}

#endif
#endif // incl_EXT_PGSQL_H_
