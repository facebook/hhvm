/*
 *  Copyright (c) 2018-present, Facebook, Inc.
 *  All rights reserved.
 *
 *  This source code is licensed under the BSD-style license found in the
 *  LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <fizz/client/ClientExtensions.h>

namespace fizz {
namespace extensions {

/**
 * Implement RFC7685 padding extension as client extension
 */
class PaddingClientExtension : public ClientExtensions {
 public:
  explicit PaddingClientExtension(uint16_t paddingTotalBytes);

  std::vector<Extension> getClientHelloExtensions() const override;

  void onEncryptedExtensions(
      const std::vector<Extension>& /*extensions*/) override {}

 private:
  uint16_t paddingTotalBytes_;
};

} // namespace extensions
} // namespace fizz
