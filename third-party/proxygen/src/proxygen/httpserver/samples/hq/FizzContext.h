/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <fizz/client/FizzClientContext.h>
#include <fizz/server/FizzServerContext.h>
#include <proxygen/httpserver/samples/hq/HQParams.h>
#include <vector>

#include <wangle/ssl/SSLContextConfig.h>

namespace quic::samples {

using FizzServerContextPtr =
    std::shared_ptr<const fizz::server::FizzServerContext>;

using FizzClientContextPtr = std::shared_ptr<fizz::client::FizzClientContext>;

FizzServerContextPtr createFizzServerContextInsecure(
    const HQServerParams& params,
    const std::vector<std::string>& supportedAlpns,
    fizz::server::ClientAuthMode clientAuth,
    const std::string& certificateFilePath,
    const std::string& keyFilePath);

FizzClientContextPtr createFizzClientContext(
    const HQBaseParams& params,
    const std::vector<std::string>& supportedAlpns,
    bool earlyData,
    const std::string& certificateFilePath,
    const std::string& keyFilePath);

wangle::SSLContextConfig createSSLContext(
    const HQBaseParams& params,
    const std::string& certificateFilePath,
    const std::string& keyFilePath);
} // namespace quic::samples
