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

import static java.lang.String.format;
import static org.apache.thrift.TApplicationException.BAD_SEQUENCE_ID;
import static org.apache.thrift.TApplicationException.INVALID_MESSAGE_TYPE;
import static org.apache.thrift.TApplicationException.MISSING_RESULT;
import static org.apache.thrift.TApplicationException.WRONG_METHOD_NAME;
import static org.apache.thrift.protocol.TMessageType.EXCEPTION;
import static org.apache.thrift.protocol.TMessageType.REPLY;

import com.facebook.thrift.exceptions.MessageTooLargeException;
import com.facebook.thrift.legacy.codec.FrameInfo;
import com.facebook.thrift.legacy.codec.LegacyTransportType;
import com.facebook.thrift.legacy.codec.ThriftFrame;
import com.facebook.thrift.legacy.exceptions.FrameTooLargeException;
import com.facebook.thrift.payload.ClientRequestPayload;
import com.facebook.thrift.payload.ClientResponsePayload;
import com.facebook.thrift.payload.Reader;
import com.facebook.thrift.protocol.TProtocolType;
import io.netty.buffer.ByteBuf;
import io.netty.buffer.ByteBufUtil;
import io.netty.channel.ChannelDuplexHandler;
import io.netty.channel.ChannelHandlerContext;
import io.netty.channel.ChannelPromise;
import io.netty.channel.VoidChannelPromise;
import io.netty.util.ReferenceCountUtil;
import io.netty.util.collection.IntObjectHashMap;
import io.netty.util.collection.IntObjectMap;
import java.nio.channels.ClosedChannelException;
import java.util.Map;
import java.util.Optional;
import org.apache.thrift.ResponseRpcMetadata;
import org.apache.thrift.TApplicationException;
import org.apache.thrift.TException;
import org.apache.thrift.protocol.TField;
import org.apache.thrift.protocol.TMessage;
import org.apache.thrift.protocol.TProtocol;
import org.apache.thrift.protocol.TProtocolUtil;
import org.apache.thrift.transport.TTransportException;
import reactor.core.publisher.MonoProcessor;

@SuppressWarnings("rawtypes")
public final class ThriftClientHandler extends ChannelDuplexHandler {
  private final IntObjectMap<RequestContext<?, ?>> requestContexts;
  private final LegacyTransportType transport;

  public ThriftClientHandler(LegacyTransportType transport) {
    this.transport = transport;
    this.requestContexts = new IntObjectHashMap<>();
  }

  @Override
  public void write(
      final ChannelHandlerContext ctx, final Object msg, final ChannelPromise promise) {
    if (msg instanceof RequestContext) {
      final RequestContext<?, ?> requestContext = (RequestContext) msg;
      final int sequenceId = requestContext.getSequenceId();
      final ThriftFrame frame = encodeThriftFrame(requestContext, sequenceId);

      try {
        requestContexts.put(requestContext.getSequenceId(), requestContext);

        final ChannelPromise p;

        if (requestContext.isOneway()) {
          p = promise instanceof VoidChannelPromise ? ctx.newPromise() : promise;

          p.addListener(
              __ -> {
                final RequestContext remove = requestContexts.remove(sequenceId);

                if (remove != null) {
                  if (__.cause() != null) {
                    remove.getProcessor().onError(__.cause());
                  } else {
                    remove.getProcessor().onComplete();
                  }
                }
              });

        } else {
          p = promise;
        }

        ctx.writeAndFlush(frame, p);
      } catch (Throwable t) {
        ReferenceCountUtil.safeRelease(frame);
        requestContext.getProcessor().onError(t);
      }
    } else {
      ctx.writeAndFlush(msg, promise);
    }
  }

  private ThriftFrame encodeThriftFrame(
      final RequestContext<?, ?> requestContext, final int sequenceId) {

    ClientRequestPayload<?> requestPayload = requestContext.getPayload();

    return new ThriftFrame(
        sequenceId,
        requestContext.getEncodedRequest(),
        requestPayload.getRequestRpcMetadata().getOtherMetadata(),
        requestPayload.getPersistentHeaders(),
        transport,
        TProtocolType.fromProtocolId(requestPayload.getRequestRpcMetadata().getProtocol()),
        true);
  }

  @Override
  @SuppressWarnings({"ResultOfMethodCallIgnored"})
  public void channelRead(ChannelHandlerContext ctx, Object msg) {
    if (msg instanceof ThriftFrame) {
      final ThriftFrame frame = (ThriftFrame) msg;
      final int sequenceId = frame.getSequenceId();
      final RequestContext<?, ?> requestContext = requestContexts.remove(sequenceId);
      final ClientRequestPayload payload = requestContext.getPayload();
      final MonoProcessor processor = requestContext.getProcessor();

      processor
          .doFinally(
              __ -> {
                ctx.fireChannelReadComplete();
                frame.release();
              })
          .subscribe();

      try {
        final ClientResponsePayload responsePayload =
            decodeResponse(
                payload.getRequestRpcMetadata().getName(),
                frame,
                payload.getResponseReader(),
                payload.getExceptionReaders(),
                sequenceId);
        processor.onNext(responsePayload);
      } catch (Throwable t) {
        processor.onError(t);
      }
    } else {
      ctx.fireChannelRead(msg);
    }
  }

  ClientResponsePayload decodeResponse(
      final String expectedName,
      final ThriftFrame frame,
      final Reader reader,
      final Map<Short, Reader> exceptionReaders,
      final int sequenceId)
      throws TApplicationException {
    final ByteBuf responseBytes = frame.getMessage();

    final TProtocol in = frame.getProtocol().apply(responseBytes);
    final TMessage message = in.readMessageBegin();

    if (message.type == EXCEPTION) {
      Exception response = readTApplicationException(in);
      in.readMessageEnd();
      return ClientResponsePayload.createException(
          response,
          new ResponseRpcMetadata.Builder().setOtherMetadata(frame.getHeaders()).build(),
          null,
          false);
    }

    if (message.type != REPLY) {
      throw new TApplicationException(
          INVALID_MESSAGE_TYPE,
          format("Received invalid message type %s from server", message.type));
    }

    if (!expectedName.equals(message.name)) {
      throw new TApplicationException(
          WRONG_METHOD_NAME,
          format(
              "Wrong method name in reply: expected %s but received %s",
              expectedName, message.name));
    }

    if (message.seqid != sequenceId) {
      throw new TApplicationException(
          BAD_SEQUENCE_ID, format("%s failed: out of sequence response", expectedName));
    }

    Object response = null;
    TApplicationException applicationException = null;
    in.readStructBegin();
    TField field = in.readFieldBegin();

    if (field.id == 0) {
      response = reader.read(in);
    } else {
      Reader exceptionReader = exceptionReaders.get(field.id);
      if (exceptionReader != null) {
        response = exceptionReader.read(in);
      } else {
        TProtocolUtil.skip(in, field.type);
        applicationException =
            new TApplicationException(
                MISSING_RESULT,
                expectedName
                    + " failed with unknown result: "
                    + ByteBufUtil.prettyHexDump(responseBytes));
      }
    }

    in.readFieldEnd();
    in.readStructEnd();
    in.readMessageEnd();

    if (applicationException != null) {
      throw applicationException;
    }

    if (response instanceof Exception) {
      return ClientResponsePayload.createException(
          (Exception) response,
          new ResponseRpcMetadata.Builder().setOtherMetadata(frame.getHeaders()).build(),
          null,
          false);
    }

    return ClientResponsePayload.createResult(
        response,
        new ResponseRpcMetadata.Builder().setOtherMetadata(frame.getHeaders()).build(),
        null,
        false);
  }

  TApplicationException readTApplicationException(TProtocol protocol) throws TException {
    try {
      return TApplicationException.read(protocol);
    } catch (TException e) {
      throw e;
    } catch (Exception e) {
      throw new TException(e);
    }
  }

  @Override
  public void exceptionCaught(final ChannelHandlerContext ctx, final Throwable cause) {
    if (cause instanceof FrameTooLargeException) {
      onFrameTooLargeException(ctx, (FrameTooLargeException) cause);
    } else {
      onError(ctx, cause);
    }
  }

  private void onFrameTooLargeException(
      final ChannelHandlerContext ctx, final FrameTooLargeException exception) {
    final TException thriftException =
        new MessageTooLargeException(exception.getMessage(), exception);

    Optional<FrameInfo> frameInfo = exception.getFrameInfo();
    if (frameInfo.isPresent()) {
      final int sequenceId = frameInfo.get().getSequenceId();
      RequestContext requestContext = requestContexts.remove(sequenceId);
      if (requestContext != null) {
        requestContext.getProcessor().onError(thriftException);
        return;
      }
    }

    onError(ctx, exception);
  }

  private void onError(final ChannelHandlerContext ctx, final Throwable throwable) {

    try {
      final TException thriftException;
      if (throwable instanceof TException) {
        thriftException = (TException) throwable;
      } else {
        thriftException = new TTransportException(throwable);
      }

      clearWithException(thriftException);
    } finally {
      ctx.close();
    }
  }

  private void clearWithException(Throwable t) {
    try {
      for (RequestContext requestContext : requestContexts.values()) {
        final MonoProcessor processor = requestContext.getProcessor();
        if (processor != null) {
          processor.onError(t);
        }
      }
    } finally {
      requestContexts.clear();
    }
  }

  @Override
  public void channelInactive(final ChannelHandlerContext ctx) {
    clearWithException(new ClosedChannelException());
  }
}
