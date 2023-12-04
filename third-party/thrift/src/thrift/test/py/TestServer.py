#!/usr/bin/env python
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

from __future__ import absolute_import, division, print_function, unicode_literals

import glob
import os.path
import sys
import time
from optparse import OptionParser

from thrift import TMultiplexedProcessor
from thrift.protocol import TBinaryProtocol, THeaderProtocol
from thrift.server import TCppServer, TServer
from thrift.Thrift import TProcessorEventHandler
from thrift.transport import TSocket, TSSLSocket, TTransport
from thrift.transport.THeaderTransport import CLIENT_TYPE
from ThriftTest import SecondService, ThriftTest
from ThriftTest.ttypes import *


sys.path.insert(0, "./gen-py")
sys.path.insert(0, os.path.dirname(os.path.dirname(__file__)))
lib_path = glob.glob("../../lib/py/build/lib.*")
if lib_path:
    sys.path.insert(0, lib_path[0])


class SecondHandler(SecondService.Iface):
    def blahBlah(self):
        print("blahBlah()")


class SecondContextHandler(SecondService.ContextIface):
    def __init__(self):
        self.th = SecondHandler()

    def blahBlah(self, handler_ctx):
        self.th.blahBlah()


class TestHandler(ThriftTest.Iface):
    def testVoid(self):
        print("testVoid()")

    def testString(self, str):
        print("testString(%s)" % str)
        return str

    def testByte(self, byte):
        print("testByte(%d)" % byte)
        return byte

    def testI16(self, i16):
        print("testI16(%d)" % i16)
        return i16

    def testI32(self, i32):
        print("testI32(%d)" % i32)
        return i32

    def testI64(self, i64):
        print("testI64(%d)" % i64)
        return i64

    def testDouble(self, dub):
        print("testDouble(%f)" % dub)
        return dub

    def testFloat(self, flt):
        print("testFloat(%f)" % flt)
        return flt

    def testStruct(self, thing):
        print(
            "testStruct({%s, %d, %d, %d})"
            % (thing.string_thing, thing.byte_thing, thing.i32_thing, thing.i64_thing)
        )
        return thing

    def testException(self, str):
        print("testException(%s)" % str)
        if str == "Xception":
            x = Xception()
            x.errorCode = 1001
            x.message = str
            raise x
        elif str == "throw_undeclared":
            raise ValueError("foo")

    def testOneway(self, seconds):
        print("testOneway(%d) => sleeping..." % seconds)
        time.sleep(seconds)
        print("done sleeping")

    def testNest(self, thing):
        return thing

    def testMap(self, thing):
        return thing

    def testSet(self, thing):
        return thing

    def testList(self, thing):
        return thing

    def testEnum(self, thing):
        return thing

    def testTypedef(self, thing):
        return thing


class TestContextHandler(ThriftTest.ContextIface):
    def __init__(self, server_port):
        self.th = TestHandler()
        self._server_port = server_port

    def testVoid(self, handler_ctx):
        self.th.testVoid()
        # This is here so we can check that handler_ctx is getting set,
        # without modifying the service definition which would require
        # modifying all the languages.
        if (
            not (handler_ctx[0].endswith("127.0.0.1") or handler_ctx[0].endswith("::1"))
            or handler_ctx[1] == self._server_port
        ):
            raise ValueError("handler_ctx not set properly " + str(handler_ctx))

    def testString(self, handler_ctx, str):
        return self.th.testString(str)

    def testByte(self, handler_ctx, byte):
        return self.th.testByte(byte)

    def testI16(self, handler_ctx, i16):
        return self.th.testI16(i16)

    def testI32(self, handler_ctx, i32):
        return self.th.testI32(i32)

    def testI64(self, handler_ctx, i64):
        return self.th.testI64(i64)

    def testDouble(self, handler_ctx, dub):
        return self.th.testDouble(dub)

    def testFloat(self, handler_ctx, flt):
        return self.th.testFloat(flt)

    def testStruct(self, handler_ctx, thing):
        return self.th.testStruct(thing)

    def testException(self, handler_ctx, str):
        return self.th.testException(str)

    def testOneway(self, handler_ctx, seconds):
        return self.th.testOneway(seconds)

    def testNest(self, handler_ctx, thing):
        return self.th.testNest(thing)

    def testMap(self, handler_ctx, thing):
        return self.th.testMap(thing)

    def testSet(self, handler_ctx, thing):
        return self.th.testSet(thing)

    def testList(self, handler_ctx, thing):
        return self.th.testList(thing)

    def testEnum(self, handler_ctx, thing):
        return self.th.testEnum(thing)

    def testTypedef(self, handler_ctx, thing):
        return self.th.testTypedef(thing)


class ContextEventHandler(TProcessorEventHandler):
    def getHandlerContext(self, fn_name, server_context):
        # this is a tuple ("hostname", port)
        return server_context.getPeerName()


class HeaderEventHandler(ContextEventHandler):
    def getHandlerContext(self, fn_name, server_context):
        self.htrans = server_context.iprot.trans
        return ContextEventHandler.getHandlerContext(self, fn_name, server_context)

    def preWrite(self, handler_context, fn_name, result):
        for str_key, str_value in self.htrans.get_headers().items():
            # Just spit them back for testing.
            self.htrans.set_header(str_key, str_value)


class TestServerEventHandler(TServer.TServerEventHandler):
    def __init__(self):
        self.num_pre_serve = 0
        self.request_count = 0
        self.num_new_conns = 0
        self.num_conns_destroyed = 0

    def newConnection(self, context):
        self.num_new_conns += 1

    def preServe(self, address):
        self.num_pre_serve += 1

    def clientBegin(self, iprot, oprot):
        self.request_count += 1

    def connectionDestroyed(self, context):
        self.num_conns_destroyed += 1


def main() -> None:
    parser = OptionParser()
    parser.add_option(
        "--ssl",
        action="store_true",
        dest="ssl",
        default=False,
        help="use SSL for encrypted transport",
    )
    parser.add_option(
        "--multiple",
        action="store_true",
        dest="multiple",
        default=False,
        help="use multiple service",
    )
    parser.add_option(
        "--header",
        action="store_true",
        dest="header",
        default=False,
        help="use the Header protocol",
    )
    parser.add_option(
        "--context",
        action="store_true",
        dest="context",
        default=False,
        help="Use the context-passing Handler",
    )
    parser.add_option("--port", action="store", type="int", dest="port", default=9090)
    parser.add_option(
        "--timeout", action="store", type="int", dest="timeout", default=60
    )
    options, args = parser.parse_args()

    event_handler = TestServerEventHandler()

    if options.header:
        pfactory = THeaderProtocol.THeaderProtocolFactory(
            True,
            [
                CLIENT_TYPE.HEADER,
                CLIENT_TYPE.FRAMED_DEPRECATED,
                CLIENT_TYPE.UNFRAMED_DEPRECATED,
                CLIENT_TYPE.HTTP_SERVER,
            ],
        )
    else:
        pfactory = TBinaryProtocol.TBinaryProtocolFactory()

    if options.context:
        processor = ThriftTest.ContextProcessor(TestContextHandler(options.port))
    else:
        processor = ThriftTest.Processor(TestHandler())

    if options.multiple:
        processor = TMultiplexedProcessor.TMultiplexedProcessor()
        if options.context:
            processor.registerProcessor(
                "ThriftTest",
                ThriftTest.ContextProcessor(TestContextHandler(options.port)),
            )
            processor.registerProcessor(
                "SecondService", SecondService.ContextProcessor(SecondContextHandler())
            )
        else:
            processor.registerProcessor(
                "ThriftTest", ThriftTest.Processor(TestHandler())
            )
            processor.registerProcessor(
                "SecondService", SecondService.Processor(SecondHandler())
            )

    server = TCppServer.TCppServer(processor)
    server.setPort(options.port)

    if options.header:
        server.processor.setEventHandler(HeaderEventHandler())
    elif options.context:
        server.processor.setEventHandler(ContextEventHandler())
    server.setServerEventHandler(event_handler)

    server.serve()


if __name__ == "__main__":
    main()  # pragma: no cover
