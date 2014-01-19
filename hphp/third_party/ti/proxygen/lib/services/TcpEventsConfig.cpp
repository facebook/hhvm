#include "TcpEventsConfig.h"

#include "ti/proxygen/lib/utils/SocketOptions.h"

#include <ostream>
#include <algorithm>

using namespace apache::thrift::async;

namespace facebook { namespace proxygen {

namespace {
const char * const kSampleRateOpt = "rate";
const char * const kEnabledOpt = "enabled";
const char * const kMaxAckTrackedOpt = "max_ack_tracked";
const char * const kAckTimeoutOpt = "ack_timeout_s";
}

const bool     TcpEventsConfig::DEFAULT_ENABLED;
const double   TcpEventsConfig::DEFAULT_SAMPLE_RATE = 1;
const uint32_t TcpEventsConfig::KERNEL_LIMIT_ACK_TRACKED;
const uint32_t TcpEventsConfig::DEFAULT_ACK_TRACKED;
const uint64_t TcpEventsConfig::DEFAULT_ACK_TIMEOUT_MS;
const char * const TcpEventsConfig::ENTITY = "tcp_events";

std::ostream& operator<<(std::ostream& os, const TcpEventsConfig& cfg) {
  os << TcpEventsConfig::ENTITY << ": { "
     << kEnabledOpt << ": " << cfg.enabled_;
  if (cfg.enabled_) {
    os << ", "
       << kSampleRateOpt << ": " << cfg.sampleRate_ << ", "
       << kMaxAckTrackedOpt << ": " << cfg.maxAckTracked_ << ", "
       << kAckTimeoutOpt << ": " << cfg.ackTimeout_.count();
  }
  os << " }";
  return os;
}

TcpEventsConfig::TcpEventsConfig() : sampleRate_{DEFAULT_SAMPLE_RATE},
  samplingKey_{static_cast<uint32_t>(std::numeric_limits<uint32_t>::max() *
                                     sampleRate_)} {
}

void TcpEventsConfig::setMaxAckTracked(uint32_t n) {
  maxAckTracked_ = std::min(KERNEL_LIMIT_ACK_TRACKED, std::max(1u, n));
}

void TcpEventsConfig::updateSocketOptions(TAsyncSocket::OptionMap& opts) {
  const TAsyncSocket::OptionKey ackOpt{IPPROTO_TCP, TCP_TRACKING_OPTNAME};

  auto i = opts.find(ackOpt);
  // TODO: Remove TCP_TRACKING_OPTNAME from "socket_options" later.
  // All per VIP tcp event related config should stay under
  // the "tcp_events" json entity in the config.
  if (i != opts.end()) {
    setMaxAckTracked(i->second);
    opts.erase(i);
  }
}

}} // namespace facebook::proxygen
