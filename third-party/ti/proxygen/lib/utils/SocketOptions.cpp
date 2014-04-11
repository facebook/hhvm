#include "ti/proxygen/lib/utils/SocketOptions.h"
#include <sys/socket.h>
#include <netinet/tcp.h>

namespace facebook { namespace proxygen {

apache::thrift::async::TAsyncSocket::OptionMap filterIPSocketOptions(
  const apache::thrift::async::TAsyncSocket::OptionMap& allOptions,
  const int addrFamily) {
  apache::thrift::async::TAsyncSocket::OptionMap opts;
  int exclude;
  if (addrFamily == AF_INET) {
    exclude = IPPROTO_IPV6;
  } else if (addrFamily == AF_INET6) {
    exclude = IPPROTO_IP;
  } else {
    LOG(FATAL) << "Address family " << addrFamily << " was not IPv4 or IPv6";
    return opts;
  }
  for (const auto& opt: allOptions) {
    if (opt.first.level != exclude) {
      opts[opt.first] = opt.second;
    }
  }
  return opts;
}

}}
