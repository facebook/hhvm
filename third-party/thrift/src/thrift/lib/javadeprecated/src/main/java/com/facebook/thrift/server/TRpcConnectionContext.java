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
import com.facebook.thrift.transport.TSocketIf;
import com.facebook.thrift.transport.TTransport;
import java.net.InetAddress;

/** Generic interface for a Thrift server. */
public class TRpcConnectionContext extends TConnectionContext {
  protected TTransport client_socket;

  public TRpcConnectionContext(
      TTransport client_socket, TProtocol input_protocol, TProtocol output_protocol) {
    super(input_protocol, output_protocol);
    this.client_socket = client_socket;
  }

  @Override
  public InetAddress getPeerAddress() {
    TSocketIf socket = (TSocketIf) client_socket;
    return socket.getSocket().getInetAddress();
  }
}
