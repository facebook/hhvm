(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

(* Information gathered from the TAST. Includes at least
 * the type and an environment with which to resolve solved
 * type variables. For function calls, the type arguments
 * are also recorded. *)
type t

val get_type : t -> Tast.ty

val get_targs : t -> Tast.targ list

val get_env : t -> Tast_env.env

val type_at_pos_fused :
  Provider_context.t ->
  Tast.program Tast_with_dynamic.t ->
  File_content.Position.t list ->
  t option list

val type_at_pos :
  Provider_context.t ->
  Tast.program Tast_with_dynamic.t ->
  File_content.Position.t ->
  t option

val human_friendly_type_at_pos :
  under_dynamic:bool ->
  Provider_context.t ->
  Tast.program Tast_with_dynamic.t ->
  File_content.Position.t ->
  t option

val type_at_range_fused :
  Provider_context.t ->
  Tast.program Tast_with_dynamic.t ->
  (File_content.Position.t * File_content.Position.t) list ->
  t option list

val go_ctx :
  ctx:Provider_context.t ->
  entry:Provider_context.entry ->
  File_content.Position.t ->
  (string * string) option
