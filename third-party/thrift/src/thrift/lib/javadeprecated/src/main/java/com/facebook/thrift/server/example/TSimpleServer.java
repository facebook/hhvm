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

package com.facebook.thrift.server.example;

import com.facebook.thrift.TException;
import com.facebook.thrift.TProcessor;
import com.facebook.thrift.TProcessorFactory;
import com.facebook.thrift.protocol.THeaderProtocol;
import com.facebook.thrift.protocol.TProtocol;
import com.facebook.thrift.protocol.TProtocolFactory;
import com.facebook.thrift.server.TRpcConnectionContext;
import com.facebook.thrift.server.TServer;
import com.facebook.thrift.transport.TServerTransport;
import com.facebook.thrift.transport.TTransport;
import com.facebook.thrift.transport.TTransportException;
import com.facebook.thrift.transport.TTransportFactory;
import com.facebook.thrift.utils.Logger;

/**
 * @deprecated "Please use Netty server instead. See https://github.com/facebook/nifty for details."
 *     <p>Simple singlethreaded server for testing.
 */
@Deprecated
public class TSimpleServer extends TServer {

  private static final Logger LOGGER = Logger.getLogger(TSimpleServer.class.getName());

  private boolean stopped_ = false;

  public TSimpleServer(TProcessor processor, TServerTransport serverTransport) {
    super(new TProcessorFactory(processor), serverTransport);
  }

  public TSimpleServer(
      TProcessor processor,
      TServerTransport serverTransport,
      TTransportFactory transportFactory,
      TProtocolFactory protocolFactory) {
    super(new TProcessorFactory(processor), serverTransport, transportFactory, protocolFactory);
  }

  public TSimpleServer(
      TProcessor processor,
      TServerTransport serverTransport,
      TTransportFactory inputTransportFactory,
      TTransportFactory outputTransportFactory,
      TProtocolFactory inputProtocolFactory,
      TProtocolFactory outputProtocolFactory) {
    super(
        new TProcessorFactory(processor),
        serverTransport,
        inputTransportFactory,
        outputTransportFactory,
        inputProtocolFactory,
        outputProtocolFactory);
  }

  public TSimpleServer(TProcessorFactory processorFactory, TServerTransport serverTransport) {
    super(processorFactory, serverTransport);
  }

  public TSimpleServer(
      TProcessorFactory processorFactory,
      TServerTransport serverTransport,
      TTransportFactory transportFactory,
      TProtocolFactory protocolFactory) {
    super(processorFactory, serverTransport, transportFactory, protocolFactory);
  }

  public TSimpleServer(
      TProcessorFactory processorFactory,
      TServerTransport serverTransport,
      TTransportFactory inputTransportFactory,
      TTransportFactory outputTransportFactory,
      TProtocolFactory inputProtocolFactory,
      TProtocolFactory outputProtocolFactory) {
    super(
        processorFactory,
        serverTransport,
        inputTransportFactory,
        outputTransportFactory,
        inputProtocolFactory,
        outputProtocolFactory);
  }

  public void serve() {
    stopped_ = false;
    try {
      serverTransport_.listen();
    } catch (TTransportException ttx) {
      LOGGER.error("Error occurred during listening.", ttx);
      return;
    }

    while (!stopped_) {
      TTransport client = null;
      TProcessor processor = null;
      TTransport inputTransport = null;
      TTransport outputTransport = null;
      TProtocol inputProtocol = null;
      TProtocol outputProtocol = null;
      try {
        client = serverTransport_.accept();
        if (client != null) {
          processor = processorFactory_.getProcessor(client);
          inputTransport = inputTransportFactory_.getTransport(client);
          inputProtocol = inputProtocolFactory_.getProtocol(inputTransport);
          // THeaderProtocol must be the same instance for both input and output
          if (inputProtocol instanceof THeaderProtocol) {
            outputProtocol = inputProtocol;
          } else {
            outputTransport = outputTransportFactory_.getTransport(client);
            outputProtocol = outputProtocolFactory_.getProtocol(outputTransport);
          }
          TRpcConnectionContext server_ctx =
              new TRpcConnectionContext(client, inputProtocol, outputProtocol);
          while (processor.process(inputProtocol, outputProtocol, server_ctx)) {}
        }
      } catch (TTransportException ttx) {
        // Client died, just move on
      } catch (TException tx) {
        if (!stopped_) {
          LOGGER.error("Thrift error occurred during processing of message.", tx);
        }
      } catch (Exception x) {
        if (!stopped_) {
          LOGGER.error("Error occurred during processing of message.", x);
        }
      }

      if (inputTransport != null) {
        inputTransport.close();
      }

      if (outputTransport != null) {
        outputTransport.close();
      }
    }
  }

  public void stop() {
    stopped_ = true;
    serverTransport_.interrupt();
  }
}
