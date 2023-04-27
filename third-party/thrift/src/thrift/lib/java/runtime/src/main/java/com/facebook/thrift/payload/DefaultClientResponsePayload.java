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

import java.nio.charset.StandardCharsets;
import java.util.HashMap;
import java.util.Map;
import org.apache.thrift.ResponseRpcMetadata;
import org.apache.thrift.StreamPayloadMetadata;

final class DefaultClientResponsePayload<T> implements ClientResponsePayload<T> {
  private final T data;
  private final Exception error;
  private final ResponseRpcMetadata responseRpcMetadata;
  private final StreamPayloadMetadata streamPayloadMetadata;
  private final boolean hasStreamingResponse;
  private final Map<String, byte[]> binaryHeaders;
  private final Integer streamId;

  DefaultClientResponsePayload(
      T data,
      Exception error,
      ResponseRpcMetadata responseRpcMetadata,
      StreamPayloadMetadata streamPayloadMetadata,
      Map<String, byte[]> binaryHeaders,
      boolean hasStreamingResponse,
      Integer streamId) {
    this.data = data;
    this.error = error;
    this.responseRpcMetadata = responseRpcMetadata;
    this.streamPayloadMetadata = streamPayloadMetadata;
    this.hasStreamingResponse = hasStreamingResponse;
    this.binaryHeaders = getBinaryHeaders(binaryHeaders, streamPayloadMetadata);
    this.streamId = streamId;
  }

  private Map<String, byte[]> getBinaryHeaders(
      final Map<String, byte[]> binaryHeaders, final StreamPayloadMetadata streamPayloadMetadata) {
    if (streamPayloadMetadata != null) {
      Map<String, byte[]> otherMetadata = streamPayloadMetadata.getOtherMetadata();
      if (otherMetadata == null && binaryHeaders != null) {
        return binaryHeaders;
      } else {
        return otherMetadata;
      }
    } else {
      return createBinaryHeaders(getHeaders());
    }
  }

  @Override
  public T getData() {
    return data;
  }

  @Override
  public Map<String, byte[]> getBinaryHeaders() {
    return binaryHeaders;
  }

  @Override
  public Exception getException() {
    return error;
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

  @Override
  public Integer getStreamId() {
    return streamId;
  }

  static Map<String, byte[]> createBinaryHeaders(Map<String, String> headers) {
    Map<String, byte[]> binaryHeaders = new HashMap<>();
    headers.forEach((key, value) -> binaryHeaders.put(key, value.getBytes(StandardCharsets.UTF_8)));
    return binaryHeaders;
  }
}
