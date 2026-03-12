(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

(** Test 2: Variant Filter — simulates typing_case_type_variant.ml's
    filter_variants_using_datatype. *)

open Hh_prelude
open OUnit2
open Approxset_tag_domain

module Test (Impl : S_for_test) = struct
  module A = ASet (Impl)
  module TD = To_datatypes (Impl)

  (* ---- test_variant_filter_basic ---- *)
  let test_variant_filter_basic _ =
    let f (h, variants, intersecting_tags) =
      let denote = make_denotation h in
      let dt_inter =
        List.fold intersecting_tags ~init:A.empty ~f:(fun acc t ->
            A.union acc (A.singleton_with_denote denote t))
      in
      List.iter variants ~f:(fun variant_tags ->
          let dt_var =
            List.fold variant_tags ~init:A.empty ~f:(fun acc t ->
                A.union acc (A.singleton_with_denote denote t))
          in
          let filtered = A.are_disjoint h dt_var dt_inter in
          if filtered then
            assert_bool
              "Filtered variant must truly be disjoint (ground truth)"
              (Set.are_disjoint dt_var.A.expected dt_inter.A.expected))
    in
    let gen =
      let open Quickcheck.Let_syntax in
      let%bind h = gen_hierarchy_from_pool in
      let%bind n_variants = Int.gen_incl 2 7 in
      let%bind variants =
        List.gen_with_length n_variants
        @@ let%bind n_tags = Int.gen_incl 1 3 in
           List.gen_with_length n_tags (gen_tag h)
      in
      let%bind n_inter = Int.gen_incl 1 3 in
      let%map intersecting_tags = List.gen_with_length n_inter (gen_tag h) in
      (h, variants, intersecting_tags)
    in
    Quickcheck.test ~trials:10000 ~sexp_of:[%sexp_of: _] ~f gen

  (* ---- test_variant_filter_with_intersection ---- *)
  let test_variant_filter_with_intersection _ =
    let f (h, variants, inter_tags1, inter_tags2) =
      let denote = make_denotation h in
      let dt_set1 =
        List.fold inter_tags1 ~init:A.empty ~f:(fun acc t ->
            A.union acc (A.singleton_with_denote denote t))
      in
      let dt_set2 =
        List.fold inter_tags2 ~init:A.empty ~f:(fun acc t ->
            A.union acc (A.singleton_with_denote denote t))
      in
      let dt_inter = A.inter dt_set1 dt_set2 in
      List.iter variants ~f:(fun variant_tags ->
          let dt_var =
            List.fold variant_tags ~init:A.empty ~f:(fun acc t ->
                A.union acc (A.singleton_with_denote denote t))
          in
          let filtered = A.are_disjoint h dt_var dt_inter in
          if filtered then
            assert_bool
              "Filtered variant must truly be disjoint (ground truth)"
              (Set.are_disjoint dt_var.A.expected dt_inter.A.expected))
    in
    let gen =
      let open Quickcheck.Let_syntax in
      let%bind h = gen_hierarchy_from_pool in
      let%bind n_variants = Int.gen_incl 2 5 in
      let%bind variants =
        List.gen_with_length n_variants
        @@ let%bind n_tags = Int.gen_incl 1 3 in
           List.gen_with_length n_tags (gen_tag h)
      in
      let%bind n1 = Int.gen_incl 1 3 in
      let%bind inter_tags1 = List.gen_with_length n1 (gen_tag h) in
      let%bind n2 = Int.gen_incl 1 3 in
      let%map inter_tags2 = List.gen_with_length n2 (gen_tag h) in
      (h, variants, inter_tags1, inter_tags2)
    in
    Quickcheck.test ~trials:5000 ~sexp_of:[%sexp_of: _] ~f gen

  (* ---- test_variant_filter_with_negation ---- *)
  let test_variant_filter_with_negation _ =
    let f (h, variants, neg_tags) =
      let denote = make_denotation h in
      let mixed_set = A.mixed_with_denote denote in
      let dt_neg_inner =
        List.fold neg_tags ~init:A.empty ~f:(fun acc t ->
            A.union acc (A.singleton_with_denote denote t))
      in
      let dt_inter = A.diff mixed_set dt_neg_inner in
      List.iter variants ~f:(fun variant_tags ->
          let dt_var =
            List.fold variant_tags ~init:A.empty ~f:(fun acc t ->
                A.union acc (A.singleton_with_denote denote t))
          in
          let filtered = A.are_disjoint h dt_var dt_inter in
          if filtered then
            assert_bool
              "Filtered variant must truly be disjoint (ground truth)"
              (Set.are_disjoint dt_var.A.expected dt_inter.A.expected))
    in
    let gen =
      let open Quickcheck.Let_syntax in
      let%bind h = gen_hierarchy_from_pool in
      let%bind n_variants = Int.gen_incl 2 5 in
      let%bind variants =
        List.gen_with_length n_variants
        @@ let%bind n_tags = Int.gen_incl 1 3 in
           List.gen_with_length n_tags (gen_tag h)
      in
      let%bind n_neg = Int.gen_incl 1 3 in
      let%map neg_tags = List.gen_with_length n_neg (gen_tag h) in
      (h, variants, neg_tags)
    in
    Quickcheck.test ~trials:5000 ~sexp_of:[%sexp_of: _] ~f gen

  (* ---- test_variant_filter_sealed ---- *)
  let test_variant_filter_sealed _ =
    let f (h, sealed_id, intersecting_cid) =
      let denote = make_denotation h in
      let whitelist = Option.value ~default:[] (h.sealed_whitelist sealed_id) in
      let variants =
        List.map whitelist ~f:(fun wid -> TD.to_datatypes h denote wid)
      in
      let dt_inter = TD.to_datatypes h denote intersecting_cid in
      List.iter variants ~f:(fun dt_var ->
          let filtered = A.are_disjoint h dt_var dt_inter in
          if filtered then
            assert_bool
              "Filtered sealed variant must truly be disjoint (ground truth)"
              (Set.are_disjoint dt_var.A.expected dt_inter.A.expected))
    in
    let gen =
      let open Quickcheck.Let_syntax in
      let%bind h = gen_hierarchy_with_sealed in
      let sealed_id =
        match find_sealed_class h with
        | Some id -> id
        | None -> Option.value_exn (find_sealed_interface h)
      in
      let%map intersecting_cid = Int.gen_incl 0 (h.num_classes - 1) in
      (h, sealed_id, intersecting_cid)
    in
    Quickcheck.test ~trials:5000 ~sexp_of:[%sexp_of: _] ~f gen

  (* ---- test_variant_filter_require_extends ---- *)
  let test_variant_filter_require_extends _ =
    let f (h, iface_id, variant_tags) =
      let denote = make_denotation h in
      let dt_inter = TD.to_datatypes h denote iface_id in
      List.iter variant_tags ~f:(fun vtags ->
          let dt_var =
            List.fold vtags ~init:A.empty ~f:(fun acc t ->
                A.union acc (A.singleton_with_denote denote t))
          in
          let filtered = A.are_disjoint h dt_var dt_inter in
          if filtered then
            assert_bool
              "Filtered variant must truly be disjoint (ground truth)"
              (Set.are_disjoint dt_var.A.expected dt_inter.A.expected))
    in
    let gen =
      let open Quickcheck.Let_syntax in
      let%bind h = gen_hierarchy_with_require in
      let iface_id = Option.value_exn (find_interface_with_require h) in
      let%bind n_variants = Int.gen_incl 2 5 in
      let%map variant_tags =
        List.gen_with_length n_variants
        @@ let%bind n_tags = Int.gen_incl 1 3 in
           List.gen_with_length n_tags (gen_tag h)
      in
      (h, iface_id, variant_tags)
    in
    Quickcheck.test ~trials:5000 ~sexp_of:[%sexp_of: _] ~f gen

  let tests =
    [
      "test_variant_filter_basic" >:: test_variant_filter_basic;
      "test_variant_filter_with_intersection"
      >:: test_variant_filter_with_intersection;
      "test_variant_filter_with_negation" >:: test_variant_filter_with_negation;
      "test_variant_filter_sealed" >:: test_variant_filter_sealed;
      "test_variant_filter_require_extends"
      >:: test_variant_filter_require_extends;
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
  "approxset_variant_filter"
  >::: [
         "approxset_impl" >::: ApproxSetTests.tests;
         "bddset_impl" >::: BddSetTests.tests;
       ]
  |> run_test_tt_main
