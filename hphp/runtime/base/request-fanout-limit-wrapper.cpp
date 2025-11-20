// (c) Meta Platforms, Inc. and affiliates. Confidential and proprietary.

#include "hphp/runtime/base/request-fanout-limit-wrapper.h"


namespace HPHP {

  static RequestFanoutLimit* s_xboxRequestFanoutLimit;
  // number of hhvmworker threads are used to initialize the 
  // request fanout map, but that thread count can be tweaked significantly
  // at runtime as part of auto-tuning. While concurrentHashMap
  // can automatically resize, to max the performance it is helpful
  // to size the map as accurate as possible upon initialization.
  // Thus adding a x2 buffer multiplier here. 
  const int BUFFER_SIZE_MULTIPLIER = 2;

  void requestFanoutLimitInit(int fanoutLimit, int rootReqThreadCount) {
    if (s_xboxRequestFanoutLimit) return;

    s_xboxRequestFanoutLimit = new RequestFanoutLimit(
      fanoutLimit, 
      rootReqThreadCount * BUFFER_SIZE_MULTIPLIER
    );
  }

  void requestFanoutLimitIncrement(RequestId rootReqId) {
    if (s_xboxRequestFanoutLimit) {
      s_xboxRequestFanoutLimit->increment(rootReqId);
    }
  }

  void requestFanoutLimitDecrement(RequestId rootReqId) {
    if (s_xboxRequestFanoutLimit) {
      s_xboxRequestFanoutLimit->decrement(rootReqId);
    }
  }

  void requestFanoutLimitSetScriptFilename(RequestId rootReqId, std::string scriptFilename) {
    if (s_xboxRequestFanoutLimit) {
      s_xboxRequestFanoutLimit->setScriptFilename(rootReqId, scriptFilename);
    }
  }

} // namespace HPHP
