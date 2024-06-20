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

#include <thrift/test/python_capi/fixture.h>

namespace apache::thrift::test {

using ::apache::thrift::python::capi::Constructor;

namespace {

template <typename Container>
void fill_list(const std::string& prefix, Container& list) {
  for (auto view : {"foo", "bar", "baz"}) {
    if constexpr (std::is_same_v<typename Container::value_type, std::string>) {
      list.emplace_back(prefix + "__" + view);
    } else {
      list.emplace_back(folly::IOBuf::COPY_BUFFER, prefix + "__ " + view);
    }
  }
}

template <typename FieldRef>
void fill_list_field(const std::string& prefix, FieldRef list) {
  fill_list(prefix, list.ensure());
}

template <typename FieldRef>
void fill_nested_list_field(const std::string& prefix, FieldRef list) {
  list.ensure();
  for (size_t i = 0; i < 3; ++i) {
    list->emplace_back();
    fill_list(prefix, list->back());
  }
}

template <typename FieldRef>
void fill_tensor_list_field(const std::string& prefix, FieldRef list) {
  list.ensure();
  for (size_t i = 0; i < 3; ++i) {
    list->emplace_back();
    auto& mid_list = list->back();
    for (size_t j = 0; j < 3; ++j) {
      mid_list.emplace_back();
      fill_list(prefix, mid_list.back());
    }
  }
}

::thrift::test::python_capi::TemplateLists fillTemplateList() noexcept {
  ::thrift::test::python_capi::TemplateLists s;
  fill_list_field("std_vector", s.std_string());
  fill_list_field("std_deque", s.deque_string());
  fill_list_field("folly_small_vector", s.small_vector_iobuf());
  fill_nested_list_field("folly_fbvector_fbvector", s.nested_small_vector());
  fill_tensor_list_field("folly_fbvector_fbvector", s.small_vector_tensor());
  return s;
}

template <typename Container>
void fill_set(const std::string& prefix, Container& set) {
  for (auto view : {"foo", "bar", "baz"}) {
    set.insert(prefix + "__" + view);
  }
}

template <typename FieldRef>
void fill_set_field(const std::string& prefix, FieldRef set) {
  fill_set(prefix, set.ensure());
}

::thrift::test::python_capi::TemplateSets fillTemplateSet() noexcept {
  ::thrift::test::python_capi::TemplateSets s;
  fill_set_field("std_set", s.std_set());
  fill_set_field("std_unordered", s.std_unordered());
  fill_set_field("folly_F14FastSet", s.folly_fast());
  fill_set_field("folly_F14NodeSet", s.folly_node());
  fill_set_field("folly_F14ValueSet", s.folly_value());
  fill_set_field("folly_F14VectorSet", s.folly_vector());
  fill_set_field("folly_sorted_vector_set", s.folly_sorted_vector());
  return s;
}

template <typename S>
std::string serializeStruct(const S& s) noexcept {
  auto iobuf_ptr = python::capi::detail::serialize_to_iobuf(s);
  std::string ret;
  iobuf_ptr->appendTo(ret);
  return ret;
}

template <typename S>
PyObject* constructStruct(const S& s) noexcept {
  Constructor<S> ctor;
  return ctor(s);
}

} // namespace

std::string serializeTemplateLists() noexcept {
  return serializeStruct(fillTemplateList());
}
PyObject* constructTemplateLists() noexcept {
  return constructStruct(fillTemplateList());
}

std::string serializeTemplateSets() noexcept {
  return serializeStruct(fillTemplateSet());
}
PyObject* constructTemplateSets() noexcept {
  return constructStruct(fillTemplateSet());
}

} // namespace apache::thrift::test
