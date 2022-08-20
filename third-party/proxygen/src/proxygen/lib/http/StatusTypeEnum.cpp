/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <proxygen/lib/http/StatusTypeEnum.h>

#define STATUS_TYPE_STR(statusType) #statusType

namespace {
static const char* statusTypeStrings[] = {STATUS_TYPES_GEN(STATUS_TYPE_STR)};
}

namespace proxygen {

const char* getStatusTypeString(StatusType statusType) {
  int statusTypeInt = static_cast<int>(statusType);
  if (static_cast<int>(statusType) < 0 ||
      statusType >= StatusType::ENUM_COUNT) {
    return statusTypeStrings[static_cast<int>(StatusType::ENUM_COUNT)];
  } else {
    return statusTypeStrings[statusTypeInt];
  }
}

} // namespace proxygen
