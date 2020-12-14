(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Asserter
open Asserter.Int_asserter

let test () =
  (* empty *)
  assert_equals 0 (BigList.length BigList.empty) "empty length";
  assert_list_equals [] (BigList.as_list BigList.empty) "empty list";

  (* create *)
  let t = BigList.create [1; 2; 3] in
  assert_equals 3 (BigList.length t) "create length";
  assert_list_equals [1; 2; 3] (BigList.as_list t) "create list";

  (* cons *)
  let u = BigList.cons 0 t in
  assert_equals 4 (BigList.length u) "cons length";
  assert_list_equals [0; 1; 2; 3] (BigList.as_list u) "cons list";

  (* is_empty *)
  Bool_asserter.assert_equals false (BigList.is_empty t) "is_empty t";
  Bool_asserter.assert_equals true (BigList.is_empty BigList.empty) "is_empty";

  (* decons *)
  begin
    match BigList.decons t with
    | None -> failwith "decons t expected some"
    | Some (hd, tl) ->
      assert_equals 1 hd "decons t hd";
      assert_equals 2 (BigList.length tl) "decons t tl length";
      assert_list_equals [2; 3] (BigList.as_list tl) "decons t tl list"
  end;
  begin
    match BigList.decons BigList.empty with
    | None -> ()
    | Some _ -> failwith "decons empty expected none"
  end;

  (* filter *)
  let is_even i = i mod 2 = 0 in
  let u = BigList.filter t ~f:is_even in
  assert_equals 1 (BigList.length u) "filter t length";
  assert_list_equals [2] (BigList.as_list u) "filter t list";
  let u = BigList.filter BigList.empty ~f:is_even in
  assert_equals 0 (BigList.length u) "filter empty length";
  assert_list_equals [] (BigList.as_list u) "filter empty list";

  (* map *)
  let inc i = i + 1 in
  let u = BigList.map t ~f:inc in
  assert_equals 3 (BigList.length u) "map t length";
  assert_list_equals [2; 3; 4] (BigList.as_list u) "map t list";
  let u = BigList.map BigList.empty ~f:inc in
  assert_equals 0 (BigList.length u) "map empty length";
  assert_list_equals [] (BigList.as_list u) "map empty list";

  (* append *)
  let u = BigList.append [0; 9] t in
  assert_equals 5 (BigList.length u) "append t length";
  assert_list_equals [0; 9; 1; 2; 3] (BigList.as_list u) "append t list";

  (* rev_append *)
  let u = BigList.rev_append [0; 9] t in
  assert_equals 5 (BigList.length u) "rev_append t length";
  assert_list_equals [9; 0; 1; 2; 3] (BigList.as_list u) "rev_append t list";

  (* rev *)
  let u = BigList.rev t in
  assert_equals 3 (BigList.length u) "rev t length";
  assert_list_equals [3; 2; 1] (BigList.as_list u) "rev t list";

  (* split_n *)
  let (split, rest) = BigList.split_n t 0 in
  assert_list_equals [] split "split t 0 split";
  assert_equals 3 (BigList.length rest) "split t 0 rest length";
  assert_list_equals [1; 2; 3] (BigList.as_list rest) "split t 0 rest list";
  let (split, rest) = BigList.split_n t 1 in
  assert_list_equals [1] split "split t 1 split";
  assert_equals 2 (BigList.length rest) "split t 1 rest length";
  assert_list_equals [2; 3] (BigList.as_list rest) "split t 1 rest list";
  let (split, rest) = BigList.split_n t 3 in
  assert_list_equals [1; 2; 3] split "split t 3 split";
  assert_equals 0 (BigList.length rest) "split t 3 rest length";
  assert_list_equals [] (BigList.as_list rest) "split t 3 rest list";
  let (split, rest) = BigList.split_n t 4 in
  assert_list_equals [1; 2; 3] split "split t 4 split";
  assert_equals 0 (BigList.length rest) "split t 4 rest length";
  assert_list_equals [] (BigList.as_list rest) "split t 4 rest list";
  (* split_n empty *)
  let (split, rest) = BigList.split_n BigList.empty 0 in
  assert_list_equals [] split "split empty 0 split";
  assert_equals 0 (BigList.length rest) "split empty 0 rest length";
  assert_list_equals [] (BigList.as_list rest) "split empty 0 rest list";
  let (split, rest) = BigList.split_n BigList.empty 1 in
  assert_list_equals [] split "split empty 1 split";
  assert_equals 0 (BigList.length rest) "split empty 1 rest length";
  assert_list_equals [] (BigList.as_list rest) "split empty 1 rest list";

  true

let () = Unit_test.run_all [("biglist", test)]
