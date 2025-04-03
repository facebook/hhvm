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

#include <thrift/test/python_capi/gen-python-capi/serialized_dep/thrift_types_capi.h>
#include <thrift/test/python_capi/parity.h>

namespace apache::thrift::test {

template <typename T>
using Namespaced = ::apache::thrift::python::capi::PythonNamespaced<
    T,
    ::thrift__test__python_capi__serialized_dep::NamespaceTag>;

static_assert(
    Constructor<
        Namespaced<::thrift::test::python_capi::MarshalStruct>>::kUsingMarshal,
    "Should be marshaled due to UseCAPI annnotation");
static_assert(
    !Constructor<Namespaced<::thrift::test::python_capi::SerializedStruct>>::
        kUsingMarshal,
    "Should be serialized due to UseCAPI annnotation");
static_assert(
    Constructor<
        Namespaced<::thrift::test::python_capi::MarshalUnion>>::kUsingMarshal,
    "Should be marshaled due to UseCAPI annnotation");
static_assert(
    !Constructor<Namespaced<::thrift::test::python_capi::SerializedUnion>>::
        kUsingMarshal,
    "Should be serialized due to UseCAPI annnotation");
static_assert(
    Constructor<
        Namespaced<::thrift::test::python_capi::MarshalError>>::kUsingMarshal,
    "Should be marshaled due to UseCAPI annnotation");
static_assert(
    !Constructor<Namespaced<::thrift::test::python_capi::SerializedError>>::
        kUsingMarshal,
    "Should be serialized due to UseCAPI annnotation");

} // namespace apache::thrift::test
