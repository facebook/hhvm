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

# Test-only shim that tears down all folly singletons -- including the global
# IO executor returned by folly::getGlobalIOExecutor() and the EventBases owned
# by its worker threads. This is the same teardown that runs at Py_FinalizeEx,
# exposed to Python so a repro can trigger it deterministically at runtime.

cdef extern from *:
    """
    #include <folly/Singleton.h>

    static void _fbthrift_destroy_all_singletons() {
      folly::SingletonVault::singleton()->destroyInstances();
      folly::SingletonVault::singleton()->reenableInstances();
    }
    """
    void _fbthrift_destroy_all_singletons()


def destroy_all_singletons() -> None:
    _fbthrift_destroy_all_singletons()
