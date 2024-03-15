(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

include Partition_intf

module Make (AtomicSet : sig
  type t
end) : S with type set := AtomicSet.t = struct
  type set = AtomicSet.t

  type dnf = set list list

  (** Represents union and intersection across [set] in
     disjunctive normal form (DNF) *)
  module Lattice = struct
    type inter = |

    type union = |

    (** We utilize a GADT to ensure construction is done in
       disjunctive normal form. This forbids disjunctions ([Or]),
       from being embedded inside of a conjunction ([And]).

       Notable we lack logical negations in our representation *)
    type _ impl =
      | And : set list -> inter impl
      | Or : inter impl list -> union impl

    type t = DNF : _ impl -> t

    (* The empty disjunction is the bottom type, i.e. "false" *)
    let bottom = DNF (Or [])

    (* The empty conjunction is the top type, i.e. "true" *)
    let top = DNF (And [])

    let singleton ty = DNF (And [ty])

    (** Unions two abstract sets *)
    let join (set1 : t) (set2 : t) : t =
      match (set1, set2) with
      (* Top | A = Top *)
      | (DNF (And []), DNF (And _))
      | (DNF (And _), DNF (And [])) ->
        top
      | (DNF (And set1), DNF (And set2)) -> DNF (Or [And set1; And set2])
      (* (A1 | A2) | B = (A1 | A2 | B) *)
      | (DNF (And conjunction), DNF (Or ands))
      | (DNF (Or ands), DNF (And conjunction)) ->
        DNF (Or (And conjunction :: ands))
      (* (A1 | A2) | (B1 | B2) = (A1 | A2 | B1 | B2) *)
      | (DNF (Or ands1), DNF (Or ands2)) -> DNF (Or (ands1 @ ands2))

    (** Intersects two abstract sets *)
    let meet (set1 : t) (set2 : t) =
      let meet (set1 : t) (set2 : set list) =
        match set1 with
        | DNF (And set1) -> DNF (And (set1 @ set2))
        | DNF (Or ands) ->
          DNF (Or (List.map (fun (And set) -> And (set @ set2)) ands))
      in
      match set1 with
      | DNF (And set) -> meet set2 set
      (* (A1 | A2) & B = (A1 & B) | (A2 & B) *)
      | DNF (Or ands) ->
        let conjunctions = List.map (fun (And set) -> meet set2 set) ands in
        List.fold_left join bottom conjunctions

    module Infix_ops = struct
      let ( ||| ) = join

      let ( &&& ) = meet
    end

    let to_list : t -> dnf = function
      | DNF (And list) -> [list]
      | DNF (Or ands) -> List.map (fun (And list) -> list) ands
  end

  (* [left] is disjoint from [right], while [span] contains sets
     that overlaps partitially with [left] and [right] *)
  type t = {
    left: Lattice.t;
    span: Lattice.t;
    right: Lattice.t;
  }

  let mk_bottom =
    { left = Lattice.bottom; span = Lattice.bottom; right = Lattice.bottom }

  let mk_left ty =
    {
      left = Lattice.singleton ty;
      span = Lattice.bottom;
      right = Lattice.bottom;
    }

  let mk_right ty =
    {
      left = Lattice.bottom;
      span = Lattice.bottom;
      right = Lattice.singleton ty;
    }

  let mk_span ty =
    {
      left = Lattice.bottom;
      span = Lattice.singleton ty;
      right = Lattice.bottom;
    }

  let join
      { left = left1; span = span1; right = right1 }
      { left = left2; span = span2; right = right2 } =
    {
      left = Lattice.Infix_ops.(left1 ||| left2);
      span = Lattice.Infix_ops.(span1 ||| span2);
      right = Lattice.Infix_ops.(right1 ||| right2);
    }

  (*
    A := Al | As | Ar
    B := Bl | Bs | Br

    When computing A & B we have:
    A & B => // Expand B

    A & (Bl | Bs | Br) => // Distribute across union

    (A & Bl) | (A & Bs) | (A & Br) => Expand A

    ((Al | As | Ar) & Bl) |
      ((Al | As | Ar) & Bs) |
      ((Al | As | Ar) & Br) => // Distribute across union

    (Al & Bl) | (As & Bl) | (Ar & Bl) |
      (Al & Bs) | (As & Bs) | (Ar & Bs) |
      (Al & Br) | (As & Br) | (Ar & Br)
        => // Al is disjoint from Br, Bl is disjoint from Ar

    (Al & Bl) | (As & Bl) | {} |
      (Al & Bs) | (As & Bs) | (Ar & Bs) |
      {} | (As & Br) | (Ar & Br) => // Simplify {}

    (Al & Bl) | (As & Bl) | (Al & Bs) | (As & Bs) | (Ar & Bs) | (As & Br) |
      (Ar & Br) => // Group Al, Bl together and Ar, Br together

    left = (Al & Bl) | (Bl & As) | (Al & Bs),
    span = As & Bs,
    right = (Ar & Bs) | (Br & As) | (Ar & Br)
  *)
  let meet
      { left = left1; span = span1; right = right1 }
      { left = left2; span = span2; right = right2 } =
    {
      left =
        Lattice.Infix_ops.(
          left1 &&& left2 ||| (left1 &&& span2) ||| (left2 &&& span1));
      span = Lattice.Infix_ops.(span1 &&& span2);
      right =
        Lattice.Infix_ops.(
          right1 &&& right2 ||| (right1 &&& span2) ||| (right2 &&& span1));
    }

  let left t = Lattice.to_list t.left

  let span t = Lattice.to_list t.span

  let right t = Lattice.to_list t.right

  module Infix_ops = struct
    let ( ||| ) = join

    let ( &&& ) = meet
  end
end
