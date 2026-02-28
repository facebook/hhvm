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

from thrift.py3.types cimport Struct

cdef class StructBuilder:

    _struct_type = None

    @staticmethod
    cdef object __from_obj__(object obj):
        if isinstance(obj, StructBuilder):
            return (<StructBuilder>obj).__build__()
        if isinstance(obj, (list, tuple, set)):
            return [StructBuilder.__from_obj__(e) for e in obj]
        if isinstance(obj, dict):
            return {k: StructBuilder.__from_obj__(v) for k, v in obj.items()}
        return obj

    def __call__(self):
        return self.__build__()

    def __iter__(self):
        pass

    cdef object __build__(self):
        return self._struct_type(**{name: StructBuilder.__from_obj__(value) for name, value in self})
