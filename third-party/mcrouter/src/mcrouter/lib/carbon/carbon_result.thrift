/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

namespace cpp2 carbon

enum Result {
  UNKNOWN = 0,
  DELETED = 1,
  TOUCHED = 2,
  FOUND = 3,
  FOUNDSTALE = 4,
  NOTFOUND = 5,
  NOTFOUNDHOT = 6,
  NOTSTORED = 7,
  STALESTORED = 8,
  OK = 9,
  STORED = 10,
  EXISTS = 11,
  OOO = 12,
  TIMEOUT = 13,
  CONNECT_TIMEOUT = 14,
  CONNECT_ERROR = 15,
  BUSY = 16,
  RES_TRY_AGAIN = 17,
  SHUTDOWN = 18,
  TKO = 19,
  BAD_COMMAND = 20,
  BAD_KEY = 21,
  BAD_FLAGS = 22,
  BAD_EXPTIME = 23,
  BAD_LEASE_ID = 24,
  BAD_CAS_ID = 25,
  BAD_VALUE = 26,
  ABORTED = 27,
  CLIENT_ERROR = 28,
  LOCAL_ERROR = 29,
  REMOTE_ERROR = 30,
  WAITING = 31,
  DEADLINE_EXCEEDED = 32,
  PERMISSION_DENIED = 33,
  HOT_KEY = 34,
  // Result::NUM_RESULTS indicates the size of the results array and should
  // always be updated to the end of the list
  NUM_RESULTS = 35,
}
