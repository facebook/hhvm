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

package com.facebook.thrift.protocol;

import io.netty.buffer.ByteBuf;
import io.netty.buffer.Unpooled;
import org.apache.thrift.TException;
import org.apache.thrift.protocol.TProtocol;
import org.apache.thrift.transport.TTransport;

public abstract class ByteBufTProtocol extends TProtocol {
  ByteBuf byteBuf;

  public void wrap(ByteBuf byteBuf) {
    reset();
    this.byteBuf = byteBuf;
  }

  public void wrap(boolean strictRead, boolean strictWrite, ByteBuf byteBuf) {
    wrap(byteBuf);
  }

  public void wrap(byte[] bytes) {
    wrap(Unpooled.wrappedBuffer(bytes));
  }

  ByteBufTProtocol() {
    super(null);
  }

  @Override
  public final TTransport getTransport() {
    throw new UnsupportedOperationException();
  }

  public ByteBuf getByteBuf() {
    return byteBuf;
  }

  /**
   * Writes a binary field represented as a ByteBuf. This method will not free the ByteBuf you're
   * writing to it - you need to manage the lifecylce yourself.
   *
   * @param bin
   * @throws TException
   */
  public abstract void writeBinaryAsByteBuf(ByteBuf bin) throws TException;

  /**
   * Returns a ByteBuf that is a binary field. Can be written to directly so you don't have to copy
   * data to a intermediary. You can only write the number of bytes you equap to size. If you write
   * more you will corrupt the serialized data.
   *
   * @param size the size of data you want to write.
   * @return
   * @throws TException
   */
  public abstract ByteBuf getWritableBinaryAsByteBuf(int size) throws TException;

  /**
   * Reads the binary field as a retained slice. You need to free this yourself or it will cause a
   * memory leak
   *
   * @return
   * @throws TException
   */
  public abstract ByteBuf readBinaryAsSlice() throws TException;

  /** @return Returns the current writer index of the ByteBuf */
  public int mark() {
    return this.byteBuf.writerIndex();
  }

  /**
   * Rollback the ByteBuf to a previous position which is returned by the {@link #mark() mark} call.
   *
   * @param pos pos should be between 0 and writer index
   */
  public void rollback(int pos) {
    this.byteBuf.writerIndex(pos);
  }

  /**
   * Returns the empty struct size when serialized. For most protocols, this is 1 byte. Other
   * protocols, having more than 1 byte must override this method.
   *
   * @return empty struct size
   */
  public int getEmptyStructSize() {
    return 1;
  };
}
