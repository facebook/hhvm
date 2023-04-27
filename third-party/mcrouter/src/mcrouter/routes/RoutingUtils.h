/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

namespace facebook {
namespace memcache {
namespace mcrouter {

/**
 * This function will issue a metaget to the route handle specified
 * to retrieve its exptime and then calculate a TTL based on the
 * current time and the object's exptime.  The TTL from this
 * moment forward should be very close in exptime of the original
 * object queried.
 * This is useful when we need a TTL to be set onto a new object that
 * has the same expiration time of another object.
 *
 * @param(rh)         - route handle to send the metaget to
 * @param(key)        - the key of the object in question
 * @param(newExptime) - an out parameter of the new exptime
 *
 * @return(bool)      - true if operation is successful,
 *                      false if a miss or if the new exptime
 *                      is already in the past.
 */
template <class RouteHandleIf>
static bool getExptimeFromRoute(
    const std::shared_ptr<RouteHandleIf>& rh,
    const folly::StringPiece& key,
    uint32_t& newExptime) {
  McMetagetRequest reqMetaget(key);
  auto warmMeta = rh->route(reqMetaget);
  if (isHitResult(*warmMeta.result_ref())) {
    newExptime = *warmMeta.exptime_ref();
    if (newExptime != 0) {
      auto curTime = time(nullptr);
      if (curTime >= newExptime) {
        return false;
      }
      newExptime -= curTime;
    }
    return true;
  }
  return false;
}

/**
 * This will create a new write request based on the value
 * of a Reply or Request object.
 *
 * @param(key)     - the key of the new request
 * @param(message) - the message that contains the value
 * @param(exptime) - exptime of the new object
 *
 * @return(ToRequest)
 */
template <class ToRequest, class Message>
static ToRequest createRequestFromMessage(
    const folly::StringPiece& key,
    const Message& message,
    uint32_t exptime) {
  ToRequest newReq(key);
  folly::IOBuf cloned = carbon::valuePtrUnsafe(message)
      ? carbon::valuePtrUnsafe(message)->cloneAsValue()
      : folly::IOBuf();
  newReq.value_ref() = std::move(cloned);
  newReq.flags_ref() = *message.flags_ref();
  newReq.exptime_ref() = exptime;
  return newReq;
}

/**
 * The function to extract the routing key from the request
 * and add the salt if it is not empty.
 *
 * @param(req)          - the request to extract the routing key from
 * @param(salt)         - salt to add to the end of the routing key, can be
 *                        empty string
 *
 * @return(std::string) - the resulting routing key
 */
template <class Request>
static std::string getRoutingKey(const Request& req, const std::string& salt) {
  auto routingKey = req.key_ref()->routingKey().str();
  if (!salt.empty()) {
    routingKey += salt;
  }
  return routingKey;
}

} // namespace mcrouter
} // namespace memcache
} // namespace facebook
