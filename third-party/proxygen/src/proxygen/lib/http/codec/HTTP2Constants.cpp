/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <proxygen/lib/http/codec/HTTP2Constants.h>

namespace proxygen { namespace http2 {

const uint32_t kFrameHeaderSize = 9;

const uint32_t kFrameHeadersBaseMaxSize = kFramePrioritySize + 1;
const uint32_t kFramePrioritySize = 5;
const uint32_t kFrameStreamIDSize = 4;
const uint32_t kFrameRstStreamSize = 4;
const uint32_t kFramePushPromiseSize = 4;
const uint32_t kFramePingSize = 8;
const uint32_t kFrameGoawaySize = 8;
const uint32_t kFrameWindowUpdateSize = 4;
const uint32_t kFrameCertificateRequestSizeBase = 2;
const uint32_t kFrameCertificateSizeBase = 2;

const uint32_t kFrameAltSvcSizeBase = 8;

const uint32_t kMaxFramePayloadLengthMin = (1u << 14);
const uint32_t kMaxFramePayloadLength = (1u << 24) - 1;
const uint32_t kMaxStreamID = (1u << 31) - 1;
const uint32_t kInitialWindow = (1u << 16) - 1;
const uint32_t kMaxWindowUpdateSize = (1u << 31) - 1;
const uint32_t kMaxHeaderTableSize = (1u << 16);

const uint32_t kMaxAuthenticatorBufSize = 0x20000; // 128k

const std::string kConnectionPreface("PRI * HTTP/2.0\r\n\r\nSM\r\n\r\n");

const std::string kProtocolString("h2");
const std::string kProtocolDraftString("h2-14");
const std::string kProtocolExperimentalString("h2-fb");
const std::string kProtocolCleartextString("h2c");
const std::string kProtocolSettingsHeader("HTTP2-Settings");

}} // namespace proxygen::http2
