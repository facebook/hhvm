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

from libcpp cimport bool as cbool

cdef class AsyncProcessorFactory:
    async def __aenter__(self):
        # Establish async context managers as a way for end users to async initalize
        # internal structures used by Service Handlers.
        return self

    async def __aexit__(self, *exc_info):
        # Same as above, but allow end users to define things to be cleaned up
        pass

    @staticmethod
    def __get_metadata__():
        raise NotImplementedError()

    @staticmethod
    def __get_thrift_name__():
        raise NotImplementedError()

    async def onStartServing(self):
        pass

    async def onStopRequested(self):
        pass

    cdef cbool requireResourcePools(AsyncProcessorFactory self):
        """
        Override this method to conditionally call the requireResourcePools
        method of ThriftServer.
        NOTE: Once resource pools are the only option, it may be possible
        to remove this method.
        """
        # Tests for some implementations that use py3 servers fail
        # if this function returns True.
        # Some py3 tests implementation do not create metadata, which is a
        # prerequisite to call requireResourcePools() on the ThriftServer.
        # This function can return False with no adverse effects
        # in production. The effect is that py3 tests will not run with
        # resource pools enabled.
        # This function is still relevant because derived implementations
        # like thrift-python may enable resource pools
        # due to better guarantees about the presence of metadata.
        return False
