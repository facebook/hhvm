(*
 * Copyright (c) 2017, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Aast
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

(* Rewrites an Xml node into a New node. The resulting New expression has
 * four arguments. This mimics the rewrite undergone before the emitter is
 * run.
 *
 * For example,
 * `<p colur='blue'>Hi</p>z`
 * becomes
 * `new :p(darray['colour' => 'blue'], varray['Hi'],"",1);`
 *
 * The first two arguments are for the attributes and children. The last two
 * arguments are for the filename and line number. These are irrelevant from a
 * typing perspective, so they are set to arbitrary defaults.
 *)
val rewrite_xml_into_new :
  Pos.t ->
  sid ->
  (Pos.t, 'b, 'c, 'd) xhp_attribute list ->
  (Pos.t, 'b, 'c, 'd) expr list ->
  (Pos.t, 'b, 'c, 'd) expr
