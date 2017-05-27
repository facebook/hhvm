(**
 * Copyright (c) 2017, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 *
*)

type memoize_info

(* Record everything about a class and its methods that's used to generate
 * wrapper code for the <<__Memoize>> methods *)
val make_info :
  Ast.class_ ->
  Hhbc_id.Class.t ->
  Ast.method_ list ->
  memoize_info

(* Emit wrapper methods for <<__Memoize>> methods in the list *)
val emit_wrapper_methods :
  memoize_info ->
  Ast.class_ ->
  Ast.method_ list ->
  Hhas_method.t list

(* Emit static and instance properties used for memoize cache *)
val emit_properties :
  memoize_info ->
  Ast.method_ list ->
  Hhas_property.t list
