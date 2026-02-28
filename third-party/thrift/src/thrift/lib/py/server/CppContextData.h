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

#pragma once

#include <boost/python.hpp>
#include <folly/SocketAddress.h>
#include <thrift/lib/cpp2/server/Cpp2ConnContext.h>

namespace apache::thrift {

class Cpp2ConnContext;

boost::python::object makePythonAddress(const folly::SocketAddress& sa) {
  if (!sa.isInitialized()) {
    return boost::python::object(); // None
  }

  // This constructs a tuple in the same form as socket.getpeername()
  if (sa.getFamily() == AF_INET) {
    return boost::python::make_tuple(sa.getAddressStr(), sa.getPort());
  } else if (sa.getFamily() == AF_INET6) {
    return boost::python::make_tuple(sa.getAddressStr(), sa.getPort(), 0, 0);
  } else if (sa.getFamily() == AF_UNIX) {
    return boost::python::str(sa.getPath());
  } else {
    LOG(FATAL) << "CppServerWrapper can't create a non-inet thrift endpoint";
    abort();
  }
}

class CppContextData {
 public:
  CppContextData() : requestCtx_(nullptr), connCtx_(nullptr) {}

  void copyContextContents(Cpp2RequestContext* reqCtx) {
    requestCtx_ = reqCtx;
    copyContextContents(requestCtx_->getConnectionContext());
  }

  void copyContextContents(Cpp2ConnContext* connCtx) {
    if (!connCtx) {
      return;
    }
    connCtx_ = connCtx;

    clientIdentity_ = connCtx->getPeerCommonName();

    auto pa = connCtx->getPeerAddress();
    if (pa) {
      peerAddress_ = *pa;
    } else {
      peerAddress_.reset();
    }

    auto la = connCtx->getLocalAddress();
    if (la) {
      localAddress_ = *la;
    } else {
      localAddress_.reset();
    }
  }
  boost::python::object getClientIdentity() const {
    if (clientIdentity_.empty()) {
      return boost::python::object();
    } else {
      return boost::python::str(clientIdentity_);
    }
  }
  boost::python::object getPeerAddress() const {
    return makePythonAddress(peerAddress_);
  }
  boost::python::object getLocalAddress() const {
    return makePythonAddress(localAddress_);
  }
  const Cpp2ConnContext* getConnCtx() const { return connCtx_; }
  Cpp2RequestContext* getReqCtx() const { return requestCtx_; }
  const std::string& getHeaderEx() { return headerEx_; }
  void setHeaderEx(const std::string& headerEx) { headerEx_ = headerEx; }
  const std::string& getHeaderExWhat() { return headerExWhat_; }
  void setHeaderExWhat(const std::string& headerExWhat) {
    headerExWhat_ = headerExWhat;
  }

 private:
  apache::thrift::Cpp2RequestContext* requestCtx_;
  const apache::thrift::Cpp2ConnContext* connCtx_;
  std::string clientIdentity_;
  folly::SocketAddress peerAddress_;
  folly::SocketAddress localAddress_;
  std::string headerEx_;
  std::string headerExWhat_;
};
} // namespace apache::thrift
