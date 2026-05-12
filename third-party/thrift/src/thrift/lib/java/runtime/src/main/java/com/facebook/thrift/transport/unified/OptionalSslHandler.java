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

package com.facebook.thrift.transport.unified;

import io.netty.buffer.ByteBuf;
import io.netty.channel.ChannelHandlerContext;
import io.netty.handler.codec.ByteToMessageDecoder;
import io.netty.handler.ssl.SslContext;
import io.netty.handler.ssl.SslHandler;
import io.netty.util.ReferenceCountUtil;
import java.util.List;
import java.util.concurrent.ScheduledFuture;
import java.util.concurrent.TimeUnit;

/**
 * Inspects the first 5 bytes of an inbound connection to decide whether it is TLS or plaintext.
 *
 * <ul>
 *   <li>If the bytes match a TLS record header, install an {@link SslHandler} immediately after
 *       this handler so that the cumulation buffer flushed at {@code handlerRemoved} flows through
 *       it. The SSL handshake is then driven by the SslHandler, which fires {@code
 *       SslHandshakeCompletionEvent} upstream when complete.
 *   <li>Otherwise, fire {@link PlaintextConfirmedEvent#INSTANCE} as a user event so that a
 *       downstream {@link DeferChannelActiveHandler} can release any deferred {@code channelActive}
 *       on the plaintext branch.
 * </ul>
 *
 * <p>This handler always removes itself once the decision is made. The buffered bytes are then
 * forwarded to the next handler in the pipeline — which is either the freshly installed {@link
 * SslHandler} (TLS branch) or the existing downstream handler (plaintext branch).
 *
 * <p>The defaults baked into {@link SslHandler} (10s handshake timeout, 3s close-notify flush
 * timeout, 0s close-notify read timeout) are kept; no configuration is applied here.
 *
 * <p>A peek-phase timeout closes the channel if the first 5 bytes do not arrive within {@link
 * #PEEK_TIMEOUT_MILLIS}. Without it, a client that connects but never sends bytes would consume a
 * server channel indefinitely (the SslHandler handshake timeout cannot start until the SslHandler
 * is in the pipeline). The value matches {@code SslHandler.handshakeTimeoutMillis}'s default so the
 * worst-case wall time before a slow/silent client is reaped is the same on both branches.
 */
final class OptionalSslHandler extends ByteToMessageDecoder {
  private static final int SSL_RECORD_HEADER_LENGTH = 5;

  /**
   * Mirrors {@code SslHandler}'s default {@code handshakeTimeoutMillis} so the worst-case wall time
   * for a slow client is the same on the TLS and plaintext branches.
   */
  static final long PEEK_TIMEOUT_MILLIS = 10_000L;

  private final SslContext sslContext;
  private final long peekTimeoutMillis;
  private ScheduledFuture<?> timeoutFuture;

  OptionalSslHandler(SslContext sslContext) {
    this(sslContext, PEEK_TIMEOUT_MILLIS);
  }

  /** Visible for testing. Production callers should use the single-arg constructor. */
  OptionalSslHandler(SslContext sslContext, long peekTimeoutMillis) {
    this.sslContext = sslContext;
    this.peekTimeoutMillis = peekTimeoutMillis;
  }

  @Override
  public void handlerAdded(ChannelHandlerContext ctx) throws Exception {
    super.handlerAdded(ctx);
    if (ctx.channel().isActive()) {
      startTimeout(ctx);
    }
  }

  @Override
  public void channelActive(ChannelHandlerContext ctx) throws Exception {
    startTimeout(ctx);
    super.channelActive(ctx);
  }

  @Override
  protected void decode(ChannelHandlerContext ctx, ByteBuf in, List<Object> out) {
    if (in.readableBytes() < SSL_RECORD_HEADER_LENGTH) {
      return;
    }

    cancelTimeout();

    if (SslHandler.isEncrypted(in, false)) {
      installSslHandler(ctx);
    } else {
      ctx.fireUserEventTriggered(PlaintextConfirmedEvent.INSTANCE);
    }
    ctx.pipeline().remove(this);
  }

  /**
   * Installs a freshly-allocated {@link SslHandler} after this handler. If {@code addAfter} throws
   * after the handler is constructed, releases the engine to avoid leaking native memory on
   * OpenSSL-backed contexts.
   */
  private void installSslHandler(ChannelHandlerContext ctx) {
    SslHandler sslHandler = sslContext.newHandler(ctx.alloc());
    try {
      ctx.pipeline().addAfter(ctx.name(), "sslHandler", sslHandler);
      sslHandler = null;
    } finally {
      if (sslHandler != null) {
        ReferenceCountUtil.safeRelease(sslHandler.engine());
      }
    }
  }

  @Override
  protected void handlerRemoved0(ChannelHandlerContext ctx) throws Exception {
    cancelTimeout();
    super.handlerRemoved0(ctx);
  }

  private void startTimeout(ChannelHandlerContext ctx) {
    if (timeoutFuture != null) {
      return;
    }
    timeoutFuture =
        ctx.executor()
            .schedule(
                () -> {
                  if (ctx.channel().isActive()) {
                    ctx.close();
                  }
                },
                peekTimeoutMillis,
                TimeUnit.MILLISECONDS);
  }

  private void cancelTimeout() {
    if (timeoutFuture != null) {
      timeoutFuture.cancel(false);
      timeoutFuture = null;
    }
  }

  @Override
  public boolean isSharable() {
    return false;
  }
}
