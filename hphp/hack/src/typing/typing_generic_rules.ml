(*
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Common
open Typing_defs
module Env = Typing_env
module TySet = Typing_set
module SN = Naming_special_names

(* Return upper bounds of a generic parameter, transitively following bounds
 * that are themselves parameters. Detect cycles. *)
let get_transitive_upper_bounds env ty =
  let rec iter seen env acc tyl =
    match tyl with
    | [] -> (env, acc)
    | ty :: tyl ->
      let (env, ety) = Env.expand_type env ty in
      (match get_node ety with
      | Tgeneric (n, tyargs) ->
        if SSet.mem n seen then
          iter seen env acc tyl
        else
          iter
            (SSet.add n seen)
            env
            acc
            (TySet.elements (Env.get_upper_bounds env n tyargs) @ tyl)
      | _ -> iter seen env (TySet.add ty acc) tyl)
  in
  let (env, resl) = iter SSet.empty env TySet.empty [ty] in
  (env, TySet.elements resl)

(* Apply generic typing rules. Here, assume E[e:t] is some expression with a
 * subexpression e of type t. For example, E[x:C] is `x->method(e1,..,en)` (method
 * invocation on object of class C) or E[x:vec<int>] is `x[e1] = e2` (array
 * assignment on a vec<int>).
 * 1. Lift unions: if e:t1|t2 and E[e:t1] : u1 and E[e:t2] : u2
 *    then E[e:t1|t2] : u1|u2. (Note: both sides of union must succeed.)
 * 2. Lift intersections: if e:t1&t2 and E[e:t1] : u1 and E[e:t2] : u2
 *    then E[e:t1&t2] : u1&u2. If E[e:t1] : u1 but E[e:t2] does not type,
 *    then E[e:t1&t2] : u1 (and similar for the other way round).
 * 3. Apply subsumption wrt upper bounds on generics and abstract types:
 *    If e:T and generic parameter T has upper bound t and E[e:t] : u,
 *    then E[e:T] : u.
 * For the implementation below, ty is the type of the subexpression e,
 * and f is the checker for the context E[e:ty].
 * We iterate the rule through intersections, unions, and bounds.
 *)
let is_shape t =
  match get_node t with
  | Tshape _ -> true
  | _ -> false

let is_arraykey t =
  match get_node t with
  | Tprim Aast.Tarraykey -> true
  | _ -> false

let fold_errs errs =
  Result.map ~f:List.rev
  @@ Result.map_error ~f:(fun (acts, exps) -> List.(rev acts, rev exps))
  @@ List.fold_left errs ~init:(Ok []) ~f:(fun acc res ->
         match (acc, res) with
         | (Ok xs, Ok x) -> Ok (x :: xs)
         | (Ok xs, Error (ty_actual, ty_expect)) ->
           Error (ty_actual :: xs, ty_expect :: xs)
         | (Error (xs, ys), Ok x) -> Error (x :: xs, x :: ys)
         | (Error (xs, ys), Error (ty_actual, ty_expect)) ->
           Error (ty_actual :: xs, ty_expect :: ys))

let intersect_errs errs =
  Result.fold
    ~ok:(fun tys -> Ok (Typing_make_type.intersection Reason.none tys))
    ~error:(fun (actuals, expects) ->
      Error
        ( Typing_make_type.intersection Reason.none actuals,
          Typing_make_type.intersection Reason.none expects ))
  @@ fold_errs errs

let union_errs errs =
  Result.fold
    ~ok:(fun tys -> Ok (Typing_make_type.union Reason.none tys))
    ~error:(fun (actuals, expects) ->
      Error
        ( Typing_make_type.union Reason.none actuals,
          Typing_make_type.union Reason.none expects ))
  @@ fold_errs errs

(*  Here we have a function returns the updated environment, a local type
    and results indicating type errors for both and index expression and
    the rhs expression of an assignment *)
let apply_rules_with_index_value_errs ?(ignore_type_structure = false) env ty f
    =
  let rec iter ~is_nonnull env ty =
    let (env, ety) = Env.expand_type env ty in
    (* This is the base case: not a union or intersection or bounded abstract type *)
    let default () = f env ty in
    match deref ety with
    (* For intersections, collect up all successful applications and compute
     * the intersection of the types and intersection of errors
     *)
    | (r, Tintersection tyl) ->
      let is_nonnull =
        Typing_solver.is_sub_type env ty (Typing_make_type.nonnull Reason.none)
      in
      let (env, tys, key_errs, errs) =
        Typing_utils.run_on_intersection_key_value_res
          env
          ~f:(iter ~is_nonnull)
          tyl
      in
      let err_res = intersect_errs errs
      and key_err_res = intersect_errs key_errs in
      let (env, ty) = Typing_intersection.intersect_list env r tys in
      (env, ty, key_err_res, err_res)
    (* For unions, just apply rule of components and compute union of result *)
    | (r, Tunion tyl) ->
      let (env, tys, key_errs, errs) =
        List.fold_left
          tyl
          ~init:(env, [], [], [])
          ~f:(fun (env, tys, key_errs, errs) ty ->
            let (env, ty, key_err, err) = iter ~is_nonnull env ty in
            (env, ty :: tys, key_err :: key_errs, err :: errs))
      in
      let err_res = union_errs errs and key_err_res = union_errs key_errs in
      let (env, ty) = Typing_union.union_list env r tys in
      (env, ty, key_err_res, err_res)
    (* Special case for `TypeStructure<_> as shape { ... }`, because some clients care about the
     * fact that we have the special type TypeStructure *)
    | (_, Tnewtype (cid, _, bound))
      when String.equal cid SN.FB.cTypeStructure
           && is_shape bound
           && ignore_type_structure ->
      default ()
    (* Enums with arraykey upper bound are treated as "abstract" *)
    | (_, Tnewtype (cid, _, bound))
      when Env.is_enum env cid && is_arraykey bound ->
      default ()
    (* If there is an explicit upper bound, delegate to it *)
    | (_, (Tnewtype (_, _, ty) | Tdependent (_, ty))) -> iter ~is_nonnull env ty
    (* For type parameters with upper bounds, delegate to intersection of the upper bounds *)
    | (r, Tgeneric _) ->
      let (env, tyl) = get_transitive_upper_bounds env ty in
      if List.is_empty tyl then
        default ()
      else
        let (env, ty) = Typing_intersection.intersect_list env r tyl in
        let (env, ty) =
          if is_nonnull then
            Typing_solver.non_null env (get_pos ty) ty
          else
            (env, ty)
        in
        iter ~is_nonnull env ty
    | _ -> default ()
  in
  let (env, ty, key_err_res, err_res) = iter ~is_nonnull:false env ty in
  let err_opt =
    Result.fold err_res ~ok:(fun _ -> None) ~error:(fun tys -> Some tys)
  and key_err_opt =
    Result.fold key_err_res ~ok:(fun _ -> None) ~error:(fun tys -> Some tys)
  in
  (env, ty, key_err_opt, err_opt)

(*  Here we have a function returns the updated environment, a local type
    and a result indicating type errors for the rhs of an assignment *)
let apply_rules_with_err ?ignore_type_structure env ty f =
  let g env ty =
    let (env, ty, err_res) = f env ty in
    (env, ty, Ok ty, err_res)
  in
  let (env, ty, _, err_res) =
    apply_rules_with_index_value_errs ?ignore_type_structure env ty g
  in
  (env, ty, err_res)

let apply_rules ?ignore_type_structure env ty f =
  let g env ty =
    let (env, ty) = f env ty in
    (env, ty, Ok ty)
  in
  let (env, ty, _) = apply_rules_with_err ?ignore_type_structure env ty g in
  (env, ty)
