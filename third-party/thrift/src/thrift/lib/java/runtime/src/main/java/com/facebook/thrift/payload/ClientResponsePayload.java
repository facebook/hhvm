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

import com.facebook.thrift.client.ResponseWrapper;
import com.facebook.thrift.model.StreamResponse;
import java.nio.charset.StandardCharsets;
import java.util.Collections;
import java.util.HashMap;
import java.util.Map;
import org.apache.thrift.ResponseRpcMetadata;
import org.apache.thrift.StreamPayloadMetadata;

public interface ClientResponsePayload<T> extends ResponseWrapper<T> {

  Exception getException();

  ResponseRpcMetadata getResponseRpcMetadata();

  StreamPayloadMetadata getStreamPayloadMetadata();

  boolean isStreamingResponse();

  Integer getStreamId();

  @SuppressWarnings({"rawtypes"})
  static ClientResponsePayload createEmptyStreamingResult(
      ResponseRpcMetadata responseRpcMetadata, StreamPayloadMetadata streamPayloadMetadata) {
    return new DefaultClientResponsePayload<>(
        StreamResponse.emptyResponse(),
        null,
        responseRpcMetadata,
        streamPayloadMetadata,
        null,
        true,
        null);
  }

  static <T> ClientResponsePayload<T> createResult(
      T data,
      ResponseRpcMetadata responseRpcMetadata,
      StreamPayloadMetadata streamPayloadMetadata,
      boolean hasStreamingResponse) {
    return new DefaultClientResponsePayload<>(
        data, null, responseRpcMetadata, streamPayloadMetadata, null, hasStreamingResponse, null);
  }

  static <T> ClientResponsePayload<T> createStreamResult(
      T data,
      ResponseRpcMetadata responseRpcMetadata,
      StreamPayloadMetadata streamPayloadMetadata,
      Map<String, byte[]> binaryHeaders,
      Integer streamId) {
    return new DefaultClientResponsePayload<>(
        data, null, responseRpcMetadata, streamPayloadMetadata, binaryHeaders, true, streamId);
  }

  @SuppressWarnings("rawtypes")
  static ClientResponsePayload createException(
      Exception error,
      ResponseRpcMetadata responseRpcMetadata,
      StreamPayloadMetadata streamPayloadMetadata,
      boolean hasStreamingResponse) {
    return new DefaultClientResponsePayload<>(
        null, error, responseRpcMetadata, streamPayloadMetadata, null, hasStreamingResponse, null);
  }

  static Map<String, String> convertToMapStringString(Map<String, byte[]> source) {
    Map<String, String> map = new HashMap<>();
    source.forEach((s, bytes) -> map.put(s, new String(bytes, StandardCharsets.UTF_8)));
    return map;
  }

  @Override
  default Map<String, String> getHeaders() {
    ResponseRpcMetadata metadata = getResponseRpcMetadata();
    StreamPayloadMetadata streamMetadata = getStreamPayloadMetadata();
    Map<String, String> responseHeaders;
    if (streamMetadata != null && streamMetadata.getOtherMetadata() != null) {
      responseHeaders = convertToMapStringString(streamMetadata.getOtherMetadata());
    } else if (metadata != null && metadata.getOtherMetadata() != null) {
      responseHeaders = metadata.getOtherMetadata();
    } else {
      responseHeaders = Collections.emptyMap();
    }
    return responseHeaders;
  }
}
