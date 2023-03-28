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

(** Shell out to HH_CLIENT_PATH and return its results once it's finished.
In case of success return stdout; in case of failure return a human-readable
representation that includes exit signal and stderr. *)
let hh ~(tmp : Path.t) ~(root : Path.t) (args : string array) : string Lwt.t =
  Sys.chdir (Path.to_string root);
  let hh_client_path = Sys.getenv "HH_CLIENT_PATH" in
  let env =
    Array.append [| "HH_TMPDIR=" ^ Path.to_string tmp |] (Unix.environment ())
  in
  Printf.eprintf
    "[%s] EXECUTE: hh %s\n%!"
    (Utils.timestring (Unix.gettimeofday ()))
    (Array.to_list args |> String.concat ~sep:" ");
  let (cancel, canceller) = Lwt.wait () in
  let _ =
    Lwt_unix.sleep 120.0 |> Lwt.map (fun () -> Lwt.wakeup_later canceller ())
  in
  let%lwt result =
    Lwt_utils.exec_checked
      (Exec_command.For_use_in_testing_only hh_client_path)
      ~cancel
      ~env
      args
  in
  match result with
  | Error e ->
    let e = Lwt_utils.Process_failure.to_string e in
    Printf.eprintf "<error> %s\n%!" e;
    Lwt.return e
  | Ok { Lwt_utils.Process_success.stdout; _ } -> Lwt.return stdout

(** This spawns hh, so you can await output. *)
let hh_open ~(tmp : Path.t) ~(root : Path.t) (args : string array) :
    Lwt_process.process Lwt.t =
  let hh_client_path = Sys.getenv "HH_CLIENT_PATH" in
  let env =
    Array.append [| "HH_TMPDIR=" ^ Path.to_string tmp |] (Unix.environment ())
  in
  Printf.eprintf
    "[%s] EXECUTE: hh %s\n%!"
    (Utils.timestring (Unix.gettimeofday ()))
    (Array.to_list args |> String.concat ~sep:" ");
  let process =
    Lwt_process.open_process
      ~env
      ~cwd:(Path.to_string root)
      (hh_client_path, Array.append [| hh_client_path |] args)
  in
  let%lwt () = Lwt_io.close process#stdin in
  Lwt.return process

(** This waits for substring to appear on a line of output.
If the process dies, or if we've gone a long time without a matching line,
then raises an exception. *)
let hh_await_substring (process : Lwt_process.process) ~(substring : string) :
    unit Lwt.t =
  (* helper, returns Error after a timeout *)
  let error_after_timeout () =
    let%lwt () = Lwt_unix.sleep 120.0 in
    Lwt.return_error ()
  in
  (* helper, reads a line and returns Ok *)
  let ok_after_read_line_opt () =
    try%lwt
      let%lwt line_opt = Lwt_io.read_line_opt process#stdout in
      Printf.eprintf
        "[%s] [hh_client] %s\n%!"
        (Utils.timestring (Unix.gettimeofday ()))
        (Option.value line_opt ~default:"<eof>");
      Lwt.return_ok line_opt
    with
    | exn ->
      let e = Exception.wrap exn in
      Printf.eprintf
        "[%s] [hh_client] exception %s\n%!"
        (Utils.timestring (Unix.gettimeofday ()))
        (Exception.to_string e);
      Lwt.return_ok None
  in

  let rec wait_for_substring () =
    let%lwt line =
      Lwt.pick [error_after_timeout (); ok_after_read_line_opt ()]
    in
    match line with
    | Error () -> failwith "timeout"
    | Ok None -> failwith "eof"
    | Ok (Some s) when String.is_substring s ~substring -> Lwt.return_unit
    | Ok (Some _) -> wait_for_substring ()
  in
  let%lwt () = wait_for_substring () in
  Lwt.return_unit

let dump_log ~(name : string) (file : string) : unit =
  let contents =
    try Sys_utils.cat file with
    | _ -> "<No log>"
  in
  Printf.eprintf "\n\n--> hh %s:\n%s\n\n\n%!" name contents

(** This creates a small repo, ready to run hh in it.
(there'll be no watchman nor informant support). *)
let try_with_server
    (files : (string * string) list)
    (f : tmp:Path.t -> root:Path.t -> hhi:Path.t -> unit Lwt.t) : unit Lwt.t =
  let root = Tempfile.mkdtemp ~skip_mocking:true in
  let hhi = Tempfile.mkdtemp ~skip_mocking:true in
  let tmp = Tempfile.mkdtemp ~skip_mocking:true in
  Lwt_utils.try_finally
    ~f:(fun () ->
      try%lwt
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
        ServerFiles.set_tmp_FOR_TESTING_ONLY tmp;
        let%lwt () = f ~root ~hhi ~tmp in
        Lwt.return_unit
      with
      | exn ->
        let e = Exception.wrap exn in
        dump_log ~name:"--logname" (ServerFiles.log_link root);
        dump_log ~name:"--monitor-logname" (ServerFiles.monitor_log_link root);
        dump_log ~name:"--client-logname" (ServerFiles.client_log root);
        let%lwt ls =
          Lwt_utils.exec_checked
            Exec_command.Ls
            ~timeout:5.0
            [| "-l"; Path.to_string tmp |]
        in
        let ls =
          match ls with
          | Ok r -> r.Lwt_utils.Process_success.stdout
          | Error e -> Lwt_utils.Process_failure.to_string e
        in
        Printf.eprintf "ls -l %s\n%s\n%!" (Path.to_string tmp) ls;
        Exception.reraise e)
    ~finally:(fun () ->
      let%lwt () =
        try%lwt
          let%lwt _ = hh ~tmp ~root [| "stop" |] in
          Lwt.return_unit
        with
        | _ -> Lwt.return_unit
      in
      Sys_utils.rm_dir_tree ~skip_mocking:true (Path.to_string root);
      Sys_utils.rm_dir_tree ~skip_mocking:true (Path.to_string hhi);
      Sys_utils.rm_dir_tree ~skip_mocking:true (Path.to_string tmp);
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
    try_with_server [a_php] (fun ~tmp ~root ~hhi ->
        let%lwt stdout =
          hh
            ~tmp
            ~root
            [|
              "start";
              "--no-load";
              "--config";
              "max_workers=1";
              "--custom-hhi-path";
              Path.to_string hhi;
            |]
        in
        String_asserter.assert_equals "" stdout "hh start doesn't give output";
        (* status should eventually reach "ready" *)
        let%lwt () =
          wait_for_progress
            ~deadline:(Unix.gettimeofday () +. 60.0)
            ~expected:"ready"
        in
        let%lwt _ = hh ~root ~tmp [| "stop" |] in
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
    try_with_server [a_php; loop_php] (fun ~tmp ~root ~hhi ->
        let%lwt stdout =
          hh
            ~root
            ~tmp
            [|
              "start";
              "--no-load";
              "--config";
              "max_workers=1";
              "--custom-hhi-path";
              Path.to_string hhi;
            |]
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
    try_with_server [a_php] (fun ~tmp ~root ~hhi ->
        let%lwt stdout =
          hh
            ~root
            ~tmp
            [|
              "--no-load";
              "--config";
              "max_workers=1";
              "--custom-hhi-path";
              Path.to_string hhi;
            |]
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
    try_with_server [a_php] (fun ~tmp ~root ~hhi ->
        let%lwt stdout =
          hh
            ~root
            ~tmp
            [|
              "--no-load";
              "--config";
              "max_workers=1";
              "--custom-hhi-path";
              Path.to_string hhi;
            |]
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
  let tests =
    [
      ("test_start_stop", test_start_stop);
      ("test_typechecking", test_typechecking);
      ("test_kill_server", test_kill_server);
      ("test_kill_monitor", test_kill_monitor);
    ]
    |> List.map ~f:(fun (name, f) ->
           ( name,
             fun () ->
               Printf.eprintf
                 "\n\n\n\n============================\n%s\n========================\n%!"
                 name;
               Lwt_main.run (f ()) ))
  in
  Unit_test.run_all tests
