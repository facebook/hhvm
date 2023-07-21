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

#include <thrift/compiler/mutator/mutator.h>

namespace apache {
namespace thrift {
namespace compiler {

namespace {

class mutator_list {
 public:
  mutator_list() = default;

  std::vector<visitor*> get_pointers() const {
    auto pointers = std::vector<visitor*>{};
    for (const auto& v : mutators_) {
      pointers.push_back(v.get());
    }
    return pointers;
  }

  template <typename T, typename... Args>
  void add(Args&&... args) {
    auto ptr = make_mutator<T>(std::forward<Args>(args)...);
    mutators_.push_back(std::move(ptr));
  }

 private:
  std::vector<std::unique_ptr<mutator>> mutators_;
};

} // namespace

static void fill_mutators(mutator_list& ms);

void mutator::mutate(t_program* const program) {
  auto mutators = mutator_list();
  fill_mutators(mutators);
  interleaved_visitor(mutators.get_pointers()).traverse(program);
}

/**
 * fill_mutators - the validator registry
 *
 * This is where all concrete validator types must be registered.
 */

static void fill_mutators(mutator_list&) {
  // add more mutators here ...
}

} // namespace compiler
} // namespace thrift
} // namespace apache
