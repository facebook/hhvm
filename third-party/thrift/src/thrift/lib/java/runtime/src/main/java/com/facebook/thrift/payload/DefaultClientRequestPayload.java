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

import java.util.Map;
import org.apache.thrift.RequestRpcMetadata;

@SuppressWarnings("rawtypes")
final class DefaultClientRequestPayload<T> implements ClientRequestPayload<T> {
  private final String serviceName;
  private final Writer dataWriter;
  private final Reader<T> reader;
  private final Reader firstResponseReader;
  private final Map<Short, Reader> exceptionReaders;
  private final Map<Short, Reader> streamExceptionReaders;
  private final RequestRpcMetadata requestRpcMetadata;
  private final Map<String, String> persistentHeaders;

  DefaultClientRequestPayload(
      final String serviceName,
      final Writer dataWriter,
      final Reader<T> reader,
      final Reader firstResponseReader,
      final Map<Short, Reader> exceptionReaders,
      final Map<Short, Reader> streamExceptionReaders,
      final RequestRpcMetadata requestRpcMetadata,
      final Map<String, String> persistentHeaders) {
    this.serviceName = serviceName;
    this.dataWriter = dataWriter;
    this.reader = reader;
    this.firstResponseReader = firstResponseReader;
    this.exceptionReaders = exceptionReaders;
    this.streamExceptionReaders = streamExceptionReaders;
    this.requestRpcMetadata = requestRpcMetadata;
    this.persistentHeaders = persistentHeaders;
  }

  @Override
  public String getServiceName() {
    return serviceName;
  }

  @Override
  public Writer getDataWriter() {
    return dataWriter;
  }

  @Override
  public Reader<T> getResponseReader() {
    return reader;
  }

  @Override
  public Reader getFirstResponseReader() {
    return firstResponseReader;
  }

  @Override
  public Map<Short, Reader> getExceptionReaders() {
    return exceptionReaders;
  }

  @Override
  public Map<Short, Reader> getStreamExceptionReaders() {
    return streamExceptionReaders;
  }

  @Override
  public RequestRpcMetadata getRequestRpcMetadata() {
    return requestRpcMetadata;
  }

  @Override
  public Map<String, String> getPersistentHeaders() {
    return persistentHeaders;
  }
}
