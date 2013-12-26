/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2013 Facebook, Inc. (http://www.facebook.com)     |
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

#ifndef incl_HPHP_RUNTIME_SERVER_FASTCGI_FASTCGI_SESSION_H_
#define incl_HPHP_RUNTIME_SERVER_FASTCGI_FASTCGI_SESSION_H_

#include "hphp/runtime/server/fastcgi/protocol-session.h"
#include "folly/io/IOBuf.h"
#include "folly/io/Cursor.h"
#include <map>

namespace HPHP {

///////////////////////////////////////////////////////////////////////////////

class FastCGISession;
class ProtocolSessionHandler;

class FastCGITransaction
  : public ProtocolSessionHandler::Callback {
public:
  typedef uint16_t RequestId;

  FastCGITransaction(FastCGISession* session,
                     RequestId request_id,
                     ProtocolSessionHandlerPtr handler);
  virtual ~FastCGITransaction();

  void onStdIn(std::unique_ptr<folly::IOBuf> chain);
  void onParams(std::unique_ptr<folly::IOBuf> chain);
  void onData(std::unique_ptr<folly::IOBuf> chain);
  void onGetValues(std::unique_ptr<folly::IOBuf> chain);

  void onStdOut(std::unique_ptr<folly::IOBuf> chain) override;
  void onStdErr(std::unique_ptr<folly::IOBuf> chain) override;
  void onComplete() override;

private:
  enum Phase {
    READ_KEY_LENGTH,
    READ_VALUE_LENGTH,
    READ_KEY,
    READ_VALUE
  };

  typedef uint8_t ShortLength;
  typedef uint32_t LongLength;

  bool parseKeyValue(folly::io::Cursor& cursor, size_t& available);
  bool parseKeyValueLength(folly::io::Cursor& cursor,
                           size_t& available,
                           size_t& lengthReturn);
  bool parseKeyValueContent(folly::io::Cursor& cursor,
                            size_t& available,
                            size_t& length,
                            folly::IOBufQueue& queue);

  void handleGetValue(std::unique_ptr<folly::IOBuf> value_buf);
  void handleInvalidRecord();

  static const ShortLength k_longLengthFlag;

  Phase m_phase;

  // A transaction will either receive PARAMS or GET_VALUES,
  // never both. Otherwise two separate buffers would be required.
  folly::IOBufQueue m_readBuf;
  size_t m_keyLength;
  size_t m_keyLeft;
  size_t m_valueLength;
  size_t m_valueLeft;
  folly::IOBufQueue m_keyBuf;
  folly::IOBufQueue m_valueBuf;
  FastCGISession* m_session;
  RequestId m_requestId;
  ProtocolSessionHandlerPtr m_handler;
};


class FastCGISession : public ProtocolSession {
friend class FastCGITransaction;
public:
  typedef FastCGITransaction Transaction;
  typedef Transaction::RequestId RequestId;

  FastCGISession();
  virtual ~FastCGISession();

  virtual size_t onIngress(const folly::IOBuf* chain) override;

  void setMaxConns(int max_conns);
  void setMaxRequests(int max_requets);

protected:
  typedef uint8_t Version;

  enum Phase {
    INVALID = 0,
    AT_RECORD_BEGIN = 1,
    INSIDE_RECORD_BODY = 2,
    AT_RECORD_BODY_END = 3
  };

  enum RecordType : uint8_t {
    BEGIN_REQUEST = 1,
    ABORT_REQUEST = 2,
    END_REQUEST = 3,
    PARAMS = 4,
    STDIN = 5,
    STDOUT = 6,
    STDERR = 7,
    DATA = 8,
    GET_VALUES = 9,
    GET_VALUES_RESULT = 10,
    UNKNOWN_TYPE = 11
  };

  enum Role : uint16_t {
    RESPONDER = 1,
    AUTHORIZER = 2,
    FILTER = 3
  };

  enum ConnectionFlags : uint8_t {
    KEEP_CONN = 1
  };

  typedef uint32_t AppStatus;

  enum ProtoStatus : uint8_t {
    REQUEST_COMPLETE = 0,
    CANT_MULTIPLEX_CONN = 1,
    OVERLOADED = 2,
    UNKNOWN_ROLE = 3
  };

  typedef uint8_t ShortLength;
  typedef uint32_t LongLength;

  typedef uint16_t ContentLength;
  typedef uint8_t PaddingLength;

  static const Version k_version;
  static const size_t k_writeGrowth;

  static const ShortLength k_longLengthFlag;
  static const size_t k_recordBeginLength;
  static const size_t k_recordReservedLength;

  static const size_t k_beginRequestLength;
  static const size_t k_beginRequestReservedLength;
  static const size_t k_abortRequestLength;
  static const size_t k_endRequestLength;
  static const size_t k_endRequestReservedLength;
  static const size_t k_unknownTypeLength;
  static const size_t k_unknownTypeReservedLength;

  static const std::string k_getValueMaxConnKey;
  static const std::string k_getValueMaxRequestsKey;
  static const std::string k_getValueMultiplexConnsKey;

  typedef std::map<RequestId, std::unique_ptr<Transaction>> TransactionMap;
  typedef std::map<std::unique_ptr<folly::IOBuf>,
                   std::unique_ptr<folly::IOBuf>> ParamMap;

  bool parseRecordBegin(folly::io::Cursor& cursor, size_t& available);
  bool parseRecordBody(folly::io::Cursor& cursor, size_t& available);
  bool parseRecordEnd(folly::io::Cursor& cursor, size_t& available);

  void parseBeginRequest(folly::io::Cursor& cursor, size_t& available);
  void parseAbortRequest(folly::io::Cursor& cursor, size_t& available);
  void parseParams(folly::io::Cursor& cursor, size_t& available);
  void parseStdIn(folly::io::Cursor& cursor, size_t& available);
  void parseData(folly::io::Cursor& cursor, size_t& available);
  void parseGetValues(folly::io::Cursor& cursor, size_t& available);
  void parseIgnoreRecord(folly::io::Cursor& cursor, size_t& available);

  void handleBeginRequest(Role role, ConnectionFlags flags);
  void handleAbortRequest();
  void handleStdIn(std::unique_ptr<folly::IOBuf> chain);
  void handleData(std::unique_ptr<folly::IOBuf> chain);
  void handleParams(std::unique_ptr<folly::IOBuf> chain);
  void handleGetValues(std::unique_ptr<folly::IOBuf> chain);
  void handleStdOut(RequestId request_id,
                    std::unique_ptr<folly::IOBuf> chain);
  void handleStdErr(RequestId request_id,
                    std::unique_ptr<folly::IOBuf> chain);
  void handleComplete(RequestId request_id);
  void handleGetValueResult(const std::string& key);
  void handleInvalidRecord();
  void handleClose();

  void writeEndRequest(RequestId request_id,
                       AppStatus app_status,
                       ProtoStatus proto_status);
  void writeStdOut(RequestId request_id,
                   std::unique_ptr<folly::IOBuf> chain);
  void writeStdErr(RequestId request_id,
                   std::unique_ptr<folly::IOBuf> chain);
  void writeGetValueResult(const std::string& key,
                           const std::string& value);
  void writeUnknownType(RecordType record_type);
  void writeStream(RequestId request_id,
                   RecordType record_type,
                   std::unique_ptr<folly::IOBuf> stream_chain);
  void writeEgress(std::unique_ptr<folly::IOBuf> chain);

  template<typename AppenderT>
  void appendRecordBegin(AppenderT& cursor,
                         typename FastCGISession::RecordType record_type,
                         typename FastCGISession::RequestId request_id,
                         size_t content_length,
                         size_t padding_length);

  void prependRecordBegin(folly::io::RWPrivateCursor& cursor,
                          RecordType record_type,
                          RequestId request_id,
                          size_t content_length,
                          size_t padding_length);

  template<typename AppenderT>
  void appendRecordEnd(AppenderT& cursor,
                       size_t padding_length);

  size_t getPaddingLength(size_t content_length);

  bool hasTransaction(RequestId request_id);
  void beginTransaction(RequestId request_id);
  std::unique_ptr<Transaction>& getTransaction(RequestId request_id);
  void endTransaction(RequestId request_id);

  Phase m_phase;
  RecordType m_recordType;
  RequestId m_requestId;
  ContentLength m_contentLength;
  ContentLength m_contentLeft;
  PaddingLength m_paddingLength;
  PaddingLength m_paddingLeft;
  bool m_keepConn;
  int m_maxConns;
  int m_maxRequests;

  TransactionMap m_transactions;
};

///////////////////////////////////////////////////////////////////////////////

template<typename AppenderT>
void FastCGISession::appendRecordBegin(AppenderT& cursor,
                       typename FastCGISession::RecordType record_type,
                       typename FastCGISession::RequestId request_id,
                       size_t content_length,
                       size_t padding_length) {
  cursor.template writeBE<Version>(k_version);
  cursor.template writeBE<uint8_t>(static_cast<uint8_t>(record_type));
  cursor.template writeBE<RequestId>(request_id);
  CHECK(content_length <= std::template numeric_limits<uint16_t>::max());
  cursor.template writeBE<uint16_t>(content_length);
  CHECK(padding_length <= std::template numeric_limits<uint8_t>::max());
  cursor.template writeBE<uint8_t>(padding_length);
  cursor.ensure(k_recordReservedLength);
  memset(cursor.writableData(), 0, k_recordReservedLength);
  cursor.append(k_recordReservedLength);
}

template<typename AppenderT>
void FastCGISession::appendRecordEnd(AppenderT& cursor,
                                     size_t padding_length) {
  CHECK(padding_length <= std::numeric_limits<uint8_t>::max());
  cursor.ensure(padding_length);
  memset(cursor.writableData(), 0, padding_length);
  cursor.append(padding_length);
}

}

#endif // incl_HPHP_HTTP_SERVER_FASTCGI_FASTCGI_SESSION_H_

