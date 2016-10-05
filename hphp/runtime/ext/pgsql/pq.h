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

#ifndef incl_PQ_H_
#define incl_PQ_H_

#include <string>
#include <iostream>
#include <libpq-fe.h>
#include <utility>

namespace PQ {

struct Connection;

struct Result {
  Result() {
    m_res = nullptr;
  }
  explicit Result(const Result&) = delete;
  Result& operator=(const Result&) = delete;

  Result(Result&& other) noexcept {
    m_res = other.m_res;
    other.m_res = nullptr;
  }

  Result& operator=(Result&& other) {
    m_res = other.m_res;
    other.m_res = nullptr;
    return *this;
  }

  ExecStatusType status() { return PQresultStatus(m_res); }

  ~Result() {
    if (m_res) {
      PQclear(m_res);
      m_res = nullptr;
    }
  }

  void clear() {
    if (m_res) {
      PQclear(m_res);
      m_res = nullptr;
    }
  }

  explicit operator bool() const {
    return (bool)m_res;
  }

  int numFields() const {
    return PQnfields(m_res);
  }

  int cmdTuples() const {
    const char * n = PQcmdTuples(m_res);
    if (n[0] == 0) return 0;
    return atoi(n);
  }

  long lcmdTuples() const {
    const char * n = PQcmdTuples(m_res);
    if(n[0] == 0) return 0;
    return atol(n);
  }

  int numTuples() const {
    return PQntuples(m_res);
  }

  char *cmdStatus() {
    return PQcmdStatus(m_res);
  }

  const char * errorMessage() {
    return PQresultErrorMessage(m_res);
  }

  char *errorField(int fieldCode) {
    return PQresultErrorField(m_res, fieldCode);
  }

  char *fieldName(int column_number) {
    return PQfname(m_res, column_number);
  }

  int fieldNumber(const char * name) {
    return PQfnumber(m_res, name);
  }

  bool fieldIsNull(int row, int field) {
    return PQgetisnull(m_res, row, field) == 1;
  }

  char * getValue(int row, int field) {
    return PQgetvalue(m_res, row, field);
  }

  int getLength(int row, int field) {
    return PQgetlength(m_res, row, field);
  }

  int size(int column_number) const {
    return PQfsize(m_res, column_number);
  }

  unsigned long precision(int column_number) const {
    return PQfmod(m_res, column_number);
  }

  Oid table(int column_number) {
    return PQftable(m_res, column_number);
  }

  Oid type(int column_number) {
    return PQftype(m_res, column_number);
  }

  Oid oidValue() {
    return PQoidValue(m_res);
  }

  friend class Connection;
private:
  /* implicit */ Result(PGresult *res) : m_res(res) {}

  PGresult *m_res;
};

struct Connection {
  // Disable copying
  explicit Connection(const Connection&) = delete;
  Connection& operator=(const Connection&) = delete;

  explicit Connection(const char *conninfo) {
    if (conninfo != nullptr)
      m_conn = PQconnectdb(conninfo);
  }

  explicit Connection(const std::string& conninfo) {
    m_conn = PQconnectdb(conninfo.c_str());
  }

  ~Connection() {
    if (m_conn) {
      PQfinish(m_conn);
      m_conn = 0;
    }
  }

  void finish() {
    if (m_conn) {
      PQfinish(m_conn);
      m_conn = 0;
    }
  }

  explicit operator bool() const {
    return (bool)m_conn;
  }

  void reset() { PQreset(m_conn); }

  std::string db() {
    std::string val;
    char * raw_val = PQdb(m_conn);
    if (raw_val)
      val = raw_val;
    return val;
  }

  std::string user() {
    std::string val;
    char * raw_val = PQuser(m_conn);
    if (raw_val)
      val = raw_val;
    return val;
  }

  std::string pass() {
    std::string val;
    char * raw_val = PQpass(m_conn);
    if (raw_val)
      val = raw_val;
    return val;
  }

  std::string host() {
    std::string val;
    char * raw_val = PQhost(m_conn);
    if (raw_val)
      val = raw_val;
    return val;
  }

  std::string port() {
    std::string val;
    char * raw_val = PQport(m_conn);
    if (raw_val)
      val = raw_val;
    return val;
  }

  std::string options() {
    std::string val;
    char * raw_val = PQoptions(m_conn);
    if (raw_val)
      val = raw_val;
    return val;
  }

  ConnStatusType status() {
    if (m_conn == nullptr) return CONNECTION_BAD;
    return PQstatus(m_conn);
  }

  PGTransactionStatusType transactionStatus() {
    return PQtransactionStatus(m_conn);
  }

  int protocolVersion() { return PQprotocolVersion(m_conn); }

  int serverVersion() { return PQserverVersion(m_conn); }

  const char *errorMessage() { return PQerrorMessage(m_conn); }

  bool needsPassword() { return PQconnectionNeedsPassword(m_conn) == 1; }
  bool usedPassword() { return PQconnectionUsedPassword(m_conn) == 1; }

  Result exec(const char *command) {
    PGresult *res = PQexec(m_conn, command);
    return Result(res);
  }
  Result exec(const std::string &cmd) { return exec(cmd.c_str()); }

  Result exec(
    const char *command, int nParams, const char * const *paramValues
  ) {
    PGresult *res = PQexecParams(m_conn, command, nParams, nullptr,
      paramValues, nullptr, nullptr, 0);
    return Result(res);
  }

  Result prepare(const char *name, const char *query, int nParams) {
    return prepare(name, query, nParams, nullptr);
  }

  Result prepare(
    const char *name, const char *query, int nParams, const Oid *paramTypes
  ) {
    PGresult *res = PQprepare(m_conn, name, query, nParams, paramTypes);
    return Result(res);
  }

  Result execPrepared(
    const char *name, int nParams, const char * const *paramValues
  ) {
    return execPrepared(name, nParams, paramValues, nullptr, nullptr);
  }

  Result execPrepared(
    const char *name,
    int nParams,
    const char * const *paramValues,
    const int *paramLengths,
    const int *paramFormats
  ){
    PGresult *res = PQexecPrepared(
      m_conn, name, nParams, paramValues, paramLengths, paramFormats, 0);
    return Result(res);

  }

  bool sendQuery(const char *query) {
    return (bool)PQsendQuery(m_conn, query);
  }

  bool sendQuery(
    const char *command, int nParams, const char * const *paramValues) {
    return (bool)PQsendQueryParams(m_conn, command, nParams, nullptr,
      paramValues, nullptr, nullptr, 0);
  }

  bool sendPrepare(const char *name, const char *query, int nParams) {
    return (bool)PQsendPrepare(m_conn, name, query, nParams, nullptr);
  }

  bool sendQueryPrepared(
    const char *name, int nParams, const char * const *paramValues) {
    return (bool)PQsendQueryPrepared(
      m_conn, name, nParams, paramValues, nullptr, nullptr, 0);
  }

  Result result() {
    return Result(PQgetResult(m_conn));
  }

  bool isNonBlocking() {
    return PQisnonblocking(m_conn);
  }

  bool setNonBlocking(bool val=true) {
    return PQsetnonblocking(m_conn, (int)val);
  }

  template<typename T>
  PQnoticeProcessor setNoticeProcessor(
    void (*proc)(T*, const char *), T* arg) {
    return PQsetNoticeProcessor(m_conn, (PQnoticeProcessor)proc,
      static_cast<void *>(arg));
  }

  int consumeInput() {
    return PQconsumeInput(m_conn);
  }

  bool isBusy() {
    return PQisBusy(m_conn) == 1;
  }

  const char *parameterStatus(const char *paramName) const {
    return PQparameterStatus(m_conn, paramName);
  }

  const char *clientEncoding() const {
    int enc = PQclientEncoding(m_conn);
    return pg_encoding_to_char(enc);
  }

  bool setClientEncoding(const char* enc) {
    return PQsetClientEncoding(m_conn, enc) == 0;
  }

  PGVerbosity setErrorVerbosity(int verbosity) {
    return PQsetErrorVerbosity(m_conn, (PGVerbosity) verbosity);
  }

  int backendPID() {
    return PQbackendPID(m_conn);
  }

  std::string escapeByteA(const char *data, size_t size) {
    size_t escape_size = 0;

    char *escaped = (char *)PQescapeByteaConn(m_conn, (unsigned char *)data,
      size, &escape_size);

    std::string ret;
    if (escaped) {
      ret.assign(escaped, escape_size-1);
      PQfreemem(escaped);
    }

    return ret;
  }

  std::string escapeIdentifier(const char *data, size_t size) {
    char * escaped = (char *)PQescapeIdentifier(m_conn, data, size);
    std::string ret;
    if (escaped) {
      ret.assign(escaped);
      PQfreemem(escaped);
    }

    return ret;
  }

  std::string escapeLiteral(const char *data, size_t size) {
    char * escaped = (char *)PQescapeLiteral(m_conn, data, size);
    std::string ret;
    if (escaped) {
      ret.assign(escaped);
      PQfreemem(escaped);
    }

    return ret;
  }

  size_t escapeString(char *to, const char *from, size_t length, int *error) {
    return PQescapeStringConn(m_conn, to, from, length, error);
  }

  int flush() {
    return PQflush(m_conn);
  }

  bool cancelRequest() {
    return (bool)PQrequestCancel(m_conn);
  }

private:
  PGconn *m_conn;
};
}

#endif//incl_PQ_H_
