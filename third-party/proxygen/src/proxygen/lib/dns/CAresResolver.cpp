/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "proxygen/lib/dns/CAresResolver.h"

#include <folly/Conv.h>
#include <folly/io/async/EventHandler.h>
#include <folly/portability/Sockets.h>
#include <glog/logging.h>
#include <proxygen/lib/utils/Time.h>

using namespace proxygen;

using folly::AsyncTimeout;
using folly::DelayedDestruction;
using folly::EventBase;
using folly::EventHandler;
using folly::SocketAddress;
using std::list;
using std::make_exception_ptr;
using std::unique_ptr;
using std::vector;

/* Make sure C-Ares error codes that map to DNS RCODE values are unchanged */
#if !(ARES_SUCCESS == 0 && ARES_EREFUSED == 6)
#error "C-Ares status does not map to DNS RCODE values"
#endif

/* Convert a C-Ares status value to an DNS RCODE; -1 for unknown */
#define ARES_TO_RCODE(x) (((x) <= ARES_EREFUSED) ? (x) : -1)

namespace {
struct HostentDeleter {
  void operator()(hostent* ptr) {
    ares_free_hostent(ptr);
  }
};

template <class T>
struct AresDataDeleter {
  void operator()(T* ptr) {
    ares_free_data(ptr);
  }
};

template <class T>
using AresDataUniquePtr = std::unique_ptr<T, AresDataDeleter<T>>;

class NullStatsCollector : public DNSResolver::StatsCollector {
 public:
  NullStatsCollector() {
  }
  ~NullStatsCollector() override {
  }
  void recordSuccess(const std::vector<DNSResolver::Answer>& /*answers*/,
                     std::chrono::milliseconds /*latency*/) noexcept override {
  }
  void recordError(const folly::exception_wrapper& /*ew*/,
                   std::chrono::milliseconds /*latency*/) noexcept override {
  }
  void recordQueryResult(uint8_t /*rcode*/) noexcept override {
  }
};

NullStatsCollector nullStatsCollector;
} // namespace

CAresResolver::Query::Query(CAresResolver* resolver,
                            RecordType type,
                            const std::string& name,
                            bool recordStats,
                            TraceEvent dnsEvent,
                            const TimeUtil* timeUtil,
                            TraceEventContext teContext)
    : AsyncTimeout(resolver->base_),
      resolver_(resolver),
      type_(type),
      name_(name),
      recordStats_(recordStats),
      timeUtil_(timeUtil),
      dnsEvent_(std::move(dnsEvent)),
      teContext_(std::move(teContext)) {
  dnsEvent_.addMeta(TraceFieldType::HostName, name);
}

void CAresResolver::Query::resolve(ResolutionCallback* cb,
                                   std::chrono::milliseconds timeout) {
  CHECK(callback_ == nullptr);
  CHECK(cb != nullptr);

  dnsEvent_.start(*timeUtil_);

  callback_ = cb;
  callback_->insertQuery(this);
  startTime_ = getCurrentTime();
  if (timeout.count() > 0 && !scheduleTimeout(timeout.count())) {
    LOG(DFATAL) << "Failed to schedule timeout for query " << name_
                << " with type " << static_cast<int>(type_);
  }
  resolver_->query(name_, type_, Query::queryCallback, this);
}

void CAresResolver::Query::cancelResolutionImpl() {
  callback_ = nullptr;
  cancelTimeout();
  resolver_->queryFinished();
}

void CAresResolver::Query::fail(ResolutionStatus status,
                                const std::string& msg) {
  if (callback_) {
    CAresResolver* resolver = resolver_;
    ResolutionCallback* cb = callback_;
    std::chrono::milliseconds resolutionTime = millisecondsSince(startTime_);
    auto ew =
        folly::make_exception_wrapper<Exception>(status, msg + " for " + name_);

    dnsEvent_.end(*timeUtil_);
    dnsEvent_.addMeta(TraceFieldType::Error, msg);
    dnsEvent_.addMeta(TraceFieldType::CNameRedirects, cnameResolutions_);
    teContext_.traceEventAvailable(std::move(dnsEvent_));

    resolver->queryFinished();
    if (recordStats_) {
      resolver->getStatsCollector()->recordError(ew, resolutionTime);
    }

    cb->eraseQuery(this);
    cb->resolutionError(ew);
  }

  delete this;
}

void CAresResolver::Query::succeed(std::vector<Answer> answers) {
  if (callback_) {
    CAresResolver* resolver = resolver_;
    ResolutionCallback* cb = callback_;
    std::chrono::milliseconds resolutionTime = millisecondsSince(startTime_);

    dnsEvent_.end(*timeUtil_);

    std::map<TraceFieldType, std::string> traces;
    if (!answers.empty()) {
      for (const auto& answer : answers) {
        std::string dnsResult;
        switch (answer.type) {
          case Answer::AnswerType::AT_TXT:
            break;
          case Answer::AnswerType::AT_ADDRESS:
            dnsResult = answer.address.getAddressStr() + " ";
            break;
          default:
            dnsResult = answer.name + " ";
        }

        TraceFieldType lookupType;
        switch (answer.type) {
          case Answer::AnswerType::AT_TXT:
            lookupType = TraceFieldType::TXT;
            break;
          case Answer::AnswerType::AT_ADDRESS:
            lookupType = TraceFieldType::IpAddr;
            break;
          default:
            lookupType = TraceFieldType::HostName;
        }
        traces[lookupType] += dnsResult;
      }
      for (auto& trace : traces) {
        dnsEvent_.addMeta(trace.first, trace.second);
      }
    } else {
      dnsEvent_.addMeta(TraceFieldType::IpAddr, "No Results");
    }

    dnsEvent_.addMeta(TraceFieldType::NumberAnswers, answers.size());
    dnsEvent_.addMeta(TraceFieldType::CNameRedirects, cnameResolutions_);
    teContext_.traceEventAvailable(std::move(dnsEvent_));

    if (recordStats_) {
      resolver->getStatsCollector()->recordSuccess(answers, resolutionTime);
    }

    resolver->queryFinished();
    cb->eraseQuery(this);
    cb->resolutionSuccess(std::move(answers));
  }
  delete this;
}

/**
 * If the DNS response contains no A/AAAA records, check for any cname
 * aliases.  If one exists, perform a recursive DNS query.  Otherwise,
 * invoke an appropriate error or success callback.
 */
void CAresResolver::Query::checkForCName(Query* self, hostent* host) {
  // C-ARES puts the CNAME in the host->h_name field
  if (host && host->h_name && self->name_ != host->h_name) {
    if (self->cnameResolutions_++ >= DNSResolver::kMaxCnameResolutions) {
      self->fail(TOO_MANY_REDIRECTS,
                 "Query failed due to too many CNAME redirects");
      return;
    }

    if (self->callback_ == nullptr) {
      self->fail(TIMEOUT, "Query failed due to timeout");
      return;
    }

    // Issue a recursive query for the cname host
    self->resolver_->query(
        host->h_name, self->type_, Query::queryCallback, self);
    self->resolver_->queryFinished();
    return;
  }

  // No results are treated as "success"
  self->succeed({});
}

void CAresResolver::Query::queryCallback(
    void* data, int status, int /*timeouts*/, unsigned char* abuf, int alen) {
  Query* self = static_cast<Query*>(data);

  // Record the RCODE value for this query. Note that we ignore the
  // recordStats_ stat, as that's only for aggregated end-of-resolution
  // stats, not per-query stats
  int8_t rcode = ARES_TO_RCODE(status);
  if (rcode >= 0) {
    self->resolver_->getStatsCollector()->recordQueryResult(rcode);
  }

  // ARES_ENOTFOUND and ARES_ENODATA are special cases that indicate
  // no results. Just call it success but with an empty results list.
  if (status == ARES_ENOTFOUND || status == ARES_ENODATA) {
    self->succeed({});
    return;
  }

  // All other Ares errors are fatal.
  if (status != ARES_SUCCESS) {
    switch (status) {
      case ARES_ECONNREFUSED:
        self->fail(CONN_REFUSED, ares_strerror(status));
        break;
      case ARES_EFORMERR:
      case ARES_ESERVFAIL:
      case ARES_ENOTIMP:
      case ARES_EREFUSED:
        self->fail(SERVER_OTHER, ares_strerror(status));
        break;
      case ARES_EBADQUERY:
      case ARES_EBADNAME:
      case ARES_EBADFAMILY:
      case ARES_EBADRESP:
      case ARES_EOF:
      case ARES_EFILE:
      case ARES_ENOMEM:
      case ARES_EDESTRUCTION:
      case ARES_EBADSTR:
        self->fail(CLIENT_OTHER, ares_strerror(status));
        break;
      case ARES_ECANCELLED:
        self->fail(CANCELLED, ares_strerror(status));
        break;
      case ARES_ETIMEOUT:
        self->fail(TIMEOUT,
                   folly::to<std::string>("Underlying c-ares timeout: ",
                                          ares_strerror(status)));
        break;
      default:
        self->fail(UNKNOWN, ares_strerror(status));
    }
    return;
  }

  std::vector<Answer> answers;

  switch (self->type_) {
    case RecordType::kA: {
      ares_addrttl ttls[kMaxRecords];
      int nttls = kMaxRecords;

      hostent* hst;
      status = ares_parse_a_reply(abuf, alen, &hst, ttls, &nttls);
      unique_ptr<hostent, HostentDeleter> host(hst);

      if (status == ARES_SUCCESS) {
        sockaddr_in saddr{};
        saddr.sin_family = AF_INET;

        auto cname = host && host->h_name ? std::string(host->h_name) : "";

        for (int i = 0; i < nttls; i++) {
          memcpy(&saddr.sin_addr, &ttls[i].ipaddr, sizeof(in_addr));
          Answer ans(std::chrono::seconds(ttls[i].ttl),
                     reinterpret_cast<sockaddr*>(&saddr));
          ans.name = self->name_;
          ans.canonicalName = cname;
          answers.push_back(std::move(ans));
        }

        // If we got no responses, check for a CNAME
        if (nttls == 0) {
          checkForCName(self, host.get());
          return;
        }
      } else {
        self->fail(PARSE_ERROR,
                   folly::to<std::string>("Failed to parse A answer ", status));
        return;
      }

      break;
    }

    case RecordType::kAAAA: {
      ares_addr6ttl ttls[kMaxRecords];
      int nttls = kMaxRecords;

      hostent* hst;
      status = ares_parse_aaaa_reply(abuf, alen, &hst, ttls, &nttls);
      unique_ptr<hostent, HostentDeleter> host(hst);

      if (status == ARES_SUCCESS) {
        sockaddr_in6 saddr{};
        saddr.sin6_family = AF_INET6;

        auto cname = host && host->h_name ? std::string(host->h_name) : "";

        for (int i = 0; i < nttls; i++) {
          memcpy(&saddr.sin6_addr, &ttls[i].ip6addr, sizeof(ares_in6_addr));
          Answer ans(std::chrono::seconds(ttls[i].ttl),
                     reinterpret_cast<sockaddr*>(&saddr));
          ans.name = self->name_;
          ans.canonicalName = cname;
          answers.push_back(std::move(ans));
        }

        // If we got no responses, check for a CNAME
        if (nttls == 0) {
          checkForCName(self, host.get());
          return;
        }
      } else {
        self->fail(
            PARSE_ERROR,
            folly::to<std::string>("Failed to parse AAAA answer ", status));
        return;
      }

      break;
    }

    case RecordType::kPtr: {
      hostent* hst = nullptr;

      // family actually doesn't matter since addr and addrlen are empty, family
      // is only used when the original input addr is used. At some point we
      // should probably retain the original input address
      status = ares_parse_ptr_reply(abuf, alen, nullptr, 0, AF_INET6, &hst);
      unique_ptr<hostent, HostentDeleter> host(hst);

      if (status == ARES_SUCCESS) {
        for (char** aliasp = host->h_aliases; *aliasp != nullptr; aliasp++) {
          answers.push_back(Answer(std::chrono::seconds(60), *aliasp));
        }
      } else {
        self->fail(
            PARSE_ERROR,
            folly::to<std::string>("Failed to parse PTR answer ", status));
        return;
      }

      break;
    }

    // Used by DNSCrypt to get certificates
    case RecordType::kTXT: {
      auto result = detail::parseTxtRecords(abuf, alen);
      if (result.hasError()) {
        self->fail(result.error().status, result.error().msg);
        return;
      } else {
        answers = std::move(result).value();
      }
      break;
    }

    case RecordType::kSRV: {
      auto result = detail::parseSrvRecords(abuf, alen);
      if (result.hasError()) {
        self->fail(result.error().status, result.error().msg);
        return;
      } else {
        answers = std::move(result).value();
      }
      break;
    }

    case RecordType::kMX: {
      auto result = detail::parseMxRecords(abuf, alen);
      if (result.hasError()) {
        self->fail(result.error().status, result.error().msg);
        return;
      } else {
        answers = std::move(result).value();
      }
      break;
    }

    default:
      LOG(ERROR) << "Couldn't handle answer for query type "
                 << static_cast<int>(self->type_) << ", during resolving "
                 << self->name_;
      self->fail(
          PARSE_ERROR,
          folly::to<std::string>("Failed to parse answer for query type: ",
                                 static_cast<int>(self->type_)));
      return;
  }

  for (auto& answer : answers) {
    answer.resolverType = self->dnscryptUsed_ ? ResolverType::CARES_DNSCRYPT
                                              : ResolverType::CARES;
  }

  self->succeed(std::move(answers));
}

// Don't use fail() here, as that would delete the object and we need to
// leave it around for the query callback to complete
void CAresResolver::Query::timeoutExpired() noexcept {
  ResolutionCallback* cb = callback_;

  callback_ = nullptr;
  resolver_->queryFinished();
  if (!cb) {
    return;
  }

  CAresResolver* resolver = resolver_;
  auto ew =
      folly::make_exception_wrapper<Exception>(TIMEOUT, "Query timed out");
  std::chrono::milliseconds resolutionTime = millisecondsSince(startTime_);
  if (recordStats_) {
    resolver->getStatsCollector()->recordError(ew, resolutionTime);
  }

  cb->eraseQuery(this);
  cb->resolutionError(ew);
}

std::array<unsigned char, kCryptoBoxHalfNonceBytes>&
CAresResolver::Query::getNonce() {
  return nonce_;
}

void CAresResolver::Query::setDnsCryptUsed(bool used, uint32_t serial) {
  dnscryptUsed_ = true;
  dnsEvent_.addMeta(TraceFieldType::DNSCryptUsed, used ? "1" : "0");
  dnsEvent_.addMeta(TraceFieldType::DNSCryptCertSerial,
                    folly::to<std::string>(serial));
}

std::chrono::steady_clock::time_point&
CAresResolver::Query::getLastNonceTimeRef() {
  return resolver_->getLastNonceTimeRef();
}

namespace proxygen {
// Helper class for issuing a multiple queries in parallel and aggregating the
// responses.
//
// Since this operates on multiple queries, the semantics for the resolution
// callbacks are a bit different: the query terminates when all resolutions
// have completed (timed out, succeeded, etc); the query invokes its error
// callback only if there were no results and at least one of the requests
// failed.
//
// This is used for querying both IPv4 and IPv6 addresses at the same time and
// returning whatever results were were able to find.
class CAresResolver::MultiQuery
    : public QueryBase
    , private DNSResolver::ResolutionCallback {
 public:
  MultiQuery(CAresResolver* resolver, std::string name)
      : resolver_(resolver), name_(name) {
  }

  ~MultiQuery() override {
  }

  void resolve(ResolutionCallback* callback,
               const std::list<Query*>& queries,
               std::chrono::milliseconds timeout) {
    CHECK(callback_ == nullptr);
    CHECK(callback != nullptr);
    CHECK_EQ(0, queries_);

    callback_ = callback;
    callback_->insertQuery(this);
    queries_ = queries.size();
    startTime_ = getCurrentTime();

    for (auto q : queries) {
      q->setDnsCryptUsed(dnsCryptUsed_, serial_);
      insertQuery(q);
      q->resolve(this, timeout);
    }
  }

  // QueryBase method
  void cancelResolutionImpl() override {
    callback_ = nullptr;
    // Cancel all outstanding sub-queries (ResolutionCallback interface)
    this->cancelResolution();

    // Deletes this object, but any outstanding sub-queries will hang around to
    // eat the replies
    finish();
  }

  void setDnsCryptUsed(bool used, uint32_t serial) {
    dnsCryptUsed_ = used;
    serial_ = serial;
  }

 private:
  CAresResolver* resolver_;
  ResolutionCallback* callback_{nullptr};
  TimePoint startTime_;
  folly::exception_wrapper error_;
  std::string name_;
  std::vector<Answer> answers_;
  uint16_t queries_{0};
  bool dnsCryptUsed_{false};
  uint32_t serial_{0};

  void resolutionSuccess(std::vector<Answer> a) noexcept override {
    --queries_;
    answers_.insert(answers_.end(), a.begin(), a.end());

    // Still waiting for some queries to terminate
    if (queries_ > 0) {
      return;
    }

    if (answers_.empty() && !error_) {
      error_ = folly::make_exception_wrapper<Exception>(
          NODATA, "No answer in MultiQuery for " + name_);
    }

    finish();
  }

  void resolutionError(const folly::exception_wrapper& ew) noexcept override {
    --queries_;

    // NOTE this will overwrite an existing error.
    if (ew) {
      error_ = ew;
    }

    // Still waiting for some queries to terminate. Track what went wrong so
    // that we can use it later
    if (queries_ > 0) {
      return;
    }

    finish();
  }

  void finish() {
    if (callback_) {
      ResolutionCallback* cb = callback_;
      CAresResolver* resolver = resolver_;
      std::chrono::milliseconds resolutionTime = millisecondsSince(startTime_);
      std::vector<Answer> answers(std::move(answers_));
      folly::exception_wrapper error = error_;

      callback_ = nullptr;
      cb->eraseQuery(this);

      if (!answers.empty()) {
        resolver->getStatsCollector()->recordSuccess(answers, resolutionTime);
        cb->resolutionSuccess(std::move(answers));
      } else {
        if (!error) {
          error = folly::make_exception_wrapper<Exception>(
              UNKNOWN, "Unknown error in MultiQuery for " + name_);
        }

        resolver->getStatsCollector()->recordError(error, resolutionTime);
        cb->resolutionError(error);
      }
    }

    delete this;
  }
};

// Helper class for managing DNS sockets
class CAresResolver::SocketHandler : public folly::EventHandler {
 public:
  SocketHandler(CAresResolver* resolver,
                folly::EventBase* base,
                int sock,
                ares_channel channel)
      : EventHandler(base, folly::NetworkSocket::fromFd(sock)),
        resolver_(resolver),
        sock_(sock),
        channel_(channel) {
  }
  ~SocketHandler() override {
  }

  void handlerReady(uint16_t events) noexcept override {
    int rsock = (events & EventHandler::READ) ? sock_ : ARES_SOCKET_BAD;
    int wsock = (events & EventHandler::WRITE) ? sock_ : ARES_SOCKET_BAD;

    DelayedDestruction::DestructorGuard dg(resolver_);
    ares_process_fd(channel_, rsock, wsock);
  }

 private:
  CAresResolver* resolver_;
  int sock_;
  ares_channel channel_;
};

CAresResolver::CAresResolver()
    : base_(nullptr),
      channel_(),
      channelRefcnt_(0),
      socketHandlers_(),
      servers_(),
      port_(0),
      statsCollector_(&nullStatsCollector) {
}

CAresResolver::~CAresResolver() {
  if (channel_) {
    ares_set_read_callback(channel_, nullptr, nullptr, nullptr);
    ares_set_write_callback(channel_, nullptr, nullptr, nullptr);
    ares_destroy(channel_);
  }

  // Once ares_destroy() has run, it should have invoked dnsSocketReady() on
  // all of our sockets to clean them up
  LOG_IF(DFATAL, !socketHandlers_.empty())
      << "Found orphaned sockets after ares_destroy()";
}

void CAresResolver::attachEventBase(EventBase* base) {
  LOG_IF(DFATAL, base_ != nullptr)
      << "Overwriting existing non-nullptr EventBase";

  base_ = base;
}

const folly::EventBase* CAresResolver::getEventBase() {
  return base_;
}

void CAresResolver::setSerializedServers() {
  serializedResolvers_ = "";
  for (auto& server : servers_) {
    serializedResolvers_ += server.getAddressStr();
    serializedResolvers_ += ";";
  }
}

void CAresResolver::setServers(const list<SocketAddress>& servers) {
  servers_ = servers;
}

void CAresResolver::setPort(uint16_t port) {
  port_ = port;
}

void CAresResolver::setStatsCollector(DNSResolver::StatsCollector* sc) {
  statsCollector_ = sc;
}

DNSResolver::StatsCollector* CAresResolver::getStatsCollector() const {
  return statsCollector_;
}

void CAresResolver::init() {
  CHECK(base_ != nullptr);

  // Initialize our channel
  int optmask = 0;
  ares_options opts;
  memset(&opts, 0, sizeof(opts));

  opts.sock_state_cb = CAresResolver::dnsSocketReady;
  opts.sock_state_cb_data = this;
  optmask |= ARES_OPT_SOCK_STATE_CB;

  opts.lookups = const_cast<char*>("b");
  optmask |= ARES_OPT_LOOKUPS;

  opts.flags = ARES_FLAG_STAYOPEN;
  optmask |= ARES_OPT_FLAGS;

  // Set the timeout to something obscenely large that will "never" fire since
  // we're managing timeouts ourself on a per-request basis
  opts.timeout = kMaxTimeout.count();
  optmask |= ARES_OPT_TIMEOUTMS;

  if (port_) {
    opts.udp_port = opts.tcp_port = port_;
    optmask |= ARES_OPT_UDP_PORT | ARES_OPT_TCP_PORT;
  }

  int err = ares_init_options(&channel_, &opts, optmask);
  if (err != ARES_SUCCESS) {
    LOG(DFATAL) << "ares_init_options() failed: " << ares_strerror(err);
    return;
  }

  // Set nameservers that we want to use
  if (!servers_.empty()) {
    setSerializedServers();

    unique_ptr<ares_addr_node[]> ares_addrs(
        new ares_addr_node[servers_.size()]);

    ares_addr_node* ares_addr = &ares_addrs[0];
    for (const auto& server : servers_) {
      ares_addr->next = nullptr;
      if (ares_addr > &ares_addrs[0]) {
        (ares_addr - 1)->next = ares_addr;
      }

      ares_addr->family = server.getFamily();

      switch (ares_addr->family) {
        case AF_INET:
          ares_addr->addr.addr4 = server.getIPAddress().asV4().toAddr();
          break;

        case AF_INET6: {
          in6_addr addr6 = server.getIPAddress().asV6().toAddr();
          memcpy(&ares_addr->addr.addr6, &addr6, sizeof(addr6));
          break;
        }

        default:
          LOG(DFATAL) << "Unknown address type " << ares_addr->family
                      << "; failing to change nameservers";
          return;
      }

      ++ares_addr;
    }

    err = ares_set_servers(channel_, ares_addrs.get());
    if (err != ARES_SUCCESS) {
      LOG(DFATAL) << "ares_set_servers() failed: " << ares_strerror(err);
      return;
    }
  }
}

void CAresResolver::resolveAddress(DNSResolver::ResolutionCallback* cb,
                                   const SocketAddress& address,
                                   std::chrono::milliseconds timeout) {
  if (timeout > kMaxTimeout) {
    LOG(WARNING) << "Attempt to resolve " << address.getAddressStr()
                 << " specified with "
                 << "timeout of " << timeout.count() << "ms; "
                 << "clamping to " << kMaxTimeout.count() << "ms";
    timeout = kMaxTimeout;
  }

  if (address.getFamily() != AF_INET && address.getFamily() != AF_INET6) {
    LOG(ERROR) << "Unsupported address family " << address.getFamily();
    auto ew = folly::make_exception_wrapper<Exception>(
        INVALID,
        folly::to<std::string>("Unsupported address family: ",
                               address.getFamily()));
    cb->resolutionError(ew);
    return;
  }

  /* TraceEvent Initialization */
  TraceEventContext teContext = TraceEventContext();
  TraceEvent dnsEvent =
      TraceEvent(TraceEventType::DnsResolution, teContext.parentID);
  dnsEvent.addMeta(TraceFieldType::NumberResolvers, servers_.size());
  dnsEvent.addMeta(TraceFieldType::RequestFamily, address.getFamily());

  auto q = new Query(this,
                     RecordType::kPtr,
                     DNSResolver::getPtrName(address),
                     true,
                     std::move(dnsEvent),
                     &timeUtil_,
                     std::move(teContext));
  q->setDnsCryptUsed(false, 0);
  q->resolve(cb, timeout);
}

bool CAresResolver::resolveLocalhost(ResolutionCallback* cb,
                                     const std::string& host,
                                     sa_family_t family) {
  if (host != "localhost") {
    return false;
  }

  // Resolve localhost because we can't use /etc/hosts
  vector<Answer> answers;
  const std::string preferredAddr = (family == AF_INET6) ? "::1" : "127.0.0.1";
  const std::string otherAddr = (preferredAddr == "::1") ? "127.0.0.1" : "::1";

  if (family == AF_UNSPEC) {
    // Return both addresses if they are loopback
    SocketAddress prefSAddr(preferredAddr, 0);
    SocketAddress otherSAddr(otherAddr, 0);

    if (prefSAddr.isLoopbackAddress()) {
      Answer ans(std::chrono::seconds(kLiteralTTL), std::move(prefSAddr));
      answers.push_back(std::move(ans));
    }

    if (otherSAddr.isLoopbackAddress()) {
      Answer ans(std::chrono::seconds(kLiteralTTL), std::move(otherSAddr));
      answers.push_back(std::move(ans));
    }

  } else {
    SocketAddress addr(preferredAddr, 0);
    if (!addr.isLoopbackAddress()) {
      addr = SocketAddress(otherAddr, 0);
    }

    Answer ans(std::chrono::seconds(kLiteralTTL), std::move(addr));
    answers.push_back(std::move(ans));
  }

  if (answers.empty()) {
    auto ew = folly::make_exception_wrapper<Exception>(
        NODATA, "No resolution for Localhost");
    cb->resolutionError(ew);
  } else {
    cb->resolutionSuccess(std::move(answers));
  }
  return true;
}

void CAresResolver::resolveHostname(DNSResolver::ResolutionCallback* cb,
                                    const std::string& host,
                                    std::chrono::milliseconds timeout,
                                    sa_family_t family,
                                    TraceEventContext teContext) {
  if (resolveLiterals(cb, host, family)) {
    return;
  }

  if (resolveLocalhost(cb, host, family)) {
    return;
  }

  if (timeout > kMaxTimeout) {
    LOG(WARNING) << "Attempt to resolve " << host << " specified with "
                 << "timeout of " << timeout.count() << "ms; clamping to "
                 << kMaxTimeout.count() << "ms";
    timeout = kMaxTimeout;
  }

  // If we are a dnscrypt enabled resolver, we need to have a valid cert
  // context installed before making any queries
  TraceEvent dnsEvent =
      TraceEvent(TraceEventType::DnsResolution, teContext.parentID);
  dnsEvent.addMeta(TraceFieldType::NumberResolvers, servers_.size());
  dnsEvent.addMeta(TraceFieldType::ResolversSerialized, serializedResolvers_);
  dnsEvent.addMeta(TraceFieldType::RequestFamily, family);

  if (resolveSRVRecord_) {
    auto q = new Query(this,
                       RecordType::kSRV,
                       host,
                       true,
                       std::move(dnsEvent),
                       &timeUtil_,
                       std::move(teContext));
    cb->insertQuery(q);
    q->setDnsCryptUsed(false, 0);
    q->resolve(cb, std::chrono::milliseconds(timeout));
  }

  if (family == AF_INET) {
    auto q = new Query(this,
                       RecordType::kA,
                       host,
                       true,
                       std::move(dnsEvent),
                       &timeUtil_,
                       std::move(teContext));
    cb->insertQuery(q);
    q->setDnsCryptUsed(false, 0);
    q->resolve(cb, std::chrono::milliseconds(timeout));
  } else if (family == AF_INET6) {
    auto q = new Query(this,
                       RecordType::kAAAA,
                       host,
                       true,
                       std::move(dnsEvent),
                       &timeUtil_,
                       std::move(teContext));
    cb->insertQuery(q);
    q->setDnsCryptUsed(false, 0);
    q->resolve(cb, std::chrono::milliseconds(timeout));
  } else if (family == AF_UNSPEC) {
    auto mq = new MultiQuery(this, host);
    mq->setDnsCryptUsed(false, 0);
    cb->insertQuery(mq);
    mq->resolve(cb,
                {new Query(this,
                           RecordType::kA,
                           host,
                           false,
                           dnsEvent,
                           &timeUtil_,
                           teContext),
                 new Query(this,
                           RecordType::kAAAA,
                           host,
                           false,
                           dnsEvent,
                           &timeUtil_,
                           teContext)},
                std::chrono::milliseconds(timeout));
  } else {
    LOG(DFATAL) << "Unsupported family specified: " << family;
    auto ew = folly::make_exception_wrapper<Exception>(
        INVALID,
        folly::to<std::string>("Unsupported address family: ", family));
    cb->resolutionError(ew);
  }
}

void CAresResolver::resolveMailExchange(DNSResolver::ResolutionCallback* cb,
                                        const std::string& domain,
                                        std::chrono::milliseconds timeout) {
  if (timeout > kMaxTimeout) {
    LOG(WARNING) << "Attempt to resolve mail exchange info for " << domain
                 << " specified with "
                 << "timeout of " << timeout.count() << "ms; "
                 << "clamping to " << kMaxTimeout.count() << "ms";
    timeout = kMaxTimeout;
  }

  /* TraceEvent Initialization */
  TraceEventContext teContext = TraceEventContext();
  TraceEvent dnsEvent =
      TraceEvent(TraceEventType::DnsResolution, teContext.parentID);
  dnsEvent.addMeta(TraceFieldType::NumberResolvers, servers_.size());

  auto q = new Query(this,
                     RecordType::kMX,
                     domain,
                     true,
                     std::move(dnsEvent),
                     &timeUtil_,
                     std::move(teContext));
  q->setDnsCryptUsed(false, 0);
  q->resolve(cb, timeout);
}

bool CAresResolver::resolveLiterals(DNSResolver::ResolutionCallback* cb,
                                    const std::string& host,
                                    sa_family_t family) {
  addrinfo hints{};
  hints.ai_family = family;
  hints.ai_socktype = SOCK_STREAM;
  hints.ai_flags = AI_PASSIVE | AI_NUMERICSERV | AI_NUMERICHOST;

  addrinfo* ainfos;
  if (getaddrinfo(host.c_str(), nullptr, &hints, &ainfos) != 0) {
    return false;
  }

  vector<Answer> answers;
  for (addrinfo* ai = ainfos; ai != nullptr; ai = ai->ai_next) {
    Answer ans(std::chrono::seconds(kLiteralTTL), ai->ai_addr);
    answers.push_back(std::move(ans));
  }

  freeaddrinfo(ainfos);

  cb->resolutionSuccess(std::move(answers));
  return true;
}

void CAresResolver::query(const std::string& name,
                          RecordType type,
                          ares_callback cb,
                          void* cb_data) {
  if (++channelRefcnt_ == 1) {
    for (auto& shp : socketHandlers_) {
      shp.second->registerHandler(shp.second->getRegisteredEvents());
    }
  }

  ares_query(channel_,
             name.c_str(),
             1 /* ns_c_in */,
             static_cast<int>(type),
             cb,
             cb_data);
}

void CAresResolver::queryFinished() {
  if (channelRefcnt_ == 0) {
    LOG(ERROR) << "Invalid channel refcount in CAresResolver::queryFinished()";
    return;
  }
  if (--channelRefcnt_ == 0) {
    for (auto& shp : socketHandlers_) {
      shp.second->registerInternalHandler(shp.second->getRegisteredEvents());
    }
  }
}

void CAresResolver::dnsSocketReady(void* data,
                                   ares_socket_t sock,
                                   int read,
                                   int write) {
  CAresResolver* self = static_cast<CAresResolver*>(data);
  auto it = self->socketHandlers_.find(sock);

  // Ares is done with this socket; stop watching it
  if (!read && !write) {
    LOG_IF(DFATAL, it == self->socketHandlers_.end())
        << "dnsSocketReady() asked to close a socket that we don't kow about";
    if (it != self->socketHandlers_.end()) {
      self->socketHandlers_.erase(it);
    }

    return;
  }

  // Find the EventHandler that's managing our socket, creating one if it does
  // not already exist
  SocketHandler* shp = nullptr;
  if (it == self->socketHandlers_.end()) {
    shp = new SocketHandler(self, self->base_, sock, self->channel_);
    self->socketHandlers_[sock].reset(shp);
  } else {
    shp = it->second.get();
  }

  // Update the set of events that we want to listen for
  uint16_t events = EventHandler::PERSIST;
  events |= (read) ? EventHandler::READ : 0;
  events |= (write) ? EventHandler::WRITE : 0;
  if (!shp->registerHandler(events)) {
    LOG(DFATAL) << "Failed to register SocketHandler";
  }
}

void CAresResolver::initGlobal() {
  int err = ares_library_init(ARES_LIB_INIT_ALL);
  LOG_IF(FATAL, err != 0) << "ares_library_init() failed";
}

void CAresResolver::destroyGlobal() {
  ares_library_cleanup();
}

std::chrono::steady_clock::time_point& CAresResolver::getLastNonceTimeRef() {
  return lastNonceTimeStamp_;
}

namespace detail {
folly::Expected<std::vector<DNSResolver::Answer>, ParseError> parseTxtRecords(
    unsigned char* aresBuffer, int bufferLen) noexcept {
  std::vector<DNSResolver::Answer> answers;
  struct ares_txt_reply* txts = nullptr;
  auto status = ares_parse_txt_reply(aresBuffer, bufferLen, &txts);
  AresDataUniquePtr<ares_txt_reply> txtsReply(txts);
  if (status == ARES_SUCCESS) {
    while (txts != nullptr) {
      auto rawTxt = std::shared_ptr<folly::IOBuf>(
          folly::IOBuf::copyBuffer(txts->txt, txts->length));
      const std::chrono::seconds ttl = std::chrono::seconds(0);
      answers.emplace_back(
          ttl, rawTxt, DNSResolver::Answer::AnswerType::AT_TXT);
      txts = txts->next;
    }
  } else {
    ParseError err;
    err.status = DNSResolver::ResolutionStatus::PARSE_ERROR;
    err.msg = folly::to<std::string>("Failed to parse TXT answer ", status);
    return folly::makeUnexpected(std::move(err));
  }

  return folly::makeExpected<ParseError>(std::move(answers));
}

folly::Expected<std::vector<DNSResolver::Answer>, ParseError> parseSrvRecords(
    unsigned char* aresBuffer, int bufferLen) noexcept {
  std::vector<DNSResolver::Answer> answers;
  struct ares_srv_reply* srvs = nullptr;
  auto status = ares_parse_srv_reply(aresBuffer, bufferLen, &srvs);
  AresDataUniquePtr<ares_srv_reply> srvsReply(srvs);
  if (status == ARES_SUCCESS) {
    while (srvs != nullptr) {
      const std::chrono::seconds ttl = std::chrono::seconds(0);
      answers.emplace_back(
          ttl, srvs->host, srvs->port, DNSResolver::Answer::AnswerType::AT_SRV);
      srvs = srvs->next;
    }
  } else {
    ParseError err;
    err.status = DNSResolver::ResolutionStatus::PARSE_ERROR;
    err.msg = folly::to<std::string>("Failed to parse SRV answer ", status);
    return folly::makeUnexpected(std::move(err));
  }

  return folly::makeExpected<ParseError>(std::move(answers));
}

folly::Expected<std::vector<DNSResolver::Answer>, ParseError> parseMxRecords(
    unsigned char* aresBuffer, int bufferLen) noexcept {
  std::vector<DNSResolver::Answer> answers;
  struct ares_mx_reply* mxs = nullptr;
  auto status = ares_parse_mx_reply(aresBuffer, bufferLen, &mxs);
  AresDataUniquePtr<ares_mx_reply> mxsReply(mxs);
  if (status == ARES_SUCCESS) {
    while (mxs != nullptr) {
      // C-ares doesn't support yet parsing the ttl for MX records
      // (https://github.com/c-ares/c-ares/issues/387).
      // For the time being, I'm using an arbitrary ttl of 0 seconds,
      // thus disabling any caching for the records (no caching is
      // bad, but only from a performance perspective. Stale records, on
      // the other hand, would violate the ttl set on the DNS server)
      const std::chrono::seconds ttl = std::chrono::seconds(0);
      answers.emplace_back(ttl, mxs->priority, mxs->host);
      mxs = mxs->next;
    }
  } else {
    ParseError err;
    err.status = DNSResolver::ResolutionStatus::PARSE_ERROR;
    err.msg = folly::to<std::string>("Failed to parse MX answer ", status);
    return folly::makeUnexpected(std::move(err));
  }

  return folly::makeExpected<ParseError>(std::move(answers));
}

} // namespace detail
} // namespace proxygen
