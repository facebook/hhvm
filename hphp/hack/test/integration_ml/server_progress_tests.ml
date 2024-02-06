(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Hh_prelude
open Asserter
open Server_progress_test_helpers

let a_php = ("a.php", "<?hh\nfunction test_a(): void {}\n")

let b_php = ("b.php", "<?hh\nfunction test_b(): string {}\n")

let loop_php =
  ("loop.php", "<?hh\nfunction test_loop(): void {hh_loop_forever();}")

let assert_substring (s : string) ~(substring : string) : unit =
  if not (String.is_substring s ~substring) then
    let msg =
      Printf.sprintf
        "Expected to find substring '%s' but only found:\n%s"
        substring
        s
    in
    failwith msg

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
  let { ServerProgress.message; disposition; _ } = ServerProgress.read () in
  let actual =
    Printf.sprintf
      "[%s] %s"
      (ServerProgress.show_disposition disposition)
      message
  in
  if String.is_substring actual ~substring:expected then
    Lwt.return_unit
  else if Float.(Unix.gettimeofday () > deadline) then
    failwith
      (Printf.sprintf
         "Timeout waiting for status '%s'; got '%s'"
         expected
         actual)
  else
    let%lwt () = Lwt_unix.sleep 0.1 in
    wait_for_progress ~deadline ~expected

(** Looks for errors-file; will keep polling every 0.1s up to 'deadline' until it finds it,
and will raise an exception if it doesn't. *)
let rec wait_for_errors_file ~(deadline : float) (errors_file_path : string) :
    Unix.file_descr Lwt.t =
  let fd =
    try Some (Unix.openfile errors_file_path [Unix.O_RDONLY] 0) with
    | Unix.Unix_error (Unix.ENOENT, _, _) -> None
  in
  match fd with
  | Some fd -> Lwt.return fd
  | None when Float.(Unix.gettimeofday () > deadline) ->
    failwith
      (Printf.sprintf "Timeout waiting for errors-file %s" errors_file_path)
  | None ->
    let%lwt () = Lwt_unix.sleep 0.1 in
    wait_for_errors_file ~deadline errors_file_path

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
            ~expected:"[DReady] ready"
        in
        let%lwt _ = hh ~root ~tmp [| "stop" |] in
        (* The progress file should be cleanly deleted *)
        if Sys_utils.file_exists (ServerFiles.server_progress_file root) then
          failwith "expected progress file to be deleted";
        let%lwt () =
          wait_for_progress ~deadline:0. ~expected:"[DStopped] stopped"
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
            ~expected:"[DWorking] typechecking"
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
            ~expected:"[DStopped] server stopped"
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
            ~expected:"[DStopped] stopped"
        in
        if not (Sys_utils.file_exists (ServerFiles.server_progress_file root))
        then
          failwith "expected progress file to remain";
        Lwt.return_unit)
  in
  Lwt.return_true

let test_errors_complete () : bool Lwt.t =
  let%lwt () =
    try_with_server [b_php] (fun ~tmp ~root ~hhi ->
        let%lwt _stdout =
          hh
            ~root
            ~tmp
            [|
              "--no-load";
              "--config";
              "max_workers=1";
              "--config";
              "produce_streaming_errors=true";
              "--custom-hhi-path";
              Path.to_string hhi;
            |]
        in
        (* at this point we should have all errors available *)
        let errors_file_path = ServerFiles.errors_file_path root in
        let fd = Unix.openfile errors_file_path [Unix.O_RDONLY] 0 in
        let { ServerProgress.ErrorsRead.pid; _ } =
          ServerProgress.ErrorsRead.openfile fd |> Result.ok |> Option.value_exn
        in
        let q = Server_progress_lwt.watch_errors_file ~pid fd in
        let%lwt () = expect_qitem q "Telemetry [to_recheck_count]" in
        let%lwt () = expect_qitem q "Telemetry [will_use_distc]" in
        let%lwt () = expect_qitem q "Telemetry [process_in_parallel]" in
        let%lwt () = expect_qitem q "Errors [b.php=1]" in
        let%lwt () = expect_qitem q "Complete [complete]" in
        let%lwt () = expect_qitem q "closed" in
        Unix.close fd;
        (* a second client will also observe the files *)
        let fd = Unix.openfile errors_file_path [Unix.O_RDONLY] 0 in
        let { ServerProgress.ErrorsRead.pid; _ } =
          ServerProgress.ErrorsRead.openfile fd |> Result.ok |> Option.value_exn
        in
        let q = Server_progress_lwt.watch_errors_file ~pid fd in
        let%lwt () = expect_qitem q "Telemetry [to_recheck_count]" in
        let%lwt () = expect_qitem q "Telemetry [will_use_distc]" in
        let%lwt () = expect_qitem q "Telemetry [process_in_parallel]" in
        let%lwt () = expect_qitem q "Errors [b.php=1]" in
        let%lwt () = expect_qitem q "Complete [complete]" in
        let%lwt () = expect_qitem q "closed" in
        Unix.close fd;
        (* let's stop the server and verify that errors are absent *)
        let%lwt _ = hh ~root ~tmp [| "stop" |] in
        assert (not (Sys_utils.file_exists errors_file_path));
        Lwt.return_unit)
  in
  Lwt.return_true

let test_errors_during () : bool Lwt.t =
  let%lwt () =
    try_with_server [b_php; loop_php] (fun ~tmp ~root ~hhi ->
        (* This test will rely upon b.php being typechecked in a separate bucket from loop.php.
           To accomplish this:
           (1) we pass a custom hhi path to an empty directory so it doesn't
           typecheck more than the two files we gave it,
           (2) we set max_workers to 4 so the bucket-dividing
           algorithm will comfortably separate them. (it seems to have off-by-one oddities to just 2
           max workers isn't enough.) *)
        let%lwt _stdout =
          hh
            ~root
            ~tmp
            [|
              "start";
              "--no-load";
              "--config";
              "max_workers=4";
              "--config";
              "produce_streaming_errors=true";
              "--custom-hhi-path";
              Path.to_string hhi;
            |]
        in
        (* the file loop_php causes the typechecker to spin, typechecking, for 10mins! *)
        let errors_file_path = ServerFiles.errors_file_path root in
        let%lwt fd1 =
          wait_for_errors_file
            errors_file_path
            ~deadline:(Unix.gettimeofday () +. 60.0)
        in
        (* at this point we should have one error available *)
        let { ServerProgress.ErrorsRead.pid; _ } =
          ServerProgress.ErrorsRead.openfile fd1
          |> Result.ok
          |> Option.value_exn
        in
        let q1 = Server_progress_lwt.watch_errors_file ~pid fd1 in
        let%lwt () = expect_qitem q1 "Telemetry [to_recheck_count]" in
        let%lwt () = expect_qitem q1 "Telemetry [will_use_distc]" in
        let%lwt () = expect_qitem q1 "Telemetry [process_in_parallel]" in
        let%lwt () = expect_qitem q1 "Errors [b.php=1]" in
        let%lwt () = expect_qitem q1 "nothing" in
        (* and a second client should be able to observe it too *)
        let fd2 = Unix.openfile errors_file_path [Unix.O_RDONLY] 0 in
        let { ServerProgress.ErrorsRead.pid; _ } =
          ServerProgress.ErrorsRead.openfile fd2
          |> Result.ok
          |> Option.value_exn
        in
        let q2 = Server_progress_lwt.watch_errors_file ~pid fd2 in
        let%lwt () = expect_qitem q2 "Telemetry [to_recheck_count]" in
        let%lwt () = expect_qitem q2 "Telemetry [will_use_distc]" in
        let%lwt () = expect_qitem q2 "Telemetry [process_in_parallel]" in
        let%lwt () = expect_qitem q2 "Errors [b.php=1]" in
        let%lwt () = expect_qitem q2 "nothing" in
        (* let's stop the server and verify that errors are absent *)
        let%lwt _ = hh ~root ~tmp [| "stop" |] in
        assert (not (Sys_utils.file_exists errors_file_path));
        (* each of the two clients will see that they're done. *)
        let%lwt () = expect_qitem q1 "Stopped [unlink]" in
        let%lwt () = expect_qitem q1 "closed" in
        let%lwt () = expect_qitem q2 "Stopped [unlink]" in
        let%lwt () = expect_qitem q2 "closed" in
        Unix.close fd1;
        Unix.close fd2;
        Lwt.return_unit)
  in
  Lwt.return_true

let test_interrupt () : bool Lwt.t =
  let%lwt () =
    try_with_server [a_php] (fun ~tmp ~root ~hhi ->
        let%lwt _stdout =
          hh
            ~root
            ~tmp
            [|
              "--no-load";
              "--config";
              "produce_streaming_errors=true";
              "--config";
              "interrupt_on_client=true";
              "--custom-hhi-path";
              Path.to_string hhi;
            |]
        in
        Sys_utils.write_file ~file:(fst loop_php) (snd loop_php);
        (* the file loop_php causes the typechecker to spin, typechecking, for 10mins! *)
        let%lwt () =
          wait_for_progress
            ~deadline:(Unix.gettimeofday () +. 60.0)
            ~expected:"[DWorking] typechecking"
        in
        let stdout_future =
          hh
            ~root
            ~tmp
            [| "check"; "--config"; "consume_streaming_errors=true" |]
        in
        (* Once it's in the middle of typechecking, and once we're reading the errors-file,
           let's interrupt it with a disk change *)
        Sys_utils.write_file
          ~file:(Path.concat root "c.php" |> Path.to_string)
          "<?hh\nfunction f():int {return 1;}\n";
        (* Tests use dfind, which don't interrupt upon file-changes. So we'll cause
           an interrupt ourselves, one that will prompt it to ask dfind for changes. *)
        let%lwt _stdout =
          hh
            ~root
            ~tmp
            [|
              "check"; "--search"; "this_is_just_to_check_liveness_of_hh_server";
            |]
        in
        let%lwt stdout = stdout_future in
        assert_substring stdout ~substring:"Exit_status.Typecheck_restarted";
        assert_substring stdout ~substring:"Files have changed on disk! [";
        assert_substring stdout ~substring:"] c.php";
        Lwt.return_unit)
  in
  Lwt.return_true

let test_errors_kill () : bool Lwt.t =
  let%lwt () =
    try_with_server [b_php; loop_php] (fun ~tmp ~root ~hhi ->
        (* This test will rely upon b.php being typechecked in a separate bucket from loop.php.
           To accomplish this:
           (1) we pass a custom hhi path to an empty directory so it doesn't
           typecheck more than the two files we gave it,
           (2) we set max_workers to 4 so the bucket-dividing
           algorithm will comfortably separate them. (it seems to have off-by-one oddities to just 2
           max workers isn't enough.) *)
        let%lwt _stdout =
          hh
            ~root
            ~tmp
            [|
              "start";
              "--no-load";
              "--config";
              "max_workers=4";
              "--config";
              "produce_streaming_errors=true";
              "--custom-hhi-path";
              Path.to_string hhi;
            |]
        in
        (* I was getting timeouts when I tried to "hh check" immediately after start,
           because the client's rpc-connect wasn't getting a response until after the ten
           minutes it takes to check loop.php.
           So instead, I'll wait until the server is at least making some progress... *)
        let%lwt () =
          wait_for_progress
            ~deadline:(Unix.gettimeofday () +. 60.0)
            ~expected:"[DWorking] typechecking"
        in
        let%lwt hh_client =
          hh_open
            ~root
            ~tmp
            [|
              "check";
              "--error-format";
              "plain";
              "--config";
              "consume_streaming_errors=true";
            |]
        in
        let%lwt () = hh_await_substring hh_client ~substring:"(Typing[" in
        (* Now, kill the server! *)
        let%lwt server_pid = get_server_pid ~root in
        Unix.kill server_pid Sys.sigkill;
        (* read the remainding output from hh_client. It should detect the kill. *)
        let%lwt stdout = Lwt_io.read hh_client#stdout in
        assert_substring stdout ~substring:"Hh_server has terminated. [Killed]";
        let errors_file_path = ServerFiles.errors_file_path root in
        assert (Sys_utils.file_exists errors_file_path);
        Lwt.return_unit)
  in
  Lwt.return_true

let test_errors_existing () : bool Lwt.t =
  let%lwt () =
    try_with_server [] (fun ~tmp ~root ~hhi ->
        let%lwt _stdout =
          hh
            ~root
            ~tmp
            [|
              "--no-load";
              "--config";
              "produce_streaming_errors=true";
              "--custom-hhi-path";
              Path.to_string hhi;
            |]
        in
        Sys_utils.write_file
          ~file:(Path.concat root "a.php" |> Path.to_string)
          "<?hh\nfunction f():int {return 1}\n";
        let%lwt stdout =
          hh ~root ~tmp [| "check"; "--error-format"; "plain" |]
        in
        Sys_utils.write_file
          ~file:(Path.concat root "b.php" |> Path.to_string)
          "<?hh\nfunction g():int {}\n";
        assert_substring
          stdout
          ~substring:"A semicolon `;` is expected here. (Parsing[1002])";
        let%lwt stdout =
          hh ~root ~tmp [| "check"; "--error-format"; "plain" |]
        in
        assert_substring
          stdout
          ~substring:"A semicolon `;` is expected here. (Parsing[1002])";
        Lwt.return_unit)
  in
  Lwt.return_true

let test_client_complete () : bool Lwt.t =
  let%lwt () =
    try_with_server [b_php] (fun ~tmp ~root ~hhi ->
        let%lwt _stdout =
          hh
            ~root
            ~tmp
            [|
              "start";
              "--no-load";
              "--config";
              "max_workers=1";
              "--config";
              "produce_streaming_errors=true";
              "--custom-hhi-path";
              Path.to_string hhi;
            |]
        in
        (* will a client be able to consume streaming errors and see all of them? *)
        let%lwt stdout =
          hh ~root ~tmp [| "check"; "--error-format"; "plain" |]
        in
        assert_substring stdout ~substring:"Invalid return type (Typing[4336])";
        Lwt.return_unit)
  in
  Lwt.return_true

let test_client_during () : bool Lwt.t =
  let%lwt () =
    try_with_server [b_php; loop_php] (fun ~tmp ~root ~hhi ->
        (* This test will rely upon b.php being typechecked in a separate bucket from loop.php.
           To accomplish this:
           (1) we pass a custom hhi path to an empty directory so it doesn't
           typecheck more than the two files we gave it,
           (2) we set max_workers to 4 so the bucket-dividing
           algorithm will comfortably separate them. (it seems to have off-by-one oddities to just 2
           max workers isn't enough.) *)
        let%lwt _stdout =
          hh
            ~root
            ~tmp
            [|
              "start";
              "--no-load";
              "--config";
              "max_workers=4";
              "--config";
              "produce_streaming_errors=true";
              "--custom-hhi-path";
              Path.to_string hhi;
            |]
        in
        (* I was getting timeouts when I tried to "hh check" immediately after start,
           because the client's rpc-connect wasn't getting a response until after the ten
           minutes it takes to check loop.php.
           So instead, I'll wait until the server is at least making some progress... *)
        let%lwt () =
          wait_for_progress
            ~deadline:(Unix.gettimeofday () +. 60.0)
            ~expected:"[DWorking] typechecking"
        in
        let args =
          [|
            "check";
            "--error-format";
            "plain";
            "--config";
            "consume_streaming_errors=true";
          |]
        in
        let%lwt hh_client = hh_open ~root ~tmp args in
        let%lwt () = hh_await_substring hh_client ~substring:"(Typing[" in
        hh_client#kill Sys.sigkill;
        let%lwt _state = hh_client#close in

        (* For our next trick, we'll kick off another hh check.
           It will presumably emit the error line pretty quickly (just by reading errors.bin).
           Once it has, we'll "hh stop".
           This should make our hh check terminate pretty quickly,
           and its output should indicate that hh died. *)
        let%lwt hh_client = hh_open ~root ~tmp args in
        let%lwt () = hh_await_substring hh_client ~substring:"(Typing[" in
        let%lwt _ = hh ~root ~tmp [| "stop" |] in
        (* read the remainding output from hh_client *)
        let%lwt stdout = Lwt_io.read hh_client#stdout in
        assert_substring stdout ~substring:"Hh_server has terminated. [Stopped]";
        Lwt.return_unit)
  in
  Lwt.return_true

let test_start () : bool Lwt.t =
  let%lwt () =
    try_with_server [b_php] (fun ~tmp ~root ~hhi ->
        let%lwt stdout =
          hh
            ~root
            ~tmp
            [|
              "--no-load";
              "--config";
              "max_workers=1";
              "--config";
              "produce_streaming_errors=true";
              "--error-format";
              "plain";
              "--custom-hhi-path";
              Path.to_string hhi;
              "--autostart-server";
              "true";
            |]
        in
        (* This check should cause the server to start up *)
        assert_substring stdout ~substring:"Invalid return type (Typing[4336])";
        Lwt.return_unit)
  in
  Lwt.return_true

let test_no_start () : bool Lwt.t =
  let%lwt () =
    try_with_server [b_php] (fun ~tmp ~root ~hhi ->
        let%lwt stdout =
          hh
            ~root
            ~tmp
            [|
              "--config";
              "max_workers=1";
              "--config";
              "produce_streaming_errors=true";
              "--error-format";
              "plain";
              "--custom-hhi-path";
              Path.to_string hhi;
              "--autostart-server";
              "false";
            |]
        in
        (* This exercises synchronous failure when clientCheckStatus.go_streaming sees
           no errors.bin, determines to call ClientConnect.connect, but the connection
           attempt exits fast because of autostart-server false. *)
        assert_substring stdout ~substring:"WEXITED";
        assert_substring stdout ~substring:"no hh_server running";
        Lwt.return_unit)
  in
  Lwt.return_true

let test_no_load () : bool Lwt.t =
  let%lwt () =
    try_with_server [b_php] (fun ~tmp ~root ~hhi ->
        let%lwt stdout =
          hh
            ~root
            ~tmp
            [|
              "--config";
              "max_workers=1";
              "--config";
              "produce_streaming_errors=true";
              "--error-format";
              "plain";
              "--custom-hhi-path";
              Path.to_string hhi;
              "--autostart-server";
              "true";
              "--config";
              "use_mini_state=true";
              "--config";
              "load_state_natively_v4=true";
              "--config";
              "require_saved_state=true";
            |]
        in
        (* This exercises asynchronous failure when clientCheckStatus.go_streaming sees
           no errors.bin, determines to call ClientConnect.connect which goes ahead,
           then the server fails to load a saved-state (we didn't pass --no-load),
           and the server exits, and ClientConnect.connect eventually gets that
           server failure and reports it. *)
        assert_substring stdout ~substring:"WEXITED";
        assert_substring stdout ~substring:"Failed_to_load_should_abort";
        assert_substring
          stdout
          ~substring:"LoadError.Find_hh_server_hash_failed";
        Lwt.return_unit)
  in
  Lwt.return_true

let test_hhi_error () : bool Lwt.t =
  let%lwt () =
    (* This test the ability of streaming-errors to render an error that mentions an hhi file.
       It should produce "stdClass::f doesn't exist; see this definition of stdClass in hhi..." *)
    let use_hhi_php =
      ("use_hhi.php", "<?hh\nfunction test_use_hhi(): void { stdClass::f(); }\n")
    in
    try_with_server [use_hhi_php] (fun ~tmp ~root ~hhi:_ ->
        let%lwt stdout =
          hh
            ~root
            ~tmp
            [|
              "--no-load";
              "--config";
              "max_workers=1";
              "--config";
              "produce_streaming_errors=true";
              "--config";
              "consume_streaming_errors=true";
              "--error-format";
              "plain";
            |]
        in
        assert_substring stdout ~substring:"Exit_status.Type_error";
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
      ("test_errors_complete", test_errors_complete);
      ("test_errors_during", test_errors_during);
      ("test_errors_kill", test_errors_kill);
      ("test_errors_existing", test_errors_existing);
      ("test_client_complete", test_client_complete);
      ("test_client_during", test_client_during);
      ("test_start", test_start);
      ("test_no_start", test_no_start);
      ("test_no_load", test_no_load);
      ("test_hhi_error", test_hhi_error);
      ("test_interrupt", test_interrupt);
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
