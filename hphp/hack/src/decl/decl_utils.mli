(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

val unwrap_class_hint :
  Aast.hint -> (Pos.t * string * Aast.hint list) * Typing_error.t option

val unwrap_class_type :
  Typing_defs.decl_ty ->
  Typing_reason.decl_t * Typing_defs.pos_id * Typing_defs.decl_ty list

(** The names of all the parentish things of a shallow class
    from `extends`, `implements`, `uses`, `require extends`, `require implements`,
    and XHP attribute uses. *)
val parentish_names : Shallow_decl_defs.shallow_class -> SSet.t

val infer_const : ('a, 'b) Aast.expr_ -> Aast.tprim option

val split_defs :
  FileInfo.names -> FileInfo.names -> FileInfo.names * FileInfo.names

val coalesce_consistent :
  Typing_defs.consistent_kind ->
  Typing_defs.consistent_kind ->
  Typing_defs.consistent_kind

val consistent_construct_kind :
  Shallow_decl_defs.shallow_class -> Typing_defs.consistent_kind
