(*
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 * *)

open Typing_defs
open Typing_env_types

val check_class_access :
  use_pos:Pos.t ->
  def_pos:Pos.t ->
  env ->
  visibility * bool ->
  Nast.class_id_ ->
  Decl_provider.class_decl ->
  unit

val check_obj_access :
  use_pos:Pos.t -> def_pos:Pos.t -> env -> visibility -> unit

val check_inst_meth_access :
  use_pos:Pos.t -> def_pos:Pos.t -> visibility -> unit

val check_deprecated : use_pos:Pos.t -> def_pos:Pos.t -> string option -> unit

val is_visible :
  env ->
  visibility * bool ->
  Nast.class_id_ option ->
  Decl_provider.class_decl ->
  bool
