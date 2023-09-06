/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

namespace proxygen {
struct FlowControlInfo {
  bool sessionWritesPaused_{false};
  bool sessionReadsPaused_{false};
  bool flowControlEnabled_{false};
  int64_t sessionSendWindow_{-1};
  int64_t sessionRecvWindow_{-1};
  int64_t sessionSendOutstanding_{-1};
  int64_t sessionRecvOutstanding_{-1};
  int64_t streamSendWindow_{-1};
  int64_t streamRecvWindow_{-1};
  int64_t streamSendOutstanding_{-1};
  int64_t streamRecvOutstanding_{-1};
};

} // namespace proxygen
