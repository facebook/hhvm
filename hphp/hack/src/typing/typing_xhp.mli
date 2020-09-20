(*
 * Copyright (c) 2017, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Typing_env_types

(**
 * This is used in computing all possible attributes for XHP spreads.
 *
 * Given a type `cty` that should represent an XHP instance, gather all the
 * XHP attributes and their localized types to verify compatibility with the
 * XHP onto which we are spreading.
 *)
val get_spread_attributes :
  env ->
  Pos.t ->
  Decl_provider.class_decl ->
  Typing_defs.locl_ty ->
  env * (Aast.pstring * (Pos.t * Typing_defs.locl_ty)) list

(**
 * Verify that an XHP body expression is legal.
 *)
val is_xhp_child : env -> Pos.t -> Typing_defs.locl_ty -> bool
