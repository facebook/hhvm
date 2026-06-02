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

package com.facebook.thrift.legacy.server;

import static org.assertj.core.api.Assertions.assertThat;

import com.facebook.thrift.legacy.codec.LegacyTransportType;
import com.facebook.thrift.legacy.codec.ThriftFrame;
import com.facebook.thrift.payload.ServerRequestPayload;
import com.facebook.thrift.payload.ServerResponsePayload;
import com.facebook.thrift.protocol.TProtocolType;
import com.facebook.thrift.server.RpcServerHandler;
import io.airlift.units.Duration;
import io.netty.buffer.ByteBuf;
import io.netty.buffer.Unpooled;
import io.netty.channel.embedded.EmbeddedChannel;
import java.util.Collections;
import java.util.concurrent.TimeUnit;
import org.apache.thrift.protocol.TMessage;
import org.apache.thrift.protocol.TMessageType;
import org.apache.thrift.protocol.TProtocol;
import org.junit.jupiter.api.Test;
import reactor.core.publisher.Mono;

class ThriftServerHandlerTest {

  /**
   * If request dispatch fails synchronously (here, the handler throws from {@code
   * singleRequestSingleResponse} while the response Mono is being assembled), the decoded frame
   * must still be released. Before the fix, the throw escaped {@code messageReceived} before the
   * reactive {@code doFinally} was engaged, leaking the underlying direct buffer.
   */
  @Test
  void releasesFrameWhenDispatchThrowsSynchronously() {
    RpcServerHandler throwingHandler =
        new RpcServerHandler() {
          @Override
          public Mono<ServerResponsePayload> singleRequestSingleResponse(
              ServerRequestPayload payload) {
            throw new RuntimeException("synchronous dispatch failure");
          }
        };

    EmbeddedChannel channel =
        new EmbeddedChannel(
            new ThriftServerHandler(throwingHandler, new Duration(60, TimeUnit.SECONDS)));

    ByteBuf message = Unpooled.buffer();
    TProtocol out = TProtocolType.TCompact.apply(message);
    // seqId != -1 -> SINGLE_REQUEST_SINGLE_RESPONSE; decodeMessage only reads the message begin.
    out.writeMessageBegin(new TMessage("doStuff", TMessageType.CALL, 1));

    ThriftFrame frame =
        new ThriftFrame(
            1,
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
