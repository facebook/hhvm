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

#include <runtime/base/server/ip_block_map.h>
#include <util/logger.h>

using namespace std;

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

bool IpBlockMap::ReadIPv4Address(const char *ip, unsigned int &start,
                                 unsigned int &end) {
  start = end = 0;
  const char *p = ip;

  bool saw_digit = false;
  int octets = 0;
  unsigned int tp = 0;
  char ch;
  while ((ch = *p++) != '\0') {
    if (ch >= '0' && ch <= '9') {
      tp = tp * 10 + (unsigned int)(ch - '0');
      if (tp > 255) {
        Logger::Error("octet larger than 255: %s", ip);
        return false;
      }
      if (!saw_digit) {
        if (++octets > 4) {
          Logger::Error("more than 4 octets: %s", ip);
          return false;
        }
        saw_digit = true;
      }
    } else if (ch == '.' && saw_digit) {
      if (octets == 4) {
        Logger::Error("dot after 4 octet bits: %s", ip);
        return false;
      }
      start <<= 8;
      start += tp;
      tp = 0;
      saw_digit = false;
    } else if (ch == '/') {
      break;
    } else {
      Logger::Error("invalid character: %s", ip);
      return false;
    }
  }
  if (octets < 4) {
    Logger::Error("less than 4 octets: %s", ip);
    return false;
  }
  start <<= 8;
  start += tp;

  if (ch == '\0') {
    end = start;
    return true;
  }

  ASSERT(ch == '/');
  ch = *p++;
  if (ch == '\0') {
    Logger::Error("missing mask: %s", ip);
    return false;
  }
  if (ch < '0' || ch > '9') {
    Logger::Error("non-digit mask: %s", ip);
    return false;
  }
  tp = (unsigned int)(ch - '0');
  ch = *p++;
  if (ch != '\0') {
    if (ch < '0' || ch > '9') {
      Logger::Error("non-digit mask: %s", ip);
      return false;
    }
    tp = tp * 10 + (unsigned int)(ch - '0');
    ch = *p++;
  }
  if (ch != '\0' || tp > 31 || tp == 0) {
    Logger::Error("invalid mask: %s", ip);
    return false;
  }

  unsigned int mask = (1 << (32 - tp)) - 1;
  start &= ~mask;
  end = start | mask;
  return true;
}

void IpBlockMap::LoadIpList(AclPtr acl, Hdf hdf, bool allow) {
  for (Hdf child = hdf.firstChild(); child.exists(); child = child.next()) {
    string ip = child.getString();

    unsigned int start, end;
    if (ReadIPv4Address(ip.c_str(), start, end)) {
      ASSERT(end >= start);
      if (end - start < 1024) {
        for (unsigned int i = start; i <= end; i++) {
          acl->ips[i] = allow;
        }
      } else {
        acl->ranges.resize(acl->ranges.size() + 1);
        IpRange &range = acl->ranges.back();
        range.start = start;
        range.end = end;
        range.allow = allow;
      }
    }
  }
}

IpBlockMap::IpBlockMap(Hdf config) {
  for (Hdf hdf = config.firstChild(); hdf.exists(); hdf = hdf.next()) {
    AclPtr acl(new Acl());
    bool allow = hdf["AllowFirst"].getBool(false);
    if (allow) {
      LoadIpList(acl, hdf["Ip.Deny"], false);
      LoadIpList(acl, hdf["Ip.Allow"], true);
    } else {
      LoadIpList(acl, hdf["Ip.Allow"], true);
      LoadIpList(acl, hdf["Ip.Deny"], false);
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
  bool translated = false;
  unsigned int start = 0;
  unsigned int end = 0;

  for (StringToAclPtrMap::const_iterator iter = m_acls.begin();
       iter != m_acls.end(); ++iter) {
    const string &path = iter->first;
    if (command.size() >= path.size() &&
        strncmp(command.c_str(), path.c_str(), path.size()) == 0) {

      if (!translated) {
        ReadIPv4Address(ip.c_str(), start, end);
        ASSERT(start == end);
        translated = true;
      }

      IpMap &ips = iter->second->ips;
      IpMap::const_iterator iter2 = ips.find(start);
      if (iter2 != ips.end()) {
        return !iter2->second;
      }

      IpRangeVec &ranges = iter->second->ranges;
      for (unsigned int i = 0; i < ranges.size(); i++) {
        IpRange &range = ranges[i];
        if (start >= range.start && start <= range.end) {
          return !range.allow;
        }
      }
    }
  }
  return false;
}

///////////////////////////////////////////////////////////////////////////////
}
