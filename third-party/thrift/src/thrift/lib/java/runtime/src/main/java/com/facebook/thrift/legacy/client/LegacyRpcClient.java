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

import static org.apache.thrift.protocol.TMessageType.CALL;
import static org.apache.thrift.protocol.TMessageType.ONEWAY;

import com.facebook.thrift.client.RpcClient;
import com.facebook.thrift.client.RpcOptions;
import com.facebook.thrift.client.ThriftClientConfig;
import com.facebook.thrift.legacy.exceptions.ChannelNotActiveException;
import com.facebook.thrift.payload.ClientRequestPayload;
import com.facebook.thrift.payload.ClientResponsePayload;
import com.facebook.thrift.payload.Writer;
import com.facebook.thrift.protocol.TProtocolType;
import com.facebook.thrift.util.NettyUtil;
import com.facebook.thrift.util.resources.RpcResources;
import io.netty.buffer.ByteBuf;
import io.netty.buffer.ByteBufAllocator;
import io.netty.channel.Channel;
import io.netty.util.ReferenceCountUtil;
import java.util.concurrent.atomic.AtomicInteger;
import org.apache.thrift.ProtocolId;
import org.apache.thrift.protocol.TMessage;
import org.apache.thrift.protocol.TProtocol;
import org.apache.thrift.protocol.TStruct;
import reactor.core.publisher.Mono;
import reactor.core.publisher.MonoProcessor;
import reactor.core.scheduler.Scheduler;

final class LegacyRpcClient implements RpcClient {
  private static final int ONEWAY_SEQ_IQ = 0xFFFFFFFF;
  private static final TStruct PARAMETERS_STRUCT = new TStruct();
  private final ByteBufAllocator alloc;
  private final AtomicInteger sequenceId;
  private final Channel channel;
  private final ThriftClientConfig nettyConfig;
  private final MonoProcessor<Void> onClose;
  private final Scheduler scheduler;
  private final boolean forceExecutionOffEventLoop;
  private final Mono<?> emitExceptionOnClose;

  LegacyRpcClient(
      final Channel channel,
      final Scheduler scheduler,
      final boolean forceExecutionOffEventLoop,
      final ThriftClientConfig nettyConfig) {
    this.sequenceId = new AtomicInteger(0);
    this.channel = channel;
    this.nettyConfig = nettyConfig;
    this.onClose = MonoProcessor.create();
    this.alloc = RpcResources.getByteBufAllocator();
    this.scheduler = scheduler;
    this.forceExecutionOffEventLoop = forceExecutionOffEventLoop;
    this.emitExceptionOnClose = onClose.then(Mono.error(ChannelNotActiveException.INSTANCE));

    NettyUtil.toMono(channel.closeFuture()).subscribe(onClose);
  }

  @Override
  public Mono<Void> onClose() {
    return onClose;
  }

  @Override
  public void dispose() {
    channel.close();
  }

  @Override
  public boolean isDisposed() {
    return onClose.isDisposed();
  }

  private <T> Mono<T> forceExecutionOffLoopIfNecessary(Mono<T> mono) {
    return forceExecutionOffEventLoop ? mono.publishOn(scheduler) : mono;
  }

  @SuppressWarnings("unchecked")
  private <T> Mono<T> emitExceptionOnClose() {
    return (Mono<T>) emitExceptionOnClose;
  }

  private <T> Mono<T> emitExceptionOnClose(Mono<T> mono) {
    return Mono.first(mono, emitExceptionOnClose());
  }

  @Override
  public <T> Mono<ClientResponsePayload<T>> singleRequestSingleResponse(
      final ClientRequestPayload<T> payload, final RpcOptions options) {

    ByteBuf encodedRequest = null;
    try {
      final MonoProcessor<ClientResponsePayload<T>> processor = MonoProcessor.create();
      final int sequenceId = this.sequenceId.getAndIncrement();
      encodedRequest = encodeRequest(alloc, payload, false, sequenceId);
      final RequestContext<T, ClientResponsePayload<T>> context =
          new RequestContext<>(processor, payload, encodedRequest, options, false, sequenceId);

      if (!channel.isActive()) {
        return Mono.error(ChannelNotActiveException.INSTANCE);
      }

      Mono<ClientResponsePayload<T>> response =
          NettyUtil.toMono(channel.writeAndFlush(context)).then(processor);

      response = emitExceptionOnClose(response);

      return forceExecutionOffLoopIfNecessary(response);
    } catch (Throwable t) {
      if (encodedRequest != null && encodedRequest.refCnt() > 0) {
        encodedRequest.release();
      }

      return Mono.error(t);
    }
  }

  @Override
  public <T> Mono<Void> singleRequestNoResponse(
      ClientRequestPayload<T> payload, RpcOptions options) {
    ByteBuf encodedRequest = null;
    try {
      final MonoProcessor<Void> processor = MonoProcessor.create();
      encodedRequest = encodeRequest(alloc, payload, true, ONEWAY_SEQ_IQ);
      final RequestContext<T, Void> context =
          new RequestContext<>(processor, payload, encodedRequest, options, true, ONEWAY_SEQ_IQ);

      if (!channel.isActive()) {
        return Mono.error(ChannelNotActiveException.INSTANCE);
      }

      Mono<Void> response = NettyUtil.toMono(channel.writeAndFlush(context));

      response = emitExceptionOnClose(response);

      return forceExecutionOffLoopIfNecessary(response);
    } catch (Throwable t) {
      if (encodedRequest != null && encodedRequest.refCnt() > 0) {
        encodedRequest.release();
      }

      return Mono.error(t);
    }
  }

  ByteBuf encodeRequest(
      final ByteBufAllocator allocator,
      final ClientRequestPayload requestPayload,
      boolean oneway,
      final int sequenceId)
      throws Exception {
    final ByteBuf request = allocator.buffer();
    try {
      final ProtocolId protocol = requestPayload.getRequestRpcMetadata().getProtocol();
      final TProtocolType protocolType = TProtocolType.fromProtocolId(protocol);
      final TProtocol out = protocolType.apply(request);

      final String name = requestPayload.getRequestRpcMetadata().getName();

      out.writeMessageBegin(new TMessage(name, oneway ? ONEWAY : CALL, sequenceId));

      out.writeStructBegin(PARAMETERS_STRUCT);

      final Writer writer = requestPayload.getDataWriter();

      writer.write(out);

      out.writeFieldStop();
      out.writeStructEnd();
      out.writeMessageEnd();

      return request;
    } catch (Throwable throwable) {
      ReferenceCountUtil.safeRelease(request);
      throw throwable;
    }
  }
}
