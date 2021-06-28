(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Hh_prelude
open OUnit2
open Diagnostic_pusher.TestExporter
open Diagnostic_pusher.TestExporter.ErrorTracker

open Diagnostic_pusher.TestExporter.ErrorTracker.TestExporter

let file1 : Relative_path.t =
  Relative_path.create Relative_path.Dummy "file1.php"

let file2 : Relative_path.t =
  Relative_path.create Relative_path.Dummy "file2.php"

let files1 : Relative_path.Set.t = Relative_path.Set.of_list [file1]

let files12 : Relative_path.Set.t = Relative_path.Set.of_list [file1; file2]

let error1 : Errors.error = Errors.make_error 123 (Pos.none, "error1") []

let error2 : Errors.error = Errors.make_error 124 (Pos.none, "error2") []

let error3 : Errors.error = Errors.make_error 125 (Pos.none, "error3") []

let no_errors : Errors.t = Errors.from_file_error_list []

let one_error file error : Errors.t =
  Errors.from_file_error_list [(file, error)]

let errors1 file : Errors.t = one_error file error1

let errors2 file : Errors.t = one_error file error2

let errors12 file : Errors.t =
  Errors.from_file_error_list [(file, error1); (file, error2)]

let assert_equal_trackers =
  assert_equal ~printer:ErrorTracker.show ~cmp:ErrorTracker.equal

let test_never_commit _ =
  let tracker = init in

  (* Errors are [file1 -> error1] *)
  let (tracker, _errors) =
    get_errors_to_push tracker ~rechecked:files1 (errors1 file1)
  in
  let expected_tracker =
    make
      ~errors_in_ide:FileMap.empty
      ~to_push:(FileMap.of_list [(file1, [error1])])
  in
  assert_equal_trackers expected_tracker tracker;

  (* Now errors are [file1 -> error2], and we were not able to push to client. *)
  let (tracker, _errors) =
    get_errors_to_push tracker ~rechecked:files1 @@ errors2 file1
  in
  let expected_tracker =
    make
      ~errors_in_ide:FileMap.empty
      ~to_push:(FileMap.of_list [(file1, [error2])])
  in
  assert_equal_trackers expected_tracker tracker;

  (* Now there are no more errors, and we were still unable to push to client. *)
  let (tracker, _errors) =
    get_errors_to_push tracker ~rechecked:files1 no_errors
  in
  let expected_tracker =
    make ~errors_in_ide:FileMap.empty ~to_push:(FileMap.of_list [])
  in
  assert_equal_trackers expected_tracker tracker;

  ()

let test_push_commit_erase _ =
  let tracker = init in

  (* Errors are [file1 -> [error1; error2] *)
  let (tracker, _errors) =
    get_errors_to_push tracker ~rechecked:files12 @@ errors12 file1
  in
  let expected_to_push = FileMap.of_list [(file1, [error1; error2])] in
  let expected_tracker =
    make ~errors_in_ide:FileMap.empty ~to_push:expected_to_push
  in
  assert_equal_trackers expected_tracker tracker;

  (* Push successful *)
  let tracker = commit_pushed_errors tracker in
  let expected_pushed_so_far = expected_to_push in
  let expected_tracker =
    make ~errors_in_ide:expected_pushed_so_far ~to_push:FileMap.empty
  in
  assert_equal_trackers expected_tracker tracker;

  (* Now errors are [file1 -> [error1] *)
  let (tracker, _errors) =
    get_errors_to_push tracker ~rechecked:files12 @@ errors1 file1
  in
  let expected_to_push = FileMap.of_list [(file1, [error1])] in
  let expected_tracker =
    make ~errors_in_ide:expected_pushed_so_far ~to_push:expected_to_push
  in
  assert_equal_trackers expected_tracker tracker;

  (* Push successful *)
  let tracker = commit_pushed_errors tracker in
  let expected_pushed_so_far = expected_to_push in
  let expected_tracker =
    make ~errors_in_ide:expected_pushed_so_far ~to_push:FileMap.empty
  in
  assert_equal_trackers expected_tracker tracker;

  (* Now errors are empty *)
  let (tracker, _errors) =
    get_errors_to_push tracker ~rechecked:files1 @@ no_errors
  in
  let expected_to_push = FileMap.of_list [(file1, [])] in
  let expected_tracker =
    make ~errors_in_ide:expected_pushed_so_far ~to_push:expected_to_push
  in
  assert_equal_trackers expected_tracker tracker;

  (* Push successful *)
  let tracker = commit_pushed_errors tracker in
  let expected_tracker =
    make ~errors_in_ide:FileMap.empty ~to_push:FileMap.empty
  in
  assert_equal_trackers expected_tracker tracker;

  ()

let () =
  "diagnostic_pusher_test"
  >::: [
         "test_never_commit" >:: test_never_commit;
         "test_push_commit_erase" >:: test_push_commit_erase;
       ]
  |> run_test_tt_main
