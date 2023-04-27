/*
 *  Copyright (c) 2018-present, Facebook, Inc.
 *  All rights reserved.
 *
 *  This source code is licensed under the BSD-style license found in the
 *  LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <fizz/server/ServerExtensions.h>

namespace fizz {
namespace server {

/**
 * This class defines an interface which allows for multiple ServerExtensions
 * to be used at the same time..
 */
class MultiServerExtensions : public ServerExtensions {
 public:
  explicit MultiServerExtensions(
      std::vector<std::shared_ptr<ServerExtensions>> extensions);

  /**
   * For each extension in the provided list, get the associated Extensions
   * and combine into one vector.
   */
  std::vector<Extension> getExtensions(const ClientHello& chlo) override;

 private:
  std::vector<std::shared_ptr<ServerExtensions>> extensions_;
};

} // namespace server
} // namespace fizz
