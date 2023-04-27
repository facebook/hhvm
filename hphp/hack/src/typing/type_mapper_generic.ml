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

open Common
open Typing_defs
module Reason = Typing_reason

let fresh_env env = env

class type ['env] type_mapper_type =
  object
    method on_tvar : 'env -> Reason.t -> int -> 'env * locl_ty

    method on_tnonnull : 'env -> Reason.t -> 'env * locl_ty

    method on_tdynamic : 'env -> Reason.t -> 'env * locl_ty

    method on_tany : 'env -> Reason.t -> 'env * locl_ty

    method on_tprim : 'env -> Reason.t -> Aast.tprim -> 'env * locl_ty

    method on_ttuple : 'env -> Reason.t -> locl_ty list -> 'env * locl_ty

    method on_tunion : 'env -> Reason.t -> locl_ty list -> 'env * locl_ty

    method on_tintersection : 'env -> Reason.t -> locl_ty list -> 'env * locl_ty

    method on_toption : 'env -> Reason.t -> locl_ty -> 'env * locl_ty

    method on_tfun : 'env -> Reason.t -> locl_fun_type -> 'env * locl_ty

    method on_tgeneric :
      'env -> Reason.t -> string -> locl_ty list -> 'env * locl_ty

    method on_tunapplied_alias : 'env -> Reason.t -> string -> 'env * locl_ty

    method on_tnewtype :
      'env -> Reason.t -> string -> locl_ty list -> locl_ty -> 'env * locl_ty

    method on_tdependent :
      'env -> Reason.t -> dependent_type -> locl_ty -> 'env * locl_ty

    method on_tclass :
      'env -> Reason.t -> pos_id -> exact -> locl_ty list -> 'env * locl_ty

    method on_tshape :
      'env ->
      Reason.t ->
      locl_ty ->
      locl_phase shape_field_type TShapeMap.t ->
      'env * locl_ty

    method on_tvec_or_dict :
      'env -> Reason.t -> locl_ty -> locl_ty -> 'env * locl_ty

    method on_taccess : 'env -> Reason.t -> locl_ty -> pos_id -> 'env * locl_ty

    method on_neg_type : 'env -> Reason.t -> neg_type -> 'env * locl_ty

    method on_type : 'env -> locl_ty -> 'env * locl_ty

    method on_locl_ty_list : 'env -> locl_ty list -> 'env * locl_ty list
  end

(* Base type mapper implementation that doesn't recursively go into the
 * types. *)
class ['env] shallow_type_mapper : ['env] type_mapper_type =
  object (this)
    method on_tvar env r n = (env, mk (r, Tvar n))

    method on_tnonnull env r = (env, mk (r, Tnonnull))

    method on_tdynamic env r = (env, mk (r, Tdynamic))

    method on_tany env r = (env, mk (r, Typing_defs.make_tany ()))

    method on_tprim env r p = (env, mk (r, Tprim p))

    method on_ttuple env r tyl = (env, mk (r, Ttuple tyl))

    method on_tunion env r tyl = (env, mk (r, Tunion tyl))

    method on_tintersection env r tyl = (env, mk (r, Tintersection tyl))

    method on_toption env r ty = (env, mk (r, Toption ty))

    method on_tfun env r fun_type = (env, mk (r, Tfun fun_type))

    method on_tgeneric env r name args = (env, mk (r, Tgeneric (name, args)))

    method on_tunapplied_alias env r name = (env, mk (r, Tunapplied_alias name))

    method on_tnewtype env r name tyl ty =
      (env, mk (r, Tnewtype (name, tyl, ty)))

    method on_tdependent env r dep ty = (env, mk (r, Tdependent (dep, ty)))

    method on_tclass env r x e tyl = (env, mk (r, Tclass (x, e, tyl)))

    method on_tshape env r shape_kind fdm =
      (env, mk (r, Tshape (Missing_origin, shape_kind, fdm)))

    method on_tvec_or_dict env r ty1 ty2 = (env, mk (r, Tvec_or_dict (ty1, ty2)))

    method on_taccess env r ty id = (env, mk (r, Taccess (ty, id)))

    method on_neg_type env r p = (env, mk (r, Tneg p))

    method on_type env ty =
      let (r, ty) = deref ty in
      match ty with
      | Tvar n -> this#on_tvar env r n
      | Tnonnull -> this#on_tnonnull env r
      | Tany _ -> this#on_tany env r
      | Tprim p -> this#on_tprim env r p
      | Ttuple tyl -> this#on_ttuple env r tyl
      | Tunion tyl -> this#on_tunion env r tyl
      | Tintersection tyl -> this#on_tintersection env r tyl
      | Toption ty -> this#on_toption env r ty
      | Tfun fun_type -> this#on_tfun env r fun_type
      | Tgeneric (x, args) -> this#on_tgeneric env r x args
      | Tnewtype (x, tyl, ty) -> this#on_tnewtype env r x tyl ty
      | Tdependent (x, ty) -> this#on_tdependent env r x ty
      | Tclass (x, e, tyl) -> this#on_tclass env r x e tyl
      | Tdynamic -> this#on_tdynamic env r
      | Tshape (_, shape_kind, fdm) -> this#on_tshape env r shape_kind fdm
      | Tvec_or_dict (ty1, ty2) -> this#on_tvec_or_dict env r ty1 ty2
      | Tunapplied_alias name -> this#on_tunapplied_alias env r name
      | Taccess (ty, id) -> this#on_taccess env r ty id
      | Tneg ty -> this#on_neg_type env r ty

    method on_locl_ty_list env tyl = List.map_env env tyl ~f:this#on_type
  end

(* Mixin class - adding it to shallow type mapper creates a mapper that
 * traverses the type by going inside Tunion *)
class virtual ['env] tunion_type_mapper =
  object (this)
    method on_tunion env r tyl : 'env * locl_ty =
      let (env, tyl) = this#on_locl_ty_list env tyl in
      (env, mk (r, Tunion tyl))

    method virtual on_locl_ty_list : 'env -> locl_ty list -> 'env * locl_ty list
  end

class virtual ['env] tinter_type_mapper =
  object (this)
    method on_tintersection env r tyl : 'env * locl_ty =
      let (env, tyl) = this#on_locl_ty_list env tyl in
      (env, mk (r, Tintersection tyl))

    method virtual on_locl_ty_list : 'env -> locl_ty list -> 'env * locl_ty list
  end

(* Mixin that expands type variables. *)
class virtual ['env] tvar_expanding_type_mapper =
  object (this)
    method on_tvar (env, expand) r n =
      let (env, ty) = expand env r n in
      if is_tyvar ty then
        ((env, expand), ty)
      else
        this#on_type (env, expand) ty

    method virtual on_type
        : 'env * ('env -> Reason.t -> int -> 'env * locl_ty) ->
          locl_ty ->
          ('env * ('env -> Reason.t -> int -> 'env * locl_ty)) * locl_ty
  end

(* Mixin that maps across the type inside the typevar, and then changes
 * its value to the result. *)
class virtual ['env] tvar_substituting_type_mapper =
  object (this)
    method on_tvar
        ((env, expand, add) :
          'env
          * ('env -> Reason.t -> int -> 'env * locl_ty)
          * ('env -> int -> locl_ty -> 'env))
        (r : Reason.t)
        (n : int) =
      let (env, ty) = expand env r n in
      if is_tyvar ty then
        (env, ty)
      else
        let ((env, _expand, add), ty) = this#on_type (env, expand, add) ty in
        let env = add env n ty in
        (env, ty)

    method virtual on_type
        : 'env
          * ('env -> Reason.t -> int -> 'env * locl_ty)
          * ('env -> int -> locl_ty -> 'env) ->
          locl_ty ->
          ('env
          * ('env -> Reason.t -> int -> 'env * locl_ty)
          * ('env -> int -> locl_ty -> 'env))
          * locl_ty
  end

(* Implementation of type_mapper that recursively visits everything in the
 * type.
 * NOTE: by default it doesn't to anything to Tvars. Include one of the mixins
 * below to specify how you want to treat type variables. *)
class ['env] deep_type_mapper =
  object (this)
    inherit ['env] shallow_type_mapper

    inherit! ['env] tunion_type_mapper

    inherit! ['env] tinter_type_mapper

    method! on_ttuple env r tyl =
      let (env, tyl) = this#on_locl_ty_list env tyl in
      (env, mk (r, Ttuple tyl))

    method! on_toption env r ty =
      let (env, ty) = this#on_type env ty in
      (env, mk (r, Toption ty))

    method! on_tfun env r ft =
      let on_param env param =
        let (env, ty) = this#on_possibly_enforced_ty env param.fp_type in
        (env, { param with fp_type = ty })
      in
      let (env, params) = List.map_env env ft.ft_params ~f:on_param in
      let (env, ret) = this#on_possibly_enforced_ty env ft.ft_ret in
      (env, mk (r, Tfun { ft with ft_params = params; ft_ret = ret }))

    method! on_tnewtype env r x tyl cstr =
      let (env, tyl) = List.map_env env tyl ~f:this#on_type in
      let (env, cstr) = this#on_type env cstr in
      (env, mk (r, Tnewtype (x, tyl, cstr)))

    method! on_tdependent env r x cstr =
      let (env, cstr) = this#on_type env cstr in
      (env, mk (r, Tdependent (x, cstr)))

    method! on_tclass env r x e tyl =
      let (env, tyl) = this#on_locl_ty_list env tyl in
      (env, mk (r, Tclass (x, e, tyl)))

    method! on_tshape env r shape_kind fdm =
      let (env, fdm) = ShapeFieldMap.map_env this#on_type env fdm in
      (env, mk (r, Tshape (Missing_origin, shape_kind, fdm)))

    method private on_opt_type env x =
      match x with
      | None -> (env, None)
      | Some x ->
        let (env, x) = this#on_type env x in
        (env, Some x)

    method private on_possibly_enforced_ty env x =
      let (env, et_type) = this#on_type env x.et_type in
      (env, { x with et_type })
  end

class type ['env] constraint_type_mapper_type =
  object
    method on_constraint_type :
      'env -> constraint_type -> 'env * constraint_type

    method on_constraint_type_ :
      'env -> Reason.t -> constraint_type_ -> 'env * constraint_type

    method on_Thas_member :
      'env -> Reason.t -> has_member -> 'env * constraint_type

    method on_Thas_type_member :
      'env -> Reason.t -> has_type_member -> 'env * constraint_type

    method on_Tcan_index :
      'env -> Reason.t -> can_index -> 'env * constraint_type

    method on_Tcan_traverse :
      'env -> Reason.t -> can_traverse -> 'env * constraint_type

    method on_Tdestructure :
      'env -> Reason.t -> destructure -> 'env * constraint_type

    method on_TCunion :
      'env -> Reason.t -> locl_ty -> constraint_type -> 'env * constraint_type

    method on_TCintersection :
      'env -> Reason.t -> locl_ty -> constraint_type -> 'env * constraint_type
  end

class type ['env] locl_constraint_type_mapper_type =
  object
    inherit ['env] constraint_type_mapper_type

    inherit ['env] type_mapper_type
  end

class ['env] constraint_type_mapper : ['env] locl_constraint_type_mapper_type =
  object (this)
    inherit ['env] deep_type_mapper

    method on_constraint_type env ty =
      let (r, ty) = deref_constraint_type ty in
      this#on_constraint_type_ env r ty

    method on_constraint_type_ env r ty_ =
      match ty_ with
      | Thas_member hm -> this#on_Thas_member env r hm
      | Thas_type_member htm -> this#on_Thas_type_member env r htm
      | Tcan_index ci -> this#on_Tcan_index env r ci
      | Tcan_traverse ct -> this#on_Tcan_traverse env r ct
      | Tdestructure tyl -> this#on_Tdestructure env r tyl
      | TCunion (lty, cty) -> this#on_TCunion env r lty cty
      | TCintersection (lty, cty) -> this#on_TCintersection env r lty cty

    method on_Thas_member env r hm =
      let { hm_name; hm_type; hm_class_id; hm_explicit_targs } = hm in
      let (env, hm_type) = this#on_type env hm_type in
      let hm = { hm_name; hm_type; hm_class_id; hm_explicit_targs } in
      (env, mk_constraint_type (r, Thas_member hm))

    method on_Thas_type_member env r htm =
      let { htm_id; htm_lower; htm_upper } = htm in
      let (env, htm_lower) = this#on_type env htm_lower in
      let (env, htm_upper) = this#on_type env htm_upper in
      let htm = { htm_id; htm_lower; htm_upper } in
      (env, mk_constraint_type (r, Thas_type_member htm))

    method on_Tcan_index env r ci =
      let { ci_key; ci_shape; ci_val; ci_expr_pos; ci_index_pos } = ci in
      let (env, ci_key) = this#on_type env ci_key in
      let (env, ci_val) = this#on_type env ci_val in
      let ci = { ci_key; ci_shape; ci_val; ci_expr_pos; ci_index_pos } in
      (env, mk_constraint_type (r, Tcan_index ci))

    method on_Tcan_traverse env r ct =
      let { ct_key; ct_val; ct_is_await; ct_reason } = ct in
      let (env, ct_key) = this#on_opt_type env ct_key in
      let (env, ct_val) = this#on_type env ct_val in
      let ct = { ct_key; ct_val; ct_is_await; ct_reason } in
      (env, mk_constraint_type (r, Tcan_traverse ct))

    method on_Tdestructure env r { d_required; d_optional; d_variadic; d_kind }
        =
      let (env, d_required) = this#on_locl_ty_list env d_required in
      let (env, d_optional) = this#on_locl_ty_list env d_optional in
      let (env, d_variadic) = this#on_opt_type env d_variadic in
      ( env,
        mk_constraint_type
          (r, Tdestructure { d_required; d_optional; d_variadic; d_kind }) )

    method on_TCunion env r lty cty =
      let (env, lty) = this#on_type env lty in
      let (env, cty) = this#on_constraint_type env cty in
      (env, mk_constraint_type (r, TCunion (lty, cty)))

    method on_TCintersection env r lty cty =
      let (env, lty) = this#on_type env lty in
      let (env, cty) = this#on_constraint_type env cty in
      (env, mk_constraint_type (r, TCintersection (lty, cty)))
  end

class type ['env] internal_type_mapper_type =
  object
    inherit ['env] locl_constraint_type_mapper_type

    method on_internal_type : 'env -> internal_type -> 'env * internal_type

    method on_LoclType : 'env -> locl_ty -> 'env * internal_type

    method on_ConstraintType : 'env -> constraint_type -> 'env * internal_type
  end

class ['env] internal_type_mapper : ['env] internal_type_mapper_type =
  object (this)
    inherit ['env] constraint_type_mapper

    method on_internal_type env ty =
      match ty with
      | LoclType ty -> this#on_LoclType env ty
      | ConstraintType ty -> this#on_ConstraintType env ty

    method on_LoclType env ty =
      let (env, ty) = this#on_type env ty in
      (env, LoclType ty)

    method on_ConstraintType env ty =
      let (env, ty) = this#on_constraint_type env ty in
      (env, ConstraintType ty)
  end
