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

let file1_absolute = "file1.php"

let file1 : Relative_path.t =
  Relative_path.create Relative_path.Dummy file1_absolute

let file2_absolute = "file2.php"

let file2 : Relative_path.t =
  Relative_path.create Relative_path.Dummy file2_absolute

let files1 : Relative_path.Set.t = Relative_path.Set.of_list [file1]

let files2 : Relative_path.Set.t = Relative_path.Set.of_list [file2]

let files12 : Relative_path.Set.t = Relative_path.Set.of_list [file1; file2]

let error1 : Errors.error = Errors.make_error 123 (Pos.none, "error1") []

let error1_absolute : Errors.finalized_error = Errors.to_absolute error1

let error2 : Errors.error = Errors.make_error 124 (Pos.none, "error2") []

let error2_absolute : Errors.finalized_error = Errors.to_absolute error2

let error3 : Errors.error = Errors.make_error 125 (Pos.none, "error3") []

let no_errors : Errors.t = Errors.from_file_error_list []

let no_errors_absolute = SMap.empty

let one_error file error : Errors.t =
  Errors.from_file_error_list [(file, error)]

let one_error_absolute file error = SMap.of_list [(file, [error])]

let errors1 file : Errors.t = one_error file error1

let errors1_absolute file = one_error_absolute file error1_absolute

let errors2 file : Errors.t = one_error file error2

let errors2_absolute file = one_error_absolute file error2_absolute

let errors12 file : Errors.t =
  Errors.from_file_error_list [(file, error1); (file, error2)]

let assert_equal_trackers =
  assert_equal ~printer:ErrorTracker.show ~cmp:ErrorTracker.equal

let assert_equal_messages =
  assert_equal
    ~printer:ServerCommandTypes.show_pushes
    ~cmp:ServerCommandTypes.equal_pushes

module ErrorTrackerTest = struct
  let test_never_commit _ =
    let tracker = init in

    (* Errors are [file1 -> error1] *)
    let (tracker, _errors) =
      get_errors_to_push tracker ~rechecked:files1 ~new_errors:(errors1 file1)
    in
    let expected_tracker =
      make
        ~errors_in_ide:FileMap.empty
        ~to_push:(FileMap.of_list [(file1, [error1])])
    in
    assert_equal_trackers expected_tracker tracker;

    (* Now errors are [file1 -> error2], and we were not able to push to client. *)
    let (tracker, _errors) =
      get_errors_to_push tracker ~rechecked:files1 ~new_errors:(errors2 file1)
    in
    let expected_tracker =
      make
        ~errors_in_ide:FileMap.empty
        ~to_push:(FileMap.of_list [(file1, [error2])])
    in
    assert_equal_trackers expected_tracker tracker;

    (* Now there are no more errors, and we were still unable to push to client. *)
    let (tracker, _errors) =
      get_errors_to_push tracker ~rechecked:files1 ~new_errors:no_errors
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
      get_errors_to_push tracker ~rechecked:files12 ~new_errors:(errors12 file1)
    in
    let expected_to_push = FileMap.of_list [(file1, [error1; error2])] in
    let expected_tracker =
      make ~errors_in_ide:FileMap.empty ~to_push:expected_to_push
    in
    assert_equal_trackers expected_tracker tracker;

    (* Push successful *)
    let tracker = commit_pushed_errors tracker in
    let expected_errors_in_ide = expected_to_push in
    let expected_tracker =
      make ~errors_in_ide:expected_errors_in_ide ~to_push:FileMap.empty
    in
    assert_equal_trackers expected_tracker tracker;

    (* Now errors are [file1 -> [error1] *)
    let (tracker, _errors) =
      get_errors_to_push tracker ~rechecked:files12 ~new_errors:(errors1 file1)
    in
    let expected_to_push = FileMap.of_list [(file1, [error1])] in
    let expected_tracker =
      make ~errors_in_ide:expected_errors_in_ide ~to_push:expected_to_push
    in
    assert_equal_trackers expected_tracker tracker;

    (* Push successful *)
    let tracker = commit_pushed_errors tracker in
    let expected_errors_in_ide = expected_to_push in
    let expected_tracker =
      make ~errors_in_ide:expected_errors_in_ide ~to_push:FileMap.empty
    in
    assert_equal_trackers expected_tracker tracker;

    (* Now errors are empty *)
    let (tracker, _errors) =
      get_errors_to_push tracker ~rechecked:files1 ~new_errors:no_errors
    in
    let expected_to_push = FileMap.of_list [(file1, [])] in
    let expected_tracker =
      make ~errors_in_ide:expected_errors_in_ide ~to_push:expected_to_push
    in
    assert_equal_trackers expected_tracker tracker;

    (* Push successful *)
    let tracker = commit_pushed_errors tracker in
    let expected_tracker =
      make ~errors_in_ide:FileMap.empty ~to_push:FileMap.empty
    in
    assert_equal_trackers expected_tracker tracker;

    ()
end

let test_push _ =
  let pusher = Diagnostic_pusher.init in
  let _ = TestClientProvider.make_persistent () in

  let errors = errors1 file1 in
  let errors_absolute = errors1_absolute file1_absolute in
  let pusher =
    Diagnostic_pusher.push_new_errors pusher ~rechecked:files1 errors
  in
  let pushed_messages = TestClientProvider.get_push_messages () in
  let expected_pushed_messages =
    [
      ServerCommandTypes.DIAGNOSTIC
        { errors = errors_absolute; is_truncated = None };
    ]
  in
  assert_equal_messages expected_pushed_messages pushed_messages;

  let _pusher =
    Diagnostic_pusher.push_new_errors pusher ~rechecked:files1 no_errors
  in
  let pushed_messages = TestClientProvider.get_push_messages () in
  (* Check that we've properly erased the errors. *)
  let expected_pushed_messages =
    [
      ServerCommandTypes.DIAGNOSTIC
        { errors = SMap.of_list [(file1_absolute, [])]; is_truncated = None };
    ]
  in
  assert_equal_messages expected_pushed_messages pushed_messages;

  ()

let test_initially_no_client _ =
  let pusher = Diagnostic_pusher.init in

  let errors = errors1 file1 in
  let errors1_absolute = errors1_absolute file1_absolute in
  let pusher =
    Diagnostic_pusher.push_new_errors pusher ~rechecked:files1 errors
  in
  let pushed_messages = TestClientProvider.get_push_messages () in
  (* There is no client, so no errors should have been pushed. *)
  assert_equal_messages [] pushed_messages;

  (* Connect a persistent client. *)
  let _ = TestClientProvider.make_persistent () in

  (* Add an new batch of new errors. *)
  let errors = errors2 file2 in
  let errors2_absolute = errors2_absolute file2_absolute in
  let _pusher =
    Diagnostic_pusher.push_new_errors
      pusher
      ~rechecked:Relative_path.Set.empty
      errors
  in
  let pushed_messages = TestClientProvider.get_push_messages () in
  (* Both errors should have been pushed. *)
  let expected_pushed_messages =
    [
      ServerCommandTypes.DIAGNOSTIC
        {
          errors = SMap.union errors1_absolute errors2_absolute;
          is_truncated = None;
        };
    ]
  in
  assert_equal_messages expected_pushed_messages pushed_messages;

  ()

let test_switch_client _ =
  let pusher = Diagnostic_pusher.init in

  let _ = TestClientProvider.make_persistent () in

  let errors = errors1 file1 in
  let errors1_absolute = errors1_absolute file1_absolute in
  let pusher =
    Diagnostic_pusher.push_new_errors pusher ~rechecked:files1 errors
  in
  let pushed_messages = TestClientProvider.get_push_messages () in
  let expected_pushed_messages =
    [
      ServerCommandTypes.DIAGNOSTIC
        { errors = errors1_absolute; is_truncated = None };
    ]
  in
  assert_equal_messages expected_pushed_messages pushed_messages;

  (* A new client connects and replaces the previous one. *)
  let _ = TestClientProvider.make_persistent () in

  (* Add an new batch of new errors. *)
  let errors = errors2 file2 in
  let errors2_absolute = errors2_absolute file2_absolute in
  let _pusher =
    Diagnostic_pusher.push_new_errors pusher ~rechecked:files2 errors
  in
  let pushed_messages = TestClientProvider.get_push_messages () in
  (* Both errors should have been pushed. *)
  let expected_pushed_messages =
    [
      ServerCommandTypes.DIAGNOSTIC
        {
          errors = SMap.union errors1_absolute errors2_absolute;
          is_truncated = None;
        };
    ]
  in
  assert_equal_messages expected_pushed_messages pushed_messages;

  ()

let test_switch_client_erase _ =
  let pusher = Diagnostic_pusher.init in

  let _ = TestClientProvider.make_persistent () in

  let errors = errors1 file1 in
  let errors1_absolute = errors1_absolute file1_absolute in
  let pusher =
    Diagnostic_pusher.push_new_errors pusher ~rechecked:files1 errors
  in
  let pushed_messages = TestClientProvider.get_push_messages () in
  let expected_pushed_messages =
    [
      ServerCommandTypes.DIAGNOSTIC
        { errors = errors1_absolute; is_truncated = None };
    ]
  in
  assert_equal_messages expected_pushed_messages pushed_messages;

  (* A new client connects and replaces the previous one. *)
  let _ = TestClientProvider.make_persistent () in

  (* No more errors in file1. *)
  let _pusher =
    Diagnostic_pusher.push_new_errors pusher ~rechecked:files1 no_errors
  in
  let pushed_messages = TestClientProvider.get_push_messages () in
  (* There was no need to erase any message. *)
  assert_equal_messages [] pushed_messages;

  ()

let () =
  "diagnostic_pusher_test"
  >::: [
         "ErrorTrackerTest.test_never_commit"
         >:: ErrorTrackerTest.test_never_commit;
         "ErrorTrackerTest.test_push_commit_erase"
         >:: ErrorTrackerTest.test_push_commit_erase;
         "test_push" >:: test_push;
         "test_initially_no_client" >:: test_initially_no_client;
         "test_switch_client" >:: test_switch_client;
         "test_switch_client_erase" >:: test_switch_client_erase;
       ]
  |> run_test_tt_main
