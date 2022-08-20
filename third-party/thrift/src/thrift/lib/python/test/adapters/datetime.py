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

from datetime import datetime

from thrift.python.adapter import Adapter


class DatetimeAdapter(Adapter[int, datetime]):
    @classmethod
    def from_thrift(cls, original: int) -> datetime:
        return datetime.fromtimestamp(original)

    @classmethod
    def to_thrift(cls, adapted: datetime) -> int:
        return int(adapted.timestamp())
