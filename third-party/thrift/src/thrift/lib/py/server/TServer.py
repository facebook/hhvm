# Copyright (c) Meta Platforms, Inc. and affiliates.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

# pyre-unsafe

from __future__ import absolute_import, division, print_function, unicode_literals

import logging

from thrift.protocol import TBinaryProtocol
from thrift.protocol.THeaderProtocol import THeaderProtocolFactory
from thrift.Thrift import TApplicationException, TProcessor
from thrift.transport import TTransport


class TConnectionContext:
    def getPeerName(self):
        """Gets the address of the client.

        Returns:
          The equivalent value of socket.getpeername() on the client socket
        """
        raise NotImplementedError


class TRpcConnectionContext(TConnectionContext):
    """Connection context class for thrift RPC calls"""

    def __init__(self, client_socket, iprot=None, oprot=None):
        """Initializer.

        Arguments:
          client_socket: the TSocket to the client
        """
        self._client_socket = client_socket
        self.iprot = iprot
        self.oprot = oprot

    def setProtocols(self, iprot, oprot):
        self.iprot = iprot
        self.oprot = oprot

    def getPeerName(self):
        """Gets the address of the client.

        Returns:
          Same value as socket.peername() for the TSocket
        """
        return self._client_socket.getPeerName()

    def getSockName(self):
        """Gets the address of the server.

        Returns:
          Same value as socket.getsockname() for the TSocket
        """
        return self._client_socket.getsockname()


class TServerEventHandler:
    """Event handler base class.

    Override selected methods on this class to implement custom event handling
    """

    def preServe(self, address):
        """Called before the server begins.

        Arguments:
          address: the address that the server is listening on
        """
        pass

    def newConnection(self, context):
        """Called when a client has connected and is about to begin processing.

        Arguments:
          context: instance of TRpcConnectionContext
        """
        pass

    def clientBegin(self, iprot, oprot):
        """Deprecated: Called when a new connection is made to the server.

        For all servers other than TNonblockingServer, this function is called
        whenever newConnection is called and vice versa.  This is the old-style
        for event handling and is not supported for TNonblockingServer. New
        code should always use the newConnection method.
        """
        pass

    def connectionDestroyed(self, context):
        """Called when a client has finished request-handling.

        Arguments:
          context: instance of TRpcConnectionContext
        """
        pass


class TServer:
    """Base interface for a server, which must have a serve method."""

    """ constructors for all servers:
    1) (processor, serverTransport)
    2) (processor, serverTransport, transportFactory, protocolFactory)
    3) (processor, serverTransport,
        inputTransportFactory, outputTransportFactory,
        inputProtocolFactory, outputProtocolFactory)

        Optionally, the handler can be passed instead of the processor,
        and a processor will be created automatically:

    4) (handler, serverTransport)
    5) (handler, serverTransport, transportFacotry, protocolFactory)
    6) (handler, serverTransport,
        inputTransportFactory, outputTransportFactory,
        inputProtocolFactory, outputProtocolFactory)

        The attribute serverEventHandler (default: None) receives
        callbacks for various events in the server lifecycle.  It should
        be set to an instance of TServerEventHandler.

        """

    def __init__(self, *args):
        if len(args) == 2:
            self.__initArgs__(
                args[0],
                args[1],
                TTransport.TTransportFactoryBase(),
                TTransport.TTransportFactoryBase(),
                TBinaryProtocol.TBinaryProtocolFactory(),
                TBinaryProtocol.TBinaryProtocolFactory(),
            )
        elif len(args) == 4:
            self.__initArgs__(args[0], args[1], args[2], args[2], args[3], args[3])
        elif len(args) == 6:
            self.__initArgs__(args[0], args[1], args[2], args[3], args[4], args[5])

    def __initArgs__(
        self,
        processor,
        serverTransport,
        inputTransportFactory,
        outputTransportFactory,
        inputProtocolFactory,
        outputProtocolFactory,
    ):
        self.processor = self._getProcessor(processor)
        self.serverTransport = serverTransport
        self.inputTransportFactory = inputTransportFactory
        self.outputTransportFactory = outputTransportFactory
        self.inputProtocolFactory = inputProtocolFactory
        self.outputProtocolFactory = outputProtocolFactory

        self.serverEventHandler = TServerEventHandler()

    def _getProcessor(self, processor):
        """Check if a processor is really a processor, or if it is a handler
        auto create a processor for it"""
        if isinstance(processor, TProcessor):
            return processor
        elif hasattr(processor, "_processor_type"):
            handler = processor
            return handler._processor_type(handler)
        else:
            raise TApplicationException(message="Could not detect processor type")

    def setServerEventHandler(self, handler):
        self.serverEventHandler = handler

    def _clientBegin(self, context, iprot, oprot):
        self.serverEventHandler.newConnection(context)
        self.serverEventHandler.clientBegin(iprot, oprot)

    def handle(self, client):
        itrans = self.inputTransportFactory.getTransport(client)
        otrans = self.outputTransportFactory.getTransport(client)
        iprot = self.inputProtocolFactory.getProtocol(itrans)

        if isinstance(self.inputProtocolFactory, THeaderProtocolFactory):
            oprot = iprot
        else:
            oprot = self.outputProtocolFactory.getProtocol(otrans)

        context = TRpcConnectionContext(client, iprot, oprot)
        self._clientBegin(context, iprot, oprot)

        try:
            while True:
                self.processor.process(iprot, oprot, context)
        except TTransport.TTransportException:
            pass
        except Exception as x:
            logging.exception(x)

        self.serverEventHandler.connectionDestroyed(context)
        itrans.close()
        otrans.close()

    def serve(self):
        pass
