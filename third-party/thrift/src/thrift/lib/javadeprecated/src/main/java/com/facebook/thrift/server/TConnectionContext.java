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

package com.facebook.thrift.server;

import com.facebook.thrift.protocol.TProtocol;
import java.net.InetAddress;

/** Generic Thrift server connection context */
public class TConnectionContext {
  protected TProtocol input_protocol;
  protected TProtocol output_protocol;

  public TConnectionContext(TProtocol input_protocol, TProtocol output_protocol) {
    this.input_protocol = input_protocol;
    this.output_protocol = output_protocol;
  }

  public TProtocol getInputProtocol() {
    return input_protocol;
  }

  public TProtocol getOutputProtocol() {
    return output_protocol;
  }

  public InetAddress getPeerAddress() {
    return null;
  }
}
