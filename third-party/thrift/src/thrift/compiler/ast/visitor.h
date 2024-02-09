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

#include <thrift/compiler/ast/t_const.h>
#include <thrift/compiler/ast/t_enum.h>
#include <thrift/compiler/ast/t_field.h>
#include <thrift/compiler/ast/t_program.h>
#include <thrift/compiler/ast/t_service.h>
#include <thrift/compiler/ast/t_structured.h>

namespace apache {
namespace thrift {
namespace compiler {

// TODO(afuller): Replace usage with ast_visitor and remove.
class visitor {
 public:
  virtual ~visitor() = default;

  /***
   *  The entry point for traversal. Call-sites should call this method.
   */
  void traverse(t_program* program);

  /***
   *  Derived visitor types will generally override these virtual methods.
   *
   *  Return whether or not to visit children AST nodes afterward. For example,
   *  if `visit(t_program*)` returns `true`, then the visitor will
   *  continue visiting the program's members.
   *
   *  The default implementations of these virtual methods is simply to return
   *  `true`. This allows derived visitor types to implement only the particular
   *  overloads that they specifically need.
   *
   *  Note: These are extension points, not entry points for traversal.
   */
  virtual bool visit(t_program* program);
  virtual bool visit(t_service* service);
  virtual bool visit(t_enum* tenum);
  virtual bool visit(t_structured* tstruct);
  virtual bool visit(t_field* tfield);
  virtual bool visit(t_const* tconst);

 protected:
  visitor() = default;

  /***
   *  Visitor combinator types will generally override these virutal methods.
   *
   *  General derived visitors should not need to.
   */
  virtual void visit_and_recurse(t_program* program);
  virtual void visit_and_recurse(t_service* service);
  virtual void visit_and_recurse(t_enum* tenum);
  virtual void visit_and_recurse(t_structured* tstruct);
  virtual void visit_and_recurse(t_field* tfield);
  virtual void visit_and_recurse(t_const* tconst);

  void recurse(t_program* program);
  void recurse(t_service* service);
  void recurse(t_enum* tenum);
  void recurse(t_structured* tstruct);
  void recurse(t_field* tfield);
  void recurse(t_const* tconst);
};

} // namespace compiler
} // namespace thrift
} // namespace apache
