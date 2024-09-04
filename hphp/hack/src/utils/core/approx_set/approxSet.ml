(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

module Set_relation = struct
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
end

module type DomainType = sig
  type t

  type ctx

  val relation : t -> ctx:ctx -> t -> Set_relation.t
end

module type S = sig
  module Domain : DomainType

  type t

  val empty : t

  val singleton : Domain.t -> t

  val union : t -> t -> t

  val inter : t -> t -> t

  val diff : t -> t -> t

  val of_list : Domain.t list -> t

  type disjoint =
    | Sat
    | Unsat of {
        left: Domain.t;
        relation: Set_relation.t;
        right: Domain.t;
      }

  val disjoint : Domain.ctx -> t -> t -> disjoint

  val are_disjoint : Domain.ctx -> t -> t -> bool

  val relate : Domain.ctx -> t -> t -> Set_relation.t
end

module Make (Domain : DomainType) : S with module Domain := Domain = struct
  type disjoint =
    | Sat
    | Unsat of {
        left: Domain.t;
        relation: Set_relation.t;
        right: Domain.t;
      }

  (* Sets over [Domain.t]; representation is in NNF by construction *)
  module Impl = struct
    (* Represents an individual atom in our abstract domain. [elt] is the particular element in
       the domain that is uniquely indentified by [Domain.t]. [comp] indicates whether it is the
       complement of the [Domain.t]. i.e.

         {
           comp = false;
           elt = [A]
         }
       Means all elements represented by A, while:
         {
           comp = true;
           elt = [A]
         }
       Means all elements not represented by A
    *)
    type atom = {
      comp: bool;
      elt: Domain.t;
    }

    type t =
      | Set of atom
      | Union of t * t
      | Inter of t * t

    let singleton elt = Set { comp = false; elt }

    let rec relate ctx set1 set2 =
      match (set1, set2) with
      | (Set { comp = false; elt = elt1 }, Set { comp = false; elt = elt2 }) ->
        Domain.relation ~ctx elt1 elt2
      | (Union (left, right), set) ->
        Set_relation.union (relate ctx left set) (relate ctx right set)
      | (Inter (left, right), set) ->
        Set_relation.inter (relate ctx left set) (relate ctx right set)
      | (Set { comp = true; elt }, set) ->
        Set_relation.complement (relate ctx (singleton elt) set)
      | ( Set { comp = false; elt },
          ((Union _ | Inter _ | Set { comp = true; elt = _ }) as set) ) ->
        Set_relation.flip (relate ctx set (singleton elt))

    let flip_unsat = function
      | Sat -> Sat
      | Unsat { left; relation; right } ->
        Unsat
          { left = right; relation = Set_relation.flip relation; right = left }

    let disjoint_atom atom1 atom2 ~ctx =
      match relate ctx (Set atom1) (Set atom2) with
      | Set_relation.Disjoint -> Sat
      | relation -> Unsat { left = atom1.elt; relation; right = atom2.elt }

    let rec disjoint ctx set1 set2 =
      match (set1, set2) with
      (* (L ∪ R) disj S if (L disj S) && (R disj S) *)
      | (Union (l, r), set) -> begin
        match disjoint ctx l set with
        | Sat -> disjoint ctx r set
        | Unsat _ as unsat -> unsat
      end
      (* (L ∩ R) disj S if (L disj S) || (R disj S) *)
      | (Inter (l, r), set) -> begin
        match disjoint ctx l set with
        | Sat -> Sat
        | Unsat _ -> disjoint ctx r set
      end
      | (Set atom1, Set atom2) -> disjoint_atom atom1 atom2 ~ctx
      | (Set _, (Union _ | Inter _)) -> disjoint ctx set2 set1 |> flip_unsat

    let union l r = Union (l, r)

    let inter l r = Inter (l, r)

    (*  Keep values in negation normal form by construction *)
    let rec comp = function
      (* !(!A) = A *)
      | Set atom -> Set { atom with comp = not atom.comp }
      (* De Morgan's Law: !(A ∪ B) = !A ∩ !B *)
      | Union (a, b) -> inter (comp a) (comp b)
      (* De Morgan's Law: !(A ∩ B) = !A ∪ !B *)
      | Inter (a, b) -> union (comp a) (comp b)

    (* A ∖ B = A ∩ !B *)
    let diff a b = inter a (comp b)
  end

  open Impl

  (* We encode an empty set using [Option.None] *)
  type nonrec t = t option

  let empty = None

  let singleton tag = Some (singleton tag)

  let union set1 set2 =
    match (set1, set2) with
    | (set, None)
    | (None, set) ->
      set
    | (Some a, Some b) -> Some (union a b)

  let inter set1 set2 =
    match (set1, set2) with
    | (_, None)
    | (None, _) ->
      None
    | (Some a, Some b) -> Some (inter a b)

  let diff set1 set2 =
    match (set1, set2) with
    | (set, None) -> set
    | (None, _) -> None
    | (Some a, Some b) -> Some (diff a b)

  let of_list elt =
    List.fold_left (fun acc tag -> union acc @@ singleton tag) empty elt

  let disjoint ctx set1 set2 =
    match (set1, set2) with
    | (None, _)
    | (_, None) ->
      Sat
    | (Some set1, Some set2) -> disjoint ctx set1 set2

  let are_disjoint ctx set1 set2 =
    match disjoint ctx set1 set2 with
    | Sat -> true
    | Unsat _ -> false

  let relate ctx set1 set2 =
    match (set1, set2) with
    | (None, None) -> Set_relation.Equal
    | (None, Some _)
    | (Some _, None) ->
      Set_relation.Disjoint
    | (Some set1, Some set2) -> relate ctx set1 set2
end
