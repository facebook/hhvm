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

package com.facebook.thrift;

import com.facebook.thrift.protocol.TProtocol;
import java.io.Serializable;

/** Generic base interface for generated Thrift objects. */
public interface TBase extends Serializable {

  /**
   * Reads the TObject from the given input protocol.
   *
   * @param iprot Input protocol
   */
  public void read(TProtocol iprot) throws TException;

  /**
   * Writes the objects out to the protocol
   *
   * @param oprot Output protocol
   */
  public void write(TProtocol oprot) throws TException;

  /**
   * Returns a copy of `this`. The type of the returned object should be the same as the type of
   * this; that is, <code>x.getClass() == x.deepCopy().getClass()</code> should be true for any
   * TBase.
   */
  public TBase deepCopy();

  /**
   * Creates an indented String representation for pretty printing
   *
   * @param indent The level of indentation desired
   * @param prettyPrint Set pretty printing on/off
   */
  public String toString(int indent, boolean prettyPrint);
}
