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

import io.netty.buffer.ByteBuf;
import io.netty.channel.ChannelHandlerContext;
import io.netty.handler.codec.ByteToMessageDecoder;
import io.netty.handler.codec.CorruptedFrameException;
import io.netty.handler.codec.TooLongFrameException;
import java.util.List;

/**
 * This is a THeader length based frame decoder adapted from Netty's LengthFieldBasedFrameDecoder,
 * that allows for variable length encoding of the length field. THeader protocol can support either
 * standard frames or big frames. The frame layout is as follows
 *
 * <pre>
 *   0 1 2 3 4 5 6 7 8 9 a b c d e f 0 1 2 3 4 5 6 7 8 9 a b c d e f
 * +----------------------------------------------------------------+
 * | 0|                         LENGTH32                            |
 * +----------------------------------------------------------------+
 * |                           LENGTH_MSW*                          |
 * +----------------------------------------------------------------+
 * |                           LENGTH_LSW*                          |
 * +----------------------------------------------------------------+
 * | 0|       HEADER MAGIC          |            FLAGS              |
 * +----------------------------------------------------------------+
 * |                              ...                               |
 * +----------------------------------------------------------------+
 * </pre>
 *
 * Length32 will contain a length for payloads up to the max sise of 0x3FFFFFFF, frame length
 * contains a length of all bytes after Length32 and optionally (Length_msw, Length_lws). If
 * Length32 contains the field BIG_FRAME_MAGIC, 0x42494746L, then the next 8 bytes should be
 * interpreted as the big endian length of the frame ( LENGTH_MSW << 8 + LENGTH_LSW )
 *
 * <p>This class ensures that we adjust the frame length variables based on whether the frame is
 * large frame or standard size frame. Once complete we strip the length bytes off of the frame and
 * return the frame to next handler.
 */
public class ThriftHeaderFrameLengthBasedDecoder extends ByteToMessageDecoder {
  private static final int MAX_FRAME_LENGTH = 0x3FFFFFFF + 4;
  private static final int MAX_BIG_FRAME_LENGTH = Integer.MAX_VALUE;
  private static final long BIG_FRAME_MAGIC = 0x42494746L;
  private int maxFrameLength;
  private int frameLengthInt = -1;
  private boolean discardingTooLongFrame;
  private long bytesToDiscard;
  private int lengthFieldSize;
  private long tooLongFrameLength;

  @Override
  protected void decode(ChannelHandlerContext ctx, ByteBuf in, List<Object> out) throws Exception {
    Object decoded = decode(ctx, in);
    if (decoded != null) {
      out.add(decoded);
    }
  }

  private Object decode(ChannelHandlerContext ctx, ByteBuf in) throws Exception {
    long frameLength = 0;
    if (frameLengthInt == -1) { // new frame

      if (discardingTooLongFrame) {
        discardingTooLongFrame(in);
      }

      // Before checking size we need at least 12 bytes read from the socket
      // Length + optional LENGTH_MSW/LENGTH_LSW
      if (in.readableBytes() < 12) {
        return null;
      }

      // Detect whether this is a standard frame or big frame
      // Standard frame: 4-byte LENGTH32 field
      // Big frame: 4-byte BIG_FRAME_MAGIC + 8-byte length (LENGTH_MSW + LENGTH_LSW)
      int index = in.readerIndex();
      frameLength = in.getUnsignedInt(index);
      lengthFieldSize = 4;
      maxFrameLength = MAX_FRAME_LENGTH;

      if (frameLength == BIG_FRAME_MAGIC) {
        // Big frame detected: read 8-byte length and adjust size to 12 bytes total
        frameLength = in.getLong(index + 4);
        lengthFieldSize = 12;
        maxFrameLength = MAX_BIG_FRAME_LENGTH;
      }

      if (frameLength < 0) {
        failOnNegativeLengthField(in, frameLength, lengthFieldSize);
      }

      // FrameLength is now == Total Frame Len including Len bytes
      frameLength += lengthFieldSize;

      if (frameLength < lengthFieldSize) {
        failOnFrameLengthLessThanLengthFieldEndOffset(in, frameLength, lengthFieldSize);
      }

      if (frameLength > maxFrameLength) {
        exceededFrameLength(in, frameLength);
        return null;
      }
      // never overflows because it's less than maxFrameLength
      frameLengthInt = (int) frameLength;
    }

    // continue to read while buf is less than frame length
    if (in.readableBytes() < frameLengthInt) {
      return null;
    }
    if (lengthFieldSize > frameLengthInt) {
      failOnFrameLengthLessThanInitialBytesToStrip(in, frameLength, lengthFieldSize);
    }
    in.skipBytes(lengthFieldSize);

    // extract frame from index - length field
    int readerIndex = in.readerIndex();
    int actualFrameLength = frameLengthInt - lengthFieldSize;
    ByteBuf frame = in.retainedSlice(readerIndex, actualFrameLength);
    // Move in reader index to the end of frame now that it's been retained
    // in the frame buf. Return frame for next handler
    in.readerIndex(readerIndex + actualFrameLength);
    frameLengthInt = -1; // start processing the next frame
    return frame;
  }

  private void discardingTooLongFrame(ByteBuf in) {
    long bytesToDiscard = this.bytesToDiscard;
    int localBytesToDiscard = (int) Math.min(bytesToDiscard, in.readableBytes());
    in.skipBytes(localBytesToDiscard);
    bytesToDiscard -= localBytesToDiscard;
    this.bytesToDiscard = bytesToDiscard;

    failIfNecessary(false);
  }

  private static void failOnNegativeLengthField(
      ByteBuf in, long frameLength, int lengthFieldEndOffset) {
    in.skipBytes(lengthFieldEndOffset);
    throw new CorruptedFrameException("negative pre-adjustment length field: " + frameLength);
  }

  private static void failOnFrameLengthLessThanLengthFieldEndOffset(
      ByteBuf in, long frameLength, int lengthFieldEndOffset) {
    in.skipBytes(lengthFieldEndOffset);
    throw new CorruptedFrameException(
        "Adjusted frame length ("
            + frameLength
            + ") is less "
            + "than lengthFieldEndOffset: "
            + lengthFieldEndOffset);
  }

  private void exceededFrameLength(ByteBuf in, long frameLength) {
    long discard = frameLength - in.readableBytes();
    tooLongFrameLength = frameLength;

    if (discard < 0) {
      // buffer contains more bytes than the frameLength so we can discard all now
      in.skipBytes((int) frameLength);
    } else {
      // Enter the discard mode and discard everything received so far.
      discardingTooLongFrame = true;
      bytesToDiscard = discard;
      in.skipBytes(in.readableBytes());
    }
    failIfNecessary(true);
  }

  private static void failOnFrameLengthLessThanInitialBytesToStrip(
      ByteBuf in, long frameLength, int initialBytesToStrip) {
    in.skipBytes((int) frameLength);
    throw new CorruptedFrameException(
        "Adjusted frame length ("
            + frameLength
            + ") is less "
            + "than initialBytesToStrip: "
            + initialBytesToStrip);
  }

  protected ByteBuf extractFrame(ChannelHandlerContext ctx, ByteBuf buffer, int index, int length) {
    return buffer.retainedSlice(index, length);
  }

  private void fail(long frameLength) {
    if (frameLength > 0) {
      throw new TooLongFrameException(
          "Adjusted frame length exceeds "
              + MAX_FRAME_LENGTH
              + ": "
              + frameLength
              + " - discarded");
    } else {
      throw new TooLongFrameException(
          "Adjusted frame length exceeds " + MAX_FRAME_LENGTH + " - discarding");
    }
  }

  private void failIfNecessary(boolean firstDetectionOfTooLongFrame) {
    if (bytesToDiscard == 0) {
      // Reset to the initial state and tell the handlers that
      // the frame was too large.
      long tooLongFrameLength = this.tooLongFrameLength;
      this.tooLongFrameLength = 0;
      discardingTooLongFrame = false;
      if (firstDetectionOfTooLongFrame) {
        fail(tooLongFrameLength);
      }
    } else {
      // Keep discarding and notify handlers if necessary.
      if (firstDetectionOfTooLongFrame) {
        fail(tooLongFrameLength);
      }
    }
  }
}
