/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <cstdint>
#include <proxygen/lib/http/codec/ErrorCode.h>
#include <string>

namespace proxygen { namespace http2 {

extern const uint32_t kFrameHeaderSize;
extern const uint32_t kFrameHeadersBaseMaxSize;
extern const uint32_t kFramePrioritySize;
extern const uint32_t kFrameStreamIDSize;
extern const uint32_t kFrameRstStreamSize;
extern const uint32_t kFramePushPromiseSize;
extern const uint32_t kFramePingSize;
extern const uint32_t kFrameGoawaySize;
extern const uint32_t kFrameWindowUpdateSize;
extern const uint32_t kFrameCertificateRequestSizeBase;
extern const uint32_t kFrameCertificateSizeBase;

// These constants indicate the size of the required fields in the frame
extern const uint32_t kFrameAltSvcSizeBase;

extern const uint32_t kMaxFramePayloadLengthMin;
extern const uint32_t kMaxFramePayloadLength;
extern const uint32_t kMaxStreamID;
extern const uint32_t kInitialWindow;
extern const uint32_t kMaxWindowUpdateSize;
extern const uint32_t kMaxHeaderTableSize;

// The maximum size of the data buffer caching an authenticator.
// For secondary authentication in HTTP/2.
extern const uint32_t kMaxAuthenticatorBufSize;

extern const uint32_t kMaxHeaderTableSize;
extern const std::string kConnectionPreface;

extern const std::string kProtocolString;
extern const std::string kProtocolDraftString;
extern const std::string kProtocolExperimentalString;
extern const std::string kProtocolCleartextString;
extern const std::string kProtocolSettingsHeader;

}} // namespace proxygen::http2
