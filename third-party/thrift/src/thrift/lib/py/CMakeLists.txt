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

include(FBPythonBinary)

set(
  THRIFT_PY_SRCS
  "TMultiplexedProcessor.py"
  "Thrift.py"
  "__init__.py"
  "async_common.py"
  "protocol/TBinaryProtocol.py"
  "protocol/TCompactProtocol.py"
  "protocol/THeaderProtocol.py"
  "protocol/TJSONProtocol.py"
  "protocol/TMultiplexedProtocol.py"
  "protocol/TProtocol.py"
  "protocol/TProtocolDecorator.py"
  "protocol/TSimpleJSONProtocol.py"
  "server/TServer.py"
  "transport/THeaderTransport.py"
  "transport/TSSLSocket.py"
  "transport/TSocket.py"
  "transport/TTransport.py"
  "util/BytesStrIO.py"
  "util/Decorators.py"
  "util/Recursive.py"
  "util/Serializer.py"
  "util/TValidator.py"
  "util/__init__.py"
  "util/async_common.py"
  "util/fuzzer.py"
  "util/randomizer.py"
  "util/remote.py"
)
# thrift_py has external dependencies on "future" python
# package.  It will use the "snappy" and "zstd" compression packages if they
# are available, but does not require them.
add_fb_python_library(
  thrift_py
  SOURCES ${THRIFT_PY_SRCS}
  NAMESPACE "thrift"
)
install_fb_python_library(
  thrift_py
  EXPORT fbthrift-exports
)

add_fb_python_library(
  thrift_py_inspect
  SOURCES "util/inspect.py"
  NAMESPACE "thrift"
)
install_fb_python_library(
  thrift_py_inspect
  EXPORT fbthrift-exports
)
