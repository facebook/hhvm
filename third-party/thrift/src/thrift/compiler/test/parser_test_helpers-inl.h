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

#include <memory>
#include <string>
#include <vector>

namespace {
using namespace apache::thrift::compiler;
}

template <typename>
class func_signature_helper;

template <typename ArgType>
class func_signature_helper {
 public:
  static std::unique_ptr<t_type> get_type() {
    return get_type(dummy<std::decay_t<ArgType>>());
  }

 private:
  template <typename>
  struct dummy {};
  static std::unique_ptr<t_type> get_type(dummy<void>) {
    return std::make_unique<t_primitive_type>(t_primitive_type::t_void());
  }
  static std::unique_ptr<t_type> get_type(dummy<int>) {
    return std::make_unique<t_primitive_type>(t_primitive_type::t_i32());
  }
  static std::unique_ptr<t_type> get_type(dummy<double>) {
    return std::make_unique<t_primitive_type>(t_primitive_type::t_double());
  }
};

template <typename...>
class func_signature;

template <>
class func_signature<> {
 public:
  static void get_arg_type(std::vector<std::unique_ptr<t_type>>&) {}
};

template <typename ArgType, typename... Tail>
class func_signature<ArgType, Tail...> {
 public:
  static void get_arg_type(std::vector<std::unique_ptr<t_type>>& vec) {
    vec.push_back(func_signature_helper<ArgType>::get_type());
    func_signature<Tail...>::get_arg_type(vec);
  }
};

template <typename ReturnType, typename... ArgsType>
class func_signature<ReturnType(ArgsType...)> {
 public:
  static std::unique_ptr<t_type> return_type() {
    return func_signature_helper<ReturnType>::get_type();
  }

  static std::vector<std::unique_ptr<t_type>> args_types() {
    std::vector<std::unique_ptr<t_type>> result;
    func_signature<ArgsType...>::get_arg_type(result);
    return result;
  }
};
