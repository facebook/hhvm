// Copyright 2004-present Facebook.  All rights reserved.
#pragma once
#include "thrift/lib/cpp/async/TAsyncSocket.h"

namespace facebook { namespace proxygen {

// this is the name of OPTNAME that starts TCP events tracking
const int TCP_TRACKING_OPTNAME = 21;

/**
 * Returns a copy of the socket options excluding options with the given
 * level.
 */
apache::thrift::async::TAsyncSocket::OptionMap filterIPSocketOptions(
  const apache::thrift::async::TAsyncSocket::OptionMap& allOptions,
  const int addrFamily);

}}
