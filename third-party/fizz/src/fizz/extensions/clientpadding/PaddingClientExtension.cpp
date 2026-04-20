/*
 *  Copyright (c) Meta Platforms, Inc. and affiliates.
 *  All rights reserved.
 *
 *  This source code is licensed under the BSD-style license found in the
 *  LICENSE file in the root directory of this source tree.
 */

#include <fizz/extensions/clientpadding/PaddingClientExtension.h>
#include <fizz/extensions/clientpadding/Types.h>

namespace fizz {
namespace extensions {

PaddingClientExtension::PaddingClientExtension(uint16_t paddingTotalBytes)
    : paddingTotalBytes_(paddingTotalBytes) {}

Status PaddingClientExtension::getClientHelloExtensions(
    std::vector<Extension>& ret,
    Error& err) const {
  ret.clear();
  extensions::Padding padding{paddingTotalBytes_};

  Extension ext;
  FIZZ_RETURN_ON_ERROR(encodeExtension(ext, err, padding));
  ret.push_back(std::move(ext));
  return Status::Success;
}

} // namespace extensions
} // namespace fizz
