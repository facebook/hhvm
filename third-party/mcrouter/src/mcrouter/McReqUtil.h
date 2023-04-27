/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include "mcrouter/config.h"
#include "mcrouter/lib/Operation.h"

// Request deadline related helper functions
template <typename T, typename = std::void_t<>>
constexpr auto hasConstDeadlineMs = false;
template <typename T>
constexpr auto hasConstDeadlineMs<
    T,
    std::void_t<decltype(std::declval<const T&>().deadlineMs_ref())>> = true;

template <typename T, typename = std::void_t<>>
constexpr auto hasNonConstDeadlineMs = false;
template <typename T>
constexpr auto hasNonConstDeadlineMs<
    T,
    std::void_t<decltype(std::declval<std::decay_t<T>&>().deadlineMs_ref())>> =
    true;

template <typename T, typename = std::void_t<>>
constexpr auto hasConstFailoverHopCount = false;
template <typename T>
constexpr auto hasConstFailoverHopCount<
    T,
    std::void_t<decltype(std::declval<const T&>().failoverHopCount_ref())>> =
    true;

template <typename T, typename = std::void_t<>>
constexpr auto hasNonConstFailoverHopCount = false;
template <typename T>
constexpr auto hasNonConstFailoverHopCount<
    T,
    std::void_t<
        decltype(std::declval<std::decay_t<T>&>().failoverHopCount_ref())>> =
    true;

/**
 * incFailoverHopCount - If the Request has failoverHopCount field in it,
 *                       increment it by count
 */
template <class Request>
void incFailoverHopCount(Request& req, uint32_t count = 1) {
  if constexpr (hasNonConstFailoverHopCount<Request>) {
    req.failoverHopCount_ref() = *req.failoverHopCount_ref() + count;
  }
}

/**
 * getFailoverHopCount - If the Request has failoverHopCount field in it,
 *                       return the value, otherwise, returns 0.
 */
template <class T>
uint32_t getFailoverHopCount(T& req) {
  if constexpr (hasConstFailoverHopCount<T>) {
    return *req.failoverHopCount_ref();
  }
  return 0;
}

/**
 * setFailoverHopCount - If the Reply has failoverHopCount field in it,
 *                       set it's value to failoverHopCnt only if it is more
 *                       than the existing value in the reply
 */
template <class T>
void setFailoverHopCount(T& reply, uint32_t failoverHopCnt) {
  if constexpr (hasNonConstFailoverHopCount<T>) {
    if (failoverHopCnt > *reply.failoverHopCount_ref()) {
      reply.failoverHopCount_ref() = failoverHopCnt;
    }
  }
}

/**
 * setRequestDeadline - sets request deadline time to current time + deadlineMs
 *                     Applicable only to the Request types that support
 *                     request deadlines by having deadlineMs field in them
 *                    If the request does not have deadlineMs field, this API
 *                    is a no-op
 */
template <class Request>
void setRequestDeadline(Request& req, uint64_t deadlineMs) {
  if constexpr (hasNonConstDeadlineMs<Request>) {
    req.deadlineMs_ref() =
        facebook::memcache::mcrouter::getCurrentTimeInMs() + deadlineMs;
  }
}

/**
 * isRequestDeadlineExceeded - checks if the request deadline exceeded by
 *                    comparing the deadline time with current time.
 *                    If the request has not set the deadline time, then this
 *                    API returns false.
 *                    Applicable only to the Request types that support
 *                    request deadlines by having deadlineMs field in them
 *                    If the request does not have deadlineMs field, this API
 *                    always return false.
 */
template <class Request>
bool isRequestDeadlineExceeded(const Request& req) {
  if constexpr (hasConstDeadlineMs<Request>) {
    if (*req.deadlineMs_ref() > 0) {
      return facebook::memcache::mcrouter::getCurrentTimeInMs() >
          *req.deadlineMs_ref();
    }
  }
  return false;
}

/**
 * getDeadline - returns a pair with
 *    a bool - indicating if the request has support for deadline
 *    a uint64_t indicating the absolute deadline time in milliseconds since
 *               epoch
 */
template <class Request>
std::pair<bool, uint64_t> getDeadline(const Request& req) {
  if constexpr (hasConstDeadlineMs<Request>) {
    return {true, *req.deadlineMs_ref()};
  }
  return {false, 0};
}

/**
 * getRemainingTime - returns a pair with
 *    a bool - indicating if the request has support for deadline
 *    a uint64_t indicating the remaining time in milliseconds since epoch
 */
template <class Request>
std::pair<bool, uint64_t> getRemainingTime(const Request& req) {
  if constexpr (hasConstDeadlineMs<Request>) {
    auto deadlineMs = *req.deadlineMs_ref();
    auto currentTime = facebook::memcache::mcrouter::getCurrentTimeInMs();
    if (deadlineMs > 0 && (deadlineMs > currentTime)) {
      return {true, deadlineMs - currentTime};
    }
    return {true, 0};
  }
  return {false, 0};
}
