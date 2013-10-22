// Copyright 2004-present Facebook.  All rights reserved.
#pragma once
#include <stdint.h>

namespace facebook { namespace proxygen {

enum class TransportDirection : uint8_t {
  DOWNSTREAM,  // toward the client
  UPSTREAM     // toward the origin application or data
};

}} // facebook::proxygen

