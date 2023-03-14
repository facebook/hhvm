(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Hh_prelude
open Asserter

let a_php = ("a.php", "<?hh\nfunction test_a(): void {}\n")

let loop_php =
  ("loop.php", "<?hh\nfunction test_loop(): void {hh_loop_forever();}")

(** Shell out to HH_CLIENT_PATH. Must have HH_SERVER_PATH defined too. *)
let hh ~(root : Path.t) (args : string array) : string Lwt.t =
  let hh_client_path = Sys.getenv "HH_CLIENT_PATH" in
  let _ = Sys.getenv "HH_SERVER_PATH" in
  (* just to assert that it's defined *)
  Sys.chdir (Path.to_string root);
  let%lwt result =
    Lwt_utils.exec_checked
      (Exec_command.For_use_in_testing_only hh_client_path)
      ~timeout:30.0
      args
  in
  match result with
  | Error e -> Lwt.return (Lwt_utils.Process_failure.to_string e)
  | Ok { Lwt_utils.Process_success.stdout; _ } -> Lwt.return stdout

(** This creates a small repo, ready to run hh in it.
(there'll be no watchman nor informant support). *)
let try_with_server
    (files : (string * string) list) (f : root:Path.t -> unit Lwt.t) :
    unit Lwt.t =
  let root = Tempfile.mkdtemp ~skip_mocking:true in
  Lwt_utils.try_finally
    ~f:(fun () ->
      Sys_utils.write_file
        ~file:(Path.concat root ".hhconfig" |> Path.to_string)
        "";
      (* we'll put a dummy .hg directory in there to avoid the possibility of hg blowups *)
      Sys_utils.mkdir_p
        ~skip_mocking:true
        (Path.concat root ".hg" |> Path.to_string);
      List.iter files ~f:(fun (name, content) ->
          Sys_utils.write_file
            ~file:(Path.concat root name |> Path.to_string)
            content);
      ServerProgress.set_root root;
      let%lwt () = f ~root in
      Lwt.return_unit)
    ~finally:(fun () ->
      let%lwt () =
        try%lwt
          let%lwt _ = hh ~root [| "stop" |] in
          Lwt.return_unit
        with
        | _ -> Lwt.return_unit
      in
      Sys_utils.rm_dir_tree ~skip_mocking:true (Path.to_string root);
      Lwt.return_unit)

(** Looks for substring 'expected' in the ServerProgress message;
will keep polling the ServerProgress every 0.1s up to 'deadline' until it finds it,
and will raise an exception if it doesn't. *)
let rec wait_for_progress ~(deadline : float) ~(expected : string) : unit Lwt.t
    =
  let { ServerProgress.message; _ } = ServerProgress.read () in
  if String.is_substring message ~substring:expected then
    Lwt.return_unit
  else if Float.(Unix.gettimeofday () > deadline) then
    failwith
      (Printf.sprintf
         "Timeout waiting for status '%s'; got '%s'"
         expected
         message)
  else
    let%lwt () = Lwt_unix.sleep 0.1 in
    wait_for_progress ~deadline ~expected

(** Scrapes the monitor log for '...server...pid: 012345' and returns it.
Alas, this is the only means we have to find PIDs. *)
let get_server_pid ~(root : Path.t) : int Lwt.t =
  let%lwt result = Lwt_utils.read_all (ServerFiles.monitor_log_link root) in
  let monitor_log = Result.ok_or_failwith result in
  let re = Str.regexp {|^.*server.*pid: \([0-9]+\).?$|} in
  let _pos = Str.search_forward re monitor_log 0 in
  let server_pid = Str.matched_group 1 monitor_log |> Int.of_string in
  Lwt.return server_pid

let test_start_stop () : bool Lwt.t =
  let%lwt () =
    try_with_server [a_php] (fun ~root ->
        let%lwt stdout =
          hh ~root [| "start"; "--no-load"; "--config"; "max_workers=1" |]
        in
        String_asserter.assert_equals "" stdout "hh start doesn't give output";
        (* status should eventually reach "ready" *)
        let%lwt () =
          wait_for_progress
            ~deadline:(Unix.gettimeofday () +. 60.0)
            ~expected:"ready"
        in
        let%lwt _ = hh ~root [| "stop" |] in
        (* The progress file should be cleanly deleted *)
        if Sys_utils.file_exists (ServerFiles.server_progress_file root) then
          failwith "expected progress file to be deleted";
        let%lwt () =
          wait_for_progress ~deadline:0. ~expected:"unknown hh_server state"
        in
        Lwt.return_unit)
  in
  Lwt.return_true

let test_typechecking () : bool Lwt.t =
  let%lwt () =
    try_with_server [a_php; loop_php] (fun ~root ->
        let%lwt stdout =
          hh ~root [| "start"; "--no-load"; "--config"; "max_workers=1" |]
        in
        String_asserter.assert_equals "" stdout "hh start doesn't give output";
        (* the file loop_php causes the typechecker to spin, typechecking, for 10mins! *)
        let%lwt () =
          wait_for_progress
            ~deadline:(Unix.gettimeofday () +. 60.0)
            ~expected:"typechecking"
        in
        Lwt.return_unit)
  in
  Lwt.return_true

let test_kill_server () : bool Lwt.t =
  let%lwt () =
    try_with_server [a_php] (fun ~root ->
        let%lwt stdout =
          hh ~root [| "--no-load"; "--config"; "max_workers=1" |]
        in
        String_asserter.assert_equals "No errors!\n" stdout "no errors";
        let%lwt server_pid = get_server_pid ~root in
        Unix.kill server_pid Sys.sigkill;
        (* The monitor remains running, hence also the progress file *)
        let%lwt () =
          wait_for_progress
            ~deadline:(Unix.gettimeofday () +. 60.0)
            ~expected:"server stopped"
        in
        if not (Sys_utils.file_exists (ServerFiles.server_progress_file root))
        then
          failwith "expected progress file to remain";
        Lwt.return_unit)
  in
  Lwt.return_true

let test_kill_monitor () : bool Lwt.t =
  let%lwt () =
    try_with_server [a_php] (fun ~root ->
        let%lwt stdout =
          hh ~root [| "--no-load"; "--config"; "max_workers=1" |]
        in
        String_asserter.assert_equals "No errors!\n" stdout "no errors";
        let%lwt server_pid = get_server_pid ~root in
        let stat = Proc.get_proc_stat server_pid |> Result.ok_or_failwith in
        let monitor_pid = stat.Proc.ppid in
        Unix.kill monitor_pid Sys.sigkill;
        (* the dead monitor will leave a progress file behind, but its pid is defunct *)
        let%lwt () =
          wait_for_progress
            ~deadline:(Unix.gettimeofday () +. 60.0)
            ~expected:"unknown hh_server state"
        in
        if not (Sys_utils.file_exists (ServerFiles.server_progress_file root))
        then
          failwith "expected progress file to remain";
        Lwt.return_unit)
  in
  Lwt.return_true

let () =
  Printexc.record_backtrace true;
  EventLogger.init_fake ();
  Unit_test.run_all
    [
      ("test_start_stop", (fun () -> Lwt_main.run (test_start_stop ())));
      ("test_typechecking", (fun () -> Lwt_main.run (test_typechecking ())));
      ("test_kill_server", (fun () -> Lwt_main.run (test_kill_server ())));
      ("test_kill_monitor", (fun () -> Lwt_main.run (test_kill_monitor ())));
    ]
