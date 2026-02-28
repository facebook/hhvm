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

from types import (
    BuiltinFunctionType,
    BuiltinMethodType,
    FunctionType,
    LambdaType,
    MethodType,
)

from thrift.protocol.TProtocol import TProtocolBase


class TProtocolDecorator:
    def __init__(self, protocol):
        TProtocolBase(protocol)
        self.protocol = protocol

    def __getattr__(self, name):
        if hasattr(self.protocol, name):
            member = getattr(self.protocol, name)
        if type(member) in [
            MethodType,
            FunctionType,
            LambdaType,
            BuiltinFunctionType,
            BuiltinMethodType,
        ]:
            return lambda *args, **kwargs: self._wrap(member, args, kwargs)
        else:
            return member
        raise AttributeError(name)

    def _wrap(self, func, args, kwargs):
        if type(func) == MethodType:
            result = func(*args, **kwargs)
        else:
            result = func(self.protocol, *args, **kwargs)
        return result
