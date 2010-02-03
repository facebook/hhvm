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

#include <cpp/base/server/ip_block_map.h>

using namespace std;

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

void IpBlockMap::LoadIpList(hphp_string_map<bool> &ips, Hdf hdf, bool allow) {
  for (Hdf child = hdf.firstChild(); child.exists(); child = child.next()) {
    string ip = child.getString();
    size_t pos = ip.find('/');
    if (pos != string::npos) {
      ip = ip.substr(0, pos);
    }
    ips[ip] = allow;
  }
}

IpBlockMap::IpBlockMap(Hdf config) {
  for (Hdf hdf = config.firstChild(); hdf.exists(); hdf = hdf.next()) {
    AclPtr acl(new Acl());
    bool allow = hdf["AllowFirst"].getBool(false);
    if (allow) {
      LoadIpList(acl->ips, hdf["Ip.Deny"], false);
      LoadIpList(acl->ips, hdf["Ip.Allow"], true);
    } else {
      LoadIpList(acl->ips, hdf["Ip.Allow"], true);
      LoadIpList(acl->ips, hdf["Ip.Deny"], false);
    }

    string location = hdf["Location"].getString();
    if (!location.empty() && location[0] == '/') {
      location = location.substr(1);
    }
    m_acls[location] = acl;
  }
}

bool IpBlockMap::isBlocking(const std::string &command,
                            const std::string &ip) const {
  for (StringToAclPtrMap::const_iterator iter = m_acls.begin();
       iter != m_acls.end(); ++iter) {
    const string &path = iter->first;
    if (command.size() >= path.size() &&
        strncmp(command.c_str(), path.c_str(), path.size()) == 0) {
      hphp_string_map<bool> &ips = iter->second->ips;
      hphp_string_map<bool>::const_iterator iter2 = ips.find(ip);
      if (iter2 != ips.end()) {
        return !iter2->second;
      }
    }
  }
  return false;
}

///////////////////////////////////////////////////////////////////////////////
}
