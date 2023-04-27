/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include "mcrouter/lib/carbon/gen-cpp2/carbon_result_types.h"

namespace carbon {

#define CARBON_RESULT_ENUM_CLASS

inline const char* resultToString(const Result result) {
  switch (result) {
    case Result::UNKNOWN:
      return "mc_res_unknown";
    case Result::DELETED:
      return "mc_res_deleted";
    case Result::TOUCHED:
      return "mc_res_touched";
    case Result::FOUND:
      return "mc_res_found";
    case Result::FOUNDSTALE:
      return "mc_res_foundstale";
    case Result::NOTFOUND:
      return "mc_res_notfound";
    case Result::NOTFOUNDHOT:
      return "mc_res_notfoundhot";
    case Result::NOTSTORED:
      return "mc_res_notstored";
    case Result::STALESTORED:
      return "mc_res_stalestored";
    case Result::OK:
      return "mc_res_ok";
    case Result::STORED:
      return "mc_res_stored";
    case Result::EXISTS:
      return "mc_res_exists";
    case Result::OOO:
      return "mc_res_ooo";
    case Result::TIMEOUT:
      return "mc_res_timeout";
    case Result::CONNECT_TIMEOUT:
      return "mc_res_connect_timeout";
    case Result::CONNECT_ERROR:
      return "mc_res_connect_error";
    case Result::BUSY:
      return "mc_res_busy";
    case Result::RES_TRY_AGAIN:
      return "mc_res_try_again";
    case Result::SHUTDOWN:
      return "mc_res_shutdown";
    case Result::TKO:
      return "mc_res_tko";
    case Result::BAD_COMMAND:
      return "mc_res_bad_command";
    case Result::BAD_KEY:
      return "mc_res_bad_key";
    case Result::BAD_FLAGS:
      return "mc_res_bad_flags";
    case Result::BAD_EXPTIME:
      return "mc_res_bad_exptime";
    case Result::BAD_LEASE_ID:
      return "mc_res_bad_lease_id";
    case Result::BAD_CAS_ID:
      return "mc_res_bad_cas_id";
    case Result::BAD_VALUE:
      return "mc_res_bad_value";
    case Result::ABORTED:
      return "mc_res_aborted";
    case Result::CLIENT_ERROR:
      return "mc_res_client_error";
    case Result::LOCAL_ERROR:
      return "mc_res_local_error";
    case Result::REMOTE_ERROR:
      return "mc_res_remote_error";
    case Result::WAITING:
      return "mc_res_waiting";
    case Result::DEADLINE_EXCEEDED:
      return "mc_res_deadline_exceeded";
    case Result::PERMISSION_DENIED:
      return "mc_res_permission_denied";
    case Result::HOT_KEY:
      return "mc_res_hot_key";
    case Result::NUM_RESULTS:
      return "mc_res_unknown";
  }
  return "mc_res_unknown";
}

Result resultFromString(const char* result);

} // namespace carbon
