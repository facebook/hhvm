(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

val type_at_pos_fused :
  Provider_context.t ->
  Tast.program Tast_with_dynamic.t ->
  (int * int) list ->
  (Tast_env.env * Tast.ty) option list

val type_at_pos :
  Provider_context.t ->
  Tast.program Tast_with_dynamic.t ->
  int ->
  int ->
  (Tast_env.env * Tast.ty) option

val human_friendly_type_at_pos :
  under_dynamic:bool ->
  Provider_context.t ->
  Tast.program Tast_with_dynamic.t ->
  int ->
  int ->
  (Tast_env.env * Tast.ty) option

val type_at_range :
  Provider_context.t ->
  Tast.program Tast_with_dynamic.t ->
  int ->
  int ->
  int ->
  int ->
  (Tast_env.env * Tast.ty) option

val go_ctx :
  ctx:Provider_context.t ->
  entry:Provider_context.entry ->
  line:int ->
  column:int ->
  (string * string) option
