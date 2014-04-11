// Copyright 2004-present Facebook.  All rights reserved.
#include "TransportInfo.h"

#include <sys/types.h>
#include <sys/socket.h>

#include "thrift/lib/cpp/async/TAsyncSocket.h"

using apache::thrift::async::TAsyncSocket;
using std::chrono::microseconds;
using std::map;
using std::string;

namespace facebook { namespace proxygen {

TransportInfo::TransportInfo()
    : acceptTime(),
      rtt(0),
      validTcpinfo(false),
      tcpinfoErrno(0),
      setupTime(0),
      totalBytes(0),
      timeToFirstByte(-1),
      timeToLastByte(-1),
      lastByteAckLatency(-1),
      proxyLatency(-1),
      clientLatency(-1),
      serverLatency(-1),
      connectLatency(-1) {
}

bool TransportInfo::initWithSocket(const TAsyncSocket* sock) {
  if (!TransportInfo::readTcpInfo(&tcpinfo, sock)) {
    tcpinfoErrno = errno;
    return false;
  }
  rtt = microseconds(tcpinfo.tcpi_rtt);
  validTcpinfo = true;
  return true;
}

int64_t TransportInfo::readRTT(const TAsyncSocket* sock) {
  struct tcp_info tcpinfo;
  if (!TransportInfo::readTcpInfo(&tcpinfo, sock)) {
    return -1;
  }
  return tcpinfo.tcpi_rtt;
}

bool TransportInfo::readTcpInfo(struct tcp_info* tcpinfo,
                                const TAsyncSocket* sock) {
  socklen_t len = sizeof(struct tcp_info);
  if (!sock) {
    return false;
  }
  if (getsockopt(sock->getFd(), IPPROTO_TCP,
                 TCP_INFO, (void*) tcpinfo, &len) < 0) {
    VLOG(4) << "Error calling getsockopt(): " << strerror(errno);
    return false;
  }
  return true;
}

TransportInfo::~TransportInfo() {
}

}} // facebook::proxygen
