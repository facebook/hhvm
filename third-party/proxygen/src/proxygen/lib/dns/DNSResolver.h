/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <chrono>
#include <list>
#include <map>
#include <memory>
#include <set>
#include <stdexcept>
#include <vector>

#include <folly/ExceptionWrapper.h>
#include <folly/SocketAddress.h>
#include <folly/io/IOBuf.h>
#include <folly/io/async/DelayedDestruction.h>
#include <folly/io/async/EventBase.h>
#include <proxygen/lib/utils/TraceEvent.h>
#include <proxygen/lib/utils/TraceEventContext.h>

namespace proxygen {
/**
 * Interface implemented by all the DNS resolvers.
 *
 * Implementations of this intreface are free to implement a single
 * resolveHostname() or resolveAddress() call by generating multiple queries.
 * As a result, the ultimate success/failure of the request may depend on the
 * results of these multiple operations.
 *
 * Implementations have to manage their own timeouts by pass error through
 * resolutionError.
 */
class DNSResolver : public folly::DelayedDestruction {
 protected:
  class QueryBase;

 public:
  typedef std::unique_ptr<DNSResolver, folly::DelayedDestruction::Destructor>
      UniquePtr;

  enum class ResolverType : uint32_t {
    UNKNOWN = 0,
    POSIX = 1,
    CARES = 2,
    CARES_DNSCRYPT = 3,
    DOH = 4,
  };

  /*
   * This means that no ttl info is available from the resolver. User should
   * ignore this if the returned ttl is kInvalidDnsTtl.
   */
  static const std::chrono::seconds kInvalidDnsTtl;

  /**
   * Maximum timeout for a single resolution.
   */
  static const std::chrono::milliseconds kMaxTimeout;

  /**
   * The maximum number of CNAME aliases we will resolve.  We keep
   * this number low to avoid implementing any fancy loop detection.
   * RFC 1034 recommends no more than one CNAME.
   */
  static const int kMaxCnameResolutions;

  /**
   * Status codes for resolution failures.
   */
// clang-format off
#define DNSRESOLVER_RESOLUTION_STATUS_GEN(xx)                                 \
  xx(OK, "Success")                                                           \
  xx(TIMEOUT, "Timed out")                                                    \
  xx(INVALID, "An invalid operation was requested")                           \
  xx(TOO_MANY_REDIRECTS, "Followed too many CNAME records")                   \
  xx(NODATA, "Server returned no answer")                                     \
  xx(PARSE_ERROR, "DNS record parse error")                                   \
  xx(CONN_REFUSED, "Client failed to connect to server")                      \
  xx(SERVER_OTHER, "Other errors happened on server side")                    \
  xx(CLIENT_OTHER, "Other errors happened on client side")                    \
  xx(CANCELLED, "Query gets cancelled")                                       \
  xx(SHUTDOWN, "Shutting down")                                               \
  xx(GETADDRINFO, "getaddrinfo failure")                                      \
  xx(THREADPOOL, "threadPool shutting down")                                  \
  xx(UNIMPLEMENTED, "Unimplemented")                                          \
                                                                              \
  /* Must be last */                                                          \
  xx(UNKNOWN, "An unknown error occurred")
// clang-format off

#define DNSRESOLVER_RESOLUTION_STATUS_ENUM(sym, descr)        sym,
  enum ResolutionStatus {
    DNSRESOLVER_RESOLUTION_STATUS_GEN(DNSRESOLVER_RESOLUTION_STATUS_ENUM)
  };
#undef DNSRESOLVER_RESOLUTION_STATUS_ENUM

  /**
   * Exception class for resolution failures.
   */
  class FOLLY_EXPORT Exception : public std::runtime_error {
   public:
    Exception(ResolutionStatus statusVal, const std::string& message)
        : std::runtime_error(message), status_(statusVal) {}
    ResolutionStatus status() const { return status_; }

   private:
    const ResolutionStatus status_;
  };

  /**
   * Answer type for resolutions.
   */
  struct Answer {
    // TTL of the answer
    std::chrono::seconds ttl;

    // Keep the time we created the answer for caching resolvers
    std::chrono::seconds creationTime;

    // Type of answer payload below
    enum AnswerType {
      AT_ADDRESS = 0,
      AT_NAME = 1,
      AT_CNAME = 2,
      AT_TXT = 3,
      AT_SRV = 4,
      AT_MX = 5,
    };

    AnswerType type;

    // The answer payload
    folly::SocketAddress address;
    std::string name;
    std::string canonicalName;
    uint16_t port{0};
    uint16_t priority{0};
    std::shared_ptr<folly::IOBuf> rawData;

    ResolverType resolverType{ResolverType::UNKNOWN};

    Answer(std::chrono::seconds cs, sockaddr* sa) :
        ttl(cs), creationTime(secondsSinceEpoch()),
        type(AT_ADDRESS), address(), name() {
      address.setFromSockaddr(sa);
    }
    Answer(std::chrono::seconds cs, const folly::SocketAddress& ta) :
        ttl(cs), creationTime(secondsSinceEpoch()),
        type(AT_ADDRESS), address(ta), name() {
    }
    Answer(std::chrono::seconds cs, const std::string& n,
           enum AnswerType t = AT_NAME) :
      ttl(cs), creationTime(secondsSinceEpoch()), type(t), address(), name(n) {
    }
    Answer(std::chrono::seconds cs, const std::string& n,
           uint16_t p, enum AnswerType t = AT_NAME) :
      ttl(cs), creationTime(secondsSinceEpoch()), type(t), address(), name(n), port(p) {
    }
    Answer(std::chrono::seconds cs, uint16_t pri, std::string  n) :
      ttl(cs), creationTime(secondsSinceEpoch()), type(AT_MX), address(), name(std::move(n)), priority(pri) {
    }
    Answer(std::chrono::seconds cs,
           std::shared_ptr<folly::IOBuf> rData,
           enum AnswerType t = AT_NAME)
        : ttl(cs),
          creationTime(secondsSinceEpoch()),
          type(t),
          address(),
          rawData(rData) {}

    // Constructors for recreating answers
    Answer(std::chrono::seconds cs, std::chrono::seconds creation,
           const folly::SocketAddress& ta) :
      ttl(cs), creationTime(creation), type(AT_ADDRESS), address(ta), name() {
    }
    Answer(std::chrono::seconds cs, std::chrono::seconds creation,
           const std::string& n,
           enum AnswerType t = AT_NAME) :
      ttl(cs), creationTime(creation), type(t), address(), name(n) {
    }

    //default ctor
    Answer() {}

    // for unit tests
    bool operator==(const Answer& rhs) const {
      return !(operator<(rhs)) && !(rhs < (*this));
    }

    bool operator<(const Answer& rhs) const {
      if (type != rhs.type) {
        return type < rhs.type;
      }

      if (type == AT_ADDRESS) {
        if (address != rhs.address) {
          return address < rhs.address;
        }
      } else if (type == AT_MX) {
        if (priority != rhs.priority) {
          return priority < rhs.priority;
        } else if (name != rhs.name) {
          return name < rhs.name;
        }
      } else {  // AT_NAME or AT_CNAME
        if (name != rhs.name) {
          return name < rhs.name;
        }
      }

      return false;
    }
  };

  /**
   * Callback interface for resolution requests.
   */
  class ResolutionCallback {
   private:
    std::set<QueryBase *> queries_;

   public:
    ResolutionCallback() {}
    virtual ~ResolutionCallback();

    /**
     * Cancel the resolution query.  This is a no-op if the query has
     * already completed.  If your ResolutionCallback is destroyed before
     * the query completes, cancel is called implicitly.
     *
     * if there are pending queries, the function resets its callback
     * so it will not bother its callback function no matter what the
     * underlying resolver return
     *
     * Note the request is not actually cancelled, but is allowed to complete
     * or timeout on its own, but the callback is not invoked.
     */
    void cancelResolution();

    /**
     * The resolution request has succeeded.
     *
     * Even simple queries can result in multiple answers coming back if the
     * server responded with multiple records.
     * subclass should be responsible for canceling queries
     *
     * When this callback is called, the implementor of the callback may delete
     * itself, resulting in the cancellation of all attached sub-queries.
     *
     * @param results         list of results from the resolution; will be
     *                        empty if the resolution succeeded but found no
     *                        records matching the query (e.g. NXDOMAIN)
     */
    virtual void resolutionSuccess(std::vector<Answer> answers)
     noexcept = 0;

    /**
     * The resolution has failed.
     *
     * While this takes an folly::exception_wrapper type, it's likely to be an
     * instance of DNSResolver::Exception. Implementations should re-throw if
     * they're interested in programmatically accessing the reason for the
     * failure in the form of a ResolutionStatus.
     * subclass should be responsible for canceling queries
     *
     * When this callback is called, the implementor of the callback may delete
     * itself, resulting in the cancellation of all attached sub-queries.
     *
     * @param ew              the exception that caused the failure
     */
    virtual void resolutionError(const folly::exception_wrapper& ew)
     noexcept = 0;

   public:
    void insertQuery(QueryBase* query) {
      if (nullptr == query) {
        throw std::runtime_error("DNSResolver::ResolutionCallback::insertQuery must be called with non-null query.");
      }
      std::lock_guard<std::mutex> g(mutex_);
      queries_.insert(query);
    }

    void eraseQuery(QueryBase* query) {
      std::lock_guard<std::mutex> g(mutex_);
      queries_.erase(query);
    }

    void resetQueries() {
      std::lock_guard<std::mutex> g(mutex_);
      queries_.clear();
    }
   private:
    std::mutex mutex_;
  };

  /**
   * Callback interface for collecting stats related to DNS resolution.
   */
  class StatsCollector {
    public:
     virtual ~StatsCollector() {}

     /**
      * These callbacks pertain to the resolution request as a whole and
      * correspond 1:1 to invocations of the ResolutionCallback methods.  Even
      * if a given resolution is implemented by multiple independent queries, a
      * single success/error callback is made.
      */

     /**
      * Record the result of a successful DNS resolution.
      *
      * @param answers           the set of answers that we got back
      * @param latency           time that the resolution took
      */
     virtual void recordSuccess(
      const std::vector<Answer>& answers,
      std::chrono::milliseconds latency) noexcept = 0;

     /**
      * Record the result of a failed DNS resolution.
      *
      * @param exp               exception that triggered the failure
      * @param latency           time that the resolution took
      */
     virtual void recordError(
      const folly::exception_wrapper& ew,
      std::chrono::milliseconds latency) noexcept = 0;

     /**
      * These callbacks pertain to the result of individual queries, multiple
      * of which may be performed in the handling of a single resolution
      * request.
      */

     /**
      * Record the result of a single query.
      *
      * @param rcode             the RFC1035 rcode value that came back
      *                          with the query's answer
      */
     virtual void recordQueryResult(uint8_t rcode) noexcept = 0;

     /**
      * Optional: record cache hit in CachingDNSResolver
      * Basically: Hit + PartialHit + Miss = # of reqs
      *            PartialHit + Miss = # of actually issued real query
      */
     virtual void recordCacheHit() noexcept {}

     /**
      * Optional: record cache full miss in CachingDNSResolver
      */
     virtual void recordCacheMiss() noexcept {}

     /**
      * Optional: record cache partial miss in CachingDNSResolver
      */
     virtual void recordCachePartialMiss() noexcept {}

    /**
      * Optional: record cache hit a stale entry in CachingDNSResolver
      */
     virtual void recordStaleCacheHit() noexcept {}
};


  /**
   * Resolve an address to a hostname.
   *
   * This performs a PTR query.
   *
   * @param cb                callback to invoke when resolution is complete
   * @param address           the address to reverse-resolve
   * @param timeout           timeout after which to give up; a value of 0
   *                          indicates no timeout and values greater than
   *                          kMaxTimeout will be clamped
   */
  virtual void resolveAddress(ResolutionCallback* cb,
   const folly::SocketAddress& address,
   std::chrono::milliseconds timeout = std::chrono::milliseconds(100)) = 0;

  /**
   * Resolve a hostname to an address.
   *
   * This does a bit more than issue a single query.
   *
   *  - Names which are IP address literals are recognized and passed through
   *    to the callback immediately and synchronously.
   *
   *  - If the address family to resolve is AF_UNSPEC, both IPv4 and IPv6
   *    addresses will be queried for and returned.
   *
   * @param cb                callback to invoke when the resolution is complete
   * @param name              the hostname to resolve
   * @param timeout           timeout after which to give up; a value of 0
   *                          indicates no timeout and values greater than
   *                          kMaxTimeout will be clamped
   * @param family            type of address to request; if AF_UNSPEC, both
   *                          AF_INET6 and AF_INET will be resolved. Note that
   *                          this will require multiple DNS queries, as ares
   *                          is not smart enough to pack multiple questions
   *                          into a single request
   * @param teContext         the TraceEventContext
   */
  virtual void resolveHostname(ResolutionCallback* cb,
   const std::string& name,
   std::chrono::milliseconds timeout = std::chrono::milliseconds(100),
   sa_family_t family = AF_INET,
   TraceEventContext teContext = TraceEventContext()) = 0;

  /**
   * Resolve mail exchange servers and priorities for the given domain name.
   *
   * This performs a MX query.
   *
   * @param cb                callback to invoke when resolution is complete
   * @param domain            the domain name to retrieve mail exchange info for
   * @param timeout           timeout after which to give up; a value of 0
   *                          indicates no timeout and values greater than
   *                          kMaxTimeout will be clamped
   */
  virtual void resolveMailExchange(ResolutionCallback* cb,
      const std::string& domain,
      std::chrono::milliseconds timeout = std::chrono::milliseconds(100));

  /**
   * Operate on the StatsCollector instance that we wish to be used to track
   * resolution statistics.
   */
  virtual void setStatsCollector(
   DNSResolver::StatsCollector* statsCollector) = 0;
  virtual DNSResolver::StatsCollector* getStatsCollector() const = 0;

  /*
   * Return pre-configured resolver addresses that match the hostname
   */
  virtual std::list<folly::SocketAddress> getResolverAddresses(
      const std::string& /* hostname */) const {
    return {};
  }

  /**
   * Get the PTR name for the given address.
   *
   * This is used to perform reverse resolution of addresses to hostnames by
   * means of a PTR query. Only supports IP addresses (v4 and v6); anything
   * else will abort the process.
   */
  static std::string getPtrName(
   const folly::SocketAddress& address);

 protected:
  ~DNSResolver() override {}

  class QueryBase {
   public:
     virtual ~QueryBase() {}
     virtual void cancelResolutionImpl() = 0;
  };

  /**
   * The TTL to use when returning results for literals
   */
  static const std::chrono::seconds kLiteralTTL;
};

std::string describe(
 const proxygen::DNSResolver::ResolutionStatus status, bool details);
std::ostream& operator<<(
 std::ostream& os, const proxygen::DNSResolver::ResolutionStatus status);

folly::StringPiece familyToString(sa_family_t);

}
