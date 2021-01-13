(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Hh_prelude
open OUnit2

let test_small _ =
  let line = 30 in
  let bol = 2349 in
  let start_cnum = 2398 in
  let end_cnum = 2450 in
  let pos_start =
    { File_pos_large.pos_lnum = line; pos_bol = bol; pos_cnum = start_cnum }
  in
  let pos_end =
    { File_pos_large.pos_lnum = line; pos_bol = bol; pos_cnum = end_cnum }
  in
  match Pos_span_tiny.make ~pos_start ~pos_end with
  | None -> assert_failure "Could not create tiny span."
  | Some span ->
    let start_line = Pos_span_tiny.start_line_number span in
    assert_equal ~msg:(Printf.sprintf "%d" start_line) line start_line;
    assert_equal line @@ Pos_span_tiny.end_line_number span;
    assert_equal bol @@ Pos_span_tiny.start_beginning_of_line span;
    assert_equal bol @@ Pos_span_tiny.end_beginning_of_line span;
    assert_equal start_cnum @@ Pos_span_tiny.start_character_number span;
    assert_equal end_cnum @@ Pos_span_tiny.end_character_number span;
    assert_equal (start_cnum - bol) @@ Pos_span_tiny.start_column span;
    assert_equal (end_cnum - bol) @@ Pos_span_tiny.end_column span;
    let (pos_start', pos_end') = Pos_span_tiny.as_large_span span in
    assert_equal ~msg:(File_pos_large.show pos_start') pos_start pos_start';
    assert_equal pos_end pos_end';
    ()

let test_dummy _ =
  let line = 0 in
  let bol = 0 in
  let start_cnum = -1 in
  let end_cnum = -1 in
  let pos_start = File_pos_large.dummy in
  let pos_end = File_pos_large.dummy in
  let span = Pos_span_tiny.dummy in
  assert_equal line @@ Pos_span_tiny.start_line_number span;
  assert_equal line @@ Pos_span_tiny.end_line_number span;
  assert_equal bol @@ Pos_span_tiny.start_beginning_of_line span;
  assert_equal bol @@ Pos_span_tiny.end_beginning_of_line span;
  assert_equal start_cnum @@ Pos_span_tiny.start_character_number span;
  assert_equal end_cnum @@ Pos_span_tiny.end_character_number span;
  assert_equal (start_cnum - bol) @@ Pos_span_tiny.start_column span;
  assert_equal (end_cnum - bol) @@ Pos_span_tiny.end_column span;
  let (pos_start', pos_end') = Pos_span_tiny.as_large_span span in
  assert_equal pos_start pos_start';
  assert_equal pos_end pos_end';
  ()

let test_large _ =
  let max_int = Int.max_value in
  let line = max_int in
  let bol = max_int in
  let start_cnum = max_int in
  let end_cnum = max_int in
  let pos_start =
    { File_pos_large.pos_lnum = line; pos_bol = bol; pos_cnum = start_cnum }
  in
  let pos_end =
    { File_pos_large.pos_lnum = line; pos_bol = bol; pos_cnum = end_cnum }
  in
  match Pos_span_tiny.make ~pos_start ~pos_end with
  | None -> (* expected *) ()
  | Some span ->
    (* [span] is most likely erronous and the following asserts will fail. *)
    let start_line = Pos_span_tiny.start_line_number span in
    assert_equal ~msg:(Printf.sprintf "%d" start_line) line start_line;
    assert_equal line @@ Pos_span_tiny.end_line_number span;
    assert_equal bol @@ Pos_span_tiny.start_beginning_of_line span;
    assert_equal bol @@ Pos_span_tiny.end_beginning_of_line span;
    assert_equal start_cnum @@ Pos_span_tiny.start_character_number span;
    assert_equal end_cnum @@ Pos_span_tiny.end_character_number span;
    assert_equal (start_cnum - bol) @@ Pos_span_tiny.start_column span;
    assert_equal (end_cnum - bol) @@ Pos_span_tiny.end_column span;
    let (pos_start', pos_end') = Pos_span_tiny.as_large_span span in
    assert_equal ~msg:(File_pos_large.show pos_start') pos_start pos_start';
    assert_equal pos_end pos_end';
    ()

let () =
  "posSpanTinyTest"
  >::: [
         "test_small" >:: test_small;
         "test_dummy" >:: test_dummy;
         "test_large" >:: test_large;
       ]
  |> run_test_tt_main
