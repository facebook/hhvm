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
end

module type DomainType = sig
  type t

  type ctx

  val relation : t -> ctx:ctx -> t -> Set_relation.t
end

module type S = sig
  module Domain : DomainType

  type t

  val singleton : Domain.t -> t

  val union : t -> t -> t

  val inter : t -> t -> t

  val diff : t -> t -> t

  type disjoint =
    | Sat
    | Unsat of Domain.t * Domain.t

  val disjoint : Domain.ctx -> t -> t -> disjoint
end

(* To keep the logic simple we do not perform any simplification during
   construction of the set. Instead specific simplification rules are
   applied when computing [disjoint] *)
module Make (Domain : DomainType) : S with module Domain := Domain = struct
  type t =
    | Set of Domain.t
    | Union of t * t
    | Inter of t * t
    | Compl of t

  type disjoint =
    | Sat
    | Unsat of Domain.t * Domain.t

  let singleton elt = Set elt

  let rec disjoint ctx set1 set2 =
    let open Set_relation in
    match (set1, set2) with
    (* (L ∪ R) disj S if (L disj S) && (R disj S) *)
    | (Union (l, r), set) ->
      let result =
        match disjoint ctx l set with
        | Sat -> disjoint ctx r set
        | Unsat _ as unsat -> unsat
      in
      result
    (* (L ∩ R) disj S if (L disj S) || (R disj S) *)
    | (Inter (l, r), set) ->
      let result =
        match disjoint ctx l set with
        | Sat -> Sat
        | Unsat _ -> disjoint ctx r set
      in
      result
    (* !(!A) = A *)
    | (Compl (Compl a), b) -> disjoint ctx a b
    (* De Morgan's Law: !(A ∪ B) = !A ∩ !B *)
    | (Compl (Union (a, b)), set) -> disjoint ctx (Inter (Compl a, Compl b)) set
    (* De Morgan's Law: !(A ∩ B) = !A ∪ !B *)
    | (Compl (Inter (a, b)), set) -> disjoint ctx (Union (Compl a, Compl b)) set
    | (Set elt1, Set elt2) ->
      let result =
        match Domain.relation ~ctx elt1 elt2 with
        | Disjoint -> Sat
        | Equal
        | Subset
        | Superset
        | Unknown ->
          Unsat (elt1, elt2)
      in
      result
    | (Set a, Compl (Set b)) ->
      (* (A disj !B) if A ⊆ B *)
      let result =
        match Domain.relation ~ctx a b with
        | Equal
        | Subset ->
          Sat
        | Superset
        | Unknown
        | Disjoint ->
          Unsat (a, b)
      in
      result
    | (Compl (Set set1), Compl (Set set2)) ->
      (* Approximation:

         (!A disj !B) iff (A ∪ B) = U && A = !B
           where U := Universal Set (Set Containing All Elements in the Domain)

         There is no way in our model to determine if (A ∪ B) = U holds
         so we are forced to approximate the result. The safest approximation
         is to assume the sets are not disjoint *)
      Unsat (set1, set2)
    | (Compl (Set _), (Union _ | Inter _ | Compl _ | Set _))
    | (Set _, (Union _ | Inter _ | Compl _)) ->
      disjoint ctx set2 set1

  let union (l : t) (r : t) : t = Union (l, r)

  let inter (l : t) (r : t) : t = Inter (l, r)

  (* A ∖ B = A ∩ !B *)
  let diff (a : t) (b : t) : t = inter a (Compl b)
end
