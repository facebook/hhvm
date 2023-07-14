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

#include <thrift/compiler/ast/visitor.h>
#include <thrift/compiler/diagnostic.h>

namespace apache {
namespace thrift {
namespace compiler {

struct mutator_exception : std::exception {
  explicit mutator_exception(
      source_location l, diagnostic_level lvl, std::string msg)
      : loc(l), level(lvl), message(std::move(msg)) {}

  source_location loc;
  diagnostic_level level;
  std::string message;
};

class mutator : virtual public visitor {
 public:
  static void mutate(t_program* program);

 private:
  template <typename T, typename... Args>
  friend std::unique_ptr<T> make_mutator(Args&&...);
};

template <typename T, typename... Args>
std::unique_ptr<T> make_mutator(Args&&... args) {
  return std::unique_ptr<T>(new T(std::forward<Args>(args)...));
}

template <typename T, typename... Args>
void run_mutator(t_program* program, Args&&... args) {
  make_mutator<T>(std::forward<Args>(args)...)->traverse(program);
}

/**
 *  This matches the type of fields to their const values
 */
class field_type_to_const_value : public mutator {
 public:
  using mutator::visit;

  bool visit(t_program* program) override;
  bool visit(t_field* field) override;

 private:
  t_program* program_;
};

/**
 *  This matches the type of consts to their const values
 */
class const_type_to_const_value : public mutator {
 public:
  using mutator::visit;

  bool visit(t_program* program) override;
  bool visit(t_const* tconst) override;

 private:
  t_program* program_;
};

/**
 *  This matches the types of structured annotations to their const values.
 */
class structured_annotation_type_to_const_value : public mutator {
 public:
  using mutator::visit;

  bool visit(t_program* program) override;
  bool visit(t_struct* tstruct) override;
  bool visit(t_field* tfield) override;

 private:
  t_program* program_;
};

} // namespace compiler
} // namespace thrift
} // namespace apache
