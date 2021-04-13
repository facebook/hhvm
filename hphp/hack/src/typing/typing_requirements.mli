(*
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

(** Checks that all the requirements of the traits and interfaces a class uses are satisfied. *)
val check_class :
  Typing_env_types.env ->
  Pos.t ->
  Decl_provider.class_decl ->
  Typing_env_types.env
