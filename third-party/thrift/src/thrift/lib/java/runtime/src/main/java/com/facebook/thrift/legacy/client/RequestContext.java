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
import io.netty.util.ReferenceCounted;
import reactor.core.publisher.Sinks;

/**
 * Reference counting is delegated to {@link #encodedRequest}. This object is written through the
 * Netty pipeline, and Netty disposes of a dropped outbound message with {@code
 * ReferenceCountUtil.release(msg)} -- a silent no-op unless the message itself is {@link
 * ReferenceCounted}. Messages are dropped when the channel closes before {@code
 * ThriftClientHandler} converts this context into a {@code ThriftFrame} (head releases with a null
 * outbound buffer) or when the event loop rejects the write task.
 */
public class RequestContext<T, R> implements ReferenceCounted {
  private final Sinks.One<R> future;
  private final ClientRequestPayload<T> payload;
  private final ByteBuf encodedRequest;
  private final RpcOptions options;
  private final boolean oneway;
  private final int sequenceId;

  public RequestContext(
      final Sinks.One<R> future,
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

  public Sinks.One<R> getProcessor() {
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

  @Override
  public int refCnt() {
    return encodedRequest.refCnt();
  }

  @Override
  public RequestContext<T, R> retain() {
    encodedRequest.retain();
    return this;
  }

  @Override
  public RequestContext<T, R> retain(int increment) {
    encodedRequest.retain(increment);
    return this;
  }

  @Override
  public RequestContext<T, R> touch() {
    encodedRequest.touch();
    return this;
  }

  @Override
  public RequestContext<T, R> touch(Object hint) {
    encodedRequest.touch(hint);
    return this;
  }

  @Override
  public boolean release() {
    return encodedRequest.release();
  }

  @Override
  public boolean release(int decrement) {
    return encodedRequest.release(decrement);
  }
}
