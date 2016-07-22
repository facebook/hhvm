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

#include "pdo_pgsql_connection.h"
#include "pdo_pgsql_statement.h"
#include "pdo_pgsql_resource.h"
#include "pdo_pgsql.h"
#include "hphp/runtime/ext/stream/ext_stream.h"
#include "hphp/runtime/vm/jit/translator-inline.h"
#undef PACKAGE_VERSION // pg_config defines it
#include "pg_config.h" // needed for PG_VERSION

#define HANDLE_ERROR(stmt, res) \
  handleError(stmt, sqlstate(res), res.errorMessage())

void ReplaceStringInPlace(std::string& subject, const std::string& search,
  const std::string& replace) {
  size_t pos = 0;
  while ((pos = subject.find(search, pos)) != std::string::npos) {
    subject.replace(pos, search.length(), replace);
    pos += replace.length();
  }
}

/* For the convenience of drivers, this function will parse a data source
 * string, of the form "name=value; name2=value2" and populate variables
 * according to the data you pass in and array of pdo_data_src_parser
 * structures
 */
struct pdo_data_src_parser {
  const char *optname;
  char *optval;
  int freeme;
};

namespace HPHP {

PDOPgSqlConnection::PDOPgSqlConnection() :
  m_server(nullptr), pgoid(InvalidOid) {}

PDOPgSqlConnection::~PDOPgSqlConnection(){
  if(m_server){
    delete m_server;
  }
}

bool PDOPgSqlConnection::create(const Array &options){
  long connect_timeout = pdo_attr_lval(options, PDO_ATTR_TIMEOUT, 30);
  struct pdo_data_src_parser vars[] = {
    { "host", "localhost", 0 },
    { "port", "5432", 0 },
    { "dbname", "", 0 },
    { "user", nullptr, 0 },
    { "password", nullptr, 0 }
  };
  parseDataSource(data_source.data(), data_source.size(), vars, 5);

  std::stringstream conninfo;
  conninfo << "host='";
  std::string host(vars[0].optval);
  ReplaceStringInPlace(host, "\\", "\\\\");
  ReplaceStringInPlace(host, "'", "\\'");
  conninfo << host << "' port='";
  std::string port(vars[1].optval);
  ReplaceStringInPlace(port, "\\", "\\\\");
  ReplaceStringInPlace(port, "'", "\\'");
  conninfo << port << "' dbname='";
  std::string dbname(vars[2].optval);
  ReplaceStringInPlace(dbname, "\\", "\\\\");
  ReplaceStringInPlace(dbname, "'", "\\'");
  conninfo << dbname << "' password='";
  std::string password(vars[4].optval ? vars[4].optval : this->password);
  ReplaceStringInPlace(password, "\\", "\\\\");
  ReplaceStringInPlace(password, "'", "\\'");
  conninfo << password << "' user='";
  std::string username(vars[3].optval ? vars[3].optval : this->username);
  ReplaceStringInPlace(username, "\\", "\\\\");
  ReplaceStringInPlace(username, "'", "\\'");
  conninfo << username << "'";
  conninfo << " connect_timeout=" << connect_timeout;

  m_server = new PQ::Connection(conninfo.str());

  if(m_server->status() == CONNECTION_OK){
    return true;
  } else {
    handleError(
      nullptr,
      PHP_PDO_PGSQL_CONNECTION_FAILURE_SQLSTATE,
      m_server->errorMessage()
    );
    return false;
  }
}

bool PDOPgSqlConnection::closer(){
  if(m_server){
    delete m_server;
    m_server = nullptr;
  }

  return false;
}

int64_t PDOPgSqlConnection::doer(const String& sql){
  testConnection();

  const char* query = sql.data();

  PQ::Result res = m_server->exec(query);

  if(!res){
    // I think this error should be handled in a different way perhaps?
    handleError(nullptr, "XX000", "Invalid result data");
    return -1;
  }

  ExecStatusType status = m_lastExec = res.status();

  int64_t ret;

  if(status == PGRES_COMMAND_OK){
    ret = (int64_t)res.cmdTuples();
  } else if(status == PGRES_TUPLES_OK) {
    ret = 0L;
  } else {
    HANDLE_ERROR(nullptr, res);
    return -1L;
  }

  this->pgoid = res.oidValue();

  return ret;
}

bool PDOPgSqlConnection::transactionCommand(const char* command){
  testConnection();

  PQ::Result res = m_server->exec(command);

  if(!res){
    // I think this error should be handled in a different way perhaps?
    handleError(nullptr, "XX000", "Invalid result data");
    return -1;
  }

  ExecStatusType status = m_lastExec = res.status();

  if(status == PGRES_COMMAND_OK){
    return true;
  }

  HANDLE_ERROR(nullptr, res);
  return false;
}

bool PDOPgSqlConnection::begin(){
  return transactionCommand("BEGIN");
}

bool PDOPgSqlConnection::rollback(){
  return transactionCommand("ROLLBACK");
}

bool PDOPgSqlConnection::commit(){
  return transactionCommand("COMMIT");
}

void PDOPgSqlConnection::testConnection(){
  if(!m_server){
    handleError(nullptr, "08003", nullptr);
  }
}

bool PDOPgSqlConnection::checkLiveness(){
  if(!m_server){
    return false;
  }

  return m_server->status() == CONNECTION_OK;
}

const char* PDOPgSqlConnection::sqlstate(PQ::Result& result){
  const char* sqlstate = result.errorField(PG_DIAG_SQLSTATE);

  // Handle case where libpq doesn't return an SQLSTATE
  // (eg. server connection lost)
  if(sqlstate == nullptr){
    sqlstate = "XX000";
  }

  return sqlstate;
}

bool PDOPgSqlConnection::quoter(
  const String& input, String &quoted, PDOParamType paramtype
){
  switch(paramtype){
    case PDO_PARAM_LOB:
      quoted = m_server->escapeByteA(input.data(), input.length());
      return true;
    default:
      // http://www.postgresql.org/message-id/14249.1273943612@sss.pgh.pa.us +
      // space for the two surrounding quotes
      std::unique_ptr<char[]> buffer(new char[input.length()*2+3]);

      buffer[0] = '\'';
      int error;
      size_t written = m_server->escapeString(
          buffer.get()+1, input.c_str(), input.length()*2+1, &error
      );
      if(error){
        return false;
      }

      buffer.get()[1+written] = '\'';
      buffer.get()[2+written] = '\0';
      quoted = String(buffer.get(), CopyString);
      break;
  }

  return true;
}

String PDOPgSqlConnection::lastId(const char *name){
  if (name == nullptr || strlen(name) == 0){
    // This branch of code doesn't seem to ever do anything useful
    // however it does pretty much the same as the zend implementation
    // and that doesn't seem to provide anything useful either
    if(this->pgoid == InvalidOid){
      return empty_string();
    }
    return String((long)this->pgoid);
  } else {
    const char *values[1];
    values[0] = name;
    PQ::Result res = m_server->exec("SELECT CURRVAL($1)", 1, values);

    ExecStatusType status = res.status();

    if(res && (status == PGRES_TUPLES_OK)){
      return String(res.getValue(0, 0), CopyString);
    } else {
      HANDLE_ERROR(nullptr, res);
      return empty_string();
    }
  }
}

int PDOPgSqlConnection::getAttribute(int64_t attr, Variant &value){
  switch(attr){
    case PDO_ATTR_CLIENT_VERSION:
      value = String(PG_VERSION);
      break;
    case PDO_ATTR_SERVER_VERSION:
      if(m_server->protocolVersion() >= 3){ // Postgres 7.4 or later
        value = String(m_server->parameterStatus("server_version"), CopyString);
      } else {
        PQ::Result res = m_server->exec("SELECT VERSION()");
        if(res && res.status() == PGRES_TUPLES_OK){
          value = String((char *)res.getValue(0, 0), CopyString);
        }
      }
      break;
    case PDO_ATTR_CONNECTION_STATUS:
      switch(m_server->status()){
        case CONNECTION_STARTED:
          value = String("Waiting for connection to be made.", CopyString);
          break;

        case CONNECTION_MADE:
        case CONNECTION_OK:
          value = String("Connection OK; waiting to send.", CopyString);
          break;

        case CONNECTION_AWAITING_RESPONSE:
          value = String("Waiting for a response from the server.", CopyString);
          break;

        case CONNECTION_AUTH_OK:
          value = String(
            "Received authentication; waiting for backend start-up to finish.",
            CopyString
          );
          break;
#ifdef CONNECTION_SSL_STARTUP
        case CONNECTION_SSL_STARTUP:
          value = String("Negotiating SSL encryption.", CopyString);
          break;
#endif
        case CONNECTION_SETENV:
          value = String(
            "Negotiating environment-driven parameter settings.", CopyString
          );
          break;

        case CONNECTION_BAD:
        default:
          value = String("Bad connection.", CopyString);
          break;
      }
      break;
    case PDO_ATTR_SERVER_INFO:
      {
        int spid = m_server->backendPID();

        std::stringstream result;
        result << "PID: ";
        result << spid;
        result << "; Client Encoding: ";
        result << m_server->parameterStatus("client_encoding");
        result << "; Is Superusser: ";
        result << m_server->parameterStatus("is_superuser");
        result << "; Session Authorization: ";
        result << m_server->parameterStatus("session_authorization");
        result << "; Date Style: ";
        result << m_server->parameterStatus("DateStyle");

        value = String(result.str());
      }
      break;
    default:
      return 0;
  }

  return 1;
}

bool PDOPgSqlConnection::fetchErr(PDOStatement *stmt, Array &info) {
  if (stmt == nullptr) {
    info.append(
      m_lastExec == InvalidOid ?
        Variant(Variant::NullInit()) :
        Variant(m_lastExec)
    );
    info.append(
      err_msg.empty() ? Variant(Variant::NullInit()) : Variant(err_msg)
    );
    return true;
  } else {
    auto *s = static_cast<PDOPgSqlStatement *>(stmt);
    auto status = s->m_result.status();
    auto emsg = s->err_msg;

    info.append(
      status == InvalidOid ? Variant(Variant::NullInit()) : Variant(status)
    );
    info.append(emsg.empty() ? Variant(Variant::NullInit()) : Variant(emsg));
    return true;
  }
}

bool PDOPgSqlConnection::setAttribute(int64_t attr, const Variant &value){
  switch(attr){
    case PDO_ATTR_EMULATE_PREPARES:
      m_emulate_prepare = value.toBoolean();
      return true;
    default:
      return false;
  }
}

bool PDOPgSqlConnection::support(SupportedMethod method){
  return true;
}

bool PDOPgSqlConnection::preparer(
  const String& sql, sp_PDOStatement *stmt, const Variant& options
) {
  auto rsrc = req::make<PDOPgSqlResource>(
    std::dynamic_pointer_cast<PDOPgSqlConnection>(shared_from_this())
  );

  auto s = req::make<PDOPgSqlStatement>(rsrc, m_server);

  *stmt = s;

  if(s->create(sql, options.toArray())){
    alloc_own_columns = 1;
    return true;
  }

  stmt->reset();

  strcpy(error_code, s->error_code);
  return false;
}

void PDOPgSqlConnection::handleError(
  PDOPgSqlStatement* stmt, const char* sqlState, const char* msg
){
  PDOErrorType* err = &error_code;
  std::string* emsg = &err_msg;
  if(stmt != nullptr){
    err = &stmt->error_code;
    emsg = &stmt->err_msg;
  }

  strncpy(*err, sqlState, 6);
  (*err)[5] = '\0';
  *emsg = std::string(msg);
}

String PDOPgSqlConnection::pgsqlLOBCreate(){
  return String("ROFL LOB");
}
}
