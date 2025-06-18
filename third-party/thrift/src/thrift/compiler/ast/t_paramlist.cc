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

#include <stdexcept>

#include <thrift/compiler/ast/t_paramlist.h>

namespace apache::thrift::compiler {

std::unique_ptr<t_paramlist> t_paramlist::clone_DO_NOT_USE() const {
  auto clone = std::make_unique<t_paramlist>(program_);
  auto itr = fields_.begin();
  for (; itr != fields_.end(); ++itr) {
    clone->append((*itr)->clone_DO_NOT_USE());
  }
  return clone;
}

} // namespace apache::thrift::compiler
