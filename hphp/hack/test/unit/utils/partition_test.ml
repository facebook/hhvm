(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Hh_prelude
open OUnit2

let char_set =
  Char.Set.quickcheck_generator
    Quickcheck.Generator.(
      union
        [
          return 'A';
          return 'B';
          return 'C';
          return 'D';
          return 'E';
          return 'F';
          return 'G';
          return 'H';
        ])

let non_empty_char_set =
  Quickcheck.Generator.filter char_set ~f:(fun s -> not @@ Set.is_empty s)

module SetOps = struct
  type t =
    | Empty
    | Union of (t * t * Char.Set.t)
    | Intersection of (t * t * Char.Set.t)
    | Singleton of Char.Set.t
  [@@deriving sexp]

  let to_set = function
    | Empty -> Char.Set.empty
    | Singleton set
    | Union (_, _, set)
    | Intersection (_, _, set) ->
      set

  let mk_union (t1 : t) (t2 : t) =
    Union (t1, t2, Set.union (to_set t1) (to_set t2))

  let mk_intersection (t1 : t) (t2 : t) =
    Intersection (t1, t2, Set.inter (to_set t1) (to_set t2))

  let partition ~predicate = Set.partition_tf ~f:Set.(mem predicate)

  let non_empty gen =
    Quickcheck.Generator.filter gen ~f:(fun set ->
        not @@ Set.is_empty (to_set set))

  let gen : t Quickcheck.Generator.t =
    let open Quickcheck.Generator in
    non_empty
    @@ recursive_union
         [return Empty; map char_set ~f:(fun set -> Singleton set)]
         ~f:(fun self ->
           let union =
             self >>= fun left ->
             self >>| fun right -> mk_union left right
           in
           let inter =
             self >>= fun left ->
             self >>| fun right -> mk_intersection left right
           in
           [union; inter])
end

module CharPartition = struct
  include Partition.Make (struct
    type t = Char.Set.t
  end)

  let rec of_setops ~predicate ops =
    let of_setops = of_setops ~predicate in
    let open SetOps in
    match ops with
    | Empty -> mk_bottom
    | Singleton set ->
      if Set.is_empty set then
        mk_bottom
      else if Set.is_subset set ~of_:predicate then
        mk_left set
      else if Set.are_disjoint set predicate then
        mk_right set
      else
        mk_span set
    | Union (set1, set2, _) -> Infix_ops.(of_setops set1 ||| of_setops set2)
    | Intersection (set1, set2, _) ->
      Infix_ops.(of_setops set1 &&& of_setops set2)

  let to_tuple t =
    let dnf_to_set dnf =
      List.fold
        ~init:Char.Set.empty
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
    let char_partition = CharPartition.of_setops ~predicate ops in
    let (actual_left, actual_span, actual_right) =
      CharPartition.to_tuple char_partition
    in
    assert_bool "Actual Left is Subset of Expected Left"
    @@ Set.is_subset actual_left ~of_:expected_left;
    assert_bool "Actual Right is Subset of Expected Right"
    @@ Set.is_subset actual_right ~of_:expected_right;
    let (span_left, span_right) = SetOps.partition ~predicate actual_span in
    assert_equal
      ~cmp:Char.Set.equal
      expected_left
      (Set.union span_left actual_left);
    assert_equal
      ~cmp:Char.Set.equal
      expected_right
      (Set.union span_right actual_right)
  in

  Quickcheck.test
    ~trials:100000
    ~sexp_of:[%sexp_of: Char.Set.t * SetOps.t]
    ~f
    Quickcheck.Generator.(tuple2 non_empty_char_set SetOps.gen)

let () = "partition" >::: ["test_quick" >:: test_quick] |> run_test_tt_main
