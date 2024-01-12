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
open Typing_defs_core
module Reason = Typing_reason

let fresh_env env = env

class type ['env] type_mapper_type =
  object
    method on_type : 'env -> locl_ty -> 'env * locl_ty

    method on_reason : 'env -> Reason.t -> 'env * Reason.t

    method on_tvar : 'env -> Reason.t -> Tvid.t -> 'env * locl_ty

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
      'env -> Reason.t -> locl_phase shape_type -> 'env * locl_ty

    method on_tvec_or_dict :
      'env -> Reason.t -> locl_ty -> locl_ty -> 'env * locl_ty

    method on_taccess : 'env -> Reason.t -> locl_ty -> pos_id -> 'env * locl_ty

    method on_neg_type : 'env -> Reason.t -> neg_type -> 'env * locl_ty

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

    method on_tshape
        env r { s_origin = _; s_unknown_value = shape_kind; s_fields = fdm } =
      ( env,
        mk
          ( r,
            Tshape
              {
                (* TODO(shapes) Should this reset the origin? *)
                s_origin = Missing_origin;
                s_unknown_value = shape_kind;
                s_fields = fdm;
              } ) )

    method on_tvec_or_dict env r ty1 ty2 = (env, mk (r, Tvec_or_dict (ty1, ty2)))

    method on_taccess env r ty id = (env, mk (r, Taccess (ty, id)))

    method on_neg_type env r p = (env, mk (r, Tneg p))

    method on_reason env r = (env, r)

    method on_type env ty =
      let (r, ty) = deref ty in
      let (env, r) = this#on_reason env r in
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
      | Tshape s -> this#on_tshape env r s
      | Tvec_or_dict (ty1, ty2) -> this#on_tvec_or_dict env r ty1 ty2
      | Tunapplied_alias name -> this#on_tunapplied_alias env r name
      | Taccess (ty, id) -> this#on_taccess env r ty id
      | Tneg ty -> this#on_neg_type env r ty

    method on_locl_ty_list env tyl = List.map_env env tyl ~f:this#on_type
  end

(* Implementation of type_mapper that recursively visits everything in the
 * type.
 * NOTE: by default it doesn't to anything to Tvars. Include one of the mixins
 * below to specify how you want to treat type variables. *)
class ['env] deep_type_mapper =
  object (this)
    inherit ['env] shallow_type_mapper

    method! on_tunion env r tyl : 'env * locl_ty =
      let (env, tyl) = List.map_env env tyl ~f:this#on_type in
      (env, mk (r, Tunion tyl))

    method! on_tintersection env r tyl : 'env * locl_ty =
      let (env, tyl) = List.map_env env tyl ~f:this#on_type in
      (env, mk (r, Tintersection tyl))

    method! on_ttuple env r tyl =
      let (env, tyl) = this#on_locl_ty_list env tyl in
      (env, mk (r, Ttuple tyl))

    method! on_toption env r ty =
      let (env, ty) = this#on_type env ty in
      (env, mk (r, Toption ty))

    method private on_tparam env tparam =
      let {
        tp_variance;
        tp_name;
        tp_tparams;
        tp_constraints;
        tp_reified;
        tp_user_attributes;
      } =
        tparam
      in
      let (env, tp_tparams) = List.map_env env tp_tparams ~f:this#on_tparam in
      let (env, tp_constraints) =
        List.map_env env tp_constraints ~f:(fun env (cstr, ty) ->
            let (env, ty) = this#on_type env ty in
            (env, (cstr, ty)))
      in
      let tparam =
        {
          tp_variance;
          tp_name;
          tp_tparams;
          tp_constraints;
          tp_reified;
          tp_user_attributes;
        }
      in
      (env, tparam)

    method private on_where_constraint env cstr =
      let (ty1, kind, ty2) = cstr in
      let (env, ty1) = this#on_type env ty1 in
      let (env, ty2) = this#on_type env ty2 in
      let cstr = (ty1, kind, ty2) in
      (env, cstr)

    method private on_param env param =
      let { fp_pos; fp_name; fp_type; fp_flags; fp_def_value } = param in
      let (env, fp_type) = this#on_type env fp_type in
      let param = { fp_pos; fp_name; fp_type; fp_flags; fp_def_value } in
      (env, param)

    method private on_capability env c =
      match c with
      | CapDefaults p -> (env, CapDefaults p)
      | CapTy ty ->
        let (env, ty) = this#on_type env ty in
        (env, CapTy ty)

    method private on_fun_implicit_params env p =
      let { capability } = p in
      let (env, capability) = this#on_capability env capability in
      let p = { capability } in
      (env, p)

    method! on_tfun env r ft =
      let {
        ft_tparams;
        ft_where_constraints;
        ft_params;
        ft_implicit_params;
        ft_ret;
        ft_flags;
        ft_cross_package;
      } =
        ft
      in
      let (env, ft_tparams) = List.map_env env ft_tparams ~f:this#on_tparam in
      let (env, ft_where_constraints) =
        List.map_env env ft_where_constraints ~f:this#on_where_constraint
      in
      let (env, ft_params) = List.map_env env ft_params ~f:this#on_param in
      let (env, ft_implicit_params) =
        this#on_fun_implicit_params env ft_implicit_params
      in
      let (env, ft_ret) = this#on_type env ft_ret in
      let ft =
        {
          ft_tparams;
          ft_where_constraints;
          ft_params;
          ft_implicit_params;
          ft_ret;
          ft_flags;
          ft_cross_package;
        }
      in
      (env, mk (r, Tfun ft))

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

    method private on_shape_field_type
        env (_sfn : tshape_field_name) (sft : locl_phase shape_field_type) =
      let { sft_optional; sft_ty } = sft in
      let (env, sft_ty) = this#on_type env sft_ty in
      let sft = { sft_optional; sft_ty } in
      (env, sft)

    method! on_tshape env r sh =
      let { s_origin; s_unknown_value; s_fields } = sh in
      let (env, s_unknown_value) = this#on_type env s_unknown_value in
      let (env, s_fields) =
        TShapeMap.map_env this#on_shape_field_type env s_fields
      in
      let sh = { s_origin; s_unknown_value; s_fields } in
      (env, mk (r, Tshape sh))

    method! on_tvec_or_dict env r ty1 ty2 =
      let (env, ty1) = this#on_type env ty1 in
      let (env, ty2) = this#on_type env ty2 in
      (env, mk (r, Tvec_or_dict (ty1, ty2)))

    method! on_taccess env r ty id =
      let (env, ty) = this#on_type env ty in
      (env, mk (r, Taccess (ty, id)))

    method! on_tgeneric env r name args =
      let (env, args) = this#on_locl_ty_list env args in
      (env, mk (r, Tgeneric (name, args)))

    method private on_opt_type env x =
      match x with
      | None -> (env, None)
      | Some x ->
        let (env, x) = this#on_type env x in
        (env, Some x)
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
