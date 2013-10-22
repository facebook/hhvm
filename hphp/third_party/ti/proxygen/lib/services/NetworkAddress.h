// Copyright 2004-present Facebook.  All rights reserved.
#pragma once

#include "thrift/lib/cpp/transport/TSocketAddress.h"

namespace facebook { namespace proxygen {

/**
 * A simple wrapper around TSocketAddress that represents
 * a network in CIDR notation
 */
class NetworkAddress {
public:
  /**
   * Create a NetworkAddress for an addr/prefixLen
   * @param addr         IPv4 or IPv6 address of the network
   * @param prefixLen    Prefix length, in bits
   */
  NetworkAddress(const apache::thrift::transport::TSocketAddress& addr,
      unsigned prefixLen):
    addr_(addr), prefixLen_(prefixLen) {}

  /** Get the network address */
  const apache::thrift::transport::TSocketAddress& getAddress() const {
    return addr_;
  }

  /** Get the prefix length in bits */
  unsigned getPrefixLength() const { return prefixLen_; }

  /** Check whether a given address lies within the network */
  bool contains(const apache::thrift::transport::TSocketAddress& addr) const {
    return addr_.prefixMatch(addr, prefixLen_);
  }

  /** Comparison operator to enable use in ordered collections */
  bool operator<(const NetworkAddress& other) const {
    if (addr_ <  other.addr_) {
      return true;
    } else if (other.addr_ < addr_) {
      return false;
    } else {
      return (prefixLen_ < other.prefixLen_);
    }
  }

private:
  apache::thrift::transport::TSocketAddress addr_;
  unsigned prefixLen_;
};

}} // facebook::proxygen
