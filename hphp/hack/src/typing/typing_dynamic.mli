(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

val check_property_sound_for_dynamic_read :
  on_error:('a -> 'b -> 'c -> Pos_or_decl.t * string -> unit) ->
  Typing_env_types.env ->
  'c ->
  'a * 'b ->
  Typing_defs.locl_ty ->
  unit

val check_property_sound_for_dynamic_write :
  on_error:('a -> 'b -> 'c -> Pos_or_decl.t * string -> unit) ->
  Typing_env_types.env ->
  'c ->
  'a * 'b ->
  Typing_defs.decl_ty ->
  unit

(* checks that a method can be invoked in a dynamic context by ensuring that
   the types of its arguments are enforceable and its return type can be
   coerced to dynamic *)

val sound_dynamic_interface_check :
  Typing_env_types.env ->
  Typing_defs.decl_ty option list ->
  Typing_defs.locl_ty ->
  bool

val sound_dynamic_interface_check_from_fun_ty :
  Typing_env_types.env -> Typing_defs.decl_ty Typing_defs.fun_type -> bool
