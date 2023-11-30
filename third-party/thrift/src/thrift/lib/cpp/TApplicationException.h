/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef _THRIFT_TAPPLICATIONEXCEPTION_H_
#define _THRIFT_TAPPLICATIONEXCEPTION_H_ 1

#include <folly/Utility.h>
#include <thrift/lib/cpp/Thrift.h>
#include <thrift/lib/cpp/protocol/TProtocol.h>

namespace apache {
namespace thrift {

namespace protocol {
class TProtocol;
}

/**
 * This class is thrown when some high-level communication errors with
 * the remote peer occur, and also when a server throws an unexpected
 * exception from a handler method.  Because of the latter case, this
 * class can be serialized.
 */
class FOLLY_EXPORT TApplicationException : public TException {
 public:
  /**
   * Error codes for the various types of exceptions.
   */
  enum TApplicationExceptionType {
    UNKNOWN = 0,
    UNKNOWN_METHOD = 1,
    INVALID_MESSAGE_TYPE = 2,
    WRONG_METHOD_NAME = 3,
    BAD_SEQUENCE_ID = 4,
    MISSING_RESULT = 5,
    INTERNAL_ERROR = 6,
    PROTOCOL_ERROR = 7,
    INVALID_TRANSFORM = 8,
    INVALID_PROTOCOL = 9,
    UNSUPPORTED_CLIENT_TYPE = 10,
    LOADSHEDDING = 11,
    TIMEOUT = 12,
    INJECTED_FAILURE = 13,
    CHECKSUM_MISMATCH = 14,
    INTERRUPTION = 15,
    TENANT_QUOTA_EXCEEDED = 16,
  };

  TApplicationException() : type_(UNKNOWN) {}

  explicit TApplicationException(TApplicationExceptionType type)
      : type_(type) {}

  explicit TApplicationException(const std::string& message)
      : message_(message), type_(UNKNOWN) {}

  TApplicationException(
      TApplicationExceptionType type, const std::string& message)
      : message_(message), type_(type) {}

  ~TApplicationException() noexcept override {}

  const std::string& getMessage() const { return message_; }

  /**
   * Returns an error code that provides information about the type of error
   * that has occurred.
   *
   * @return Error code
   */
  TApplicationExceptionType getType() const { return type_; }

  void setMessage(std::string&& msg) { message_ = std::move(msg); }

  void setType(TApplicationExceptionType type) { type_ = type; }

  const char* what() const noexcept override {
    if (message_.empty()) {
      switch (type_) {
        case UNKNOWN:
          return "TApplicationException: Unknown application exception";
        case UNKNOWN_METHOD:
          return "TApplicationException: Unknown method";
        case INVALID_MESSAGE_TYPE:
          return "TApplicationException: Invalid message type";
        case WRONG_METHOD_NAME:
          return "TApplicationException: Wrong method name";
        case BAD_SEQUENCE_ID:
          return "TApplicationException: Bad sequence identifier";
        case MISSING_RESULT:
          return "TApplicationException: Missing result";
        case INTERNAL_ERROR:
          return "TApplicationException: Internal error";
        case PROTOCOL_ERROR:
          return "TApplicationException: Protocol error";
        case INVALID_TRANSFORM:
          return "TApplicationException: Invalid transform";
        case INVALID_PROTOCOL:
          return "TApplicationException: Invalid protocol";
        case UNSUPPORTED_CLIENT_TYPE:
          return "TApplicationException: Unsupported client type";
        case LOADSHEDDING:
          return "TApplicationException: Loadshedding";
        case TIMEOUT:
          return "TApplicationException: Timeout";
        case INJECTED_FAILURE:
          return "TApplicationException: Injected failure";
        case CHECKSUM_MISMATCH:
          return "TApplicationException: Checksum mismatch";
        case INTERRUPTION:
          return "TApplicationException: interruption";
        case TENANT_QUOTA_EXCEEDED:
          return "TApplicationException: Tenant quota exceeded";
        default:
          return "TApplicationException: (Invalid exception type)";
      }
    } else {
      return message_.c_str();
    }
  }

  uint32_t read(apache::thrift::protocol::TProtocol* iprot) {
    uint32_t xfer = 0;
    std::string fname;
    apache::thrift::protocol::TType ftype;
    int16_t fid;

    xfer += iprot->readStructBegin(fname);

    while (true) {
      xfer += iprot->readFieldBegin(fname, ftype, fid);
      if (ftype == apache::thrift::protocol::T_STOP) {
        break;
      }
      switch (fid) {
        case 1:
          if (ftype == apache::thrift::protocol::T_STRING) {
            xfer += iprot->readString(message_);
          } else {
            xfer += iprot->skip(ftype);
          }
          break;
        case 2:
          if (ftype == apache::thrift::protocol::T_I32) {
            int32_t type;
            xfer += iprot->readI32(type);
            type_ = static_cast<TApplicationExceptionType>(type);
          } else {
            xfer += iprot->skip(ftype);
          }
          break;
        default:
          xfer += iprot->skip(ftype);
          break;
      }
      xfer += iprot->readFieldEnd();
    }

    xfer += iprot->readStructEnd();
    return xfer;
  }

  template <class Protocol_>
  uint32_t read(Protocol_* iprot) {
    uint32_t xfer = folly::to_narrow(iprot->getCursorPosition());
    std::string fname;
    apache::thrift::protocol::TType ftype;
    int16_t fid;

    iprot->readStructBegin(fname);

    while (true) {
      iprot->readFieldBegin(fname, ftype, fid);
      if (ftype == apache::thrift::protocol::T_STOP) {
        break;
      }
      switch (fid) {
        case 1:
          if (ftype == apache::thrift::protocol::T_STRING) {
            iprot->readString(message_);
          } else {
            iprot->skip(ftype);
          }
          break;
        case 2:
          if (ftype == apache::thrift::protocol::T_I32) {
            int32_t type;
            iprot->readI32(type);
            type_ = static_cast<TApplicationExceptionType>(type);
          } else {
            iprot->skip(ftype);
          }
          break;
        default:
          iprot->skip(ftype);
          break;
      }
      iprot->readFieldEnd();
    }

    iprot->readStructEnd();
    return folly::to_narrow(iprot->getCursorPosition() - xfer);
  }

  template <class Protocol_>
  uint32_t write(Protocol_* prot_) const {
    uint32_t xfer = 0;
    xfer += prot_->writeStructBegin("TApplicationException");
    xfer += prot_->writeFieldBegin(
        "message_", apache::thrift::protocol::T_STRING, 1);
    xfer += prot_->writeString(message_);
    xfer += prot_->writeFieldEnd();
    xfer += prot_->writeFieldBegin("type_", apache::thrift::protocol::T_I32, 2);
    xfer += prot_->writeI32(static_cast<int32_t>(type_));
    xfer += prot_->writeFieldEnd();
    xfer += prot_->writeFieldStop();
    xfer += prot_->writeStructEnd();
    return xfer;
  }

  template <class Protocol_>
  uint32_t serializedSize(Protocol_* prot_) const {
    uint32_t xref = 0;
    xref += prot_->serializedStructSize("TApplicationException");
    xref += prot_->serializedFieldSize(
        "message_", apache::thrift::protocol::T_STRING, 1);
    xref += prot_->serializedSizeString(message_);
    xref +=
        prot_->serializedFieldSize("type_", apache::thrift::protocol::T_I32, 2);
    xref += prot_->serializedSizeI32(static_cast<int32_t>(type_));
    xref += prot_->serializedSizeStop();
    return xref;
  }

  template <class Protocol_>
  uint32_t serializedSizeZC(Protocol_* prot_) const {
    uint32_t xref = 0;
    xref += prot_->serializedStructSize("TApplicationException");
    xref += prot_->serializedFieldSize(
        "message_", apache::thrift::protocol::T_STRING, 1);
    xref += prot_->serializedSizeString(message_);
    xref +=
        prot_->serializedFieldSize("type_", apache::thrift::protocol::T_I32, 2);
    xref += prot_->serializedSizeI32(static_cast<int32_t>(type_));
    xref += prot_->serializedSizeStop();
    return xref;
  }

 protected:
  std::string message_;

  /**
   * Error code
   */
  TApplicationExceptionType type_;
};

/*
 * @AppClientError
 * An error Thrift application can return from preprocess callback.
 * Indicates the error has been caused by the client
 */
struct FOLLY_EXPORT AppClientException : public TApplicationException {
  AppClientException(std::string&& name, std::string&& message)
      : TApplicationException(std::move(message)), name_(std::move(name)) {}
  const auto& name() const noexcept { return name_; }

 private:
  std::string name_;
};

/*
 * @AppServerError
 * An error Thrift application can return from preprocess callback.
 * Indicates the server is aware it has encountered an error and is incapable
 * of performing the request.
 */
struct FOLLY_EXPORT AppServerException : TApplicationException {
  AppServerException(std::string&& name, std::string&& message)
      : TApplicationException(std::move(message)), name_(std::move(name)) {}
  const auto& name() const noexcept { return name_; }

 private:
  std::string name_;
};

/*
 * @AppOverloadedError
 * An error Thrift application can return from preprocess callback.
 * Indicates the server is overloaded and is rejecting the request before
 * processing starts in order to shed load.
 */
struct FOLLY_EXPORT AppOverloadedException : TApplicationException {
  AppOverloadedException(std::string&& name, std::string&& message)
      : TApplicationException(std::move(message)), name_(std::move(name)) {}
  const auto& name() const noexcept { return name_; }

 private:
  std::string name_;
};

/*
 * @AppQuotaExceededError
 * An error Thrift application can return from preprocess callback.
 * Indicates the client is overbooked for quota and is rejecting the request
 * before processing starts.
 */
struct FOLLY_EXPORT AppQuotaExceededException : TApplicationException {
  AppQuotaExceededException(
      const std::string& tenantId, const std::string& methodName)
      : TApplicationException(fmt::format(
            "Tenant {} has exceeded the quota for method {}.",
            tenantId,
            methodName)) {}
};

} // namespace thrift
} // namespace apache

#endif // #ifndef _THRIFT_TAPPLICATIONEXCEPTION_H_
