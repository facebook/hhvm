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
 * DecoratorReturnType is a helper class for determining the type of the arg
 * passed to the decorator after_ methods. For small trivially copyable types
 * <= 64 bytes, we just pass by value and allow the copy to happen. For
 * everything else, we pass by const reference.
 */
template <typename T>
struct DecoratorReturnType {
  using type = const T&;
};

template <SmallTriviallyCopyable T>
struct DecoratorReturnType<T> {
  using type = T;
};

template <typename Response, typename StreamItem>
struct DecoratorReturnType<ResponseAndServerStream<Response, StreamItem>>
    : public DecoratorReturnType<Response> {};

template <typename Response, typename StreamItem>
struct DecoratorReturnType<
    ResponseAndServerStream<std::unique_ptr<Response>, StreamItem>>
    : public DecoratorReturnType<Response> {};

template <typename Response, typename SinkElement, typename FinalResponse>
struct DecoratorReturnType<
    ResponseAndSinkConsumer<Response, SinkElement, FinalResponse>>
    : public DecoratorReturnType<Response> {};

template <typename Response, typename SinkElement, typename FinalResponse>
struct DecoratorReturnType<ResponseAndSinkConsumer<
    std::unique_ptr<Response>,
    SinkElement,
    FinalResponse>> : public DecoratorReturnType<Response> {};

template <typename Response, typename InputElement, typename OutputElement>
struct DecoratorReturnType<
    ResponseAndStreamTransformation<Response, InputElement, OutputElement>>
    : public DecoratorReturnType<Response> {};

template <typename Response, typename InputElement, typename OutputElement>
struct DecoratorReturnType<ResponseAndStreamTransformation<
    std::unique_ptr<Response>,
    InputElement,
    OutputElement>> : public DecoratorReturnType<Response> {};

} // namespace detail

} // namespace apache::thrift
