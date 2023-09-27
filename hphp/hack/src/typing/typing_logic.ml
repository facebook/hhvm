(*
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Hh_prelude
open Typing_defs

type coercion_direction =
  | CoerceToDynamic
  | CoerceFromDynamic
[@@deriving show, eq]

(* See comment in .mli file *)
type subtype_prop =
  | IsSubtype of coercion_direction option * internal_type * internal_type
  | Conj of subtype_prop list
  | Disj of Typing_error.t option * subtype_prop list
[@@deriving show]

let rec equal_subtype_prop p1 p2 =
  match (p1, p2) with
  | (IsSubtype (c1, ty1, ty1'), IsSubtype (c2, ty2, ty2')) ->
    Option.equal equal_coercion_direction c1 c2
    && equal_internal_type ty1 ty2
    && equal_internal_type ty1' ty2'
  | (Conj ps1, Conj ps2)
  | (Disj (_, ps1), Disj (_, ps2)) ->
    Int.equal (List.length ps1) (List.length ps2)
    && List.for_all2_exn ps1 ps2 ~f:equal_subtype_prop
  | (_, (IsSubtype _ | Conj _ | Disj _)) -> false

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
  | Conj ps -> List.for_all ps ~f:is_valid
  | Disj (_, ps) -> List.exists ps ~f:is_valid
  | IsSubtype _ -> false

(* Is this proposition always false? e.g. Unsat _ but also Conj [Conj []; Disj (_, [])]
   * if not simplified
*)
and is_unsat p =
  match p with
  | Conj ps -> List.exists ps ~f:is_unsat
  | Disj (_, ps) -> List.for_all ps ~f:is_unsat
  | IsSubtype _ -> false

let rec get_error_if_unsat p =
  match p with
  | Disj (err, ps) ->
    if List.for_all ps ~f:is_unsat then
      err
    else
      None
  | Conj ps -> List.find_map ps ~f:get_error_if_unsat
  | IsSubtype _ -> None

(* Smart constructor for binary conjunction *)
let conj p1 p2 =
  if equal_subtype_prop p1 p2 then
    p1
  else
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

let add_prop p ps =
  if List.exists ps ~f:(equal_subtype_prop p) then
    ps
  else
    p :: ps

(* Smart constructor for binary disjunction *)
let disj ~fail p1 p2 =
  if equal_subtype_prop p1 p2 then
    p1
  else
    match (p1, p2) with
    | (_, _) when is_valid p1 || is_valid p2 -> Conj []
    | (_, _) when is_unsat p1 && is_unsat p2 -> Disj (fail, [])
    | (_, _) when is_unsat p1 -> Disj (fail, [p2])
    | (_, _) when is_unsat p2 -> Disj (fail, [p1])
    | (Disj (_, ps1), Disj (_, ps2)) ->
      Disj (fail, List.fold ps2 ~init:ps1 ~f:(fun ps p -> add_prop p ps))
    | (_, Disj (_, ps)) -> Disj (fail, add_prop p1 ps)
    | (Disj (_, ps), _) -> Disj (fail, add_prop p2 ps)
    | (_, _) -> Disj (fail, [p1; p2])

let rec force_lazy_values (prop : subtype_prop) =
  match prop with
  | IsSubtype (dir, ty1, ty2) ->
    IsSubtype
      ( dir,
        Type_force_lazy_values.internal_type ty1,
        Type_force_lazy_values.internal_type ty2 )
  | Conj props -> Conj (List.map props ~f:force_lazy_values)
  | Disj (_err, props) ->
    Disj
      ( (* TODO force lazy values in error *) None,
        List.map props ~f:force_lazy_values )
