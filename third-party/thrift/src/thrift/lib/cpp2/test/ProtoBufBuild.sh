#!/bin/sh
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

set -e

# remove --install_dir
shift 1

${PROTOC} --cpp_out="$INSTALL_DIR" "$@"

# Fix up include path
sed -i \
  's|ProtoBufBenchData.pb.h|thrift/lib/cpp2/test/ProtoBufBenchData.pb.h|g' \
  "$INSTALL_DIR"/ProtoBufBenchData.pb.cc
