(*
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Hh_prelude
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
  List.fold_left (List.rev errs) ~init:(Ok []) ~f:(fun acc res ->
      match (acc, res) with
      | (Ok xs, Ok x) -> Ok (x :: xs)
      | (Ok xs, Error (ty_actual, ty_expect)) ->
        Error (ty_actual :: xs, ty_expect :: xs)
      | (Error (xs, ys), Ok x) -> Error (x :: xs, x :: ys)
      | (Error (xs, ys), Error (ty_actual, ty_expect)) ->
        Error (ty_actual :: xs, ty_expect :: ys))

let intersect_errs env errs =
  Result.fold
    ~ok:(fun tys ->
      let (env, ty) = Typing_intersection.intersect_list env Reason.none tys in
      (env, Ok ty))
    ~error:(fun (actuals, expects) ->
      let (env, ty_actual) =
        Typing_intersection.intersect_list env Reason.none actuals
      in
      let (env, ty_expect) =
        Typing_intersection.intersect_list env Reason.none expects
      in
      (env, Error (ty_actual, ty_expect)))
  @@ fold_errs errs

let union_errs env errs =
  Result.fold
    ~ok:(fun tys ->
      let (env, ty) = Typing_union.union_list env Reason.none tys in
      (env, Ok ty))
    ~error:(fun (actuals, expects) ->
      let (env, ty_actual) = Typing_union.union_list env Reason.none actuals in
      let (env, ty_expect) = Typing_union.union_list env Reason.none expects in
      (env, Error (ty_actual, ty_expect)))
  @@ fold_errs errs

(* Here we have a function returns the updated environment, a local type
   and results indicating type errors for both and index expression and
   the rhs expression of an assignment *)
let apply_rules_with_array_index_value_ty_mismatches
    ?(ignore_type_structure = false) ?(preserve_supportdyn = true) env ty f =
  let rec iter ~supportdyn ~is_nonnull env ty =
    let (env, ety) = Env.expand_type env ty in
    (* This is the base case: not a union or intersection or bounded abstract type *)
    let default () = f env ~supportdyn ty in

    match deref ety with
    (* For intersections, collect up all successful applications and compute
     * the intersection of the types and intersection of errors
     *)
    | (r, Tintersection tyl) ->
      let (is_nonnull, ty_err_opt) =
        Typing_solver.is_sub_type env ty (Typing_make_type.nonnull Reason.none)
      in
      Option.iter ~f:(Typing_error_utils.add_typing_error ~env) ty_err_opt;
      let (env, res) =
        Typing_utils.run_on_intersection
          env
          ~f:(iter ~supportdyn ~is_nonnull)
          tyl
      in
      let (tys, arr_errs, key_errs, val_errs) = List.unzip4 res in
      let (env, arr_ty_mismatch) = intersect_errs env arr_errs in
      let (env, key_ty_mismatch) = intersect_errs env key_errs in
      let (env, val_ty_mismatch) = intersect_errs env val_errs in
      let (env, ty) = Typing_intersection.intersect_list env r tys in
      (env, (ty, arr_ty_mismatch, key_ty_mismatch, val_ty_mismatch))
    | (_, Tnewtype (cid, _, _))
      when String.equal cid SN.Classes.cSupportDyn
           && Tast.is_under_dynamic_assumptions env.Typing_env_types.checked ->
      (* If we are under_dynamic_assumptions, we might want to take advantage of
         the dynamic in supportdyn<t>, so don't break it apart as in the next case. *)
      default ()
    (* Preserve supportdyn<_> across operation *)
    | (r, Tnewtype (cid, _, bound)) when String.equal cid SN.Classes.cSupportDyn
      ->
      let (env, (ty, arr_errs, key_errs, val_errs)) =
        iter ~supportdyn:true ~is_nonnull env bound
      in
      let (env, ty) =
        if preserve_supportdyn then
          Typing_utils.make_supportdyn r env ty
        else
          (env, ty)
      in
      (env, (ty, arr_errs, key_errs, val_errs))
    (* For unions, just apply rule of components and compute union of result *)
    | (r, Tunion tyl) ->
      let (env, tys, arr_errs, key_errs, val_errs) =
        List.fold_left
          tyl
          ~init:(env, [], [], [], [])
          ~f:(fun (env, tys, arr_errs, key_errs, val_errs) ty ->
            let (env, (ty, arr_err, key_err, val_err)) =
              iter ~supportdyn ~is_nonnull env ty
            in
            ( env,
              ty :: tys,
              arr_err :: arr_errs,
              key_err :: key_errs,
              val_err :: val_errs ))
      in
      let (env, arr_ty_mismatch) = union_errs env arr_errs in
      let (env, key_ty_mismatch) = union_errs env key_errs in
      let (env, val_ty_mismatch) = union_errs env val_errs in
      let (env, ty) = Typing_union.union_list env r tys in
      (env, (ty, arr_ty_mismatch, key_ty_mismatch, val_ty_mismatch))
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
    | (_, (Tnewtype (_, _, ty) | Tdependent (_, ty))) ->
      iter ~supportdyn ~is_nonnull env ty
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
        iter ~supportdyn ~is_nonnull env ty
    | _ -> default ()
  in
  let (env, (ty, arr_ty_mismatch, key_ty_mismatch, val_ty_mismatch)) =
    iter ~supportdyn:false ~is_nonnull:false env ty
  in
  let opt_of_res =
    Result.fold ~ok:(fun _ -> None) ~error:(fun tys -> Some tys)
  in
  let arr_err_opt = opt_of_res arr_ty_mismatch
  and key_err_opt = opt_of_res key_ty_mismatch
  and val_err_opt = opt_of_res val_ty_mismatch in
  (env, (ty, arr_err_opt, key_err_opt, val_err_opt))

let apply_rules_with_index_value_ty_mismatches
    ?ignore_type_structure ~preserve_supportdyn env ty f =
  let g env ~supportdyn ty =
    let (env, (ty, idx_ty_mismatch, val_ty_mismatch)) = f env ~supportdyn ty in
    (env, (ty, Ok ty, idx_ty_mismatch, val_ty_mismatch))
  in
  let (env, (ty, _, idx_ty_mismatch, val_ty_mismatch)) =
    apply_rules_with_array_index_value_ty_mismatches
      ~preserve_supportdyn
      ?ignore_type_structure
      env
      ty
      g
  in
  (env, (ty, idx_ty_mismatch, val_ty_mismatch))
