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

#include "hphp/runtime/server/ip-block-map.h"
#include "hphp/util/logger.h"

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

IpBlockMap::Acl::Acl() : m_networks(true) {}

IpBlockMap::BinaryPrefixTrie::BinaryPrefixTrie(bool allow) {
  m_children[0] = m_children[1] = nullptr;
  setAllowed(allow);
}

void IpBlockMap::BinaryPrefixTrie::setAllowed(bool allow) {
  m_allow = allow;
}

bool IpBlockMap::BinaryPrefixTrie::isAllowed(
    const void *search,
    const int num_bits) {
  return isAllowedImpl(search, num_bits, 0);
}

bool IpBlockMap::BinaryPrefixTrie::isAllowedImpl(
    const void *search,
    const int num_bits,
    const int bit_offset) {
  const unsigned char *search_bytes = (const unsigned char *)search;
  BinaryPrefixTrie *child;

  if (bit_offset == num_bits) {
    if (m_children[0] != nullptr || m_children[1] != nullptr) {
      // This should never happen because the trie should only ever contain
      // prefixes of fixed-size network addresses, so the trie should never be
      // any deeper than the network address size.
      Logger::Error("trie depth exceeds search depth");
      return false;
    }
    return m_allow;
  }

  assert(bit_offset < num_bits);

  child = m_children[(*search_bytes >> (7 - bit_offset)) & 1];
  if (child) {
    if (bit_offset < 7) {
      return child->isAllowedImpl(search_bytes, num_bits, bit_offset + 1);
    } else {
      return child->isAllowedImpl(search_bytes + 1, num_bits - 8, 0);
    }
  }

  return m_allow;
}

void IpBlockMap::BinaryPrefixTrie::InsertNewPrefix(
    BinaryPrefixTrie *root,
    const void *value,
    const int num_bits,
    bool allow) {
  const unsigned char *bytes = (const unsigned char *)value;
  BinaryPrefixTrie *node = root;
  int curr_bit_num = 0;
  int curr_bit_val;
  bool next_allow = root->m_allow;

  while (curr_bit_num < num_bits) {
    // Peel off the bit at the current position
    curr_bit_val = (bytes[curr_bit_num / 8] >> (7 - (curr_bit_num & 7))) & 1;
    curr_bit_num++;
    if (!node->m_children[curr_bit_val]) {
      // When inserting the leaf node, stop inheriting the "allow" value
      // from ancestor nodes.
      if (curr_bit_num == num_bits) {
        next_allow = allow;
      }
      BinaryPrefixTrie *new_node = new BinaryPrefixTrie(next_allow);
      node->m_children[curr_bit_val] = new_node;
    }

    node = node->m_children[curr_bit_val];
    next_allow = node->m_allow;
  }
}

bool IpBlockMap::ReadIPv6Address(const char *text,
                                 struct in6_addr *output,
                                 int &significant_bits) {
#define STRING_IPV4_ADDR_MAX_LENGTH 15
#define STRING_IPV6_ADDR_MAX_LENGTH 39
  char address[STRING_IPV6_ADDR_MAX_LENGTH + 1];
  int address_len;
  const char *slash;
  bool is_ipv6 = (nullptr != strchr(text, ':'));

  // Find the bit count, if any.
  slash = strchr(text, '/');

  if (slash) {
    significant_bits = atoi(slash + 1);
    if (!significant_bits) {
      Logger::Error("invalid bit count: %s", text);
      return false;
    }
    address_len = slash - text;
  } else {
    significant_bits = is_ipv6 ? 128 : 32;
    address_len = strlen(text);
  }

  if (is_ipv6) {
    memcpy(address, text, address_len);
    address[address_len] = '\0';
  } else {
    // An IPv4 mapped address.
    if (address_len > STRING_IPV4_ADDR_MAX_LENGTH) {
      Logger::Error("invalid IPv4 address: %s", text);
      return false;
    }
    memcpy(address, "::ffff:", 7);
    memcpy(address + 7, text, address_len);
    address[address_len + 7] = '\0';
    significant_bits += 96;
  }

  if (inet_pton(AF_INET6, address, output) <= 0) {
    Logger::Error("invalid IPv6 address: %s", address);
    return false;
  }

  return true;
}

void IpBlockMap::LoadIpList(AclPtr acl, Hdf hdf, bool allow) {
  for (Hdf child = hdf.firstChild(); child.exists(); child = child.next()) {
    string ip = child.getString();

    int bits;
    struct in6_addr address;
    if (ReadIPv6Address(ip.c_str(), &address, bits)) {
      BinaryPrefixTrie::InsertNewPrefix(&acl->m_networks,
                                        &address,
                                        bits,
                                        allow);
    }
  }
}

IpBlockMap::IpBlockMap(Hdf config) {
  for (Hdf hdf = config.firstChild(); hdf.exists(); hdf = hdf.next()) {
    AclPtr acl(new Acl());
    // sgrimm note: not sure AllowFirst is relevant with my implementation
    // since we always search for the narrowest matching rule -- it really
    // just sets whether we deny or allow by default, I think.
    bool allow = hdf["AllowFirst"].getBool(false);
    if (allow) {
      acl->m_networks.setAllowed(true);
      LoadIpList(acl, hdf["Ip.Deny"], false);
      LoadIpList(acl, hdf["Ip.Allow"], true);
    } else {
      acl->m_networks.setAllowed(false);
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
  struct in6_addr address;
  int bits;

  for (StringToAclPtrMap::const_iterator iter = m_acls.begin();
       iter != m_acls.end(); ++iter) {
    const string &path = iter->first;
    if (command.size() >= path.size() &&
        strncmp(command.c_str(), path.c_str(), path.size()) == 0) {

      if (!translated) {
        ReadIPv6Address(ip.c_str(), &address, bits);
        assert(bits == 128);
        translated = true;
      }

      return !iter->second->m_networks.isAllowed(&address);
    }
  }
  return false;
}

///////////////////////////////////////////////////////////////////////////////
}
