(**
 * Copyright (c) 2017, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Typing_defs
module T = Tast

module ExpandedTypeAnnotations = struct
  module ExprAnnotation = T.Annotations.ExprAnnotation
  module EnvAnnotation = Nast.UnitAnnotation
  module ClassIdAnnotation = T.Annotations.ClassIdAnnotation
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
    let _, ety = Tast_env.expand_type env ty in
    match ety with
    | (_, (Tany | Tmixed | Tnonnull | Tprim _ | Tobject | Tdynamic)) -> ety
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
                     ft_mutability; ft_returns_mutable;
                     ft_is_coroutine; ft_ret_by_ref; ft_decl_errors;
                     ft_returns_void_to_rx } =
  { ft_pos; ft_deprecated; ft_arity; ft_abstract; ft_reactive; ft_is_coroutine;
    ft_return_disposable; ft_ret_by_ref; ft_mutability; ft_returns_mutable;
    ft_tparams = List.map exp_tparam ft_tparams;
    ft_where_constraints = List.map exp_where_constraint ft_where_constraints;
    ft_ret = exp_ty ft_ret;
    ft_params = List.map exp_fun_param ft_params;
    ft_decl_errors; ft_returns_void_to_rx;
  }

  and exp_fun_param { fp_pos; fp_name; fp_kind; fp_type; fp_mutability;
                      fp_accept_disposable; fp_rx_condition; } =
  { fp_pos; fp_name; fp_kind; fp_accept_disposable; fp_mutability;
    fp_type = exp_ty fp_type; fp_rx_condition;
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

  and exp_tparam (var, id, cstrs, reified) =
    (var, id, List.map (fun (ck, ty) -> (ck, exp_ty ty)) cstrs, reified)

  and exp_where_constraint (ty1, ck, ty2) = (exp_ty ty1, ck, exp_ty ty2) in
  exp_ty ty

let expand_annotation env (pos, ty) = (pos, expand_ty env ty)

let expander = object
  inherit Tast_visitor.endo
  method! on_expr_annotation = expand_annotation
  method! on_class_id_annotation = expand_annotation
end

module ExpandAST =
  Aast_mapper.MapAnnotatedAST(Tast.Annotations)(ExpandedTypeAnnotations)

(* Replace all types in a program AST by their expansions *)
let expand_program tast =
  let tast = expander#go tast in
  ExpandAST.map_program tast
    ~map_env_annotation:(fun _ -> ())
    ~map_class_id_annotation:(fun _ x -> x)
    ~map_expr_annotation:(fun _ x -> x)
