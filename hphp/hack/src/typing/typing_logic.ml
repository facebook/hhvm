(*
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Core_kernel
open Typing_defs

(* Logical proposition about types *)
type subtype_prop =
  (* IsSubtype(ty1,ty2) if ty1 is a subtype of ty2, written ty1 <: ty2 *)
  | IsSubtype of locl_ty * locl_ty
  (* Conjunction. Conj [] means "true" *)
  | Conj of subtype_prop list
  (* Disjunction. Disj f [] means "false".  The error message function f
   * wraps the error that should be produced in this case. *)
  | Disj of (unit -> unit) * subtype_prop list

let rec size (p : subtype_prop) : int =
  match p with
  | IsSubtype _ -> 1
  | Conj l
  | Disj (_, l) ->
    let sizes = List.map l ~f:size in
    List.fold ~init:0 ~f:( + ) sizes

(** Sum of the sizes of the disjunctions. *)
let rec n_disj (p : subtype_prop) : int =
  match p with
  | IsSubtype _ -> 0
  | Conj l ->
    let n_disjs = List.map l ~f:n_disj in
    List.fold ~init:0 ~f:( + ) n_disjs
  | Disj (_, l) ->
    let n_disjs = List.map l ~f:n_disj in
    List.length l + List.fold ~init:0 ~f:( + ) n_disjs

(** Sum of the sizes of the conjunctions. *)
let rec n_conj (p : subtype_prop) : int =
  match p with
  | IsSubtype _ -> 0
  | Disj (_, l) ->
    let n_conjs = List.map l ~f:n_conj in
    List.fold ~init:0 ~f:( + ) n_conjs
  | Conj l ->
    let n_conjs = List.map l ~f:n_conj in
    List.length l + List.fold ~init:0 ~f:( + ) n_conjs

let valid = Conj []

let invalid ~fail = Disj (fail, [])

(* Is this proposition always true? (e.g. Conj [] but also Disj [Conj []; Disj (_, [])]
* if not simplified
*)
let rec is_valid p =
  match p with
  | Conj ps -> List.for_all ps is_valid
  | Disj (_, ps) -> List.exists ps is_valid
  | IsSubtype (_, _) -> false

(* Is this proposition always false? e.g. Unsat _ but also Conj [Conj []; Disj (_, [])]
* if not simplified
*)
and is_unsat p =
  match p with
  | Conj ps -> List.exists ps is_unsat
  | Disj (_, ps) -> List.for_all ps is_unsat
  | IsSubtype (_, _) -> false

(* Smart constructor for binary conjunction *)
let conj p1 p2 =
  match (p1, p2) with
  | (Conj [], p)
  | (p, Conj []) ->
    p
  | (Conj ps1, Conj ps2) -> Conj (ps1 @ ps2)
  (* Preserve the order to maintain legacy behaviour. If two errors share the
   * same position then the first one to be emitted wins.
   * TODO: consider relaxing this behaviour *)
  | (Conj ps, _) -> Conj (ps @ [p2])
  | (_, Conj ps) -> Conj (p1 :: ps)
  | (_, _) -> Conj [p1; p2]

let conj_list ps = List.fold ~init:(Conj []) ps ~f:conj

(* Smart constructor for binary disjunction *)
let disj ~fail p1 p2 =
  match (p1, p2) with
  | (_, _) when is_valid p1 || is_valid p2 -> Conj []
  | (_, _) when is_unsat p1 && is_unsat p2 -> Disj (fail, [])
  | (_, _) when is_unsat p1 -> Disj (fail, [p2])
  | (_, _) when is_unsat p2 -> Disj (fail, [p1])
  | (Disj (_, ps1), Disj (_, ps2)) -> Disj (fail, ps1 @ ps2)
  | (_, Disj (_, ps)) -> Disj (fail, p1 :: ps)
  | (Disj (_, ps), _) -> Disj (fail, p2 :: ps)
  | (_, _) -> Disj (fail, [p1; p2])
