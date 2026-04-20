/*
 *  Copyright (c) 2018-present, Facebook, Inc.
 *  All rights reserved.
 *
 *  This source code is licensed under the BSD-style license found in the
 *  LICENSE file in the root directory of this source tree.
 */

#include <fizz/client/MultiClientExtensions.h>

namespace fizz {
namespace client {

MultiClientExtensions::MultiClientExtensions(
    std::vector<std::shared_ptr<ClientExtensions>> extensions)
    : extensions_(std::move(extensions)) {}

/**
 * For each extension in the provided list, get the associated Extensions
 * and combine into one vector.
 */
Status MultiClientExtensions::getClientHelloExtensions(
    std::vector<Extension>& ret,
    Error& err) const {
  ret.clear();
  for (const auto& ext : extensions_) {
    std::vector<Extension> tmp;
    FIZZ_RETURN_ON_ERROR(ext->getClientHelloExtensions(tmp, err));
    for (auto& e : tmp) {
      ret.push_back(std::move(e));
    }
  }
  return Status::Success;
}

Status MultiClientExtensions::onEncryptedExtensions(
    Error& err,
    const std::vector<Extension>& extensions) {
  for (auto& ext : extensions_) {
    FIZZ_RETURN_ON_ERROR(ext->onEncryptedExtensions(err, extensions));
  }
  return Status::Success;
}

} // namespace client
} // namespace fizz
