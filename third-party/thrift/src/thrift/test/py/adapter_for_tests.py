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

from typing import Dict

# pyre-fixme[21]: Could not find module `thrift.test.py.adapter_bar.ttypes`.
from .adapter_bar.ttypes import Bar


class AdapterTestStructToDict:
    Type = Dict[str, int]

    @staticmethod
    # pyre-fixme[3]: Return type must be annotated.
    # pyre-fixme[2]: Parameter must be annotated.
    def from_thrift(thrift_value):
        return {k: v for k, v in thrift_value.__dict__.items() if v is not None}

    @staticmethod
    # pyre-fixme[3]: Return type must be annotated.
    # pyre-fixme[2]: Parameter must be annotated.
    def to_thrift(py_value):
        # pyre-fixme[16]: Module `py` has no attribute `adapter_bar`.
        return Bar(**py_value)
