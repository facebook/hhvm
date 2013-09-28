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

#ifndef incl_HPHP_IP_BLOCK_MAP_H_
#define incl_HPHP_IP_BLOCK_MAP_H_

#include "hphp/util/hdf.h"
#include "hphp/runtime/base/types.h"
#include <netinet/in.h>

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////
// Blacklisted IP address range support. Loads a set of networks from
// configuration, then is used to test candidate addresses to see if they
// fall into one of the forbidden networks for a particular request type.

DECLARE_BOOST_TYPES(IpBlockMap);
class IpBlockMap {
public:
  // Reads a textual IPv4 or IPv6 address, possibly including a bit count,
  // and turns it into an IPv6 address and a number of significant bits.
  // IPv4 addresses are turned into mapped IPv6 addresses.
  static bool ReadIPv6Address(const char *text,
                              struct in6_addr *output,
                              int &significant_bits);

public:
  explicit IpBlockMap(Hdf config);

  bool isBlocking(const std::string &command, const std::string &ip) const;

  /////////////////////////////////////////////////////////////////////////////
  // We put all the network addresses (which are simply strings of bits) in a
  // trie that we can match against a candidate network address. Each trie
  // node has a flag to indicate whether matching addresses are allowed or
  // disallowed. The value at the deepest trie node that matches a prefix of
  // the candidate address is the value for that address's network.
  class BinaryPrefixTrie {
  public:
    explicit BinaryPrefixTrie(bool allow);

    // Returns the "allow" value of the longest matching prefix of the
    // search value.
    bool isAllowed(const void *search,
                   const int search_bits = 128);

    void setAllowed(bool allow);

    // Inserts a new prefix into the trie with an allow value at the leaf node.
    // Nodes other than the new leaf will inherit the allow setting of the
    // longest existing prefix.
    static void InsertNewPrefix(BinaryPrefixTrie *root,
                                const void *value,
                                const int num_bits,
                                const bool allow);

  private:
    bool isAllowedImpl(const void *search,
                       const int search_bits,
                       const int bit_offset);

    BinaryPrefixTrie *m_children[2];
    bool m_allow;
  };

private:
  DECLARE_BOOST_TYPES(Acl);
  class Acl {
  public:
    Acl();

    BinaryPrefixTrie m_networks; // prefix => true: allow; false: deny
  };
  StringToAclPtrMap m_acls; // location => acl

  static void LoadIpList(AclPtr acl, Hdf hdf, bool allow);
};

///////////////////////////////////////////////////////////////////////////////
}

#endif // incl_HPHP_IP_BLOCK_MAP_H_
