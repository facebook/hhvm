(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Hh_prelude
module Output = Client_isolation_move.Output
module Error = Client_isolation_move.Move_error

let source = "projects/a"

let target = "packages/deep/a"

let capture_stdout f =
  let file = Stdlib.Filename.temp_file "hh_isolation_stdout" "" in
  let fd = Unix.openfile file [Unix.O_WRONLY; Unix.O_TRUNC] 0o600 in
  Stdlib.flush Stdlib.stdout;
  let saved = Unix.dup Unix.stdout in
  Unix.dup2 fd Unix.stdout;
  Unix.close fd;
  Exn.protect ~f ~finally:(fun () ->
      Stdlib.flush Stdlib.stdout;
      Unix.dup2 saved Unix.stdout;
      Unix.close saved);
  let output = Disk.cat file in
  Stdlib.Sys.remove file;
  output

let printed ~output_json outcome =
  capture_stdout (fun () ->
      Output.print_outcome ~source ~target ~output_json outcome)

let json_field name outcome =
  Yojson.Safe.Util.member
    name
    (Yojson.Safe.from_string (printed ~output_json:true outcome))

let is_json json expected = Yojson.Safe.equal json expected

let contains = String.is_substring

let an_error n =
  {
    Error.code = 4000 + n;
    file =
      Relative_path.from_root ~suffix:(Printf.sprintf "%s/e%d.php" source n);
    line = n + 1;
    message = Printf.sprintf "first line %d\nsecond line" n;
  }

let not_moved ?(baseline_restored = true) errors =
  Output.Not_moved { errors; baseline_restored }

let test_the_text_says_what_moved () =
  String.equal
    (printed ~output_json:false (Output.Moved { files = 2 }))
    "Moved: 2 files from projects/a to packages/deep/a, with no new errors.\n"

let test_the_text_counts_the_blockers () =
  let blocked blockers = printed ~output_json:false (Output.Blocked blockers) in
  let one = blocked ["f.php"] in
  let two = blocked ["f.php"; "g.php"] in
  contains one ~substring:"Not moved: 1 file outside references in."
  && contains one ~substring:"  f.php\n"
  && contains two ~substring:"Not moved: 2 files outside reference in."
  && contains two ~substring:"  g.php\n"

let test_the_text_shows_ten_errors_and_counts_the_rest () =
  let output =
    printed ~output_json:false (not_moved (List.init 12 ~f:an_error))
  in
  contains output ~substring:"the move introduced 12 errors, and it was undone"
  && contains output ~substring:"  4000 projects/a/e0.php:1: first line 0\n"
  && contains output ~substring:"  4009 projects/a/e9.php:10: first line 9\n"
  && (not (contains output ~substring:"e10.php"))
  && contains output ~substring:"  and 2 more\n"
  && not (contains output ~substring:"second line")

let test_the_json_of_a_move_has_null_for_what_does_not_apply () =
  let moved = Output.Moved { files = 2 } in
  is_json (json_field "moved" moved) (`Bool true)
  && is_json (json_field "source" moved) (`String source)
  && is_json (json_field "target" moved) (`String target)
  && is_json (json_field "files" moved) (`Int 2)
  && is_json (json_field "refused" moved) `Null
  && is_json (json_field "blockers" moved) `Null
  && is_json (json_field "new_errors" moved) `Null
  && is_json (json_field "baseline_restored" moved) `Null

let test_the_json_of_a_refusal_and_of_blockers () =
  let refused = Output.Refused "no such directory" in
  let blocked = Output.Blocked ["lib/o.php"] in
  is_json (json_field "moved" refused) (`Bool false)
  && is_json (json_field "refused" refused) (`String "no such directory")
  && is_json (json_field "files" refused) `Null
  && is_json (json_field "moved" blocked) (`Bool false)
  && is_json (json_field "blockers" blocked) (`List [`String "lib/o.php"])
  && is_json (json_field "refused" blocked) `Null

let test_the_json_of_a_move_that_was_undone_names_the_errors () =
  let outcome = not_moved ~baseline_restored:false [an_error 3] in
  is_json (json_field "moved" outcome) (`Bool false)
  && is_json (json_field "baseline_restored" outcome) (`Bool false)
  && is_json
       (json_field "new_errors" outcome)
       (`List
         [
           `Assoc
             [
               ("code", `Int 4003);
               ("file", `String "projects/a/e3.php");
               ("line", `Int 4);
               ("message", `String "first line 3\nsecond line");
             ];
         ])

let test_the_exit_status_is_zero_only_for_a_move () =
  let code outcome = Exit_status.exit_code (Output.exit_status outcome) in
  code (Output.Moved { files = 1 }) = Exit_status.exit_code Exit_status.No_error
  && code (Output.Refused "x") = Exit_status.exit_code Exit_status.Input_error
  && code (Output.Blocked ["f.php"])
     = Exit_status.exit_code Exit_status.Type_error
  && code (not_moved [an_error 0])
     = Exit_status.exit_code Exit_status.Type_error

let tests =
  [
    ("test_the_text_says_what_moved", test_the_text_says_what_moved);
    ("test_the_text_counts_the_blockers", test_the_text_counts_the_blockers);
    ( "test_the_text_shows_ten_errors_and_counts_the_rest",
      test_the_text_shows_ten_errors_and_counts_the_rest );
    ( "test_the_json_of_a_move_has_null_for_what_does_not_apply",
      test_the_json_of_a_move_has_null_for_what_does_not_apply );
    ( "test_the_json_of_a_refusal_and_of_blockers",
      test_the_json_of_a_refusal_and_of_blockers );
    ( "test_the_json_of_a_move_that_was_undone_names_the_errors",
      test_the_json_of_a_move_that_was_undone_names_the_errors );
    ( "test_the_exit_status_is_zero_only_for_a_move",
      test_the_exit_status_is_zero_only_for_a_move );
  ]

let () = Unit_test.run_all tests
