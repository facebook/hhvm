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

module Make (AtomicSet : sig
  type t

  val compare : t -> t -> int
end) : S with type set := AtomicSet.t = struct
  type set = AtomicSet.t

  type dnf = set list list

  module SetSet = Stdlib.Set.Make (AtomicSet)
  module ConjunctionSet = Stdlib.Set.Make (SetSet)

  (** Represents union and intersection across [set] in
     disjunctive normal form (DNF) *)
  module Lattice = struct
    (** Notable we lack logical negations in our representation *)
    type t =
      | And of SetSet.t
      | Or of ConjunctionSet.t

    (* The empty disjunction is the bottom type, i.e. "false" *)
    let bottom = Or ConjunctionSet.empty

    (* The empty conjunction is the top type, i.e. "true" *)
    let top = And SetSet.empty

    let is_bottom = function
      | Or ands when ConjunctionSet.is_empty ands -> true
      | _ -> false

    let singleton ty = And (SetSet.singleton ty)

    (** Unions two abstract sets *)
    let join (set1 : t) (set2 : t) : t =
      match (set1, set2) with
      (* Top | A = Top *)
      | (And ss1, And ss2) when SetSet.is_empty ss1 || SetSet.is_empty ss2 ->
        top
      | (And ss1, And ss2) -> Or (ConjunctionSet.of_list [ss1; ss2])
      (* (A1 | A2) | B = (A1 | A2 | B) *)
      | (And conjunction, Or ands)
      | (Or ands, And conjunction) ->
        Or (ConjunctionSet.add conjunction ands)
      (* (A1 | A2) | (B1 | B2) = (A1 | A2 | B1 | B2) *)
      | (Or ands1, Or ands2) -> Or (ConjunctionSet.union ands1 ands2)

    (** Intersects two abstract sets *)
    let meet (set1 : t) (set2 : t) =
      let meet (set1 : t) (set2 : SetSet.t) =
        match set1 with
        | And set1 -> And (SetSet.union set1 set2)
        | Or ands ->
          Or (ConjunctionSet.map (fun set -> SetSet.union set set2) ands)
      in
      match set1 with
      | And set -> meet set2 set
      (* (A1 | A2) & B = (A1 & B) | (A2 & B) *)
      | Or ands ->
        (* dedupe should occur in join *)
        ConjunctionSet.fold
          (fun conj dnf -> join dnf @@ meet set2 conj)
          ands
          bottom

    module Infix_ops = struct
      let ( ||| ) = join

      let ( &&& ) = meet
    end

    let to_list : t -> dnf = function
      | And list -> [SetSet.elements list]
      | Or ands ->
        List.map (fun list -> SetSet.elements list)
        @@ ConjunctionSet.elements ands

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
      let product_of_setsets (setset_l : SetSet.t list) =
        let product_elts =
          setset_l
          |> List.map SetSet.elements
          |> cartesian
          |> List.map product_f
        in
        let sets = List.map singleton product_elts in
        List.fold_left meet top sets
      in
      let product_of_csets (cset_l : ConjunctionSet.t list) =
        let product_elts =
          cartesian @@ List.map ConjunctionSet.elements cset_l
        in
        List.fold_left join bottom @@ List.map product_of_setsets product_elts
      in
      let to_cset t =
        match t with
        | And setset -> ConjunctionSet.singleton setset
        | Or cset -> cset
      in
      product_of_csets @@ List.map to_cset tl
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

  (* If the partition is fully within the left, span or right we can
     simplify the DNF to be [set], otherwise return [t] unchanged *)
  let simplify t set =
    match
      ( Lattice.is_bottom t.left,
        Lattice.is_bottom t.span,
        Lattice.is_bottom t.right )
    with
    | (false, true, true) -> mk_left set
    | (true, false, true) -> mk_span set
    | (true, true, false) -> mk_right set
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
    span = (As x (Bl | Bs) x (Cl | Cs)) |
           ((Al | As) x Bs x (Cl | Cs)) |
           ((Al | As) x (Bl | Bs) x Cs)
    right = (Ar x B x C) | (A x Br x C) | (A x B x Cr)
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
