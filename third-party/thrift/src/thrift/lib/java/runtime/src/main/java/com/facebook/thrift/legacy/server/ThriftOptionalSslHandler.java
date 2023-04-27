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

import static com.facebook.swift.service.SwiftConstants.THRIFT_SSL_SESSION_KEY;

import com.facebook.thrift.util.RpcServerUtils;
import io.netty.buffer.ByteBuf;
import io.netty.channel.ChannelHandlerContext;
import io.netty.channel.ChannelPipeline;
import io.netty.handler.codec.ByteToMessageDecoder;
import io.netty.handler.ssl.SslContext;
import io.netty.handler.ssl.SslHandler;
import java.util.List;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

public class ThriftOptionalSslHandler extends ByteToMessageDecoder {

  private static final Logger LOGGER = LoggerFactory.getLogger(ThriftOptionalSslHandler.class);

  private static final int SSL_RECORD_HEADER_LENGTH =
      Byte.BYTES
          + // content type
          Byte.BYTES
          + // major version
          Byte.BYTES
          + // minor version
          Short.BYTES; // length

  private final SslContext sslContext;

  public ThriftOptionalSslHandler(SslContext sslContext) {
    this.sslContext = sslContext;
  }

  @Override
  protected void decode(ChannelHandlerContext context, ByteBuf in, List<Object> out) {
    // minimum bytes required to detect ssl
    if (in.readableBytes() < SSL_RECORD_HEADER_LENGTH) {
      return;
    }

    ChannelPipeline pipeline = context.pipeline();
    if (SslHandler.isEncrypted(in)) {
      SslHandler sslHandler = sslContext.newHandler(context.alloc());
      sslHandler
          .handshakeFuture()
          .addListener(
              future ->
                  context
                      .channel()
                      .attr(THRIFT_SSL_SESSION_KEY)
                      .set(RpcServerUtils.getThriftSslSession(sslHandler)));
      pipeline.replace(this, "ssl", sslHandler);
    } else {
      pipeline.remove(this);
    }
  }
}
