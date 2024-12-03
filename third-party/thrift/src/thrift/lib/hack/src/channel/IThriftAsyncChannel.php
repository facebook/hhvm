<?hh
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
 *
 */

// @oss-enable: use namespace FlibSL\{C, Math, Str, Vec};

// Result message and read headers
type TAsyncChannelResponse = (string, darray<string, string>);
type TAsyncChannelStreamResponse =
  (string, darray<string, string>, TClientBufferedStream);
type TAsyncChannelSinkResponse = (string, darray<string, string>, TClientSink);

<<Oncalls('thrift')>> // @oss-disable
interface IThriftAsyncChannel {
  public function genSendRequestResponse(
    \RpcOptions $options,
    string $msg,
  ): Awaitable<TAsyncChannelResponse>;

  public function genSendRequestNoResponse(
    \RpcOptions $options,
    string $msg,
  ): Awaitable<void>;

  public function genSendRequestStreamResponse(
    \RpcOptions $options,
    string $msg,
  ): Awaitable<TAsyncChannelStreamResponse>;

  public function genSendRequestSink(
    \RpcOptions $options,
    string $msg,
  ): Awaitable<TAsyncChannelSinkResponse>;
}
