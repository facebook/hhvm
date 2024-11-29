(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

type attribute_interface_name = string

type new_object_checker =
  Pos.t ->
  Typing_env_types.env ->
  Typing_defs.pos_string ->
  Nast.argument list ->
  Typing_env_types.env

(** Checks a list of user attributes:
    - checks that the attributes are on the proper kind of element,
      by checking subtyping of the attribute class with the provided attribute interface
      (e.g. FileAttribute, MethodAttribute, see Naming_special_names for a full list)
    - checks for unbound attribute names
    - runs the provided new_object_checker, which checks creating an instance of the attribute class. *)
val check_def :
  Typing_env_types.env ->
  new_object_checker ->
  attribute_interface_name ->
  Nast.user_attribute list ->
  Typing_env_types.env
