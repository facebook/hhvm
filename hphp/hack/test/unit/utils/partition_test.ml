(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Hh_prelude
open OUnit2

let string_set =
  String.Set.quickcheck_generator
    Quickcheck.Generator.(
      union [return "A"; return "B"; return "C"; return "D"])

let non_empty_string_set =
  Quickcheck.Generator.filter string_set ~f:(fun s -> not @@ Set.is_empty s)

let string_set_product (string_sets : String.Set.t list) : String.Set.t =
  String.Set.of_list
  @@ List.map
       (Partition.cartesian @@ List.map ~f:Set.elements string_sets)
       ~f:(fun parts -> "(" ^ String.concat ~sep:"," parts ^ ")")

type predicate_type =
  | PSet of String.Set.t
  | PProduct of predicate_type list
[@@deriving sexp]

let rec predicate_type_to_set predicate =
  match predicate with
  | PSet set -> set
  | PProduct predicates ->
    let parts = List.map ~f:predicate_type_to_set predicates in
    string_set_product parts

module SetOps = struct
  type t =
    | Empty
    | Union of (t * t * String.Set.t)
    | Intersection of (t * t * String.Set.t)
    | Singleton of String.Set.t
    | Product of (t list * String.Set.t)
  [@@deriving sexp]

  let to_set = function
    | Empty -> String.Set.empty
    | Singleton set
    | Union (_, _, set)
    | Intersection (_, _, set)
    | Product (_, set) ->
      set

  let mk_union (t1 : t) (t2 : t) =
    Union (t1, t2, Set.union (to_set t1) (to_set t2))

  let mk_intersection (t1 : t) (t2 : t) =
    Intersection (t1, t2, Set.inter (to_set t1) (to_set t2))

  let mk_product (ts : t list) =
    Product (ts, string_set_product @@ List.map ~f:to_set ts)

  let partition ~predicate =
    Set.partition_tf ~f:Set.(mem @@ predicate_type_to_set predicate)

  let non_empty gen =
    Quickcheck.Generator.filter gen ~f:(fun set ->
        not @@ Set.is_empty (to_set set))

  let gen : t Quickcheck.Generator.t =
    let open Quickcheck.Generator in
    non_empty
    @@ recursive_union
         [
           return Empty;
           map string_set ~f:(fun set -> Singleton set);
           return @@ mk_product [];
         ]
         ~f:(fun self ->
           let union =
             self >>= fun left ->
             self >>| fun right -> mk_union left right
           in
           let inter =
             self >>= fun left ->
             self >>| fun right -> mk_intersection left right
           in
           let product1 = self >>| fun a -> mk_product [a] in
           let product2 =
             self >>= fun a ->
             self >>| fun b -> mk_product [a; b]
           in
           let product3 =
             self >>= fun a ->
             self >>= fun b ->
             self >>| fun c -> mk_product [a; b; c]
           in
           [union; inter; product1; product2; product3])
end

let predicate_set : predicate_type Quickcheck.Generator.t =
  let open Quickcheck.Generator in
  recursive_union
    [return @@ PProduct []; map string_set ~f:(fun set -> PSet set)]
    ~f:(fun self ->
      let product1 = self >>| fun a -> PProduct [a] in
      let product2 =
        self >>= fun a ->
        self >>| fun b -> PProduct [a; b]
      in
      let product3 =
        self >>= fun a ->
        self >>= fun b ->
        self >>| fun c -> PProduct [a; b; c]
      in
      [product1; product2; product3])

module StringPartition = struct
  include Partition.Make (struct
    type t = String.Set.t

    let compare = String.Set.compare
  end)

  let rec of_setops ~predicate ops =
    let open SetOps in
    match ops with
    | Empty -> mk_bottom
    | Singleton set ->
      if Set.is_empty set then
        mk_bottom
      else begin
        match predicate with
        | PSet predicate ->
          if Set.is_subset set ~of_:predicate then
            mk_left set
          else if Set.are_disjoint set predicate then
            mk_right set
          else
            mk_span set
        | PProduct _ -> mk_right set
      end
    | Union (set1, set2, _) ->
      Infix_ops.(of_setops ~predicate set1 ||| of_setops ~predicate set2)
    | Intersection (set1, set2, _) ->
      Infix_ops.(of_setops ~predicate set1 &&& of_setops ~predicate set2)
    | Product (ops, actual_set) ->
      let right () = mk_right actual_set in
      begin
        match predicate with
        | PSet _ -> right ()
        | PProduct predicates ->
          (match List.zip predicates ops with
          | List.Or_unequal_lengths.Unequal_lengths -> right ()
          | List.Or_unequal_lengths.Ok pairs ->
            product string_set_product
            @@ List.map
                 ~f:(fun (predicate, ops) -> of_setops ~predicate ops)
                 pairs)
      end

  let to_tuple t =
    let dnf_to_set dnf =
      List.fold
        ~init:String.Set.empty
        ~f:(fun acc ands ->
          List.reduce_exn ~f:Set.inter ands |> Set.(union acc))
        dnf
    in
    (dnf_to_set @@ left t, dnf_to_set @@ span t, dnf_to_set @@ right t)
end

let test_quick _ =
  let f (predicate, ops) =
    let (expected_left, expected_right) =
      SetOps.(partition ~predicate @@ to_set ops)
    in
    let string_partition = StringPartition.of_setops ~predicate ops in
    let (actual_left, actual_span, actual_right) =
      StringPartition.to_tuple string_partition
    in
    assert_bool "Actual Left is Subset of Expected Left"
    @@ Set.is_subset actual_left ~of_:expected_left;
    assert_bool "Actual Right is Subset of Expected Right"
    @@ Set.is_subset actual_right ~of_:expected_right;
    let (span_left, span_right) = SetOps.partition ~predicate actual_span in
    assert_equal
      ~cmp:String.Set.equal
      expected_left
      (Set.union span_left actual_left);
    assert_equal
      ~cmp:String.Set.equal
      expected_right
      (Set.union span_right actual_right)
  in

  Quickcheck.test
    ~trials:1000
    ~sexp_of:[%sexp_of: predicate_type * SetOps.t]
    ~f
    Quickcheck.Generator.(tuple2 predicate_set SetOps.gen)

let assert_equal_string_set_ll =
  assert_equal ~cmp:(List.equal (List.equal String.Set.equal))

let test_meet_singleton _ =
  let partition_a = StringPartition.mk_span @@ String.Set.of_list ["X"] in
  let partition_b = StringPartition.mk_span @@ String.Set.of_list ["X"] in
  let meeted = StringPartition.meet partition_a partition_b in
  assert_equal_string_set_ll
    ~msg:
      "Meet of two identical singleton spans should be produce a singleton span"
    [[String.Set.singleton "X"]]
    (StringPartition.span meeted);
  assert_equal_string_set_ll
    ~msg:"Meet of two identical singleton spans should not produce a left"
    []
    (StringPartition.left meeted);
  assert_equal_string_set_ll
    ~msg:"Meet of two identical singleton spans should not produce a right"
    []
    (StringPartition.right meeted)

let test_meet_overlapping _ =
  let open StringPartition in
  (* I'm only going to check the right so a left doesn't matter *)
  let s c = mk_span @@ String.Set.of_list [c] in
  let r c = mk_right @@ String.Set.of_list [c] in
  (* span: A & B; right: A & B *)
  let partition_a = join (meet (s "A") (s "B")) (meet (r "A") (r "C")) in
  (* span: B & C; right: B & A *)
  let partition_b = join (meet (s "B") (s "C")) (meet (r "B") (r "A")) in
  (* naively:
       span:  A & B & B & C
       right: A & C & B & A | A & C & B & C | B & A & A & B
  *)
  let meeted = meet partition_a partition_b in
  assert_equal_string_set_ll
    ~msg:
      "Meet of overlapping partitions should be have deduplicated dnfs (checking left)"
    []
    (left meeted);
  assert_equal_string_set_ll
    ~msg:
      "Meet of overlapping partitions should be have deduplicated dnfs (checking span)"
    (List.map [["A"; "B"; "C"]] ~f:(List.map ~f:String.Set.singleton))
    (span meeted);
  assert_equal_string_set_ll
    ~msg:
      "Meet of overlapping partitions should be have deduplicated dnfs (checking right)"
    (List.map
       [["A"; "B"]; ["A"; "B"; "C"]]
       ~f:(List.map ~f:String.Set.singleton))
    (right meeted)

let () =
  "partition"
  >::: [
         "test_quick" >:: test_quick;
         "test_meet_singleton" >:: test_meet_singleton;
         "test_meet_overlapping" >:: test_meet_overlapping;
       ]
  |> run_test_tt_main
