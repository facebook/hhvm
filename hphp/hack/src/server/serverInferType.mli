(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

val type_at_pos :
  Provider_context.t ->
  Tast.program ->
  int ->
  int ->
  (Tast_env.env * Tast.ty) option

val expanded_type_at_pos :
  Provider_context.t ->
  Tast.program ->
  int ->
  int ->
  (Tast_env.env * Tast.ty) option

val type_at_range :
  Provider_context.t ->
  Tast.program ->
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
