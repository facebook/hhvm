(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)
open Hh_prelude

(** Types of parents, interfaces and traits *)
val all_ancestors :
  lin_ancestors_drop_one:Decl_defs.linearization ->
  (string * Typing_defs.decl_ty) Sequence.t

val req_ancestor_names :
  lin_members:Decl_defs.linearization -> (string * unit) Sequence.t

(** All the requirements appearing in `require extends` and `require implements`
  * trait constraints. Does not include `require class` constraints.
  *)
val all_req_extends_implements_requirements :
  lin_members:Decl_defs.linearization ->
  (Pos_or_decl.t * Typing_defs.decl_ty) Sequence.t

(** All the classes appearing in `require class` constraints along `use` chains.
  *)
val all_req_class_requirements :
  lin_members:Decl_defs.linearization ->
  (Pos_or_decl.t * Typing_defs.decl_ty) Sequence.t
