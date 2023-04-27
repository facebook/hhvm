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

public class DefaultServerResponsePayload implements ServerResponsePayload {
  private final Writer dataWriter;

  private final boolean isTApplicationException;

  private final ResponseRpcMetadata responseRpcMetadata;

  private final StreamPayloadMetadata streamPayloadMetadata;

  private final boolean hasStreamingResponse;

  public DefaultServerResponsePayload(
      Writer dataWriter,
      boolean isTApplicationException,
      ResponseRpcMetadata responseRpcMetadata,
      StreamPayloadMetadata streamPayloadMetadata,
      boolean hasStreamingResponse) {
    this.dataWriter = dataWriter;
    this.isTApplicationException = isTApplicationException;
    this.responseRpcMetadata = responseRpcMetadata;
    this.streamPayloadMetadata = streamPayloadMetadata;
    this.hasStreamingResponse = hasStreamingResponse;
  }

  @Override
  public Writer getDataWriter() {
    return dataWriter;
  }

  @Override
  public boolean isTApplicationException() {
    return isTApplicationException;
  }

  @Override
  public ResponseRpcMetadata getResponseRpcMetadata() {
    return responseRpcMetadata;
  }

  @Override
  public StreamPayloadMetadata getStreamPayloadMetadata() {
    return streamPayloadMetadata;
  }

  @Override
  public boolean isStreamingResponse() {
    return hasStreamingResponse;
  }
}
