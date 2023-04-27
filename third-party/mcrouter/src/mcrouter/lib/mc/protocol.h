/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <string.h>

#define MC_KEY_MAX_LEN_ASCII (250)
#define MC_KEY_MAX_LEN_CARET (2 * 1024)
#define MC_KEY_MAX_LEN (MC_KEY_MAX_LEN_CARET)

typedef enum mc_protocol_e {
  mc_unknown_protocol = 0,
  mc_ascii_protocol = 1,
  mc_binary_protocol = 2,
  mc_caret_protocol = 4,
  mc_thrift_protocol = 5,
  mc_nprotocols, // placeholder
} mc_protocol_t;

static inline mc_protocol_t mc_string_to_protocol(const char* str) {
  if (!strcmp(str, "ascii")) {
    return mc_ascii_protocol;
  } else if (!strcmp(str, "binary")) {
    return mc_binary_protocol;
  } else if (!strcmp(str, "caret")) {
    return mc_caret_protocol;
  } else if (!strcmp(str, "thrift")) {
    return mc_thrift_protocol;
  } else {
    return mc_unknown_protocol;
  }
}

static inline const char* mc_protocol_to_string(const mc_protocol_t value) {
  switch (value) {
    case mc_ascii_protocol:
      return "ascii";
    case mc_binary_protocol:
      return "binary";
    case mc_caret_protocol:
      return "caret";
    case mc_thrift_protocol:
      return "thrift";
    case mc_unknown_protocol:
    default:
      return "unknown-protocol";
  }
}
