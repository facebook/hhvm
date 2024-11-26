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

try:
    # pyre-fixme[21]: Could not find module `thrift.python.abstract_types`.
    import thrift.python.abstract_types as abstract_types  # noqa: 401
except ModuleNotFoundError:
    pass

try:
    # pyre-fixme[21]: Could not find module `thrift.python.types`.
    import thrift.python.types as types  # noqa: 401
except ModuleNotFoundError:
    pass
try:
    # pyre-fixme[21]: Could not find module `thrift.python.exceptions`.
    import thrift.python.exceptions as exceptions  # noqa: 401
except ModuleNotFoundError:
    pass

try:
    # pyre-fixme[21]: Could not find module `thrift.python.serializer`.
    import thrift.python.serializer as serializer  # noqa: 401
except ModuleNotFoundError:
    pass

try:
    # pyre-fixme[21]: Could not find module `thrift.python.common`.
    import thrift.python.common as common  # noqa: 401
except ModuleNotFoundError:
    pass
