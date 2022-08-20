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

from thrift.Thrift import TType


def fix_spec(all_structs) -> None:
    for s in all_structs:
        spec = s.thrift_spec
        for t in spec:
            if t is None:
                continue
            elif t[1] == TType.STRUCT:
                t[3][1] = t[3][0].thrift_spec
            elif t[1] in (TType.LIST, TType.SET):
                _fix_list_or_set(t[3])
            elif t[1] == TType.MAP:
                _fix_map(t[3])


def _fix_list_or_set(element_type) -> None:
    if element_type[0] == TType.STRUCT:
        element_type[1][1] = element_type[1][0].thrift_spec
    elif element_type[0] in (TType.LIST, TType.SET):
        _fix_list_or_set(element_type[1])
    elif element_type[0] == TType.MAP:
        _fix_map(element_type[1])


def _fix_map(element_type) -> None:
    if element_type[0] == TType.STRUCT:
        element_type[1][1] = element_type[1][0].thrift_spec
    elif element_type[0] in (TType.LIST, TType.SET):
        _fix_list_or_set(element_type[1])
    elif element_type[0] == TType.MAP:
        _fix_map(element_type[1])

    if element_type[2] == TType.STRUCT:
        element_type[3][1] = element_type[3][0].thrift_spec
    elif element_type[2] in (TType.LIST, TType.SET):
        _fix_list_or_set(element_type[3])
    elif element_type[2] == TType.MAP:
        _fix_map(element_type[3])
