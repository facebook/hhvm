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
  def_pos:Pos_or_decl.t ->
  env ->
  ce_visibility * bool ->
  Nast.class_id_ ->
  Decl_provider.class_decl ->
  unit

val check_obj_access :
  use_pos:Pos.t -> def_pos:Pos_or_decl.t -> env -> ce_visibility -> unit

val check_inst_meth_access :
  use_pos:Pos.t -> def_pos:Pos_or_decl.t -> ce_visibility -> unit

val check_meth_caller_access :
  use_pos:Pos.t -> def_pos:Pos_or_decl.t -> ce_visibility -> unit

val check_deprecated :
  use_pos:Pos.t -> def_pos:Pos_or_decl.t -> string option -> unit

val is_visible :
  env ->
  ce_visibility * bool ->
  Nast.class_id_ option ->
  Decl_provider.class_decl ->
  bool

(* Can the class in a type hint be accessed from code or signature checked under [env]?
 *   If class is public, then yes
 *   If class is internal, then only if modules match, and in the case that
 *     we are in a signature, the signature had better be internal too.
 *)
val check_classname_access :
  use_pos:Pos.t -> in_signature:bool -> env -> Decl_provider.class_decl -> unit

(* Can the typdef in a type hint be accessed from code or signature checked under [env]?
 *   If type is public, then yes
 *   If type is internal, then only if modules match, and in the case that
 *     we are in a signature, the signature had better be internal too.
 *)
val check_typedef_access :
  use_pos:Pos.t ->
  in_signature:bool ->
  env ->
  Decl_provider.typedef_decl ->
  unit
