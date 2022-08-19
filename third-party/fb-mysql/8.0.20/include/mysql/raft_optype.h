// Copyright 2004-present Facebook. All Rights Reserved.

#pragma once

enum class RaftReplicateMsgOpType {
  OP_TYPE_INVALID = 0,
  OP_TYPE_TRX = 1,
  OP_TYPE_ROTATE = 2,
  OP_TYPE_NOOP = 3,
  OP_TYPE_CHANGE_CONFIG = 4,
};

// WARNING - do not add any other dependencies to this header
// file as it is also included in IoCacheUtils
