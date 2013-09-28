/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2013 Facebook, Inc. (http://www.facebook.com)     |
   +----------------------------------------------------------------------+
   | This source file is subject to version 3.01 of the PHP license,      |
   | that is bundled with this package in the file LICENSE, and is        |
   | available through the world-wide-web at the following url:           |
   | http://www.php.net/license/3_01.txt                                  |
   | If you did not receive a copy of the PHP license and are unable to   |
   | obtain it through the world-wide-web, please send a note to          |
   | license@php.net so we can mail you a copy immediately.               |
   +----------------------------------------------------------------------+
*/

#ifndef incl_HPHP_NETWORK_H_
#define incl_HPHP_NETWORK_H_

#include "hphp/util/base.h"
#include <string>
#include <netdb.h>
#include <sys/socket.h>
#include <stdlib.h>

/**
 * Network utility functions.
 */
namespace HPHP { namespace Util {
///////////////////////////////////////////////////////////////////////////////
// thread-safe network functions

class HostEnt {
public:
  HostEnt() : tmphstbuf(nullptr) {}
  ~HostEnt() { if (tmphstbuf) free(tmphstbuf);}

  struct hostent hostbuf;
  char *tmphstbuf;
  int herr;
};

// Extract out the scheme, host and port from a URL
// http://192.168.1.0:80
//   http, 192.168.1.0, 80
// http://[2a03:2880::1]:80
//   http, 2a03:2880::1, 80
// ssl://192.168.1.0:443
//   ssl, 192.168.1.0, 80
// ssl://[2a03:2880::1]:443
//   ssl, 2a03:2880::1, 443
class HostURL {
public:
  explicit HostURL(const std::string &hosturl, int port = 0);

  bool isIPv6() const {return m_ipv6;}
  bool isValid() const {return m_valid;}
  uint16_t getPort() const {return m_port;}
  std::string getScheme() const {return m_scheme;}
  std::string getHost() const {return m_host;}
  std::string getHostURL() const {return m_hosturl;}

private:
  bool        m_ipv6;
  bool        m_valid;
  uint16_t    m_port;
  std::string m_scheme;
  std::string m_host;
  std::string m_hosturl;
};

bool safe_gethostbyname(const char *address, HostEnt &result);
std::string safe_inet_ntoa(struct in_addr &in);

///////////////////////////////////////////////////////////////////////////////
/**
 * Get local machine's primary IP address.
 */
std::string GetPrimaryIP();

///////////////////////////////////////////////////////////////////////////////
}}

#endif // incl_HPHP_NETWORK_H_
