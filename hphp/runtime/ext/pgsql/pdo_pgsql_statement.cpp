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

#include "pdo_pgsql_statement.h"
#include "pdo_pgsql_resource.h"
#include "pdo_pgsql_connection.h"
#include "pdo_pgsql.h"

#include <boost/format.hpp>

#define STMT_HANDLE_ERROR(res) \
  (*m_conn).handleError(this, (*m_conn).sqlstate(res), res.errorMessage())

namespace HPHP {

unsigned long PDOPgSqlStatement::m_stmtNameCounter = 0;
unsigned long PDOPgSqlStatement::m_cursorNameCounter = 0;
PDOPgSqlStatement::PDOPgSqlStatement(
  req::ptr<PDOPgSqlResource> conn, PQ::Connection* server
) : m_conn(conn->conn()), m_server(server),
  m_result(), m_isPrepared(false), m_current_row(0) {
  this->dbh = cast<PDOResource>(conn);
}

PDOPgSqlStatement::~PDOPgSqlStatement(){
  sweep();
}

void PDOPgSqlStatement::sweep(){
  if(m_stmtName.size() > 0){
    if(m_isPrepared){
      std::stringstream ss;
      ss << "DEALLOCATE " << m_stmtName;
      m_server->exec(ss.str());
    }
  }

  if(m_cursorName.size() > 0){
    // Do we need a check here maybe to see if we've actually got
    // a cursor or not?
    std::stringstream ss;
    ss << "CLOSE " << m_cursorName;
    m_server->exec(ss.str());
  }

  m_server = nullptr;
  m_conn = nullptr;
  if (m_result) {
    m_result.clear();
  }
}

bool PDOPgSqlStatement::create(const String& sql, const Array &options){
  supports_placeholders = PDO_PLACEHOLDER_NAMED;

  bool scrollable = pdo_attr_lval(
    options, PDO_ATTR_CURSOR, PDO_CURSOR_FWDONLY
  ) == PDO_CURSOR_SCROLL;

  if(scrollable){
    m_cursorName = (
      boost::format("pdo_crsr_%08x") % ++m_cursorNameCounter
    ).str();
    // Disable prepared statements
    supports_placeholders = PDO_PLACEHOLDER_NONE;
  } else if (
    m_conn->m_emulate_prepare ||
    (!options.empty() && pdo_attr_lval(options, PDO_ATTR_EMULATE_PREPARES, 0))
  ) {
    supports_placeholders = PDO_PLACEHOLDER_NONE;
  }

  if(
    supports_placeholders != PDO_PLACEHOLDER_NONE &&
    m_server->protocolVersion() > 2
  ) {
    named_rewrite_template = "$%d";
    String nsql;
    int ret = pdo_parse_params(sp_PDOStatement(this), sql, nsql);
    if(ret == 1){
      // Query was rewritten
    } else if (ret == -1){
      // Query didn't parse - exception should have been thrown at this point
      strncpy(m_conn->error_code, error_code, 6);
      m_conn->error_code[5] = '\0';
      return false;
    } else {
      // Original is great
      nsql = sql;
    }

    m_stmtName = (boost::format("pdo_stmt_%08x") % ++m_stmtNameCounter).str();

    m_resolvedQuery = (std::string)nsql;
  }

  return true;
}

bool PDOPgSqlStatement::executer(){
  ExecStatusType status;
  if(m_result){
    m_result = PQ::Result();
  }
  m_current_row = 0;

  if(m_cursorName.size() > 0){
    if(m_isPrepared){
      std::stringstream ss;
      ss << "CLOSE " << m_cursorName;
      m_result = m_server->exec(ss.str());
    }

    std::stringstream q;
    q << "DECLARE " << m_cursorName;
    q << " SCROLL CURSOR WITH HOLD FOR " << active_query_string.data();

    m_result = m_server->exec(q.str());

    status = m_result.status();

    if(status != PGRES_COMMAND_OK && status != PGRES_TUPLES_OK){
      STMT_HANDLE_ERROR(m_result);
      return false;
    }

    m_isPrepared = true;

    q.str(std::string());

    // Fetch to be able to get total number of rows
    q << "FETCH FORWARD 0 FROM " << m_cursorName;
    m_result = m_server->exec(q.str());
  } else if(m_stmtName.size() > 0) {
    if(!m_isPrepared){
stmt_retry:
      m_result = m_server->prepare(
        m_stmtName.c_str(),
        m_resolvedQuery.c_str(),
        bound_params.size(),
        param_types.data()
      );

      status = m_result.status();
      switch(status) {
        case PGRES_COMMAND_OK:
        case PGRES_TUPLES_OK:
          // It worked!
          m_isPrepared = true;
          break;
        default:
          // Read Zend implementation for this one. I am not sure if this
          // applies to hhvm as well or not
          // but figure better leave it in here than not
          if(!strcmp(m_conn->sqlstate(m_result), "42P05")){
            std::stringstream q;
            q << "DEALLOCATE " << m_stmtName;
            m_server->exec(q.str());
            goto stmt_retry;
          } else {
            STMT_HANDLE_ERROR(m_result);
            return false;
          }
      }
    }

    std::vector<const char*> params;
    params.reserve(bound_params.size());

    for(auto it = param_values.begin(); it != param_values.end(); it++){
      if((*it).isNull()){
        params.push_back(nullptr);
      } else {
        params.push_back((*it).toString().c_str());
      }
    }

    if(params.size() != bound_params.size()){
      m_conn->handleError(
        this, "XX000", "Parameters not being bound correctly"
      );
      return false;
    }

    m_result = m_server->execPrepared(
      m_stmtName.c_str(),
      bound_params.size(),
      params.data(),
      param_lengths.data(),
      param_formats.data()
    );
  } else {
    m_result = m_server->exec(active_query_string.data());
  }

  status = m_result.status();

  if(status != PGRES_COMMAND_OK && status != PGRES_TUPLES_OK){
    STMT_HANDLE_ERROR(m_result);
    return false;
  }

  if(!executed && !column_count){
    column_count = (long)m_result.numFields();
    columns.reset();
    m_pgsql_column_types.clear();
    m_pgsql_column_types.reserve(column_count);
  }

  if(status == PGRES_COMMAND_OK){
    row_count = m_result.lcmdTuples();
    m_conn->pgoid = m_result.oidValue();
  } else {
    row_count = m_result.numTuples();
  }

  return true;
}

bool PDOPgSqlStatement::describer(int colno){
  if(!m_result){
    return false;
  }

  if(colno < 0 || colno >= column_count){
    // Outside column ranges
    return false;
  }

  if(columns.empty()){
    for(int i = 0; i < column_count; i++){
      columns.set(i, Resource(req::make<PDOColumn>()));
    }
  }

  req::ptr<PDOColumn> col = cast<PDOColumn>(columns[colno].toResource());
  col->name = String(m_result.fieldName(colno));
  col->maxlen = m_result.size(colno);
  col->precision = m_result.precision(colno);
  Oid oid = m_result.type(colno);

  Oid *column_type = m_pgsql_column_types.data()+colno;
  *column_type = oid;

  switch(oid){
    case BOOLOID:
      col->param_type = PDO_PARAM_BOOL;
      break;
    case OIDOID:
      // Let's get back to this later
      col->param_type = PDO_PARAM_STR;
      break;
    case INT2OID:
    case INT4OID:
      col->param_type = PDO_PARAM_INT;
      break;
    case INT8OID:
      if(sizeof(long) >= 8){
        col->param_type = PDO_PARAM_INT;
      } else {
        col->param_type = PDO_PARAM_STR;
      }
      break;
    case BYTEAOID:
      col->param_type = PDO_PARAM_LOB;
      break;
    default:
      col->param_type = PDO_PARAM_STR;
  }

  return true;
}

bool PDOPgSqlStatement::fetcher(PDOFetchOrientation ori, long offset){
  if(m_cursorName.size() > 0){
    bool oriOk;
    std::string oriStr = oriToStr(ori, offset, oriOk);
    if(!oriOk){
      return false;
    }
    auto q = std::string("FETCH ") + oriStr + " FROM " + m_cursorName;

    m_result = m_server->exec(q);
    ExecStatusType status = m_result.status();

    if(status != PGRES_COMMAND_OK && status != PGRES_TUPLES_OK){
      STMT_HANDLE_ERROR(m_result);
      return false;
    }

    if(m_result.numTuples()){
      m_current_row = 1;
      return true;
    } else {
      return false;
    }
  } else {
    if(m_current_row < row_count){
      m_current_row++;
      return true;
    } else {
      return false;
    }
  }
}

const StaticString
  s_pgsql_oid("pgsql:oid"),
  s_native_type("native_type");

bool PDOPgSqlStatement::getColumnMeta(int64_t colno, Array &return_value){
  if (!m_result) {
    return false;
  }

  if (colno < 0 || colno >= column_count){
    return false;
  }

  Oid coltype = m_pgsql_column_types[colno];

  return_value.set(s_pgsql_oid, (long)coltype);

  auto q = std::string("SELECT TYPNAME FROM PG_TYPE WHERE OID=") +
           std::to_string(coltype);

  PQ::Result res = m_server->exec(q);

  ExecStatusType status = res.status();

  if (status != PGRES_TUPLES_OK){
    return true;
  }

  if (res.numTuples() != 1){
    return true;
  }

  return_value.set(s_native_type, String(res.getValue(0, 0)));

  return true;
}

bool PDOPgSqlStatement::getColumn(int colno, Variant &value){
  if(!m_result){
    return false;
  }

  if(colno < 0 || colno >= column_count){
    return false;
  }

  // We have already increased m_current_row by 1 in fetch
  long current_row = m_current_row - 1;
  if(m_result.fieldIsNull(current_row, colno)){
    value = Variant(Variant::NullInit());
    return true;
  }

  char* val = m_result.getValue(current_row, colno);

  req::ptr<PDOColumn> col = cast<PDOColumn>(columns[colno].toResource());

  switch(col->param_type){
    case PDO_PARAM_INT:
      value = Variant(atol(val));
      return true;
    case PDO_PARAM_BOOL:
      value = Variant(*val == 't');
      return true;
    case PDO_PARAM_LOB:
      // Not implemented
      return false;
    case PDO_PARAM_NULL:
    case PDO_PARAM_STR:
    case PDO_PARAM_STMT:
    case PDO_PARAM_ZVAL:
    default:
      value = Variant(String(val, CopyString));
      return true;
  }
}

const StaticString
  s_t("t"),
  s_f("f");

bool PDOPgSqlStatement::paramHook(
  PDOBoundParam* param, PDOParamEvent event_type
){
  if (m_stmtName.size() > 0) {
    switch(event_type){
      case PDO_PARAM_EVT_FREE:
        param_values.clear();
        param_values.resize(0);
        param_lengths.clear();
        param_lengths.resize(0);
        param_formats.clear();
        param_formats.resize(0);
        param_types.clear();
        param_types.resize(0);
        m_pgsql_column_types.clear();
        m_pgsql_column_types.resize(0);
        break;
      case PDO_PARAM_EVT_NORMALIZE:
        // decode from $1, $2 into 0, 1, etc.
        if(param->name){
          if(param->name.data()[0] == '$'){
            param->paramno = atoi(param->name.data() + 1)-1;
          } else {
            // resolve name
            if(bound_param_map.exists(param->name, true)){
              param->paramno = atoi(
                bound_param_map[param->name].asCStrRef().data() + 1
              )-1;
            } else {
              m_conn->handleError(this, "HY093", param->name.data());
              return false;
            }
          }
        }
        break;
      case PDO_PARAM_EVT_ALLOC:
      case PDO_PARAM_EVT_EXEC_POST:
      case PDO_PARAM_EVT_FETCH_PRE:
      case PDO_PARAM_EVT_FETCH_POST:
        // work is done in normalize
        return true;
      case PDO_PARAM_EVT_EXEC_PRE:
        if(bound_param_map.isNull()){
          return false;
        }
        int elems = bound_params.size();
        if(param_values.size() == 0){
          param_values.resize(elems);
          param_lengths.resize(elems);
          param_formats.resize(elems);
          param_types.resize(elems);
        }

        if(param->paramno >= 0){
          if(param->paramno >= elems){
            m_conn->handleError(this, "HY105", "Too many parameters bound");
            return false;
          }


          Oid* param_ts = param_types.data();
          Variant* param_vals = param_values.data();
          int* param_fs = param_formats.data();
          int* param_ls = param_lengths.data();

          if(PDO_PARAM_TYPE(param->param_type) == PDO_PARAM_LOB){
            // Todo, implement LOBs
            return false;
          }

          if(PDO_PARAM_TYPE(param->param_type) == PDO_PARAM_NULL ||
              param->parameter.isNull()){
            param_vals[param->paramno] = Variant(Variant::NullInit());
            param_ls[param->paramno] = 0;
          } else if(param->parameter.isBoolean()){
            // Sadly we need to convert this to a 'real' pgsql boolean literal,
            // ie a string
            param_vals[param->paramno] =
              param->parameter.asBooleanVal() ? Variant(s_t) : Variant(s_f);
            param_ls[param->paramno] = 1;
            param_fs[param->paramno] = 0;
          } else {
            String str = param->parameter.toString();
            param_vals[param->paramno] = str;
            param_ls[param->paramno] = str.length();
            param_fs[param->paramno] = 0;
          }

          if(PDO_PARAM_TYPE(param->param_type) == PDO_PARAM_LOB){
            param_ts[param->paramno] = 0;
            param_fs[param->paramno] = 1;
          } else {
            param_ts[param->paramno] = 0;
          }
        }
        break;
    }
  } else {
    // Convert into a native pgsql boolean literal
    if(
      PDO_PARAM_TYPE(param->param_type) == PDO_PARAM_BOOL &&
      ((param->param_type & PDO_PARAM_INPUT_OUTPUT) !=
       PDO_PARAM_INPUT_OUTPUT)
    ){
      param->param_type = PDO_PARAM_STR;
      param->parameter =
        param->parameter.asBooleanVal() ? String(s_t) : String(s_f);
    }
  }

  return true;
}

bool PDOPgSqlStatement::cursorCloser(){
  // For some reason, even without using a cursor, this is ok
  // This is what Zend does
  return true;
}

bool PDOPgSqlStatement::support(SupportedMethod method){
  switch (method) {
    case MethodSetAttribute:
    case MethodGetAttribute:
    case MethodNextRowset:
      return false;
    default:
      return true;
  }
}
}
