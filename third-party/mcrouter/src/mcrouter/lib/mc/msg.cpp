/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "msg.h"

#include <string>
#include <unordered_map>

#include "mcrouter/lib/mc/protocol.h"

mc_op_t mc_op_from_string(const char* str) {
  int i = 0;
  for (i = mc_op_unknown; i < mc_nops; ++i) {
    if (0 == strcmp(mc_op_to_string((mc_op_t)i), str)) {
      return (mc_op_t)i;
    }
  }
  return mc_op_unknown;
}

const char* mc_req_err_to_string(const mc_req_err_t err) {
  switch (err) {
    case mc_req_err_valid:
      return "valid";
    case mc_req_err_no_key:
      return "invalid key: missing";
    case mc_req_err_key_too_long:
      return "invalid key: too long";
    case mc_req_err_space_or_ctrl:
      return "invalid key: space or control character";
  }
  return "unknown";
}

static std::unordered_map<std::string, mc_res_t> makeStringToMcRes() {
  std::unordered_map<std::string, mc_res_t> resMap;
  for (mc_res_t i = mc_res_unknown; i < mc_nres; i = mc_res_t(i + 1)) {
    resMap[mc_res_to_string(i)] = i;
  }
  return resMap;
}

const static std::unordered_map<std::string, mc_res_t> kStringToMcRes =
    makeStringToMcRes();

mc_res_t mc_res_from_string(const char* result) {
  auto it = kStringToMcRes.find(result);
  if (it != kStringToMcRes.cend()) {
    return it->second;
  }
  return mc_res_unknown;
}
