(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

val overload_extract_from_awaitable :
  Typing_env_types.env ->
  p:Pos.t ->
  Typing_defs.locl_ty ->
  Typing_env_types.env * Typing_defs.locl_ty

val overload_extract_from_awaitable_list :
  Typing_env_types.env ->
  Pos.t ->
  Typing_defs.locl_ty list ->
  Typing_env_types.env * Typing_defs.locl_ty list

val overload_extract_from_awaitable_shape :
  Typing_env_types.env ->
  Pos.t ->
  ('a * Typing_defs.locl_ty) Typing_defs.ShapeFieldMap.t ->
  Typing_env_types.env * ('a * Typing_defs.locl_ty) Typing_defs.ShapeFieldMap.t
