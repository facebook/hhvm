/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <folly/Memory.h>
#include <proxygen/lib/http/codec/HTTP1xCodec.h>
#include <proxygen/lib/http/codec/HTTP2Codec.h>
#include <proxygen/lib/http/codec/TransportDirection.h>
#include <proxygen/lib/http/codec/test/MockHTTPCodec.h>
#include <proxygen/lib/http/session/test/HTTPSessionMocks.h>

template <class MyCodec, class Version>
typename std::enable_if<std::is_enum<Version>::value,
                        std::unique_ptr<MyCodec>>::type
makeClientCodec(Version version) {
  return std::make_unique<MyCodec>(proxygen::TransportDirection::UPSTREAM,
                                   version);
}

template <class MyCodec, class Version>
typename std::enable_if<std::is_same<MyCodec, proxygen::HTTP1xCodec>::value,
                        std::unique_ptr<MyCodec>>::type
makeClientCodec(Version /*version*/) {
  return std::make_unique<MyCodec>(proxygen::TransportDirection::UPSTREAM);
}

template <class MyCodec, class Version>
typename std::enable_if<std::is_same<MyCodec, proxygen::HTTP2Codec>::value,
                        std::unique_ptr<MyCodec>>::type
makeClientCodec(Version /*version*/) {
  return std::make_unique<MyCodec>(proxygen::TransportDirection::UPSTREAM);
}

template <class MyCodec, class Version>
typename std::enable_if<std::is_same<MyCodec, proxygen::MockHTTPCodec>::value,
                        std::unique_ptr<MyCodec>>::type
makeClientCodec(Version /*version*/) {
  return std::make_unique<MyCodec>();
}

template <class MyCodec, class Version>
typename std::enable_if<std::is_enum<Version>::value,
                        std::unique_ptr<MyCodec>>::type
makeServerCodec(Version version) {
  return std::make_unique<MyCodec>(proxygen::TransportDirection::DOWNSTREAM,
                                   (Version)version);
}

template <class MyCodec, class Version>
typename std::enable_if<std::is_same<MyCodec, proxygen::HTTP1xCodec>::value,
                        std::unique_ptr<MyCodec>>::type
makeServerCodec(Version /*version*/) {
  return std::make_unique<MyCodec>(proxygen::TransportDirection::DOWNSTREAM);
}

template <class MyCodec, class Version>
typename std::enable_if<std::is_same<MyCodec, proxygen::HTTP2Codec>::value,
                        std::unique_ptr<MyCodec>>::type
makeServerCodec(Version /*version*/) {
  return std::make_unique<MyCodec>(proxygen::TransportDirection::DOWNSTREAM);
}

template <class MyCodec, class Version>
typename std::enable_if<std::is_same<MyCodec, proxygen::MockHTTPCodec>::value,
                        std::unique_ptr<MyCodec>>::type
makeServerCodec(Version /*version*/) {
  return std::make_unique<MyCodec>();
}
