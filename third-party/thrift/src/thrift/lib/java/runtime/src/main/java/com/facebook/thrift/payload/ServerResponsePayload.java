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

package com.facebook.thrift.payload;

import org.apache.thrift.ResponseRpcMetadata;
import org.apache.thrift.StreamPayloadMetadata;

public interface ServerResponsePayload {
  /*
   * In thrift, every response is either a thrift union for result and user declared exception or a TApplicationException.
   * For the first case we return a serialized thrift union where,
   * (1) field 0 is for response type (either primitive type such as int or thrift struct);
   * (2) field 1,2,3… - are for user declare exceptions in thrift IDL;
   * (3) In case of a void return no fields in that union should be set (empty union).
   * For the second case, a serialized TApplicationException is returned.
   */
  Writer getDataWriter();

  boolean isTApplicationException();

  ResponseRpcMetadata getResponseRpcMetadata();

  StreamPayloadMetadata getStreamPayloadMetadata();

  boolean isStreamingResponse();

  static ServerResponsePayload create(
      Writer dataWriter,
      ResponseRpcMetadata responseRpcMetadata,
      StreamPayloadMetadata streamPayloadMetadata,
      boolean hasStreamingResponse) {
    return new DefaultServerResponsePayload(
        dataWriter, false, responseRpcMetadata, streamPayloadMetadata, hasStreamingResponse);
  }

  static ServerResponsePayload createWithTApplicationException(
      Writer tApplicationExceptionWriter,
      ResponseRpcMetadata responseRpcMetadata,
      StreamPayloadMetadata streamPayloadMetadata,
      boolean hasStreamingResponse) {
    return new DefaultServerResponsePayload(
        tApplicationExceptionWriter,
        true,
        responseRpcMetadata,
        streamPayloadMetadata,
        hasStreamingResponse);
  }
}
