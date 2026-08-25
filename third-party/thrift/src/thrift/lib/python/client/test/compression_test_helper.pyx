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

from libcpp.memory cimport make_shared, static_pointer_cast
from thrift.python.client.async_client_factory cimport get_client_with_channel_factory
from thrift.python.client.request_channel cimport ChannelFactory
from thrift.python.client.request_channel import ClientType
from thrift.python.protocol cimport Protocol as cProtocol


def get_compression_test_client(client_class, host, int port):
    return get_client_with_channel_factory(
        client_class,
        static_pointer_cast[ChannelFactory, CompressionTestChannelFactory](
            make_shared[CompressionTestChannelFactory]()
        ),
        host=host,
        port=port,
        path=None,
        timeout=1,
        client_type=ClientType.THRIFT_ROCKET_CLIENT_TYPE,
        protocol=cProtocol.COMPACT,
        ssl_context=None,
        ssl_timeout=1,
        channel_timeout=None,
        keep_alive_timeout_ms=0,
        http_host=host,
    )


def set_compression_offload_enabled(bint enabled):
    setCompressionOffloadEnabled(enabled)


def reset_compression_offload_flag():
    resetCompressionOffloadFlag()
