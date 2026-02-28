/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#pragma once

#include <memory>
#include <type_traits>

namespace apache::thrift {

template <typename Response, typename StreamItem>
struct ResponseAndServerStream;

template <typename Response, typename SinkElement, typename FinalResponse>
class ResponseAndSinkConsumer;

template <typename Response, typename InputElement, typename OutputElement>
struct ResponseAndStreamTransformation;

namespace detail {

// I found 64 bytes from here https://fburl.com/wiki/nstmt10g
template <typename T>
concept SmallTriviallyCopyable =
    std::is_trivially_copyable_v<T> && sizeof(T) <= 64;

/// Unwraps std::unique_ptr<T> to T; identity for non-unique_ptr types.
template <typename T>
struct inner_type {
  using type = T;
};
template <typename T>
struct inner_type<std::unique_ptr<T>> {
  using type = T;
};

/// Detects compound response types that have a ResponseType typedef and a
/// .response member (e.g. ResponseAndServerStream, ResponseAndSinkConsumer,
/// ResponseAndStreamTransformation).
template <typename T>
concept CompoundResponseType = requires { typename T::ResponseType; };

/**
 * DecoratorArgType is a helper class for determining the type of the arg
 * passed to the decorator. For small trivially copyable types <= 64 bytes,
 * we just pass by value and allow the copy to happen. For everything else, we
 * pass by const reference.
 */
template <typename T>
struct DecoratorArgType {
  using type = const T&;
};

template <SmallTriviallyCopyable T>
struct DecoratorArgType<T> {
  using type = T;
};

/**
 * DecoratorReturnType determines the type of the arg passed to the decorator
 * after_ methods. Delegates to DecoratorArgType for pass-by-value vs
 * const-ref logic. For compound response types, recursively unwraps to the
 * inner response type.
 */
template <typename T>
struct DecoratorReturnType {
  using type = typename DecoratorArgType<T>::type;
};

template <CompoundResponseType T>
struct DecoratorReturnType<T>
    : DecoratorReturnType<typename inner_type<typename T::ResponseType>::type> {
};

} // namespace detail

} // namespace apache::thrift
