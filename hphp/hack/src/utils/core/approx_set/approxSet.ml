(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

include ApproxSet_intf

module Make (Domain : DomainType) : S with module Domain := Domain = struct
  type disjoint =
    | Sat
    | Unsat of {
        left: Domain.t;
        relation: SetRelation.t;
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
        SetRelation.union (relate ctx left set) (relate ctx right set)
      | (Inter (left, right), set) ->
        SetRelation.inter (relate ctx left set) (relate ctx right set)
      | (Set { comp = true; elt }, set) ->
        SetRelation.complement (relate ctx (singleton elt) set)
      | ( Set { comp = false; elt },
          ((Union _ | Inter _ | Set { comp = true; elt = _ }) as set) ) ->
        SetRelation.flip (relate ctx set (singleton elt))

    let flip_unsat = function
      | Sat -> Sat
      | Unsat { left; relation; right } ->
        Unsat
          { left = right; relation = SetRelation.flip relation; right = left }

    let disjoint_atom atom1 atom2 ~ctx =
      let relation = relate ctx (Set atom1) (Set atom2) in
      if SetRelation.is_disjoint relation then
        Sat
      else
        Unsat { left = atom1.elt; relation; right = atom2.elt }

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
    | (None, None) -> SetRelation.all
    | (None, Some _) ->
      SetRelation.make ~subset:true ~superset:false ~disjoint:true
    | (Some _, None) ->
      SetRelation.make ~subset:false ~superset:true ~disjoint:true
    | (Some set1, Some set2) -> relate ctx set1 set2
end
