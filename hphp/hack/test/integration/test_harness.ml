open Hh_prelude

module Tools = struct
  let boxed_string content =
    Printf.sprintf
      "%s%s%s"
      "\n============================\n"
      content
      "\n============================\n"
end

module Tempfile = struct
  let mkdtemp () =
    let tmp_dir = Sys_utils.temp_dir_name in
    let tmp_dir = Path.make tmp_dir in
    let name = Random_id.short_string () in
    let tmp_dir = Path.concat tmp_dir name in
    let () = Unix.mkdir (Path.to_string tmp_dir) 0o740 in
    tmp_dir
end

exception Process_failed of Process_types.failure

type t = {
  repo_dir: Path.t;
  test_env: string list;
  hh_client_path: string;
  hh_server_path: string;
}

type config = {
  hh_server: Path.t;
  hh_client: Path.t;
  template_repo: Path.t;
}

(** Invoke a subprocess on the harness's repo with its environment. *)
let exec_hh_client args harness =
  let args =
    args @ [Path.to_string harness.repo_dir; "--config"; "max_workers=2"]
  in
  Printf.eprintf
    "executing hh_client. Args: %s\n%!"
    (String.concat ~sep:", " args);
  Process.exec
    (Exec_command.For_use_in_testing_only harness.hh_client_path)
    ~env:(Process_types.Augment harness.test_env)
    args

let get_server_logs harness =
  let process = exec_hh_client ["--logname"] harness in
  match Process.read_and_wait_pid ~timeout:5 process with
  | Ok { Process_types.stdout; _ } ->
    let log_path = Path.make (String.strip stdout) in
    (try Some (Sys_utils.cat (Path.to_string log_path)) with
    | Sys_error m when Sys_utils.string_contains m "No such file or directory"
      ->
      None)
  | Error failure -> raise @@ Process_failed failure

let wait_until_lock_free lock_file _harness =
  Lock.blocking_grab_then_release lock_file

let get_recording_path harness =
  let recording_re =
    Str.regexp
      ("^.+ About to spawn recorder daemon\\. "
      ^ "Output will go to \\(.+\\)\\. Logs to \\(.+\\)\\. "
      ^ "Lock_file to \\(.+\\)\\.$")
  in
  Option.(
    let logs = get_server_logs harness in
    logs >>= fun logs ->
    try
      let _ = Str.search_forward recording_re logs 0 in
      Some
        ( Path.make (Str.matched_group 1 logs),
          Path.make (Str.matched_group 2 logs) )
    with
    | Stdlib.Not_found ->
      Printf.eprintf "recorder path or lock file not found\n%!";
      Printf.eprintf "See also server logs: %s\n%!" logs;
      None)

let check_cmd harness =
  let process = exec_hh_client ["check"] harness in
  Printf.eprintf "Waiting for process\n%!";
  match Process.read_and_wait_pid ~timeout:30 process with
  | Ok { Process_types.stdout; _ } -> Sys_utils.split_lines stdout
  | Error failure -> raise @@ Process_failed failure

let stop_server harness =
  let process = exec_hh_client ["stop"] harness in
  match Process.read_and_wait_pid ~timeout:30 process with
  | Ok { Process_types.stdout; _ } -> stdout
  | Error failure -> raise @@ Process_failed failure

let run_test ?(stop_server_in_teardown = true) config test_case =
  let base_tmp_dir = Tempfile.mkdtemp () in
  let repo_dir = Path.concat base_tmp_dir "repo" in
  let () = Unix.mkdir (Path.to_string repo_dir) 0o740 in
  let command =
    Printf.sprintf
      "cp -r %s/. %s"
      (Path.to_string config.template_repo)
      (Path.to_string repo_dir)
  in
  let () = Printf.eprintf "Executing command: %s\n" command in
  let retcode = Sys.command command in
  if not (retcode = 0) then
    let () = Printf.eprintf "Failed to copy template repo\n" in
    false
  else
    (* Where the hhi files, socket, etc get extracted *)
    (* TODO: Get HH_TMPDIR working; currently this is commented out.
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
    (* let hh_tmpdir = Tempfile.mkdtemp () in *)
    let bin_dir = Tempfile.mkdtemp () in
    let hh_server_dir = Path.parent config.hh_server in
    let test_env =
      [
        ("HH_TEST_MODE", "1");
        (* ("HH_TMPDIR", (Path.to_string hh_tmpdir)); *)
        ( "PATH",
          Printf.sprintf
            "'%s:%s:/bin:/usr/bin:/usr/local/bin"
            (Path.to_string hh_server_dir)
            (Path.to_string bin_dir) );
        ("OCAMLRUNPARAM", "b");
        ("HH_LOCALCONF_PATH", Path.to_string repo_dir);
      ]
    in
    let test_env =
      List.map test_env ~f:(fun (k, v) -> Printf.sprintf "%s=%s" k v)
    in
    let harness =
      {
        repo_dir;
        test_env;
        hh_client_path = Path.to_string config.hh_client;
        hh_server_path = Path.to_string config.hh_server;
      }
    in
    let () =
      Printf.eprintf
        "Repo_dir: %s; bin_dir: %s;\n%!"
        (Path.to_string repo_dir)
        (Path.to_string bin_dir)
    in
    let tear_down () =
      let () =
        Printf.eprintf "Tearing down test, deleting temp directories\n%!"
      in
      let () =
        if stop_server_in_teardown then
          ignore @@ stop_server harness
        else
          ()
      in
      let () = Sys_utils.rm_dir_tree (Path.to_string bin_dir) in
      let () = Sys_utils.rm_dir_tree (Path.to_string repo_dir) in
      let () = Sys_utils.rm_dir_tree (Path.to_string base_tmp_dir) in
      ()
    in
    let with_exception_printing test_case harness =
      let result =
        try test_case harness with
        | Process_failed (Process_types.Abnormal_exit { status; stderr; _ }) as
          e ->
          Printf.eprintf
            "Process exited abnormally (%s). See also Stderr: %s\n"
            (Process.status_to_string status)
            (Tools.boxed_string stderr);
          raise e
      in
      result
    in
    Utils.try_finally
      ~f:(fun () -> with_exception_printing test_case harness)
      ~finally:tear_down

let with_local_conf local_conf_str test_case harness =
  let conf_file = Path.concat harness.repo_dir "hh.conf" in
  let oc = Stdlib.open_out (Path.to_string conf_file) in
  let () = Stdlib.output_string oc local_conf_str in
  let () = Stdlib.close_out oc in
  test_case harness
