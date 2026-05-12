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

import io.netty.channel.ChannelHandlerContext;
import io.netty.channel.ChannelInboundHandlerAdapter;
import io.netty.handler.ssl.SslHandshakeCompletionEvent;

/**
 * Suppresses the original {@code channelActive} event and re-fires it once one of the following
 * arrives:
 *
 * <ul>
 *   <li>{@link SslHandshakeCompletionEvent} with {@code isSuccess() == true} — TLS branch, fired by
 *       the {@code SslHandler} that {@link OptionalSslHandler} installed.
 *   <li>{@link PlaintextConfirmedEvent#INSTANCE} — plaintext branch, fired by {@link
 *       OptionalSslHandler} when the first 5 bytes are not a TLS record header.
 * </ul>
 *
 * <p>Mirrors reactor-netty's internal {@code SslReadHandler} but works on either branch, so
 * downstream user callbacks (notably reactor-netty's {@code doOnConnection}) fire only once we know
 * which protocol path the connection is on.
 *
 * <p>If the SSL handshake fails, the cause is propagated via {@code fireExceptionCaught} and the
 * channel is closed by the SslHandler — {@code channelActive} is never fired downstream.
 */
final class DeferChannelActiveHandler extends ChannelInboundHandlerAdapter {

  private boolean fired;

  @Override
  public void channelActive(ChannelHandlerContext ctx) {
    // Begin reading so OptionalSslHandler can decide. Do not propagate channelActive yet —
    // we will re-fire it once the protocol branch is known.
    ctx.read();
  }

  @Override
  public void channelReadComplete(ChannelHandlerContext ctx) {
    if (!fired) {
      ctx.read();
    }
    ctx.fireChannelReadComplete();
  }

  @Override
  public void userEventTriggered(ChannelHandlerContext ctx, Object evt) {
    if (!fired) {
      if (evt instanceof SslHandshakeCompletionEvent) {
        SslHandshakeCompletionEvent handshake = (SslHandshakeCompletionEvent) evt;
        if (handshake.isSuccess()) {
          fired = true;
          ctx.pipeline().remove(this);
          ctx.fireChannelActive();
        } else {
          ctx.fireExceptionCaught(handshake.cause());
        }
      } else if (evt == PlaintextConfirmedEvent.INSTANCE) {
        fired = true;
        ctx.pipeline().remove(this);
        ctx.fireChannelActive();
      }
    }
    ctx.fireUserEventTriggered(evt);
  }

  @Override
  public boolean isSharable() {
    return false;
  }
}
