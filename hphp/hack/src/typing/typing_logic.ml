(**
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
| IsSubtype of locl ty * locl ty
(* IsEqual(ty1,ty2) if ty1 is equivalent to ty2. Should be same as ty1 <: ty2 /\ ty2 <: ty1 *)
| IsEqual of locl ty * locl ty
(* Conjunction. Conj [] means "true" *)
| Conj of subtype_prop list
(* Disjunction. Disj [] means "false", but use Unsat if you want specific error *)
| Disj of subtype_prop list
(* Equivalent to Disj [], but with an error message function attached.
 * TODO: actually store the error rather than a suspension *)
| Unsat of (unit -> unit)

let rec size (p : subtype_prop) : int =
  match p with
  | IsSubtype _ | IsEqual _ | Unsat _ -> 1
  | Conj l | Disj l ->
    let sizes = List.map l ~f:size in
    List.fold ~init:0 ~f:(+) sizes

(** Sum of the sizes of the disjunctions. *)
let rec n_disj (p : subtype_prop) : int =
  match p with
  | IsSubtype _ | IsEqual _ | Unsat _ -> 0
  | Conj l ->
    let n_disjs = List.map l ~f:n_disj in
    List.fold ~init:0 ~f:(+) n_disjs
  | Disj l ->
    let n_disjs = List.map l ~f:n_disj in
    (List.length l) + (List.fold ~init:0 ~f:(+) n_disjs)

(** Sum of the sizes of the conjunctions. *)
let rec n_conj (p : subtype_prop) : int =
  match p with
  | IsSubtype _ | IsEqual _ | Unsat _ -> 0
  | Disj l ->
    let n_conjs = List.map l ~f:n_conj in
    List.fold ~init:0 ~f:(+) n_conjs
  | Conj l ->
    let n_conjs = List.map l ~f:n_conj in
    (List.length l) + (List.fold ~init:0 ~f:(+) n_conjs)

let rec has_disj p =
  match p with
  | Disj _ -> true
  | IsSubtype _ | IsEqual _ | Unsat _ -> false
  | Conj l ->
    List.exists l ~f:has_disj

let valid = Conj []

(* Is this proposition always true? (e.g. Conj [] but also Disj [Conj []; Unsat _]
* if not simplified
*)
let rec is_valid p =
match p with
| Conj ps -> List.for_all ps is_valid
| Disj ps -> List.exists ps is_valid
| Unsat _ -> false
| IsSubtype (_, _) -> false
| IsEqual (_, _) -> false

(* Is this proposition always false? e.g. Unsat _ but also Conj [Conj []; Unsat _]
* if not simplified
*)
and is_unsat p =
match p with
| Conj ps -> List.exists ps is_unsat
| Disj ps -> List.for_all ps is_unsat
| Unsat _ -> true
| IsSubtype (_, _) -> false
| IsEqual (_, _) -> false

(* Smart constructor for binary conjunction *)
let conj p1 p2 =
  match p1, p2 with
  | Conj [], p
  | p, Conj [] -> p
  | Conj ps1, Conj ps2 -> Conj (ps1 @ ps2)
  (* Preserve the order to maintain legacy behaviour. If two errors share the
   * same position then the first one to be emitted wins.
   * TODO: consider relaxing this behaviour *)
  | Conj ps, _ -> Conj (ps @ [p2])
  | _, Conj ps -> Conj (p1::ps)
  | _, _ -> Conj [p1; p2]

let conj_list ps =
  List.fold ~init:(Conj []) ps ~f:conj

(* Smart constructor for binary disjunction *)
let disj p1 p2 =
  match p1, p2 with
  | p, p' when is_unsat p' -> p
  | p', p when is_unsat p' -> p
  | Conj [], _
  | _, Conj [] -> Conj []
  | Disj ps1, Disj ps2 -> Disj (ps1 @ ps2)
  | _, Disj ps -> Disj (p1::ps)
  | Disj ps, _ -> Disj (p2::ps)
  | _, _ -> Disj [p1; p2]

let disj_list ps =
  List.fold ~init:(Disj []) ps ~f:disj
