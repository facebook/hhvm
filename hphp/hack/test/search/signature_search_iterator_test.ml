(**
 * Copyright (c) 2018, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Hh_core
open OUnit
module Iter = SignatureSearchIterator

let default_files =
  [
    "simple", "3\n5\n6\n9\n11\n";
    "incremental", "1\n2\n3\n4\n5\n6\n7\n8\n9\n10\n";
    "singleton", "3\n";
    "empty", "\n";
  ]

let test_dir = "/tmp/iter_test/"

let rec iter_to_list iter =
  match Iter.next iter with
  | None -> []
  | Some curr -> [curr] @ iter_to_list iter

let init_tempdir () =
  Sys_utils.mkdir_p test_dir;
  List.iter default_files ~f:(fun (file, contents) ->
    Sys_utils.write_file ~file: (test_dir ^ file) contents
  )

let assert_next
    ~file_name
    ~exp
    () =
  let iter = Iter.from_file (test_dir ^ file_name) in
  let results = iter_to_list iter in
  assert_equal exp results

let assert_list
    ~file_list
    ~exp
    () =
  let iter = Iter.from_list file_list in
  let results = iter_to_list iter in
  assert_equal exp results

let assert_or
    ~file_name_1
    ~file_name_2
    ~exp
    () =
  let iter_1 = Iter.from_file (test_dir ^ file_name_1) in
  let iter_2 = Iter.from_file (test_dir ^ file_name_2) in
  let or_iter = Iter.make_or iter_1 iter_2 in
  let results = iter_to_list or_iter in
  assert_equal exp results

let assert_and
    ~file_name_1
    ~file_name_2
    ~exp
    () =
  let iter_1 = Iter.from_file (test_dir ^ file_name_1) in
  let iter_2 = Iter.from_file (test_dir ^ file_name_2) in
  let and_iter = Iter.make_and iter_1 iter_2 in
  let results = iter_to_list and_iter in
  assert_equal exp results

let assert_diff
    ~file_name_1
    ~file_name_2
    ~exp
    () =
  let iter_1 = Iter.from_file (test_dir ^ file_name_1) in
  let iter_2 = Iter.from_file (test_dir ^ file_name_2) in
  let diff_iter = Iter.make_diff iter_1 iter_2 in
  let results = iter_to_list diff_iter in
  assert_equal exp results

let iterator_test_suite =
  init_tempdir ();
  "update_index" >:::
  [ "file_iter" >::
      assert_next
        ~file_name:"simple"
        ~exp: [3;5;6;9;11]

  ; "list_iter" >::
      assert_list
        ~file_list:[1;2;3;4;5;6;7]
        ~exp: [1;2;3;4;5;6;7]
  ; "or_iter" >::
      assert_or
        ~file_name_1:"simple"
        ~file_name_2:"incremental"
        ~exp: [1;2;3;4;5;6;7;8;9;10;11]

  ; "and_iter" >::
      assert_and
        ~file_name_1:"simple"
        ~file_name_2:"incremental"
        ~exp: [3;5;6;9]

  ; "diff_iter" >::
      assert_diff
        ~file_name_1:"incremental"
        ~file_name_2:"simple"
        ~exp: [1;2;4;7;8;10]
  ; "end_and_early" >::
      assert_and
        ~file_name_1:"incremental"
        ~file_name_2:"singleton"
        ~exp: [3]

  ; "diff_iter_singleton" >::
      assert_diff
        ~file_name_1:"incremental"
        ~file_name_2:"singleton"
        ~exp: [1;2;4;5;6;7;8;9;10]

  ; "empty_list_diff" >::
      assert_diff
        ~file_name_1:"simple"
        ~file_name_2:"empty"
        ~exp: [3;5;6;9;11]

  ]

let _ : OUnit.test_result list =
  run_test_tt_main iterator_test_suite
