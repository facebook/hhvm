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

let apply_rules ?(ignore_type_structure = false) env ty f =
  let rec iter ~is_nonnull env ty =
    let (env, ety) = Env.expand_type env ty in
    (* This is the base case: not a union or intersection or bounded abstract type *)
    let default () = f env ty in
    match deref ety with
    (* For intersections, collect up all successful applications and compute
     * the intersection of the types.
     *)
    | (r, Tintersection tyl) ->
      let is_nonnull =
        Typing_solver.is_sub_type env ty (Typing_make_type.nonnull Reason.none)
      in
      let (env, resl) =
        Typing_utils.run_on_intersection env (iter ~is_nonnull) tyl
      in
      Typing_intersection.intersect_list env r resl
    (* For unions, just apply rule of components and compute union of result *)
    | (r, Tunion tyl) ->
      let (env, resl) = List.map_env env tyl (iter ~is_nonnull) in
      Typing_union.union_list env r resl
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
  iter ~is_nonnull:false env ty
