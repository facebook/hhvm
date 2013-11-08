// Copyright 2004-present Facebook.  All rights reserved.

#pragma once

#include "thrift/lib/cpp/async/TAsyncTransport.h"
#include "thrift/lib/cpp/transport/TSocketAddress.h"

#include <chrono>
#include <list>
#include <string>

namespace apache { namespace thrift { namespace async {
class TEventBase;
}}}

namespace facebook { namespace proxygen {

class AsyncTransportFactory {
 public:
  /**
   * Abstract interface implemented by all the Connectors. Forces them to
   * implement `cancel` functionality. Connector is expected to delete itself
   * after `cancel` is invoked.
   */
  class Connector {
   public:
    virtual ~Connector() {}

    virtual void start(std::chrono::milliseconds timeout) noexcept = 0;
    virtual void cancel() noexcept = 0;
  };

  /**
   * Callback class that the user of the factory implements to get
   * notified about the results of their request
   */
  class Callback {
   public:
    virtual ~Callback() {}

    virtual void getTransportSuccess(
        apache::thrift::async::TAsyncTransport::UniquePtr transport)
            noexcept = 0;

    virtual void getTransportError(const std::exception_ptr& ex) noexcept = 0;

    virtual void logTransportFailedAddr(
      const apache::thrift::transport::TSocketAddress& addr) noexcept = 0;
  };

  /**
   * Callback that can be used to update stats
   */
  class StatsCallback {
   public:
    virtual ~StatsCallback() {}
    virtual void onConnectionAttemptStarted() {}
    virtual void onConnectionAttemptFinished() {}
    virtual void onSocketConnectError() {}
  };

  // Minimal interface to be implemented by implementation of transport factory.
  //
  // This has been purposefully kept very small so that its easy to mock it out
  // in our test cases.
  //
  // We return the connector which you can use to `cancel` the request.
  virtual Connector* getTransport(
      Callback* cb,
      const std::string& hostname,
      const std::list<apache::thrift::transport::TSocketAddress>& connectAddrs)
          noexcept = 0;

  virtual ~AsyncTransportFactory() {
  }
};

}}
