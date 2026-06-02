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

import com.facebook.thrift.legacy.codec.LegacyTransportType;
import com.facebook.thrift.legacy.codec.ThriftFrame;
import com.facebook.thrift.protocol.TProtocolType;
import io.netty.buffer.ByteBuf;
import io.netty.buffer.Unpooled;
import io.netty.channel.embedded.EmbeddedChannel;
import java.util.Collections;
import org.junit.jupiter.api.Test;

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
}
