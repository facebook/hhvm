<?hh
/*
 *  Copyright (c) 2004-present, Facebook, Inc.
 *  All rights reserved.
 *
 *  This source code is licensed under the MIT license found in the
 *  LICENSE file in the root directory of this source tree.
 *
 */

namespace HH\Lib\TCP;

use namespace HH\Lib\{OS, Network};
use namespace HH\Lib\_Private\{_Network, _OS, _TCP};

final class Server implements Network\Server<CloseableSocket> {
  /** Host and port */
  const type TAddress = (string, int);

  private function __construct(private OS\FileDescriptor $impl) {
  }

  /** Create a bound and listening instance */
  public static async function createAsync(
    Network\IPProtocolVersion $ipv,
    string $host,
    int $port,
    ServerOptions $opts = shape(),
  ): Awaitable<this> {
    // FIXME: rewrite this once we have OS\getaddrinfo
    switch ($ipv) {
      case Network\IPProtocolVersion::IPV6:
        try {
          $in6_addr = OS\inet_pton_inet6($host);
        } catch (OS\ErrnoException $e) {
          if ($e->getErrno() !== OS\Errno::EINVAL) {
            throw $e;
          }
          $host = _Network\resolve_hostname(OS\AddressFamily::AF_INET6, $host);
          if ($host === null) {
            $host = _Network\resolve_hostname(
              OS\AddressFamily::AF_INET6,
              'localhost',
            );
            if ($host === null) {
              // match bind() errno
              _OS\throw_errno(
                OS\Errno::EADDRNOTAVAIL,
                'failed to resolve localhost to IPv6, assuming IPv6 unsupported',
              );
            }
            throw $e;
          }
          $in6_addr = OS\inet_pton_inet6($host);
        }
        $sd = OS\SocketDomain::PF_INET6;
        $sa = new OS\sockaddr_in6(
          $port,
          /* flowInfo = */ 0,
          $in6_addr,
          /* scopeID = */ 0,
        );
        break;
      case Network\IPProtocolVersion::IPV4:
        try {
          $in_addr = OS\inet_pton_inet($host);
        } catch (OS\ErrnoException $e) {
          if ($e->getErrno() !== OS\Errno::EINVAL) {
            throw $e;
          }
          $host = _Network\resolve_hostname(OS\AddressFamily::AF_INET, $host);
          if ($host === null) {
            $host = _Network\resolve_hostname(
              OS\AddressFamily::AF_INET,
              'localhost',
            );
            if ($host === null) {
              // match bind() errno
              _OS\throw_errno(
                OS\Errno::EADDRNOTAVAIL,
                'failed to resolve localhost to IPv4, assuming IPv4 unsupported',
              );
            }
            throw $e;
          }

          $in_addr = OS\inet_pton_inet($host);
        }
        $sd = OS\SocketDomain::PF_INET;
        $sa = new OS\sockaddr_in($port, $in_addr);
        break;
    }

    return await _Network\socket_create_bind_listen_async(
      $sd,
      OS\SocketType::SOCK_STREAM,
      /* proto = */ 0,
      $sa,
      $opts['backlog'] ?? 16,
      $opts['socket_options'] ?? shape(),
    )
      |> new self($$);
  }

  public async function nextConnectionAsync(): Awaitable<CloseableSocket> {
    return await _Network\socket_accept_async($this->impl)
      |> new _TCP\CloseableTCPSocket($$);
  }

  public function getLocalAddress(): (string, int) {
    $sa = OS\getsockname($this->impl);
    if ($sa is OS\sockaddr_in) {
      return tuple(OS\inet_ntop_inet($sa->getAddress()), $sa->getPort());
    }
    if ($sa is OS\sockaddr_in6) {
      return tuple(OS\inet_ntop_inet6($sa->getAddress()), $sa->getPort());
    }
    _OS\throw_errno(
      OS\Errno::EAFNOSUPPORT,
      "%s is not supported",
      OS\AddressFamily::getNames()[$sa->getFamily()],
    );
  }

  public function stopListening(): void {
    OS\close($this->impl);
  }
}
