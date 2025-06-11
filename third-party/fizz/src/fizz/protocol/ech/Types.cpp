/*
 *  Copyright (c) Meta Platforms, Inc. and affiliates.
 *  All rights reserved.
 *
 *  This source code is licensed under the BSD-style license found in the
 *  LICENSE file in the root directory of this source tree.
 */

#include "fizz/protocol/ech/Types.h"

namespace fizz::ech {
folly::Optional<ECHConfigContentDraft>
ECHConfigContentDraft::parseSupportedECHConfig(const ECHConfig& config) {
  if (config.version == ECHVersion::Draft15) {
    folly::io::Cursor cursor(config.ech_config_content.get());
    return decode<ECHConfigContentDraft>(cursor);
  }
  return folly::none;
}
} // namespace fizz::ech
