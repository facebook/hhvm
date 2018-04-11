(**
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Typing_defs

(* Returns (class_name, tconst_name, tconst_reference_position) for each type
 * constant referenced in a type access. *)
val referenced_typeconsts:
  Typing_env.env ->
  expand_env ->
  Typing_reason.t ->
  taccess_type ->
  (string * string * Pos.t) list
