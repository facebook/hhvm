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

import sys

if sys.version_info[0] >= 3:
    from io import BytesIO

    class BytesStrIO(BytesIO):
        def __init__(self, *args):
            args_new = []
            for arg in args:
                if not isinstance(arg, (bytes, memoryview)):
                    args_new.append(arg.encode())
                else:
                    args_new.append(arg)
            BytesIO.__init__(self, *args_new)

        def write(self, data):
            if isinstance(data, (bytes, memoryview)):
                BytesIO.write(self, data)
            else:
                BytesIO.write(self, data.encode())
