(*
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

(* checks that a method can be invoked in a dynamic context by ensuring that
   the types of its arguments are enforceable and its return type can be
   coerced to dynamic *)

val sound_dynamic_interface_check :
  Typing_env_types.env ->
  Typing_defs.decl_ty option list ->
  Typing_defs.locl_ty ->
  bool

(*****************************************************************************)
(* Checks that a class implements an interface, extends a base class, or     *)
(* uses a trait.                                                             *)
(*****************************************************************************)

val check_implements :
  Typing_env_types.env ->
  Typing_defs.decl_ty list ->
  (* The parent (interface, base type, or trait) *)
  Typing_defs.decl_ty ->
  (* The type to be checked, instantiated at its generic parameters *)
  Pos.t * Typing_defs.decl_ty ->
  Typing_env_types.env
