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

package com.facebook.thrift.legacy.codec;

import static com.google.common.base.Preconditions.checkArgument;
import static java.lang.Math.min;
import static java.lang.Math.toIntExact;
import static java.util.Objects.requireNonNull;

import com.facebook.thrift.legacy.exceptions.FrameTooLargeException;
import io.netty.buffer.ByteBuf;
import io.netty.channel.ChannelHandlerContext;
import io.netty.handler.codec.ByteToMessageDecoder;
import java.util.List;
import java.util.Optional;

public class ThriftFramedDecoder extends ByteToMessageDecoder {
  private final FrameInfoDecoder frameInfoDecoder;
  private final int maxFrameSizeInBytes;

  private Optional<FrameInfo> tooLongFrameInfo = Optional.empty();
  private long tooLongFrameSizeInBytes;
  private long bytesToDiscard;

  public ThriftFramedDecoder(FrameInfoDecoder frameInfoDecoder, int maxFrameSizeInBytes) {
    this.frameInfoDecoder = requireNonNull(frameInfoDecoder, "sequenceIdDecoder is null");
    checkArgument(maxFrameSizeInBytes >= 0, "maxFrameSizeInBytes");
    this.maxFrameSizeInBytes = maxFrameSizeInBytes;
  }

  @Override
  protected void decode(ChannelHandlerContext context, ByteBuf buffer, List<Object> output) {
    decode(buffer).ifPresent(output::add);
  }

  private Optional<ByteBuf> decode(ByteBuf buffer) {
    if (bytesToDiscard > 0) {
      discardTooLongFrame(buffer);
      return Optional.empty();
    }

    int initialReaderIndex = buffer.readerIndex();

    if (buffer.readableBytes() < Integer.BYTES) {
      return Optional.empty();
    }
    long frameSizeInBytes = buffer.readUnsignedInt();

    if (frameSizeInBytes > maxFrameSizeInBytes) {
      // this invocation doesn't move the readerIndex
      Optional<FrameInfo> frameInfo = frameInfoDecoder.tryDecodeFrameInfo(buffer);
      if (frameInfo.isPresent()) {
        tooLongFrameInfo = frameInfo;
        tooLongFrameSizeInBytes = frameSizeInBytes;
        bytesToDiscard = frameSizeInBytes;
        discardTooLongFrame(buffer);
        return Optional.empty();
      }
      // Basic frame info cannot be decoded and the max frame size is already exceeded.
      // Instead of waiting forever, fail without providing the sequence ID.
      if (buffer.readableBytes() >= maxFrameSizeInBytes) {
        tooLongFrameInfo = Optional.empty();
        tooLongFrameSizeInBytes = frameSizeInBytes;
        bytesToDiscard = frameSizeInBytes;
        discardTooLongFrame(buffer);
        return Optional.empty();
      }
      buffer.readerIndex(initialReaderIndex);
      return Optional.empty();
    }

    if (buffer.readableBytes() >= frameSizeInBytes) {
      // toIntExact must be safe, as frameSizeInBytes <= maxFrameSize
      ByteBuf frame = buffer.retainedSlice(buffer.readerIndex(), toIntExact(frameSizeInBytes));
      buffer.readerIndex(buffer.readerIndex() + toIntExact(frameSizeInBytes));
      return Optional.of(frame);
    }

    buffer.readerIndex(initialReaderIndex);
    return Optional.empty();
  }

  private void discardTooLongFrame(ByteBuf buffer) {
    // readableBytes returns int, toIntExact must be safe
    int bytesToSkip = toIntExact(min(bytesToDiscard, buffer.readableBytes()));
    buffer.skipBytes(bytesToSkip);
    bytesToDiscard -= bytesToSkip;

    if (bytesToDiscard == 0) {
      RuntimeException exception =
          new FrameTooLargeException(
              tooLongFrameInfo, tooLongFrameSizeInBytes, maxFrameSizeInBytes);
      tooLongFrameInfo = Optional.empty();
      tooLongFrameSizeInBytes = 0;
      throw exception;
    }
  }
}
