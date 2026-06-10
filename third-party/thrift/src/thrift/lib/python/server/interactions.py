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

# pyre-strict

"""Core concepts for thrift-python server-side interactions.

An *interaction* is a stateful sub-session within a single Thrift connection.
The client opens an interaction (via a factory method on the service), issues
one or more method calls scoped to it, and then terminates it. Unlike ordinary
service methods -- which are dispatched against the single service handler
instance -- each interaction gets its own handler instance that holds the
per-session state, mirroring the C++ ``apache::thrift::Tile`` model.

This module is part of the ``thrift.python.server_impl`` package; the public
import path is ``thrift.python.server`` (which re-exports ``Interaction``).
"""

from __future__ import annotations


class Interaction:
    """Base class for thrift-python interaction handlers.

    Subclass this to implement an interaction. One instance is created per
    client interaction session and owned by the C++ runtime for the lifetime
    of that interaction, so instance attributes are a safe place to keep
    per-session state::

        class CounterHandler(Interaction):
            def __init__(self) -> None:
                self._value = 0

            async def add(self, n: int) -> None:
                self._value += n

            async def get(self) -> int:
                return self._value

    The compiler generates an ``<InteractionName>Interface`` subclass of this
    base with the wire-dispatch wrappers; users subclass that generated
    interface and implement only the business-logic methods.
    """
