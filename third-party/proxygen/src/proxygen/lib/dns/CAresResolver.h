/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <folly/portability/Windows.h> // make sure that NOMINMAX is defined for windows

#include <chrono>
#include <list>
#include <map>
#include <memory>

#include <ares.h>
#include <folly/Expected.h>
#include <folly/SocketAddress.h>
#include <folly/io/async/AsyncTimeout.h>
#include <folly/io/async/EventBase.h>
#include <sodium.h>

#include "proxygen/lib/dns/DNSResolver.h"

namespace {
constexpr size_t kCryptoBoxHalfNonceBytes(crypto_box_NONCEBYTES / 2U);
}

namespace proxygen {
/**
 * Class to perform asynchronous DNS resolutions.
 *
 * Implementation notes:
 *
 *  . No caching is performed; each call to resolve() will trigger a
 *    request to the configured nameserver
 *
 *  . Hostname mappings specified in /etc/hosts are not honored. This is an
 *    implementation artifact of the underlying DNS library, which performs
 *    blocking file I/O at resolution time to honor this file.
 *
 *  . DelayedDestruction is required because the underlying DNS library is not
 *    reentrant in all cases. Specifially, a call to ares_destroy() cannot run
 *    while ares_process_fd() is executing higher up the stack.
 *
 *  . Search domains in /etc/resolv.conf are not honored. This is because
 *    we have to use the ares_query() API to get TTL information, and it does
 *    not use the search domains. We can work around this if we need to by
 *    parsing /etc/resolv.conf ourselves, or by adding an API method to Ares to
 *    expose this.
 */
namespace detail {
struct ParseError {
  DNSResolver::ResolutionStatus status;
  std::string msg;
};

folly::Expected<std::vector<DNSResolver::Answer>, ParseError> parseTxtRecords(
    unsigned char* aresBuffer, int alen) noexcept;

folly::Expected<std::vector<DNSResolver::Answer>, ParseError> parseSrvRecords(
    unsigned char* aresBuffer, int alen) noexcept;

folly::Expected<std::vector<DNSResolver::Answer>, ParseError> parseMxRecords(
    unsigned char* aresBuffer, int alen) noexcept;
} // namespace detail

class CAresResolver : public DNSResolver {
 public:
  using UniquePtr = std::unique_ptr<CAresResolver, CAresResolver::Destructor>;

  // ns_type isn't defined on all target platforms so we define this ourselves.
  enum class RecordType : int {
    kA = 1,
    kAAAA = 28,
    kPtr = 12,
    kTXT = 16,
    kSRV = 33,
    kMX = 15,
  };

  // Helper class for handling a single query / answer + timeout.
  //
  // This mainly exists to handle dispatching of parsing for the different query
  // types and corresponding conversion to Answer objects.
  //
  // The lifecycle for this object expires once the queryCallback() method has
  // been called. Even if a timeout has fired earlier. This is because we can't
  // cancel individual queries in Ares -- we just have to let it complete and
  // ignore the result. To facilitate this, we clear the callback_ field once a
  // timeout has fired and use this as an indication that we should ignore any
  // subsequent result.
  class Query
      : public DNSResolver::QueryBase
      , private folly::AsyncTimeout {
   public:
    Query(CAresResolver* resolver,
          RecordType type,
          const std::string& name,
          bool recordStats,
          TraceEvent dnsEvent,
          const TimeUtil* timeUtil = nullptr,
          TraceEventContext teContext = TraceEventContext());

    ~Query() override {
    }

    void resolve(ResolutionCallback* cb, std::chrono::milliseconds timeout);
    void cancelResolutionImpl() override;
    std::array<unsigned char, kCryptoBoxHalfNonceBytes>& getNonce();
    void setDnsCryptUsed(bool used, uint32_t serial);
    std::chrono::steady_clock::time_point& getLastNonceTimeRef();

   protected:
    // Don't use fail() here, as that would delete the object and we need to
    // leave it around for the query callback to complete
    void timeoutExpired() noexcept override;
    void fail(ResolutionStatus status, const std::string& msg);

    ResolutionCallback* callback_ = {nullptr};

   private:
    static const size_t kMaxRecords = 64;

    CAresResolver* resolver_;
    RecordType type_;
    std::string name_;
    int cnameResolutions_ = {0};
    TimePoint startTime_;
    bool recordStats_;
    bool dnscryptUsed_{false};
    const TimeUtil* timeUtil_;

    // TraceEvent for resolution
    TraceEvent dnsEvent_;
    TraceEventContext teContext_;

    std::array<unsigned char, kCryptoBoxHalfNonceBytes> nonce_;

    void succeed(std::vector<Answer> answers);

    /**
     * If the DNS response contains no A/AAAA records, check for any cname
     * aliases.  If one exists, perform a recursive DNS query.  Otherwise,
     * invoke an appropriate error or success callback.
     */
    static void checkForCName(Query* self, hostent* host);

    static void queryCallback(
        void* data, int status, int timeouts, unsigned char* abuf, int alen);
  };

  template <typename... Args>
  static UniquePtr newResolver(Args&&... args) {
    return UniquePtr(new CAresResolver(std::forward<Args>(args)...));
  }

  CAresResolver();

  /**
   * Use the given EventBase for scheduling I/O.
   *
   * Must be called before init().
   */
  void attachEventBase(folly::EventBase* base);
  const folly::EventBase* getEventBase();

  /**
   * Set the DNS servers to use for resolution; overrides whatever is
   * configured via /etc/resolv.conf.
   *
   * Note that this *only* sets the IP addresses of servers to connect to. It
   * does *not* change the port to use. For that, use setPort(). Note that
   * this implies that all servers must use the same port.
   *
   * Must be called before init().
   */
  void setServers(const std::list<folly::SocketAddress>& servers);

  /**
   * Set the port to use for DNS queries. The default is 53.
   *
   * Must be called before init().
   */
  void setPort(uint16_t port);

  void setResolveSRVOnly(bool resolve) {
    resolveSRVRecord_ = resolve;
  }
  bool resolveSRVOnly() const {
    return resolveSRVRecord_;
  }

  /**
   * Initialize the resolver.
   */
  virtual void init();

  // DNSResolver API
  void resolveAddress(ResolutionCallback* cb,
                      const folly::SocketAddress& address,
                      std::chrono::milliseconds timeout =
                          std::chrono::milliseconds(100)) override;
  void resolveHostname(
      ResolutionCallback* cb,
      const std::string& name,
      std::chrono::milliseconds timeout = std::chrono::milliseconds(100),
      sa_family_t family = AF_INET,
      TraceEventContext teContext = TraceEventContext()) override;
  void resolveMailExchange(ResolutionCallback* cb,
                           const std::string& domain,
                           std::chrono::milliseconds timeout =
                               std::chrono::milliseconds(100)) override;
  void setStatsCollector(DNSResolver::StatsCollector* statsCollector) override;
  DNSResolver::StatsCollector* getStatsCollector() const override;
  std::chrono::steady_clock::time_point& getLastNonceTimeRef();

  const TimeUtil* getTimeUtilPtr() const {
    return &timeUtil_;
  }

 protected:
 private:
  class SocketHandler;
  class MultiQuery;

  void setSerializedServers();

  folly::EventBase* base_;
  ares_channel channel_;
  uint16_t channelRefcnt_;
  std::map<int, std::unique_ptr<SocketHandler>> socketHandlers_;
  std::list<folly::SocketAddress> servers_;
  std::string serializedResolvers_;
  uint16_t port_;
  StatsCollector* statsCollector_;
  TimeUtil timeUtil_;
  std::chrono::steady_clock::time_point lastNonceTimeStamp_;
  bool resolveSRVRecord_{false};

  // Attempt to resolve literal IPs, invoking the callback and returning
  // true if we succeeded.
  bool resolveLiterals(ResolutionCallback* cb,
                       const std::string& host,
                       sa_family_t family);

  // Attempt to resolve localhost, If Ipv6 is available, we will use
  // that otherwise we will use ipv4. If AF_UNSPEC is requested, we
  // will return both
  bool resolveLocalhost(ResolutionCallback* cb,
                        const std::string& host,
                        sa_family_t family);

  // Low-level API for issuing a query using Ares
  virtual void query(const std::string& name,
                     RecordType type,
                     ares_callback cb,
                     void* data_data);

  // Callback for users of the query() API to indicate that they're done. This
  // can be invoked whenever they no longer care about the response: either
  // because it's been received and processed, or because they want to ignore
  // it (though the callback will still be invoked in this case).
  virtual void queryFinished();

  // Callback for Ares socket state changes
  static void dnsSocketReady(void* data,
                             ares_socket_t sock,
                             int read,
                             int write);

  // Global constructor/destructor

#if defined(_MSC_VER) && !defined(__clang__)
#define ATTRIBUTE_CONSTRUCTOR
#else
#define ATTRIBUTE_CONSTRUCTOR __attribute__((__constructor__))
#endif
#if defined(_MSC_VER) && !defined(__clang__)
#define ATTRIBUTE_DESTRUCTOR
#else
#define ATTRIBUTE_DESTRUCTOR __attribute__((__destructor__))
#endif
  static void initGlobal() ATTRIBUTE_CONSTRUCTOR;
  static void destroyGlobal() ATTRIBUTE_DESTRUCTOR;
#undef ATTRIBUTE_CONSTRUCTOR
#undef ATTRIBUTE_DESTRUCTOR

 protected:
  // Use DelayedDestruction::destroy() instead
  ~CAresResolver() override;
};
} // namespace proxygen
