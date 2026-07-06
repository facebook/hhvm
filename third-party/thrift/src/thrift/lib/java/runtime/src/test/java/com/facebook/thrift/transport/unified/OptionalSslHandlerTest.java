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

import static org.junit.jupiter.api.Assertions.assertFalse;
import static org.junit.jupiter.api.Assertions.assertNotNull;
import static org.junit.jupiter.api.Assertions.assertNull;
import static org.junit.jupiter.api.Assertions.assertSame;
import static org.junit.jupiter.api.Assertions.assertTrue;

import io.netty.buffer.Unpooled;
import io.netty.channel.ChannelHandlerContext;
import io.netty.channel.ChannelInboundHandlerAdapter;
import io.netty.channel.embedded.EmbeddedChannel;
import io.netty.handler.ssl.SslContext;
import io.netty.handler.ssl.SslContextBuilder;
import io.netty.handler.ssl.SslHandler;
import io.netty.handler.ssl.util.SelfSignedCertificate;
import java.util.concurrent.TimeUnit;
import org.junit.jupiter.api.AfterAll;
import org.junit.jupiter.api.BeforeAll;
import org.junit.jupiter.api.Test;

/** Unit tests for {@link OptionalSslHandler}. */
public class OptionalSslHandlerTest {

  private static SelfSignedCertificate cert;
  private static SslContext sslContext;

  @BeforeAll
  public static void setUpClass() throws Exception {
    cert = new SelfSignedCertificate();
    sslContext = SslContextBuilder.forServer(cert.certificate(), cert.privateKey()).build();
  }

  @AfterAll
  public static void tearDownClass() {
    if (cert != null) {
      cert.delete();
    }
  }

  /**
   * Plaintext path: when the first 5 bytes are not a TLS record header, the peeker fires {@link
   * PlaintextConfirmedEvent#INSTANCE} as a user event and removes itself from the pipeline.
   */
  @Test
  public void firesPlaintextConfirmedEventOnNonTlsBytes() {
    EmbeddedChannel channel = new EmbeddedChannel(new OptionalSslHandler(sslContext));
    UserEventCapture capture = new UserEventCapture();
    channel.pipeline().addLast(capture);

    // 5 bytes that do not match a TLS record header (record type byte != 0x14..0x17).
    channel.writeInbound(Unpooled.wrappedBuffer(new byte[] {0x00, 0x00, 0x00, 0x00, 0x05}));

    assertSame(PlaintextConfirmedEvent.INSTANCE, capture.lastEvent);
    assertNull(channel.pipeline().get(OptionalSslHandler.class), "peeker should remove itself");
    assertNull(channel.pipeline().get(SslHandler.class), "no SslHandler on plaintext branch");
    channel.finishAndReleaseAll();
  }

  /**
   * TLS path: when the first 5 bytes are a TLS record header, the peeker installs an SslHandler
   * after itself and removes itself.
   */
  @Test
  public void installsSslHandlerOnTlsBytes() {
    EmbeddedChannel channel = new EmbeddedChannel(new OptionalSslHandler(sslContext));

    // TLS 1.2 ClientHello prefix: record type 0x16 (handshake), version 0x0303, length 0x00FF
    // (255 bytes of body to follow, which we don't send — so SslHandler waits and stays in the
    // pipeline rather than trying to process an empty record).
    channel.writeInbound(Unpooled.wrappedBuffer(new byte[] {0x16, 0x03, 0x03, 0x00, (byte) 0xFF}));

    assertNull(channel.pipeline().get(OptionalSslHandler.class), "peeker should remove itself");
    assertNotNull(channel.pipeline().get(SslHandler.class), "SslHandler should be in the pipeline");
    channel.finishAndReleaseAll();
  }

  /** Insufficient bytes do not trigger a decision; the peeker remains in place. */
  @Test
  public void waitsForFiveBytes() {
    EmbeddedChannel channel = new EmbeddedChannel(new OptionalSslHandler(sslContext));
    UserEventCapture capture = new UserEventCapture();
    channel.pipeline().addLast(capture);

    channel.writeInbound(Unpooled.wrappedBuffer(new byte[] {0x16, 0x03}));

    assertNull(capture.lastEvent, "no decision yet");
    assertNotNull(
        channel.pipeline().get(OptionalSslHandler.class), "peeker should still be in the pipeline");
    channel.finishAndReleaseAll();
  }

  /**
   * Peek-phase timeout: if no bytes arrive within the timeout, the peeker closes the channel.
   * Without this protection, a client that connects but never sends data would consume a server
   * channel indefinitely (no SslHandler is in the pipeline yet, so its handshake timeout cannot
   * apply).
   */
  @Test
  public void timeoutClosesIdleChannel() {
    EmbeddedChannel channel =
        new EmbeddedChannel(
            new OptionalSslHandler(sslContext, OptionalSslHandler.PEEK_TIMEOUT_MILLIS));

    assertTrue(channel.isOpen(), "channel should be open before timeout");

    channel.advanceTimeBy(OptionalSslHandler.PEEK_TIMEOUT_MILLIS, TimeUnit.MILLISECONDS);
    channel.runScheduledPendingTasks();

    assertFalse(channel.isOpen(), "channel should have been closed by the peek-phase timeout");
  }

  /** The timeout is cancelled once the peeker decides on a branch. */
  @Test
  public void timeoutDoesNotCloseAfterDecision() {
    EmbeddedChannel channel =
        new EmbeddedChannel(
            new OptionalSslHandler(sslContext, OptionalSslHandler.PEEK_TIMEOUT_MILLIS));

    // Send TLS bytes immediately so the peeker decides before the timeout would fire.
    channel.writeInbound(Unpooled.wrappedBuffer(new byte[] {0x16, 0x03, 0x03, 0x00, 0x00}));

    channel.advanceTimeBy(OptionalSslHandler.PEEK_TIMEOUT_MILLIS, TimeUnit.MILLISECONDS);
    channel.runScheduledPendingTasks();

    assertTrue(channel.isOpen(), "channel must remain open after a successful peek decision");
    channel.finishAndReleaseAll();
  }

  private static final class UserEventCapture extends ChannelInboundHandlerAdapter {
    Object lastEvent;

    @Override
    public void userEventTriggered(ChannelHandlerContext ctx, Object evt) {
      lastEvent = evt;
      ctx.fireUserEventTriggered(evt);
    }
  }
}
