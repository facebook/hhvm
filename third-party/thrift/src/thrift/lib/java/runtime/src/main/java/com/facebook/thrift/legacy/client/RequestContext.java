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

package com.facebook.thrift.legacy.client;

import com.facebook.thrift.client.RpcOptions;
import com.facebook.thrift.payload.ClientRequestPayload;
import io.netty.buffer.ByteBuf;
import reactor.core.publisher.MonoProcessor;

public class RequestContext<T, R> {
  private final MonoProcessor<R> future;
  private final ClientRequestPayload<T> payload;
  private final ByteBuf encodedRequest;
  private final RpcOptions options;
  private final boolean oneway;
  private final int sequenceId;

  public RequestContext(
      final MonoProcessor<R> future,
      final ClientRequestPayload<T> payload,
      final ByteBuf encodedRequest,
      final RpcOptions options,
      final boolean oneway,
      final int sequenceId) {
    this.future = future;
    this.payload = payload;
    this.encodedRequest = encodedRequest;
    this.options = options;
    this.oneway = oneway;
    this.sequenceId = sequenceId;
  }

  public MonoProcessor<R> getProcessor() {
    return future;
  }

  public ClientRequestPayload<T> getPayload() {
    return payload;
  }

  public RpcOptions getOptions() {
    return options;
  }

  public boolean isOneway() {
    return oneway;
  }

  public ByteBuf getEncodedRequest() {
    return encodedRequest;
  }

  public int getSequenceId() {
    return sequenceId;
  }
}
