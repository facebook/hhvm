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


from thrift.python.mutable_containers cimport MutableList

def create_MutableList(object typeinfo):
    """
    Creating a usable `MutableList` from Python is not possible. Although it can
    be created, both `_list_data` and `_val_typeinfo` members will be `None`,
    and they cannot be populated from Python. This function serves as a factory,
    creating a usable `MutableList` only for testing purposes.
    """
    cdef MutableList inst = MutableList.__new__(MutableList)
    inst._val_typeinfo = typeinfo
    inst._list_data = []
    return inst
