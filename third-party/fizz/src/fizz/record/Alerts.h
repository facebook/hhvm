/*
 *  Copyright (c) 2018-present, Facebook, Inc.
 *  All rights reserved.
 *
 *  This source code is licensed under the BSD-style license found in the
 *  LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <cstdint>

namespace fizz {
enum class AlertDescription : uint8_t {
  close_notify = 0,
  end_of_early_data = 1,
  unexpected_message = 10,
  bad_record_mac = 20,
  record_overflow = 22,
  handshake_failure = 40,
  bad_certificate = 42,
  unsupported_certificate = 43,
  certificate_revoked = 44,
  certificate_expired = 45,
  certificate_unknown = 46,
  illegal_parameter = 47,
  unknown_ca = 48,
  access_denied = 49,
  decode_error = 50,
  decrypt_error = 51,
  protocol_version = 70,
  insufficient_security = 71,
  internal_error = 80,
  inappropriate_fallback = 86,
  user_canceled = 90,
  missing_extension = 109,
  unsupported_extension = 110,
  certificate_unobtainable = 111,
  unrecognized_name = 112,
  bad_certificate_status_response = 113,
  bad_certificate_hash_value = 114,
  unknown_psk_identity = 115,
  certificate_required = 116,
  no_application_protocol = 120,
  ech_required = 121
};
} // namespace fizz
