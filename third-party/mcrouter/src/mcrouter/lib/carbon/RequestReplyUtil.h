/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <optional>
#include <ostream>
#include <type_traits>

#include <folly/Optional.h>
#include <folly/Range.h>
#include <folly/io/IOBuf.h>
#include <thrift/lib/cpp2/FieldRef.h>

#include "mcrouter/lib/fbi/cpp/TypeList.h"
#include "mcrouter/lib/fbi/cpp/util.h"
#include "mcrouter/lib/mc/protocol.h"
#include "mcrouter/lib/network/MessageHelpers.h"

namespace facebook {
namespace memcache {
struct CaretMessageInfo;
class McServerRequestContext;
} // namespace memcache
} // namespace facebook

namespace carbon {

namespace detail {

template <typename T, typename = std::enable_if_t<true>>
struct HasIsFailover : std::false_type {};
template <typename T>
struct HasIsFailover<
    T,
    std::void_t<decltype(std::declval<T>().isFailover_ref())>>
    : std::true_type {};

template <typename T, typename = std::enable_if_t<true>>
struct HasMessage : std::false_type {};
template <typename T>
struct HasMessage<T, std::void_t<decltype(std::declval<T>().message_ref())>>
    : std::true_type {};

template <typename T, typename = std::enable_if_t<true>>
struct HasAppSpecificErrorCode : std::false_type {};
template <typename T>
struct HasAppSpecificErrorCode<
    T,
    std::void_t<decltype(std::declval<T>().appSpecificErrorCode_ref())>>
    : std::true_type {};

template <class RequestList>
struct GetRequestReplyPairsImpl;

template <class Request, class... Requests>
struct GetRequestReplyPairsImpl<List<Request, Requests...>> {
  using type = facebook::memcache::PrependT<
      facebook::memcache::Pair<Request, typename Request::reply_type>,
      typename GetRequestReplyPairsImpl<List<Requests...>>::type>;
};

template <>
struct GetRequestReplyPairsImpl<List<>> {
  using type = List<>;
};

} // namespace detail

template <class RequestList>
using GetRequestReplyPairs =
    typename detail::GetRequestReplyPairsImpl<RequestList>::type;

template <typename Reply>
typename std::enable_if_t<detail::HasMessage<Reply>::value> setMessageIfPresent(
    Reply& reply,
    std::string msg) {
  reply.message_ref() = std::move(msg);
}
template <typename Reply>
typename std::enable_if_t<!detail::HasMessage<Reply>::value>
setMessageIfPresent(Reply&, std::string) {}

template <typename Reply>
typename std::enable_if_t<detail::HasMessage<Reply>::value, folly::StringPiece>
getMessage(const Reply& reply) {
  return *reply.message_ref();
}
template <typename Reply>
typename std::enable_if_t<!detail::HasMessage<Reply>::value, folly::StringPiece>
getMessage(const Reply&) {
  return folly::StringPiece{};
}

template <typename Reply>
typename std::enable_if_t<detail::HasIsFailover<Reply>::value>
setIsFailoverIfPresent(Reply& reply, bool isFailover) {
  reply.isFailover_ref() = isFailover;
}
template <typename Reply>
typename std::enable_if_t<!detail::HasIsFailover<Reply>::value>
setIsFailoverIfPresent(Reply&, bool) {}

template <class Request>
typename std::enable_if_t<
    facebook::memcache::HasKeyTrait<Request>::value,
    folly::StringPiece>
getFullKey(const Request& req) {
  return req.key_ref()->fullKey();
}
template <class Request>
typename std::enable_if_t<
    !facebook::memcache::HasKeyTrait<Request>::value,
    folly::StringPiece>
getFullKey(const Request&) {
  return "";
}

template <typename Reply>
typename std::enable_if_t<
    detail::HasAppSpecificErrorCode<Reply>::value,
    folly::Optional<int16_t>>
getAppSpecificErrorCode(const Reply& reply) {
  return *reply.appSpecificErrorCode_ref();
}
template <typename Reply>
typename std::enable_if_t<
    !detail::HasAppSpecificErrorCode<Reply>::value,
    folly::Optional<int16_t>>
getAppSpecificErrorCode(const Reply&) {
  return folly::none;
}

template <class Message>
typename std::enable_if_t<
    facebook::memcache::HasExptimeTrait<Message>::value,
    std::optional<int32_t>>
getExptime(const Message& message) {
  return std::make_optional(*message.exptime_ref());
}

template <class Message>
typename std::enable_if_t<
    !facebook::memcache::HasExptimeTrait<Message>::value,
    std::optional<int32_t>>
getExptime(const Message& /* message */) {
  return std::nullopt;
}

namespace detail {

inline folly::IOBuf* bufPtr(folly::Optional<folly::IOBuf>& buf) {
  return buf.get_pointer();
}
inline const folly::IOBuf* bufPtr(const folly::Optional<folly::IOBuf>& buf) {
  return buf.get_pointer();
}

inline folly::IOBuf* bufPtr(folly::IOBuf& buf) {
  return &buf;
}
inline const folly::IOBuf* bufPtr(const folly::IOBuf& buf) {
  return &buf;
}

inline folly::IOBuf* bufPtr(apache::thrift::field_ref<folly::IOBuf&> buf) {
  return &buf.value();
}
inline const folly::IOBuf* bufPtr(
    apache::thrift::field_ref<const folly::IOBuf&> buf) {
  return &buf.value();
}

inline folly::IOBuf* bufPtr(
    apache::thrift::optional_field_ref<folly::IOBuf&> buf) {
  return buf.has_value() ? &buf.value() : nullptr;
}
inline const folly::IOBuf* bufPtr(
    apache::thrift::optional_field_ref<const folly::IOBuf&> buf) {
  return buf.has_value() ? &buf.value() : nullptr;
}

} // namespace detail

template <class R>
typename std::enable_if<
    facebook::memcache::HasValueTrait<R>::value,
    const folly::IOBuf*>::type
valuePtrUnsafe(const R& requestOrReply) {
  return detail::bufPtr(requestOrReply.value_ref());
}
template <class R>
typename std::
    enable_if<facebook::memcache::HasValueTrait<R>::value, folly::IOBuf*>::type
    valuePtrUnsafe(R& requestOrReply) {
  return detail::bufPtr(requestOrReply.value_ref());
}
template <class R>
typename std::
    enable_if<!facebook::memcache::HasValueTrait<R>::value, folly::IOBuf*>::type
    valuePtrUnsafe(const R& /* requestOrReply */) {
  return nullptr;
}

template <class R>
typename std::enable_if<
    facebook::memcache::HasValueTrait<R>::value,
    folly::StringPiece>::type
valueRangeSlow(R& requestOrReply) {
  auto* buf = detail::bufPtr(requestOrReply.value_ref());
  return buf ? folly::StringPiece(buf->coalesce()) : folly::StringPiece();
}

template <class R>
typename std::enable_if<
    !facebook::memcache::HasValueTrait<R>::value,
    folly::StringPiece>::type
valueRangeSlow(R& /* requestOrReply */) {
  return folly::StringPiece();
}

// Helper class to determine whether a type is a Carbon request.
template <class Msg>
class IsRequestTrait {
  template <class T>
  static std::true_type check(typename T::reply_type*);
  template <class T>
  static std::false_type check(...);

 public:
  static constexpr bool value = decltype(check<Msg>(0))::value;
};

template <class Request>
std::optional<std::string> getBucketIdFromRequest(const Request& req) {
  static_assert(IsRequestTrait<Request>::value);
  if constexpr (facebook::memcache::HasBucketIdTrait<Request>::value) {
    if (req.bucketId_ref().has_value()) {
      return std::make_optional(*req.bucketId_ref());
    }
    return std::nullopt;
  }
  return std::nullopt;
}

template <class R>
typename std::enable_if<facebook::memcache::HasFlagsTrait<R>::value, uint64_t>::
    type
    getFlags(const R& requestOrReply) {
  return *requestOrReply.flags_ref();
}

template <class R>
typename std::
    enable_if<!facebook::memcache::HasFlagsTrait<R>::value, uint64_t>::type
    getFlags(const R&) {
  return 0;
}

/**
 * Helper function to determine typeId by its name.
 *
 * @param name  Name of the struct as specified by Type::name
 *
 * @return  Type::typeId for matched Type if any. 0 if no match found.
 */
template <class TypeList>
inline size_t getTypeIdByName(folly::StringPiece name, TypeList);

template <>
inline size_t getTypeIdByName(folly::StringPiece /* name */, List<>) {
  return 0;
}

template <class T, class... Ts>
inline size_t getTypeIdByName(folly::StringPiece name, List<T, Ts...>) {
  return name == T::name ? T::typeId : getTypeIdByName(name, List<Ts...>());
}

/**
 * Helpers to print out all know type names.
 */
template <class TypeList>
inline std::ostream& insertTypeIds(std::ostream&, TypeList);

template <>
inline std::ostream& insertTypeIds(std::ostream& str, List<>) {
  return str;
}

template <class T, class... Ts>
inline std::ostream& insertTypeIds(std::ostream& str, List<T, Ts...>) {
  str << ' ' << T::name;
  return insertTypeIds(str, List<Ts...>());
}

template <class TypeList>
inline ssize_t getIndexInListByName(folly::StringPiece name, TypeList);

template <>
inline ssize_t getIndexInListByName(folly::StringPiece /* name */, List<>) {
  return -1;
}

template <class T, class... Ts>
inline ssize_t getIndexInListByName(folly::StringPiece name, List<T, Ts...>) {
  return name == T::name
      ? 0
      : (getIndexInListByName(name, List<Ts...>()) == -1
             ? -1
             : 1 + getIndexInListByName(name, List<Ts...>()));
}

namespace detail {
template <class List>
struct RequestListLimitsImpl;

template <>
struct RequestListLimitsImpl<List<>> {
  static constexpr size_t minTypeId = std::numeric_limits<size_t>::max();
  static constexpr size_t maxTypeId = std::numeric_limits<size_t>::min();
  static constexpr size_t typeIdRangeSize = 0;
};

template <class T, class... Ts>
struct RequestListLimitsImpl<List<T, Ts...>> {
  static constexpr size_t minTypeId =
      T::typeId <= RequestListLimitsImpl<List<Ts...>>::minTypeId
      ? T::typeId
      : RequestListLimitsImpl<List<Ts...>>::minTypeId;
  static constexpr size_t maxTypeId =
      T::typeId >= RequestListLimitsImpl<List<Ts...>>::maxTypeId
      ? T::typeId
      : RequestListLimitsImpl<List<Ts...>>::maxTypeId;
  static constexpr size_t typeIdRangeSize = maxTypeId - minTypeId + 1;
};
} // namespace detail

/**
 * Limits (min, max and rangeSize) of a list of requests.
 */
template <class RequestList>
using RequestListLimits = detail::RequestListLimitsImpl<RequestList>;

/**
 * Map of type T, where the key is Request::typeId.
 *
 * @tparam RequestList  List of request.
 * @tparam T            Type of the elements of the map.
 */
template <class RequestList, class T>
class RequestIdMap {
 public:
  static constexpr size_t kMinId = RequestListLimits<RequestList>::minTypeId;
  static constexpr size_t kMaxId = RequestListLimits<RequestList>::maxTypeId;
  static constexpr size_t kSize =
      RequestListLimits<RequestList>::typeIdRangeSize;

  const T& getById(size_t id) const {
    facebook::memcache::checkLogic(
        kMinId <= id && id <= kMaxId,
        "Id {} is out of range [{}, {}]",
        id,
        kMinId,
        kMaxId);
    return container_[id - kMinId];
  }

  template <class Request>
  const T& getByRequestType() const {
    static_assert(
        ListContains<RequestList, Request>::value,
        "Supplied Request type is not in RequestList");
    return container_[Request::typeId - kMinId];
  }

  void set(size_t id, T&& val) {
    facebook::memcache::checkLogic(
        kMinId <= id && id <= kMaxId,
        "Id {} is out of range [{}, {}]",
        id,
        kMinId,
        kMaxId);
    container_[id - kMinId] = std::move(val);
  }

 private:
  std::array<T, kSize> container_;
};

template <class RequestList, class T>
constexpr size_t RequestIdMap<RequestList, T>::kMinId;

template <class RequestList, class T>
constexpr size_t RequestIdMap<RequestList, T>::kMaxId;

namespace detail {

/**
 * Utility class useful for checking whether a particular OnRequest handler
 * class defines an onRequest() handler for Request.
 *
 * @tparam Request    The Request type.
 * @tparam OnReqest   The OnRequest type.
 */
class CanHandleRequest {
  template <class Request, class OnRequest>
  static constexpr auto check(int)
      -> decltype(std::declval<OnRequest>().onRequest(std::declval<facebook::memcache::McServerRequestContext>(), std::declval<Request>()), std::true_type()) {
    return {};
  }

  template <class R, class O>
  static constexpr std::false_type check(...) {
    return {};
  }

 public:
  template <class Request, class OnRequest>
  static constexpr auto value() -> decltype(check<Request, OnRequest>(0)) {
    return {};
  }
};

/**
 * Utility class useful for checking whether a particular OnRequest handler
 * class defines an onRequest() handler for Request with a pointer to the raw
 * buffer.
 *
 * @tparam Request    The Request type.
 * @tparam OnReqest   The OnRequest type.
 */
class CanHandleRequestWithBuffer {
  template <class Request, class OnRequest>
  static constexpr auto check(int) -> decltype(std::declval<OnRequest>().onRequest(std::declval<facebook::memcache::McServerRequestContext>(), std::declval<Request>(), std::declval<facebook::memcache::CaretMessageInfo*>(), std::declval<folly::IOBuf*>()), std::true_type()) {
    return {};
  }

  template <class Request, class OnRequest>
  static constexpr std::false_type check(...) {
    return {};
  }

 public:
  template <class Request, class OnRequest>
  static constexpr auto value() -> decltype(check<Request, OnRequest>(0)) {
    return {};
  }
};

} // namespace detail

} // namespace carbon
