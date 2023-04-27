/*
 *  Copyright (c) 2018-present, Facebook, Inc.
 *  All rights reserved.
 *
 *  This source code is licensed under the BSD-style license found in the
 *  LICENSE file in the root directory of this source tree.
 */

#include <fizz/server/MultiServerExtensions.h>

using fizz::server::MultiServerExtensions;

MultiServerExtensions::MultiServerExtensions(
    std::vector<std::shared_ptr<ServerExtensions>> extensions)
    : extensions_(std::move(extensions)) {}

/**
 * For each extension in the provided list, get the associated Extensions
 * and combine into one vector.
 */
std::vector<fizz::Extension> MultiServerExtensions::getExtensions(
    const ClientHello& chlo) {
  std::vector<fizz::Extension> result;
  for (auto& ext : extensions_) {
    auto next = ext->getExtensions(chlo);
    result.insert(
        result.end(),
        std::make_move_iterator(next.begin()),
        std::make_move_iterator(next.end()));
  }
  return result;
}
