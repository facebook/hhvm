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

#pragma once

#include <string>

#include <fmt/core.h>
#include <thrift/lib/cpp2/op/Get.h>

namespace thrift::test::lib {

class StringDoubler {
 public:
  static std::string fromThrift(std::string s) {
    s += s;
    return s;
  }
  static std::string toThrift(const std::string& s) {
    return s.substr(0, s.size() / 2);
  }
};

class StructDoubler {
 public:
  template <class T>
  static T fromThrift(T&& s) {
    apache::thrift::op::for_each_field_id<T>([&s]<class Id>(Id) {
      apache::thrift::op::get<Id>(s) =
          apache::thrift::op::get<Id>(s) + apache::thrift::op::get<Id>(s);
    });
    return s;
  }
  template <class T>
  static T toThrift(T&& doubled) {
    apache::thrift::op::for_each_field_id<T>([&doubled]<class Id>(Id) {
      if constexpr (std::is_same_v<
                        std::string,
                        decltype(*apache::thrift::op::get<Id>(doubled))>) {
        const std::string& str = *apache::thrift::op::get<Id>(doubled);
        apache::thrift::op::get<Id>(doubled) = str.substr(0, str.size() / 2);
      } else {
        apache::thrift::op::get<Id>(doubled) =
            apache::thrift::op::get<Id>(doubled) / 2;
      }
    });
    return doubled;
  }
};

} // namespace thrift::test::lib
