(*
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Typing_defs
open Typing_env_types

type type_member =
  | Error of Typing_error.t option
  | Exact of locl_ty
  | Abstract of {
      lower: locl_ty;
      upper: locl_ty;
    }

val lookup_type_member :
  env ->
  on_error:Typing_error.Reasons_callback.t option ->
  this_ty:locl_ty ->
  pos_id * exact ->
  pos_id ->
  env * type_member
