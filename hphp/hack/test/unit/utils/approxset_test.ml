(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Hh_prelude
open OUnit2

module ASet = ApproxSet.Make (struct
  type 'a t = Char.Set.t

  type ctx = unit

  let relation set1 ~ctx:_ set2 =
    let open ApproxSet.Set_relation in
    if Char.Set.equal set1 set2 then
      Equal
    else if Char.Set.is_subset set1 ~of_:set2 then
      Subset
    else if Char.Set.is_subset set2 ~of_:set1 then
      Superset
    else if Char.Set.are_disjoint set1 set2 then
      Disjoint
    else
      Unknown
end)

let mk lst : unit ASet.t = ASet.singleton @@ Char.Set.of_list lst

let a = mk ['a']

let d = mk ['d']

let ab = mk ['a'; 'b']

let abc = mk ['a'; 'b'; 'c']

let bc = mk ['b'; 'c']

let de = mk ['d'; 'e']

let def = mk ['d'; 'e'; 'f']

let ef = mk ['e'; 'f']

let is_sat = function
  | ASet.Sat -> true
  | ASet.Unsat _ -> false

let is_unsat = function
  | ASet.Sat -> false
  | ASet.Unsat _ -> true

let assert_sat msg set1 set2 =
  assert_bool msg (is_sat @@ ASet.disjoint () set1 set2)

let assert_unsat msg set1 set2 =
  assert_bool msg (is_unsat @@ ASet.disjoint () set1 set2)

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
  let result = is_sat @@ ASet.disjoint () a set in

  let ( ||| ) set1 set2 = Char.Set.union set1 set2 in
  let ( &&& ) set1 set2 = Char.Set.inter set1 set2 in
  let ( --- ) set1 set2 = Char.Set.diff set1 set2 in
  let abc = Char.Set.of_list ['a'; 'b'; 'c'] in
  let def = Char.Set.of_list ['d'; 'e'; 'f'] in
  let ab = Char.Set.of_list ['a'; 'b'] in
  let de = Char.Set.of_list ['d'; 'e'] in
  let bc = Char.Set.of_list ['b'; 'c'] in
  let a = Char.Set.singleton 'a' in

  let set = (abc ||| def) --- (ab &&& de) --- (bc ||| ab --- def) in
  let expected = Char.Set.are_disjoint a set in

  assert_equal
    ~msg:"Expect same result as performing same operation on a Char.Set"
    expected
    result

let test_origin_reporting _ =
  let lhs = a and rhs = def ||| abc in
  let res1 = ASet.disjoint () lhs rhs and res2 = ASet.disjoint () rhs lhs in
  assert_equal
    ~msg:"Expect same result whichever way round we provide arguments"
    res1
    res2
    ~cmp:(fun t1 t2 ->
      match (t1, t2) with
      | (ASet.Sat, ASet.Sat) -> true
      | ( ASet.Unsat { left = l1; right = l2; _ },
          ASet.Unsat { left = r1; right = r2; _ } ) ->
        Char.Set.equal l1 r1 && Char.Set.equal l2 r2
      | _ -> false)

let test_unknown _ =
  let abcd = mk ['a'; 'b'; 'c'; 'd'] in
  assert_unsat "Cannot determine if abcd is disjoint from def" abcd def;
  assert_unsat "Cannot determine if def is disjoint from abcd" def abcd

let () =
  "approxset"
  >::: [
         "test_union" >:: test_union;
         "test_intersection" >:: test_intersection;
         "test_diff" >:: test_diff;
         "test_complex" >:: test_complex;
         "test_unknown" >:: test_unknown;
         "test_origin_reporting" >:: test_origin_reporting;
       ]
  |> run_test_tt_main
