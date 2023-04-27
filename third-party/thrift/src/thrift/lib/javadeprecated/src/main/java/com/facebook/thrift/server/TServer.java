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

import com.facebook.thrift.TProcessorFactory;
import com.facebook.thrift.protocol.TBinaryProtocol;
import com.facebook.thrift.protocol.TProtocolFactory;
import com.facebook.thrift.transport.TServerTransport;
import com.facebook.thrift.transport.TTransportFactory;

/** Generic interface for a Thrift server. */
public abstract class TServer {

  /** Core processor */
  protected final TProcessorFactory processorFactory_;

  /** Server transport */
  protected final TServerTransport serverTransport_;

  /** Input Transport Factory */
  protected final TTransportFactory inputTransportFactory_;

  /** Output Transport Factory */
  protected final TTransportFactory outputTransportFactory_;

  /** Input Protocol Factory */
  protected final TProtocolFactory inputProtocolFactory_;

  /** Output Protocol Factory */
  protected final TProtocolFactory outputProtocolFactory_;

  /** Default constructors. */
  protected TServer(TProcessorFactory processorFactory, TServerTransport serverTransport) {
    this(
        processorFactory,
        serverTransport,
        new TTransportFactory(),
        new TTransportFactory(),
        new TBinaryProtocol.Factory(),
        new TBinaryProtocol.Factory());
  }

  protected TServer(
      TProcessorFactory processorFactory,
      TServerTransport serverTransport,
      TTransportFactory transportFactory) {
    this(
        processorFactory,
        serverTransport,
        transportFactory,
        transportFactory,
        new TBinaryProtocol.Factory(),
        new TBinaryProtocol.Factory());
  }

  protected TServer(
      TProcessorFactory processorFactory,
      TServerTransport serverTransport,
      TTransportFactory transportFactory,
      TProtocolFactory protocolFactory) {
    this(
        processorFactory,
        serverTransport,
        transportFactory,
        transportFactory,
        protocolFactory,
        protocolFactory);
  }

  protected TServer(
      TProcessorFactory processorFactory,
      TServerTransport serverTransport,
      TTransportFactory inputTransportFactory,
      TTransportFactory outputTransportFactory,
      TProtocolFactory inputProtocolFactory,
      TProtocolFactory outputProtocolFactory) {
    processorFactory_ = processorFactory;
    serverTransport_ = serverTransport;
    inputTransportFactory_ = inputTransportFactory;
    outputTransportFactory_ = outputTransportFactory;
    inputProtocolFactory_ = inputProtocolFactory;
    outputProtocolFactory_ = outputProtocolFactory;
  }

  /** The run method fires up the server and gets things going. */
  public abstract void serve();

  /**
   * Stop the server. This is optional on a per-implementation basis. Not all servers are required
   * to be cleanly stoppable.
   */
  public void stop() {}
}
