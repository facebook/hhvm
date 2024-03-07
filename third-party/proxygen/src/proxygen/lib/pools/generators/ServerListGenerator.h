/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <chrono>
#include <map>
#include <set>
#include <string>
#include <vector>

#include <folly/SocketAddress.h>
#include <folly/io/async/AsyncTimeout.h>
#include <glog/logging.h>

#include <proxygen/lib/pools/generators/MemberGroupConfig.h>

namespace proxygen {

/**
 * ServerListGenerator is an abstract class that provides an API for obtaining
 * a list of servers.
 *
 * The same ServerListGenerator may be invoked periodically, and the list of
 * servers returned may change over time.
 */
class ServerListGenerator {
 public:
  struct ServerConfig {
    ServerConfig(const std::string& name, const folly::SocketAddress& address)
        : name(name), address(address) {
    }

    ServerConfig(const std::string& name,
                 const folly::SocketAddress& address,
                 const std::map<std::string, std::string>& properties)
        : name(name), address(address), properties(properties) {
    }

    std::string name;
    folly::SocketAddress address;
    // A field for other addresses that alias the same server.
    // For example a server may have a v4 and a v6 address.
    // Most vector implementations start with a cap of 0 so minimal memory
    // would be used when unused and is why this is still separated from
    // the above preferred address.
    std::vector<folly::SocketAddress> altAddresses;
    std::map<std::string, std::string> properties;
    // Optional parameter. It's only set if a server belongs to a group, which
    // is configured in Pool Config.
    MemberGroupId groupId_{kInvalidPoolMemberGroupId};

    bool operator==(const ServerConfig& other) const {
      return name == other.name && address == other.address &&
             altAddresses == other.altAddresses &&
             properties == other.properties && groupId_ == other.groupId_;
    }

    bool operator<(const ServerConfig& other) const {
      return address < other.address;
    }
  };

  /**
   * Handle that can be used to stop any request in progress
   **/
  class Generator {
   public:
    virtual ~Generator() {
    }

    virtual void cancelServerListRequest() = 0;
  };

  class Callback {
   public:
    Callback() : gen_(nullptr) {
    }

    virtual ~Callback() {
      // An act of gentlemen
      cancelServerListRequest();
    }

    /**
     * onServerListAvailable() will be invoked when the list of servers is
     * available.
     *
     * The results are passed in as an rvalue-reference.  The callback is free
     * to swap out the contents of the results vector and then store it as
     * their own.
     */
    void serverListAvailable(std::vector<ServerConfig> results) noexcept {
      resetGenerator();
      onServerListAvailable(std::move(results));
    }

    virtual void onServerListAvailable(
        std::vector<ServerConfig>&& results) noexcept = 0;

    /**
     * onServerListError will be invoked if there was a problem fetching servers
     * list.
     */
    void serverListError(std::exception_ptr error) noexcept {
      resetGenerator();
      onServerListError(error);
    }

    virtual void onServerListError(std::exception_ptr error) noexcept = 0;

    /**
     * Cancel all the server generator requests running at the moment
     */
    void cancelServerListRequest() {
      if (gen_) {
        gen_->cancelServerListRequest();
        resetGenerator();
      }
    }

    void resetGenerator(Generator* g = nullptr, bool takeOwnership = false) {
      // This function may get called when Generator is not instantiated.
      // This can happen if the tier is down and hence the
      // generator is not even created, but instead serverListAvailable gets
      // called with empty list
      if ((gen_ == nullptr) && (g == nullptr)) {
        return;
      }
      CHECK((gen_ == nullptr) ^ (g == nullptr)) << gen_ << " " << g;

      // Subclasses first call resetGenerator(gen, takeOwnership) after creating
      // a generator. After a success/error/timeout callback is called, this
      // class calls resetGenerator(nullptr)
      // gen_ can be self-owning or we can have ServerListGenerator own gen_.
      // In the self-owning case, the subclassed ServerListGenerator uses the
      // default value for takeOwnership (= false) and the generator is expected
      // to delete itself after calling a callback.
      // The non-self-owning case is needed when the generator may  run on a
      // different thread and hence it would be unwise to delete itself
      // before a callback calls resetGenerator()
      if (g == nullptr && takeOwnershipOfGenerator_) {
        delete gen_;
      }
      gen_ = g;
      takeOwnershipOfGenerator_ = takeOwnership;
    }

    /*
     * Represents the current on-going request
     */
    Generator* gen_;
    bool takeOwnershipOfGenerator_{false};
  };

  explicit ServerListGenerator(folly::EventBase* base = nullptr)
      : eventBase_(base) {
  }

  virtual ~ServerListGenerator() {
    detachEventBase();
  }

  /**
   * Attach and detach from a EventBase.
   *
   * attachEventBase() will be called before the first listServers*() call is
   * made, and detachEventBase() will be called when we're done. The
   * ServerListGenerator implementation MUST be able to handle a call to
   * attachEventBase() after detachEventBase().
   */
  virtual void attachEventBase(folly::EventBase* base);
  virtual void detachEventBase();

  /**
   * Generate the list of servers and invoke the callback when completed.
   */
  virtual void listServers(Callback* callback,
                           std::chrono::milliseconds timeout) = 0;

  /**
   * Generate the list of servers synchronously.
   *
   * This method will block and not return until the list of servers is
   * available.  If an error occurs, it will throw an exception.
   *
   * This is intended as a convenience API for call sites that do not need
   * asynchronous operation.
   *
   * The vector "results" will be replaced by the server list.
   *
   * The ServerListGenerator must not already have been attached to a
   * EventBase.
   */
  virtual void listServersBlocking(std::vector<ServerConfig>* results,
                                   std::chrono::milliseconds timeout);

  void setGroupId(const MemberGroupId serverGroupId) {
    groupId_ = serverGroupId;
  }

  MemberGroupId getGroupId() const {
    return groupId_;
  }

 protected:
  folly::EventBase* eventBase_;
  /**
   * All servers generated by this generator could belong to a server group.
   * This is the ID of such group. Note that it's optional, and only useful
   * if you have multiple ServerListGenerators and are grouping them in
   * different ways.
   */
  MemberGroupId groupId_{kInvalidPoolMemberGroupId};

 private:
  // Forbidden copy constructor and assignment operator
  ServerListGenerator(ServerListGenerator const&) = delete;
  ServerListGenerator& operator=(ServerListGenerator const&) = delete;
};

using ServerConfigList = std::vector<ServerListGenerator::ServerConfig>;

// A default ServerListGenerator::Callback interface for consumers that
// simply want the call status and result returned directly.
class ServerListCallback : public ServerListGenerator::Callback {
 public:
  enum StatusEnum {
    NOT_FINISHED,
    SUCCESS,
    ERROR,
    CANCELLED,
  };

  explicit ServerListCallback() : status(NOT_FINISHED) {
  }

  void onServerListAvailable(std::vector<ServerListGenerator::ServerConfig>&&
                                 results) noexcept override {
    servers.swap(results);
    status = SUCCESS;
  }
  void onServerListError(std::exception_ptr error) noexcept override {
    errorPtr = error;
    status = ERROR;
  }
  virtual void serverListRequestCancelled() {
    status = CANCELLED;
  }

  StatusEnum status;
  std::vector<ServerListGenerator::ServerConfig> servers;
  std::exception_ptr errorPtr;
};

} // namespace proxygen
