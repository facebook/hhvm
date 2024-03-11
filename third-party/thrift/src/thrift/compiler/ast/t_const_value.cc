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

#include "thrift/compiler/ast/t_const_value.h"

#include <type_traits>

namespace apache::thrift::compiler {

template <typename CV>
void t_const_value::do_assign(CV&& other) {
  static_assert(std::is_same_v<
                std::remove_cv_t<std::remove_reference_t<CV>>,
                t_const_value>);
  assert(kind_ == CV_BOOL);
  kind_ = other.kind_;

  constexpr bool move = std::is_same_v<CV, t_const_value&&>;
  switch (kind_) {
    case CV_BOOL:
      set_bool(other.int_val_ != 0);
      break;
    case CV_INTEGER:
      int_val_ = other.int_val_;
      break;
    case CV_DOUBLE:
      double_val_ = other.double_val_;
      break;
    case CV_STRING:
    case CV_IDENTIFIER:
      new (&string_val_) std::string(std::forward<CV>(other).string_val_);
      break;
    case CV_MAP:
      if constexpr (move) {
        new (&map_val_) map_value(std::forward<CV>(other).map_val_);
      } else {
        new (&map_val_) map_value();
        for (const auto& elem : other.get_map()) {
          add_map(elem.first->clone(), elem.second->clone());
        }
      }
      break;
    case CV_LIST:
      if constexpr (move) {
        new (&list_val_) list_value(std::forward<CV>(other).list_val_);
      } else {
        new (&list_val_) list_value();
        for (const auto& elem : other.get_list()) {
          add_list(elem->clone());
        }
      }
      break;
  }

  owner_ = other.owner_;
  program_ = other.program_;
  ttype_ = std::forward<CV>(other).ttype_;
  ref_range_ = other.ref_range_;
  is_enum_ = other.is_enum_;
  enum_ = other.enum_;
  enum_val_ = other.enum_val_;
}

void t_const_value::reset() {
  switch (kind_) {
    case CV_BOOL:
    case CV_INTEGER:
    case CV_DOUBLE:
      // Do nothing.
      break;
    case CV_STRING:
    case CV_IDENTIFIER:
      string_val_.~basic_string();
      break;
    case CV_MAP:
      map_val_.~map_value();
      break;
    case CV_LIST:
      list_val_.~list_value();
      break;
  }
  kind_ = CV_BOOL;
  int_val_ = 0;
}

t_const_value::t_const_value(const t_const_value& other) {
  do_assign(other);
}

t_const_value& t_const_value::operator=(t_const_value&& other) {
  reset();
  do_assign(std::move(other));
  return *this;
}

const std::vector<t_const_value*>& t_const_value::get_list_or_empty_map()
    const {
  if (kind_ == CV_LIST) {
    return list_val_.raw;
  }
  static const std::vector<t_const_value*> empty;
  return empty;
}

} // namespace apache::thrift::compiler
