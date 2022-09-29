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

# cython: c_string_type=unicode, c_string_encoding=utf8

from libc.stdint cimport int64_t
from libcpp cimport bool

def define_int_flag(name, default_value):
    FlagRegistry[int64_t].define(name, default_value)

def get_int_flag(name):
    try:
        return FlagRegistry[int64_t].get(name)
    except IndexError:
        raise KeyError(f"No such flag: {name}")

def define_bool_flag(name, default_value):
    FlagRegistry[bool].define(name, default_value)

def get_bool_flag(name):
    try:
        return FlagRegistry[bool].get(name)
    except IndexError:
        raise KeyError(f"No such flag: {name}")
