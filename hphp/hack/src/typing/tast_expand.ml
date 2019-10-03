(*
 * Copyright (c) 2017, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Core_kernel
open Typing_defs
module T = Aast

(* Eliminate residue of type inference:
 *   1. Tvars are replaced (deep) by the expanded type
 *   2. Tanon is replaced by a Tfun function type
 *   3. Singleton unions are eliminated
 * TODO TAST:
 *   Transform completely unconstrained types to Tmixed
 *   Consider using a fresh datatype for TAST types.
 *)
let expand_ty ?var_hook ?pos env ty =
  let rec exp_ty ty =
    begin
      match (ty, var_hook) with
      | ((_, Tvar var), Some hook) -> hook var
      | _ -> ()
    end;
    let (_, ety) = Tast_env.expand_type env ty in
    let ety =
      match ety with
      | (_, (Tany _ | Tnonnull | Tprim _ | Tobject | Tdynamic)) -> ety
      | (p, Tclass (n, e, tyl)) -> (p, Tclass (n, e, exp_tys tyl))
      | (p, Tunion tyl) -> (p, Tunion (exp_tys tyl))
      | (p, Tintersection tyl) -> (p, Tintersection (exp_tys tyl))
      | (p, Toption ty) -> (p, Toption (exp_ty ty))
      | (p, Ttuple tyl) -> (p, Ttuple (exp_tys tyl))
      | (p, Tdestructure tyl) -> (p, Tdestructure (exp_tys tyl))
      | (p, Tfun ft) -> (p, Tfun (exp_fun_type ft))
      | (p, Tabstract (ak, tyopt)) ->
        (p, Tabstract (exp_abstract_kind ak, Option.map tyopt exp_ty))
      | (p, Tshape (shape_kind, fields)) ->
        (p, Tshape (shape_kind, Nast.ShapeMap.map exp_sft fields))
      | (p, Tarraykind ak) -> (p, Tarraykind (exp_array_kind ak))
      | (p, Tvar v) ->
        (match pos with
        | None -> (p, Tvar v)
        | Some pos ->
          if
            TypecheckerOptions.disallow_unresolved_type_variables
              (Tast_env.get_tcopt env)
          then
            Errors.unresolved_type_variable pos;
          (p, Tvar v))
      (* TODO TAST: replace with Tfun type *)
      | (p, Tanon (x, y)) -> (p, Tanon (x, y))
      | (p, Terr) -> (p, Terr)
      (* TODO(T36532263) see if that needs updating *)
      | (p, Tpu (base, enum, kind)) -> (p, Tpu (exp_ty base, enum, kind))
      | (p, Tpu_access (base, sid)) -> (p, Tpu_access (exp_ty base, sid))
    in
    let (_env, ety) = Tast_env.simplify_unions env ety in
    ety
  and exp_tys tyl = List.map ~f:exp_ty tyl
  and exp_fun_type
      {
        ft_pos;
        ft_arity;
        ft_tparams;
        ft_where_constraints;
        ft_ret;
        ft_fun_kind;
        ft_params;
        ft_reactive;
        ft_return_disposable;
        ft_mutability;
        ft_returns_mutable;
        ft_is_coroutine;
        ft_returns_void_to_rx;
      } =
    {
      ft_pos;
      ft_arity;
      ft_fun_kind;
      ft_reactive;
      ft_is_coroutine;
      ft_return_disposable;
      ft_mutability;
      ft_returns_mutable;
      ft_tparams = Tuple.T2.map_fst ~f:(List.map ~f:exp_tparam) ft_tparams;
      ft_where_constraints =
        List.map ~f:exp_where_constraint ft_where_constraints;
      ft_ret = exp_possibly_enforced_ty ft_ret;
      ft_params = List.map ~f:exp_fun_param ft_params;
      ft_returns_void_to_rx;
    }
  and exp_fun_param
      {
        fp_pos;
        fp_name;
        fp_kind;
        fp_type;
        fp_mutability;
        fp_accept_disposable;
        fp_rx_annotation;
      } =
    {
      fp_pos;
      fp_name;
      fp_kind;
      fp_accept_disposable;
      fp_mutability;
      fp_type = exp_possibly_enforced_ty fp_type;
      fp_rx_annotation;
    }
  and exp_possibly_enforced_ty { et_type; et_enforced } =
    { et_type = exp_ty et_type; et_enforced }
  and exp_sft { sft_optional; sft_ty } =
    { sft_optional; sft_ty = exp_ty sft_ty }
  and exp_array_kind ak =
    match ak with
    | AKany -> AKany
    | AKvarray ty -> AKvarray (exp_ty ty)
    | AKvec ty -> AKvec (exp_ty ty)
    | AKdarray (ty1, ty2) -> AKdarray (exp_ty ty1, exp_ty ty2)
    | AKvarray_or_darray ty -> AKvarray_or_darray (exp_ty ty)
    | AKmap (ty1, ty2) -> AKmap (exp_ty ty1, exp_ty ty2)
    | AKempty -> AKempty
  and exp_abstract_kind ak =
    match ak with
    | AKnewtype (n, tyl) -> AKnewtype (n, exp_tys tyl)
    | AKgeneric _
    | AKdependent _ ->
      ak
  and exp_tparam t =
    {
      t with
      tp_constraints =
        List.map ~f:(fun (ck, ty) -> (ck, exp_ty ty)) t.tp_constraints;
    }
  and exp_where_constraint (ty1, ck, ty2) = (exp_ty ty1, ck, exp_ty ty2) in
  exp_ty ty

let expander =
  object
    inherit Tast_visitor.endo

    method! on_'ex env (pos, ty) = (pos, expand_ty ~pos env ty)

    method! on_'hi env ty = expand_ty env ty
  end

(* Replace all types in a program AST by their expansions *)
let expand_program tast = expander#go tast
