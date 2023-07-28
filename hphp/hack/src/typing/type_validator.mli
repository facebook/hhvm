(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

type validity

type reification =
  | Resolved
  | Unresolved

type validation_state = {
  env: Typing_env_types.env;
  ety_env: Typing_defs.expand_env;
  validity: validity;
  inside_reified_class_generic_position: bool;
  reification: reification;
  expanded_typedefs: SSet.t;
}

type error_emitter = Pos.t -> (Pos_or_decl.t * string) list Lazy.t -> unit

class virtual type_validator :
  object
    inherit [validation_state] Type_visitor.decl_type_visitor_type

    method invalid :
      validation_state -> Typing_reason.decl_t -> string -> validation_state

    method invalid_list :
      validation_state ->
      (Typing_reason.decl_t * string) list ->
      validation_state

    method on_alias :
      validation_state ->
      Typing_reason.decl_t ->
      Pos_or_decl.t * string ->
      Typing_defs.decl_ty list ->
      Typing_defs.decl_ty ->
      validation_state

    method on_class :
      validation_state ->
      Typing_reason.decl_t ->
      Pos_or_decl.t * string ->
      Typing_defs.decl_ty list ->
      validation_state

    method on_enum :
      validation_state ->
      Typing_reason.decl_t ->
      Pos_or_decl.t * string ->
      validation_state

    method on_newtype :
      validation_state ->
      Typing_reason.decl_t ->
      Pos_or_decl.t * string ->
      Typing_defs.decl_ty list ->
      Typing_defs.decl_ty ->
      Typing_defs.decl_ty ->
      Typing_defs.decl_ty ->
      validation_state

    method on_taccess :
      validation_state ->
      Typing_reason.decl_t ->
      Typing_defs.decl_phase Typing_defs.taccess_type ->
      validation_state

    method on_typeconst :
      validation_state ->
      Decl_provider.Class.t ->
      Typing_defs.typeconst_type ->
      validation_state

    method validate_hint :
      Typing_env_types.env ->
      Aast.hint ->
      ?reification:reification ->
      error_emitter ->
      unit

    method validate_type :
      Typing_env_types.env ->
      Pos.t ->
      Typing_defs.decl_ty ->
      ?reification:reification ->
      error_emitter ->
      unit
  end
