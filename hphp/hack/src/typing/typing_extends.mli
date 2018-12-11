(**
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)


(*****************************************************************************)
(* Checks that a class implements an interface *)
(*****************************************************************************)

open Core_kernel
val check_implements:
    Typing_env.env ->
    string list String.Map.t ->
    Typing_defs.decl Typing_defs.ty ->
    Typing_defs.decl Typing_defs.ty -> unit
