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

import typing

TAdaptFrom = typing.TypeVar("TAdaptFrom")
TAdaptTo = typing.TypeVar("TAdaptTo")

# Need this workaround until we upgrade to Cython 3.0a7,
# in which proper support of PEP-560 is added
# https://github.com/cython/cython/pull/4005
AdapterGeneric = typing.Generic[TAdaptFrom, TAdaptTo]


class Adapter(*AdapterGeneric.__mro_entries__((AdapterGeneric,))):
    __orig_bases__ = (AdapterGeneric,)

    @classmethod
    def from_thrift(cls, original):
        raise NotImplementedError()

    @classmethod
    def to_thrift(cls, adapted):
        raise NotImplementedError()

    @classmethod
    def from_thrift_field(cls, ori, field_id, strct):
        return cls.from_thrift(ori)

    @classmethod
    def to_thrift_field(cls, adapted, field_id, strct):
        return cls.to_thrift(adapted)
