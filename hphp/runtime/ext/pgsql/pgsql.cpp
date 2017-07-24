/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2016 PocketRent Ltd and contributors                   |
   +----------------------------------------------------------------------+
   | This source file is subject to version 3.01 of the PHP license,      |
   | that is bundled with this package in the file LICENSE, and is        |
   | available through the world-wide-web at the following url:           |
   | http://www.php.net/license/3_01.txt                                  |
   | If you did not receive a copy of the PHP license and are unable to   |
   | obtain it through the world-wide-web, please send a note to          |
   | license@php.net so we can mail you a copy immediately.               |
   +----------------------------------------------------------------------+
 */

#include <queue>
#include "pq.h"

#include "hphp/runtime/base/array-iterator.h"
#include "hphp/runtime/base/zend-string.h"

#include "hphp/runtime/base/runtime-option.h"
#include "hphp/runtime/server/server-stats.h"
#include "hphp/runtime/ext/string/ext_string.h"

#define PGSQL_ASSOC 1
#define PGSQL_NUM 2
#define PGSQL_BOTH (PGSQL_ASSOC | PGSQL_NUM)
#define PGSQL_STATUS_LONG 1
#define PGSQL_STATUS_STRING 2

#define FAIL_RETURN return false
#define MAKE_STR_VECTOR(name, params)                 \
  std::vector<const char *> name;                     \
  std::vector<String> name##_decrefs;                 \
  name.reserve(params.size());                        \
  name##_decrefs.reserve(params.size());              \
                                                      \
  for (ArrayIter iter(params); iter; ++iter) {        \
    auto const param = tvToCell(iter.secondRval());   \
    if (isNullType(param.type())) {                   \
      name.push_back(nullptr);                        \
    } else {                                          \
      name##_decrefs.push_back(                       \
        String::attach(tvCastToString(param.tv()))    \
      );                                              \
      name.push_back(name##_decrefs.back().c_str());  \
    }                                                 \
  }

namespace HPHP {
namespace { // Anonymous namespace

struct ScopeNonBlocking {
  ScopeNonBlocking(PQ::Connection& conn, bool mode) :
    m_conn(conn), m_mode(mode) {}

  ~ScopeNonBlocking() {
    m_conn.setNonBlocking(m_mode);
  }

  PQ::Connection& m_conn;
  bool m_mode;
};

struct PGSQLConnectionPool;

struct PGSQLConnectionPoolContainer {
  PGSQLConnectionPoolContainer();
  explicit PGSQLConnectionPoolContainer(PGSQLConnectionPoolContainer const&);
  void operator=(PGSQLConnectionPoolContainer const&);

  ~PGSQLConnectionPoolContainer();

  PGSQLConnectionPool& GetPool(const String);
  std::vector<PGSQLConnectionPool*>& GetPools();

private:
  std::map<String, PGSQLConnectionPool*> m_pools;
  Mutex m_lock;
} s_connectionPoolContainer;

struct PGSQLConnectionPool {
  long SweepedConnections() const { return m_sweepedConnections; }
  long OpenedConnections() const { return m_openedConnections; }
  long RequestedConnections() const { return m_requestedConnections; }
  long ReleasedConnections() const { return m_releasedConnections; }
  long Errors() const { return m_errors; }

  int TotalConnectionsCount() const { return m_connections.size(); }
  int FreeConnectionsCount() const { return m_availableConnections.size(); }

  PGSQLConnectionPool(
    String connectionString, int maximumConnections = -1
  );
  ~PGSQLConnectionPool();

  PQ::Connection& GetConnection();
  void Release(PQ::Connection& connection);

  String GetConnectionString() const { return m_connectionString; }
  String GetCleanedConnectionString() const {
    return m_cleanedConnectionString;
  }

  void CloseAllConnections();
  void CloseFreeConnections();
  int MaximumConnections() const { return m_maximumConnections; }
  void SweepConnection(PQ::Connection& connection);

private:
  int m_maximumConnections;
  Mutex m_lock;
  String m_connectionString;
  String m_cleanedConnectionString;
  std::queue<PQ::Connection*> m_availableConnections;
  std::vector<PQ::Connection*> m_connections;

  long m_sweepedConnections = 0;
  long m_openedConnections = 0;
  long m_requestedConnections = 0;
  long m_releasedConnections = 0;
  long m_errors = 0;
};

struct PGSQL : SweepableResourceData {
  DECLARE_RESOURCE_ALLOCATION(PGSQL);
  static bool AllowPersistent;
  static int MaxPersistent;
  static int MaxLinks;
  static bool AutoResetPersistent;
  static bool IgnoreNotice;
  static bool LogNotice;

  static req::ptr<PGSQL> Get(const Variant& conn_id);

  explicit PGSQL(String conninfo);
  explicit PGSQL(PGSQLConnectionPool& connectionPool);
  ~PGSQL();

  void ReleaseConnection();

  static StaticString s_class_name;
  virtual const String& o_getClassNameHook() const { return s_class_name; }
  virtual bool isResource() const { return m_conn != nullptr; }

  PQ::Connection &get() { return *m_conn; }

  ScopeNonBlocking asNonBlocking() {
    auto mode = m_conn->isNonBlocking();
    return ScopeNonBlocking(*m_conn, mode);
  }

  bool IsConnectionPooled() const { return m_connectionPool != nullptr; }

private:
  PQ::Connection* m_conn;

  PGSQLConnectionPool* m_connectionPool = nullptr;

public:
  String m_conn_string;

  String m_db;
  String m_user;
  String m_pass;
  String m_host;
  String m_port;
  String m_options;

  String m_last_notice;
  void SetupInformation();
};

struct PGSQLResult : SweepableResourceData {
  DECLARE_RESOURCE_ALLOCATION(PGSQLResult);
  static req::ptr<PGSQLResult> Get(const Variant& result);
  PGSQLResult(req::ptr<PGSQL> conn, PQ::Result res);
  ~PGSQLResult();

  static StaticString s_class_name;
  const String& o_getClassNameHook() const override { return s_class_name; }
  bool isResource() const { return (bool)m_res; }

  void close();

  PQ::Result& get() { return m_res; }

  int getFieldNumber(const Variant& field);
  int getNumFields();
  int getNumRows();

  bool convertFieldRow(const Variant& row, const Variant& field,
    int *out_row, int *out_field, const char *fn_name = nullptr);

  Variant fieldIsNull(
    const Variant& row, const Variant& field, const char *fn_name = nullptr
  );

  Variant getFieldVal(
    const Variant& row, const Variant& field, const char *fn_name = nullptr
  );
  String getFieldVal(int row, int field, const char *fn_name = nullptr);

  req::ptr<PGSQL> getConn() { return m_conn; }

  int m_current_row;
private:
  PQ::Result m_res;
  int m_num_fields;
  int m_num_rows;
  req::ptr<PGSQL> m_conn;
};
}

////////////////////////////////////////////////////////////////////////////////

StaticString PGSQL::s_class_name("pgsql connection");
StaticString PGSQLResult::s_class_name("pgsql result");


req::ptr<PGSQL> PGSQL::Get(const Variant& conn_id) {
  if (conn_id.isNull()) {
    return nullptr;
  }

  req::ptr<PGSQL> pgsql = dyn_cast_or_null<PGSQL>(conn_id.toResource());
  return pgsql;
}

static void notice_processor(PGSQL *pgsql, const char *message) {
  if (pgsql != nullptr) {
    pgsql->m_last_notice = message;

    if (PGSQL::LogNotice) {
      raise_notice("%s", message);
    }
  }
}

void PGSQL::SetupInformation() {
  if (!m_conn) return;

  m_db = m_conn->db();
  m_user = m_conn->user();
  m_pass = m_conn->pass();
  m_host = m_conn->host();
  m_port = m_conn->port();
  m_options = m_conn->options();

  if (!PGSQL::IgnoreNotice) {
    m_conn->setNoticeProcessor(notice_processor, this);
  } else {
    m_conn->setNoticeProcessor<PGSQL>(notice_processor, nullptr);
  }
}

PGSQL::PGSQL(String conninfo)
  : m_conn_string(conninfo), m_last_notice("") {
  m_conn = new PQ::Connection(conninfo.data());

  if (RuntimeOption::EnableStats && RuntimeOption::EnableSQLStats) {
    ServerStats::Log("sql.conn", 1);
  }

  ConnStatusType st = m_conn->status();
  if (m_conn && st == CONNECTION_OK) {
    // Load up the fixed information
    SetupInformation();
  } else if (st == CONNECTION_BAD) {
    m_conn->finish();
  }
}

PGSQL::PGSQL(PGSQLConnectionPool &connectionPool)
  : m_conn_string(connectionPool.GetConnectionString()),
  m_last_notice("")
{
  m_conn = &(connectionPool.GetConnection());
  m_connectionPool = &connectionPool;

  SetupInformation();
}

PGSQL::~PGSQL() {
  ReleaseConnection();
}

void PGSQL::sweep() {
  ReleaseConnection();
}

void PGSQL::ReleaseConnection() {
  if (!m_conn) return;

  if (!IsConnectionPooled()) {
    m_conn->finish();
  } else {
    m_connectionPool->Release(*m_conn);
    m_connectionPool = nullptr;
    m_conn = nullptr;
  }
}

req::ptr<PGSQLResult> PGSQLResult::Get(const Variant& result) {
  if (result.isNull()) {
    return nullptr;
  }

  req::ptr<PGSQLResult> res =
    dyn_cast_or_null<PGSQLResult>(result.toResource());
  return res;
}

PGSQLResult::PGSQLResult(req::ptr<PGSQL> conn, PQ::Result res)
  : m_current_row(0), m_res(std::move(res)),
  m_num_fields(-1), m_num_rows(-1), m_conn(conn) {
  m_conn->incRefCount();
}

void PGSQLResult::close() {
  m_res.clear();
}

PGSQLResult::~PGSQLResult() {
  close();
}

void PGSQLResult::sweep() {
  close();
}

int PGSQLResult::getFieldNumber(const Variant& field) {
  int n;
  if (field.isNumeric(true)) {
    n = field.toInt32();
  } else if (field.isString()){
    n = m_res.fieldNumber(field.asCStrRef().data());
  } else {
    n = -1;
  }

  return n;
}

int PGSQLResult::getNumFields() {
  if (m_num_fields == -1) {
    m_num_fields = m_res.numFields();
  }
  return m_num_fields;
}

int PGSQLResult::getNumRows() {
  if (m_num_rows == -1) {
    m_num_rows = m_res.numTuples();
  }
  return m_num_rows;
}

bool PGSQLResult::convertFieldRow(const Variant& row, const Variant& field,
  int *out_row, int *out_field, const char *fn_name) {
  Variant actual_field;
  int actual_row;

  assert(out_row && out_field && "Output parameters cannot be null");

  if (!fn_name) {
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
      raise_warning(
        "%s(): Unknown column name \"%s\"",
        fn_name, actual_field.asCStrRef().data()
      );
    } else {
      raise_warning(
        "%s(): Column offset `%d` out of range", fn_name, field_number
      );
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

Variant PGSQLResult::fieldIsNull(
  const Variant& row, const Variant& field, const char *fn_name
) {
  int r, f;
  if (convertFieldRow(row, field, &r, &f, fn_name)) {
    return m_res.fieldIsNull(r, f) ? 1 : 0;
  }

  return false;
}

Variant PGSQLResult::getFieldVal(
  const Variant& row, const Variant& field, const char *fn_name
) {
  int r, f;
  if (convertFieldRow(row, field, &r, &f, fn_name)) {
    return getFieldVal(r, f, fn_name);
  }

  return false;
}

String PGSQLResult::getFieldVal(int row, int field, const char *fn_name) {
  if (m_res.fieldIsNull(row, field)) {
    return null_string;
  } else {
    char * value = m_res.getValue(row, field);
    int length = m_res.getLength(row, field);

    return String(value, length, CopyString);
  }
}


////////////////////////////////////////////////////////////////////////////////

PGSQLConnectionPool::PGSQLConnectionPool(
  String connectionString, int maximumConnections
) :m_maximumConnections(maximumConnections),
  m_connectionString(connectionString),
  m_availableConnections(),
  m_connections() {
}

PGSQLConnectionPool::~PGSQLConnectionPool() {
  CloseAllConnections();
}

PQ::Connection& PGSQLConnectionPool::GetConnection() {
  Lock lock(m_lock);

  // 1) m_availableConnections
  // 2) newconn, max 1

  m_requestedConnections++;

  while (!m_availableConnections.empty()) {
    PQ::Connection* pconn = m_availableConnections.front();
    PQ::Connection& conn = *pconn;
    m_availableConnections.pop();

    ConnStatusType st = conn.status();
    if (conn && st == CONNECTION_OK) {
      return conn;
    } else if (st == CONNECTION_BAD) {
      SweepConnection(conn);

      conn.finish();
    };
  }

  if (RuntimeOption::EnableStats && RuntimeOption::EnableSQLStats) {
    ServerStats::Log("sql.conn", 1);
  }

  int maxConnections = MaximumConnections();
  int connections = m_connections.size();

  if (maxConnections > 0 && connections < maxConnections)
    raise_error("The connection pool is full, cannot open new connection.");

  PQ::Connection& conn = *(new PQ::Connection(GetConnectionString().data()));

  ConnStatusType st = conn.status();
  if (st == CONNECTION_OK) {
    m_openedConnections++;
    m_connections.push_back(&conn);
  } else if (st == CONNECTION_BAD) {
    m_errors++;

    conn.finish();

    raise_error("Getting connection from pool failed.");
  }

  if (m_cleanedConnectionString.empty()) {
    std::string connectionString("host=");
    connectionString += conn.host();
    connectionString += " port=";
    connectionString += conn.port();
    connectionString += " user=";
    connectionString += conn.user();
    connectionString += " dbname=";
    connectionString += conn.db();
    m_cleanedConnectionString = String(connectionString);
  }

  return conn;
}

void PGSQLConnectionPool::SweepConnection(PQ::Connection& connection) {
  auto p = std::find(m_connections.begin(), m_connections.end(), &connection);

  if (p != m_connections.end())
    m_connections.erase(p);

  m_sweepedConnections++;
}

void PGSQLConnectionPool::Release(PQ::Connection& connection) {
  Lock lock(m_lock);

  m_releasedConnections++;

  ConnStatusType st = connection.status();
  if (connection && st == CONNECTION_OK) {
    m_availableConnections.push(&connection);
  } else if (st == CONNECTION_BAD) {
    connection.finish();
    SweepConnection(connection);
  }
}

void PGSQLConnectionPool::CloseAllConnections() {
  Lock lock(m_lock);

  while (!m_availableConnections.empty()) {
    PQ::Connection* pconn = m_availableConnections.front();
    pconn->finish();

    m_availableConnections.pop();
  }

  for (PQ::Connection* conn : m_connections)
    conn->finish();

  m_connections.clear();
}

void PGSQLConnectionPool::CloseFreeConnections() {
  Lock lock(m_lock);

  while (!m_availableConnections.empty()) {
    PQ::Connection* pconn = m_availableConnections.front();
    pconn->finish();

    m_availableConnections.pop();
  }
}

PGSQLConnectionPoolContainer::PGSQLConnectionPoolContainer() :m_pools() {
}

PGSQLConnectionPoolContainer::~PGSQLConnectionPoolContainer() {
  for (auto & any : m_pools) {
    PGSQLConnectionPool* pool = any.second;
    pool->CloseAllConnections();
  }
}

PGSQLConnectionPool& PGSQLConnectionPoolContainer::GetPool(
  const String connString
) {
  Lock lock(m_lock);

  auto pool = m_pools[connString];

  if (!pool) {
    pool = new PGSQLConnectionPool(connString);

    m_pools[connString] = pool;
  }

  return *pool;
}

std::vector<PGSQLConnectionPool*>& PGSQLConnectionPoolContainer::GetPools() {
  Lock lock(m_lock);

  std::vector<PGSQLConnectionPool*>* v =
    new std::vector<PGSQLConnectionPool*>();

  for (auto it : m_pools)
  {
    v->push_back(it.second);
  }

  return *v;
}

//////////////////// Connection functions /////////////////////////

static Variant HHVM_FUNCTION(pg_connect,
  const String& connection_string, int connect_type /* = 0 */
) {
  auto pgsql = req::make<PGSQL>(connection_string);

  if (!pgsql->get()) {
    FAIL_RETURN;
  }
  return Resource(pgsql);
}


static Variant HHVM_FUNCTION(pg_pconnect,
  const String& connection_string, int connect_type /* = 0 */
) {
  PGSQLConnectionPool& pool =
    s_connectionPoolContainer.GetPool(connection_string.toCppString());

  auto pgsql = req::make<PGSQL>(pool);

  if (!pgsql->get()) {
    FAIL_RETURN;
  }
  return Resource(pgsql);
}

static bool HHVM_FUNCTION(pg_close, const Resource& connection) {
  auto pgsql = PGSQL::Get(connection);
  if (pgsql) {
    pgsql->ReleaseConnection();

    return true;
  } else {
    return false;
  }
}

static bool HHVM_FUNCTION(pg_ping, const Resource& connection) {
  auto pgsql = PGSQL::Get(connection);

  if (!pgsql->get()) {
    return false;
  }

  PGPing response = PQping(pgsql->m_conn_string.data());

  if (response == PQPING_OK) {
    if (pgsql->get().status() == CONNECTION_BAD) {
      pgsql->get().reset();
      return pgsql->get().status() != CONNECTION_BAD;
    } else {
      return true;
    }
  }

  return false;
}

static bool HHVM_FUNCTION(pg_connection_reset, const Resource& connection) {
  auto pgsql = PGSQL::Get(connection);

  if (!pgsql->get()) {
    return false;
  }

  pgsql->get().reset();

  return pgsql->get().status() != CONNECTION_BAD;
}


//////////////////// Connection Pool functions /////////////////////////


const StaticString
  s_connection_string("connection_string"),
  s_sweeped_connections("sweeped_connections"),
  s_opened_connections("opened_connections"),
  s_requested_connections("requested_connections"),
  s_released_connections("released_connections"),
  s_errors("errors"),
  s_total_connections("total_connections"),
  s_free_connections("free_connections");

static Variant HHVM_FUNCTION(pg_connection_pool_stat) {
  auto pools = s_connectionPoolContainer.GetPools();

  Array arr;

  int i = 0;

  for (auto pool : pools) {
    Array poolArr;

    String poolName(pool->GetCleanedConnectionString().c_str(), CopyString);

    poolArr.set(s_connection_string, poolName);
    poolArr.set(s_sweeped_connections, pool->SweepedConnections());
    poolArr.set(s_opened_connections, pool->OpenedConnections());
    poolArr.set(s_requested_connections, pool->RequestedConnections());
    poolArr.set(s_released_connections, pool->ReleasedConnections());
    poolArr.set(s_errors, pool->Errors());
    poolArr.set(s_total_connections, pool->TotalConnectionsCount());
    poolArr.set(s_free_connections, pool->FreeConnectionsCount());

    arr.set(i, poolArr);
    i++;
  }

  return arr;
}

static void HHVM_FUNCTION(pg_connection_pool_sweep_free) {
  auto pools = s_connectionPoolContainer.GetPools();

  for (auto pool : pools) {
    pool->CloseFreeConnections();
  }
}


///////////// Interrogation Functions ////////////////////

static int64_t HHVM_FUNCTION(pg_connection_status, const Resource& connection) {
  auto pgsql = PGSQL::Get(connection);
  if (!pgsql) return CONNECTION_BAD;
  return (int64_t)pgsql->get().status();
}

static bool HHVM_FUNCTION(pg_connection_busy, const Resource& connection) {
  auto pgsql = PGSQL::Get(connection);
  if (!pgsql) {
    return false;
  }

  auto blocking = pgsql->asNonBlocking();

  pgsql->get().consumeInput();
  return pgsql->get().isBusy();
}

static Variant HHVM_FUNCTION(pg_dbname, const Resource& connection) {
  auto pgsql = PGSQL::Get(connection);

  if (!pgsql) {
    FAIL_RETURN;
  }

  return pgsql->m_db;
}

static Variant HHVM_FUNCTION(pg_host, const Resource& connection) {
  auto pgsql = PGSQL::Get(connection);

  if (!pgsql) {
    FAIL_RETURN;
  }

  return pgsql->m_host;
}

static Variant HHVM_FUNCTION(pg_port, const Resource& connection) {
  auto pgsql = PGSQL::Get(connection);

  if (!pgsql) {
    FAIL_RETURN;
  }

  String ret = pgsql->m_port;
  if (ret.isNumeric()) {
    return ret.toInt32();
  } else {
    return ret;
  }
}

static Variant HHVM_FUNCTION(pg_options, const Resource& connection) {
  auto pgsql = PGSQL::Get(connection);

  if (!pgsql) {
    FAIL_RETURN;
  }

  return pgsql->m_options;
}

static Variant HHVM_FUNCTION(pg_parameter_status,
  const Resource& connection, const String& param_name) {
  auto pgsql = PGSQL::Get(connection);

  if (!pgsql) {
    return false;
  }

  String ret(pgsql->get().parameterStatus(param_name.data()), CopyString);

  return ret;
}

static Variant HHVM_FUNCTION(pg_client_encoding, const Resource& connection) {
  auto pgsql = PGSQL::Get(connection);

  if (!pgsql) {
    FAIL_RETURN;
  }

  String ret(pgsql->get().clientEncoding(), CopyString);

  return ret;
}

static int HHVM_FUNCTION(pg_set_client_encoding,
  const Resource& connection, const String& enc) {
  auto pgsql = PGSQL::Get(connection);

  if (!pgsql) {
    return -1;
  }

  return pgsql->get().setClientEncoding(enc.c_str()) ? 0 : -1;
}

static Variant HHVM_FUNCTION(pg_set_error_verbosity,
  const Resource& connection, int64_t verbosity) {
  auto pgsql = PGSQL::Get(connection);

  if (!pgsql) {
    return false;
  }

  if (verbosity & (PQERRORS_TERSE|PQERRORS_DEFAULT|PQERRORS_VERBOSE)) {
    return pgsql->get().setErrorVerbosity(verbosity);
  }

  return false;
}

static int64_t HHVM_FUNCTION(pg_transaction_status,
  const Resource& connection) {
  auto pgsql = PGSQL::Get(connection);

  if (!pgsql) {
    return PQTRANS_UNKNOWN;
  }

  return (int64_t)pgsql->get().transactionStatus();
}

static Variant HHVM_FUNCTION(pg_last_error, const Resource& connection) {
  auto pgsql = PGSQL::Get(connection);
  if (!pgsql) {
    FAIL_RETURN;
  }

  String ret(pgsql->get().errorMessage(), CopyString);

  return f_trim(ret);
}

static Variant HHVM_FUNCTION(pg_last_notice, const Resource& connection) {
  auto pgsql = PGSQL::Get(connection);
  if (!pgsql) {
    FAIL_RETURN;
  }

  return pgsql->m_last_notice;
}

static Variant HHVM_FUNCTION(pg_version, const Resource& connection) {
  static StaticString client_key("client");
  static StaticString protocol_key("protocol");
  static StaticString server_key("server");

  auto pgsql = PGSQL::Get(connection);
  if (!pgsql) {
    FAIL_RETURN;
  }

  Array ret;

  int proto_ver = pgsql->get().protocolVersion();
  if (proto_ver) {
    ret.set(protocol_key, String(proto_ver) + ".0");
  }

  int server_ver = pgsql->get().serverVersion();
  if (server_ver) {
    int revision = server_ver % 100;
    int minor = (server_ver / 100) % 100;
    int major = server_ver / 10000;

    ret.set(
      server_key, String(major) + "." + String(minor) + "." + String(revision)
    );
  }

  int client_ver = PQlibVersion();
  if (client_ver) {
    int revision = client_ver % 100;
    int minor = (client_ver / 100) % 100;
    int major = client_ver / 10000;

    ret.set(
      client_key, String(major) + "." + String(minor) + "." + String(revision)
    );
  }

  return ret;
}

static int64_t HHVM_FUNCTION(pg_get_pid, const Resource& connection) {
  auto pgsql = PGSQL::Get(connection);
  if (!pgsql) {
    return -1;
  }

  return (int64_t)pgsql->get().backendPID();
}

//////////////// Escaping Functions ///////////////////////////

static String HHVM_FUNCTION(pg_escape_bytea,
  const Resource& connection, const String& data) {
  auto pgsql = PGSQL::Get(connection);
  if (!pgsql) {
    return null_string;
  }

  std::string escaped = pgsql->get().escapeByteA(data.data(), data.size());

  if (escaped.empty()) {
    raise_warning("pg_escape_bytea(): %s", pgsql->get().errorMessage());
    return null_string;
  }

  String ret(escaped);

  return ret;
}

static String HHVM_FUNCTION(pg_escape_identifier,
  const Resource& connection, const String& data) {
  auto pgsql = PGSQL::Get(connection);
  if (!pgsql) {
    return null_string;
  }

  std::string escaped = pgsql->get().escapeIdentifier(data.data(), data.size());

  if (escaped.empty()) {
    raise_warning("pg_escape_identifier(): %s", pgsql->get().errorMessage());
    return null_string;
  }

  String ret(escaped);

  return ret;
}

static String HHVM_FUNCTION(pg_escape_literal,
  const Resource& connection, const String& data) {
  auto pgsql = PGSQL::Get(connection);
  if (!pgsql) {
    return null_string;
  }

  std::string escaped = pgsql->get().escapeLiteral(data.data(), data.size());

  if (escaped.empty()) {
    raise_warning("pg_escape_literal(): %s", pgsql->get().errorMessage());
    return null_string;
  }

  String ret(escaped);

  return ret;
}

static String HHVM_FUNCTION(pg_escape_string,
  const Resource& connection, const String& data) {
  auto pgsql = PGSQL::Get(connection);
  if (!pgsql) {
    return null_string;
  }

  // Reserve enough space as defined by PQescapeStringConn
  String ret((data.size()*2)+1, ReserveString);

  int error = 0;
  size_t size = pgsql->get().escapeString(
      ret.get()->mutableData(), data.data(), data.size(),
      &error);

  if (error) {
    return null_string;
  }

  // Shrink to the returned size, `shrink` may re-alloc and free up space
  ret.shrink(size);

  return ret;
}

static String HHVM_FUNCTION(pg_unescape_bytea, const String& data) {
  size_t to_len = 0;
  char * unesc = (char *)PQunescapeBytea((unsigned char *)data.data(), &to_len);
  String ret = String(unesc, to_len, CopyString);
  PQfreemem(unesc);
  return ret;
}

///////////// Command Execution / Querying /////////////////////////////

static int64_t HHVM_FUNCTION(pg_affected_rows, const Resource& result) {
  auto res = PGSQLResult::Get(result);
  if (!res) return 0;

  return (int64_t)res->get().cmdTuples();
}

static Variant HHVM_FUNCTION(pg_result_status,
  const Resource& result, int64_t type /* = PGSQL_STATUS_LONG */) {
  auto res = PGSQLResult::Get(result);

  if (type == PGSQL_STATUS_LONG) {
    if (!res) return 0;

    return (int64_t)res->get().status();
  } else {
    if (!res) return null_string;

    String ret(res->get().cmdStatus(), CopyString);
    return ret;
  }
}

static bool HHVM_FUNCTION(pg_free_result, const Resource& result) {
  auto res = PGSQLResult::Get(result);
  if (res) {
    res->close();
    return true;
  } else {
    return false;
  }
}

static bool _handle_query_result(
  const char *fn_name, PQ::Connection &conn, PQ::Result &result) {
  if (!result) {
    const char * err = conn.errorMessage();
    raise_warning("%s(): Query failed: %s", fn_name, err);
    return true;
  } else {
    int st = result.status();
    switch (st) {
      default:
        break;
      case PGRES_EMPTY_QUERY:
      case PGRES_BAD_RESPONSE:
      case PGRES_NONFATAL_ERROR:
      case PGRES_FATAL_ERROR:
        const char * msg = result.errorMessage();
        raise_warning("%s(): Query failed: %s", fn_name, msg);
        return true;
    }
    return false;
  }
}

static Variant HHVM_FUNCTION(pg_query,
  const Resource& connection, const String& query) {
  auto conn = PGSQL::Get(connection);
  if (!conn) {
    FAIL_RETURN;
  }

  PQ::Result res = conn->get().exec(query.data());

  if (_handle_query_result("pg_query", conn->get(), res))
    FAIL_RETURN;

  auto pgresult = req::make<PGSQLResult>(conn, std::move(res));

  return Resource(pgresult);
}

static Variant HHVM_FUNCTION(pg_query_params,
  const Resource& connection, const String& query, const Array& params) {
  auto conn = PGSQL::Get(connection);
  if (!conn) {
    FAIL_RETURN;
  }

  MAKE_STR_VECTOR(str_array, params);

  PQ::Result res =
    conn->get().exec(query.data(), params.size(), str_array.data());

  if (_handle_query_result("pg_query_params", conn->get(), res))
    FAIL_RETURN;

  auto pgresult = req::make<PGSQLResult>(conn, std::move(res));

  return Resource(pgresult);
}

static Variant HHVM_FUNCTION(pg_prepare,
  const Resource& connection, const String& stmtname, const String& query) {
  auto conn = PGSQL::Get(connection);
  if (!conn) {
    FAIL_RETURN;
  }

  PQ::Result res = conn->get().prepare(stmtname.data(), query.data(), 0);

  if (_handle_query_result("pg_prepare", conn->get(), res))
    FAIL_RETURN;

  auto pgres = req::make<PGSQLResult>(conn, std::move(res));

  return Resource(pgres);
}

static Variant HHVM_FUNCTION(pg_execute,
  const Resource& connection, const String& stmtname, const Array& params) {
  auto conn = PGSQL::Get(connection);
  if (!conn) {
    FAIL_RETURN;
  }

  MAKE_STR_VECTOR(str_array, params);

  PQ::Result res =
    conn->get().execPrepared(stmtname.data(), params.size(), str_array.data());
  if (_handle_query_result("pg_execute", conn->get(), res)) {
    FAIL_RETURN;
  }

  auto pgres = req::make<PGSQLResult>(conn, std::move(res));

  return Resource(pgres);
}

static bool HHVM_FUNCTION(pg_send_query,
  const Resource& connection, const String& query) {
  auto conn = PGSQL::Get(connection);
  if (!conn) {
    return false;
  }

  auto nb = conn->asNonBlocking();

  bool empty = true;
  PQ::Result res = conn->get().result();
  while (res) {
    res.clear();
    empty = false;
    res = conn->get().result();
  }
  if (!empty) {
    raise_notice("There are results on this connection."
        " Call pg_get_result() until it returns FALSE");
  }

  if (!conn->get().sendQuery(query.data())) {
    // TODO: Do persistent auto-reconnect
    return false;
  }

  int ret;
  while ((ret = conn->get().flush())) {
    if (ret == -1) {
      raise_notice("Could not empty PostgreSQL send buffer");
      break;
    }
    // Sleep a little as Postgres hasn't received all of the async query
    /* sleep override */
    usleep(1000);
  }

  return true;
}

static Variant HHVM_FUNCTION(pg_get_result, const Resource& connection) {
  auto conn = PGSQL::Get(connection);
  if (!conn) {
    FAIL_RETURN;
  }

  PQ::Result res = conn->get().result();

  if (!res) {
    FAIL_RETURN;
  }

  auto pgresult = req::make<PGSQLResult>(conn, std::move(res));

  return Resource(pgresult);
}

static bool HHVM_FUNCTION(pg_send_query_params,
  const Resource& connection, const String& query, const Array& params) {
  auto conn = PGSQL::Get(connection);
  if (!conn) {
    return false;
  }

  auto nb = conn->asNonBlocking();

  bool empty = true;
  PQ::Result res = conn->get().result();
  while (res) {
    res.clear();
    empty = false;
    res = conn->get().result();
  }
  if (!empty) {
    raise_notice("There are results on this connection."
        " Call pg_get_result() until it returns FALSE");
  }

  MAKE_STR_VECTOR(str_array, params);

  if (!conn->get().sendQuery(query.data(), params.size(), str_array.data())) {
    return false;
  }

  int ret;
  while ((ret = conn->get().flush())) {
    if (ret == -1) {
      raise_notice("Could not empty PostgreSQL send buffer");
      break;
    }
    // Sleep a little as Postgres hasn't received all of the async query
    /* sleep override */
    usleep(1000);
  }

  return true;
}

static bool HHVM_FUNCTION(pg_send_prepare,
  const Resource& connection, const String& stmtname, const String& query) {
  auto conn = PGSQL::Get(connection);
  if (!conn) {
    return false;
  }

  return conn->get().sendPrepare(stmtname.data(), query.data(), 0);
}

static bool HHVM_FUNCTION(pg_send_execute,
  const Resource& connection, const String& stmtname, const Array& params) {
  auto conn = PGSQL::Get(connection);
  if (!conn) {
    return false;
  }

  MAKE_STR_VECTOR(str_array, params);

  return conn->get().sendQueryPrepared(stmtname.data(),
    params.size(), str_array.data());
}

static bool HHVM_FUNCTION(pg_cancel_query, const Resource& connection) {
  auto conn = PGSQL::Get(connection);
  if (!conn) {
    return false;
  }

  auto nb = conn->asNonBlocking();

  bool ret = conn->get().cancelRequest();

  PQ::Result res = conn->get().result();
  while(res) {
    res.clear();
    res = conn->get().result();
  }

  return ret;
}

////////////////////////

static Variant HHVM_FUNCTION(pg_fetch_all_columns,
  const Resource& result, int64_t column /* = 0 */) {
  auto res = PGSQLResult::Get(result);
  if (!res) {
    FAIL_RETURN;
  }

  if (column < 0 || column >= res->getNumFields()) {
    raise_warning(
      "pg_fetch_all_columns(): Column offset `%d` out of range",
      (int)column
    );
    FAIL_RETURN;
  }

  int num_rows = res->getNumRows();

  Array arr;
  for (int i = 0; i < num_rows; i++) {
    Variant field = res->getFieldVal(i, column);
    arr.set(i, field);
  }

  return arr;
}

static Variant HHVM_FUNCTION(pg_fetch_array,
  const Resource& result,
  const Variant& row /* = uninit_variant */,
  int64_t result_type /* = PGSQL_BOTH */
) {
  auto res = PGSQLResult::Get(result);
  if (!res) {
    FAIL_RETURN;
  }

  int r;
  if (row.isNull()) {
    r = res->m_current_row;
    if (r >= res->getNumRows()) {
      FAIL_RETURN;
    }
    res->m_current_row++;
  } else {
    r = row.toInt32();
  }

  if (r < 0 || r >= res->getNumRows()) {
    raise_warning("Row `%d` out of range", r);
    FAIL_RETURN;
  }

  Array arr;

  for (int i = 0; i < res->getNumFields(); i++) {
    Variant field = res->getFieldVal(r, i);
    if (result_type & PGSQL_NUM) {
      arr.set(i, field);
    }
    if (result_type & PGSQL_ASSOC) {
      char * name = res->get().fieldName(i);
      String fn(name, CopyString);
      arr.set(fn, field);
    }
  }

  return arr;
}

static Variant HHVM_FUNCTION(pg_fetch_assoc,
  const Resource& result, const Variant& row /* = uninit_variant */) {
  return f_pg_fetch_array(result, row, PGSQL_ASSOC);
}

static Variant HHVM_FUNCTION(pg_fetch_all, const Resource& result) {
  auto res = PGSQLResult::Get(result);
  if (!res) {
    FAIL_RETURN;
  }

  int num_rows = res->getNumRows();
  if (num_rows == 0) {
    FAIL_RETURN;
  }

  Array rows;
  for (int i = 0; i < num_rows; i++) {
    Variant row = f_pg_fetch_assoc(result, i);
    rows.set(i, row);
  }

  return rows;
}

static Variant HHVM_FUNCTION(pg_fetch_result,
  const Resource& result,
  const Variant& row /* = uninit_variant */,
  const Variant& field /* = uninit_variant */
) {
  auto res = PGSQLResult::Get(result);
  if (!res) {
    FAIL_RETURN;
  }

  return res->getFieldVal(row, field, "pg_fetch_result");
}

static Variant HHVM_FUNCTION(pg_fetch_row,
  const Resource& result, const Variant& row /* = uninit_variant */) {
  return f_pg_fetch_array(result, row, PGSQL_NUM);
}

///////////////////// Field information //////////////////////////

static Variant HHVM_FUNCTION(pg_field_is_null,
  const Resource& result,
  const Variant& row,
  const Variant& field /* = uninit_variant */
) {
  auto res = PGSQLResult::Get(result);
  if (!res) {
    FAIL_RETURN;
  }

  return res->fieldIsNull(row, field, "pg_field_is_null");
}

static Variant HHVM_FUNCTION(pg_field_name,
  const Resource& result, int64_t field_number
) {
  auto res = PGSQLResult::Get(result);
  if (!res) {
    FAIL_RETURN;
  }

  if (field_number < 0 || field_number >= res->getNumFields()) {
    raise_warning(
      "pg_field_name(): Column offset `%d` out of range", (int)field_number);
    FAIL_RETURN;
  }

  char * name = res->get().fieldName((int)field_number);
  if (!name) {
    raise_warning("pg_field_name(): %s", res->get().errorMessage());
    FAIL_RETURN;
  } else {
    return String(name, CopyString);
  }
}

static int64_t HHVM_FUNCTION(pg_field_num,
  const Resource& result, const String& field_name) {
  auto res = PGSQLResult::Get(result);
  if (!res) {
    return -1;
  }

  return res->get().fieldNumber(field_name.data());
}

static Variant HHVM_FUNCTION(pg_field_prtlen,
  const Resource& result,
  const Variant& row_number,
  const Variant& field /* = uninit_variant */
) {
  auto res = PGSQLResult::Get(result);
  if (!res) {
    FAIL_RETURN;
  }

  int r, f;
  if (res->convertFieldRow(row_number, field, &r, &f, "pg_field_prtlen")) {
    return res->get().getLength(r, f);
  }
  FAIL_RETURN;
}

static Variant HHVM_FUNCTION(pg_field_size,
  const Resource& result, int64_t field_number) {
  auto res = PGSQLResult::Get(result);
  if (!res) {
    FAIL_RETURN;
  }

  if (field_number < 0 || field_number > res->getNumFields()) {
    raise_warning(
      "pg_field_size(): Column offset `%d` out of range",
      (int)field_number
    );
    FAIL_RETURN;
  }

  return res->get().size(field_number);
}

static Variant HHVM_FUNCTION(pg_field_table,
  const Resource& result, int64_t field_number, bool oid_only /* = false */) {
  auto res = PGSQLResult::Get(result);

  if (!res) {
    FAIL_RETURN;
  }

  if (field_number < 0 || field_number > res->getNumFields()) {
    raise_warning(
      "pg_field_table(): Column offset `%d` out of range", (int)field_number);
    FAIL_RETURN;
  }

  Oid id = res->get().table(field_number);
  if (id == InvalidOid) FAIL_RETURN;

  if (oid_only) {
    return (int64_t)id;
  } else {
    // TODO: cache the Oids somewhere
    std::ostringstream query;
    query << "SELECT relname FROM pg_class WHERE oid=" << id;

    PQ::Result name_res = res->getConn()->get().exec(query.str().c_str());
    if (!name_res)
      FAIL_RETURN;

    if (name_res.status() != PGRES_TUPLES_OK)
      FAIL_RETURN;

    char * name = name_res.getValue(0, 0);
    if (!name) {
      FAIL_RETURN;
    }

    String ret(name, CopyString);

    return ret;
  }
}

static Variant HHVM_FUNCTION(pg_field_type_oid,
  const Resource& result, int64_t field_number) {
  auto res = PGSQLResult::Get(result);

  if (!res) {
    FAIL_RETURN;
  }

  if (field_number < 0 || field_number > res->getNumFields()) {
    raise_warning(
      "pg_field_table(): Column offset `%d` out of range", (int)field_number);
    FAIL_RETURN;
  }

  Oid id = res->get().type(field_number);
  if (id == InvalidOid) FAIL_RETURN;
  return (int64_t)id;
}

// TODO: Cache the results of this function somewhere
static Variant HHVM_FUNCTION(pg_field_type,
  const Resource& result, int64_t field_number) {
  auto res = PGSQLResult::Get(result);

  if (!res) {
    FAIL_RETURN;
  }

  if (field_number < 0 || field_number > res->getNumFields()) {
    raise_warning(
      "pg_field_type(): Column offset `%d` out of range", (int)field_number);
    FAIL_RETURN;
  }

  Oid id = res->get().type(field_number);
  if (id == InvalidOid) FAIL_RETURN;

  // This should really get all of the rows in pg_type and build a map
  std::ostringstream query;
  query << "SELECT typname FROM pg_type WHERE oid=" << id;

  PQ::Result name_res = res->getConn()->get().exec(query.str().c_str());
  if (!name_res)
    FAIL_RETURN;

  if (name_res.status() != PGRES_TUPLES_OK)
    FAIL_RETURN;

  char * name = name_res.getValue(0, 0);
  if (!name)
    FAIL_RETURN;

  String ret(name, CopyString);

  return ret;
}

static int64_t HHVM_FUNCTION(pg_num_fields, const Resource& result) {
  auto res = PGSQLResult::Get(result);
  if (!res) {
    return -1;
  }

  return res->getNumFields();
}

static int64_t HHVM_FUNCTION(pg_num_rows, const Resource& result) {
  auto res = PGSQLResult::Get(result);
  if (!res) {
    return -1;
  }

  return res->getNumRows();
}

static Variant HHVM_FUNCTION(pg_result_error_field,
    const Resource& result, int64_t fieldcode) {
  auto res = PGSQLResult::Get(result);
  if (!res) {
    FAIL_RETURN;
  }

  char * msg = res->get().errorField(fieldcode);
  if (msg) {
    return f_trim(String(msg, CopyString));
  }

  FAIL_RETURN;
}

static Variant HHVM_FUNCTION(pg_result_error, const Resource& result) {
  auto res = PGSQLResult::Get(result);
  if (!res) {
    FAIL_RETURN;
  }

  const char * msg = res->get().errorMessage();
  if (msg) {
    return f_trim(String(msg, CopyString));
  }

  FAIL_RETURN;
}

static bool HHVM_FUNCTION(pg_result_seek,
    const Resource& result, int64_t offset) {
  auto res = PGSQLResult::Get(result);
  if (!res) {
    return false;
  }

  if (offset < 0 || offset > res->getNumRows()) {
    raise_warning("pg_result_seek(): Cannot seek to row %d", (int)offset);
    return false;
  }

  res->m_current_row = (int)offset;
  return true;
}

static Variant HHVM_FUNCTION(pg_last_oid, const Resource& result) {
  auto res = PGSQLResult::Get(result);
  if (!res) {
    FAIL_RETURN;
  }

  Oid oid = res->get().oidValue();

  if (oid == InvalidOid) FAIL_RETURN;
  else return String((int64_t)oid);
}
///////////////////////////////////////////////////////////////////////////////

bool PGSQL::AllowPersistent     = true;
int  PGSQL::MaxPersistent       = -1;
int  PGSQL::MaxLinks            = -1;
bool PGSQL::AutoResetPersistent = false;
bool PGSQL::IgnoreNotice        = false;
bool PGSQL::LogNotice           = false;

namespace { // Anonymous Namespace
static struct pgsqlExtension : Extension {
  pgsqlExtension() : Extension("pgsql") {}

  void moduleLoad(const IniSetting::Map& ini, Hdf hdf) override {
    Hdf pgsql = hdf["PGSQL"];

    PGSQL::AllowPersistent =
      Config::GetBool(ini, pgsql, "AllowPersistent", true);
    PGSQL::MaxPersistent =
      Config::GetInt32(ini, pgsql, "MaxPersistent", -1);
    PGSQL::MaxLinks =
      Config::GetInt32(ini, pgsql, "MaxLinks", -1);
    PGSQL::AutoResetPersistent =
      Config::GetBool(ini, pgsql, "AutoResetPersistent");
    PGSQL::IgnoreNotice = Config::GetBool(ini, pgsql, "IgnoreNotice");
    PGSQL::LogNotice = Config::GetBool(ini, pgsql, "LogNotice");
  }

  void moduleInit() override {
    HHVM_FE(pg_affected_rows);
    HHVM_FE(pg_cancel_query);
    HHVM_FE(pg_client_encoding);
    HHVM_FE(pg_close);
    HHVM_FE(pg_connect);
    HHVM_FE(pg_pconnect);
    HHVM_FE(pg_connection_pool_stat);
    HHVM_FE(pg_connection_pool_sweep_free);
    HHVM_FE(pg_connection_busy);
    HHVM_FE(pg_connection_reset);
    HHVM_FE(pg_connection_status);
    HHVM_FE(pg_dbname);
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
    HHVM_FE(pg_get_pid);
    HHVM_FE(pg_get_result);
    HHVM_FE(pg_host);
    HHVM_FE(pg_last_error);
    HHVM_FE(pg_last_notice);
    HHVM_FE(pg_last_oid);
    HHVM_FE(pg_num_fields);
    HHVM_FE(pg_num_rows);
    HHVM_FE(pg_options);
    HHVM_FE(pg_parameter_status);
    HHVM_FE(pg_ping);
    HHVM_FE(pg_port);
    HHVM_FE(pg_prepare);
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
    HHVM_FE(pg_set_error_verbosity);
    HHVM_FE(pg_transaction_status);
    HHVM_FE(pg_unescape_bytea);
    HHVM_FE(pg_version);

    // Register constants

    HHVM_RC_INT(PGSQL_ASSOC, PGSQL_ASSOC);
    HHVM_RC_INT(PGSQL_NUM, PGSQL_NUM);
    HHVM_RC_INT(PGSQL_BOTH, PGSQL_BOTH);

    HHVM_RC_INT(PGSQL_CONNECT_FORCE_NEW, 1);
    HHVM_RC_INT(PGSQL_CONNECTION_BAD, CONNECTION_BAD);
    HHVM_RC_INT(PGSQL_CONNECTION_OK, CONNECTION_OK);
    HHVM_RC_INT(PGSQL_CONNECTION_STARTED, CONNECTION_STARTED);
    HHVM_RC_INT(PGSQL_CONNECTION_MADE, CONNECTION_MADE);
    HHVM_RC_INT(PGSQL_CONNECTION_AWAITING_RESPONSE,
                CONNECTION_AWAITING_RESPONSE);
    HHVM_RC_INT(PGSQL_CONNECTION_AUTH_OK, CONNECTION_AUTH_OK);
    HHVM_RC_INT(PGSQL_CONNECTION_SETENV, CONNECTION_SETENV);
    HHVM_RC_INT(PGSQL_CONNECTION_SSL_STARTUP, CONNECTION_SSL_STARTUP);

    HHVM_RC_INT(PGSQL_SEEK_SET, SEEK_SET);
    HHVM_RC_INT(PGSQL_SEEK_CUR, SEEK_CUR);
    HHVM_RC_INT(PGSQL_SEEK_END, SEEK_END);

    HHVM_RC_INT(PGSQL_EMPTY_QUERY, PGRES_EMPTY_QUERY);
    HHVM_RC_INT(PGSQL_COMMAND_OK, PGRES_COMMAND_OK);
    HHVM_RC_INT(PGSQL_TUPLES_OK, PGRES_TUPLES_OK);
    HHVM_RC_INT(PGSQL_COPY_OUT, PGRES_COPY_OUT);
    HHVM_RC_INT(PGSQL_COPY_IN, PGRES_COPY_IN);
    HHVM_RC_INT(PGSQL_BAD_RESPONSE, PGRES_BAD_RESPONSE);
    HHVM_RC_INT(PGSQL_NONFATAL_ERROR, PGRES_NONFATAL_ERROR);
    HHVM_RC_INT(PGSQL_FATAL_ERROR, PGRES_FATAL_ERROR);

    HHVM_RC_INT(PGSQL_TRANSACTION_IDLE, PQTRANS_IDLE);
    HHVM_RC_INT(PGSQL_TRANSACTION_ACTIVE, PQTRANS_ACTIVE);
    HHVM_RC_INT(PGSQL_TRANSACTION_INTRANS, PQTRANS_INTRANS);
    HHVM_RC_INT(PGSQL_TRANSACTION_INERROR, PQTRANS_INERROR);
    HHVM_RC_INT(PGSQL_TRANSACTION_UNKNOWN, PQTRANS_UNKNOWN);

    HHVM_RC_INT(PGSQL_DIAG_SEVERITY, PG_DIAG_SEVERITY);
    HHVM_RC_INT(PGSQL_DIAG_SQLSTATE, PG_DIAG_SQLSTATE);
    HHVM_RC_INT(PGSQL_DIAG_MESSAGE_PRIMARY, PG_DIAG_MESSAGE_PRIMARY);
    HHVM_RC_INT(PGSQL_DIAG_MESSAGE_DETAIL, PG_DIAG_MESSAGE_DETAIL);
    HHVM_RC_INT(PGSQL_DIAG_MESSAGE_HINT, PG_DIAG_MESSAGE_HINT);
    HHVM_RC_INT(PGSQL_DIAG_STATEMENT_POSITION, PG_DIAG_STATEMENT_POSITION);
    HHVM_RC_INT(PGSQL_DIAG_INTERNAL_POSITION, PG_DIAG_INTERNAL_POSITION);
    HHVM_RC_INT(PGSQL_DIAG_INTERNAL_QUERY, PG_DIAG_INTERNAL_QUERY);
    HHVM_RC_INT(PGSQL_DIAG_CONTEXT, PG_DIAG_CONTEXT);
    HHVM_RC_INT(PGSQL_DIAG_SOURCE_FILE, PG_DIAG_SOURCE_FILE);
    HHVM_RC_INT(PGSQL_DIAG_SOURCE_LINE, PG_DIAG_SOURCE_LINE);
    HHVM_RC_INT(PGSQL_DIAG_SOURCE_FUNCTION, PG_DIAG_SOURCE_FUNCTION);

    HHVM_RC_INT(PGSQL_ERRORS_TERSE, PQERRORS_TERSE);
    HHVM_RC_INT(PGSQL_ERRORS_DEFAULT, PQERRORS_DEFAULT);
    HHVM_RC_INT(PGSQL_ERRORS_VERBOSE, PQERRORS_VERBOSE);

    HHVM_RC_INT(PGSQL_STATUS_LONG, PGSQL_STATUS_LONG);
    HHVM_RC_INT(PGSQL_STATUS_STRING, PGSQL_STATUS_STRING);

    HHVM_RC_INT(PGSQL_CONV_IGNORE_DEFAULT, 1);
    HHVM_RC_INT(PGSQL_CONV_FORCE_NULL, 2);
    HHVM_RC_INT(PGSQL_CONV_IGNORE_NOT_NULL, 4);

    loadSystemlib();
  }
} s_pgsql_extension;
}

HHVM_GET_MODULE(pgsql);

///////////////////////////////////////////////////////////////////////////////
}
