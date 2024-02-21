/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <folly/fibers/FiberManager.h>
#include <folly/io/async/EventBase.h>
#include <folly/json/dynamic.h>

#include "mcrouter/lib/network/AsyncMcClient.h"

namespace carbon {

/**
 * Virtual class responsible for sending json requests to carbon servers.
 *
 * NOTE: This is not intented to be used in production. Is does slow json
 *       parsing/serialization. It's main purpose is to be used as a simple
 *       client tool, for testing/debugging purposes.
 */
class JsonClient {
 public:
  struct Options {
    /**
     * Hostname of the carbon server.
     */
    std::string host{"localhost"};

    /**
     * Port of the carbon server.
     */
    uint16_t port;

    /**
     * Server timout in milliseconds.
     */
    size_t serverTimeoutMs{200};

    /**
     * Whether of not parsing errors should be ignored (they will be displayed).
     */
    bool ignoreParsingErrors{true};

    /**
     * Whether or not to use SSL.
     */
    bool useSsl;

    /**
     * The SSL cert/key/CA paths.
     */
    std::string pemCertPath;
    std::string pemKeyPath;
    std::string pemCaPath;

    /**
     * The SSL service identity.
     */
    std::string sslServiceIdentity;
  };

  /**
   * Creates JsonClient object.
   * May throw std::bad_alloc if some allocation fails.
   *
   * @paramm options  Client settings.
   * @params onError  Callback for when an error happens.
   */
  explicit JsonClient(
      Options options,
      std::function<void(const std::string& msg)> onError = nullptr);
  virtual ~JsonClient() = default;

  /**
   * Send requests and return replies synchronously.
   *
   * @param requestName   The name of the request.
   * @param requests      Folly::dynamic containing either a single request, or
   *                      a list of requests.
   * @param replies       Output argument. Folly::dynamic containing the replies
   *                      to the requests present in "requests". If "requests"
   *                      contains multiple requests, the replies will contain a
   *                      json with all the replies, where the reply index will
   *                      match the corresponsing request index.
   *
   * @return              True if no errors are found.
   */
  bool sendRequests(
      const std::string& requestName,
      const folly::dynamic& requests,
      folly::dynamic& replies);

 protected:
  /**
   * Sends a single request synchronously
   *
   * @param requestName   Name of the request.
   * @param requestJson   Folly::dynamic containing a sinlge request to be sent.
   * @param replyJson     Output argument. Folly::dynmaic containing the reply
   *                      for the given request.
   *
   * @return              True if no errors are found.
   */
  virtual bool sendRequestByName(
      const std::string& requestName,
      const folly::dynamic& requestJson,
      folly::dynamic& replyJson) = 0;

  /**
   * Sends a request to the specified carbon server.
   *
   * @param requestJson   Json containing the request to be sent.
   * @param replyJson     Output parameter with the json containing the reply.
   *
   * @return              True if the request was sent and the reply received
   *                      successfully.
   */
  template <class Request>
  bool sendRequest(
      const folly::dynamic& requestJson,
      folly::dynamic& replyJson);

  const Options& options() const {
    return options_;
  }

 private:
  const Options options_;
  const std::function<void(const std::string& msg)> onError_;

  folly::EventBase evb_;
  facebook::memcache::AsyncMcClient client_;
  folly::fibers::FiberManager& fiberManager_;

  void onError(const std::string& msg) const;
};

} // namespace carbon

#include "JsonClient-inl.h"
