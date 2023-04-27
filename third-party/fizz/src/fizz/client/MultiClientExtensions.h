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
namespace client {

/**
 * This class defines an interface which allows for multiple ClientExtensions
 * to be used at the same time..
 */
class MultiClientExtensions : public ClientExtensions {
 public:
  explicit MultiClientExtensions(
      std::vector<std::shared_ptr<ClientExtensions>> extensions);

  /**
   * For each extension in the provided list, get the associated Extensions
   * and combine into one vector.
   */
  std::vector<Extension> getClientHelloExtensions() const override;

  /**
   * Call each extension's implementation of onEncryptedExtensions.
   */
  void onEncryptedExtensions(const std::vector<Extension>& extensions) override;

 private:
  std::vector<std::shared_ptr<ClientExtensions>> extensions_;
};

} // namespace client
} // namespace fizz
