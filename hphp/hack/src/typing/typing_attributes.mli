(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

type attribute_interface_name = string

(** Checks a list of user attributes:
    - checks that the attributes are on the proper kind of element,
      by checking subtyping of the attribute class with the provided attribute interface
      (e.g. FileAttribute, MethodAttribute, see Naming_special_names for a full list)
    - checks for unbound attribute names
    - runs the provided function, which checks creating an instance of the attribute class. *)
val check_def :
  Typing_env_types.env ->
  (expected:'a option ->
  check_parent:bool ->
  check_not_abstract:bool ->
  is_using_clause:bool ->
  Pos.t ->
  Typing_env_types.env ->
  Nast.class_id_ ->
  'b list ->
  (Ast_defs.param_kind * Nast.expr) list ->
  'c option ->
  Typing_env_types.env * 'l * 'm * 'n * 'o * 'p * 'q * 'r) ->
  attribute_interface_name ->
  Nast.user_attribute list ->
  Typing_env_types.env
