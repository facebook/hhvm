(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

(** Test 1: Refinement — simulates typing_refinement.ml's split_ty_by_tag. *)

open Hh_prelude
open OUnit2
open Approxset_tag_domain

module Test (Impl : S_for_test) = struct
  module A = ASet (Impl)
  module TD = To_datatypes (Impl)

  (* ---- test_single_refinement ---- *)
  let test_single_refinement _ =
    let f (h, components, predicate) =
      let denote = make_denotation h in
      let mixed_set = A.mixed_with_denote denote in
      let dt_pred = A.singleton_with_denote denote predicate in
      let dt_comp_pred = A.diff mixed_set dt_pred in
      List.iter components ~f:(fun comp_tag ->
          let dt_comp = A.singleton_with_denote denote comp_tag in
          let is_right = A.are_disjoint h dt_comp dt_pred in
          let is_left = A.are_disjoint h dt_comp dt_comp_pred in
          let d_comp = denote comp_tag in
          let d_pred = denote predicate in
          if is_right then
            assert_bool
              (Printf.sprintf
                 "RIGHT: denote(component) ∩ denote(predicate) should be empty [impl=%s, comp=%s, pred=%s, d_comp=%s, d_pred=%s]"
                 Impl.name
                 (Sexp.to_string_hum (sexp_of_tag comp_tag))
                 (Sexp.to_string_hum (sexp_of_tag predicate))
                 (Sexp.to_string_hum ([%sexp_of: Int.Set.t] d_comp))
                 (Sexp.to_string_hum ([%sexp_of: Int.Set.t] d_pred)))
              (Set.are_disjoint d_comp d_pred);
          if is_left then
            assert_bool
              (Printf.sprintf
                 "LEFT: denote(component) should be subset of denote(predicate) [impl=%s, comp=%s, pred=%s, d_comp=%s, d_pred=%s]"
                 Impl.name
                 (Sexp.to_string_hum (sexp_of_tag comp_tag))
                 (Sexp.to_string_hum (sexp_of_tag predicate))
                 (Sexp.to_string_hum ([%sexp_of: Int.Set.t] d_comp))
                 (Sexp.to_string_hum ([%sexp_of: Int.Set.t] d_pred)))
              (Set.is_subset d_comp ~of_:d_pred);
          if is_right && is_left then
            assert_failure "Cannot be both RIGHT and LEFT simultaneously")
    in
    let gen =
      let open Quickcheck.Let_syntax in
      let%bind h = gen_hierarchy_from_pool in
      let%bind n = Int.gen_incl 1 5 in
      let%bind components = List.gen_with_length n (gen_tag h) in
      let%map predicate = gen_tag h in
      (h, components, predicate)
    in
    Quickcheck.test ~trials:10000 ~sexp_of:[%sexp_of: _] ~f gen

  (* ---- test_if_else_chain ---- *)
  let test_if_else_chain _ =
    let f (h, case_tags, predicate_tags) =
      let denote = make_denotation h in
      let remaining =
        List.fold case_tags ~init:A.empty ~f:(fun acc t ->
            A.union acc (A.singleton_with_denote denote t))
      in
      let _ =
        List.fold predicate_tags ~init:remaining ~f:(fun rem pred_tag ->
            let pred_dt = A.singleton_with_denote denote pred_tag in
            let disj = A.are_disjoint h rem pred_dt in
            if disj then begin
              (* Ground truth check *)
              assert_bool
                "Disjointness claim should be sound"
                (Set.are_disjoint rem.A.expected (denote pred_tag))
            end;
            let new_rem = A.diff rem pred_dt in
            assert_bool
              "remaining.expected should shrink monotonically"
              (Set.is_subset new_rem.A.expected ~of_:rem.A.expected);
            new_rem)
      in
      ()
    in
    let gen =
      let open Quickcheck.Let_syntax in
      let%bind h = gen_hierarchy_from_pool in
      let%bind n = Int.gen_incl 2 5 in
      let%bind case_tags = List.gen_with_length n (gen_tag h) in
      let%bind k = Int.gen_incl 1 3 in
      let%map predicate_tags = List.gen_with_length k (gen_tag h) in
      (h, case_tags, predicate_tags)
    in
    Quickcheck.test ~trials:5000 ~sexp_of:[%sexp_of: _] ~f gen

  (* ---- test_negation_refinement ---- *)
  let test_negation_refinement _ =
    let f (h, type_tags, pred1, pred2) =
      let denote = make_denotation h in
      let type_dt =
        List.fold type_tags ~init:A.empty ~f:(fun acc t ->
            A.union acc (A.singleton_with_denote denote t))
      in
      let pred1_dt = A.singleton_with_denote denote pred1 in
      let pred2_dt = A.singleton_with_denote denote pred2 in
      let combined_pred = A.union pred1_dt pred2_dt in
      let mixed_set = A.mixed_with_denote denote in
      let negated_pred = A.diff mixed_set combined_pred in
      let disj_neg = A.are_disjoint h type_dt negated_pred in
      let sub_comb = A.is_subset h type_dt combined_pred in
      if disj_neg then
        assert_bool
          "Disjoint from negation => subset of combined (ground truth)"
          (Set.is_subset
             type_dt.A.expected
             ~of_:(Set.union (denote pred1) (denote pred2)));
      if sub_comb then
        assert_bool
          "Subset of combined should be sound"
          (Set.is_subset
             type_dt.A.expected
             ~of_:(Set.union (denote pred1) (denote pred2)))
    in
    let gen =
      let open Quickcheck.Let_syntax in
      let%bind h = gen_hierarchy_from_pool in
      let%bind n = Int.gen_incl 2 4 in
      let%bind type_tags = List.gen_with_length n (gen_tag h) in
      let%bind pred1 = gen_tag h in
      let%map pred2 = gen_tag h in
      (h, type_tags, pred1, pred2)
    in
    Quickcheck.test ~trials:5000 ~sexp_of:[%sexp_of: _] ~f gen

  (* ---- test_refinement_sealed_interface ---- *)
  let test_refinement_sealed_interface _ =
    let f (h, sealed_iface_id, pred_class_id) =
      let denote = make_denotation h in
      let iface_dt = TD.to_datatypes h denote sealed_iface_id in
      let pred_dt = TD.to_datatypes h denote pred_class_id in
      let mixed_set = A.mixed_with_denote denote in
      let pred_comp = A.diff mixed_set pred_dt in
      let is_left = A.are_disjoint h iface_dt pred_comp in
      let is_right = A.are_disjoint h iface_dt pred_dt in
      if is_left then
        assert_bool
          "LEFT: sealed iface should be subset of pred (ground truth)"
          (Set.is_subset iface_dt.A.expected ~of_:pred_dt.A.expected);
      if is_right then
        assert_bool
          "RIGHT: sealed iface should be disjoint from pred (ground truth)"
          (Set.are_disjoint iface_dt.A.expected pred_dt.A.expected)
    in
    let gen =
      let open Quickcheck.Let_syntax in
      let%bind h = gen_hierarchy_with_sealed_interface in
      let sealed_id = Option.value_exn (find_sealed_interface h) in
      let whitelist = Option.value ~default:[] (h.sealed_whitelist sealed_id) in
      let%map pred_class_id =
        if List.is_empty whitelist then
          Int.gen_incl 0 (h.num_classes - 1)
        else
          (* ~50% chance to pick from whitelist, ~50% random *)
          Quickcheck.Generator.union
            [
              Quickcheck.Generator.of_list whitelist;
              Int.gen_incl 0 (h.num_classes - 1);
            ]
      in
      (h, sealed_id, pred_class_id)
    in
    Quickcheck.test ~trials:5000 ~sexp_of:[%sexp_of: _] ~f gen

  (* ---- test_refinement_require_extends ---- *)
  let test_refinement_require_extends _ =
    let f (h, iface_id, pred_class_id) =
      let denote = make_denotation h in
      let iface_dt = TD.to_datatypes h denote iface_id in
      let pred_dt = TD.to_datatypes h denote pred_class_id in
      let is_disj = A.are_disjoint h iface_dt pred_dt in
      if is_disj then
        assert_bool
          "Disjointness should be sound (ground truth)"
          (Set.are_disjoint iface_dt.A.expected pred_dt.A.expected)
    in
    let gen =
      let open Quickcheck.Let_syntax in
      let%bind h = gen_hierarchy_with_require in
      let iface_id = Option.value_exn (find_interface_with_require h) in
      let%map pred_class_id = Int.gen_incl 0 (h.num_classes - 1) in
      (h, iface_id, pred_class_id)
    in
    Quickcheck.test ~trials:5000 ~sexp_of:[%sexp_of: _] ~f gen

  let tests =
    [
      "test_single_refinement" >:: test_single_refinement;
      "test_if_else_chain" >:: test_if_else_chain;
      "test_negation_refinement" >:: test_negation_refinement;
      "test_refinement_sealed_interface" >:: test_refinement_sealed_interface;
      "test_refinement_require_extends" >:: test_refinement_require_extends;
    ]
end

module ApproxSetImpl : S_for_test = struct
  include ApproxSet.Make (TagDomain)

  let name = "ApproxSet"

  let expect_complete = false
end

module BddSetImpl : S_for_test = struct
  include BddSet.Make (OrderedTagDomain)

  let name = "BddSet"

  let expect_complete = false
end

module ApproxSetTests = Test (ApproxSetImpl)
module BddSetTests = Test (BddSetImpl)

let () =
  "approxset_refinement"
  >::: [
         "approxset_impl" >::: ApproxSetTests.tests;
         "bddset_impl" >::: BddSetTests.tests;
       ]
  |> run_test_tt_main
