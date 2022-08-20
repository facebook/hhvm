/*
 * Copyright (c) Facebook, Inc. and its affiliates.
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

#ifndef THRIFT_TCONNECTIONCONTEXT_H_
#define THRIFT_TCONNECTIONCONTEXT_H_ 1

#include <stddef.h>
#include <memory>

#include <folly/Indestructible.h>
#include <folly/Memory.h>
#include <folly/SocketAddress.h>
#include <folly/io/async/EventBaseManager.h>
#include <thrift/lib/cpp/transport/THeader.h>

namespace apache {
namespace thrift {

namespace server {

class TConnectionContext {
 public:
  TConnectionContext(transport::THeader* header = nullptr) : header_(header) {}

  virtual ~TConnectionContext() = default;

  virtual const folly::SocketAddress* getPeerAddress() const { return nullptr; }

  // Expose the THeader to read headers or other flags
  transport::THeader* getHeader() const { return header_; }

  bool setHeader(const std::string& key, const std::string& value) {
    if (header_) {
      header_->setHeader(key, value);
      return true;
    } else {
      return false;
    }
  }

  const transport::THeader::StringToStringMap& getHeaders() const {
    if (header_) {
      return header_->getHeaders();
    } else {
      static const folly::Indestructible<transport::THeader::StringToStringMap>
          kEmpty;
      return *kEmpty;
    }
  }

  const transport::THeader::StringToStringMap* getHeadersPtr() const {
    return header_ ? &header_->getHeaders() : nullptr;
  }

  virtual folly::EventBaseManager* getEventBaseManager() { return nullptr; }

  /**
   * Get the user data field.
   */
  virtual void* getUserData() const { return nullptr; }

  /**
   * Set the user data field.
   *
   * @param data         The new value for the user data field.
   * @param destructor   A function pointer to invoke when the connection
   *                     context is destroyed.  It will be invoked with the
   *                     contents of the user data field.
   *
   * @return Returns the old user data value.
   */
  void* setUserData(void* data, void (*destructor)(void*) = nullptr) {
    return setUserData({data, destructor ? destructor : no_op_destructor});
  }
  virtual void* setUserData(folly::erased_unique_ptr) {
    throw std::runtime_error(
        "setUserData is not implemented by this connection context type");
  }

 protected:
  apache::thrift::transport::THeader* header_;

  static void no_op_destructor(void* /*ptr*/) {}
};

} // namespace server
} // namespace thrift
} // namespace apache

#endif // THRIFT_TCONNECTIONCONTEXT_H_
