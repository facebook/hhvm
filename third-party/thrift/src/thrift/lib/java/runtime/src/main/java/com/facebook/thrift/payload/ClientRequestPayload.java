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

import java.util.HashMap;
import java.util.Map;
import org.apache.thrift.RequestRpcMetadata;

@SuppressWarnings("rawtypes")
public interface ClientRequestPayload<T> {
  String getServiceName();

  Writer getDataWriter();

  Reader<T> getResponseReader();

  Reader getFirstResponseReader();

  Map<Short, Reader> getExceptionReaders();

  Map<Short, Reader> getStreamExceptionReaders();

  RequestRpcMetadata getRequestRpcMetadata();

  Map<String, String> getPersistentHeaders();

  @SuppressWarnings("unused")
  default ClientRequestPayload<T> withAdditionalHeader(Map<String, String> additionalHeader) {
    Map<String, String> allHeaders = new HashMap<>();
    allHeaders.putAll(getRequestRpcMetadata().getOtherMetadata());
    allHeaders.putAll(additionalHeader);

    return new DefaultClientRequestPayload<>(
        getServiceName(),
        getDataWriter(),
        getResponseReader(),
        getFirstResponseReader(),
        getExceptionReaders(),
        getStreamExceptionReaders(),
        new RequestRpcMetadata.Builder(getRequestRpcMetadata())
            .setOtherMetadata(allHeaders)
            .build(),
        getPersistentHeaders());
  }

  static <T> ClientRequestPayload<T> create(
      final String serviceName,
      final Writer dataWriter,
      final Reader<T> reader,
      final Map<Short, Reader> exceptionReaders,
      final RequestRpcMetadata requestRpcMetadata,
      final Map<String, String> persistentHeaders) {
    return new DefaultClientRequestPayload<>(
        serviceName,
        dataWriter,
        reader,
        null,
        exceptionReaders,
        null,
        requestRpcMetadata,
        persistentHeaders);
  }

  static <T> ClientRequestPayload<T> create(
      final String serviceName,
      final Writer dataWriter,
      final Reader<T> reader,
      final Map<Short, Reader> exceptionReaders,
      final Map<Short, Reader> streamExceptionReaders,
      final RequestRpcMetadata requestRpcMetadata,
      final Map<String, String> persistentHeaders) {
    return new DefaultClientRequestPayload<>(
        serviceName,
        dataWriter,
        reader,
        null,
        exceptionReaders,
        streamExceptionReaders,
        requestRpcMetadata,
        persistentHeaders);
  }

  static <T> ClientRequestPayload<T> create(
      final String serviceName,
      final Writer dataWriter,
      final Reader<T> reader,
      final Reader firstResponseReader,
      final Map<Short, Reader> exceptionReaders,
      final Map<Short, Reader> streamExceptionReaders,
      final RequestRpcMetadata requestRpcMetadata,
      final Map<String, String> persistentHeaders) {
    return new DefaultClientRequestPayload<>(
        serviceName,
        dataWriter,
        reader,
        firstResponseReader,
        exceptionReaders,
        streamExceptionReaders,
        requestRpcMetadata,
        persistentHeaders);
  }
}
