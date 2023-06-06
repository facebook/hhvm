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

import sys
import unittest.mock as mock
from asyncio import iscoroutinefunction
from typing import Type

from thrift.py3.client import Client


def mock_client(client_klass: Type[Client]) -> mock.AsyncMock:
    """
    Given a thrift-py3 client class create a robust mock for it.

    thrift-py3 rpc client methods are sync methods that return awaitables
    which is not detectable by the mock library. This patches the logic to always return
    true for all methods of the client_klass.
    """

    def magic(thing: object) -> bool:
        try:
            # pyre-ignore
            if thing.__name__ in ("__aenter__", "__aexit__"):
                return True
            if not thing.__name__.startswith("__") and issubclass(
                client_klass,
                # pyre-ignore
                thing.__objclass__,
            ):
                return True
        except Exception:
            pass
        return iscoroutinefunction(thing)

    if sys.version_info >= (3, 9):
        patch_loc = "unittest.mock.iscoroutinefunction"
    else:
        patch_loc = "asyncio.iscoroutinefunction"
    with mock.patch(patch_loc, magic):
        client_mock = mock.AsyncMock(client_klass)
        client_mock.__aenter__.return_value = client_mock
        # When running with Cinder, __aexit__ is not auto patched.
        # Refering the method name once to ensure it's patched. It's a no-op.
        client_mock.__aexit__
    return client_mock
