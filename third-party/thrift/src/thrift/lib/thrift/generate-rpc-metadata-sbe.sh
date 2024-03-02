#!/bin/bash
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

# This script should be invoked under fbcode
if [[ "$(basename "$PWD")" != "fbcode" ]]; then
  die "Please run this script under fbcode"
fi

buck2 run thrift/vendor/simple-binary-encoding/sbe-tool:sbe_tool -- \
  -J-Dsbe.target.language=Cpp \
  -J-Dsbe.generate.precedence.checks=true \
  -J-Dsbe.output.dir=thrift/lib/thrift \
  thrift/lib/thrift/RpcMetadata.xml

# mark all generated source code as `@generated`
sed -i '1i // @generated using thrift/lib/thrift/generate-rpc-metadata-sbe.sh' thrift/lib/thrift/apache_thrift_sbe/*.h
