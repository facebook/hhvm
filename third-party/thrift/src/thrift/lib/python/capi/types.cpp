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

#include <thrift/lib/python/capi/types.h>

namespace apache::thrift::python::capi {

StrongRef& StrongRef::operator=(StrongRef&& other) {
  if (this != &other) {
    obj_ = nullptr;
    std::swap(obj_, other.obj_);
  }
  return *this;
}

PyObject* StrongRef::release() && {
  PyObject* ptr = obj_;
  obj_ = nullptr;
  return ptr;
}

bool StrongRef::isNone() const {
  return obj_ == Py_None;
}

} // namespace apache::thrift::python::capi
