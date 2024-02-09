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

#include <thrift/compiler/validator/validator.h>

#include <vector>

namespace apache {
namespace thrift {
namespace compiler {

namespace {

// Deprecated in favor of ast_visitor.
//
// Useful for splitting up a single large visitor into multiple smaller ones,
// each doing its own portion of the work.
//
// Runs the visitors across the AST nodes in lockstep with each other, taking
// care to recurse with only those visitors which return true from visit().
//
// Performs a single concurrent traversal will all visitors, rather than a
// sequence of traversals with one visitor each. The concurrent traversal will
// interleave all the visitor traversals in lockstep.
class interleaved_visitor : public visitor {
 public:
  explicit interleaved_visitor(std::vector<visitor*> visitors);

 protected:
  void visit_and_recurse(t_program* program) override;
  void visit_and_recurse(t_service* service) override;
  void visit_and_recurse(t_enum* tenum) override;
  void visit_and_recurse(t_structured* tstruct) override;
  void visit_and_recurse(t_field* tfield) override;
  void visit_and_recurse(t_const* tconst) override;

 private:
  template <typename Visitee>
  void visit_and_recurse_gen(Visitee* visitee);

  std::vector<visitor*> visitors_;
  std::vector<bool> rec_mask_{std::vector<bool>(visitors_.size(), true)};
};

interleaved_visitor::interleaved_visitor(std::vector<visitor*> visitors)
    : visitor(), visitors_(std::move(visitors)) {}

void interleaved_visitor::visit_and_recurse(t_program* const program) {
  visit_and_recurse_gen(program);
}

void interleaved_visitor::visit_and_recurse(t_service* const service) {
  visit_and_recurse_gen(service);
}

void interleaved_visitor::visit_and_recurse(t_enum* const tenum) {
  visit_and_recurse_gen(tenum);
}

void interleaved_visitor::visit_and_recurse(t_structured* const tstruct) {
  visit_and_recurse_gen(tstruct);
}

void interleaved_visitor::visit_and_recurse(t_field* const tfield) {
  visit_and_recurse_gen(tfield);
}

void interleaved_visitor::visit_and_recurse(t_const* const tconst) {
  visit_and_recurse_gen(tconst);
}

template <typename Visitee>
void interleaved_visitor::visit_and_recurse_gen(Visitee* const visitee) {
  // track the set of visitors which return true from visit()
  auto rec_mask = std::vector<bool>(visitors_.size());
  auto any = false;
  for (size_t i = 0; i < visitors_.size(); ++i) {
    const auto rec = rec_mask_[i] && visitors_[i]->visit(visitee);
    rec_mask[i] = rec;
    any = any || rec;
  }
  // only recurse with the set of visitors which return true from visit()
  if (any) {
    std::swap(rec_mask_, rec_mask);
    recurse(visitee);
    std::swap(rec_mask_, rec_mask);
  }
}

} // namespace

void validator_list::traverse(t_program* const program) {
  auto pointers = std::vector<visitor*>{};
  for (const auto& v : validators_) {
    pointers.push_back(v.get());
  }
  interleaved_visitor(pointers).traverse(program);
}

void validator::validate(t_program*, diagnostics_engine&) {}

} // namespace compiler
} // namespace thrift
} // namespace apache
