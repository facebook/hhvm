#!/usr/bin/env fbpython
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

import unittest

from thrift.protocol import TSimpleJSONProtocol
from thrift.transport.TTransport import TMemoryBuffer

from .NameConflicts.ttypes import bar


class NameConflictsTest(unittest.TestCase):
    def testReadFromJson(self) -> None:
        obj = bar()
        obj.readFromJson('{ "foo" : {"anything": 1} }')

    def testReadPath(self) -> None:
        obj = bar()
        trans = TMemoryBuffer('{ "foo" : {"anything": 1} }')
        proto = TSimpleJSONProtocol.TSimpleJSONProtocol(trans, bar.thrift_spec)
        obj.read(proto)
