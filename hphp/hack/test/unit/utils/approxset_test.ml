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
  @@ Quickcheck.Generator.of_list
       ['A'; 'B'; 'C'; 'D'; 'E'; 'F'; 'G'; 'H'; 'I'; 'J']

let non_empty_char_set =
  Quickcheck.Generator.filter char_set ~f:(fun s -> not @@ Set.is_empty s)

module Atom = struct
  type t = Char.Set.t [@@deriving sexp_of]

  type ctx = unit

  let relation set1 ~ctx:_ set2 =
    let open ApproxSet.Set_relation in
    if Char.Set.equal set1 set2 then
      Equal
    else if Set.is_subset set1 ~of_:set2 then
      Subset
    else if Set.is_subset set2 ~of_:set1 then
      Superset
    else if Set.are_disjoint set1 set2 then
      Disjoint
    else
      Unknown
end

module ASet = struct
  module Impl = ApproxSet.Make (Atom)

  type op =
    | Union of op * op
    | Inter of op * op
    | Set of Atom.t
    | Empty
    | Diff of op * op
  [@@deriving sexp_of]

  type t = {
    set: (Impl.t[@sexp.opaque]);
    op: op;
    expected: Char.Set.t;
  }

  let sexp_of_t { set = _; op; expected } =
    [%sexp_of: op * Char.Set.t] (op, expected)

  let empty = { set = Impl.empty; op = Empty; expected = Char.Set.empty }

  let singleton set = { set = Impl.singleton set; op = Set set; expected = set }

  let union l r =
    {
      set = Impl.union l.set r.set;
      op = Union (l.op, r.op);
      expected = Set.union l.expected r.expected;
    }

  let inter l r =
    {
      set = Impl.inter l.set r.set;
      op = Inter (l.op, r.op);
      expected = Set.inter l.expected r.expected;
    }

  let diff l r =
    {
      set = Impl.diff l.set r.set;
      op = Diff (l.op, r.op);
      expected = Set.diff l.expected r.expected;
    }

  let of_list elt =
    List.fold_left
      ~f:(fun acc tag -> union acc @@ singleton tag)
      ~init:empty
      elt

  let disjoint l r = Impl.disjoint () l.set r.set

  let are_disjoint l r = Impl.are_disjoint () l.set r.set

  let relate l r = Impl.relate () l.set r.set

  let non_empty gen =
    Quickcheck.Generator.filter gen ~f:(fun set ->
        not @@ Set.is_empty set.expected)

  let gen : t Quickcheck.Generator.t =
    let module Gen = Quickcheck.Generator in
    let open Quickcheck.Let_syntax in
    non_empty
    @@ Gen.recursive_union
         [
           return empty;
           Gen.map char_set ~f:singleton;
           Gen.map char_set ~f:singleton;
         ]
         ~f:(fun self ->
           let union =
             self >>= fun left ->
             self >>| fun right -> union left right
           in
           let inter =
             self >>= fun left ->
             self >>| fun right -> inter left right
           in
           let diff =
             self >>= fun left ->
             self >>| fun right -> diff left right
           in
           [union; inter; diff])
end

let mk lst : ASet.t = ASet.singleton @@ Char.Set.of_list lst

let a = mk ['a']

let d = mk ['d']

let ab = mk ['a'; 'b']

let abc = mk ['a'; 'b'; 'c']

let bc = mk ['b'; 'c']

let de = mk ['d'; 'e']

let def = mk ['d'; 'e'; 'f']

let ef = mk ['e'; 'f']

let is_sat = function
  | ASet.Impl.Sat -> true
  | ASet.Impl.Unsat _ -> false

let is_unsat = function
  | ASet.Impl.Sat -> false
  | ASet.Impl.Unsat _ -> true

let assert_sat msg set1 set2 =
  assert_bool msg (is_sat @@ ASet.disjoint set1 set2)

let assert_unsat msg set1 set2 =
  assert_bool msg (is_unsat @@ ASet.disjoint set1 set2)

let ( ||| ) set1 set2 = ASet.union set1 set2

let ( &&& ) set1 set2 = ASet.inter set1 set2

let ( --- ) set1 set2 = ASet.diff set1 set2

let test_union _ =
  assert_sat
    "Union(bc, ab) is disjoint from Union(de, ef)"
    (bc ||| ab)
    (de ||| ef);
  assert_unsat "Union(bc, ab) is not disjoint from abc" abc (ab ||| bc);
  assert_sat "Union(bc, ab) is disjoint from def" def (bc ||| ab);
  assert_unsat
    "Union(ab, bc, ef, de) is not disjoint from def"
    def
    (ab ||| bc ||| ef ||| de)

let test_intersection _ =
  assert_unsat "'a' is not disjoint from Inter(bc, abc)" a (ab &&& abc);
  assert_sat "'a' is disjoint from Inter(bc, abc)" a (ab &&& bc);
  assert_sat "'a' is disjoint from Inter(abc, ab, def)" abc (abc &&& ab &&& def);
  assert_sat
    "Inter(abc, ab) is disjoint from Inter(def, de)"
    (abc &&& ab)
    (def &&& de);
  assert_sat
    "Inter(ab, bc) is disjoint from Inter(de, ab)"
    (ab &&& bc)
    (de &&& ab)

let test_diff _ =
  assert_sat "'a' is disjoint from Diff(abc, ab)" a (abc --- ab);
  assert_unsat "'a' is not disjoint from Diff(abc,  bc)" a (abc --- bc)

let test_complex _ =
  let set = (abc ||| def) --- (ab &&& de) --- (bc ||| ab --- def) in
  let result = is_sat @@ ASet.disjoint a set in

  let ( ||| ) set1 set2 = Set.union set1 set2 in
  let ( &&& ) set1 set2 = Set.inter set1 set2 in
  let ( --- ) set1 set2 = Set.diff set1 set2 in
  let abc = Char.Set.of_list ['a'; 'b'; 'c'] in
  let def = Char.Set.of_list ['d'; 'e'; 'f'] in
  let ab = Char.Set.of_list ['a'; 'b'] in
  let de = Char.Set.of_list ['d'; 'e'] in
  let bc = Char.Set.of_list ['b'; 'c'] in
  let a = Char.Set.singleton 'a' in

  let set = (abc ||| def) --- (ab &&& de) --- (bc ||| ab --- def) in
  let expected = Set.are_disjoint a set in

  assert_equal
    ~msg:"Expect same result as performing same operation on a Char.Set"
    expected
    result

let test_origin_reporting _ =
  let lhs = a and rhs = def ||| abc in
  let res1 = ASet.disjoint lhs rhs and res2 = ASet.disjoint rhs lhs in
  assert_equal ~msg:"Expect results to be flipped" res1 res2 ~cmp:(fun t1 t2 ->
      match (t1, t2) with
      | (ASet.Impl.Sat, ASet.Impl.Sat) -> true
      | ( ASet.Impl.Unsat { left = l1; right = l2; _ },
          ASet.Impl.Unsat { left = r1; right = r2; _ } ) ->
        Char.Set.equal l1 r2 && Char.Set.equal l2 r1
      | _ -> false)

let test_unknown _ =
  let abcd = mk ['a'; 'b'; 'c'; 'd'] in
  assert_unsat "Cannot determine if abcd is disjoint from def" abcd def;
  assert_unsat "Cannot determine if def is disjoint from abcd" def abcd

let test_quick_disjoint _ =
  let f (set1, set2) =
    let expected = Set.are_disjoint set1.ASet.expected set2.ASet.expected in
    if ASet.are_disjoint set1 set2 && not expected then
      assert_failure
        "ApproxSet concluded disjoint when the underlying sets are not disjoint"
  in

  Quickcheck.test
    ~trials:10000
    ~sexp_of:[%sexp_of: ASet.t * ASet.t]
    ~f
    Quickcheck.Generator.(tuple2 ASet.gen ASet.gen)

let test_quick_relate _ =
  let f (set1, set2) =
    let fail s =
      assert_failure
      @@ Printf.sprintf
           "ApproxSet concluded %s when the underlying sets are not %s"
           s
           s
    in
    let open ApproxSet.Set_relation in
    match ASet.relate set1 set2 with
    | Equal ->
      if not @@ Set.equal set1.ASet.expected set2.ASet.expected then
        fail "equal"
    | Subset ->
      if not @@ Set.is_subset set1.ASet.expected ~of_:set2.ASet.expected then
        fail "subset"
    | Superset ->
      if not @@ Set.is_subset set2.ASet.expected ~of_:set1.ASet.expected then
        fail "superset"
    | Disjoint ->
      if not @@ Set.are_disjoint set1.ASet.expected set2.ASet.expected then
        fail "disjoint"
    | Unknown -> ()
  in

  Quickcheck.test
    ~trials:10000
    ~sexp_of:[%sexp_of: ASet.t * ASet.t]
    ~f
    Quickcheck.Generator.(tuple2 ASet.gen ASet.gen)

let () =
  "approxset"
  >::: [
         "test_union" >:: test_union;
         "test_intersection" >:: test_intersection;
         "test_diff" >:: test_diff;
         "test_complex" >:: test_complex;
         "test_unknown" >:: test_unknown;
         "test_origin_reporting" >:: test_origin_reporting;
         "test_quick_disjoint" >:: test_quick_disjoint;
         "test_quick_relate" >:: test_quick_relate;
       ]
  |> run_test_tt_main
