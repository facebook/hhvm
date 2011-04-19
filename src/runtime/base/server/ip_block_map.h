/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010- Facebook, Inc. (http://www.facebook.com)         |
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

#ifndef __IP_BLOCK_MAP_H__
#define __IP_BLOCK_MAP_H__

#include <util/hdf.h>
#include <runtime/base/types.h>

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

DECLARE_BOOST_TYPES(IpBlockMap);
class IpBlockMap {
public:
  static bool ReadIPv4Address(const char *ip, unsigned int &start,
                              unsigned int &end);

public:
  IpBlockMap(Hdf config);

  bool isBlocking(const std::string &command, const std::string &ip) const;

private:
  struct IpRange {
    unsigned int start;
    unsigned int end;
    bool allow;
  };
  typedef hphp_hash_map<unsigned int, bool> IpMap;
  typedef std::vector<IpRange> IpRangeVec;

  DECLARE_BOOST_TYPES(Acl);
  class Acl {
  public:
    IpMap ips; // ip => true: allow; false: deny
    IpRangeVec ranges; // (start, end) => true: allow; false: deny
  };
  StringToAclPtrMap m_acls; // location => acl

  static void LoadIpList(AclPtr acl, Hdf hdf, bool allow);
};

///////////////////////////////////////////////////////////////////////////////
}

#endif // __IP_BLOCK_MAP_H__
