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

#include <wangle/acceptor/TransportInfo.h>

#include <sys/types.h>

#include <folly/String.h>
#include <folly/io/async/AsyncSocket.h>
#include <folly/portability/Sockets.h>

using std::chrono::microseconds;

namespace wangle {

bool TransportInfo::initWithSocket(const folly::AsyncSocket* sock) {
#if defined(__linux__) || defined(__FreeBSD__) || defined(__APPLE__)
  if (!TransportInfo::readTcpInfo(&tcpinfo, sock)) {
    tcpinfoErrno = errno;
    return false;
  }
#ifdef __APPLE__
  rtt = microseconds(tcpinfo.tcpi_srtt * 1000);
  rtt_var = tcpinfo.tcpi_rttvar * 1000;
  rto = tcpinfo.tcpi_rto * 1000;
  rtx_tm = -1;
  mss = tcpinfo.tcpi_maxseg;
  cwndBytes = tcpinfo.tcpi_snd_cwnd;
  if (mss > 0) {
    cwnd = (cwndBytes + mss - 1) / mss;
  }
#else
  rtt = microseconds(tcpinfo.tcpi_rtt);
  rtt_var = tcpinfo.tcpi_rttvar;
  rto = tcpinfo.tcpi_rto;
#ifdef __FreeBSD__
  rtx_tm = tcpinfo.__tcpi_retransmits;
#else
  rtx_tm = tcpinfo.tcpi_retransmits;
#endif
  mss = tcpinfo.tcpi_snd_mss;
  cwnd = tcpinfo.tcpi_snd_cwnd;
  cwndBytes = cwnd * mss;
#endif // __APPLE__
  ssthresh = tcpinfo.tcpi_snd_ssthresh;
#if __GLIBC__ >= 2 && __GLIBC_MINOR__ >= 17
  rtx = tcpinfo.tcpi_total_retrans;
#else
  rtx = -1;
#endif // __GLIBC__ >= 2 && __GLIBC_MINOR__ >= 17
  validTcpinfo = true;
#else
  (sock); // unused
  tcpinfoErrno = EINVAL;
  rtt = microseconds(-1);
  rtt_var = -1;
  rtx = -1;
  rtx_tm = -1;
  rto = -1;
  cwnd = -1;
  mss = -1;
  ssthresh = -1;
#endif
  return true;
}

#if defined(__linux__) || defined(__FreeBSD__)
bool TransportInfo::readTcpCongestionControl(const folly::AsyncSocket* sock) {
  if (!sock) {
    return false;
  }
#ifdef TCP_CONGESTION
  // TCP_CA_NAME_MAX from <net/tcp.h> (Linux) or <netinet/tcp.h> (FreeBSD)
  constexpr unsigned int kTcpCaNameMax = 16;
  std::array<char, kTcpCaNameMax> tcpCongestion{{0}};
  socklen_t optlen = tcpCongestion.size();
  if (getsockopt(
          sock->getNetworkSocket().toFd(),
          IPPROTO_TCP,
          TCP_CONGESTION,
          tcpCongestion.data(),
          &optlen) < 0) {
    VLOG(4) << "Error calling getsockopt(): " << folly::errnoStr(errno);
    return false;
  }

  caAlgo = std::string(tcpCongestion.data());
  return true;
#else // TCP_CONGESTION
  return false;
#endif // TCP_CONGESTION
}

bool TransportInfo::readMaxPacingRate(const folly::AsyncSocket* sock) {
  if (!sock) {
    return false;
  }
#ifdef SO_MAX_PACING_RATE
  socklen_t optlen = sizeof(maxPacingRate);
  if (getsockopt(
          sock->getNetworkSocket().toFd(),
          SOL_SOCKET,
          SO_MAX_PACING_RATE,
          &maxPacingRate,
          &optlen) < 0) {
    VLOG(4) << "Error calling getsockopt(): " << folly::errnoStr(errno);
    return false;
  }
  return true;
#else // SO_MAX_PACING_RATE
  return false;
#endif // SO_MAX_PACING_RATE
}
#endif // defined(__linux__) || defined(__FreeBSD__)

int64_t TransportInfo::readRTT(const folly::AsyncSocket* sock) {
#if defined(__linux__) || defined(__FreeBSD__) || defined(__APPLE__)
  tcp_info tcpinfo;
  if (!TransportInfo::readTcpInfo(&tcpinfo, sock)) {
    return -1;
  }
#endif
#if defined(__linux__) || defined(__FreeBSD__)
  return tcpinfo.tcpi_rtt;
#elif defined(__APPLE__)
  return tcpinfo.tcpi_srtt;
#else
  (sock); // unused
  return -1;
#endif
}

#ifdef __APPLE__
#define TCP_INFO TCP_CONNECTION_INFO
#endif

#if defined(__linux__) || defined(__FreeBSD__) || defined(__APPLE__)
bool TransportInfo::readTcpInfo(
    tcp_info* tcpinfo,
    const folly::AsyncSocket* sock) {
  socklen_t len = sizeof(tcp_info);
  if (!sock) {
    return false;
  }
  if (getsockopt(
          sock->getNetworkSocket().toFd(),
          IPPROTO_TCP,
          TCP_INFO,
          (void*)tcpinfo,
          &len) < 0) {
    VLOG(4) << "Error calling getsockopt(): " << folly::errnoStr(errno);
    return false;
  }
  return true;
}
#endif

} // namespace wangle
