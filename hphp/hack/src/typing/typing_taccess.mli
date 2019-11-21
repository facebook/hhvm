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

(* Returns (class_name, tconst_name, tconst_reference_position) for each type
 * constant referenced in a type access. *)
val referenced_typeconsts :
  env ->
  expand_env ->
  taccess_type ->
  on_error:Errors.typing_error_callback ->
  (string * string * Pos.t) list
