(*
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

(*****************************************************************************)
(* Checks that a class implements an interface, extends a base class, or     *)
(* uses a trait.                                                             *)
(*****************************************************************************)

open Hh_prelude

val check_implements :
  Typing_env_types.env ->
  string list String.Map.t ->
  (* The parent (interface, base type, or trait) *)
  Typing_defs.decl_ty ->
  (* The type to be checked, instantiated at its generic parameters *)
  Typing_defs.decl_ty ->
  Typing_env_types.env
