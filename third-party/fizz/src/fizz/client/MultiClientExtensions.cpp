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
std::vector<Extension> MultiClientExtensions::getClientHelloExtensions() const {
  std::vector<Extension> result;
  for (const auto& ext : extensions_) {
    auto next = ext->getClientHelloExtensions();
    result.insert(
        result.end(),
        std::make_move_iterator(next.begin()),
        std::make_move_iterator(next.end()));
  }
  return result;
}

void MultiClientExtensions::onEncryptedExtensions(
    const std::vector<Extension>& extensions) {
  for (auto& ext : extensions_) {
    ext->onEncryptedExtensions(extensions);
  }
}

} // namespace client
} // namespace fizz
