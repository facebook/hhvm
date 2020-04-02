(*
 * Copyright (c) 2017, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Hh_prelude
open Typing_defs
module T = Aast
module MakeType = Typing_make_type

(* Eliminate residue of type inference:
 *   1. Tvars are replaced (deep) by the expanded type
 *   2. Singleton unions are eliminated
 * TODO TAST:
 *   Transform completely unconstrained types to Tmixed
 *   Consider using a fresh datatype for TAST types.
 *)
let expand_ty ?var_hook ?pos env ty =
  let rec exp_ty ty =
    begin
      match (get_node ty, var_hook) with
      | (Tvar var, Some hook) -> hook var
      | _ -> ()
    end;
    let (_, ety) = Tast_env.expand_type env ty in
    let ety =
      match deref ety with
      | (_, (Tany _ | Tnonnull | Tprim _ | Tobject | Tdynamic | Tgeneric _)) ->
        ety
      | (p, Tclass (n, e, tyl)) -> mk (p, Tclass (n, e, exp_tys tyl))
      | (p, Tunion tyl) -> mk (p, Tunion (exp_tys tyl))
      | (p, Tintersection tyl) -> mk (p, Tintersection (exp_tys tyl))
      | (p, Toption ty) -> mk (p, Toption (exp_ty ty))
      | (p, Ttuple tyl) -> mk (p, Ttuple (exp_tys tyl))
      | (p, Tfun ft) -> mk (p, Tfun (exp_fun_type ft))
      | (p, Tnewtype (n, tyl, ty)) ->
        mk (p, Tnewtype (n, exp_tys tyl, exp_ty ty))
      | (p, Tdependent (n, ty)) -> mk (p, Tdependent (n, exp_ty ty))
      | (p, Tshape (shape_kind, fields)) ->
        mk (p, Tshape (shape_kind, Nast.ShapeMap.map exp_sft fields))
      | (p, Tarraykind ak) -> mk (p, Tarraykind (exp_array_kind ak))
      | (p, Tvar v) ->
        (match pos with
        | None -> mk (p, Tvar v)
        | Some pos ->
          if
            TypecheckerOptions.disallow_unresolved_type_variables
              (Tast_env.get_tcopt env)
          then
            Errors.unresolved_type_variable pos;
          mk (p, Tvar v))
      | (p, Terr) -> MakeType.err p
      (* TODO(T36532263) see if that needs updating *)
      | (p, Tpu (base, enum)) -> mk (p, Tpu (exp_ty base, enum))
      | (p, Tpu_type_access (base, enum, member, tyname)) ->
        let base = exp_ty base in
        mk (p, Tpu_type_access (base, enum, member, tyname))
    in
    ety
  and exp_tys tyl = List.map ~f:exp_ty tyl
  and exp_fun_type
      {
        ft_arity;
        ft_tparams;
        ft_where_constraints;
        ft_ret;
        ft_flags;
        ft_params;
        ft_reactive;
      } =
    {
      ft_arity;
      ft_flags;
      ft_reactive;
      ft_tparams = List.map ~f:exp_tparam ft_tparams;
      ft_where_constraints =
        List.map ~f:exp_where_constraint ft_where_constraints;
      ft_ret = exp_possibly_enforced_ty ft_ret;
      ft_params = List.map ~f:exp_fun_param ft_params;
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
    | AKvarray ty -> AKvarray (exp_ty ty)
    | AKdarray (ty1, ty2) -> AKdarray (exp_ty ty1, exp_ty ty2)
    | AKvarray_or_darray (ty1, ty2) ->
      AKvarray_or_darray (exp_ty ty1, exp_ty ty2)
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
