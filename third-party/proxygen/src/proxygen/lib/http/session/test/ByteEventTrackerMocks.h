/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <folly/portability/GMock.h>
#include <proxygen/lib/http/session/ByteEventTracker.h>

class MockByteEventTrackerCallback
    : public proxygen::ByteEventTracker::Callback {
 public:
  MOCK_METHOD((void), onPingReplyLatency, (int64_t), (noexcept));
  MOCK_METHOD((void),
              onTxnByteEventWrittenToBuf,
              (const proxygen::ByteEvent&),
              (noexcept));
  MOCK_METHOD((void), onDeleteTxnByteEvent, (), (noexcept));
};
