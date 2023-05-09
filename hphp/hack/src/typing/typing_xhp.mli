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
  (env * Typing_error.t option)
  * (Aast.pstring * (Pos.t * Typing_defs.locl_ty)) list

(**
 * Verify that an XHP body expression is legal.
 *)
val is_xhp_child :
  env -> Pos.t -> Typing_defs.locl_ty -> bool * Typing_error.t option

(* Rewrites an Xml node into a New node. The resulting New expression has
 * four arguments. This mimics the rewrite undergone before the emitter is
 * run.
 *
 * For example,
 * `<p colour='blue'>Hi</p>;`
 * becomes
 * `new :p(darray['colour' => 'blue'], varray['Hi'],"",1);`
 *
 * The first two arguments are for the attributes and children. The last two
 * arguments are for the filename and line number. These are irrelevant from a
 * typing perspective, so they are set to arbitrary defaults.
 *)
val rewrite_xml_into_new :
  Pos.t -> sid -> Nast.xhp_attribute list -> Nast.expr list -> Nast.expr

(* Rewrites an Xml attribute access into a call to getAttribute. The
 * resulting Call expression has one argument fake string argument to match the
 * expected parameter of getAttribute.
 *
 * For example,
 * `$xml->:attr;`
 * becomes
 * `$xml->getAttribute('');`
 *
 * The null flavor argument is indicates an expression of the form
 * `$xml?->:attr`
 *)
val rewrite_attribute_access_into_call :
  Pos.t -> Nast.expr -> og_null_flavor -> Nast.expr
