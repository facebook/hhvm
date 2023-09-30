/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "AsciiSerialized.h"

#include "mcrouter/lib/IOBufUtil.h"
#include "mcrouter/lib/McResUtil.h"

namespace facebook {
namespace memcache {

namespace {
const char* errorResultStr(const carbon::Result result) {
  switch (result) {
    case carbon::Result::OOO:
      return "SERVER_ERROR out of order\r\n";
    case carbon::Result::TIMEOUT:
      return "SERVER_ERROR timeout\r\n";
    case carbon::Result::CONNECT_TIMEOUT:
      return "SERVER_ERROR connection timeout\r\n";
    case carbon::Result::CONNECT_ERROR:
      return "SERVER_ERROR connection error\r\n";
    case carbon::Result::BUSY:
      return "SERVER_ERROR 307 busy\r\n";
    case carbon::Result::RES_TRY_AGAIN:
      return "SERVER_ERROR 302 try again\r\n";
    case carbon::Result::SHUTDOWN:
      return "SERVER_ERROR 301 shutdown\r\n";
    case carbon::Result::TKO:
      return "SERVER_ERROR unavailable\r\n";
    case carbon::Result::DEADLINE_EXCEEDED:
      return "SERVER_ERROR request deadline exceeded\r\n";
    case carbon::Result::BAD_COMMAND:
      return "CLIENT_ERROR bad command\r\n";
    case carbon::Result::BAD_KEY:
      return "CLIENT_ERROR bad key\r\n";
    case carbon::Result::BAD_FLAGS:
      return "CLIENT_ERROR bad flags\r\n";
    case carbon::Result::BAD_EXPTIME:
      return "CLIENT_ERROR bad exptime\r\n";
    case carbon::Result::BAD_LEASE_ID:
      return "CLIENT_ERROR bad lease_id\r\n";
    case carbon::Result::BAD_CAS_ID:
      return "CLIENT_ERROR bad cas_id\r\n";
    case carbon::Result::BAD_VALUE:
      return "SERVER_ERROR bad value\r\n";
    case carbon::Result::ABORTED:
      return "SERVER_ERROR aborted\r\n";
    case carbon::Result::CLIENT_ERROR:
      return "CLIENT_ERROR\r\n";
    case carbon::Result::LOCAL_ERROR:
      return "SERVER_ERROR local error\r\n";
    case carbon::Result::REMOTE_ERROR:
      return "SERVER_ERROR remote error\r\n";
    case carbon::Result::PERMISSION_DENIED:
      return "SERVER_ERROR permission denied\r\n";
    case carbon::Result::HOT_KEY:
      return "SERVER_ERROR hot key\r\n";
    default:
      return "SERVER_ERROR unknown result\r\n";
  }
}
} // anonymous namespace

size_t AsciiSerializedRequest::getSize() const {
  return iovsTotalLen_;
}

void AsciiSerializedRequest::addString(folly::ByteRange range) {
  assert(iovsCount_ < kMaxIovs);
  auto bufLen = range.size();
  iovs_[iovsCount_].iov_base = const_cast<unsigned char*>(range.begin());
  iovs_[iovsCount_].iov_len = bufLen;
  ++iovsCount_;
  iovsTotalLen_ += bufLen;
}

void AsciiSerializedRequest::addString(folly::StringPiece str) {
  // cause implicit conversion.
  addString(folly::ByteRange(str));
}

template <class Request>
void AsciiSerializedRequest::keyValueRequestCommon(
    folly::StringPiece prefix,
    const Request& request) {
  auto value = coalesceAndGetRange(request.value_ref());
  auto len = snprintf(
      printBuffer_,
      kMaxBufferLength,
      " %lu %d %zd\r\n",
      *request.flags_ref(),
      *request.exptime_ref(),
      value.size());
  assert(len > 0 && static_cast<size_t>(len) < kMaxBufferLength);
  addStrings(
      prefix,
      request.key_ref()->fullKey(),
      folly::StringPiece(printBuffer_, static_cast<size_t>(len)),
      value,
      "\r\n");
}

// Get-like ops.
void AsciiSerializedRequest::prepareImpl(const McGetRequest& request) {
  addStrings("get ", request.key_ref()->fullKey(), "\r\n");
}

void AsciiSerializedRequest::prepareImpl(const McGetsRequest& request) {
  addStrings("gets ", request.key_ref()->fullKey(), "\r\n");
}

void AsciiSerializedRequest::prepareImpl(const McMetagetRequest& request) {
  addStrings("metaget ", request.key_ref()->fullKey(), "\r\n");
}

void AsciiSerializedRequest::prepareImpl(const McLeaseGetRequest& request) {
  addStrings("lease-get ", request.key_ref()->fullKey(), "\r\n");
}

void AsciiSerializedRequest::prepareImpl(const McGatRequest& request) {
  auto len =
      snprintf(printBuffer_, kMaxBufferLength, "%d ", *request.exptime_ref());
  addStrings(
      "gat ",
      folly::StringPiece(printBuffer_, static_cast<size_t>(len)),
      request.key_ref()->fullKey(),
      "\r\n");
}

void AsciiSerializedRequest::prepareImpl(const McGatsRequest& request) {
  auto len =
      snprintf(printBuffer_, kMaxBufferLength, "%d ", *request.exptime_ref());
  addStrings(
      "gats ",
      folly::StringPiece(printBuffer_, static_cast<size_t>(len)),
      request.key_ref()->fullKey(),
      "\r\n");
}

// Update-like ops.
void AsciiSerializedRequest::prepareImpl(const McSetRequest& request) {
  keyValueRequestCommon("set ", request);
}

void AsciiSerializedRequest::prepareImpl(const McAddRequest& request) {
  keyValueRequestCommon("add ", request);
}

void AsciiSerializedRequest::prepareImpl(const McReplaceRequest& request) {
  keyValueRequestCommon("replace ", request);
}

void AsciiSerializedRequest::prepareImpl(const McAppendRequest& request) {
  keyValueRequestCommon("append ", request);
}

void AsciiSerializedRequest::prepareImpl(const McPrependRequest& request) {
  keyValueRequestCommon("prepend ", request);
}

void AsciiSerializedRequest::prepareImpl(const McCasRequest& request) {
  auto value = coalesceAndGetRange(request.value_ref());
  auto len = snprintf(
      printBuffer_,
      kMaxBufferLength,
      " %lu %d %zd %lu\r\n",
      *request.flags_ref(),
      *request.exptime_ref(),
      value.size(),
      *request.casToken_ref());
  assert(len > 0 && static_cast<size_t>(len) < kMaxBufferLength);
  addStrings(
      "cas ",
      request.key_ref()->fullKey(),
      folly::StringPiece(printBuffer_, static_cast<size_t>(len)),
      value,
      "\r\n");
}

void AsciiSerializedRequest::prepareImpl(const McLeaseSetRequest& request) {
  auto value = coalesceAndGetRange(request.value_ref());
  auto len = snprintf(
      printBuffer_,
      kMaxBufferLength,
      " %lu %lu %d %zd\r\n",
      *request.leaseToken_ref(),
      *request.flags_ref(),
      *request.exptime_ref(),
      value.size());
  assert(len > 0 && static_cast<size_t>(len) < kMaxBufferLength);
  addStrings(
      "lease-set ",
      request.key_ref()->fullKey(),
      folly::StringPiece(printBuffer_, static_cast<size_t>(len)),
      value,
      "\r\n");
}

// Arithmetic ops.
void AsciiSerializedRequest::prepareImpl(const McIncrRequest& request) {
  auto len = snprintf(
      printBuffer_, kMaxBufferLength, " %lu\r\n", *request.delta_ref());
  assert(len > 0 && static_cast<size_t>(len) < kMaxBufferLength);
  addStrings(
      "incr ",
      request.key_ref()->fullKey(),
      folly::StringPiece(printBuffer_, static_cast<size_t>(len)));
}

void AsciiSerializedRequest::prepareImpl(const McDecrRequest& request) {
  auto len = snprintf(
      printBuffer_, kMaxBufferLength, " %lu\r\n", *request.delta_ref());
  assert(len > 0 && static_cast<size_t>(len) < kMaxBufferLength);
  addStrings(
      "decr ",
      request.key_ref()->fullKey(),
      folly::StringPiece(printBuffer_, static_cast<size_t>(len)));
}

// Delete op.
void AsciiSerializedRequest::prepareImpl(const McDeleteRequest& request) {
  addStrings("delete ", request.key_ref()->fullKey());
  if (*request.exptime_ref() != 0) {
    auto len = snprintf(
        printBuffer_, kMaxBufferLength, " %d\r\n", *request.exptime_ref());
    assert(len > 0 && static_cast<size_t>(len) < kMaxBufferLength);
    addString(folly::StringPiece(printBuffer_, static_cast<size_t>(len)));
  } else {
    addString("\r\n");
  }
}

// Touch op.
void AsciiSerializedRequest::prepareImpl(const McTouchRequest& request) {
  auto len = snprintf(
      printBuffer_, kMaxBufferLength, " %u\r\n", *request.exptime_ref());
  assert(len > 0 && static_cast<size_t>(len) < kMaxBufferLength);
  addStrings(
      "touch ",
      request.key_ref()->fullKey(),
      folly::StringPiece(printBuffer_, static_cast<size_t>(len)));
}

// Version op.
void AsciiSerializedRequest::prepareImpl(const McVersionRequest&) {
  addString("version\r\n");
}

// FlushAll op.

void AsciiSerializedRequest::prepareImpl(const McFlushAllRequest& request) {
  addString("flush_all");
  if (*request.delay_ref() != 0) {
    auto len =
        snprintf(printBuffer_, kMaxBufferLength, " %u", *request.delay_ref());
    assert(len > 0 && static_cast<size_t>(len) < kMaxBufferLength);
    addString(folly::StringPiece(printBuffer_, static_cast<size_t>(len)));
  }
  addString("\r\n");
}

void AsciiSerializedReply::clear() {
  iovsCount_ = 0;
  iobuf_.reset();
  auxString_.reset();
}

void AsciiSerializedReply::addString(folly::ByteRange range) {
  assert(iovsCount_ < kMaxIovs);
  iovs_[iovsCount_].iov_base = const_cast<unsigned char*>(range.begin());
  iovs_[iovsCount_].iov_len = range.size();
  ++iovsCount_;
}

void AsciiSerializedReply::addString(folly::StringPiece str) {
  // cause implicit conversion.
  addString(folly::ByteRange(str));
}

void AsciiSerializedReply::handleError(
    carbon::Result result,
    uint16_t errorCode,
    std::string&& message) {
  assert(isErrorResult(result));

  if (!message.empty()) {
    if (result == carbon::Result::CLIENT_ERROR) {
      addString("CLIENT_ERROR ");
    } else {
      addString("SERVER_ERROR ");
    }
    if (errorCode != 0) {
      const auto len =
          snprintf(printBuffer_, kMaxBufferLength, "%d ", errorCode);
      assert(len > 0);
      assert(static_cast<size_t>(len) < kMaxBufferLength);
      addString(folly::StringPiece(printBuffer_, static_cast<size_t>(len)));
    }
    auxString_ = std::move(message);
    addStrings(*auxString_, "\r\n");
  } else {
    addString(errorResultStr(result));
  }
}

void AsciiSerializedReply::handleUnexpected(
    carbon::Result result,
    const char* requestName) {
  assert(iovsCount_ == 0);

  // Note: this is not totally compatible with the old way of handling
  // unexpected behavior in mc_ascii_response_write_iovs()
  const auto len = snprintf(
      printBuffer_,
      kMaxBufferLength,
      "SERVER_ERROR unexpected result %s (%d) for %s\r\n",
      carbon::resultToString(result),
      static_cast<int32_t>(result),
      requestName);
  assert(len > 0);
  assert(static_cast<size_t>(len) < kMaxBufferLength);
  addString(folly::StringPiece(printBuffer_, static_cast<size_t>(len)));
}

// Get-like ops
void AsciiSerializedReply::prepareImpl(
    McGetReply&& reply,
    folly::StringPiece key) {
  if (isHitResult(*reply.result_ref())) {
    if (key.empty()) {
      // Multi-op hack: if key is empty, this is the END context
      if (isErrorResult(*reply.result_ref())) {
        handleError(
            *reply.result_ref(),
            *reply.appSpecificErrorCode_ref(),
            std::move(*reply.message_ref()));
      }
      addString("END\r\n");
    } else {
      const auto valueStr = coalesceAndGetRange(reply.value_ref());

      const auto len = snprintf(
          printBuffer_,
          kMaxBufferLength,
          " %lu %zu\r\n",
          *reply.flags_ref(),
          valueStr.size());
      assert(len > 0);
      assert(static_cast<size_t>(len) < kMaxBufferLength);

      addStrings(
          "VALUE ",
          key,
          folly::StringPiece(printBuffer_, static_cast<size_t>(len)));
      assert(!iobuf_.has_value());
      // value was coalesced in coalesceAndGetRange()
      if (reply.value_ref().has_value()) {
        iobuf_ = std::move(reply.value_ref().value());
      }
      addStrings(valueStr, "\r\n");
    }
  } else if (isErrorResult(*reply.result_ref())) {
    handleError(
        *reply.result_ref(),
        *reply.appSpecificErrorCode_ref(),
        std::move(*reply.message_ref()));
  } else {
    handleUnexpected(*reply.result_ref(), "get");
  }
}

void AsciiSerializedReply::prepareImpl(
    McGetsReply&& reply,
    folly::StringPiece key) {
  if (isHitResult(*reply.result_ref())) {
    const auto valueStr = coalesceAndGetRange(reply.value_ref());
    const auto len = snprintf(
        printBuffer_,
        kMaxBufferLength,
        " %lu %zu %lu\r\n",
        *reply.flags_ref(),
        valueStr.size(),
        *reply.casToken_ref());
    assert(len > 0);
    assert(static_cast<size_t>(len) < kMaxBufferLength);

    addStrings(
        "VALUE ",
        key,
        folly::StringPiece(printBuffer_, static_cast<size_t>(len)));
    assert(!iobuf_.has_value());
    // value was coalesced in coalescedAndGetRange()
    if (reply.value_ref().has_value()) {
      iobuf_ = std::move(reply.value_ref().value());
    }
    addStrings(valueStr, "\r\n");
  } else if (isErrorResult(*reply.result_ref())) {
    handleError(
        *reply.result_ref(),
        *reply.appSpecificErrorCode_ref(),
        std::move(*reply.message_ref()));
  } else {
    handleUnexpected(*reply.result_ref(), "gets");
  }
}

void AsciiSerializedReply::prepareImpl(
    McMetagetReply&& reply,
    folly::StringPiece key) {
  /**
   * META key age: (unknown|\d+); exptime: \d+;
   * from: (\d+\.\d+\.\d+\.\d+|unknown); is_transient: (1|0)\r\n
   *
   * age is at most 11 characters, with - sign.
   * exptime is at most 10 characters.
   * IP6 address is at most 39 characters.
   * To be safe, we set kMaxBufferLength = 100 bytes.
   */
  if (*reply.result_ref() == carbon::Result::FOUND) {
    // age
    std::string ageStr("unknown");
    if (*reply.age_ref() != -1) {
      ageStr = folly::to<std::string>(*reply.age_ref());
    }
    // exptime
    const auto exptimeStr = folly::to<std::string>(*reply.exptime_ref());
    // from
    std::string fromStr("unknown");
    if (!reply.ipAddress_ref()->empty()) { // assume valid IP
      fromStr = *reply.ipAddress_ref();
    }

    const auto len = snprintf(
        printBuffer_,
        kMaxBufferLength,
        "%s; exptime: %s; from: %s",
        ageStr.data(),
        exptimeStr.data(),
        fromStr.data());
    assert(len > 0);
    assert(static_cast<size_t>(len) < kMaxBufferLength);

    /* TODO(stuclar): Once mcrouter change to make ascii parsing of
     *  is_transient is deployed everywhere, remove is_transient.
     */
    addStrings(
        "META ",
        key,
        " age: ",
        folly::StringPiece(printBuffer_, static_cast<size_t>(len)),
        "; is_transient: 0\r\n");
  } else if (isErrorResult(*reply.result_ref())) {
    handleError(
        *reply.result_ref(),
        *reply.appSpecificErrorCode_ref(),
        std::move(*reply.message_ref()));
  } else {
    handleUnexpected(*reply.result_ref(), "metaget");
  }
}

void AsciiSerializedReply::prepareImpl(
    McLeaseGetReply&& reply,
    folly::StringPiece key) {
  const auto valueStr = coalesceAndGetRange(reply.value_ref());

  if (*reply.result_ref() == carbon::Result::FOUND) {
    const auto len = snprintf(
        printBuffer_,
        kMaxBufferLength,
        " %lu %zu\r\n",
        *reply.flags_ref(),
        valueStr.size());
    assert(len > 0);
    assert(static_cast<size_t>(len) < kMaxBufferLength);

    addStrings(
        "VALUE ",
        key,
        folly::StringPiece(printBuffer_, static_cast<size_t>(len)));
    assert(!iobuf_.has_value());
    // value was coalesced in coalescedAndGetRange()
    if (reply.value_ref().has_value()) {
      iobuf_ = std::move(reply.value_ref().value());
    }
    addStrings(valueStr, "\r\n");
  } else if (*reply.result_ref() == carbon::Result::NOTFOUND) {
    const auto len = snprintf(
        printBuffer_,
        kMaxBufferLength,
        " %lu %lu %zu\r\n",
        *reply.leaseToken_ref(),
        *reply.flags_ref(),
        valueStr.size());
    addStrings(
        "LVALUE ",
        key,
        folly::StringPiece(printBuffer_, static_cast<size_t>(len)));
    if (reply.value_ref().has_value()) {
      iobuf_ = std::move(reply.value_ref().value());
    }
    addStrings(valueStr, "\r\n");
  } else if (*reply.result_ref() == carbon::Result::NOTFOUNDHOT) {
    addString("NOT_FOUND_HOT\r\n");
  } else if (isErrorResult(*reply.result_ref())) {
    LOG(ERROR) << "Got reply result "
               << static_cast<size_t>(*reply.result_ref());
    handleError(
        *reply.result_ref(),
        *reply.appSpecificErrorCode_ref(),
        std::move(*reply.message_ref()));
  } else {
    LOG(ERROR) << "Got unexpected reply result "
               << static_cast<size_t>(*reply.result_ref());
    handleUnexpected(*reply.result_ref(), "lease-get");
  }
}

void AsciiSerializedReply::prepareImpl(
    McGatReply&& reply,
    folly::StringPiece key) {
  if (isHitResult(*reply.result_ref())) {
    if (key.empty()) {
      // Multi-op hack: if key is empty, this is the END context
      if (isErrorResult(*reply.result_ref())) {
        handleError(
            *reply.result_ref(),
            *reply.appSpecificErrorCode_ref(),
            std::move(*reply.message_ref()));
      }
      addString("END\r\n");
    } else {
      const auto valueStr = coalesceAndGetRange(reply.value_ref());

      const auto len = snprintf(
          printBuffer_,
          kMaxBufferLength,
          " %lu %zu\r\n",
          *reply.flags_ref(),
          valueStr.size());
      assert(len > 0);
      assert(static_cast<size_t>(len) < kMaxBufferLength);

      addStrings(
          "VALUE ",
          key,
          folly::StringPiece(printBuffer_, static_cast<size_t>(len)));
      assert(!iobuf_.has_value());
      // value was coalesced in coalesceAndGetRange()
      if (reply.value_ref().has_value()) {
        iobuf_ = std::move(reply.value_ref().value());
      }
      addStrings(valueStr, "\r\n");
    }
  } else if (isErrorResult(*reply.result_ref())) {
    handleError(
        *reply.result_ref(),
        *reply.appSpecificErrorCode_ref(),
        std::move(*reply.message_ref()));
  } else {
    handleUnexpected(*reply.result_ref(), "gat");
  }
}

void AsciiSerializedReply::prepareImpl(
    McGatsReply&& reply,
    folly::StringPiece key) {
  if (isHitResult(*reply.result_ref())) {
    const auto valueStr = coalesceAndGetRange(reply.value_ref());
    const auto len = snprintf(
        printBuffer_,
        kMaxBufferLength,
        " %lu %zu %lu\r\n",
        *reply.flags_ref(),
        valueStr.size(),
        *reply.casToken_ref());
    assert(len > 0);
    assert(static_cast<size_t>(len) < kMaxBufferLength);

    addStrings(
        "VALUE ",
        key,
        folly::StringPiece(printBuffer_, static_cast<size_t>(len)));
    assert(!iobuf_.has_value());
    // value was coalesced in coalescedAndGetRange()
    if (reply.value_ref().has_value()) {
      iobuf_ = std::move(reply.value_ref().value());
    }
    addStrings(valueStr, "\r\n");
  } else if (isErrorResult(*reply.result_ref())) {
    handleError(
        *reply.result_ref(),
        *reply.appSpecificErrorCode_ref(),
        std::move(*reply.message_ref()));
  } else {
    handleUnexpected(*reply.result_ref(), "gats");
  }
}

// Update-like ops
void AsciiSerializedReply::prepareUpdateLike(
    carbon::Result result,
    uint16_t errorCode,
    std::string&& message,
    const char* requestName) {
  if (isErrorResult(result)) {
    handleError(result, errorCode, std::move(message));
    return;
  }

  if (FOLLY_UNLIKELY(result == carbon::Result::OK)) {
    addString("STORED\r\n");
    return;
  }

  switch (result) {
    case carbon::Result::STORED:
      addString("STORED\r\n");
      break;
    case carbon::Result::STALESTORED:
      addString("STALE_STORED\r\n");
      break;
    case carbon::Result::FOUND:
      addString("FOUND\r\n");
      break;
    case carbon::Result::NOTSTORED:
      addString("NOT_STORED\r\n");
      break;
    case carbon::Result::NOTFOUND:
      addString("NOT_FOUND\r\n");
      break;
    case carbon::Result::EXISTS:
      addString("EXISTS\r\n");
      break;
    default:
      handleUnexpected(result, requestName);
      break;
  }
}

void AsciiSerializedReply::prepareImpl(McSetReply&& reply) {
  prepareUpdateLike(
      *reply.result_ref(),
      *reply.appSpecificErrorCode_ref(),
      std::move(*reply.message_ref()),
      "set");
}

void AsciiSerializedReply::prepareImpl(McAddReply&& reply) {
  prepareUpdateLike(
      *reply.result_ref(),
      *reply.appSpecificErrorCode_ref(),
      std::move(*reply.message_ref()),
      "add");
}

void AsciiSerializedReply::prepareImpl(McReplaceReply&& reply) {
  prepareUpdateLike(
      *reply.result_ref(),
      *reply.appSpecificErrorCode_ref(),
      std::move(*reply.message_ref()),
      "replace");
}

void AsciiSerializedReply::prepareImpl(McAppendReply&& reply) {
  prepareUpdateLike(
      *reply.result_ref(),
      *reply.appSpecificErrorCode_ref(),
      std::move(*reply.message_ref()),
      "append");
}

void AsciiSerializedReply::prepareImpl(McPrependReply&& reply) {
  prepareUpdateLike(
      *reply.result_ref(),
      *reply.appSpecificErrorCode_ref(),
      std::move(*reply.message_ref()),
      "prepend");
}

void AsciiSerializedReply::prepareImpl(McCasReply&& reply) {
  prepareUpdateLike(
      *reply.result_ref(),
      *reply.appSpecificErrorCode_ref(),
      std::move(*reply.message_ref()),
      "cas");
}

void AsciiSerializedReply::prepareImpl(McLeaseSetReply&& reply) {
  prepareUpdateLike(
      *reply.result_ref(),
      *reply.appSpecificErrorCode_ref(),
      std::move(*reply.message_ref()),
      "lease-set");
}

void AsciiSerializedReply::prepareArithmeticLike(
    carbon::Result result,
    const uint64_t delta,
    uint16_t errorCode,
    std::string&& message,
    const char* requestName) {
  if (isStoredResult(result)) {
    const auto len = snprintf(printBuffer_, kMaxBufferLength, "%lu\r\n", delta);
    assert(len > 0);
    assert(static_cast<size_t>(len) < kMaxBufferLength);
    addString(folly::StringPiece(printBuffer_, static_cast<size_t>(len)));
  } else if (isMissResult(result)) {
    addString("NOT_FOUND\r\n");
  } else if (isErrorResult(result)) {
    handleError(result, errorCode, std::move(message));
  } else {
    handleUnexpected(result, requestName);
  }
}

// Arithmetic-like ops
void AsciiSerializedReply::prepareImpl(McIncrReply&& reply) {
  prepareArithmeticLike(
      *reply.result_ref(),
      *reply.delta_ref(),
      *reply.appSpecificErrorCode_ref(),
      std::move(*reply.message_ref()),
      "incr");
}

void AsciiSerializedReply::prepareImpl(McDecrReply&& reply) {
  prepareArithmeticLike(
      *reply.result_ref(),
      *reply.delta_ref(),
      *reply.appSpecificErrorCode_ref(),
      std::move(*reply.message_ref()),
      "decr");
}

// Delete
void AsciiSerializedReply::prepareImpl(McDeleteReply&& reply) {
  if (*reply.result_ref() == carbon::Result::DELETED) {
    addString("DELETED\r\n");
  } else if (*reply.result_ref() == carbon::Result::NOTFOUND) {
    addString("NOT_FOUND\r\n");
  } else if (isErrorResult(*reply.result_ref())) {
    handleError(
        *reply.result_ref(),
        *reply.appSpecificErrorCode_ref(),
        std::move(*reply.message_ref()));
  } else {
    handleUnexpected(*reply.result_ref(), "delete");
  }
}

// Touch
void AsciiSerializedReply::prepareImpl(McTouchReply&& reply) {
  if (*reply.result_ref() == carbon::Result::TOUCHED) {
    addString("TOUCHED\r\n");
  } else if (*reply.result_ref() == carbon::Result::NOTFOUND) {
    addString("NOT_FOUND\r\n");
  } else if (isErrorResult(*reply.result_ref())) {
    handleError(
        *reply.result_ref(),
        *reply.appSpecificErrorCode_ref(),
        std::move(*reply.message_ref()));
  } else {
    handleUnexpected(*reply.result_ref(), "touch");
  }
}

// Version
void AsciiSerializedReply::prepareImpl(McVersionReply&& reply) {
  if (*reply.result_ref() == carbon::Result::OK) {
    // TODO(jmswen) Do something sane when version is empty
    addString("VERSION ");
    if (reply.value_ref().is_set() && !reply.value_ref()->empty()) {
      const auto valueStr = coalesceAndGetRange(reply.value_ref());
      assert(!iobuf_.has_value());
      // value was coalesced in coalesceAndGetRange()
      iobuf_ = std::move(*reply.value_ref());
      addString(valueStr);
    }
    addString("\r\n");
  } else if (isErrorResult(*reply.result_ref())) {
    handleError(
        *reply.result_ref(),
        *reply.appSpecificErrorCode_ref(),
        std::move(*reply.message_ref()));
  } else {
    handleUnexpected(*reply.result_ref(), "version");
  }
}

// Stats
void AsciiSerializedReply::prepareImpl(McStatsReply&& reply) {
  if (*reply.result_ref() == carbon::Result::OK) {
    if (!reply.stats_ref()->empty()) {
      auxString_ = folly::join("\r\n", *reply.stats_ref());
      addStrings(*auxString_, "\r\n");
    }
    addString("END\r\n");
  } else if (isErrorResult(*reply.result_ref())) {
    handleError(
        *reply.result_ref(),
        *reply.appSpecificErrorCode_ref(),
        std::move(*reply.message_ref()));
  } else {
    handleUnexpected(*reply.result_ref(), "stats");
  }
}

// FlushAll
void AsciiSerializedReply::prepareImpl(McFlushAllReply&& reply) {
  if (isErrorResult(*reply.result_ref())) {
    handleError(
        *reply.result_ref(),
        *reply.appSpecificErrorCode_ref(),
        std::move(*reply.message_ref()));
  } else { // Don't handleUnexpected(), just return OK
    addString("OK\r\n");
  }
}

// FlushRe
void AsciiSerializedReply::prepareImpl(McFlushReReply&& reply) {
  if (isErrorResult(*reply.result_ref())) {
    handleError(
        *reply.result_ref(),
        *reply.appSpecificErrorCode_ref(),
        std::move(*reply.message_ref()));
  } else { // Don't handleUnexpected(), just return OK
    addString("OK\r\n");
  }
}

// Exec
void AsciiSerializedReply::prepareImpl(McExecReply&& reply) {
  if (*reply.result_ref() == carbon::Result::OK) {
    if (!reply.response_ref()->empty()) {
      auxString_ = std::move(*reply.response_ref());
      addStrings(*auxString_, "\r\n");
    } else {
      addString("OK\r\n");
    }
  } else if (isErrorResult(*reply.result_ref())) {
    handleError(
        *reply.result_ref(),
        *reply.appSpecificErrorCode_ref(),
        std::move(*reply.message_ref()));
  } else {
    handleUnexpected(*reply.result_ref(), "exec");
  }
}

// Shutdown
void AsciiSerializedReply::prepareImpl(McShutdownReply&& reply) {
  if (*reply.result_ref() == carbon::Result::OK) {
    addString("OK\r\n");
  } else if (isErrorResult(*reply.result_ref())) {
    handleError(
        *reply.result_ref(),
        *reply.appSpecificErrorCode_ref(),
        std::move(*reply.message_ref()));
  } else {
    handleUnexpected(*reply.result_ref(), "shutdown");
  }
}
} // namespace memcache
} // namespace facebook
