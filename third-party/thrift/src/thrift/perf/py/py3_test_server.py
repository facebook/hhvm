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

import optparse
import sys

from apache.thrift.test.load import LoadTest
from apache.thrift.test.py3.py3_load_handler import LoadHandler
from thrift.protocol.TBinaryProtocol import TBinaryProtocolAcceleratedFactory
from thrift.protocol.THeaderProtocol import THeaderProtocolFactory
from thrift.server import TCppServer, TServer
from thrift.transport import TSocket, TTransport
from thrift.transport.THeaderTransport import CLIENT_TYPE


def main():
    op = optparse.OptionParser(usage="%prog [options]", add_help_option=False)
    op.add_option(
        "-p",
        "--port",
        action="store",
        type="int",
        dest="port",
        default=1234,
        help="The server port",
    )
    op.add_option(
        "-s",
        "--servertype",
        action="store",
        type="string",
        dest="servertype",
        default="TCppServer",
        help="Type name of server",
    )
    op.add_option(
        "-w",
        "--num_workers",
        action="store",
        type="int",
        dest="workers",
        default=4,
        help="Number of worker processes/threads",
    )
    op.add_option(
        "-h",
        "--header",
        action="store_true",
        default=True,
        help="Use the generated ContextIface",
    )
    op.add_option("-?", "--help", action="help", help="Show this help message and exit")

    (options, args) = op.parse_args()
    if args:
        op.error("trailing arguments: " + " ".join(args))

    handler = LoadHandler()
    processor = LoadTest.Processor(handler)

    if options.header:
        pfactory = THeaderProtocolFactory(
            True,
            [
                CLIENT_TYPE.HEADER,
                CLIENT_TYPE.FRAMED_DEPRECATED,
                CLIENT_TYPE.UNFRAMED_DEPRECATED,
                CLIENT_TYPE.HTTP_SERVER,
            ],
        )
        if options.servertype == "TCppServer":
            print(
                "C++ ThriftServer, Header transport, backwards compatible "
                "with all other types"
            )
        else:
            print(
                "Header transport, backwards compatible with "
                + "unframed, framed, http"
            )
    else:
        if options.servertype == "TCppServer":
            if not options.header:
                op.error("TCppServer cannot be used without header")
        else:
            print("Unframed transport")
        pfactory = TBinaryProtocolAcceleratedFactory()

    if options.servertype == "TCppServer":
        server = TCppServer.TCppServer(processor)
        server.setPort(options.port)
        print("Worker threads: " + str(options.workers))
        server.setNumIOWorkerThreads(options.workers)
    else:
        transport = TSocket.TServerSocket(options.port)
        tfactory = TTransport.TBufferedTransportFactory()
        ServerClass = getattr(TServer, options.servertype)
        server = ServerClass(processor, transport, tfactory, pfactory)

    print("Serving " + options.servertype + " requests on port %d..." % (options.port,))
    server.serve()


def invoke_main() -> None:
    rc = main()
    sys.exit(rc)


if __name__ == "__main__":
    invoke_main()  # pragma: no cover
