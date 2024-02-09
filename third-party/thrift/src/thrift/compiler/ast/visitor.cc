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

#include <thrift/compiler/ast/visitor.h>

namespace apache {
namespace thrift {
namespace compiler {

void visitor::traverse(t_program* const program) {
  visit_and_recurse(program);
}

bool visitor::visit(t_program* const /* program */) {
  return true;
}

bool visitor::visit(t_service* const /* service */) {
  return true;
}

bool visitor::visit(t_enum* const /* tenum */) {
  return true;
}

bool visitor::visit(t_structured* const /* tstruct */) {
  return true;
}

bool visitor::visit(t_field* const /* tfield */) {
  return true;
}

bool visitor::visit(t_const* const /* tconst */) {
  return true;
}

void visitor::visit_and_recurse(t_program* const program) {
  if (visit(program)) {
    recurse(program);
  }
}

void visitor::visit_and_recurse(t_service* const service) {
  if (visit(service)) {
    recurse(service);
  }
}

void visitor::visit_and_recurse(t_enum* const tenum) {
  if (visit(tenum)) {
    recurse(tenum);
  }
}

void visitor::visit_and_recurse(t_structured* const tstruct) {
  if (visit(tstruct)) {
    recurse(tstruct);
  }
}

void visitor::visit_and_recurse(t_field* const tfield) {
  if (visit(tfield)) {
    recurse(tfield);
  }
}

void visitor::visit_and_recurse(t_const* const tconst) {
  if (visit(tconst)) {
    recurse(tconst);
  }
}

void visitor::recurse(t_program* const program) {
  for (t_service* const service : program->services()) {
    visit_and_recurse(service);
  }
  for (t_enum* const tenum : program->enums()) {
    visit_and_recurse(tenum);
  }
  for (t_structured* const tstruct : program->structs_and_unions()) {
    visit_and_recurse(tstruct);
  }
  for (t_exception* const texception : program->exceptions()) {
    visit_and_recurse(texception);
  }
  for (t_const* const tconst : program->consts()) {
    visit_and_recurse(tconst);
  }
}

void visitor::recurse(t_service* const /* service */) {
  // partial implementation - that's the end of the line for now
}

void visitor::recurse(t_enum* const /* tenum */) {
  // partial implementation - that's the end of the line for now
}

void visitor::recurse(t_structured* const tstruct) {
  for (auto* tfield : tstruct->get_members()) {
    visit_and_recurse(tfield);
  }
}

void visitor::recurse(t_field* const /* tfield */) {
  // partial implementation - that's the end of the line for now
}

void visitor::recurse(t_const* const /* tconst */) {
  // partial implementation - that's the end of the line for now
}

} // namespace compiler
} // namespace thrift
} // namespace apache
