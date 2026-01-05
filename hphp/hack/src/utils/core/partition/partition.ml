(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

include Partition_intf

let cartesian xss =
  let rec aux xss ~k =
    match xss with
    (* Product of the empty list is a single empty list   *)
    | [] -> k [[]]
    | xs :: xss ->
      (* Get the cartesian product of the tail lists *)
      aux xss ~k:(fun xss ->
          (* Then, for each element in the head list,
             we cons to each list in the product of the tail
             e.g. if we had [1;2] :: [3] :: []  then we find the cartesian product
             of [3] :: [] === [[3]] then map over the head list ([1;2]) prepending it
             to that product, i.e. [[1;3];[2;3]]
          *)
          k @@ auxs xs xss [])
  and auxs xs xss acc =
    match xs with
    | [] -> List.rev acc
    | x :: xs ->
      auxs xs xss @@ List.fold_left (fun acc xs -> (x :: xs) :: acc) acc xss
  in
  aux xss ~k:(fun x -> x)

module Make (Atom : sig
  type t

  val compare : t -> t -> int
end) : S with type atom := Atom.t = struct
  type atom = Atom.t

  type dnf = atom list list

  module Conjunction = Stdlib.Set.Make (Atom)
  module Disjunction = Stdlib.Set.Make (Conjunction)

  (** Represents union and intersection across [atom] in
     disjunctive normal form (DNF) *)
  module Lattice = struct
    (** Notable we lack logical negations in our representation *)
    type t =
      | And of Conjunction.t
      | Or of Disjunction.t

    (* The empty disjunction is the bottom type, i.e. "false" *)
    let bottom = Or Disjunction.empty

    (* The empty conjunction is the top type, i.e. "true" *)
    let top = And Conjunction.empty

    let is_bottom = function
      | Or disjunction when Disjunction.is_empty disjunction -> true
      | _ -> false

    let singleton atom = And (Conjunction.singleton atom)

    (** Unions two abstract sets *)
    let join (set1 : t) (set2 : t) : t =
      match (set1, set2) with
      (* Top | A = Top *)
      | (And conj1, And conj2)
        when Conjunction.is_empty conj1 || Conjunction.is_empty conj2 ->
        top
      | (And conj1, And conj2) -> Or (Disjunction.of_list [conj1; conj2])
      (* (A1 | A2) | B = (A1 | A2 | B) *)
      | (And conjunction, Or disjunction)
      | (Or disjunction, And conjunction) ->
        Or (Disjunction.add conjunction disjunction)
      (* (A1 | A2) | (B1 | B2) = (A1 | A2 | B1 | B2) *)
      | (Or disj1, Or disj2) -> Or (Disjunction.union disj1 disj2)

    (** Intersects two abstract sets *)
    let meet (set1 : t) (set2 : t) =
      let meet (set1 : t) (conj2 : Conjunction.t) =
        match set1 with
        | And conj1 -> And (Conjunction.union conj1 conj2)
        | Or disj ->
          Or (Disjunction.map (fun conj -> Conjunction.union conj conj2) disj)
      in
      match set1 with
      | And conj -> meet set2 conj
      (* (A1 | A2) & B = (A1 & B) | (A2 & B) *)
      | Or disj ->
        (* dedupe should occur in join *)
        Disjunction.fold
          (fun conj set -> join set @@ meet set2 conj)
          disj
          bottom

    module Infix_ops = struct
      let ( ||| ) = join

      let ( &&& ) = meet
    end

    let to_list : t -> dnf = function
      | And conjunction -> [Conjunction.elements conjunction]
      | Or disjunction ->
        List.map (fun conj -> Conjunction.elements conj)
        @@ Disjunction.elements disjunction

    (** This function computes a product of N-lattices by pushing the product
        operation through the DNF such that the resulting lattice
        is a DNF of N-ary product of atoms
        (and each product of atoms is converted into an atom via product_f).

        This pushing and conversion is done by the N-ary extrapolation of the
        following:

        A = a_0 | a_1 | ... | a_n
          a_i = a_i_0 & a_i_1 & ... & a_i_xi
        B = b_0 | b_1 | ... | b_m
          b_i = b_i_0 & b_i_1 & ... & b_i_yi

        A x B = or(for all i=0..n, j=0..m : a_i x b_j)
          a_i x b_j = and(for all k=0..xi, l=0..yi : product_f [a_i_k; b_j_l])
    *)
    let product product_f (tl : t list) =
      let product_of_conjunctions (conjunctions : Conjunction.t list) =
        let product_elts =
          conjunctions
          |> List.map Conjunction.elements
          |> cartesian
          |> List.map product_f
        in
        let sets = List.map singleton product_elts in
        List.fold_left meet top sets
      in
      let product_of_disjunctions (disjunctions : Disjunction.t list) =
        let product_elts =
          cartesian @@ List.map Disjunction.elements disjunctions
        in
        List.fold_left join bottom
        @@ List.map product_of_conjunctions product_elts
      in
      let to_disjunction t =
        match t with
        | And conjunction -> Disjunction.singleton conjunction
        | Or disjunction -> disjunction
      in
      product_of_disjunctions @@ List.map to_disjunction tl
  end

  (* [left] is disjoint from [right], while [span] overlaps partitially with
     [left] and [right] *)
  type t = {
    left: Lattice.t;
    span: Lattice.t;
    right: Lattice.t;
  }

  let mk_bottom =
    { left = Lattice.bottom; span = Lattice.bottom; right = Lattice.bottom }

  let mk_left atom =
    {
      left = Lattice.singleton atom;
      span = Lattice.bottom;
      right = Lattice.bottom;
    }

  let mk_right atom =
    {
      left = Lattice.bottom;
      span = Lattice.bottom;
      right = Lattice.singleton atom;
    }

  let mk_span atom =
    {
      left = Lattice.bottom;
      span = Lattice.singleton atom;
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

  (*
    Combines two partitions using the algebra for union predicates.

    When combining partitions P1 and P2 for union predicates (P1 | P2):
    - left: passes at least one predicate = L1 | L2
    - right: fails both predicates = R1 & R2
    - span: uncertain (not definitely left or right) = (S1 & S2) | (S1 & R2) | (R1 & S2)

    This is different from regular join which does component-wise union.
  *)
  let union_combine
      { left = left1; span = span1; right = right1 }
      { left = left2; span = span2; right = right2 } =
    {
      left = Lattice.Infix_ops.(left1 ||| left2);
      span =
        Lattice.Infix_ops.(
          span1 &&& span2 ||| (span1 &&& right2) ||| (right1 &&& span2));
      right = Lattice.Infix_ops.(right1 &&& right2);
    }

  (* If the partition is fully within the left, span or right we can
     simplify the DNF to be [set], otherwise return [t] unchanged *)
  let simplify t atom =
    match
      ( Lattice.is_bottom t.left,
        Lattice.is_bottom t.span,
        Lattice.is_bottom t.right )
    with
    | (false, true, true) -> mk_left atom
    | (true, false, true) -> mk_span atom
    | (true, true, false) -> mk_right atom
    | _ -> t

  let left t = Lattice.to_list t.left

  let span t = Lattice.to_list t.span

  let right t = Lattice.to_list t.right

  (*
    A := Al | As | Ar
    B := Bl | Bs | Br
    c := Cl | Cs | Cr

    When computing A x B x C we have:

    left = Al x Bl x Cl
    span = (As x (B - Br) x (C - Cr)) |
           ((A - Ar) x Bs x (C - Cr)) |
           ((A - Ar) x (B - Br) x Cs)
         = (As x (Bl | Bs) x (Cl | Cs)) |
           ((Al | As) x Bs x (Cl | Cs)) |
           ((Al | As) x (Bl | Bs) x Cs)
    right = (Ar x B x C) | (A x Br x C) | (A x B x Cr)

    This is:
    - "left if ALL parts are left"
    - "right if ANY parts are right"
    - "span otherwise" which is
      "span if no parts are right and at least one part is not left"
  *)
  let product product_f tl =
    let left_lattices = List.map (fun t -> t.left) tl in
    let span_lattices = List.map (fun t -> t.span) tl in
    let left_or_span_lattices =
      List.map (fun t -> Lattice.join t.left t.span) tl
    in
    let right_lattices = List.map (fun t -> t.right) tl in
    let full_lattices =
      List.map (fun t -> Lattice.Infix_ops.(t.left ||| t.span ||| t.right)) tl
    in
    let left = Lattice.product product_f left_lattices in
    (* Given
       befores @ current_and_afters = [(a0, b0), (a1, b1) .. (an, bn)]
       Return
       [product(a0, b1 .. bn), product(b0, a1, b2 .. bn) .. product(b0, b1 .. bn-1, an)]
    *)
    let rec helper befores current_and_afters =
      match current_and_afters with
      | [] -> []
      | ((curr, _other) as pair) :: afters ->
        let parts =
          List.map (fun (_curr, other) -> other) befores
          @ [curr]
          @ List.map (fun (_curr, other) -> other) afters
        in
        if List.exists Lattice.is_bottom parts then
          helper (List.rev (pair :: befores)) afters
        else
          Lattice.product product_f parts
          :: helper (List.rev (pair :: befores)) afters
    in
    let span =
      List.fold_left Lattice.join Lattice.bottom
      @@ helper []
      @@ List.combine span_lattices left_or_span_lattices
    in
    let right =
      List.fold_left Lattice.join Lattice.bottom
      @@ helper []
      @@ List.combine right_lattices full_lattices
    in
    { left; span; right }

  module Infix_ops = struct
    let ( ||| ) = join

    let ( &&& ) = meet
  end
end
