(**
 * Copyright (c) 2017, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 *
*)

(* Given an AST for a block, compute (needs_local_this, decl_vars).
 * `needs_local_this` is true if the method must use InitThisLoc and reference
 * $this as a local.
 * `decl_vars` is the list of locals that must be emitted
 * in the .declvars declaration, as referenced or defined in the block,
 * in the order in which they appear.
 * Do not include function parameters or $GLOBALS. Include $this only if
 *    (a) it's not available implicitly because this is a function or static
 *        method (has_this=false)
 * or (b) this is a closure body (so is_closure_body=true)
 * or (c) it appears in a bare position as a function parameter.
 *)
 val from_ast :
  is_closure_body: bool ->
  has_this: bool ->
  params: Hhas_param.t list ->
  is_toplevel: bool ->
  is_in_static_method: bool ->
  explicit_use_set: SSet.t ->
  Ast.program ->
  bool * string list

 val vars_from_ast :
  is_closure_body: bool ->
  has_this: bool ->
  params: Ast.fun_param list ->
  is_toplevel: bool ->
  is_in_static_method: bool ->
  Ast.program ->
  SSet.t
