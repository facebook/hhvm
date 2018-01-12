(**
 * Copyright (c) 2017, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 *
 *)

open Typing_defs
module T = Tast

module ExpandedTypeAnnotations = struct
  module ExprAnnotation = T.Annotations.ExprAnnotation
  module EnvAnnotation = Nast.UnitAnnotation
end

module ExpandedTypeAnnotatedAST = Nast.AnnotatedAST(ExpandedTypeAnnotations)
module ETast = ExpandedTypeAnnotatedAST

(* Eliminate residue of type inference:
 *   1. Tvars are replaced (deep) by the expanded type
 *   2. Tanon is replaced by a Tfun function type
 *   3. Singleton unions are eliminated
 * TODO TAST:
 *   Transform completely unconstrained types to Tmixed
 *   Consider using a fresh datatype for TAST types.
 *)
let expand_ty env ty =
  let rec exp_ty ty =
    let _, ety = Typing_env.expand_type env ty in
    match ety with
    | (_, (Tany | Tmixed | Tprim _ | Tobject)) -> ety
    | (p, Tclass(n, tyl)) -> (p, Tclass(n, exp_tys tyl))
    | (_, Tunresolved [ty]) -> exp_ty ty
    | (p, Tunresolved tyl) -> (p, Tunresolved (exp_tys tyl))
    | (p, Toption ty) -> (p, Toption (exp_ty ty))
    | (p, Ttuple tyl) -> (p, Ttuple (exp_tys tyl))
    | (p, Tfun ft) -> (p, Tfun (exp_fun_type ft))
    | (p, Tabstract (ak, tyopt)) ->
      (p, Tabstract (exp_abstract_kind ak, Option.map tyopt exp_ty))
    | (p, Tshape (fk, fields)) ->
      (p, Tshape (fk, Nast.ShapeMap.map exp_sft fields))
    | (p, Tarraykind ak) ->
      (p, Tarraykind (exp_array_kind ak))
      (* TODO TAST: replace with a user error *)
    | (p, Tvar v) -> (p, Tvar v)
      (* TODO TAST: replace with Tfun type *)
    | (p, Tanon(x, y)) -> (p, Tanon(x,y))
    | (p, Terr) -> (p, Terr)

  and exp_tys tyl = List.map exp_ty tyl

  and exp_fun_type { ft_pos; ft_deprecated; ft_arity; ft_abstract; ft_tparams;
                     ft_where_constraints; ft_ret; ft_params;
                     ft_reactive; ft_return_disposable;
                     ft_mutable; ft_returns_mutable;
                     ft_is_coroutine; ft_ret_by_ref } =
  { ft_pos; ft_deprecated; ft_arity; ft_abstract; ft_reactive; ft_is_coroutine;
    ft_return_disposable; ft_ret_by_ref; ft_mutable; ft_returns_mutable;
    ft_tparams = List.map exp_tparam ft_tparams;
    ft_where_constraints = List.map exp_where_constraint ft_where_constraints;
    ft_ret = exp_ty ft_ret;
    ft_params = List.map exp_fun_param ft_params;
  }

  and exp_fun_param { fp_pos; fp_name; fp_kind; fp_type; fp_mutable;
                      fp_accept_disposable } =
  { fp_pos; fp_name; fp_kind; fp_accept_disposable; fp_mutable;
    fp_type = exp_ty fp_type;
  }

  and exp_sft { sft_optional; sft_ty } =
  { sft_optional;
    sft_ty = exp_ty sft_ty;
  }

  and exp_array_kind ak =
    match ak with
    | AKany -> AKany
    | AKvarray ty -> AKvarray (exp_ty ty)
    | AKvec ty -> AKvec (exp_ty ty)
    | AKdarray (ty1, ty2) -> AKdarray (exp_ty ty1, exp_ty ty2)
    | AKvarray_or_darray ty -> AKvarray_or_darray (exp_ty ty)
    | AKmap (ty1, ty2) -> AKmap (exp_ty ty1, exp_ty ty2)
    | AKempty -> AKempty
    | AKtuple tm -> AKtuple (IMap.map exp_ty tm)
    | AKshape s -> AKshape (Nast.ShapeMap.map
      (fun (ty1, ty2) -> (exp_ty ty1, exp_ty ty2)) s)

  and exp_abstract_kind ak =
    match ak with
    | AKnewtype(n, tyl) -> AKnewtype(n, exp_tys tyl)
    | AKenum _ | AKgeneric _ | AKdependent _ -> ak

  and exp_tparam (var, id, cstrs) =
    (var, id, List.map (fun (ck, ty) -> (ck, exp_ty ty)) cstrs)

  and exp_where_constraint (ty1, ck, ty2) = (exp_ty ty1, ck, exp_ty ty2) in
  exp_ty ty

let expand_annotation env (pos, tyopt) =
  (pos, Option.map tyopt (expand_ty env))

let restore_saved_env env saved_env =
  let module Env = Typing_env in
  {env with
    Env.genv = {env.Env.genv with Env.tcopt = saved_env.Tast.tcopt};
    Env.tenv = saved_env.Tast.tenv;
    Env.subst = saved_env.Tast.subst;
  }

module ExpandAST =
  Aast_mapper.MapAnnotatedAST(Tast.Annotations)(ExpandedTypeAnnotations)

(* Replace all types in a program AST by their expansions *)
let expand_program env =
  ExpandAST.map_program
    ~map_env_annotation:(fun _ -> ())
    ~map_expr_annotation:begin fun saved_env annotation ->
      let env = restore_saved_env env saved_env in
      expand_annotation env annotation
    end
