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

import time

from apache.thrift.test.load.ttypes import LoadError


def us_to_sec(microseconds):
    return float(microseconds) / 1000000


class LoadHandler:
    def __init__(self):
        pass

    def noop(self):
        pass

    def onewayNoop(self):
        pass

    def asyncNoop(self):
        pass

    def sleep(self, us):
        time.sleep(us_to_sec(us))

    def onewaySleep(self, us):
        self.sleep(us)

    def burn(self, us):
        now = time.time()
        end = now + us_to_sec(us)
        while True:
            now = time.time()
            if now > end:
                break

    def onewayBurn(self, us):
        self.burn(us)

    def badSleep(self, us):
        self.sleep(us)

    def badBurn(self, us):
        self.burn(us)

    def throwError(self, code):
        raise LoadError(code=code)

    def throwUnexpected(self, code):
        raise LoadError(code=code)

    def onewayThrow(self, code):
        raise LoadError(code=code)

    def send(self, data):
        pass

    def onewaySend(self, data):
        pass

    def recv(self, bytes):
        return "a" * bytes

    def sendrecv(self, data, recvBytes):
        return "a" * recvBytes

    def echo(self, data):
        return data

    def add(self, a, b):
        return a + b

    def largeContainer(self, data):
        pass

    def iterAllFields(self, data):
        for item in data:
            _ = item.stringField
            for _ in item.stringList:
                pass
        return data
