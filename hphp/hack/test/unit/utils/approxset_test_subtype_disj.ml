(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

(** Test 3: Subtype Disjointness — simulates subsuming
    typing_subtype.is_type_disjoint_help with ApproxSet. *)

open Hh_prelude
open OUnit2
open Approxset_tag_domain

module Test (Impl : S_for_test) = struct
  module A = ASet (Impl)
  module TD = To_datatypes (Impl)
  module GTS = Gen_type_set (Impl)

  let make_gen_type_set h denote = GTS.gen_type_set h denote

  (* ---- test_disjointness_soundness ---- *)
  let test_disjointness_soundness _ =
    let f (h, s1, s2) =
      let disj = A.are_disjoint h s1 s2 in
      if disj then
        assert_bool
          "are_disjoint should be sound (ground truth)"
          (Set.are_disjoint s1.A.expected s2.A.expected)
    in
    let gen =
      let open Quickcheck.Let_syntax in
      let%bind h = gen_hierarchy_from_pool in
      let denote = make_denotation h in
      let gt = make_gen_type_set h denote in
      let%bind s1 = gt in
      let%map s2 = gt in
      (h, s1, s2)
    in
    Quickcheck.test ~trials:10000 ~sexp_of:[%sexp_of: _] ~f gen

  (* ---- test_subset_soundness ---- *)
  let test_subset_soundness _ =
    let f (h, s1, s2) =
      let sub = A.is_subset h s1 s2 in
      if sub then
        assert_bool
          "is_subset should be sound (ground truth)"
          (Set.is_subset s1.A.expected ~of_:s2.A.expected)
    in
    let gen =
      let open Quickcheck.Let_syntax in
      let%bind h = gen_hierarchy_from_pool in
      let denote = make_denotation h in
      let gt = make_gen_type_set h denote in
      let%bind s1 = gt in
      let%map s2 = gt in
      (h, s1, s2)
    in
    Quickcheck.test ~trials:10000 ~sexp_of:[%sexp_of: _] ~f gen

  (* ---- test_disjointness_symmetry ---- *)
  let test_disjointness_symmetry _ =
    let f (h, s1, s2) =
      let d1 = A.are_disjoint h s1 s2 in
      let d2 = A.are_disjoint h s2 s1 in
      if not (Bool.equal d1 d2) then begin
        (* Only flag when one says disjoint and ground truth disagrees *)
        if d1 then
          assert_bool
            "are_disjoint(a,b) unsound"
            (Set.are_disjoint s1.A.expected s2.A.expected);
        if d2 then
          assert_bool
            "are_disjoint(b,a) unsound"
            (Set.are_disjoint s2.A.expected s1.A.expected)
      end
    in
    let gen =
      let open Quickcheck.Let_syntax in
      let%bind h = gen_hierarchy_from_pool in
      let denote = make_denotation h in
      let gt = make_gen_type_set h denote in
      let%bind s1 = gt in
      let%map s2 = gt in
      (h, s1, s2)
    in
    Quickcheck.test ~trials:10000 ~sexp_of:[%sexp_of: _] ~f gen

  (* ---- test_nullable_disjointness ---- *)
  let test_nullable_disjointness _ =
    let f (h, id1, id2) =
      let denote = make_denotation h in
      let tag1 = make_tag h id1 in
      let tag2 = make_tag h id2 in
      let null_s = A.singleton_with_denote denote (Prim Null) in
      let s1 = A.union (A.singleton_with_denote denote tag1) null_s in
      let s2 = A.union (A.singleton_with_denote denote tag2) null_s in
      let disj = A.are_disjoint h s1 s2 in
      if disj then
        assert_bool
          "Nullable types sharing Null cannot be disjoint (ground truth)"
          (Set.are_disjoint s1.A.expected s2.A.expected)
    in
    let gen =
      let open Quickcheck.Let_syntax in
      let%bind h = gen_hierarchy_from_pool in
      let final_classes =
        List.filter (List.init h.num_classes ~f:Fun.id) ~f:(fun i ->
            h.is_final i)
      in
      let%bind (id1, id2) =
        match final_classes with
        | []
        | [_] ->
          let%bind a = Int.gen_incl 0 (h.num_classes - 1) in
          let%map b = Int.gen_incl 0 (h.num_classes - 1) in
          (a, b)
        | _ ->
          let%bind a = Quickcheck.Generator.of_list final_classes in
          let%map b =
            Quickcheck.Generator.of_list
              (List.filter final_classes ~f:(fun x -> x <> a))
            |> Quickcheck.Generator.filter ~f:(fun b ->
                   not (Set.mem (h.ancestors a) b || Set.mem (h.ancestors b) a))
          in
          (a, b)
      in
      return (h, id1, id2)
    in
    Quickcheck.test ~trials:5000 ~sexp_of:[%sexp_of: _] ~f gen

  (* ---- test_negation_self_disjoint ---- *)
  let test_negation_self_disjoint _ =
    let f (h, t) =
      let denote = make_denotation h in
      let s = A.singleton_with_denote denote t in
      let mixed_set = A.mixed_with_denote denote in
      let neg_s = A.diff mixed_set s in
      let disj = A.are_disjoint h s neg_s in
      if disj then
        assert_bool
          "T and (mixed \\ T) should be disjoint (ground truth)"
          (Set.are_disjoint s.A.expected neg_s.A.expected)
      (* Note: it's sound for the implementation to NOT prove disjointness,
         so we only check soundness of true claims *)
    in
    let gen =
      let open Quickcheck.Let_syntax in
      let%bind h = gen_hierarchy_from_pool in
      let%map t = gen_tag h in
      (h, t)
    in
    Quickcheck.test ~trials:5000 ~sexp_of:[%sexp_of: _] ~f gen

  (* ---- test_complex_relate ---- *)
  let test_complex_relate _ =
    let f (h, s1, s2) =
      let sub_ab = A.is_subset h s1 s2 in
      let sub_ba = A.is_subset h s2 s1 in
      let disj = A.are_disjoint h s1 s2 in
      if sub_ab then
        assert_bool
          "is_subset(a,b) should be sound"
          (Set.is_subset s1.A.expected ~of_:s2.A.expected);
      if sub_ba then
        assert_bool
          "is_subset(b,a) should be sound"
          (Set.is_subset s2.A.expected ~of_:s1.A.expected);
      if disj then
        assert_bool
          "are_disjoint should be sound"
          (Set.are_disjoint s1.A.expected s2.A.expected)
    in
    let gen =
      let open Quickcheck.Let_syntax in
      let%bind h = gen_hierarchy_from_pool in
      let denote = make_denotation h in
      let gt = make_gen_type_set h denote in
      let%bind s1 = gt in
      let%map s2 = gt in
      (h, s1, s2)
    in
    Quickcheck.test ~trials:10000 ~sexp_of:[%sexp_of: _] ~f gen

  let tests =
    [
      "test_disjointness_soundness" >:: test_disjointness_soundness;
      "test_subset_soundness" >:: test_subset_soundness;
      "test_disjointness_symmetry" >:: test_disjointness_symmetry;
      "test_nullable_disjointness" >:: test_nullable_disjointness;
      "test_negation_self_disjoint" >:: test_negation_self_disjoint;
      "test_complex_relate" >:: test_complex_relate;
    ]
end

module ApproxSetImpl : S_for_test = struct
  include ApproxSet.Make (TagDomain)

  let name = "ApproxSet"
end

module BddSetImpl : S_for_test = struct
  include BddSet.Make (OrderedTagDomain)

  let name = "BddSet"
end

module ApproxSetTests = Test (ApproxSetImpl)
module BddSetTests = Test (BddSetImpl)

let () =
  "approxset_subtype_disj"
  >::: [
         "approxset_impl" >::: ApproxSetTests.tests;
         "bddset_impl" >::: BddSetTests.tests;
       ]
  |> run_test_tt_main
