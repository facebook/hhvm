open Core

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

end


module Test_harness = struct

  type t = {
    repo_dir : Path.t;
    test_env : string list;
    hh_client_path : string;
    hh_server_path : string
  }

  (** Invoke a subprocess on the harness's repo with its environment. *)
  let exec_hh_client args harness =
    Process.exec harness.hh_client_path
      ~env:harness.test_env (args @ [Path.to_string harness.repo_dir])

  let get_server_logs harness =
    let process = exec_hh_client ["--logname"] harness in
    let log_path = Process.read_and_close_pid process in
    Sys_utils.cat (String.trim log_path)

  let check_cmd harness =
    let process = exec_hh_client [] harness in
    let result = Process.read_and_close_pid process in
    Sys_utils.split_lines result

  let stop_server harness =
    let process = exec_hh_client ["stop"] harness in
    Process.read_and_close_pid process

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
      let hh_tmp_dir = Tempfile.mkdtemp () in
      let bin_dir = Tempfile.mkdtemp () in
      let hh_server_dir = Path.parent args.Args.hh_server in
      let test_env = [
        "HH_TEST_MODE=1";
        "HH_TMPDIR=" ^ (Path.to_string hh_tmp_dir);
        "PATH=" ^ (Printf.sprintf "'%s:%s:/bin:/usr/bin:/usr/local/bin"
          (Path.to_string hh_server_dir) (Path.to_string bin_dir));
        "OCAMLRUNPARAM=b";
        "HH_LOCALCONF_PATH=" ^ (Path.to_string repo_dir);
      ] in
      let harness = {
        repo_dir;
        test_env;
        hh_client_path = Path.to_string args.Args.hh_client;
        hh_server_path = Path.to_string args.Args.hh_server;
      } in
      let () = Printf.eprintf "Repo_dir: %s; bin_dir: %s; hh_tmp_dir: %s;"
        (Path.to_string repo_dir)
        (Path.to_string bin_dir)
        (Path.to_string hh_tmp_dir) in
      let tear_down () =
        let _ = stop_server harness in
        let () = Sys_utils.rm_dir_tree (Path.to_string bin_dir) in
        let () = Sys_utils.rm_dir_tree (Path.to_string hh_tmp_dir) in
        let () = Sys_utils.rm_dir_tree (Path.to_string base_tmp_dir) in
        let () = Sys_utils.rm_dir_tree (Path.to_string repo_dir) in
        ()
      in
      Utils.try_finally ~f:(fun () -> test_case harness) ~finally:tear_down

  let with_local_conf local_conf_str test_case harness =
    let conf_file = Path.concat harness.repo_dir "hh.conf" in
    let oc = Pervasives.open_out (Path.to_string conf_file) in
    let () = Pervasives.output_string oc local_conf_str in
    let () = Pervasives.close_out oc in
    test_case harness

  let assert_equals exp actual harness =
    if exp = actual then
      ()
    else
      let () = Printf.eprintf "Expected: %s; But Found: %s\n" exp actual in
      let logs = get_server_logs harness in
      let () = Printf.eprintf "See also server logs:\n%s\n" logs in
      assert false

  let assert_list_equals exp actual harness =
    if (List.length exp) = (List.length actual) then
      List.iter2_exn exp actual ~f:(fun exp actual ->
        assert_equals exp actual harness)
    else
      let () = Printf.eprintf
        "assert_list_equals failed. Counts not equal\n" in
      let () = Printf.eprintf
        "Expected:\n%s\n\n But Found:\n%s\n"
        (String.concat "\n" exp) (String.concat "\n" actual) in
      let logs = get_server_logs harness in
      let () = Printf.eprintf "See also server logs:\n%s\n" logs in
      assert false

end

let test_check_cmd harness =
  let results = Test_harness.check_cmd harness in
  let () = Test_harness.assert_list_equals ["No errors!"] results harness in
  let _ = Test_harness.stop_server harness in
  true

let tests args = [
  "test_check_cmd", fun () -> Test_harness.run_test args (
    Test_harness.with_local_conf
    "use_mini_state = true\nuse_watchman = true\n"
    test_check_cmd);
]

let () =
  let args = Args.parse () in
  Unit_test.run_all (tests args)
