/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

namespace facebook {
namespace memcache {

struct AsciiSerializedRequest::PrepareImplWrapper {
  template <class Request>
  using PrepareType =
      decltype(std::declval<AsciiSerializedRequest>().prepareImpl(
          std::declval<const Request&>()));

  template <class Request>
  typename std::enable_if<
      std::is_same<PrepareType<Request>, std::false_type>::value,
      bool>::type static prepare(AsciiSerializedRequest&, const Request&) {
    return false;
  }

  template <class Request>
  typename std::enable_if<
      std::is_same<PrepareType<Request>, void>::value,
      bool>::
      type static prepare(AsciiSerializedRequest& s, const Request& request) {
    s.prepareImpl(request);
    return true;
  }
};

template <class Arg1, class Arg2>
void AsciiSerializedRequest::addStrings(Arg1&& arg1, Arg2&& arg2) {
  addString(std::forward<Arg1>(arg1));
  addString(std::forward<Arg2>(arg2));
}

template <class Arg, class... Args>
void AsciiSerializedRequest::addStrings(Arg&& arg, Args&&... args) {
  addString(std::forward<Arg>(arg));
  addStrings(std::forward<Args>(args)...);
}

template <class Request>
bool AsciiSerializedRequest::prepare(
    const Request& request,
    const struct iovec*& iovOut,
    size_t& niovOut) {
  iovsCount_ = 0;
  auto r = PrepareImplWrapper::prepare(*this, request);
  iovOut = iovs_;
  niovOut = iovsCount_;
  return r;
}

template <class Arg1, class Arg2>
void AsciiSerializedReply::addStrings(Arg1&& arg1, Arg2&& arg2) {
  addString(std::forward<Arg1>(arg1));
  addString(std::forward<Arg2>(arg2));
}

template <class Arg, class... Args>
void AsciiSerializedReply::addStrings(Arg&& arg, Args&&... args) {
  addString(std::forward<Arg>(arg));
  addStrings(std::forward<Args>(args)...);
}
} // namespace memcache
} // namespace facebook
