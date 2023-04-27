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

import threading

from thrift.server.TCppServer import TCppServer
from thrift.Thrift import TProcessor


class TCppServerTestManager(object):
    """
    A context manager for running a TCppServer in unit tests.

    Caller may pass either an Iface, a Processor, or a not-running TCppServer.

    Basic example:

        from mylib import MyServiceHandler
        from thrift.util.TCppServerTestManager import TCppServerTestManager

        class MyServiceTest(unittest.TestCase)

            def test_traffic(self):
                handler = MyServiceHandler()  # derived from MyService.Iface
                with TCppServerTestManager(handler) as server:
                    host, port = server.addr()
                    # Talk to the server using thrift in here....

    See the unit-tests for this class for better-worked-out examples.
    """

    @staticmethod
    def make_server(processor):
        """
        Creates a TCppServer given a processor. This is the function used
        internally, but it may be of interest separately as well.
        """
        server = TCppServer(processor)
        server.setPort(0)
        server.setNumCPUWorkerThreads(1)
        server.setNumIOWorkerThreads(1)
        server.setNewSimpleThreadManager(
            count=1,
            pendingTaskCountMax=5,
        )
        return server

    def __init__(self, obj, cleanUp=True):
        self.__obj = obj
        self.__handler = None
        self.__processor = None
        self.__server = None
        self.__thread = None
        self.__thread_started_ev = None
        self.__do_cleanup = cleanUp

    def __enter__(self):
        self.start()
        return self

    def __exit__(self, type, value, traceback):
        self.stop()

    def start(self):
        """
        Starts the server in another thread.

        Returns after the server has bound to and listened on its port. Callers
        may immediately open connections without needing to wait or poll.
        """
        if self.__is_handler(self.__obj):
            self.__handler = self.__obj
            self.__processor = self.__make_processor(self.__handler)
            self.__server = self.__make_server(self.__processor)
        elif self.__is_processor(self.__obj):
            self.__processor = self.__obj
            self.__server = self.__make_server(self.__processor)
        elif self.__is_server(self.__obj):
            self.__server = self.__obj
        else:
            raise Exception("Not a handler, a processor, or a server.")
        self.__server_started_ev = threading.Event()
        self.__thread = threading.Thread(target=self.__serve)
        self.__thread.start()
        self.__server_started_ev.wait()
        self.__server_started_ev = None

    def stop(self):
        """
        Stops the server.

        Returns after the server has been stopped and all resources have been
        cleaned up.
        """
        self.__server.stop()
        self.__thread.join()
        self.__thread = None
        self.__server = None
        self.__processor = None
        self.__handler = None

    def addr(self):
        """
        Returns a pair of host-addr and port on which the running server is
        listening.

        If constructed with a handler or a processor, addr is * or :: and port
        is ephemeral.
        """
        addr = self.__server.getAddress()
        return addr[0], addr[1]

    def __serve(self):
        self.__server.setup()
        self.__server_started_ev.set()
        try:
            self.__server.loop()
        finally:
            if self.__do_cleanup:
                self.__server.cleanUp()

    def __is_handler(self, obj):
        return hasattr(obj, "_processor_type") and not self.__is_processor(obj)

    def __is_processor(self, obj):
        return isinstance(obj, TProcessor)

    def __is_server(self, obj):
        return isinstance(obj, TCppServer)

    def __make_processor(self, handler):
        return handler._processor_type(handler)

    def __make_server(self, processor):
        return self.__class__.make_server(self.__processor)
