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

(* Reference implementation: obviously sound and complete.
   Stores sets as expression trees. Evaluates by enumerating all
   valuations consistent with pairwise Domain.relation results. *)
module ValSet = struct
  type t =
    | Empty
    | Singleton of tag
    | Union of t * t
    | Inter of t * t
    | Diff of t * t

  let empty = Empty

  let singleton t = Singleton t

  let union a b = Union (a, b)

  let inter a b = Inter (a, b)

  let diff a b = Diff (a, b)

  let rec eval (v : tag -> bool) = function
    | Empty -> false
    | Singleton t -> v t
    | Union (a, b) -> eval v a || eval v b
    | Inter (a, b) -> eval v a && eval v b
    | Diff (a, b) -> eval v a && not (eval v b)

  let rec atoms_acc acc = function
    | Empty -> acc
    | Singleton t -> t :: acc
    | Union (a, b)
    | Inter (a, b)
    | Diff (a, b) ->
      atoms_acc (atoms_acc acc a) b

  let atoms t = List.dedup_and_sort ~compare:compare_tag (atoms_acc [] t)

  let is_consistent (h : hierarchy) (atoms_arr : tag array) (bits : int) =
    let n = Array.length atoms_arr in
    let in_ i = bits land (1 lsl i) <> 0 in
    let ok = ref true in
    for i = 0 to n - 1 do
      for j = i to n - 1 do
        if !ok then begin
          let rel = TagDomain.relation atoms_arr.(i) ~ctx:h atoms_arr.(j) in
          if SetRelation.is_disjoint rel && in_ i && in_ j then ok := false;
          if SetRelation.is_subset rel && in_ i && not (in_ j) then ok := false;
          if SetRelation.is_superset rel && in_ j && not (in_ i) then
            ok := false
        end
      done
    done;
    !ok

  (** Returns [Some true] if EVERY consistent valuation satisfies the
      property, [Some false] if some consistent valuation falsifies it,
      or [None] if there are too many atoms to enumerate. *)
  let check h s1 s2 ~prop =
    let all_atoms = atoms (Union (s1, s2)) in
    let n = List.length all_atoms in
    if n > 20 then
      None
    else begin
      let atoms_arr = Array.of_list all_atoms in
      let result = ref true in
      for bits = 0 to (1 lsl n) - 1 do
        if !result then begin
          if is_consistent h atoms_arr bits then begin
            let v tag =
              match Array.findi atoms_arr ~f:(fun _ a -> equal_tag a tag) with
              | Some (i, _) -> bits land (1 lsl i) <> 0
              | None -> false
            in
            if not (prop (eval v s1) (eval v s2)) then result := false
          end
        end
      done;
      Some !result
    end

  let is_subset h s1 s2 = check h s1 s2 ~prop:(fun in1 in2 -> (not in1) || in2)

  let are_disjoint h s1 s2 =
    check h s1 s2 ~prop:(fun in1 in2 -> not (in1 && in2))
end

module Test (Impl : S_for_test) = struct
  module A = ASet (Impl)
  module TD = To_datatypes (Impl)
  module GTS = Gen_type_set (Impl)

  (** Replay an ASet op tree as a ValSet expression *)
  let rec op_to_valset : A.op -> ValSet.t = function
    | A.Singleton t -> ValSet.singleton t
    | A.MixedOp -> ValSet.singleton Mixed
    | A.EmptyOp -> ValSet.empty
    | A.Union (a, b) -> ValSet.union (op_to_valset a) (op_to_valset b)
    | A.Inter (a, b) -> ValSet.inter (op_to_valset a) (op_to_valset b)
    | A.Diff (a, b) -> ValSet.diff (op_to_valset a) (op_to_valset b)

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
      let gt = GTS.gen_type_set h denote in
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
      let gt = GTS.gen_type_set h denote in
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
      let gt = GTS.gen_type_set h denote in
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
      let gt = GTS.gen_type_set h denote in
      let%bind s1 = gt in
      let%map s2 = gt in
      (h, s1, s2)
    in
    Quickcheck.test ~trials:10000 ~sexp_of:[%sexp_of: _] ~f gen

  (* ---- test_negated_interface_incompleteness ---- *)
  (* Models typing_refinement.ml's split_ty_by_tag:
     predicate_complement = diff(mixed, singleton(Interface))
     Check: are_disjoint(singleton(FinalClass), predicate_complement)
     Ground truth: Final class unrelated to interface IS subset of complement,
     so they are NOT disjoint. But ApproxSet may fail to prove this because
     complement(none) = none. *)
  let test_negated_interface_incompleteness _ =
    let f (h, iface_id, class_id) =
      let denote = make_denotation h in
      let iface_dt = TD.to_datatypes h denote iface_id in
      let mixed_set = A.mixed_with_denote denote in
      let iface_complement = A.diff mixed_set iface_dt in
      let class_dt = TD.to_datatypes h denote class_id in
      let disj = A.are_disjoint h class_dt iface_complement in
      if disj then
        assert_bool
          "are_disjoint(class, complement(interface)) should be sound"
          (Set.are_disjoint class_dt.A.expected iface_complement.A.expected)
    in
    let gen =
      let open Quickcheck.Let_syntax in
      let%bind h = gen_hierarchy_from_pool in
      let iface_ids =
        List.filter (List.init h.num_classes ~f:Fun.id) ~f:(fun i ->
            h.is_interface i)
      in
      let class_ids =
        List.filter (List.init h.num_classes ~f:Fun.id) ~f:(fun i ->
            not (h.is_interface i))
      in
      match (iface_ids, class_ids) with
      | ([], _)
      | (_, []) ->
        let%map _ = Quickcheck.Generator.return () in
        (h, 0, 0)
      | _ ->
        let%bind iface_id = Quickcheck.Generator.of_list iface_ids in
        let%map class_id = Quickcheck.Generator.of_list class_ids in
        (h, iface_id, class_id)
    in
    Quickcheck.test ~trials:5000 ~sexp_of:[%sexp_of: _] ~f gen

  (* ---- test_completeness ---- *)
  let test_completeness _ =
    let f (h, s1, s2) =
      (* Soundness: if impl says true, witness denotation must agree *)
      let impl_disj = A.are_disjoint h s1 s2 in
      let impl_sub = A.is_subset h s1 s2 in
      if impl_disj then
        assert_bool
          (Printf.sprintf
             "[%s] Unsound: claimed disjoint but ground truth disagrees"
             Impl.name)
          (Set.are_disjoint s1.A.expected s2.A.expected);
      if impl_sub then
        assert_bool
          (Printf.sprintf
             "[%s] Unsound: claimed subset but ground truth disagrees"
             Impl.name)
          (Set.is_subset s1.A.expected ~of_:s2.A.expected);
      (* Completeness: compare BddSet against ValSet reference.
         Only check when BddSet says false (early exit optimization). *)
      if Impl.expect_complete then begin
        if not impl_disj then begin
          let vs1 = op_to_valset s1.A.op in
          let vs2 = op_to_valset s2.A.op in
          match ValSet.are_disjoint h vs1 vs2 with
          | Some true ->
            assert_failure
              (Printf.sprintf
                 "[%s] Incomplete disjointness: reference impl proves disjoint but impl doesn't\n  s1 = %s\n  s2 = %s"
                 Impl.name
                 (Sexp.to_string_hum (A.sexp_of_t s1))
                 (Sexp.to_string_hum (A.sexp_of_t s2)))
          | _ -> ()
        end;
        if not impl_sub then begin
          let vs1 = op_to_valset s1.A.op in
          let vs2 = op_to_valset s2.A.op in
          match ValSet.is_subset h vs1 vs2 with
          | Some true ->
            assert_failure
              (Printf.sprintf
                 "[%s] Incomplete subset: reference impl proves subset but impl doesn't\n  s1 = %s\n  s2 = %s"
                 Impl.name
                 (Sexp.to_string_hum (A.sexp_of_t s1))
                 (Sexp.to_string_hum (A.sexp_of_t s2)))
          | _ -> ()
        end
      end
    in
    let gen =
      let open Quickcheck.Let_syntax in
      let%bind h = gen_hierarchy_from_pool in
      let denote = make_denotation h in
      let%bind s1 = GTS.gen_type_set h denote in
      let%map s2 = GTS.gen_type_set h denote in
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
      "test_negated_interface_incompleteness"
      >:: test_negated_interface_incompleteness;
      "test_completeness" >:: test_completeness;
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

  let expect_complete = true
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
