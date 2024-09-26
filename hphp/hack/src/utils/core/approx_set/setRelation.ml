(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

type t =
  | Equal
  | Subset
  | Superset
  | Disjoint
  | Unknown

let flip = function
  | Subset -> Superset
  | Superset -> Subset
  | r -> r

(* Derived from Alloy model. See wellformed_set.als *)

(** Let rel be the relation between the sets A and B.
    [complement] determines what a safe approximation of
    !A rel B, given we know A rel B
   *)
let complement = function
  (* A = B => !A disj B *)
  | Equal -> Disjoint
  (* A ⊇ B => !A disj B *)
  | Superset -> Disjoint
  (* A disj B => !A ⊇ B *)
  | Disjoint -> Superset
  (* Otherwise we cannot assume anything *)
  | Subset
  | Unknown ->
    Unknown

(** Let rel be the relation between sets A and B.
    [union] determines what a safe approximation of
    (L ∪ R) rel A, given we know L rel A and R rel A
   *)
let rec union l r =
  match (l, r) with
  | (Equal, rel)
  | (rel, Equal) -> begin
    match rel with
    (* L = A && R ⊆ A => (L ∪ R) = A *)
    | Equal
    | Subset ->
      Equal
    (* L = A => (L ∪ R) ⊇ A for any R *)
    | Superset
    | Disjoint
    | Unknown ->
      Superset
  end
  | (Subset, rel)
  | (rel, Subset) -> begin
    match rel with
    (* (L ∪ R) = (R ∪ L) so it is ok to flip query *)
    | Equal -> union r l
    (* L ⊆ A && R ⊆ A => (L ∪ R) ⊆ A *)
    | Subset -> Subset
    (* L ⊆ A && R ⊇ A => (L ∪ R) ⊇ A *)
    | Superset -> Superset
    (* Otherwise we cannot assume anything *)
    | Disjoint
    | Unknown ->
      Unknown
  end
  | (Superset, rel)
  | (rel, Superset) -> begin
    match rel with
    (* (L ∪ R) = (R ∪ L) so it is ok to flip query *)
    | Equal
    | Subset ->
      union r l
    (* L ⊇ A => (L ∪ R) ⊇ A for any R *)
    | Superset
    | Disjoint
    | Unknown ->
      Superset
  end
  | (Disjoint, rel)
  | (rel, Disjoint) -> begin
    match rel with
    (* (L ∪ R) = (R ∪ L) so it is ok to flip query *)
    | Equal
    | Subset
    | Superset ->
      union r l
    (* L disj A && R disj A => (L ∪ R) disj A *)
    | Disjoint -> Disjoint
    (* Otherwise we cannot assume anything *)
    | Unknown -> Unknown
  end
  (* Otherwise we cannot assume anything *)
  | (Unknown, Unknown) -> Unknown

(** Let rel be the relation between sets A and B.
    [inter] determines what a safe approximation of
    (L ∩ R) rel A, given we know L rel A and R rel A
   *)
let rec inter l r =
  match (l, r) with
  | (Equal, rel)
  | (rel, Equal) -> begin
    match rel with
    (* L = A && R ⊇ A => (L ∩ R) = A *)
    | Equal
    | Superset ->
      Equal
    (* L = A && R disj A => (L ∩ R) disj A *)
    | Disjoint -> Disjoint
    (* L = A => (L ∩ R) ⊆ A for any R *)
    | Subset
    | Unknown ->
      Subset
  end
  | (Subset, rel)
  | (rel, Subset) -> begin
    match rel with
    (* (L ∩ R) = (R ∩ L) so it is ok to flip query *)
    | Equal -> inter r l
    (* L ⊆ A && R disj A => (L ∩ R) disj A *)
    | Disjoint -> Disjoint
    (* L ⊆ A => (L ∩ R) ⊆ A for any R *)
    | Subset
    | Superset
    | Unknown ->
      Subset
  end
  | (Superset, rel)
  | (rel, Superset) -> begin
    match rel with
    (* (L ∩ R) = (R ∩ L) so it is ok to flip query *)
    | Equal
    | Subset ->
      inter r l
    (* L ⊇ A && R ⊇ A => (L ∩ R) ⊇ A *)
    | Superset -> Superset
    (* L ⊇ A && R disj A => (L ∩ R) disj A *)
    | Disjoint -> Disjoint
    (* Otherwise we cannot assume anything *)
    | Unknown -> Unknown
  end
  | (Disjoint, rel)
  | (rel, Disjoint) -> begin
    match rel with
    (* (L ∩ R) = (R ∩ L) so it is ok to flip query *)
    | Equal
    | Subset
    | Superset ->
      inter r l
    (* L disj A => (L ∩ R) disj A for any R *)
    | Disjoint -> Disjoint
    | Unknown -> Disjoint
  end
  (* Otherwise we cannot assume anything *)
  | (Unknown, Unknown) -> Unknown
