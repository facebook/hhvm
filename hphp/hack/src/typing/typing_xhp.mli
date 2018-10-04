(**
 * Copyright (c) 2017, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

(**
 * This is used in computing all possible attributes for XHP spreads.
 *
 * Given a type `cty` that should represent an XHP instance, gather all the
 * XHP attributes and their localized types to verify compatibility with the
 * XHP onto which we are spreading.
 *)
val get_spread_attributes :
  Typing_env.env ->
  Pos.t ->
  Typing_heap.Classes.t ->
  Typing_defs.locl Typing_defs.ty ->
  Typing_env.env * (Nast.pstring * (Nast.PosAnnotation.t * Typing_defs.locl Typing_defs.ty)) list

(**
 * Verify that an XHP body expression is legal.
 *)
val is_xhp_child :
  Typing_env.env ->
  Pos.t ->
  Typing_defs.locl Typing_defs.ty ->
  bool
