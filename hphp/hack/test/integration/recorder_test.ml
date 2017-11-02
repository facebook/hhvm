open Hh_core
open Asserter


module Args = Test_harness_common_args


let see_also_logs harness =
  let logs = Test_harness.get_server_logs harness in
  let logs = Option.value logs ~default:"" in
  Printf.sprintf "See also server logs:%s"
  (Test_harness.Tools.boxed_string logs)

let test_check_cmd harness =
  let results = Test_harness.check_cmd harness in
  let () = String_asserter.assert_list_equals ["No errors!"] results
    (see_also_logs harness) in
  let _ = Test_harness.stop_server harness in
  true

let rec read_recording_rec ic acc =
  try begin
    let item = Marshal.from_channel ic in
    read_recording_rec ic (item :: acc)
  end with
  | End_of_file ->
    acc
  | Failure s when s = "input_value: truncated object" ->
    acc

let read_recording recording_path =
  let ic = Pervasives.open_in (Path.to_string recording_path) in
  let _ = Recorder.Header.parse_header ic in
  Utils.try_finally ~f:(fun () ->
    let items_rev = read_recording_rec ic [] in
    List.rev items_rev
  ) ~finally:(fun () -> Pervasives.close_in ic)

let assert_recording_matches expected actual harness =
  let actual = List.map actual ~f:Recorder_types.to_string in
  String_asserter.assert_list_equals expected actual
    (see_also_logs harness)

let test_server_driver_case_0 harness =
  let results = Test_harness.check_cmd harness in
  let () = String_asserter.assert_list_equals ["No errors!"] results
    (see_also_logs harness) in
  let () = Printf.eprintf "Finished check on: %s\n%!"
    (Path.to_string harness.Test_harness.repo_dir) in
  let _ = Server_driver.connect_and_run_case 0 harness.Test_harness.repo_dir in
  let results = Test_harness.check_cmd harness in
  let () = String_asserter.assert_list_equals ["No errors!"] results
    (see_also_logs harness) in
  let _ = Test_harness.stop_server harness in
  match Test_harness.get_recording_path harness with
  | None ->
    Printf.eprintf "Could not find recording\n%!";
    false
  | Some (recording_path, recorder_lock_file) ->
    let () = Test_harness.wait_until_lock_free
      (Path.to_string recorder_lock_file) harness in
    let recording = read_recording recording_path in
    let expected = [
      "Start_recording";
      "(HandleServerCommand STATUS)";
      "(HandleServerCommand STATUS)";
      "(HandleServerCommand INFER_TYPE)";
      "(HandleServerCommand STATUS)";
    ] in
    let () = assert_recording_matches expected recording harness in
    true

let test_server_driver_case_1_and_turntable harness =
  let results = Test_harness.check_cmd harness in
  let () = String_asserter.assert_list_equals ["No errors!"] results
    (see_also_logs harness) in
  let () = Printf.eprintf "Finished check on: %s\n%!"
    (Path.to_string harness.Test_harness.repo_dir) in
  Printf.eprintf "Playing server driver case 1\n";
  let _conn = Server_driver.connect_and_run_case 1
    harness.Test_harness.repo_dir in
  Printf.eprintf "Finished Playing server driver case 1\n";
  let results = Test_harness.check_cmd harness in
  let substitutions = (module struct
    let substitutions = [
      ("repo", (Path.to_string harness.Test_harness.repo_dir));
    ]
  end : Pattern_substitutions) in
  let expected_error = "{repo}/foo_1.php:3:26,37: " ^
    "Undefined variable: $missing_var (Naming[2050])" in
  let module MySubstitutions = (val substitutions : Pattern_substitutions) in
  let module MyPatternComparator = Pattern_comparator (MySubstitutions) in
  let module MyAsserter = Make_asserter (MyPatternComparator) in
  let () = MyAsserter.assert_list_equals [expected_error] results
    (see_also_logs harness) in
  match Test_harness.get_recording_path harness with
  | None ->
    Printf.eprintf "Could not find recording\n%!";
    false
  | Some (recording_path, recorder_lock_file) ->
    let _ = Test_harness.stop_server harness in
    let () = Test_harness.wait_until_lock_free
      (Path.to_string recorder_lock_file) harness in
    let results = Test_harness.check_cmd harness in
    (** Fresh server has no errors. *)
    let () = String_asserter.assert_list_equals ["No errors!"] results
      (see_also_logs harness) in
    let () = Turntable.spin_record true recording_path
      harness.Test_harness.repo_dir in
    (** After playback, we get the previous errors again. *)
    let results = Test_harness.check_cmd harness in
    let () = MyAsserter.assert_list_equals [expected_error] results
      (see_also_logs harness) in
    true

let tests args =
  let harness_config = {
    Test_harness.hh_server = args.Args.hh_server;
    hh_client = args.Args.hh_client;
    template_repo = args.Args.template_repo;
  } in
  [
  ("test_check_cmd", fun () -> Test_harness.run_test harness_config (
    Test_harness.with_local_conf
    "use_mini_state = true\nuse_watchman = true\n"
    test_check_cmd));
  ("test_server_driver_case_0", fun () -> Test_harness.run_test harness_config (
    Test_harness.with_local_conf
      ("use_mini_state = false\n" ^
      "use_watchman = true\n" ^
      "start_with_recorder_on = true\n")
    test_server_driver_case_0));
  ("test_server_driver_case_1_and_turntable",
    fun () -> Test_harness.run_test harness_config (
    Test_harness.with_local_conf
      ("use_mini_state = false\n" ^
      "use_watchman = true\n" ^
      "start_with_recorder_on = true\n")
    test_server_driver_case_1_and_turntable));
  ]

let () =
  let args = Args.parse () in
  let () = HackEventLogger.client_init (args.Args.template_repo) in
  Unit_test.run_all (tests args)
