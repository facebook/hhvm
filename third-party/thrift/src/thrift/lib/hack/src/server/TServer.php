<?hh
/*
 * Copyright (c) Facebook, Inc. and its affiliates.
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
 *
 * @package thrift.server
 */

/**
 * Base interface for a server.
 *
 *     The attribute serverEventHandler (default: null) receives
 *     callbacks for various events in the server lifecycle.  It should
 *     be set to an instance of TServerEventHandler.
 */
abstract class TServer {
  protected IThriftProcessor $processor;
  protected TServerSocket $serverTransport;
  protected TTransportFactory $transportFactory;
  protected TProtocolFactory $protocolFactory;
  protected ?TServerEventHandler $serverEventHandler;

  public function __construct(
    IThriftProcessor $processor,
    TServerSocket $serverTransport,
    TTransportFactory $transportFactory,
    TProtocolFactory $protocolFactory,
  ) {
    $this->processor = $processor;
    $this->serverTransport = $serverTransport;
    $this->transportFactory = $transportFactory;
    $this->protocolFactory = $protocolFactory;

    $this->serverEventHandler = null;
  }

  protected function _clientBegin(TProtocol $prot): void {
    if ($this->serverEventHandler) {
      $this->serverEventHandler->clientBegin($prot);
    }
  }

  protected function handle(TBufferedTransport $client): bool {
    $trans = $this->transportFactory->getTransport($client);
    $prot = $this->protocolFactory->getProtocol($trans);

    $this->_clientBegin($prot);
    try {
      $this->processor->process($prot, $prot);
    } catch (TTransportException $tx) {
      // ignore
      return false;
    } catch (Exception $x) {
      echo 'Handle caught transport exception: '.$x->getMessage()."\n";
      return false;
    }

    $trans->close();
    return true;
  }

  public abstract function serve(): void;
}
