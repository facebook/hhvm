(*
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

class type ['a] decl_type_visitor_type =
  object
    method on_tany : 'a -> Typing_reason.decl_t -> 'a

    method on_tmixed : 'a -> Typing_reason.decl_t -> 'a

    method on_twildcard : 'a -> Typing_reason.decl_t -> 'a

    method on_tnonnull : 'a -> Typing_reason.decl_t -> 'a

    method on_tdynamic : 'a -> Typing_reason.decl_t -> 'a

    method on_tthis : 'a -> Typing_reason.decl_t -> 'a

    method on_tvec_or_dict :
      'a ->
      Typing_reason.decl_t ->
      Typing_defs.decl_ty ->
      Typing_defs.decl_ty ->
      'a

    method on_tgeneric :
      'a -> Typing_reason.decl_t -> string -> Typing_defs.decl_ty list -> 'a

    method on_toption : 'a -> Typing_reason.decl_t -> Typing_defs.decl_ty -> 'a

    method on_tlike : 'a -> Typing_reason.decl_t -> Typing_defs.decl_ty -> 'a

    method on_tprim : 'a -> Typing_reason.decl_t -> Aast.tprim -> 'a

    method on_tvar : 'a -> Typing_reason.decl_t -> Tvid.t -> 'a

    method on_type : 'a -> Typing_defs.decl_ty -> 'a

    method on_tfun :
      'a -> Typing_reason.decl_t -> Typing_defs.decl_fun_type -> 'a

    method on_tapply :
      'a ->
      Typing_reason.decl_t ->
      Typing_defs.pos_id ->
      Typing_defs.decl_ty list ->
      'a

    method on_ttuple :
      'a -> Typing_reason.decl_t -> Typing_defs.decl_ty list -> 'a

    method on_tunion :
      'a -> Typing_reason.decl_t -> Typing_defs.decl_ty list -> 'a

    method on_tintersection :
      'a -> Typing_reason.decl_t -> Typing_defs.decl_ty list -> 'a

    method on_tshape :
      'a ->
      Typing_reason.decl_t ->
      Typing_defs.decl_phase Typing_defs.shape_type ->
      'a

    method on_taccess :
      'a ->
      Typing_reason.decl_t ->
      Typing_defs.decl_phase Typing_defs.taccess_type ->
      'a

    method on_trefinement :
      'a ->
      Typing_reason.decl_t ->
      Typing_defs.decl_ty ->
      Typing_defs.decl_class_refinement ->
      'a

    method on_tnewtype :
      'a ->
      Typing_reason.decl_t ->
      string ->
      Typing_defs.decl_ty list ->
      Typing_defs.decl_ty ->
      'a
  end

class virtual ['a] decl_type_visitor : ['a] decl_type_visitor_type

class type ['a] locl_type_visitor_type =
  object
    method on_tany : 'a -> Typing_reason.t -> 'a

    method on_tnonnull : 'a -> Typing_reason.t -> 'a

    method on_tdynamic : 'a -> Typing_reason.t -> 'a

    method on_toption : 'a -> Typing_reason.t -> Typing_defs.locl_ty -> 'a

    method on_tprim : 'a -> Typing_reason.t -> Aast.tprim -> 'a

    method on_tvar : 'a -> Typing_reason.t -> Tvid.t -> 'a

    method on_type : 'a -> Typing_defs.locl_ty -> 'a

    method on_tfun : 'a -> Typing_reason.t -> Typing_defs.locl_fun_type -> 'a

    method on_tgeneric :
      'a -> Typing_reason.t -> string -> Typing_defs.locl_ty list -> 'a

    method on_tnewtype :
      'a ->
      Typing_reason.t ->
      string ->
      Typing_defs.locl_ty list ->
      Typing_defs.locl_ty ->
      'a

    method on_tdependent :
      'a ->
      Typing_reason.t ->
      Typing_defs.dependent_type ->
      Typing_defs.locl_ty ->
      'a

    method on_ttuple : 'a -> Typing_reason.t -> Typing_defs.locl_ty list -> 'a

    method on_tunion : 'a -> Typing_reason.t -> Typing_defs.locl_ty list -> 'a

    method on_tintersection :
      'a -> Typing_reason.t -> Typing_defs.locl_ty list -> 'a

    method on_tunion : 'a -> Typing_reason.t -> Typing_defs.locl_ty list -> 'a

    method on_tintersection :
      'a -> Typing_reason.t -> Typing_defs.locl_ty list -> 'a

    method on_tvec_or_dict :
      'a -> Typing_reason.t -> Typing_defs.locl_ty -> Typing_defs.locl_ty -> 'a

    method on_tshape :
      'a ->
      Typing_reason.t ->
      Typing_defs.locl_phase Typing_defs.shape_type ->
      'a

    method on_tclass :
      'a ->
      Typing_reason.t ->
      Typing_defs.pos_id ->
      Typing_defs.exact ->
      Typing_defs.locl_ty list ->
      'a

    method on_class_refinement : 'a -> Typing_defs.locl_class_refinement -> 'a

    method on_tlist : 'a -> Typing_reason.t -> Typing_defs.locl_ty list -> 'a

    method on_tunapplied_alias : 'a -> Typing_reason.t -> string -> 'a

    method on_taccess :
      'a ->
      Typing_reason.t ->
      Typing_defs.locl_phase Typing_defs.taccess_type ->
      'a

    method on_neg_type : 'a -> Typing_reason.t -> Typing_defs.neg_type -> 'a
  end

class virtual ['a] locl_type_visitor : ['a] locl_type_visitor_type

class type ['a] internal_type_visitor_type =
  object
    inherit ['a] locl_type_visitor_type

    method on_internal_type : 'a -> Typing_defs.internal_type -> 'a

    method on_constraint_type : 'a -> Typing_defs.constraint_type -> 'a

    method on_locl_type : 'a -> Typing_defs.locl_ty -> 'a

    method on_locl_type_list : 'a -> Typing_defs.locl_ty list -> 'a

    method on_locl_type_option : 'a -> Typing_defs.locl_ty option -> 'a

    method on_thas_member :
      'a -> Typing_reason.t -> Typing_defs.has_member -> 'a

    method on_has_member : 'a -> Typing_reason.t -> Typing_defs.has_member -> 'a

    method on_thas_type_member :
      'a -> Typing_reason.t -> Typing_defs.has_type_member -> 'a

    method on_has_type_member :
      'a -> Typing_reason.t -> Typing_defs.has_type_member -> 'a

    method on_tcan_index : 'a -> Typing_reason.t -> Typing_defs.can_index -> 'a

    method on_tcan_traverse :
      'a -> Typing_reason.t -> Typing_defs.can_traverse -> 'a

    method on_can_index : 'a -> Typing_reason.t -> Typing_defs.can_index -> 'a

    method on_can_traverse :
      'a -> Typing_reason.t -> Typing_defs.can_traverse -> 'a

    method on_tdestructure :
      'a -> Typing_reason.t -> Typing_defs.destructure -> 'a

    method on_destructure :
      'a -> Typing_reason.t -> Typing_defs.destructure -> 'a

    method on_tcunion :
      'a ->
      Typing_reason.t ->
      Typing_defs.locl_ty ->
      Typing_defs.constraint_type ->
      'a

    method on_tcintersection :
      'a ->
      Typing_reason.t ->
      Typing_defs.locl_ty ->
      Typing_defs.constraint_type ->
      'a
  end

class ['a] internal_type_visitor : ['a] internal_type_visitor_type
