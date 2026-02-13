(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

val find_recursive_mentions_in_decl_ty_via_typedef :
  Typing_env_types.env ->
  string ->
  Typing_defs.decl_ty ->
  Typing_defs.decl_ty list

val decl_ty_mentions_name_via_typedef :
  Typing_env_types.env -> string -> Typing_defs.decl_ty -> bool

val find_where_clause_recursive_mentions :
  Typing_env_types.env ->
  Aast.sid ->
  Aast.where_constraint_hint list ->
  (Aast.where_constraint_hint * Typing_defs.decl_ty list) list

val filter_where_clauses_with_recursive_mentions :
  Typing_env_types.env ->
  Aast.sid ->
  Aast.where_constraint_hint list ->
  Aast.where_constraint_hint list
