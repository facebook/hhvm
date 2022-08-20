/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <folly/Memory.h>
#include <folly/dynamic.h>
#include <memory>
#include <proxygen/lib/http/HTTPMessage.h>
#include <proxygen/lib/http/codec/compress/HPACKHeader.h>
#include <string>
#include <vector>

namespace proxygen {

class HTTPArchive {
 public:
  std::vector<HTTPMessage> requests;
  std::vector<HTTPMessage> responses;

  static std::vector<std::vector<HPACKHeader>> convertToHPACK(
      const std::vector<HTTPMessage>& msgs);

  static std::unique_ptr<HTTPArchive> fromFile(const std::string& filename);

  static std::unique_ptr<HTTPArchive> fromPublicFile(const std::string& fname);

  static uint32_t getSize(const HTTPMessage& msg);
  static uint32_t getSize(const std::vector<HPACKHeader>& headers);
};
} // namespace proxygen
