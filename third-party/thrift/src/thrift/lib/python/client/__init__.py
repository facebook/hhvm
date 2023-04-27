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


from __future__ import annotations

from thrift.python.client.async_client import AsyncClient  # noqa: F401
from thrift.python.client.async_client_factory import (  # noqa: F401
    get_client,
    get_proxy_factory,
    install_proxy_factory,
)
from thrift.python.client.client_wrapper import Client  # noqa: F401
from thrift.python.client.request_channel import ClientType  # noqa: F401
from thrift.python.client.sync_client import SyncClient  # noqa: F401
from thrift.python.client.sync_client_factory import (  # noqa: F401
    get_client as get_sync_client,
)
