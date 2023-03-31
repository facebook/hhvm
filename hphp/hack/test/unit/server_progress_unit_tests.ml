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

let pid = Unix.getpid ()

let expect_exn (f : unit -> unit) : unit =
  let got_exn =
    try
      f ();
      false
    with
    | _ -> true
  in
  if not got_exn then failwith "Expected an exception"

let expect_state (expected : string) : unit =
  let actual = ServerProgress.ErrorsWrite.get_state_FOR_TEST () in
  String_asserter.assert_equals expected actual "expect_state"

let telemetry = Telemetry.create ()

let try_with_tmp (f : root:Path.t -> unit Lwt.t) : unit Lwt.t =
  let tmp = Tempfile.mkdtemp ~skip_mocking:true |> Path.to_string in
  Lwt_utils.try_finally
    ~f:(fun () ->
      (* We want ServerFiles.errors_file to be placed in this test's temp directory *)
      Unix.putenv "HH_TMPDIR" tmp;
      (* We need ServerProgress.Errors to have a root (any root) so it knows how to name the errors file *)
      let root = Path.make "test" in
      ServerProgress.set_root root;
      Relative_path.set_path_prefix Relative_path.Root root;
      (* To isolate tests, we have to reset ServerProgress.Errors global state. *)
      ServerProgress.ErrorsWrite.unlink_at_server_stop ();
      let%lwt () = f ~root in
      Lwt.return_unit)
    ~finally:(fun () ->
      Sys_utils.rm_dir_tree ~skip_mocking:true tmp;
      Lwt.return_unit)

let make_errors (errors : (string * string) list) : Errors.t =
  let errors =
    List.map errors ~f:(fun (suffix, message) ->
        let path = Relative_path.from_root ~suffix in
        let error = User_error.make 101 (Pos.make_from path, message) [] in
        (path, error))
  in
  Errors.from_file_error_list errors

let an_error : Errors.t = make_errors [("c", "oops")]

let test_completed () : bool Lwt.t =
  let%lwt () =
    try_with_tmp (fun ~root ->
        let errors_file = ServerFiles.errors_file root in
        ServerProgress.ErrorsWrite.new_empty_file
          ~clock:None
          ~ignore_hh_version:false;
        ServerProgress.ErrorsWrite.report
          (make_errors [("a", "hello"); ("a", "there"); ("b", "world")]);
        ServerProgress.ErrorsWrite.report (make_errors [("c", "oops")]);
        ServerProgress.ErrorsWrite.complete telemetry;
        let fd = Unix.openfile errors_file [Unix.O_RDONLY] 0 in
        assert (ServerProgress.ErrorsRead.openfile fd |> Result.is_ok);
        String_asserter.assert_equals
          "Errors [a=2,b=1]"
          (ServerProgress.ErrorsRead.read_next_errors fd |> show_read)
          "errors";
        String_asserter.assert_equals
          "Errors [c=1]"
          (ServerProgress.ErrorsRead.read_next_errors fd |> show_read)
          "errors";
        String_asserter.assert_equals
          "Complete [complete]"
          (ServerProgress.ErrorsRead.read_next_errors fd |> show_read)
          "complete";
        Unix.close fd;
        Lwt.return_unit)
  in
  Lwt.return_true

let test_read_empty () : bool Lwt.t =
  let%lwt () =
    try_with_tmp (fun ~root ->
        let errors_file = ServerFiles.errors_file root in
        Sys_utils.touch
          (Sys_utils.Touch_existing_or_create_new
             { mkdir_if_new = false; perm_if_new = 0o666 })
          errors_file;
        let fd = Unix.openfile errors_file [Unix.O_RDONLY] 0 in
        String_asserter.assert_equals
          "NothingYet [no additional bytes]"
          (ServerProgress.ErrorsRead.read_next_errors fd |> show_read)
          "killed";
        Unix.close fd;
        Lwt.return_unit)
  in
  Lwt.return_true

let test_read_unlinked () : bool Lwt.t =
  let%lwt () =
    try_with_tmp (fun ~root ->
        let errors_file = ServerFiles.errors_file root in
        ServerProgress.ErrorsWrite.new_empty_file
          ~clock:None
          ~ignore_hh_version:false;
        let fd = Unix.openfile errors_file [Unix.O_RDONLY] 0 in
        ServerProgress.ErrorsWrite.unlink_at_server_stop ();
        assert (ServerProgress.ErrorsRead.openfile fd |> Result.is_ok);
        String_asserter.assert_equals
          "Stopped [unlink]"
          (ServerProgress.ErrorsRead.read_next_errors fd |> show_read)
          "end";
        Unix.close fd;
        Lwt.return_unit)
  in
  Lwt.return_true

let test_read_unlinked_empty () : bool Lwt.t =
  let%lwt () =
    try_with_tmp (fun ~root ->
        let errors_file = ServerFiles.errors_file root in
        ServerProgress.ErrorsWrite.new_empty_file
          ~clock:None
          ~ignore_hh_version:false;
        let fd = Unix.openfile errors_file [Unix.O_RDONLY] 0 in
        assert (ServerProgress.ErrorsRead.openfile fd |> Result.is_ok);
        ServerProgress.ErrorsWrite.unlink_at_server_stop ();
        String_asserter.assert_equals
          "Stopped [unlink]"
          (ServerProgress.ErrorsRead.read_next_errors fd |> show_read)
          "end";
        Unix.close fd;
        Lwt.return_unit)
  in
  Lwt.return_true

let test_read_restarted () : bool Lwt.t =
  let%lwt () =
    try_with_tmp (fun ~root ->
        let errors_file = ServerFiles.errors_file root in
        ServerProgress.ErrorsWrite.new_empty_file
          ~clock:None
          ~ignore_hh_version:false;
        ServerProgress.ErrorsWrite.report an_error;
        let fd = Unix.openfile errors_file [Unix.O_RDONLY] 0 in
        ServerProgress.ErrorsWrite.new_empty_file
          ~clock:None
          ~ignore_hh_version:false;
        assert (ServerProgress.ErrorsRead.openfile fd |> Result.is_ok);
        String_asserter.assert_equals
          "Errors [c=1]"
          (ServerProgress.ErrorsRead.read_next_errors fd |> show_read)
          "errors";
        String_asserter.assert_equals
          "Restarted [new_empty_file]"
          (ServerProgress.ErrorsRead.read_next_errors fd |> show_read)
          "end";
        Unix.close fd;
        Lwt.return_unit)
  in
  Lwt.return_true

let test_locks () : bool Lwt.t =
  (* This is just too irritating to test, so I'm not going to...
     The ServerProgress.Errors functions all wait synchronously until they can acquire a lock,
     then do a tiny amount of work, then release the lock. The only way to test this would be
     with multiple threads, which I'm loathe to do. *)
  Lwt.return_true

let test_read_half_message () : bool Lwt.t =
  let%lwt () =
    try_with_tmp (fun ~root ->
        let errors_file = ServerFiles.errors_file root in
        let preamble = Marshal_tools.make_preamble 15 in
        Sys_utils.write_file ~file:errors_file (Bytes.to_string preamble);
        let fd = Unix.openfile errors_file [Unix.O_RDONLY] 0 in
        String_asserter.assert_equals
          "Killed [no payload]"
          (ServerProgress.ErrorsRead.read_next_errors fd |> show_read)
          "half";
        Unix.close fd;
        Lwt.return_unit)
  in
  Lwt.return_true

let test_read_dead_pid () : bool Lwt.t =
  let%lwt () =
    try_with_tmp (fun ~root ->
        ServerProgress.ErrorsWrite.create_file_FOR_TEST ~pid:1 ~cmdline:"bogus";
        let errors_file = ServerFiles.errors_file root in
        let fd = Unix.openfile errors_file [Unix.O_RDONLY] 0 in
        begin
          match ServerProgress.ErrorsRead.openfile fd with
          | Error (ServerProgress.Killed, "Errors-file is from defunct PID") ->
            ()
          | Ok _ -> failwith "Expected a dead-pid failure, not success"
          | Error e -> failwith ("Expected Killed, not " ^ show_read (Error e))
        end;
        Unix.close fd;
        Lwt.return_unit)
  in
  Lwt.return_true

let test_produce_disordered () : bool Lwt.t =
  (* This test tests all actions "new_empty_file, report, complete, unlink"
     from all possible states "Absent, Empty, Reporting, Closed" *)
  let%lwt () =
    try_with_tmp (fun ~root:_ ->
        (* Actions from state "Absent"... *)
        expect_state "Absent";
        expect_exn (fun () -> ServerProgress.ErrorsWrite.report an_error);
        expect_state "Absent";
        expect_exn (fun () -> ServerProgress.ErrorsWrite.complete telemetry);
        expect_state "Absent";
        ServerProgress.ErrorsWrite.unlink_at_server_stop ();
        expect_state "Absent";
        ServerProgress.ErrorsWrite.new_empty_file
          ~clock:None
          ~ignore_hh_version:false;
        expect_state "Reporting[0]";
        Lwt.return_unit)
  in
  let%lwt () =
    try_with_tmp (fun ~root:_ ->
        (* Actions from state "Reporting[0]"... *)
        ServerProgress.ErrorsWrite.new_empty_file
          ~clock:None
          ~ignore_hh_version:false;
        expect_state "Reporting[0]";
        ServerProgress.ErrorsWrite.new_empty_file
          ~clock:None
          ~ignore_hh_version:false;
        expect_state "Reporting[0]";
        ServerProgress.ErrorsWrite.report an_error;
        expect_state "Reporting[1]";
        ServerProgress.ErrorsWrite.new_empty_file
          ~clock:None
          ~ignore_hh_version:false;
        ServerProgress.ErrorsWrite.unlink_at_server_stop ();
        expect_state "Absent";
        ServerProgress.ErrorsWrite.new_empty_file
          ~clock:None
          ~ignore_hh_version:false;
        expect_state "Reporting[0]";
        ServerProgress.ErrorsWrite.complete telemetry;
        expect_state "Closed";
        Lwt.return_unit)
  in
  let%lwt () =
    try_with_tmp (fun ~root:_ ->
        (* Actions from state "Reporting[1]" *)
        ServerProgress.ErrorsWrite.new_empty_file
          ~clock:None
          ~ignore_hh_version:false;
        ServerProgress.ErrorsWrite.report an_error;
        expect_state "Reporting[1]";
        ServerProgress.ErrorsWrite.new_empty_file
          ~clock:None
          ~ignore_hh_version:false;
        expect_state "Reporting[0]";
        ServerProgress.ErrorsWrite.report an_error;
        expect_state "Reporting[1]";
        ServerProgress.ErrorsWrite.report an_error;
        expect_state "Reporting[2]";
        ServerProgress.ErrorsWrite.complete telemetry;
        expect_state "Closed";
        ServerProgress.ErrorsWrite.new_empty_file
          ~clock:None
          ~ignore_hh_version:false;
        ServerProgress.ErrorsWrite.report an_error;
        expect_state "Reporting[1]";
        ServerProgress.ErrorsWrite.unlink_at_server_stop ();
        expect_state "Absent";
        Lwt.return_unit)
  in
  let%lwt () =
    try_with_tmp (fun ~root:_ ->
        (* Actions from state "Closed" *)
        ServerProgress.ErrorsWrite.new_empty_file
          ~clock:None
          ~ignore_hh_version:false;
        ServerProgress.ErrorsWrite.complete telemetry;
        expect_state "Closed";
        ServerProgress.ErrorsWrite.new_empty_file
          ~clock:None
          ~ignore_hh_version:false;
        expect_state "Reporting[0]";
        ServerProgress.ErrorsWrite.complete telemetry;
        expect_state "Closed";
        expect_exn (fun () -> ServerProgress.ErrorsWrite.report an_error);
        expect_state "Closed";
        expect_exn (fun () -> ServerProgress.ErrorsWrite.complete telemetry);
        expect_state "Closed";
        ServerProgress.ErrorsWrite.unlink_at_server_stop ();
        expect_state "Absent";
        Lwt.return_unit)
  in
  Lwt.return_true

let test_async_read_completed () : bool Lwt.t =
  let%lwt () =
    try_with_tmp (fun ~root ->
        let errors_file = ServerFiles.errors_file root in
        ServerProgress.ErrorsWrite.new_empty_file
          ~clock:None
          ~ignore_hh_version:false;
        ServerProgress.ErrorsWrite.report
          (make_errors [("a", "hello"); ("a", "there"); ("b", "world")]);
        ServerProgress.ErrorsWrite.report (make_errors [("c", "oops")]);
        ServerProgress.ErrorsWrite.complete telemetry;
        let fd = Unix.openfile errors_file [Unix.O_RDONLY] 0 in
        let _open = ServerProgress.ErrorsRead.openfile fd in
        let q = ServerProgressLwt.watch_errors_file ~pid fd in
        let%lwt () = expect_qitem q "Errors [a=2,b=1]" in
        let%lwt () = expect_qitem q "Errors [c=1]" in
        let%lwt () = expect_qitem q "Complete [complete]" in
        let%lwt () = expect_qitem q "closed" in
        Lwt.return_unit)
  in
  Lwt.return_true

let test_async_read_partial () : bool Lwt.t =
  let%lwt () =
    try_with_tmp (fun ~root ->
        let errors_file = ServerFiles.errors_file root in
        ServerProgress.ErrorsWrite.new_empty_file
          ~clock:None
          ~ignore_hh_version:false;
        let fd = Unix.openfile errors_file [Unix.O_RDONLY] 0 in
        let _open = ServerProgress.ErrorsRead.openfile fd in
        let q = ServerProgressLwt.watch_errors_file ~pid fd in
        (* initially there are no items available *)
        let%lwt () = expect_qitem q "nothing" in
        (* we'll put in one report, and after this there should be exactly one item available *)
        ServerProgress.ErrorsWrite.report
          (make_errors [("a", "hello"); ("a", "there"); ("b", "world")]);
        let%lwt () = expect_qitem q "Errors [a=2,b=1]" in
        let%lwt () = expect_qitem q "nothing" in
        (* we'll complete the file, and after this the stream should be closed *)
        ServerProgress.ErrorsWrite.complete (Telemetry.create ());
        let%lwt () = expect_qitem q "Complete [complete]" in
        let%lwt () = expect_qitem q "closed" in
        Lwt.return_unit)
  in
  Lwt.return_true

let test_async_read_unlinked () : bool Lwt.t =
  let%lwt () =
    try_with_tmp (fun ~root ->
        let errors_file = ServerFiles.errors_file root in
        ServerProgress.ErrorsWrite.new_empty_file
          ~clock:(Some "clock123")
          ~ignore_hh_version:false;
        let fd = Unix.openfile errors_file [Unix.O_RDONLY] 0 in
        let open_result = ServerProgress.ErrorsRead.openfile fd in
        let { ServerProgress.ErrorsRead.pid; clock; _ } =
          Result.ok open_result |> Option.value_exn
        in
        assert (Option.equal String.equal (Some "clock123") clock);
        assert (pid = Unix.getpid ());
        let q = ServerProgressLwt.watch_errors_file ~pid fd in
        (* we'll unlink the file, and after this the stream should be closed *)
        ServerProgress.ErrorsWrite.unlink_at_server_stop ();
        let%lwt () = expect_qitem q "Stopped [unlink]" in
        let%lwt () = expect_qitem q "closed" in
        Lwt.return_unit)
  in
  Lwt.return_true

let test_start_read_killed () : bool Lwt.t =
  let%lwt () =
    try_with_tmp (fun ~root ->
        (* we'll write an incomplete file *)
        let errors_file = ServerFiles.errors_file root in
        Sys_utils.write_file ~file:errors_file "a";
        let fd = Unix.openfile errors_file [Unix.O_RDONLY] 0 in
        (* files are created atomically; there should be no way to read
           an empty file, not even if the creating process got killed. *)
        let is_ok =
          try
            let _ = ServerProgress.ErrorsRead.openfile fd in
            true
          with
          | _ -> false
        in
        Unix.close fd;
        if is_ok then
          failwith "we shouldn't be able to openfile on an impossible file";
        Lwt.return_unit)
  in
  Lwt.return_true

let test_async_read_killed () : bool Lwt.t =
  let%lwt () =
    try_with_tmp (fun ~root ->
        let errors_file = ServerFiles.errors_file root in
        ServerProgress.ErrorsWrite.new_empty_file
          ~clock:None
          ~ignore_hh_version:false;
        (* we'll write an incomplete file *)
        let preamble = Marshal_tools.make_preamble 100 in
        Sys_utils.append_file ~file:errors_file (Bytes.to_string preamble);
        let fd = Unix.openfile errors_file [Unix.O_RDONLY] 0 in
        let { ServerProgress.ErrorsRead.pid; _ } =
          ServerProgress.ErrorsRead.openfile fd |> Result.ok |> Option.value_exn
        in
        let q = ServerProgressLwt.watch_errors_file ~pid fd in
        (* because the file is incomplete, the queue should assume that the
           creating process was killed *)
        let%lwt () = expect_qitem q "Killed [no payload]" in
        let%lwt () = expect_qitem q "closed" in
        Lwt.return_unit)
  in
  Lwt.return_true

let test_async_pid_killed () : bool Lwt.t =
  let%lwt () =
    try_with_tmp (fun ~root ->
        (* This will simulate a file which is in a complete state, but the producing
           PID simply died *)
        let errors_file = ServerFiles.errors_file root in
        ServerProgress.ErrorsWrite.new_empty_file
          ~clock:None
          ~ignore_hh_version:false;
        let fd = Unix.openfile errors_file [Unix.O_RDONLY] 0 in
        let _open = ServerProgress.ErrorsRead.openfile fd in
        (* For our test, let us pick a pid which doesn't exist.
           This is a bit racey, but it's the best we can do.
           We trust that new pids are assigned in ascending order,
           so we'll start from the current pid and work our way down.
           If we hit 0, we'll wrap-around to Int.max_value_30bits because
           Unix.kill needs a positive PID. *)
        let rec pick_dead_pid pid =
          try
            Unix.kill pid 0;
            let pid =
              if pid = 1 then
                Int.max_value_30_bits
              else
                pid - 1
            in
            pick_dead_pid pid
          with
          | _ -> pid
        in
        let dead_pid = pick_dead_pid (Unix.getpid ()) in
        let q = ServerProgressLwt.watch_errors_file ~pid:dead_pid fd in
        (* because the pid is dead, within 5s, the queue should report
           that the creating process was killed *)
        let%lwt () = expect_qitem ~delay:60.0 q "Killed [pid]" in
        let%lwt () = expect_qitem q "closed" in
        Lwt.return_unit)
  in
  Lwt.return_true

let test_async_read_start () : bool Lwt.t =
  let%lwt () =
    try_with_tmp (fun ~root ->
        let errors_file = ServerFiles.errors_file root in
        let pid = Unix.getpid () in
        ServerProgress.ErrorsWrite.new_empty_file
          ~clock:None
          ~ignore_hh_version:false;
        let fd = Unix.openfile errors_file [Unix.O_RDONLY] 0 in
        (* oops! we didn't call ServerProgress.ErrorsRead.openfile *)
        let exn =
          try
            let _q = ServerProgressLwt.watch_errors_file ~pid fd in
            "successfully opened queue"
          with
          | exn -> Exn.to_string exn
        in
        assert (
          String.is_substring exn ~substring:"openfile before read_next_error");
        Lwt.return_unit)
  in
  Lwt.return_true

let () =
  Printexc.record_backtrace true;
  EventLogger.init_fake ();
  Unit_test.run_all
    [
      ("test_completed", (fun () -> Lwt_main.run (test_completed ())));
      ("test_read_empty", (fun () -> Lwt_main.run (test_read_empty ())));
      ("test_read_unlinked", (fun () -> Lwt_main.run (test_read_unlinked ())));
      ( "test_read_unlinked_empty",
        (fun () -> Lwt_main.run (test_read_unlinked_empty ())) );
      ("test_read_restarted", (fun () -> Lwt_main.run (test_read_restarted ())));
      ("test_read_halfway", (fun () -> Lwt_main.run (test_read_half_message ())));
      ("test_read_dead_pid", (fun () -> Lwt_main.run (test_read_dead_pid ())));
      ("test_locks", (fun () -> Lwt_main.run (test_locks ())));
      ( "test_produce_disordered",
        (fun () -> Lwt_main.run (test_produce_disordered ())) );
      ( "test_async_read_completed",
        (fun () -> Lwt_main.run (test_async_read_completed ())) );
      ( "test_async_read_partial",
        (fun () -> Lwt_main.run (test_async_read_partial ())) );
      ( "test_async_read_unlinked",
        (fun () -> Lwt_main.run (test_async_read_unlinked ())) );
      ( "test_start_read_killed",
        (fun () -> Lwt_main.run (test_start_read_killed ())) );
      ( "test_async_read_killed",
        (fun () -> Lwt_main.run (test_async_read_killed ())) );
      ( "test_async_pid_killed",
        (fun () -> Lwt_main.run (test_async_pid_killed ())) );
      ( "test_async_read_start",
        (fun () -> Lwt_main.run (test_async_read_start ())) );
    ]
