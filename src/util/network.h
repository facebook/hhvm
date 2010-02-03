/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010 Facebook, Inc. (http://www.facebook.com)          |
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

#ifndef __NETWORK_H__
#define __NETWORK_H__

#include "base.h"
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
  HostEnt() : tmphstbuf(NULL) {}
  ~HostEnt() { if (tmphstbuf) free(tmphstbuf);}

  struct hostent hostbuf;
  char *tmphstbuf;
  int herr;
};

bool safe_gethostbyname(const char *address, HostEnt &result);
std::string safe_inet_ntoa(struct in_addr &in);

///////////////////////////////////////////////////////////////////////////////
/**
 * Get local machine's primary IP address.
 */
std::string GetPrimaryIP();

/**
 * Get network bytes per second.
 */
bool GetNetworkStats(const char *iface, int &in_bps, int &out_bps);

///////////////////////////////////////////////////////////////////////////////
}}

#endif // __NETWORK_H__
