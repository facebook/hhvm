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

import static org.assertj.core.api.Assertions.assertThat;
import static org.mockito.Mockito.mock;
import static org.mockito.Mockito.when;

import com.facebook.thrift.legacy.codec.LegacyTransportType;
import com.facebook.thrift.legacy.codec.ThriftFrame;
import com.facebook.thrift.payload.ClientRequestPayload;
import com.facebook.thrift.protocol.TProtocolType;
import io.netty.buffer.ByteBuf;
import io.netty.buffer.Unpooled;
import io.netty.channel.embedded.EmbeddedChannel;
import java.util.Collections;
import org.junit.jupiter.api.Test;
import reactor.core.publisher.Sinks;

class ThriftClientHandlerTest {

  /**
   * A response whose sequenceId matches no in-flight request (a duplicate, late, or otherwise
   * unexpected response) must not leak its decoded frame. Before the fix, {@code
   * requestContexts.remove(sequenceId)} returned null and the handler dereferenced it (NPE) before
   * wiring the {@code doFinally(... frame.release())}, leaking the underlying direct buffer.
   */
  @Test
  void releasesFrameWhenNoRequestContextMatchesSequenceId() {
    EmbeddedChannel channel =
        new EmbeddedChannel(new ThriftClientHandler(LegacyTransportType.HEADER));

    ByteBuf message = Unpooled.buffer().writeZero(8);
    ThriftFrame frame =
        new ThriftFrame(
            4242, // sequenceId with no matching in-flight request
            message,
            Collections.emptyMap(),
            Collections.emptyMap(),
            LegacyTransportType.HEADER,
            TProtocolType.TCompact,
            false);

    channel.writeInbound(frame);

    assertThat(message.refCnt()).isZero();

    channel.finishAndReleaseAll();
  }

  /**
   * A request written after the channel closed never reaches {@link ThriftClientHandler} -- the
   * pipeline has been torn down, so the {@link RequestContext} travels straight to head, which
   * disposes of it with {@code ReferenceCountUtil.release(msg)}. That is a no-op unless the context
   * itself is reference counted, which leaked the request buffer allocated by {@code
   * LegacyRpcClient.encodeRequest}.
   */
  @Test
  void releasesEncodedRequestWhenChannelClosedBeforeWrite() {
    EmbeddedChannel channel =
        new EmbeddedChannel(new ThriftClientHandler(LegacyTransportType.HEADER));
    channel.close().syncUninterruptibly();
    channel.runPendingTasks();

    ByteBuf encodedRequest = Unpooled.buffer().writeZero(8);
    channel.writeAndFlush(newRequestContext(mock(ClientRequestPayload.class), encodedRequest));

    assertThat(encodedRequest.refCnt()).isZero();
  }

  /**
   * Encoding the frame reads from the payload and can throw. The request buffer is owned by the
   * context until the frame takes it over, so a failure before that hand-off must release the
   * context.
   */
  @Test
  void releasesEncodedRequestWhenFrameEncodingFails() {
    EmbeddedChannel channel =
        new EmbeddedChannel(new ThriftClientHandler(LegacyTransportType.HEADER));

    ClientRequestPayload<?> payload = mock(ClientRequestPayload.class);
    when(payload.getRequestRpcMetadata()).thenThrow(new IllegalStateException("boom"));

    ByteBuf encodedRequest = Unpooled.buffer().writeZero(8);
    channel.writeAndFlush(newRequestContext(payload, encodedRequest));

    assertThat(encodedRequest.refCnt()).isZero();

    channel.finishAndReleaseAll();
  }

  @SuppressWarnings("unchecked")
  private static RequestContext<Object, Object> newRequestContext(
      ClientRequestPayload<?> payload, ByteBuf encodedRequest) {
    return new RequestContext<>(
        Sinks.one(), (ClientRequestPayload<Object>) payload, encodedRequest, null, false, 4243);
  }
}
