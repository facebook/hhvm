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

#include <folly/SocketAddress.h>
#include <folly/futures/Future.h>
#include <folly/io/SocketOptionMap.h>
#include <folly/io/async/AsyncSSLSocket.h>
#include <folly/io/async/AsyncSocket.h>
#include <folly/ssl/SSLSession.h>
#include <wangle/channel/Pipeline.h>
#include <memory>

namespace wangle {

class SSLSessionEstablishedCallback {
 public:
  virtual ~SSLSessionEstablishedCallback() = default;
  // notified when a non-reused SSLSession is established.
  virtual void onEstablished(
      std::shared_ptr<folly::ssl::SSLSession> session) = 0;
};

class ClientBootstrapSocketOptions {
 public:
  ClientBootstrapSocketOptions& setV4OptionMap(
      const folly::SocketOptionMap& options) {
    v4_ = options;
    return *this;
  }
  folly::SocketOptionMap const& getV4OptionMap() const {
    return v4_;
  }
  ClientBootstrapSocketOptions& setV6OptionMap(
      const folly::SocketOptionMap& options) {
    v6_ = options;
    return *this;
  }
  folly::SocketOptionMap const& getV6OptionMap() const {
    return v6_;
  }

 private:
  folly::SocketOptionMap v4_;
  folly::SocketOptionMap v6_;
};

using SSLSessionEstablishedCallbackUniquePtr =
    std::unique_ptr<SSLSessionEstablishedCallback>;

/*
 * A wrapper template around Pipeline and AsyncSocket or SPDY/HTTP/2 session to
 * match ServerBootstrap so BroadcastPool can work with either option
 */
template <typename P = DefaultPipeline>
class BaseClientBootstrap {
 public:
  using Ptr = std::unique_ptr<BaseClientBootstrap>;
  BaseClientBootstrap() = default;

  virtual ~BaseClientBootstrap() = default;

  BaseClientBootstrap<P>* pipelineFactory(
      std::shared_ptr<PipelineFactory<P>> factory) noexcept {
    pipelineFactory_ = factory;
    return this;
  }

  P* getPipeline() {
    return pipeline_.get();
  }

  virtual folly::Future<P*> connect(
      const folly::SocketAddress& address,
      std::chrono::milliseconds timeout = std::chrono::milliseconds(0)) = 0;

  BaseClientBootstrap* sslContext(
      std::shared_ptr<const folly::SSLContext> sslContext) {
    sslContext_ = sslContext;
    return this;
  }

  BaseClientBootstrap* sslSession(
      std::shared_ptr<folly::ssl::SSLSession> sslSession) {
    sslSession_ = sslSession;
    return this;
  }

  BaseClientBootstrap* serverName(const std::string& sni) {
    sni_ = sni;
    return this;
  }

  BaseClientBootstrap* sslSessionEstablishedCallback(
      SSLSessionEstablishedCallbackUniquePtr sslSessionEstablishedCallback) {
    sslSessionEstablishedCallback_ = std::move(sslSessionEstablishedCallback);
    return this;
  }

  BaseClientBootstrap* deferSecurityNegotiation(bool deferSecurityNegotiation) {
    deferSecurityNegotiation_ = deferSecurityNegotiation;
    return this;
  }

  void setPipeline(const typename P::Ptr& pipeline) {
    pipeline_ = pipeline;
  }

  virtual void makePipeline(std::shared_ptr<folly::AsyncTransport> socket) {
    pipeline_ = pipelineFactory_->newPipeline(socket);
  }

  void setSocketOptions(const ClientBootstrapSocketOptions& sockOpts) {
    socketOptions_ = sockOpts;
  }

 protected:
  const folly::SocketOptionMap& getSocketOptions(sa_family_t ipFamily) {
    if (ipFamily == AF_INET) {
      return socketOptions_.getV4OptionMap();
    } else if (ipFamily == AF_INET6) {
      return socketOptions_.getV6OptionMap();
    }
    return folly::emptySocketOptionMap;
  }

  std::shared_ptr<PipelineFactory<P>> pipelineFactory_;
  typename P::Ptr pipeline_;
  std::shared_ptr<const folly::SSLContext> sslContext_;
  std::shared_ptr<folly::ssl::SSLSession> sslSession_{nullptr};
  std::string sni_;
  bool deferSecurityNegotiation_{false};
  SSLSessionEstablishedCallbackUniquePtr sslSessionEstablishedCallback_;
  ClientBootstrapSocketOptions socketOptions_;
};

template <typename ClientBootstrap = BaseClientBootstrap<>>
class BaseClientBootstrapFactory {
 public:
  virtual typename ClientBootstrap::Ptr newClient() = 0;
  virtual ~BaseClientBootstrapFactory() = default;
};

} // namespace wangle
