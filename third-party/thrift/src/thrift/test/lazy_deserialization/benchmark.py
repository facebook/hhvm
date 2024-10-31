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

import time

from thrift.py3 import deserialize, Protocol, serialize
from thrift.test.lazy_deserialization.simple.types import Foo, LazyFoo


field = [10] * 5000000
s = serialize(LazyFoo(field4=field), Protocol.COMPACT)

start = time.time()
eager = deserialize(Foo, s, Protocol.COMPACT)
print(f"eager deserialization: {time.time() - start:.8f}")

start = time.time()
lazy = deserialize(LazyFoo, s, Protocol.COMPACT)
print(f"lazy deserialization: {time.time() - start:.8f}")

start = time.time()
eager.field4
print(f"eager first field access: {time.time() - start:.8f}")

start = time.time()
lazy.field4
print(f"lazy first field access: {time.time() - start:.8f}")

start = time.time()
eager.field4
print(f"eager second field access: {time.time() - start:.8f}")

start = time.time()
lazy.field4
print(f"lazy second field access: {time.time() - start:.8f}")
