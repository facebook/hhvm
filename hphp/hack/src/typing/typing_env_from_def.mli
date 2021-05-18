(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

(** Construct a Typing_env from an AAST toplevel definition. *)

val fun_env :
  ?origin:Decl_counters.origin ->
  Provider_context.t ->
  ('a, 'b, 'c, 'd) Aast.fun_def ->
  Typing_env_types.env

val class_env :
  ?origin:Decl_counters.origin ->
  Provider_context.t ->
  ('a, 'b, 'c, 'd) Aast.class_ ->
  Typing_env_types.env

val record_def_env :
  ?origin:Decl_counters.origin ->
  Provider_context.t ->
  ('a, 'b, 'c, 'd) Aast.record_def ->
  Typing_env_types.env

val typedef_env :
  ?origin:Decl_counters.origin ->
  Provider_context.t ->
  ('a, 'b, 'c, 'd) Aast.typedef ->
  Typing_env_types.env

val gconst_env :
  ?origin:Decl_counters.origin ->
  Provider_context.t ->
  ('a, 'b, 'c, 'd) Aast.gconst ->
  Typing_env_types.env

val get_self_from_c : ('a, 'b, 'c, 'd) Aast.class_ -> Typing_defs.decl_ty
