open Core
open Asserter

(** The exit status and stderr from the process. *)
exception Process_exited_abnormally of Unix.process_status * string

module Args = struct

  type t = {
    hh_server : Path.t;
    hh_client : Path.t;
    template_repo : Path.t;
  }

  let usage = Printf.sprintf
    "Usage: %s --hh-server <%s> --hh-client <%s> [template repo]\n"
    "hh_server_path" "hh_client_path" Sys.argv.(0)
  let usage_exit () =
    Printf.eprintf "%s\n" usage;
    exit 1

  let string_spec str_ref =
    Arg.String (fun x -> str_ref := Some x)

  let requires name opt = match !opt with
    | None ->
      let () = Printf.eprintf "Missing required argument: %s\n" name in
      usage_exit ()
    | Some x -> x

  let parse () =
    let template_repo = ref None in
    let hh_server = ref None in
    let hh_client = ref None in
    let options = [
      "--hh-server", string_spec hh_server, "Path to hh_server";
      "--hh-client", string_spec hh_client, "Path to hh_client";
    ] in
    let () = Arg.parse options (fun s -> template_repo := (Some s)) usage in
    let template_repo = requires "template repo" template_repo in
    let hh_server = requires "hh_server" hh_server in
    let hh_client = requires "hh_client" hh_client in
    {
      hh_server = Path.make hh_server;
      hh_client = Path.make hh_client;
      template_repo = Path.make template_repo;
    }

end;;


module Tempfile = struct
  let mkdtemp () =
    let tmp_dir = Sys_utils.temp_dir_name in
    let tmp_dir = Path.make tmp_dir in
    let name = Random_id.(short_string_with_alphabet alphanumeric_alphabet) in
    let tmp_dir = Path.concat tmp_dir name in
    let () = Unix.mkdir (Path.to_string tmp_dir) 0o740 in
    tmp_dir

end;;


module Test_harness = struct

  type t = {
    repo_dir : Path.t;
    test_env : string list;
    hh_client_path : string;
    hh_server_path : string;
  }

  (** Invoke a subprocess on the harness's repo with its environment. *)
  let exec_hh_client args harness =
    Printf.eprintf "executing hh_client. Args: %s\n%!"
      (String.concat ", " args);
    Process.exec harness.hh_client_path
      ~env:harness.test_env (args @ [Path.to_string harness.repo_dir])

  let get_server_logs harness =
    let process = exec_hh_client ["--logname"] harness in
    let status, log_path, err = Process.read_and_wait_pid process in
    match status with
    | Unix.WEXITED 0 ->
      let log_path = Path.make (String.trim log_path) in
      (try Some (Sys_utils.cat (Path.to_string log_path)) with
      | Sys_error(m)
        when Sys_utils.string_contains m "No such file or directory" ->
        None)
    | _ ->
      raise (Process_exited_abnormally (status, err))

  let wait_until_lock_free lock_file _harness =
    Lock.blocking_grab_then_release lock_file

  let get_recording_path harness =
    let recording_re = Str.regexp
      ("^.+ About to spawn recorder daemon\\. " ^
      "Output will go to \\(.+\\)\\. Logs to \\(.+\\)\\. " ^
      "Lock_file to \\(.+\\)\\.$") in
    let open Option in
    let logs = get_server_logs harness in
    logs >>= fun logs ->
    try begin
      let _ = Str.search_forward recording_re logs 0 in
      Some (
        (Path.make (Str.matched_group 1 logs)),
        (Path.make (Str.matched_group 2 logs)))
    end with
    | Not_found ->
      Printf.eprintf "recorder path or lock file not found\n%!";
      Printf.eprintf "See also server logs: %s\n%!" logs;
      None

  let check_cmd harness =
    let process = exec_hh_client ["check"] harness in
    Printf.eprintf "Waiting for process\n%!";
    let status, result, err = Process.read_and_wait_pid process in
    match status with
    | Unix.WEXITED _ ->
      Sys_utils.split_lines result
    | _ ->
      raise (Process_exited_abnormally (status, err))

  let stop_server harness =
    let process = exec_hh_client ["stop"] harness in
    let status, result, err = Process.read_and_wait_pid process in
    match status with
    | Unix.WEXITED 0 ->
      result
    | _ ->
      Printf.eprintf "Stop server failed\n%!";
      raise (Process_exited_abnormally (status, err))

  let run_test args test_case =
    let base_tmp_dir = Tempfile.mkdtemp () in
    let repo_dir = Path.concat base_tmp_dir "repo" in
    let () = Unix.mkdir (Path.to_string repo_dir) 0o740 in
    let command = Printf.sprintf
      "cp -r %s/. %s" (Path.to_string args.Args.template_repo)
      (Path.to_string repo_dir) in
    let () = Printf.eprintf "Executing command: %s\n" command in
    let retcode = Sys.command command in
    if not (retcode = 0) then
      let () = Printf.eprintf "Failed to copy template repo\n" in
      false
    else

      (** Where the hhi files, socket, etc get extracted *)
      (** TODO: Get HH_TMPDIR working; currently this is commented out.
       *
       * Right now, we will have to pollute the system's main tmp directory
       * for HH_TMPDIR instead of using a custom one for testing.
       *
       * The problem is that we look for a server's socket file in
       * GlobalConfig.tmp_dir, which is a static constant. So, when we
       * start a server by forking hh_client (with a custom HH_TMPDIR env
       * variable), it puts the socket file in that custom directory. But
       * when we try to open a connection inside this existing process,
       * it looks for the socket file in the default directory.
       *
       * Globals suck. *)
      (** let hh_tmpdir = Tempfile.mkdtemp () in *)
      let bin_dir = Tempfile.mkdtemp () in
      let hh_server_dir = Path.parent args.Args.hh_server in
      let test_env = [
        ("HH_TEST_MODE","1");
        (** ("HH_TMPDIR", (Path.to_string hh_tmpdir)); *)
        ("PATH", (Printf.sprintf "'%s:%s:/bin:/usr/bin:/usr/local/bin"
          (Path.to_string hh_server_dir) (Path.to_string bin_dir)));
        ("OCAMLRUNPARAM","b");
        ("HH_LOCALCONF_PATH", Path.to_string repo_dir);
      ] in
      let test_env = List.map test_env ~f:(fun (k,v) ->
        Printf.sprintf "%s=%s" k v) in
      let harness = {
        repo_dir;
        test_env;
        hh_client_path = Path.to_string args.Args.hh_client;
        hh_server_path = Path.to_string args.Args.hh_server;
      } in
      let () = Printf.eprintf "Repo_dir: %s; bin_dir: %s;\n%!"
        (Path.to_string repo_dir)
        (Path.to_string bin_dir) in
      let tear_down () =
        let () = Printf.eprintf
          "Tearing down test, deleting temp directories\n%!" in
        let _ = stop_server harness in
        let () = Sys_utils.rm_dir_tree (Path.to_string bin_dir) in
        let () = Sys_utils.rm_dir_tree (Path.to_string repo_dir) in
        let () = Sys_utils.rm_dir_tree (Path.to_string base_tmp_dir) in
        ()
      in
      Utils.try_finally ~f:(fun () -> test_case harness) ~finally:tear_down

  let with_local_conf local_conf_str test_case harness =
    let conf_file = Path.concat harness.repo_dir "hh.conf" in
    let oc = Pervasives.open_out (Path.to_string conf_file) in
    let () = Pervasives.output_string oc local_conf_str in
    let () = Pervasives.close_out oc in
    test_case harness

end;;

let boxed_string content =
  Printf.sprintf "%s%s%s"
  "\n============================\n"
  content
  "\n============================\n"

let see_also_logs harness =
  let logs = Test_harness.get_server_logs harness in
  let logs = Option.value logs ~default:"" in
  Printf.sprintf "See also server logs:%s"
  (boxed_string logs)

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
  let _conn = Server_driver.connect_and_run_case 1
    harness.Test_harness.repo_dir in
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

let tests args = [
  ("test_check_cmd", fun () -> Test_harness.run_test args (
    Test_harness.with_local_conf
    "use_mini_state = true\nuse_watchman = true\n"
    test_check_cmd));
  ("test_server_driver_case_0", fun () -> Test_harness.run_test args (
    Test_harness.with_local_conf
      ("use_mini_state = false\n" ^
      "use_watchman = true\n" ^
      "start_with_recorder_on = true\n")
    test_server_driver_case_0));
  ("test_server_driver_case_1_and_turntable",
    fun () -> Test_harness.run_test args (
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
