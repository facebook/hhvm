/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include "mcrouter/lib/carbon/Result.h"

namespace facebook {
namespace memcache {

inline int resultSeverity(carbon::Result result) {
  switch (result) {
    case carbon::Result::OK:
    case carbon::Result::STORED:
    case carbon::Result::STALESTORED:
    case carbon::Result::EXISTS:
    case carbon::Result::DELETED:
    case carbon::Result::FOUND:
      return 1;

    case carbon::Result::WAITING:
      return 2;

    case carbon::Result::NOTFOUND:
    case carbon::Result::NOTSTORED:
      return 4;

    case carbon::Result::OOO:
    case carbon::Result::TIMEOUT:
    case carbon::Result::CONNECT_TIMEOUT:
    case carbon::Result::CONNECT_ERROR:
    case carbon::Result::BUSY:
    case carbon::Result::SHUTDOWN:
    case carbon::Result::RES_TRY_AGAIN:
    case carbon::Result::TKO:
      return 5;

    case carbon::Result::BAD_KEY:
    case carbon::Result::BAD_VALUE:
    case carbon::Result::ABORTED:
    case carbon::Result::DEADLINE_EXCEEDED:
      return 6;

    case carbon::Result::REMOTE_ERROR:
    case carbon::Result::UNKNOWN:
      return 7;

    case carbon::Result::LOCAL_ERROR:
      return 8;

    case carbon::Result::CLIENT_ERROR:
      return 9;

    default:
      return 10;
  }
}

/**
 * carbon::Result convenience functions, useful for replies
 */
/**
 * Is this reply an error?
 */
inline bool isErrorResult(const carbon::Result result) {
  return (result == carbon::Result::DEADLINE_EXCEEDED) ||
      (result >= carbon::Result::OOO && result < carbon::Result::WAITING);
}

/**
 * Is this reply an error as far as failover logic is concerned?
 */
inline bool isFailoverErrorResult(const carbon::Result result) {
  switch (result) {
    case carbon::Result::BUSY:
    case carbon::Result::SHUTDOWN:
    case carbon::Result::TKO:
    case carbon::Result::RES_TRY_AGAIN:
    case carbon::Result::LOCAL_ERROR:
    case carbon::Result::CONNECT_ERROR:
    case carbon::Result::CONNECT_TIMEOUT:
    case carbon::Result::TIMEOUT:
    case carbon::Result::REMOTE_ERROR:
    // Consider DEADLINE_EXCEEDED as failover result because
    // exceeding deadline on one destination might not mean it is exceeded
    // deadline everywhere because that one destination
    // may be having time-sync issues and is not properly sync'ed to time
    // server
    case carbon::Result::DEADLINE_EXCEEDED:
      return true;
    default:
      return false;
  }
}

/**
 * Is this reply a soft TKO error?
 */
inline bool isSoftTkoErrorResult(const carbon::Result result) {
  switch (result) {
    case carbon::Result::TIMEOUT:
      return true;
    default:
      return false;
  }
}

/**
 * Is this reply a hard TKO error?
 */
inline bool isHardTkoErrorResult(const carbon::Result result) {
  switch (result) {
    case carbon::Result::CONNECT_ERROR:
    case carbon::Result::CONNECT_TIMEOUT:
    case carbon::Result::SHUTDOWN:
      return true;
    default:
      return false;
  }
}

/**
 * Did we not even attempt to send request out because at some point
 * we decided the destination is in TKO state?
 *
 * Used to short-circuit failover decisions in certain RouteHandles.
 *
 * If isTkoResult() is true, isErrorResult() must also be true.
 */
inline bool isTkoResult(const carbon::Result result) {
  return result == carbon::Result::TKO;
}

/**
 * Checks whether a reply is either a TKO or hard TKO error.
 */
inline bool isTkoOrHardTkoResult(const carbon::Result result) {
  return isTkoResult(result) || isHardTkoErrorResult(result);
}

/**
 * Did we not even attempt to send request out because it is invalid/we hit
 * per-destination rate limit
 */
inline bool isLocalErrorResult(const carbon::Result result) {
  return result == carbon::Result::LOCAL_ERROR;
}

/**
 * Was there some client-side issue? Invalid request, bad client, etc?
 */
inline bool isClientErrorResult(const carbon::Result result) {
  return result == carbon::Result::CLIENT_ERROR;
}

/**
 * Was there some problem with the remote server?
 */
inline bool isRemoteErrorResult(const carbon::Result result) {
  return result == carbon::Result::REMOTE_ERROR;
}

/**
 * Was the connection attempt refused?
 */
inline bool isConnectErrorResult(const carbon::Result result) {
  return result == carbon::Result::CONNECT_ERROR;
}

/**
 * Was there a timeout while attempting to establish a connection?
 */
inline bool isConnectTimeoutResult(const carbon::Result result) {
  return result == carbon::Result::CONNECT_TIMEOUT;
}

/**
 * Was there a timeout when sending data on an established connection?
 * Note: the distinction is important, since in this case we don't know
 * if the data reached the server or not.
 */
inline bool isDataTimeoutResult(const carbon::Result result) {
  return result == carbon::Result::TIMEOUT;
}

/**
 * Application-specific redirect code. Server is up, but doesn't want
 * to reply now.
 */
inline bool isRedirectResult(const carbon::Result result) {
  return result == carbon::Result::BUSY ||
      result == carbon::Result::RES_TRY_AGAIN;
}

/**
 * Was the data found?
 */
inline bool isHitResult(const carbon::Result result) {
  return result == carbon::Result::DELETED || result == carbon::Result::FOUND ||
      result == carbon::Result::TOUCHED;
}

/**
 * Was data not found and no errors occured?
 */
inline bool isMissResult(const carbon::Result result) {
  return result == carbon::Result::NOTFOUND;
}

/**
 * Lease hot miss?
 */
inline bool isHotMissResult(const carbon::Result result) {
  return result == carbon::Result::FOUNDSTALE ||
      result == carbon::Result::NOTFOUNDHOT;
}

/**
 * Was the data stored?
 */
inline bool isStoredResult(const carbon::Result result) {
  return result == carbon::Result::STORED ||
      result == carbon::Result::STALESTORED;
}

/**
 * Was the request deadline exceeded?
 */
inline bool isDeadlineExceededResult(const carbon::Result result) {
  return result == carbon::Result::DEADLINE_EXCEEDED;
}

inline bool worseThan(carbon::Result first, carbon::Result second) {
  return resultSeverity(first) > resultSeverity(second);
}
} // namespace memcache
} // namespace facebook
