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

module Constants = struct
  let file1_absolute = "file1.php"

  let file1 : Relative_path.t =
    Relative_path.create Relative_path.Dummy file1_absolute

  let file2_absolute = "file2.php"

  let file2 : Relative_path.t =
    Relative_path.create Relative_path.Dummy file2_absolute

  let files1 : Relative_path.Set.t = Relative_path.Set.of_list [file1]

  let files2 : Relative_path.Set.t = Relative_path.Set.of_list [file2]

  let files12 : Relative_path.Set.t = Relative_path.Set.of_list [file1; file2]

  let error1 : Errors.error = User_error.make 123 (Pos.none, "error1") []

  let error1_absolute : Errors.finalized_error = User_error.to_absolute error1

  let error2 : Errors.error = User_error.make 124 (Pos.none, "error2") []

  let error2_absolute : Errors.finalized_error = User_error.to_absolute error2

  let error3 : Errors.error = User_error.make 125 (Pos.none, "error3") []

  let error3_absolute : Errors.finalized_error = User_error.to_absolute error3

  let no_errors : Errors.t = Errors.from_file_error_list []

  (* let no_errors_absolute = SMap.empty *)

  let one_error file error : Errors.t =
    Errors.from_file_error_list [(file, error)]

  let one_error_absolute file error = SMap.of_list [(file, [error])]

  let errors1 file : Errors.t = one_error file error1

  let errors1_absolute file = one_error_absolute file error1_absolute

  let errors2 file : Errors.t = one_error file error2

  let errors2_absolute file = one_error_absolute file error2_absolute

  let errors12 file : Errors.t =
    Errors.from_file_error_list [(file, error1); (file, error2)]

  let errors_of_list phase list =
    List.map list ~f:(fun (file, errors) ->
        (file, Errors.PhaseMap.of_list [(phase, errors)]))
    |> FileMap.of_list
end

open Constants

let assert_equal_trackers =
  assert_equal ~printer:ErrorTracker.show ~cmp:ErrorTracker.equal

let assert_equal_messages =
  assert_equal
    ~printer:ServerCommandTypes.show_pushes
    ~cmp:ServerCommandTypes.equal_pushes

module ErrorTrackerTest = struct
  let make_no_limit = make ~errors_beyond_limit:FileMap.empty

  let test_never_commit _ =
    let phase = Errors.Typing in
    let tracker = init in

    (* Errors are [file1 -> error1] *)
    let (tracker, _errors) =
      get_errors_to_push
        ~phase
        tracker
        ~rechecked:files1
        ~new_errors:(errors1 file1)
        ~priority_files:None
    in
    let expected_tracker =
      make_no_limit
        ~errors_in_ide:FileMap.empty
        ~to_push:(errors_of_list phase [(file1, [error1])])
    in
    assert_equal_trackers expected_tracker tracker;

    (* Now errors are [file1 -> error2], and we were not able to push to client. *)
    let (tracker, _errors) =
      get_errors_to_push
        ~phase
        tracker
        ~rechecked:files1
        ~new_errors:(errors2 file1)
        ~priority_files:None
    in
    let expected_tracker =
      make_no_limit
        ~errors_in_ide:FileMap.empty
        ~to_push:(errors_of_list phase [(file1, [error2])])
    in
    assert_equal_trackers expected_tracker tracker;

    (* Now there are no more errors, and we were still unable to push to client. *)
    let (tracker, _errors) =
      get_errors_to_push
        ~phase
        tracker
        ~rechecked:files1
        ~new_errors:no_errors
        ~priority_files:None
    in
    let expected_tracker =
      make_no_limit ~errors_in_ide:FileMap.empty ~to_push:(FileMap.of_list [])
    in
    assert_equal_trackers expected_tracker tracker;

    ()

  let test_push_commit_erase _ =
    let phase = Errors.Typing in
    let tracker = init in

    (* Errors are [file1 -> [error1; error2] *)
    let (tracker, _errors) =
      get_errors_to_push
        ~phase
        tracker
        ~rechecked:files12
        ~new_errors:(errors12 file1)
        ~priority_files:None
    in
    let expected_to_push = errors_of_list phase [(file1, [error1; error2])] in
    let expected_tracker =
      make_no_limit ~errors_in_ide:FileMap.empty ~to_push:expected_to_push
    in
    assert_equal_trackers expected_tracker tracker;

    (* Push successful *)
    let tracker = commit_pushed_errors tracker in
    let expected_errors_in_ide = expected_to_push in
    let expected_tracker =
      make_no_limit ~errors_in_ide:expected_errors_in_ide ~to_push:FileMap.empty
    in
    assert_equal_trackers expected_tracker tracker;

    (* Now errors are [file1 -> [error1] *)
    let (tracker, _errors) =
      get_errors_to_push
        ~phase
        tracker
        ~rechecked:files12
        ~new_errors:(errors1 file1)
        ~priority_files:None
    in
    let expected_to_push = errors_of_list phase [(file1, [error1])] in
    let expected_tracker =
      make_no_limit
        ~errors_in_ide:expected_errors_in_ide
        ~to_push:expected_to_push
    in
    assert_equal_trackers expected_tracker tracker;

    (* Push successful *)
    let tracker = commit_pushed_errors tracker in
    let expected_errors_in_ide = expected_to_push in
    let expected_tracker =
      make_no_limit ~errors_in_ide:expected_errors_in_ide ~to_push:FileMap.empty
    in
    assert_equal_trackers expected_tracker tracker;

    (* Now errors are empty *)
    let (tracker, _errors) =
      get_errors_to_push
        ~phase
        tracker
        ~rechecked:files1
        ~new_errors:no_errors
        ~priority_files:None
    in
    let expected_to_push = errors_of_list phase [(file1, [])] in
    let expected_tracker =
      make_no_limit
        ~errors_in_ide:expected_errors_in_ide
        ~to_push:expected_to_push
    in
    assert_equal_trackers expected_tracker tracker;

    (* Push successful *)
    let tracker = commit_pushed_errors tracker in
    let expected_tracker =
      make_no_limit ~errors_in_ide:FileMap.empty ~to_push:FileMap.empty
    in
    assert_equal_trackers expected_tracker tracker;

    ()
end

let connect_persistent () =
  TestClientProvider.mock_new_client_type ServerCommandTypes.Non_persistent;
  let client_provider = ClientProvider.provider_for_test () in
  let client =
    match
      ClientProvider.sleep_and_check
        client_provider
        None
        ~ide_idle:true
        ~idle_gc_slice:0
        `Any
    with
    | ClientProvider.Select_new
        { ClientProvider.client; m2s_sequence_number = _ } ->
      client
    | ClientProvider.Select_persistent
    | ClientProvider.Select_nothing
    | ClientProvider.Select_exception _
    | ClientProvider.Not_selecting_hg_updating ->
      assert false
  in
  Ide_info_store.new_ client;
  ()

let test_push _ =
  let phase = Errors.Typing in
  let pusher = Diagnostic_pusher.init in
  connect_persistent ();

  let errors = errors1 file1 in
  let errors_absolute = errors1_absolute file1_absolute in
  let (pusher, _) =
    Diagnostic_pusher.push_new_errors pusher ~rechecked:files1 errors ~phase
  in
  let pushed_messages = TestClientProvider.get_push_messages () in
  let expected_pushed_messages =
    [
      ServerCommandTypes.DIAGNOSTIC
        { errors = errors_absolute; is_truncated = None };
    ]
  in
  assert_equal_messages expected_pushed_messages pushed_messages;

  let (pusher, _) =
    Diagnostic_pusher.push_new_errors pusher ~rechecked:files1 no_errors ~phase
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

  let (_ : Diagnostic_pusher.t) = pusher in
  ()

let test_initially_no_client _ =
  let phase = Errors.Typing in
  let pusher = Diagnostic_pusher.init in

  let errors = errors1 file1 in
  let errors1_absolute = errors1_absolute file1_absolute in
  let (pusher, _) =
    Diagnostic_pusher.push_new_errors ~phase pusher ~rechecked:files1 errors
  in
  let pushed_messages = TestClientProvider.get_push_messages () in
  (* There is no client, so no errors should have been pushed. *)
  assert_equal_messages [] pushed_messages;

  (* Connect a persistent client. *)
  connect_persistent ();

  (* Add an new batch of new errors. *)
  let errors = errors2 file2 in
  let errors2_absolute = errors2_absolute file2_absolute in
  let (pusher, _) =
    Diagnostic_pusher.push_new_errors
      ~phase
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

  let (_ : Diagnostic_pusher.t) = pusher in
  ()

let test_disconnects _ =
  let phase = Errors.Typing in
  let pusher = Diagnostic_pusher.init in

  let errors = Errors.from_file_error_list ~phase [(file1, error1)] in
  let errors1_absolute = errors1_absolute file1_absolute in
  let (pusher, _) =
    Diagnostic_pusher.push_new_errors ~phase pusher ~rechecked:files1 errors
  in
  let pushed_messages = TestClientProvider.get_push_messages () in
  (* There is no client, so no errors should have been pushed. *)
  assert_equal_messages [] pushed_messages;

  (* Connect a persistent client. *)
  connect_persistent ();

  (* This time error should be pushed *)
  let (pusher, _) = Diagnostic_pusher.push_whats_left pusher in
  let pushed_messages = TestClientProvider.get_push_messages () in
  let expected_pushed_messages =
    [
      ServerCommandTypes.DIAGNOSTIC
        { errors = errors1_absolute; is_truncated = None };
    ]
  in
  assert_equal_messages expected_pushed_messages pushed_messages;

  (* Client reconnect *)
  Ide_info_store.ide_disconnect ();
  connect_persistent ();

  (* Errors should be repushed *)
  let (pusher, _) = Diagnostic_pusher.push_whats_left pusher in
  let pushed_messages = TestClientProvider.get_push_messages () in
  let expected_pushed_messages =
    [
      ServerCommandTypes.DIAGNOSTIC
        { errors = errors1_absolute; is_truncated = None };
    ]
  in
  assert_equal_messages expected_pushed_messages pushed_messages;

  (* Client disconnect *)
  Ide_info_store.ide_disconnect ();

  (* No errors pushed *)
  let (pusher, _) = Diagnostic_pusher.push_whats_left pusher in
  let pushed_messages = TestClientProvider.get_push_messages () in
  let expected_pushed_messages = [] in
  assert_equal_messages expected_pushed_messages pushed_messages;

  (* Client reconnect *)
  connect_persistent ();

  (* Errors should be repushed *)
  let (pusher, _) = Diagnostic_pusher.push_whats_left pusher in
  let pushed_messages = TestClientProvider.get_push_messages () in
  let expected_pushed_messages =
    [
      ServerCommandTypes.DIAGNOSTIC
        { errors = errors1_absolute; is_truncated = None };
    ]
  in
  assert_equal_messages expected_pushed_messages pushed_messages;

  let (_ : Diagnostic_pusher.t) = pusher in
  ()

let test_switch_client _ =
  let phase = Errors.Typing in
  let pusher = Diagnostic_pusher.init in

  connect_persistent ();

  let errors = errors1 file1 in
  let errors1_absolute = errors1_absolute file1_absolute in
  let (pusher, _) =
    Diagnostic_pusher.push_new_errors ~phase pusher ~rechecked:files1 errors
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
  connect_persistent ();

  (* Add an new batch of new errors. *)
  let errors = errors2 file2 in
  let errors2_absolute = errors2_absolute file2_absolute in
  let _pusher =
    Diagnostic_pusher.push_new_errors ~phase pusher ~rechecked:files2 errors
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

  let (_ : Diagnostic_pusher.t) = pusher in
  ()

let test_switch_client_erase _ =
  let phase = Errors.Typing in
  let pusher = Diagnostic_pusher.init in

  connect_persistent ();

  let errors = errors1 file1 in
  let errors1_absolute = errors1_absolute file1_absolute in
  let (pusher, _) =
    Diagnostic_pusher.push_new_errors ~phase pusher ~rechecked:files1 errors
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
  connect_persistent ();

  (* No more errors in file1. *)
  let _pusher =
    Diagnostic_pusher.push_new_errors ~phase pusher ~rechecked:files1 no_errors
  in
  let pushed_messages = TestClientProvider.get_push_messages () in
  (* There was no need to erase any message. *)
  assert_equal_messages [] pushed_messages;

  let (_ : Diagnostic_pusher.t) = pusher in
  ()

let test_error_limit_one_file _ =
  let phase = Errors.Typing in
  with_error_limit 1 @@ fun () ->
  let pusher = Diagnostic_pusher.init in
  connect_persistent ();

  (* Add 2 errors in the same file file1: nothing should be pushed. *)
  let (pusher, _) =
    Diagnostic_pusher.push_new_errors
      ~phase
      pusher
      ~rechecked:files1
      (errors12 file1)
  in
  let pushed_messages = TestClientProvider.get_push_messages () in
  assert_equal_messages [] pushed_messages;

  (* Now file1 only has 1 error: it should be pushed. *)
  let (pusher, _) =
    Diagnostic_pusher.push_new_errors
      ~phase
      pusher
      ~rechecked:files1
      (errors1 file1)
  in
  let pushed_messages = TestClientProvider.get_push_messages () in
  let expected_pushed_messages =
    [
      ServerCommandTypes.DIAGNOSTIC
        {
          errors = SMap.of_list [(file1_absolute, [error1_absolute])];
          is_truncated = None;
        };
    ]
  in
  assert_equal_messages expected_pushed_messages pushed_messages;

  (* Now file1 has 2 errors again: the previous error should be erased
   * because for a given file, we want either all errors or none. *)
  let (pusher, _) =
    Diagnostic_pusher.push_new_errors
      ~phase
      pusher
      ~rechecked:files1
      (errors12 file1)
  in
  let pushed_messages = TestClientProvider.get_push_messages () in
  let expected_pushed_messages =
    [
      ServerCommandTypes.DIAGNOSTIC
        { errors = SMap.of_list [(file1_absolute, [])]; is_truncated = None };
    ]
  in
  assert_equal_messages expected_pushed_messages pushed_messages;

  let (_ : Diagnostic_pusher.t) = pusher in
  ()

let test_error_limit_two_files _ =
  let phase = Errors.Typing in
  with_error_limit 2 @@ fun () ->
  let pusher = Diagnostic_pusher.init in
  connect_persistent ();

  (* Add 2 errors in file1: they should be pushed *)
  let (pusher, _) =
    Diagnostic_pusher.push_new_errors
      ~phase
      pusher
      ~rechecked:files1
      (errors12 file1)
  in
  let pushed_messages = TestClientProvider.get_push_messages () in
  let expected_pushed_messages =
    [
      ServerCommandTypes.DIAGNOSTIC
        {
          errors =
            SMap.of_list [(file1_absolute, [error1_absolute; error2_absolute])];
          is_truncated = None;
        };
    ]
  in
  assert_equal_messages expected_pushed_messages pushed_messages;

  (* Add 1 error in file2: it should not be pushed *)
  let (pusher, _) =
    Diagnostic_pusher.push_new_errors
      ~phase
      pusher
      ~rechecked:files2
      (errors2 file2)
  in
  let pushed_messages = TestClientProvider.get_push_messages () in
  assert_equal_messages [] pushed_messages;

  (* Now file1 only has 1 error: we should push that error and the error in file2. *)
  let (pusher, _) =
    Diagnostic_pusher.push_new_errors
      ~phase
      pusher
      ~rechecked:files1
      (errors1 file1)
  in
  let pushed_messages = TestClientProvider.get_push_messages () in
  let expected_pushed_messages =
    [
      ServerCommandTypes.DIAGNOSTIC
        {
          errors =
            SMap.of_list
              [
                (file1_absolute, [error1_absolute]);
                (file2_absolute, [error2_absolute]);
              ];
          is_truncated = None;
        };
    ]
  in
  assert_equal_messages expected_pushed_messages pushed_messages;

  let (_ : Diagnostic_pusher.t) = pusher in
  ()

let test_error_limit_two_files_priority _ =
  let phase = Errors.Typing in
  with_error_limit 2 @@ fun () ->
  let pusher = Diagnostic_pusher.init in
  connect_persistent ();

  (* Add 2 errors in file1: they should be pushed *)
  let (pusher, _) =
    Diagnostic_pusher.push_new_errors
      ~phase
      pusher
      ~rechecked:files1
      (errors12 file1)
  in
  let pushed_messages = TestClientProvider.get_push_messages () in
  let expected_pushed_messages =
    [
      ServerCommandTypes.DIAGNOSTIC
        {
          errors =
            SMap.of_list [(file1_absolute, [error1_absolute; error2_absolute])];
          is_truncated = None;
        };
    ]
  in
  assert_equal_messages expected_pushed_messages pushed_messages;

  Ide_info_store.open_file file2;
  (* Add 1 error in file2: it should be pushed even if it goes beyond the limit.
   * The limit will only be exceeded by the number of errors in open files, which should
   * be small relatively. *)
  let (pusher, _) =
    Diagnostic_pusher.push_new_errors
      ~phase
      pusher
      ~rechecked:files2
      (errors12 file2)
  in
  let pushed_messages = TestClientProvider.get_push_messages () in
  let expected_pushed_messages =
    [
      ServerCommandTypes.DIAGNOSTIC
        {
          errors =
            SMap.of_list [(file2_absolute, [error1_absolute; error2_absolute])];
          is_truncated = None;
        };
    ]
  in
  assert_equal_messages expected_pushed_messages pushed_messages;

  (* Now file1 only has 1 error: we can't push that error because the error count would be beyond
   * the limit, so we erase errors for file 1. *)
  let (pusher, _) =
    Diagnostic_pusher.push_new_errors
      ~phase
      pusher
      ~rechecked:files1
      (errors1 file1)
  in
  let pushed_messages = TestClientProvider.get_push_messages () in
  let expected_pushed_messages =
    [
      ServerCommandTypes.DIAGNOSTIC
        { errors = SMap.of_list [(file1_absolute, [])]; is_truncated = None };
    ]
  in
  assert_equal_messages expected_pushed_messages pushed_messages;

  let (_ : Diagnostic_pusher.t) = pusher in
  ()

let test_multiple_phases _ =
  let pusher = Diagnostic_pusher.init in
  connect_persistent ();

  (* Typechecking phase. One error in file 1. *)
  let phase = Errors.Typing in
  let (pusher, _) =
    Diagnostic_pusher.push_new_errors
      pusher
      ~phase
      ~rechecked:files1
      (Errors.from_file_error_list ~phase [(file1, error1)])
  in
  let pushed_messages = TestClientProvider.get_push_messages () in
  let expected_pushed_messages =
    [
      ServerCommandTypes.DIAGNOSTIC
        {
          errors = SMap.of_list [(file1_absolute, [error1_absolute])];
          is_truncated = None;
        };
    ]
  in
  assert_equal_messages expected_pushed_messages pushed_messages;

  (* Naming phase. One additional naming error in file 1.
   * Errors from previous phases should be kept around. *)
  let phase = Errors.Naming in
  let (pusher, _) =
    Diagnostic_pusher.push_new_errors
      pusher
      ~phase
      ~rechecked:files1
      (Errors.from_file_error_list ~phase [(file1, error2)])
  in
  let pushed_messages = TestClientProvider.get_push_messages () in
  let expected_pushed_messages =
    [
      ServerCommandTypes.DIAGNOSTIC
        {
          errors =
            SMap.of_list [(file1_absolute, [error1_absolute; error2_absolute])];
          is_truncated = None;
        };
    ]
  in
  assert_equal_messages expected_pushed_messages pushed_messages;

  (* Typing phase. One additional typing error in file 1.
   * Errors from previous Naming phase should be kept around. *)
  let phase = Errors.Typing in
  let (pusher, _) =
    Diagnostic_pusher.push_new_errors
      pusher
      ~phase
      ~rechecked:files1
      (Errors.from_file_error_list ~phase [(file1, error3)])
  in
  let pushed_messages = TestClientProvider.get_push_messages () in
  let expected_pushed_messages =
    [
      ServerCommandTypes.DIAGNOSTIC
        {
          errors =
            SMap.of_list [(file1_absolute, [error2_absolute; error3_absolute])];
          is_truncated = None;
        };
    ]
  in
  assert_equal_messages expected_pushed_messages pushed_messages;

  (* Back to Naming phase. Error2 is fixed, and errors from other phases should be preserved. *)
  let phase = Errors.Naming in
  let (pusher, _) =
    Diagnostic_pusher.push_new_errors
      pusher
      ~phase
      ~rechecked:files1
      Errors.empty
  in
  let pushed_messages = TestClientProvider.get_push_messages () in
  let expected_pushed_messages =
    [
      ServerCommandTypes.DIAGNOSTIC
        {
          errors = SMap.of_list [(file1_absolute, [error3_absolute])];
          is_truncated = None;
        };
    ]
  in
  assert_equal_messages expected_pushed_messages pushed_messages;

  let (_ : Diagnostic_pusher.t) = pusher in
  ()

let test_multiple_phases_common _ =
  let pusher = Diagnostic_pusher.init in
  connect_persistent ();

  (* Typing phase. No errors *)
  let phase = Errors.Typing in
  let (pusher, _) =
    Diagnostic_pusher.push_new_errors
      pusher
      ~phase
      ~rechecked:files1
      Errors.empty
  in
  let pushed_messages = TestClientProvider.get_push_messages () in
  let expected_pushed_messages = [] in
  assert_equal_messages expected_pushed_messages pushed_messages;

  (* Naming phase. Still no errors. *)
  let phase = Errors.Naming in
  let (pusher, _) =
    Diagnostic_pusher.push_new_errors
      pusher
      ~phase
      ~rechecked:files1
      Errors.empty
  in
  let pushed_messages = TestClientProvider.get_push_messages () in
  let expected_pushed_messages = [] in
  assert_equal_messages expected_pushed_messages pushed_messages;

  (* Typing phase. One typing error in file 1.
   * Errors from previous phases should be kept around. *)
  let phase = Errors.Typing in
  let (pusher, _) =
    Diagnostic_pusher.push_new_errors
      pusher
      ~phase
      ~rechecked:files1
      (Errors.from_file_error_list ~phase [(file1, error3)])
  in
  let pushed_messages = TestClientProvider.get_push_messages () in
  let expected_pushed_messages =
    [
      ServerCommandTypes.DIAGNOSTIC
        {
          errors = SMap.of_list [(file1_absolute, [error3_absolute])];
          is_truncated = None;
        };
    ]
  in
  assert_equal_messages expected_pushed_messages pushed_messages;

  (* Back to Typing phase. Typing error should be removed. *)
  let phase = Errors.Typing in
  let (pusher, _) =
    Diagnostic_pusher.push_new_errors
      pusher
      ~phase
      ~rechecked:files1
      Errors.empty
  in
  let pushed_messages = TestClientProvider.get_push_messages () in
  let expected_pushed_messages =
    [
      ServerCommandTypes.DIAGNOSTIC
        { errors = SMap.of_list [(file1_absolute, [])]; is_truncated = None };
    ]
  in
  assert_equal_messages expected_pushed_messages pushed_messages;

  let (_ : Diagnostic_pusher.t) = pusher in
  ()

let tear_down () =
  Ide_info_store.ide_disconnect ();
  ()

let with_tear_down f ctx =
  f ctx;
  tear_down ();
  ()

let () =
  "diagnostic_pusher_test"
  >::: [
         "ErrorTrackerTest.test_never_commit"
         >:: with_tear_down ErrorTrackerTest.test_never_commit;
         "ErrorTrackerTest.test_push_commit_erase"
         >:: with_tear_down ErrorTrackerTest.test_push_commit_erase;
         "test_push" >:: with_tear_down test_push;
         "test_initially_no_client" >:: with_tear_down test_initially_no_client;
         "test_disconnects" >:: with_tear_down test_disconnects;
         "test_switch_client" >:: with_tear_down test_switch_client;
         "test_switch_client_erase" >:: with_tear_down test_switch_client_erase;
         "test_error_limit_one_file"
         >:: with_tear_down test_error_limit_one_file;
         "test_error_limit_two_files"
         >:: with_tear_down test_error_limit_two_files;
         "test_error_limit_two_files_priority"
         >:: with_tear_down test_error_limit_two_files_priority;
         "test_multiple_phases" >:: with_tear_down test_multiple_phases;
         "test_multiple_phases_common"
         >:: with_tear_down test_multiple_phases_common;
       ]
  |> run_test_tt_main
