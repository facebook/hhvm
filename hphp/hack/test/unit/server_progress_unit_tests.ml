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

let cancel_reason = ("test", "")

let try_with_tmp (f : root:Path.t -> unit Lwt.t) : unit Lwt.t =
  let tmp = Tempfile.mkdtemp ~skip_mocking:true in
  Lwt_utils.try_finally
    ~f:(fun () ->
      (* We want ServerFiles.errors_file to be placed in this test's temp directory *)
      ServerFiles.set_tmp_FOR_TESTING_ONLY tmp;
      (* We need ServerProgress.Errors to have a root (any root) so it knows how to name the errors file *)
      let root = Path.make "test" in
      ServerProgress.set_root root;
      Relative_path.set_path_prefix Relative_path.Root root;
      (* To isolate tests, we have to reset ServerProgress.Errors global state in memory. *)
      ServerProgress.ErrorsWrite.unlink_at_server_stop ();
      let%lwt () = f ~root in
      Lwt.return_unit)
    ~finally:(fun () ->
      Sys_utils.rm_dir_tree ~skip_mocking:true (Path.to_string tmp);
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
        let errors_file_path = ServerFiles.errors_file_path root in
        ServerProgress.ErrorsWrite.new_empty_file
          ~clock:None
          ~ignore_hh_version:false
          ~cancel_reason;
        ServerProgress.ErrorsWrite.report
          (make_errors [("a", "hello"); ("a", "there"); ("b", "world")]);
        ServerProgress.ErrorsWrite.report (make_errors [("c", "oops")]);
        ServerProgress.ErrorsWrite.complete telemetry;
        let fd = Unix.openfile errors_file_path [Unix.O_RDONLY] 0 in
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
        let errors_file_path = ServerFiles.errors_file_path root in
        Sys_utils.touch
          (Sys_utils.Touch_existing_or_create_new
             { mkdir_if_new = false; perm_if_new = 0o666 })
          errors_file_path;
        let fd = Unix.openfile errors_file_path [Unix.O_RDONLY] 0 in
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
        let errors_file_path = ServerFiles.errors_file_path root in
        ServerProgress.ErrorsWrite.new_empty_file
          ~clock:None
          ~ignore_hh_version:false
          ~cancel_reason;
        let fd = Unix.openfile errors_file_path [Unix.O_RDONLY] 0 in
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
        let errors_file_path = ServerFiles.errors_file_path root in
        ServerProgress.ErrorsWrite.new_empty_file
          ~clock:None
          ~ignore_hh_version:false
          ~cancel_reason;
        let fd = Unix.openfile errors_file_path [Unix.O_RDONLY] 0 in
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
        let errors_file_path = ServerFiles.errors_file_path root in
        ServerProgress.ErrorsWrite.new_empty_file
          ~clock:None
          ~ignore_hh_version:false
          ~cancel_reason;
        ServerProgress.ErrorsWrite.report an_error;
        let fd = Unix.openfile errors_file_path [Unix.O_RDONLY] 0 in
        ServerProgress.ErrorsWrite.new_empty_file
          ~clock:None
          ~ignore_hh_version:false
          ~cancel_reason;
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
        let errors_file_path = ServerFiles.errors_file_path root in
        let preamble = Marshal_tools.make_preamble 15 in
        Sys_utils.write_file ~file:errors_file_path (Bytes.to_string preamble);
        let fd = Unix.openfile errors_file_path [Unix.O_RDONLY] 0 in
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
        let errors_file_path = ServerFiles.errors_file_path root in
        let fd = Unix.openfile errors_file_path [Unix.O_RDONLY] 0 in
        begin
          match ServerProgress.ErrorsRead.openfile fd with
          | Error (ServerProgress.Killed _, "Errors-file is from defunct PID")
            ->
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
          ~ignore_hh_version:false
          ~cancel_reason;
        expect_state "Reporting[0]";
        Lwt.return_unit)
  in
  let%lwt () =
    try_with_tmp (fun ~root:_ ->
        (* Actions from state "Reporting[0]"... *)
        ServerProgress.ErrorsWrite.new_empty_file
          ~clock:None
          ~ignore_hh_version:false
          ~cancel_reason;
        expect_state "Reporting[0]";
        ServerProgress.ErrorsWrite.new_empty_file
          ~clock:None
          ~ignore_hh_version:false
          ~cancel_reason;
        expect_state "Reporting[0]";
        ServerProgress.ErrorsWrite.report an_error;
        expect_state "Reporting[1]";
        ServerProgress.ErrorsWrite.new_empty_file
          ~clock:None
          ~ignore_hh_version:false
          ~cancel_reason;
        ServerProgress.ErrorsWrite.unlink_at_server_stop ();
        expect_state "Absent";
        ServerProgress.ErrorsWrite.new_empty_file
          ~clock:None
          ~ignore_hh_version:false
          ~cancel_reason;
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
          ~ignore_hh_version:false
          ~cancel_reason;
        ServerProgress.ErrorsWrite.report an_error;
        expect_state "Reporting[1]";
        ServerProgress.ErrorsWrite.new_empty_file
          ~clock:None
          ~ignore_hh_version:false
          ~cancel_reason;
        expect_state "Reporting[0]";
        ServerProgress.ErrorsWrite.report an_error;
        expect_state "Reporting[1]";
        ServerProgress.ErrorsWrite.report an_error;
        expect_state "Reporting[2]";
        ServerProgress.ErrorsWrite.complete telemetry;
        expect_state "Closed";
        ServerProgress.ErrorsWrite.new_empty_file
          ~clock:None
          ~ignore_hh_version:false
          ~cancel_reason;
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
          ~ignore_hh_version:false
          ~cancel_reason;
        ServerProgress.ErrorsWrite.complete telemetry;
        expect_state "Closed";
        ServerProgress.ErrorsWrite.new_empty_file
          ~clock:None
          ~ignore_hh_version:false
          ~cancel_reason;
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
        let errors_file_path = ServerFiles.errors_file_path root in
        ServerProgress.ErrorsWrite.new_empty_file
          ~clock:None
          ~ignore_hh_version:false
          ~cancel_reason;
        ServerProgress.ErrorsWrite.report
          (make_errors [("a", "hello"); ("a", "there"); ("b", "world")]);
        ServerProgress.ErrorsWrite.report (make_errors [("c", "oops")]);
        ServerProgress.ErrorsWrite.complete telemetry;
        let fd = Unix.openfile errors_file_path [Unix.O_RDONLY] 0 in
        let _open = ServerProgress.ErrorsRead.openfile fd in
        let q = Server_progress_lwt.watch_errors_file ~pid fd in
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
        let errors_file_path = ServerFiles.errors_file_path root in
        ServerProgress.ErrorsWrite.new_empty_file
          ~clock:None
          ~ignore_hh_version:false
          ~cancel_reason;
        let fd = Unix.openfile errors_file_path [Unix.O_RDONLY] 0 in
        let _open = ServerProgress.ErrorsRead.openfile fd in
        let q = Server_progress_lwt.watch_errors_file ~pid fd in
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
        let errors_file_path = ServerFiles.errors_file_path root in
        ServerProgress.ErrorsWrite.new_empty_file
          ~clock:(Some "clock123")
          ~ignore_hh_version:false
          ~cancel_reason;
        let fd = Unix.openfile errors_file_path [Unix.O_RDONLY] 0 in
        let open_result = ServerProgress.ErrorsRead.openfile fd in
        let { ServerProgress.ErrorsRead.pid; clock; _ } =
          Result.ok open_result |> Option.value_exn
        in
        assert (Option.equal String.equal (Some "clock123") clock);
        assert (pid = Unix.getpid ());
        let q = Server_progress_lwt.watch_errors_file ~pid fd in
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
        let errors_file_path = ServerFiles.errors_file_path root in
        Sys_utils.write_file ~file:errors_file_path "a";
        let fd = Unix.openfile errors_file_path [Unix.O_RDONLY] 0 in
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
        let errors_file_path = ServerFiles.errors_file_path root in
        ServerProgress.ErrorsWrite.new_empty_file
          ~clock:None
          ~ignore_hh_version:false
          ~cancel_reason;
        (* we'll write an incomplete file *)
        let preamble = Marshal_tools.make_preamble 100 in
        Sys_utils.append_file ~file:errors_file_path (Bytes.to_string preamble);
        let fd = Unix.openfile errors_file_path [Unix.O_RDONLY] 0 in
        let { ServerProgress.ErrorsRead.pid; _ } =
          ServerProgress.ErrorsRead.openfile fd |> Result.ok |> Option.value_exn
        in
        let q = Server_progress_lwt.watch_errors_file ~pid fd in
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
        let errors_file_path = ServerFiles.errors_file_path root in
        ServerProgress.ErrorsWrite.new_empty_file
          ~clock:None
          ~ignore_hh_version:false
          ~cancel_reason;
        let fd = Unix.openfile errors_file_path [Unix.O_RDONLY] 0 in
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
        let q = Server_progress_lwt.watch_errors_file ~pid:dead_pid fd in
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
        let errors_file_path = ServerFiles.errors_file_path root in
        let pid = Unix.getpid () in
        ServerProgress.ErrorsWrite.new_empty_file
          ~clock:None
          ~ignore_hh_version:false
          ~cancel_reason;
        let fd = Unix.openfile errors_file_path [Unix.O_RDONLY] 0 in
        (* oops! we didn't call ServerProgress.ErrorsRead.openfile *)
        let exn =
          try
            let _q = Server_progress_lwt.watch_errors_file ~pid fd in
            "successfully opened queue"
          with
          | exn -> Exn.to_string exn
        in
        assert (
          String.is_substring exn ~substring:"openfile before read_next_error");
        Lwt.return_unit)
  in
  Lwt.return_true

let env =
  ClientEnv.
    {
      autostart = false;
      config = [];
      custom_hhi_path = None;
      custom_telemetry_data = [];
      error_format = Errors.Plain;
      force_dormant_start = false;
      from = "test";
      show_spinner = false;
      gen_saved_ignore_type_errors = false;
      ignore_hh_version = true;
      saved_state_ignore_hhconfig = true;
      paths = [];
      log_inference_constraints = false;
      max_errors = None;
      mode = ClientEnv.MODE_STATUS;
      no_load = true;
      save_64bit = None;
      save_human_readable_64bit_dep_map = None;
      output_json = false;
      prechecked = None;
      mini_state = None;
      root = Path.dummy_path;
      sort_results = true;
      stdin_name = None;
      deadline = None;
      watchman_debug_logging = false;
      allow_non_opt_build = true;
      desc = "testing";
    }

let test_check_success () : bool Lwt.t =
  let%lwt () =
    try_with_tmp (fun ~root ->
        ServerProgress.write ~include_in_logs:false "test1";
        ServerProgress.ErrorsWrite.new_empty_file
          ~clock:None
          ~ignore_hh_version:false
          ~cancel_reason;
        let env = { env with ClientEnv.root } in
        let connect = ref false in
        let connect_then_close () =
          connect := true;
          Lwt.return_unit
        in
        let partial_telemetry_ref = ref None in
        let check_future =
          ClientCheckStatus.go_streaming
            env
            ~partial_telemetry_ref
            ~connect_then_close
        in
        let%lwt () = Lwt_unix.sleep 1.0 in
        (* It should have picked up the errors-file, and seen that it's valid, and be just waiting *)
        assert (Lwt.is_sleeping check_future);
        (* Now we'll complete the errors file. This should make our check complete. *)
        ServerProgress.ErrorsWrite.complete
          (Telemetry.create () |> Telemetry.string_ ~key:"hot" ~value:"potato");
        let%lwt (exit_status, telemetry) = check_future in
        let exit_status = Exit_status.show exit_status in
        let telemetry = Telemetry.to_string telemetry in
        Printf.eprintf "%s\n%s\n%!" exit_status telemetry;
        String_asserter.assert_equals "Exit_status.No_error" exit_status "x";
        Bool_asserter.assert_equals false !connect "!connect";
        assert (String.is_substring telemetry ~substring:"\"hot\":\"potato\"");
        assert (String.is_substring telemetry ~substring:"\"streaming\":true");
        Lwt.return_unit)
  in
  Lwt.return_true

let test_check_errors () : bool Lwt.t =
  let%lwt () =
    try_with_tmp (fun ~root ->
        ServerProgress.write ~include_in_logs:false "test1";
        let env = { env with ClientEnv.root } in
        let connect_then_close () = Lwt.return_unit in
        let partial_telemetry_ref = ref None in
        let check_future =
          ClientCheckStatus.go_streaming
            env
            ~partial_telemetry_ref
            ~connect_then_close
        in
        let%lwt () = Lwt_unix.sleep 0.2 in
        ServerProgress.ErrorsWrite.new_empty_file
          ~clock:None
          ~ignore_hh_version:false
          ~cancel_reason;
        ServerProgress.ErrorsWrite.report (make_errors [("c", "oops")]);
        ServerProgress.ErrorsWrite.complete telemetry;
        let%lwt (exit_status, _telemetry) = check_future in
        let exit_status = Exit_status.show exit_status in
        String_asserter.assert_equals "Exit_status.Type_error" exit_status "x";
        Lwt.return_unit)
  in
  Lwt.return_true

let test_check_connect_success () : bool Lwt.t =
  let%lwt () =
    try_with_tmp (fun ~root ->
        ServerProgress.write ~include_in_logs:false "test1";
        let env = { env with ClientEnv.root } in
        let (future1, trigger1) = Lwt.wait () in
        let (future2, trigger2) = Lwt.wait () in
        let connect = ref "A" in
        let connect_then_close () =
          connect := "B";
          let%lwt () = future1 in
          ServerProgress.ErrorsWrite.new_empty_file
            ~clock:None
            ~ignore_hh_version:false
            ~cancel_reason;
          let%lwt () = future2 in
          Lwt.return_unit
        in
        let partial_telemetry_ref = ref None in

        let check_future =
          ClientCheckStatus.go_streaming
            env
            ~partial_telemetry_ref
            ~connect_then_close
        in
        let%lwt () = Lwt_unix.sleep 1.0 in
        (* It should have found errors.bin absent, and is waiting on the callback *)
        assert (Lwt.is_sleeping check_future);
        String_asserter.assert_equals "B" !connect "!connect";
        Lwt.wakeup_later trigger1 ();
        (* Even though we created an error file, it won't proceed until the callback's done *)
        let%lwt () = Lwt_unix.sleep 1.0 in
        assert (Lwt.is_sleeping check_future);
        Lwt.wakeup_later trigger2 ();
        (* Now the check will happily have found errors.bin and is waiting on it. *)
        let%lwt () = Lwt_unix.sleep 1.0 in
        assert (Lwt.is_sleeping check_future);
        (* Let us pretend that the server exit; this will trigger check to complete, and return exit status. *)
        ServerProgress.ErrorsWrite.unlink_at_server_stop ();
        let%lwt r =
          try%lwt
            let%lwt _ = check_future in
            Lwt.return_error "expected exception"
          with
          | Exit_status.(Exit_with Typecheck_abandoned) -> Lwt.return_ok ()
          | exn ->
            let e = Exception.wrap exn in
            Lwt.return_error (Exception.to_string e)
        in
        let () = Result.ok_or_failwith r in
        Lwt.return_unit)
  in
  Lwt.return_true

let test_check_connect_failure () : bool Lwt.t =
  let%lwt () =
    try_with_tmp (fun ~root ->
        ServerProgress.write ~include_in_logs:false "test1";
        let env = { env with ClientEnv.root } in
        (* This is an async failure, so the exception is stored in check_future
           rather than being returned from ClientCheckStatus.go itself *)
        let connect_then_close () =
          let%lwt () = Lwt_unix.sleep 0.1 in
          failwith "nostart"
        in
        let partial_telemetry_ref = ref None in
        let check_future =
          ClientCheckStatus.go_streaming
            env
            ~partial_telemetry_ref
            ~connect_then_close
        in
        let%lwt r =
          try%lwt
            let%lwt _ = check_future in
            Lwt.return_error "expected exception"
          with
          | exn
            when String.is_substring (Exn.to_string exn) ~substring:"nostart" ->
            Lwt.return_ok ()
          | exn ->
            let e = Exception.wrap exn in
            Lwt.return_error (Exception.to_string e)
        in
        let () = Result.ok_or_failwith r in
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
      ("test_check_success", (fun () -> Lwt_main.run (test_check_success ())));
      ("test_check_errors", (fun () -> Lwt_main.run (test_check_errors ())));
      ( "test_check_connect_success",
        (fun () -> Lwt_main.run (test_check_connect_success ())) );
      ( "test_check_connect_failure",
        (fun () -> Lwt_main.run (test_check_connect_failure ())) );
    ]
