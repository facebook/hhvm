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

#include "hphp/runtime/server/fastcgi/fastcgi-session.h"
#include "folly/io/IOBuf.h"
#include "folly/io/Cursor.h"
#include "folly/Memory.h"
#include <limits>
#include <map>

namespace HPHP {

using folly::IOBuf;
using folly::IOBufQueue;
using folly::io::Cursor;
using folly::io::Appender;
using folly::io::QueueAppender;
using folly::io::RWPrivateCursor;

////////////////////////////////////////////////////////////////////////////////

FastCGITransaction::FastCGITransaction(FastCGISession* session,
                                       RequestId request_id,
                                       ProtocolSessionHandlerPtr handler)
  : m_phase(Phase::READ_KEY_LENGTH),
    m_readBuf(IOBufQueue::cacheChainLength()),
    m_keyBuf(IOBufQueue::cacheChainLength()),
    m_valueBuf(IOBufQueue::cacheChainLength()),
    m_session(session),
    m_requestId(request_id),
    m_handler(handler) {
  if (handler) {
    handler->setCallback(this);
  }
}

FastCGITransaction::~FastCGITransaction() {}

void FastCGITransaction::onStdIn(std::unique_ptr<IOBuf> chain) {
  if (m_requestId == 0) {
    handleInvalidRecord();
    return;
  }
  assert(m_handler);

  if (chain->empty()) {
    m_handler->onBodyComplete();
  } else {
    m_handler->onBody(std::move(chain));
  }
}

void FastCGITransaction::onParams(std::unique_ptr<IOBuf> chain) {
  if (m_requestId == 0) {
    handleInvalidRecord();
    return;
  }
  assert(m_handler);

  if (chain == nullptr || chain->computeChainDataLength() == 0) {
    if (m_readBuf.chainLength() != 0 ||
        m_phase != Phase::READ_KEY_LENGTH) {
      // We could just exit here, but we don't want to risk
      // busy waiting.
      handleInvalidRecord();
    } else {
      m_handler->onHeadersComplete();
    }
    return;
  }

  m_readBuf.append(std::move(chain));
  size_t available = m_readBuf.chainLength();
  size_t avail = available;
  Cursor cursor(m_readBuf.front());

  while (parseKeyValue(cursor, avail)) {
    m_handler->onHeader(m_keyBuf.move(), m_valueBuf.move());
  }

  m_readBuf.split(available - avail);
}

void FastCGITransaction::onData(std::unique_ptr<IOBuf> chain) {
  // Since we act in the RESPONER role, we don't accept DATA stream.
  handleInvalidRecord();
}

void FastCGITransaction::onGetValues(std::unique_ptr<IOBuf> chain) {
  if (m_requestId != 0) {
    handleInvalidRecord();
    return;
  }
  assert(!m_handler);

  if (chain == nullptr || chain->computeChainDataLength() == 0) {
    if (m_readBuf.chainLength() != 0 ||
        m_phase != Phase::READ_KEY_LENGTH) {
      // We could just exit here, but we dont want to risk
      // busy waiting.
      handleInvalidRecord();
    }
    return;
  }

  m_readBuf.append(std::move(chain));
  size_t available = m_readBuf.chainLength();
  size_t avail = available;
  Cursor cursor(m_readBuf.front());

  while (parseKeyValue(cursor, avail)) {
    if (m_valueBuf.chainLength() > 0) {
      handleInvalidRecord();
      return;
    }
    handleGetValue(m_keyBuf.move());
  }

  m_readBuf.split(available - avail);
}

void FastCGITransaction::onStdOut(std::unique_ptr<IOBuf> chain) {
  m_session->handleStdOut(m_requestId, std::move(chain));
}

void FastCGITransaction::onStdErr(std::unique_ptr<IOBuf> chain) {
  m_session->handleStdErr(m_requestId, std::move(chain));
}

void FastCGITransaction::onComplete() {
  m_session->handleComplete(m_requestId);
}

bool FastCGITransaction::parseKeyValue(Cursor& cursor, size_t& available) {
  if (m_phase == Phase::READ_KEY_LENGTH) {
    if (parseKeyValueLength(cursor, available, m_keyLength)) {
      m_keyLeft = m_keyLength;
      m_phase = Phase::READ_VALUE_LENGTH;
    } else {
      return false;
    }
  }
  if (m_phase == Phase::READ_VALUE_LENGTH) {
    if (parseKeyValueLength(cursor, available, m_valueLength)) {
      m_valueLeft = m_valueLength;
      m_phase = Phase::READ_KEY;
    } else {
      return false;
    }
  }
  if (m_phase == Phase::READ_KEY) {
    if (parseKeyValueContent(cursor, available, m_keyLeft, m_keyBuf)) {
      m_phase = Phase::READ_VALUE;
    } else {
      return false;
    }
  }
  if (m_phase == Phase::READ_VALUE) {
    if (parseKeyValueContent(cursor, available, m_valueLeft, m_valueBuf)) {
      m_phase = Phase::READ_KEY_LENGTH;
      return true;
    } else {
      return false;
    }
  }
  return false;
}

const FastCGITransaction::ShortLength
  FastCGITransaction::k_longLengthFlag = 1 << 7;

bool FastCGITransaction::parseKeyValueLength(Cursor& cursor,
                                             size_t& available,
                                             size_t& lengthReturn) {
  if (available < sizeof(ShortLength)) {
    return false;
  }
  auto peeked = cursor.peek();
  if (*peeked.first & k_longLengthFlag) {
    if (available < sizeof(LongLength)) {
      return false;
    }
    lengthReturn = cursor.readBE<LongLength>();
    lengthReturn &= ~(k_longLengthFlag << 24);
    available -= sizeof(LongLength);
  } else {
    lengthReturn = cursor.readBE<ShortLength>();
    available -= sizeof(ShortLength);
  }
  return true;
}

bool FastCGITransaction::parseKeyValueContent(Cursor& cursor,
                                              size_t& available,
                                              size_t& length,
                                              IOBufQueue& queue) {
  std::unique_ptr<IOBuf> buf;
  size_t len = cursor.cloneAtMost(buf, length);
  queue.append(std::move(buf));
  assert(length >= len);
  length -= len;
  assert(available >= len);
  available -= len;
  return (length == 0);
}

void FastCGITransaction::handleGetValue(std::unique_ptr<IOBuf> key_chain) {
  assert(m_requestId == 0);
  size_t key_length = key_chain ? key_chain->computeChainDataLength() : 0;
  Cursor cursor(key_chain.get());
  std::string key = cursor.readFixedString(key_length);
  m_session->handleGetValueResult(key);
}

void FastCGITransaction::handleInvalidRecord() {
  m_session->handleInvalidRecord();
}

////////////////////////////////////////////////////////////////////////////////

FastCGISession::FastCGISession()
  : m_phase(Phase::AT_RECORD_BEGIN),
    m_keepConn(true),
    m_maxConns(0),
    m_maxRequests(0) {
  m_transactions[0] = folly::make_unique<FastCGITransaction>(
                        this, 0, nullptr);
}

FastCGISession::~FastCGISession() {}

const FastCGISession::Version FastCGISession::k_version = 1;

const size_t FastCGISession::k_writeGrowth = 256;

size_t FastCGISession::onIngress(const IOBuf* chain) {
  if (m_phase == Phase::INVALID) {
    return 0;
  }
  assert(m_keepConn);

  size_t available = chain ? chain->computeChainDataLength() : 0;
  size_t avail = available;

  Cursor cursor(chain);
  while (m_phase != Phase::INVALID) {
    if (m_phase == Phase::AT_RECORD_BEGIN) {
      if (parseRecordBegin(cursor, avail)) {
        m_phase = Phase::INSIDE_RECORD_BODY;
      } else break;
    }
    if (m_phase == Phase::INSIDE_RECORD_BODY) {
      if (parseRecordBody(cursor, avail)) {
        m_phase = Phase::AT_RECORD_BODY_END;
      } else break;
    }
    if (m_phase == Phase::AT_RECORD_BODY_END) {
      if (parseRecordEnd(cursor, avail)) {
        m_phase = Phase::AT_RECORD_BEGIN;
      } else break;
    }
  }
  return available - avail;
}

void FastCGISession::setMaxConns(int max_conns) {
  assert(max_conns > 0);
  m_maxConns = max_conns;
}

void FastCGISession::setMaxRequests(int max_requests) {
  assert(max_requests > 0);
  m_maxRequests = max_requests;
}

const size_t FastCGISession::k_recordBeginLength = (
    sizeof(Version) +
    sizeof(FastCGISession::m_recordType) +
    sizeof(FastCGISession::m_requestId) +
    sizeof(FastCGISession::m_contentLength) +
    sizeof(FastCGISession::m_paddingLength) +
    k_recordReservedLength
  );

const size_t FastCGISession::k_recordReservedLength = 1;

bool FastCGISession::parseRecordBegin(Cursor& cursor, size_t& available) {
  assert(m_phase == Phase::AT_RECORD_BEGIN);
  if (available < FastCGISession::k_recordBeginLength) {
    return false;
  }
  Version version = cursor.readBE<Version>();
  if (version != k_version) {
    handleInvalidRecord();
    return false;
  }
  m_recordType = static_cast<RecordType>(cursor.readBE<uint8_t>());
  m_requestId = cursor.readBE<decltype(m_requestId)>();
  m_contentLength = cursor.readBE<decltype(m_contentLength)>();
  m_contentLeft = m_contentLength;
  m_paddingLength = cursor.readBE<decltype(m_paddingLength)>();
  cursor.skip(k_recordReservedLength);
  available -= k_recordBeginLength;
  return true;
}

bool FastCGISession::parseRecordBody(Cursor& cursor, size_t& available) {
  assert(m_phase == Phase::INSIDE_RECORD_BODY);

  if (m_contentLength > 0 && available == 0) {
    return false;
  }

  size_t avail = available;
  bool unknown = false;

  if (m_requestId == 0 ||
      m_recordType == RecordType::BEGIN_REQUEST ||
      hasTransaction(m_requestId)) {
    switch (m_recordType) {
      case RecordType::BEGIN_REQUEST:
        parseBeginRequest(cursor, avail);
        break;
      case RecordType::ABORT_REQUEST:
        parseAbortRequest(cursor, avail);
        break;
      case RecordType::PARAMS:
        parseParams(cursor, avail);
        break;
      case RecordType::STDIN:
        parseStdIn(cursor, avail);
        break;
      case RecordType::DATA:
        parseData(cursor, avail);
        break;
      case RecordType::GET_VALUES:
        parseGetValues(cursor, avail);
        break;
      default:
        parseIgnoreRecord(cursor, avail);
        if (m_requestId == 0) {
          unknown = true;
        } else {
          handleInvalidRecord();
        }
    }
  } else {
    parseIgnoreRecord(cursor, avail);
  }

  size_t length = available - avail;
  available = avail;
  assert(m_contentLength >= m_contentLeft);
  assert(m_contentLeft >= length);
  m_contentLeft -= length;

  if (unknown && m_contentLeft == 0) {
    writeUnknownType(m_recordType);
  }

  return (m_contentLeft == 0);
}

bool FastCGISession::parseRecordEnd(Cursor& cursor, size_t& available) {
  assert(m_phase == Phase::AT_RECORD_BODY_END);
  size_t length = cursor.skipAtMost(m_paddingLength);
  assert(available >= length);
  available -= length;
  assert(m_paddingLength >= length);
  m_paddingLength -= length;
  if (m_paddingLength != 0) {
    return false;
  }
  return true;
}

const size_t FastCGISession::k_beginRequestLength = (
    sizeof(Role) +
    sizeof(ConnectionFlags) +
    FastCGISession::k_beginRequestReservedLength
  );

const size_t FastCGISession::k_beginRequestReservedLength = 5;

const size_t FastCGISession::k_endRequestLength = (
    sizeof(AppStatus) +
    sizeof(ProtoStatus) +
    FastCGISession::k_endRequestReservedLength
  );

const size_t FastCGISession::k_endRequestReservedLength = 3;

void FastCGISession::parseBeginRequest(Cursor& cursor, size_t& available) {
  if (m_contentLeft != k_beginRequestLength) {
    handleInvalidRecord();
    return;
  }
  if (available < k_beginRequestLength) {
    return;
  }
  Role role = static_cast<Role>(cursor.readBE<uint16_t>());
  ConnectionFlags flags = static_cast<ConnectionFlags>(
                            cursor.readBE<uint8_t>());
  cursor.skip(k_beginRequestReservedLength);
  available -= k_beginRequestLength;
  handleBeginRequest(role, flags);
}

const size_t FastCGISession::k_abortRequestLength = 0;

void FastCGISession::parseAbortRequest(Cursor& cursor, size_t& available) {
  if (m_contentLeft != k_abortRequestLength) {
    handleInvalidRecord();
    return;
  }
  // empty body
  handleAbortRequest();
}

void FastCGISession::parseParams(Cursor& cursor, size_t& available) {
  std::unique_ptr<IOBuf> chain;
  size_t length = cursor.cloneAtMost(chain, m_contentLeft);
  available -= length;
  handleParams(std::move(chain));
}

void FastCGISession::parseStdIn(Cursor& cursor, size_t& available) {
  std::unique_ptr<IOBuf> chain;
  size_t length = cursor.cloneAtMost(chain, m_contentLeft);
  available -= length;
  handleStdIn(std::move(chain));
}

void FastCGISession::parseData(Cursor& cursor, size_t& available) {
  std::unique_ptr<IOBuf> chain;
  size_t length = cursor.cloneAtMost(chain, m_contentLeft);
  available -= length;
  handleData(std::move(chain));
}

void FastCGISession::parseGetValues(Cursor& cursor, size_t& available) {
  std::unique_ptr<IOBuf> chain;
  size_t length = cursor.cloneAtMost(chain, m_contentLeft);
  available -= length;
  handleGetValues(std::move(chain));
}

void FastCGISession::parseIgnoreRecord(Cursor& cursor, size_t& available) {
  size_t length = cursor.skipAtMost(m_contentLeft);
  available -= length;
}

void FastCGISession::handleBeginRequest(Role role, ConnectionFlags flags) {
  if (m_requestId == 0 || hasTransaction(m_requestId)) {
    handleInvalidRecord();
    return;
  }
  if (role != Role::RESPONDER) {
    writeEndRequest(m_requestId, 0, ProtoStatus::UNKNOWN_ROLE);
    return;
  }
  beginTransaction(m_requestId);
  m_keepConn = (flags & ConnectionFlags::KEEP_CONN);
}

void FastCGISession::handleAbortRequest() {
  if (m_requestId == 0 || !hasTransaction(m_requestId)) {
    handleInvalidRecord();
    return;
  }
  endTransaction(m_requestId);
  writeEndRequest(m_requestId, 1, ProtoStatus::REQUEST_COMPLETE);
  if (!m_keepConn) {
    handleClose();
  }
}

void FastCGISession::handleStdIn(std::unique_ptr<IOBuf> chain) {
  if (m_requestId == 0 || !hasTransaction(m_requestId)) {
    handleInvalidRecord();
    return;
  }
  getTransaction(m_requestId)->onStdIn(std::move(chain));
}

void FastCGISession::handleData(std::unique_ptr<IOBuf> chain) {
  if (m_requestId == 0 || !hasTransaction(m_requestId)) {
    handleInvalidRecord();
    return;
  }
  getTransaction(m_requestId)->onData(std::move(chain));
}

void FastCGISession::handleParams(std::unique_ptr<IOBuf> chain) {
  getTransaction(m_requestId)->onParams(std::move(chain));
}

void FastCGISession::handleGetValues(std::unique_ptr<IOBuf> chain) {
  if (m_requestId != 0) {
    handleInvalidRecord();
    return;
  }
  getTransaction(m_requestId)->onGetValues(std::move(chain));
}

void FastCGISession::handleStdOut(RequestId request_id,
                                  std::unique_ptr<IOBuf> chain) {
  writeStdOut(request_id, std::move(chain));
}

void FastCGISession::handleStdErr(RequestId request_id,
                                  std::unique_ptr<IOBuf> chain) {
  writeStdErr(request_id, std::move(chain));
}

void FastCGISession::handleComplete(RequestId request_id) {
  writeEndRequest(request_id, 0, ProtoStatus::REQUEST_COMPLETE);
  endTransaction(request_id);
  if (!m_keepConn) {
    handleClose();
  }
}

const std::string FastCGISession::k_getValueMaxConnKey =
  "FCGI_MAX_CONNS";
const std::string FastCGISession::k_getValueMaxRequestsKey =
  "FCGI_MAX_REQS";
const std::string FastCGISession::k_getValueMultiplexConnsKey =
  "FCGI_MPXS_CONNS";

void FastCGISession::handleGetValueResult(const std::string& key) {
  std::string value;
  if (key == k_getValueMaxConnKey) {
    assert(m_maxConns > 0);
    value = std::to_string(m_maxConns);
    writeGetValueResult(key, value);
  } else if (key == k_getValueMaxRequestsKey) {
    assert(m_maxRequests > 0);
    value = std::to_string(m_maxRequests);
    writeGetValueResult(key, value);
  } else if (key == k_getValueMultiplexConnsKey) {
    value = "1"; // The implementation does support connection
                 // multiplexing.
    writeGetValueResult(key, value);
  } else {
    // No-op we are supposed to ignore the keys that we
    // don't understand.
  }
}

void FastCGISession::handleInvalidRecord() {
  LOG(ERROR) << "FastCGI protocol: received an invalid record";
  m_phase = Phase::INVALID;
  if (m_callback) {
    m_callback->onSessionError();
  }
}

void FastCGISession::handleClose() {
  if (m_callback) {
    m_callback->onSessionClose();
  }
}

void FastCGISession::writeEgress(std::unique_ptr<IOBuf> chain) {
  if (m_callback) {
    m_callback->onSessionEgress(std::move(chain));
  }
}

void FastCGISession::writeEndRequest(RequestId request_id,
                                     AppStatus app_status,
                                     ProtoStatus proto_status) {
  std::unique_ptr<IOBuf> chain(IOBuf::create(0));
  Appender appender(chain.get(), k_writeGrowth);
  size_t content_length = k_endRequestLength;
  size_t padding_length = getPaddingLength(content_length);
  appendRecordBegin(appender,
                    RecordType::END_REQUEST,
                    request_id,
                    content_length,
                    padding_length);
  appender.writeBE<AppStatus>(app_status);
  appender.writeBE<uint8_t>(static_cast<uint8_t>(proto_status));
  appender.ensure(k_endRequestReservedLength);
  memset(appender.writableData(), 0, k_endRequestReservedLength);
  appender.append(k_endRequestReservedLength);
  appendRecordEnd(appender, padding_length);
  writeEgress(std::move(chain));
}

void FastCGISession::writeStream(RequestId request_id,
                                 RecordType record_type,
                                 std::unique_ptr<IOBuf> stream_chain) {
  assert(record_type == RecordType::STDOUT ||
        record_type == RecordType::STDERR);
  if (stream_chain == nullptr) {
    return; // Nothing to do.
  }
  assert(stream_chain != nullptr);
  IOBufQueue queue(IOBufQueue::cacheChainLength());
  QueueAppender appender(&queue, k_writeGrowth);
  Cursor cursor(stream_chain.get());
  size_t available = stream_chain->computeChainDataLength();
  while (available > 0) {
    size_t content_length = std::min(
      (size_t) std::numeric_limits<ContentLength>::max(), available);
    size_t padding_length = getPaddingLength(content_length);
    appendRecordBegin(appender,
                      record_type,
                      request_id,
                      content_length,
                      padding_length);
    std::unique_ptr<IOBuf> chunk;
    cursor.clone(chunk, content_length);
    available -= content_length;
    appender.insert(std::move(chunk));
    appendRecordEnd(appender, padding_length);
  }
  writeEgress(queue.move());
}

void FastCGISession::writeStdOut(RequestId request_id,
                                 std::unique_ptr<IOBuf> chain) {
  writeStream(request_id, RecordType::STDOUT, std::move(chain));
}

void FastCGISession::writeStdErr(RequestId request_id,
                                 std::unique_ptr<IOBuf> chain) {
  writeStream(request_id, RecordType::STDERR, std::move(chain));
}

const FastCGISession::ShortLength FastCGISession::k_longLengthFlag = 1 << 7;

void FastCGISession::writeGetValueResult(const std::string& key,
                                         const std::string& value) {
  std::unique_ptr<IOBuf> chain(IOBuf::create(0));
  Appender cursor(chain.get(), k_writeGrowth);
  cursor.ensure(k_recordBeginLength);
  memset(cursor.writableData(), 0, k_recordBeginLength);
  cursor.append(k_recordBeginLength);
  if (key.size() & k_longLengthFlag) {
    cursor.writeBE<LongLength>(key.size() | (k_longLengthFlag << 24));
  } else {
    cursor.writeBE<ShortLength>(key.size());
  }
  if (value.size() & k_longLengthFlag) {
    cursor.writeBE<LongLength>(value.size() | (k_longLengthFlag << 24));
  } else {
    cursor.writeBE<ShortLength>(value.size());
  }
  // These instructions perform copying internally
  cursor.push(reinterpret_cast<const uint8_t*>(key.c_str()), key.size());
  cursor.push(reinterpret_cast<const uint8_t*>(value.c_str()), value.size());
  size_t content_length = chain->computeChainDataLength() - k_recordBeginLength;
  size_t padding_length = getPaddingLength(content_length);
  appendRecordEnd(cursor, padding_length);
  RWPrivateCursor begin(chain.get());
  prependRecordBegin(begin,
                     RecordType::GET_VALUES_RESULT,
                     0, // This is a management record
                     content_length,
                     padding_length);
  writeEgress(std::move(chain));
}

const size_t FastCGISession::k_unknownTypeLength = (
    sizeof(RecordType) +
    k_unknownTypeReservedLength
  );
const size_t FastCGISession::k_unknownTypeReservedLength = 7;

void FastCGISession::writeUnknownType(RecordType record_type) {
  std::unique_ptr<IOBuf> chain(IOBuf::create(0));
  Appender cursor(chain.get(), k_writeGrowth);
  size_t padding_length = getPaddingLength(k_unknownTypeLength);
  appendRecordBegin(cursor,
                    RecordType::UNKNOWN_TYPE,
                    0, // This is a management record
                    k_unknownTypeLength,
                    padding_length);
  cursor.writeBE<uint8_t>(static_cast<uint8_t>(record_type));
  cursor.ensure(k_unknownTypeReservedLength);
  memset(cursor.writableData(), 0, k_unknownTypeReservedLength);
  cursor.append(k_unknownTypeReservedLength);
  appendRecordEnd(cursor, padding_length);
  writeEgress(std::move(chain));
}

void FastCGISession::prependRecordBegin(RWPrivateCursor& cursor,
                                        RecordType record_type,
                                        RequestId request_id,
                                        size_t content_length,
                                        size_t padding_length) {
  cursor.writeBE<Version>(k_version);
  cursor.writeBE<uint8_t>(static_cast<uint8_t>(record_type));
  cursor.writeBE<RequestId>(request_id);
  assert(content_length <= std::numeric_limits<uint16_t>::max());
  cursor.writeBE<uint16_t>(content_length);
  assert(padding_length <= std::numeric_limits<uint8_t>::max());
  cursor.writeBE<uint8_t>(padding_length);
  cursor.skip(k_recordReservedLength);
}

size_t FastCGISession::getPaddingLength(size_t content_length) {
  // Align to 8-byte boundry.
  return (8 - ((k_recordBeginLength + content_length) % 8)) % 8;
}

void FastCGISession::beginTransaction(RequestId request_id) {
  assert(request_id != 0);
  assert(!m_transactions.count(request_id));
  // TODO: Make transactions reusable for performance.
  assert(m_callback != nullptr);
  auto handler = m_callback->newSessionHandler(request_id);
  m_transactions[request_id] = folly::make_unique<Transaction>(
                                 this, request_id, handler);
}

bool FastCGISession::hasTransaction(RequestId request_id) {
  return m_transactions.count(request_id);
}

std::unique_ptr<FastCGISession::Transaction>& FastCGISession::getTransaction(
                                                        RequestId request_id) {
  assert(m_transactions.count(request_id));
  return m_transactions[request_id];
}

void FastCGISession::endTransaction(RequestId request_id) {
  assert(request_id != 0);
  assert(m_transactions.count(request_id));
  // TODO: Make transactions reusable for performance.
  m_transactions.erase(request_id);
}

////////////////////////////////////////////////////////////////////////////////
}

