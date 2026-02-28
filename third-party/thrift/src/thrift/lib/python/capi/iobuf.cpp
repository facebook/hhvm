/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <thrift/lib/python/capi/iobuf.h>

namespace apache::thrift::python::capi::detail {

void handle_protocol_error(const apache::thrift::TProtocolException& e) {
  // TODO(T189799332): use thrift.python.ProtocolError when available
  PyErr_SetString(PyExc_ValueError, e.what());
}

} // namespace apache::thrift::python::capi::detail
