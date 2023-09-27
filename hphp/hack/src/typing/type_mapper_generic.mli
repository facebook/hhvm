(*
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

[@@@warning "-33"]

open Hh_prelude

[@@@warning "+33"]

val fresh_env : 'env -> 'env

class type ['env] type_mapper_type =
  object
    method on_type : 'env -> Typing_defs.locl_ty -> 'env * Typing_defs.locl_ty

    method on_reason : 'env -> Typing_reason.t -> 'env * Typing_reason.t

    method on_tvar :
      'env -> Typing_reason.t -> int -> 'env * Typing_defs.locl_ty

    method on_tnonnull : 'env -> Typing_reason.t -> 'env * Typing_defs.locl_ty

    method on_tdynamic : 'env -> Typing_reason.t -> 'env * Typing_defs.locl_ty

    method on_tany : 'env -> Typing_reason.t -> 'env * Typing_defs.locl_ty

    method on_tprim :
      'env -> Typing_reason.t -> Aast.tprim -> 'env * Typing_defs.locl_ty

    method on_ttuple :
      'env ->
      Typing_reason.t ->
      Typing_defs.locl_ty list ->
      'env * Typing_defs.locl_ty

    method on_tunion :
      'env ->
      Typing_reason.t ->
      Typing_defs.locl_ty list ->
      'env * Typing_defs.locl_ty

    method on_tintersection :
      'env ->
      Typing_reason.t ->
      Typing_defs.locl_ty list ->
      'env * Typing_defs.locl_ty

    method on_toption :
      'env ->
      Typing_reason.t ->
      Typing_defs.locl_ty ->
      'env * Typing_defs.locl_ty

    method on_tfun :
      'env ->
      Typing_reason.t ->
      Typing_defs.locl_fun_type ->
      'env * Typing_defs.locl_ty

    method on_tgeneric :
      'env ->
      Typing_reason.t ->
      string ->
      Typing_defs.locl_ty list ->
      'env * Typing_defs.locl_ty

    method on_tunapplied_alias :
      'env -> Typing_reason.t -> string -> 'env * Typing_defs.locl_ty

    method on_tnewtype :
      'env ->
      Typing_reason.t ->
      string ->
      Typing_defs.locl_ty list ->
      Typing_defs.locl_ty ->
      'env * Typing_defs.locl_ty

    method on_tdependent :
      'env ->
      Typing_reason.t ->
      Typing_defs.dependent_type ->
      Typing_defs.locl_ty ->
      'env * Typing_defs.locl_ty

    method on_tclass :
      'env ->
      Typing_reason.t ->
      Typing_defs.pos_id ->
      Typing_defs.exact ->
      Typing_defs.locl_ty list ->
      'env * Typing_defs.locl_ty

    method on_tshape :
      'env ->
      Typing_reason.t ->
      Typing_defs.locl_phase Typing_defs.shape_type ->
      'env * Typing_defs.locl_ty

    method on_tvec_or_dict :
      'env ->
      Typing_reason.t ->
      Typing_defs.locl_ty ->
      Typing_defs.locl_ty ->
      'env * Typing_defs.locl_ty

    method on_taccess :
      'env ->
      Typing_reason.t ->
      Typing_defs.locl_ty ->
      Typing_defs.pos_id ->
      'env * Typing_defs.locl_ty

    method on_neg_type :
      'env ->
      Typing_reason.t ->
      Typing_defs.neg_type ->
      'env * Typing_defs.locl_ty

    method on_locl_ty_list :
      'env -> Typing_defs.locl_ty list -> 'env * Typing_defs.locl_ty list
  end

(* Base type mapper implementation that doesn't recursively go into the
 * types. *)
class ['env] shallow_type_mapper : ['env] type_mapper_type

(* Implementation of type_mapper that recursively visits everything in the
 * type.
 * NOTE: by default it doesn't do anything to Tvars. Include one of the mixins
 * below to specify how you want to treat type variables. *)
class ['env] deep_type_mapper : ['env] type_mapper_type

class type ['env] constraint_type_mapper_type =
  object
    method on_constraint_type :
      'env -> Typing_defs.constraint_type -> 'env * Typing_defs.constraint_type

    method on_constraint_type_ :
      'env ->
      Typing_reason.t ->
      Typing_defs.constraint_type_ ->
      'env * Typing_defs.constraint_type

    method on_Thas_member :
      'env ->
      Typing_reason.t ->
      Typing_defs.has_member ->
      'env * Typing_defs.constraint_type

    method on_Thas_type_member :
      'env ->
      Typing_reason.t ->
      Typing_defs.has_type_member ->
      'env * Typing_defs.constraint_type

    method on_Tcan_index :
      'env ->
      Typing_reason.t ->
      Typing_defs.can_index ->
      'env * Typing_defs.constraint_type

    method on_Tcan_traverse :
      'env ->
      Typing_reason.t ->
      Typing_defs.can_traverse ->
      'env * Typing_defs.constraint_type

    method on_Tdestructure :
      'env ->
      Typing_reason.t ->
      Typing_defs.destructure ->
      'env * Typing_defs.constraint_type

    method on_TCunion :
      'env ->
      Typing_reason.t ->
      Typing_defs.locl_ty ->
      Typing_defs.constraint_type ->
      'env * Typing_defs.constraint_type

    method on_TCintersection :
      'env ->
      Typing_reason.t ->
      Typing_defs.locl_ty ->
      Typing_defs.constraint_type ->
      'env * Typing_defs.constraint_type
  end

class type ['env] locl_constraint_type_mapper_type =
  object
    inherit ['env] constraint_type_mapper_type

    inherit ['env] type_mapper_type
  end

class ['env] constraint_type_mapper : ['env] locl_constraint_type_mapper_type

class type ['env] internal_type_mapper_type =
  object
    inherit ['env] locl_constraint_type_mapper_type

    method on_internal_type :
      'env -> Typing_defs.internal_type -> 'env * Typing_defs.internal_type

    method on_LoclType :
      'env -> Typing_defs.locl_ty -> 'env * Typing_defs.internal_type

    method on_ConstraintType :
      'env -> Typing_defs.constraint_type -> 'env * Typing_defs.internal_type
  end

class ['env] internal_type_mapper : ['env] internal_type_mapper_type
