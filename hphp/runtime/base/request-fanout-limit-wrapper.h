// (c) Meta Platforms, Inc. and affiliates. Confidential and proprietary.

#pragma once

#include "hphp/runtime/base/request-fanout-limit.h"


namespace HPHP {

void requestFanoutLimitInit(int fanoutLimit, int rootReqThreadCount);

void requestFanoutLimitIncrement(RequestId rootReqId);
void requestFanoutLimitDecrement(RequestId rootReqId);

void requestFanoutLimitSetScriptFilename(RequestId rootReqId, std::string scriptFilename);

} // namespace HPHP
