#include "ext_pgsql.h"

#include "hphp/runtime/base/runtime-option.h"
#include "hphp/runtime/server/server-stats.h"
#include "hphp/runtime/ext/ext_string.h"

namespace HPHP {
#ifdef HAVE_PGSQL

bool PGSQL::AllowPersistent     = true;
int  PGSQL::MaxPersistent       = -1;
int  PGSQL::MaxLinks            = -1;
bool PGSQL::AutoResetPersistent = false;
bool PGSQL::IgnoreNotice        = false;
bool PGSQL::LogNotice           = false;

static class pgsqlExtension : public Extension {
public:
    pgsqlExtension() : Extension("pgsql") {}

    virtual void moduleLoad(Hdf hdf)
    {

        HHVM_FE(pg_affected_rows);
        HHVM_FE(pg_cancel_query);
        HHVM_FE(pg_client_encoding);
        HHVM_FE(pg_close);
        HHVM_FE(pg_connect);
        HHVM_FE(pg_async_connect);
        HHVM_FE(pg_connection_busy);
        HHVM_FE(pg_connection_reset);
        HHVM_FE(pg_connection_status);
        HHVM_FE(pg_convert);
        HHVM_FE(pg_copy_from);
        HHVM_FE(pg_copy_to);
        HHVM_FE(pg_dbname);
        HHVM_FE(pg_end_copy);
        HHVM_FE(pg_escape_bytea);
        HHVM_FE(pg_escape_identifier);
        HHVM_FE(pg_escape_literal);
        HHVM_FE(pg_escape_string);
        HHVM_FE(pg_execute);
        HHVM_FE(pg_fetch_all_columns);
        HHVM_FE(pg_fetch_all);
        HHVM_FE(pg_fetch_array);
        HHVM_FE(pg_fetch_assoc);
        HHVM_FE(pg_fetch_result);
        HHVM_FE(pg_fetch_row);
        HHVM_FE(pg_field_is_null);
        HHVM_FE(pg_field_name);
        HHVM_FE(pg_field_num);
        HHVM_FE(pg_field_prtlen);
        HHVM_FE(pg_field_size);
        HHVM_FE(pg_field_table);
        HHVM_FE(pg_field_type_oid);
        HHVM_FE(pg_field_type);
        HHVM_FE(pg_free_result);
        HHVM_FE(pg_get_notify);
        HHVM_FE(pg_get_pid);
        HHVM_FE(pg_get_result);
        HHVM_FE(pg_host);
        HHVM_FE(pg_last_error);
        HHVM_FE(pg_last_notice);
        HHVM_FE(pg_last_oid);
        HHVM_FE(pg_meta_data);
        HHVM_FE(pg_num_fields);
        HHVM_FE(pg_num_rows);
        HHVM_FE(pg_options);
        HHVM_FE(pg_parameter_status);
        HHVM_FE(pg_pconnect);
        HHVM_FE(pg_ping);
        HHVM_FE(pg_port);
        HHVM_FE(pg_prepare);
        HHVM_FE(pg_put_line);
        HHVM_FE(pg_query_params);
        HHVM_FE(pg_query);
        HHVM_FE(pg_result_error_field);
        HHVM_FE(pg_result_error);
        HHVM_FE(pg_result_seek);
        HHVM_FE(pg_result_status);
        HHVM_FE(pg_send_execute);
        HHVM_FE(pg_send_prepare);
        HHVM_FE(pg_send_query_params);
        HHVM_FE(pg_send_query);
        HHVM_FE(pg_set_client_encoding);
        HHVM_FE(pg_trace);
        HHVM_FE(pg_transaction_status);
        HHVM_FE(pg_unescape_bytea);
        HHVM_FE(pg_untrace);
        HHVM_FE(pg_version);

        Hdf pgsql = hdf["PGSQL"];

        PGSQL::AllowPersistent       = pgsql["AllowPersistent"].getBool(true);
        PGSQL::MaxPersistent         = pgsql["MaxPersistent"].getInt32(-1);
        PGSQL::MaxLinks              = pgsql["MaxLinks"].getInt32(-1);
        PGSQL::AutoResetPersistent   = pgsql["AutoResetPersistent"].getBool();
        PGSQL::IgnoreNotice          = pgsql["IgnoreNotice"].getBool();
        PGSQL::LogNotice             = pgsql["LogNotice"].getBool();

    }

    virtual void moduleInit() {
#define C(name, value) Native::registerConstant<KindOfInt64>(makeStaticString("PGSQL_" #name), (value))
        // Register constants

        C(ASSOC, PGSQL_ASSOC);
        C(NUM, PGSQL_NUM);
        C(BOTH, PGSQL_BOTH);

        C(CONNECT_FORCE_NEW, 1);
        C(CONNECTION_BAD, CONNECTION_BAD);
        C(CONNECTION_OK, CONNECTION_OK);
        C(CONNECTION_STARTED, CONNECTION_STARTED);
        C(CONNECTION_MADE, CONNECTION_MADE);
        C(CONNECTION_AWAITING_RESPONSE, CONNECTION_AWAITING_RESPONSE);
        C(CONNECTION_AUTH_OK, CONNECTION_AUTH_OK);
        C(CONNECTION_SETENV, CONNECTION_SETENV);
        C(CONNECTION_SSL_STARTUP, CONNECTION_SSL_STARTUP);

        C(SEEK_SET, SEEK_SET);
        C(SEEK_CUR, SEEK_CUR);
        C(SEEK_END, SEEK_END);

        C(EMPTY_QUERY, PGRES_EMPTY_QUERY);
        C(COMMAND_OK, PGRES_COMMAND_OK);
        C(TUPLES_OK, PGRES_TUPLES_OK);
        C(COPY_OUT, PGRES_COPY_OUT);
        C(COPY_IN, PGRES_COPY_IN);
        C(BAD_RESPONSE, PGRES_BAD_RESPONSE);
        C(NONFATAL_ERROR, PGRES_NONFATAL_ERROR);
        C(FATAL_ERROR, PGRES_FATAL_ERROR);

        C(TRANSACTION_IDLE, PQTRANS_IDLE);
        C(TRANSACTION_ACTIVE, PQTRANS_ACTIVE);
        C(TRANSACTION_INTRANS, PQTRANS_INTRANS);
        C(TRANSACTION_INERROR, PQTRANS_INERROR);
        C(TRANSACTION_UNKNOWN, PQTRANS_UNKNOWN);

        C(DIAG_SEVERITY, PG_DIAG_SEVERITY);
        C(DIAG_SQLSTATE, PG_DIAG_SQLSTATE);
        C(DIAG_MESSAGE_PRIMARY, PG_DIAG_MESSAGE_PRIMARY);
        C(DIAG_MESSAGE_DETAIL, PG_DIAG_MESSAGE_DETAIL);
        C(DIAG_MESSAGE_HINT, PG_DIAG_MESSAGE_HINT);
        C(DIAG_STATEMENT_POSITION, PG_DIAG_STATEMENT_POSITION);
        C(DIAG_INTERNAL_POSITION, PG_DIAG_INTERNAL_POSITION);
        C(DIAG_INTERNAL_QUERY, PG_DIAG_INTERNAL_QUERY);
        C(DIAG_CONTEXT, PG_DIAG_CONTEXT);
        C(DIAG_SOURCE_FILE, PG_DIAG_SOURCE_FILE);
        C(DIAG_SOURCE_LINE, PG_DIAG_SOURCE_LINE);
        C(DIAG_SOURCE_FUNCTION, PG_DIAG_SOURCE_FUNCTION);

        C(ERRORS_TERSE, PQERRORS_TERSE);
        C(ERRORS_DEFAULT, PQERRORS_DEFAULT);
        C(ERRORS_VERBOSE, PQERRORS_VERBOSE);

        C(STATUS_LONG, PGSQL_STATUS_LONG);
        C(STATUS_STRING, PGSQL_STATUS_STRING);

        C(CONV_IGNORE_DEFAULT, 1);
        C(CONV_FORCE_NULL, 2);
        C(CONV_IGNORE_NOT_NULL, 4);

#undef C
    }
} s_pgsql_ext;

//////////////////////////////////////////////////////////////////////////////////

StaticString PGSQL::s_class_name("pgsql connection");
StaticString PGSQLResult::s_class_name("pgsql result");

PGSQL *PGSQL::Get(CVarRef conn_id) {
    if (conn_id.isNull()) {
        return NULL;
    }

    PGSQL *pgsql = conn_id.toResource().getTyped<PGSQL>
        (!RuntimeOption::ThrowBadTypeExceptions,
         !RuntimeOption::ThrowBadTypeExceptions);
    return pgsql;
}

PGconn *PGSQL::GetConn(CVarRef conn_id, PGSQL **rconn /* = NULL */) {
    PGSQL *pgsql = Get(conn_id);
    PGconn *ret = NULL;
    if (pgsql) {
        ret = pgsql->get();
    }
    if (ret == NULL) {
        raise_warning("supplied argument is not a valid Postgres connection resource");
    }
    if (rconn) {
        *rconn = pgsql;
    }

    return ret;
}

static void notice_processor(void *arg, const char *message) {
    if (arg != NULL) {
        auto pgsql = static_cast<PGSQL *>(arg);

        pgsql->m_last_notice = message;

        if (PGSQL::LogNotice) {
            raise_notice("%s", message);
        }
    }
}

PGSQL::PGSQL(String conninfo, bool async)
    : m_conn_string(conninfo.data()), m_last_notice("") {

    if (RuntimeOption::EnableStats && RuntimeOption::EnableSQLStats) {
        ServerStats::Log("sql.conn", 1);
    }

    if (async) {
        m_conn = PQconnectStart(conninfo.data());
    } else {
        m_conn = PQconnectdb(conninfo.data());

        ConnStatusType st = PQstatus(m_conn);
        if (m_conn && st == CONNECTION_OK) {
            // Load up the fixed information
            m_db = PQdb(m_conn);
            m_user = PQuser(m_conn);
            m_pass = PQpass(m_conn);
            m_host = PQhost(m_conn);
            m_port = PQport(m_conn);
            m_options = PQoptions(m_conn);
        } else if (m_conn) {
            raise_warning("pg_connect(): Connection failed: %s", PQerrorMessage(m_conn));
            PQfinish(m_conn);
            m_conn = NULL;
        }
    }

    if (m_conn) {
        if (!PGSQL::IgnoreNotice) {
            PQsetNoticeProcessor(m_conn, notice_processor, static_cast<void *>(this));
        } else {
            PQsetNoticeProcessor(m_conn, notice_processor, NULL);
        }
    }
}

void PGSQL::close() {
    if (m_conn != NULL) {
        PQfinish(m_conn);
        m_conn = NULL;
    }
}

PGSQL::~PGSQL() {
    close();
}

PGSQLResult *PGSQLResult::Get(CVarRef result) {
    if (result.isNull()) {
        return NULL;
    }

    PGSQLResult *res = result.toResource().getTyped<PGSQLResult>
        (!RuntimeOption::ThrowBadTypeExceptions,
         !RuntimeOption::ThrowBadTypeExceptions);
    return res;
}

PGresult *PGSQLResult::GetResult(CVarRef result, PGSQLResult **rres) {
    PGSQLResult *res = Get(result);
    PGresult *ret = NULL;
    if (res) {
        ret = res->get();
    }
    if (ret == NULL) {
        raise_warning("supplied argument is not a valid Postgres result resource");
    }
    if (rres) {
        *rres = res;
    }

    return ret;
}

PGSQLResult::PGSQLResult(PGSQL * conn, PGresult * res)
    : m_current_row(0), m_res(res),
      m_num_fields(-1), m_num_rows(-1), m_conn(conn) {
    m_conn->incRefCount();
}

void PGSQLResult::close() {
    if (m_res) {
        PQclear(m_res);
        m_res = NULL;
    }
}

PGSQLResult::~PGSQLResult() {
    m_conn->decRefCount();
    close();
}

int PGSQLResult::getFieldNumber(CVarRef field) {
    int n;
    if (field.isNumeric(true)) {
        n = field.toInt32();
    } else if (field.isString()){
        n = PQfnumber(m_res, field.asCStrRef().data());
    } else {
        n = -1;
    }

    return n;
}

int PGSQLResult::getNumFields() {
    if (m_num_fields == -1) {
        m_num_fields = PQnfields(m_res);
    }
    return m_num_fields;
}

int PGSQLResult::getNumRows() {
    if (m_num_rows == -1) {
        m_num_rows = PQntuples(m_res);
    }
    return m_num_rows;
}

bool PGSQLResult::convertFieldRow(CVarRef row, CVarRef field,
        int *out_row, int *out_field, const char *fn_name) {

    Variant actual_field;
    int actual_row;

    assert(out_row && out_field && "Output parameters cannot be null");

    if (fn_name == NULL) {
        fn_name = "__internal_pgsql_func";
    }

    if (field.isInitialized()) {
        actual_row = row.toInt64();
        actual_field = field;
    } else {
        actual_row = m_current_row;
        actual_field = row;
    }

    int field_number = getFieldNumber(actual_field);

    if (field_number < 0 || field_number >= getNumFields()) {
        if (actual_field.isString()) {
            raise_warning("%s(): Unknown column name \"%s\"",
                    fn_name, actual_field.asCStrRef().data());
        } else {
            raise_warning("%s(): Column offset `%d` out of range", fn_name, field_number);
        }
        return false;
    }

    if (actual_row < 0 || actual_row >= getNumRows()) {
        raise_warning("%s(): Row `%d` out of range", fn_name, actual_row);
        return false;
    }

    *out_row = actual_row;
    *out_field = field_number;

    return true;
}

Variant PGSQLResult::fieldIsNull(CVarRef row, CVarRef field, const char *fn_name) {
    int r, f;
    if (convertFieldRow(row, field, &r, &f, fn_name)) {
        return PQgetisnull(m_res, r, f);
    }

    return false;
}

Variant PGSQLResult::getFieldVal(CVarRef row, CVarRef field, const char *fn_name) {
    int r, f;
    if (convertFieldRow(row, field, &r, &f, fn_name)) {
        return getFieldVal(r, f, fn_name);
    }

    return false;
}

String PGSQLResult::getFieldVal(int row, int field, const char *fn_name) {
    if (PQgetisnull(m_res, row, field) == 1) {
        return null_string;
    } else {
        char * value = PQgetvalue(m_res, row, field);
        int length = PQgetlength(m_res, row, field);

        return String(value, length, CopyString);
    }
}

// Simple wrapper class to convert a CArrRef to a
// list of C strings to pass to pgsql functions. Needs
// to be like this because string conversion may-or-may
// not allocate and therefore needs to ensure that the
// underlying data lasts long enough.
class CStringArray {
public:
    std::vector<String> m_strings;
    std::vector<const char *> m_c_strs;

    CStringArray(CArrRef arr) {
        int size = arr.size();

        m_strings.reserve(size);

        for(int i = 0; i < size; i++) {
            Variant param = arr[i];
            if (param.isNull()) {
                m_strings.push_back(null_string);
            } else {
                m_strings.push_back(param.toString());
            }
        }

        m_c_strs.reserve(size);
        for (int i = 0; i < size; i++) {
            if (m_strings[i].isNull()) {
                m_c_strs.push_back(NULL);
            } else {
                m_c_strs.push_back(m_strings[i].data());
            }
        }
    }

    const char * const *data() {
        return m_c_strs.data();
    }

};

//////////////////// Connection functions /////////////////////////

Variant HHVM_FUNCTION(pg_connect, CStrRef connection_string, int connect_type /* = 0 */) {
    PGSQL * pgsql = NULL;

    pgsql = new PGSQL(connection_string, false);

    if (pgsql->get() == NULL) {
        delete pgsql;
        return false;
    }
    return Resource(pgsql);
}

Variant HHVM_FUNCTION(pg_async_connect, CStrRef connection_string, int connect_type /* = 0 */) {
    PGSQL * pgsql = NULL;

    pgsql = new PGSQL(connection_string, true);

    if (pgsql->get() == NULL) {
        delete pgsql;
        return false;
    }

    return Resource(pgsql);
}

bool HHVM_FUNCTION(pg_close, CResRef connection) {
    PGSQL * pgsql = PGSQL::Get(connection);
    pgsql->close();
    return true;
}

bool HHVM_FUNCTION(pg_ping, CResRef connection) {
    PGSQL * pgsql = PGSQL::Get(connection);

    if (pgsql->get() == NULL) {
        return false;
    }

    PGPing response = PQping(pgsql->m_conn_string.data());

    if (response == PQPING_OK) {
        if (PQstatus(pgsql->get()) == CONNECTION_BAD) {
            PQreset(pgsql->get());
            return PQstatus(pgsql->get()) != CONNECTION_BAD;
        } else {
            return true;
        }
    }

    return false;
}

bool HHVM_FUNCTION(pg_connection_reset, CResRef connection) {
    PGSQL * pgsql = PGSQL::Get(connection);

    if (pgsql->get() == NULL) {
        return false;
    }

    PQreset(pgsql->get());

    return PQstatus(pgsql->get()) != CONNECTION_BAD;
}

///////////// Interrogation Functions ////////////////////

int64_t HHVM_FUNCTION(pg_connection_status, CResRef connection) {
    PGconn *conn = PGSQL::GetConn(connection);
    if (!conn) return CONNECTION_BAD;
    return (int64_t)PQstatus(conn);
}

bool HHVM_FUNCTION(pg_connection_busy, CResRef connection) {
    PGSQL * pgsql = PGSQL::Get(connection);
    if (pgsql == NULL) {
        return false;
    }

    bool mode = pgsql->isNonBlocking();
    pgsql->setNonBlocking();

    PQconsumeInput(pgsql->get());

    bool ret = (bool)PQisBusy(pgsql->get());

    pgsql->setNonBlocking(mode);

    return ret;
}

Variant HHVM_FUNCTION(pg_dbname, CResRef connection) {
    PGSQL * pgsql = PGSQL::Get(connection);

    if (pgsql == NULL) {
        return false;
    }

    if (pgsql->m_db.empty()) {
        const char * db = PQdb(pgsql->get());
        if (db == NULL) {
            return false;
        } else {
            pgsql->m_db = db;
        }
    }

    return pgsql->m_db;
}

Variant HHVM_FUNCTION(pg_host, CResRef connection) {
    PGSQL * pgsql = PGSQL::Get(connection);

    if (pgsql == NULL) {
        return false;
    }

    if (pgsql->m_host.empty()) {
        const char * host = PQhost(pgsql->get());
        if (host == NULL) {
            return false;
        } else {
            pgsql->m_host = host;
        }
    }

    return pgsql->m_host;
}

Variant HHVM_FUNCTION(pg_port, CResRef connection) {
    PGSQL * pgsql = PGSQL::Get(connection);

    if (pgsql == NULL) {
        return false;
    }

    if (pgsql->m_port.empty()) {
        const char * port = PQport(pgsql->get());
        if (port == NULL) {
            return false;
        } else {
            pgsql->m_port = port;
        }
    }

    String ret = pgsql->m_port;
    if (ret.isNumeric()) {
        return ret.toInt32();
    } else {
        return ret;
    }
}

Variant HHVM_FUNCTION(pg_options, CResRef connection) {
    PGSQL * pgsql = PGSQL::Get(connection);

    if (pgsql == NULL) {
        return false;
    }

    if (pgsql->m_options.empty()) {
        const char * options = PQoptions(pgsql->get());
        if (options == NULL) {
            return false;
        } else {
            pgsql->m_options = options;
        }
    }

    return pgsql->m_options;
}

Variant HHVM_FUNCTION(pg_parameter_status, CResRef connection, CStrRef param_name) {
    PGSQL * pgsql = PGSQL::Get(connection);

    if (pgsql == NULL) {
        return false;
    }

    String ret(PQparameterStatus(pgsql->get(), param_name.data()), CopyString);

    return ret;
}

Variant HHVM_FUNCTION(pg_client_encoding, CResRef connection) {
    PGSQL * pgsql = PGSQL::Get(connection);

    if (pgsql == NULL) {
        return false;
    }

    int enc = PQclientEncoding(pgsql->get());

    String ret(pg_encoding_to_char(enc), CopyString);

    return ret;
}

int64_t HHVM_FUNCTION(pg_transaction_status, CResRef connection) {
    PGSQL * pgsql = PGSQL::Get(connection);

    if (pgsql == NULL) {
        return PQTRANS_UNKNOWN;
    }

    return (int64_t)PQtransactionStatus(pgsql->get());
}

Variant HHVM_FUNCTION(pg_last_error, CResRef connection) {
    PGSQL * pgsql = PGSQL::Get(connection);
    if (pgsql == NULL) {
        return false;
    }

    String ret(PQerrorMessage(pgsql->get()), CopyString);

    return f_trim(ret);
}

Variant HHVM_FUNCTION(pg_last_notice, CResRef connection) {
    PGSQL * pgsql = PGSQL::Get(connection);
    if (pgsql == NULL) {
        return false;
    }

    return pgsql->m_last_notice;
}

Variant HHVM_FUNCTION(pg_version, CResRef connection) {
    static StaticString client_key("client");
    static StaticString protocol_key("protocol");
    static StaticString server_key("server");

    PGSQL * pgsql = PGSQL::Get(connection);
    if (pgsql == NULL) {
        return false;
    }

    Array ret;

    int proto_ver = PQprotocolVersion(pgsql->get());
    if (proto_ver) {
        ret.set(protocol_key, String(proto_ver) + ".0");
    }

    int server_ver = PQserverVersion(pgsql->get());
    if (server_ver) {
        int revision = server_ver % 100;
        int minor = (server_ver / 100) % 100;
        int major = server_ver / 10000;

        ret.set(server_key, String(major) + "." + String(minor) + "." + String(revision));
    }

    int client_ver = PQlibVersion();
    if (client_ver) {
        int revision = client_ver % 100;
        int minor = (client_ver / 100) % 100;
        int major = client_ver / 10000;

        ret.set(client_key, String(major) + "." + String(minor) + "." + String(revision));
    }

    return ret;
}

int64_t HHVM_FUNCTION(pg_get_pid, CResRef connection) {
    PGSQL * pgsql = PGSQL::Get(connection);
    if (pgsql == NULL) {
        return -1;
    }

    return (int64_t)PQbackendPID(pgsql->get());
}

//////////////// Escaping Functions ///////////////////////////

String HHVM_FUNCTION(pg_escape_bytea, CResRef connection, CStrRef data) {
    PGSQL * pgsql = PGSQL::Get(connection);
    if (pgsql == NULL) {
        return null_string;
    }

    size_t escape_size = 0;
    char * escaped = (char *)PQescapeByteaConn(pgsql->get(),
            (unsigned char *)data.data(), data.size(), &escape_size);

    if (escaped == NULL) {
        raise_warning("pg_escape_bytea(): %s", PQerrorMessage(pgsql->get()));
        return null_string;
    }

    String ret(escaped, escape_size, CopyString);

    PQfreemem(escaped);

    return ret;
}

String HHVM_FUNCTION(pg_escape_identifier, CResRef connection, CStrRef data) {
    PGSQL * pgsql = PGSQL::Get(connection);
    if (pgsql == NULL) {
        return null_string;
    }

    char * escaped = (char *)PQescapeIdentifier(pgsql->get(), data.data(), data.size());

    if (escaped == NULL) {
        raise_warning("pg_escape_identifier(): %s", PQerrorMessage(pgsql->get()));
        return null_string;
    }

    String ret(escaped, CopyString);

    PQfreemem(escaped);

    return ret;
}

String HHVM_FUNCTION(pg_escape_literal, CResRef connection, CStrRef data) {
    PGSQL * pgsql = PGSQL::Get(connection);
    if (pgsql == NULL) {
        return null_string;
    }

    char * escaped = (char *)PQescapeLiteral(pgsql->get(), data.data(), data.size());

    if (escaped == NULL) {
        raise_warning("pg_escape_literal(): %s", PQerrorMessage(pgsql->get()));
        return null_string;
    }

    String ret(escaped, CopyString);

    PQfreemem(escaped);

    return ret;
}

String HHVM_FUNCTION(pg_escape_string, CResRef connection, CStrRef data) {
    PGSQL * pgsql = PGSQL::Get(connection);
    if (pgsql == NULL) {
        return null_string;
    }

    String ret((data.size()*2)+1, ReserveString); // Reserve enough space as defined by PQescapeStringConn

    int error = 0;
    size_t size = PQescapeStringConn(pgsql->get(),
            ret.get()->mutableData(), data.data(), data.size(),
            &error);

    if (error) {
        return null_string;
    }

    ret.shrink(size); // Shrink to the returned size, `shrink` may re-alloc and free up space

    return ret;
}

String HHVM_FUNCTION(pg_unescape_bytea, CStrRef data) {
    size_t to_len = 0;
    char * unesc = (char *)PQunescapeBytea((unsigned char *)data.data(), &to_len);
    String ret = String(unesc, to_len, CopyString);
    PQfreemem(unesc);
    return ret;
}

///////////// Command Execution / Querying /////////////////////////////

int64_t HHVM_FUNCTION(pg_affected_rows, CResRef result) {
    PGresult *res = PGSQLResult::GetResult(result);
    return (int64_t)PQcmdTuples(res);
}

Variant HHVM_FUNCTION(pg_result_status, CResRef result, int64_t type /* = PGSQL_STATUS_LONG */) {
    PGresult *res = PGSQLResult::GetResult(result);

    if (type == PGSQL_STATUS_LONG) {
        return (int64_t)PQresultStatus(res);
    } else {
        String ret(PQcmdStatus(res), CopyString);
        return ret;
    }
}

bool HHVM_FUNCTION(pg_free_result, CResRef result) {
    PGSQLResult *res = PGSQLResult::Get(result);
    if (res) {
        res->close();
        return true;
    } else {
        return false;
    }
}

Variant HHVM_FUNCTION(pg_query, CResRef connection, CStrRef query) {
    PGSQL *conn = PGSQL::Get(connection);
    if (conn == NULL) {
        return false;
    }

    PGresult *res = PQexec(conn->get(), query.data());

    if (res == NULL) {
        char * err = PQerrorMessage(conn->get());
        raise_warning("pg_query(): Query Failed: %s", err);
        return false;
    } else {
        int st = PQresultStatus(res);
        switch (st) {
            default:
                break;
            case PGRES_EMPTY_QUERY:
            case PGRES_BAD_RESPONSE:
            case PGRES_NONFATAL_ERROR:
            case PGRES_FATAL_ERROR:
                char * msg = PQresultErrorMessage(res);
                raise_warning("pg_query(): Query Failed: %s", msg);
                PQclear(res);
                return false;
        }

    }

    PGSQLResult *pgresult = new PGSQLResult(conn, res);

    return Resource(pgresult);
}

Variant HHVM_FUNCTION(pg_query_params, CResRef connection, CStrRef query, CArrRef params) {
    PGSQL *conn = PGSQL::Get(connection);
    if (conn == NULL) {
        return false;
    }

    CStringArray str_array(params);

    PGresult * res = PQexecParams(conn->get(), query.data(),
            params.size(), NULL, str_array.data(), NULL, NULL, 0);

    if (res == NULL) {
        char * err = PQerrorMessage(conn->get());
        raise_warning("pg_query_params(): Query Failed: %s", err);
        return false;
    } else {
        int st = PQresultStatus(res);
        switch (st) {
            default:
                break;
            case PGRES_EMPTY_QUERY:
            case PGRES_BAD_RESPONSE:
            case PGRES_NONFATAL_ERROR:
            case PGRES_FATAL_ERROR:
                char * msg = PQresultErrorMessage(res);
                raise_warning("pg_query_params(): Query Failed: %s", msg);
                PQclear(res);
                return false;
        }

    }

    PGSQLResult *pgresult = new PGSQLResult(conn, res);

    return Resource(pgresult);
}

Variant HHVM_FUNCTION(pg_prepare, CResRef connection, CStrRef stmtname, CStrRef query) {
    PGSQL *conn = PGSQL::Get(connection);
    if (conn == NULL) {
        return false;
    }

    PGresult *res = PQprepare(conn->get(), stmtname.data(), query.data(), 0, NULL);

    if (res == NULL) {
        char * err = PQerrorMessage(conn->get());
        raise_warning("pg_prepare(): Query Failed: %s", err);
        return false;
    } else {
        int st = PQresultStatus(res);
        switch (st) {
            default:
                break;
            case PGRES_EMPTY_QUERY:
            case PGRES_BAD_RESPONSE:
            case PGRES_NONFATAL_ERROR:
            case PGRES_FATAL_ERROR:
                char * msg = PQresultErrorMessage(res);
                raise_warning("pg_prepare(): Query Failed: %s", msg);
                PQclear(res);
                return false;
        }

    }

    PGSQLResult *pgres = new PGSQLResult(conn, res);

    return Resource(pgres);
}

Variant HHVM_FUNCTION(pg_execute, CResRef connection, CStrRef stmtname, CArrRef params) {
    PGSQL *conn = PGSQL::Get(connection);
    if (conn == NULL) {
        return false;
    }

    CStringArray str_array(params);

    PGresult *res = PQexecPrepared(conn->get(), stmtname.data(), params.size(),
            str_array.data(), NULL, NULL, 0);

    PGSQLResult *pgres = new PGSQLResult(conn, res);

    return Resource(pgres);
}

bool HHVM_FUNCTION(pg_send_query, CResRef connection, CStrRef query) {
    PGSQL *conn = PGSQL::Get(connection);
    if (conn == NULL) {
        return false;
    }

    bool origBlock = conn->isNonBlocking();
    conn->setNonBlocking();

    PGresult *res;
    bool empty = true;
    while ((res = PQgetResult(conn->get()))) {
        PQclear(res);
        empty = false;
    }
    if (!empty) {
        raise_notice("There are results on this connection."
                     " Call pg_get_result() until it returns FALSE");
    }

    if (PQsendQuery(conn->get(), query.data()) == 0) {
        // TODO: Do persistent auto-reconnect
        return false;
    }

    int ret;
    while ((ret = PQflush(conn->get()))) {
        if (ret == -1) {
            raise_notice("Could not empty PostgreSQL send buffer");
            break;
        }
        usleep(5000);
    }

    conn->setNonBlocking(origBlock);

    return true;
}

Variant HHVM_FUNCTION(pg_get_result, CResRef connection) {
    PGSQL *conn = PGSQL::Get(connection);
    if (conn == NULL) {
        return false;
    }

    PGresult *res = PQgetResult(conn->get());

    if (res == NULL) {
        return false;
    }

    PGSQLResult *pgresult = new PGSQLResult(conn, res);

    return Resource(pgresult);
}

bool HHVM_FUNCTION(pg_send_query_params, CResRef connection, CStrRef query, CArrRef params) {
    PGSQL *conn = PGSQL::Get(connection);
    if (conn == NULL) {
        return false;
    }

    bool origBlock = conn->isNonBlocking();
    conn->setNonBlocking();

    PGresult *res;
    bool empty = true;
    while ((res = PQgetResult(conn->get()))) {
        PQclear(res);
        empty = false;
    }
    if (!empty) {
        raise_notice("There are results on this connection."
                     " Call pg_get_result() until it returns FALSE");
    }

    CStringArray str_array(params);

    if (PQsendQueryParams(conn->get(), query.data(),
            params.size(), NULL, str_array.data(), NULL, NULL, 0) == 0) {
        return false;
    }

    int ret;
    while ((ret = PQflush(conn->get()))) {
        if (ret == -1) {
            raise_notice("Could not empty PostgreSQL send buffer");
            break;
        }
        usleep(5000);
    }

    conn->setNonBlocking(origBlock);

    return true;
}

bool HHVM_FUNCTION(pg_send_prepare, CResRef connection, CStrRef stmtname, CStrRef query) {
    PGSQL *conn = PGSQL::Get(connection);
    if (conn == NULL) {
        return false;
    }

    return (bool)PQsendPrepare(conn->get(), stmtname.data(), query.data(), 0, NULL);
}

bool HHVM_FUNCTION(pg_send_execute, CResRef connection, CStrRef stmtname, CArrRef params) {
    PGSQL *conn = PGSQL::Get(connection);
    if (conn == NULL) {
        return false;
    }

    CStringArray str_array(params);

    return (bool)PQsendQueryPrepared(conn->get(), stmtname.data(),
            params.size(), str_array.data(), NULL, NULL, 0);
}

bool HHVM_FUNCTION(pg_cancel_query, CResRef connection) {
    PGSQL *conn = PGSQL::Get(connection);
    if (conn == NULL) {
        return false;
    }

    bool origBlock = conn->isNonBlocking();
    conn->setNonBlocking();

    bool ret = PQrequestCancel(conn->get());

    PGresult *res = NULL;
    while((res = PQgetResult(conn->get()))) {
        PQclear(res);
    }

    conn->setNonBlocking(origBlock);

    return ret;
}

////////////////////////

Variant HHVM_FUNCTION(pg_fetch_all_columns, CResRef result, int64_t column /* = 0 */) {
    PGSQLResult *res = PGSQLResult::Get(result);
    if (res == NULL) {
        return false;
    }

    if (column < 0 || column >= res->getNumFields()) {
        raise_warning("pg_fetch_all_columns(): Column offset `%d` out of range", (int)column);
        return false;
    }

    int num_rows = res->getNumRows();

    Array arr;
    for (int i = 0; i < num_rows; i++) {
        Variant field = res->getFieldVal(i, column);
        arr.set(i, field);
    }

    return arr;
}

Variant HHVM_FUNCTION(pg_fetch_all, CResRef result) {
    PGSQLResult *res = PGSQLResult::Get(result);
    if (res == NULL) {
        return false;
    }

    int num_rows = res->getNumRows();

    Array rows;
    for (int i = 0; i < num_rows; i++) {
        Variant row = f_pg_fetch_assoc(result, i);
        rows.set(i, row);
    }

    return rows;
}

Variant HHVM_FUNCTION(pg_fetch_array, CResRef result, CVarRef row /* = null_variant */, int64_t result_type /* = PGSQL_BOTH */) {
    PGSQLResult *res = PGSQLResult::Get(result);
    if (res == NULL) {
        return false;
    }

    int r;
    if (row.isNull()) {
        r = res->m_current_row;
        if (r >= res->getNumRows()) {
            return false;
        }
        res->m_current_row++;
    } else {
        r = row.toInt32();
    }

    if (r < 0 || r >= res->getNumRows()) {
        raise_warning("Row `%d` out of range", r);
        return false;
    }

    Array arr;

    for (int i = 0; i < res->getNumFields(); i++) {
        Variant field = res->getFieldVal(r, i);
        if (result_type & PGSQL_NUM) {
            arr.set(i, field);
        }
        if (result_type & PGSQL_ASSOC) {
            char * name = PQfname(res->get(), i);
            String fn(name, CopyString);
            arr.set(fn, field);
        }
    }

    return arr;
}

Variant HHVM_FUNCTION(pg_fetch_assoc, CResRef result, CVarRef row /* = null_variant */) {
    return f_pg_fetch_array(result, row, PGSQL_ASSOC);
}

Variant HHVM_FUNCTION(pg_fetch_result, CResRef result, CVarRef row /* = null_variant */, CVarRef field /* = null_variant */) {
    PGSQLResult *res = PGSQLResult::Get(result);
    if (res == NULL) {
        return false;
    }

    return res->getFieldVal(row, field, "pg_fetch_result");
}

Variant HHVM_FUNCTION(pg_fetch_row, CResRef result, CVarRef row /* = null_variant */) {
    return f_pg_fetch_array(result, row, PGSQL_NUM);
}

///////////////////// Field information //////////////////////////

Variant HHVM_FUNCTION(pg_field_is_null, CResRef result, CVarRef row, CVarRef field /* = null_variant */) {
    PGSQLResult *res = PGSQLResult::Get(result);
    if (res == NULL) {
        return false;
    }

    return res->fieldIsNull(row, field, "pg_field_is_null");
}

Variant HHVM_FUNCTION(pg_field_name, CResRef result, int64_t field_number) {
    PGSQLResult *res_obj;
    PGresult *res = PGSQLResult::GetResult(result, &res_obj);
    if (res == NULL) {
        return null_variant;
    }

    if (field_number < 0 || field_number >= res_obj->getNumFields()) {
        raise_warning("pg_field_name(): Column offset `%d` out of range", (int)field_number);
        return false;
    }

    char * name = PQfname(res, (int)field_number);
    if (name == NULL) {
        raise_warning("pg_field_name(): %s", PQresultErrorMessage(res));
        return false;
    } else {
        return String(name, CopyString);
    }
}

int64_t HHVM_FUNCTION(pg_field_num, CResRef result, CStrRef field_name) {
    PGresult *res = PGSQLResult::GetResult(result);
    if (res == NULL) {
        return -1;
    }

    return PQfnumber(res, field_name.data());
}

Variant HHVM_FUNCTION(pg_field_prtlen, CResRef result, CVarRef row_number, CVarRef field /* = null_variant */) {
    PGSQLResult *res = PGSQLResult::Get(result);
    if (res == NULL) {
        return false;
    }

    int r, f;
    if (res->convertFieldRow(row_number, field, &r, &f, "pg_field_prtlen")) {
        return PQgetlength(res->get(), r, f);
    } else {
        return false;
    }
}

Variant HHVM_FUNCTION(pg_field_size, CResRef result, int64_t field_number) {
    PGSQLResult *res = PGSQLResult::Get(result);
    if (res == NULL) {
        return false;
    }

    if (field_number < 0 || field_number > res->getNumFields()) {
        raise_warning("pg_field_size(): Column offset `%d` out of range", (int)field_number);
        return false;
    }

    return PQfsize(res->get(), field_number);
}

Variant HHVM_FUNCTION(pg_field_table, CResRef result, int64_t field_number, bool oid_only /* = false */) {
    PGSQLResult *res = PGSQLResult::Get(result);

    if (res == NULL) {
        return false;
    }

    if (field_number < 0 || field_number > res->getNumFields()) {
        raise_warning("pg_field_table(): Column offset `%d` out of range", (int)field_number);
        return false;
    }

    Oid id = PQftable(res->get(), field_number);
    if (id == InvalidOid) return false;

    if (oid_only) {
        return (int64_t)id;
    } else {
        // TODO: cache the Oids somewhere
        std::ostringstream query;
        query << "SELECT relname FROM pg_class WHERE oid=" << id;

        PGresult *name_res = PQexec(res->getConn()->get(), query.str().c_str());
        if (name_res && PQresultStatus(name_res) == PGRES_TUPLES_OK) {
            if (name_res) PQclear(name_res);
            return false;
        }

        char * name = PQgetvalue(name_res, 0, 0);
        if (name == NULL) {
            PQclear(name_res);
            return false;
        }

        String ret(name, CopyString);
        PQclear(name_res);

        return ret;
    }
}

Variant HHVM_FUNCTION(pg_field_type_oid, CResRef result, int64_t field_number) {
    PGSQLResult *res = PGSQLResult::Get(result);

    if (res == NULL) {
        return false;
    }

    if (field_number < 0 || field_number > res->getNumFields()) {
        raise_warning("pg_field_table(): Column offset `%d` out of range", (int)field_number);
        return false;
    }

    Oid id = PQftype(res->get(), field_number);
    if (id == InvalidOid) return false;
    return (int64_t)id;
}

int64_t HHVM_FUNCTION(pg_num_fields, CResRef result) {
    PGSQLResult *res = PGSQLResult::Get(result);
    if (res == NULL) {
        return -1;
    }

    return res->getNumFields();
}

int64_t HHVM_FUNCTION(pg_num_rows, CResRef result) {
    PGSQLResult *res = PGSQLResult::Get(result);
    if (res == NULL) {
        return -1;
    }

    return res->getNumRows();
}

Variant HHVM_FUNCTION(pg_result_error_field, CResRef result, int64_t fieldcode) {
    PGSQLResult *res = PGSQLResult::Get(result);
    if (res == NULL) {
        return false;
    }

    char * msg = PQresultErrorField(res->get(), fieldcode);
    if (msg) {
        return f_trim(String(msg, CopyString));
    }

    return false;
}

Variant HHVM_FUNCTION(pg_result_error, CResRef result) {
    PGSQLResult *res = PGSQLResult::Get(result);
    if (res == NULL) {
        return false;
    }

    char * msg = PQresultErrorMessage(res->get());
    if (msg) {
        return f_trim(String(msg, CopyString));
    }

    return false;
}

bool HHVM_FUNCTION(pg_result_seek, CResRef result, int64_t offset) {
    PGSQLResult *res = PGSQLResult::Get(result);
    if (res == NULL) {
        return false;
    }

    if (offset < 0 || offset > res->getNumRows()) {
        raise_warning("pg_result_seek(): Cannot seek to row %d", (int)offset);
        return false;
    }

    res->m_current_row = (int)offset;
    return true;
}

///////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
#endif
// Currently Unimplemented functions

Variant HHVM_FUNCTION(pg_field_type, CResRef result, int64_t field_number) {
  throw NotImplementedException("pg_field_type");
}

bool HHVM_FUNCTION(pg_end_copy, CResRef connection) {
  throw NotImplementedException("pg_end_copy");
}

Variant HHVM_FUNCTION(pg_get_notify, CResRef connection, int64_t result_type /* = PGSQL_BOTH */) {
  throw NotImplementedException("pg_get_notify");
}

Variant HHVM_FUNCTION(pg_last_oid, CResRef result) {
  throw NotImplementedException("pg_last_oid");
}

Variant HHVM_FUNCTION(pg_meta_data, CResRef connection, CStrRef table_name) {
  throw NotImplementedException("pg_meta_data");
}

int64_t HHVM_FUNCTION(pg_set_client_encoding, CResRef connection, CStrRef encoding) {
  throw NotImplementedException("pg_set_client_encoding");
}

bool HHVM_FUNCTION(pg_trace, CStrRef pathname, CStrRef mode /* = "w" */, CResRef connection) {
  throw NotImplementedException("pg_trace");
}

bool HHVM_FUNCTION(pg_untrace, CResRef connection) {
  throw NotImplementedException("pg_untrace");
}

Variant HHVM_FUNCTION(pg_pconnect, CStrRef connection_string, int64_t connect_type /* = 0 */) {
  throw NotImplementedException("pg_pconnect");
}

Variant HHVM_FUNCTION(pg_convert, CResRef connection, CStrRef table_name, CArrRef assoc_array, int options /* = 0 */) {
  throw NotImplementedException("pg_convert");
}

bool HHVM_FUNCTION(pg_copy_from, CResRef connection, CStrRef table_name, CArrRef rows, CStrRef delimiter /* = "\t" */, CStrRef null_as /* = "\n" */) {
  throw NotImplementedException("pg_copy_from");
}

Variant HHVM_FUNCTION(pg_copy_to, CResRef connection, CStrRef table_name, CStrRef delimiter /* = "\t" */, CStrRef null_as /* = "\n" */) {
  throw NotImplementedException("pg_copy_to");
}

bool HHVM_FUNCTION(pg_put_line, CResRef connection, CStrRef data) {
  throw NotImplementedException("pg_put_line");
}

}
