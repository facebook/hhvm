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
  let actual = Server_progress.ErrorsWrite.get_state_FOR_TEST () in
  String_asserter.assert_equals expected actual "expect_state"

let telemetry = Telemetry.create ()

let cancel_reason = ("test", "")

let root = Path.make "test"

let try_with_tmp (f : root:Path.t -> unit Lwt.t) : unit Lwt.t =
  let tmp = Tempfile.mkdtemp ~skip_mocking:true in
  Lwt_utils.try_finally
    ~f:(fun () ->
      (* We want ServerFiles.errors_file to be placed in this test's temp directory *)
      ServerFiles.set_tmp_FOR_TESTING_ONLY tmp;
      (* We need Server_progress.Errors to have a root (any root) so it knows how to name the errors file *)
      Server_progress.set_root root;
      Relative_path.set_path_prefix Relative_path.Root root;
      (* To isolate tests, we have to reset Server_progress.Errors global state in memory. *)
      Server_progress.ErrorsWrite.unlink_at_server_stop ();
      let%lwt () = f ~root in
      Lwt.return_unit)
    ~finally:(fun () ->
      Sys_utils.rm_dir_tree ~skip_mocking:true (Path.to_string tmp);
      Lwt.return_unit)

type simple_error = int * string * string [@@deriving ord, eq, show]

type simple_error_list = simple_error list [@@deriving show]

let make_errors (errors : (int * string * string) list) : Errors.t =
  let errors =
    List.map errors ~f:(fun (code, rel_path, message) ->
        let path = Relative_path.from_root ~suffix:rel_path in
        let error =
          User_error.make_err
            code
            (Pos.make_from path, message)
            []
            Explanation.empty
        in
        (path, error))
  in
  Errors.from_file_error_list errors

let an_error : Errors.t = make_errors [(101, "c", "oops")]

let test_completed () : bool Lwt.t =
  let%lwt () =
    try_with_tmp (fun ~root ->
        let errors_file_path = ServerFiles.errors_file_path root in
        Server_progress.ErrorsWrite.new_empty_file
          ~clock:None
          ~ignore_hh_version:false
          ~cancel_reason;
        Server_progress.ErrorsWrite.report
          (make_errors
             [(101, "a", "hello"); (101, "a", "there"); (101, "b", "world")]);
        Server_progress.ErrorsWrite.report (make_errors [(101, "c", "oops")]);
        Server_progress.ErrorsWrite.complete telemetry;
        let fd = Unix.openfile errors_file_path [Unix.O_RDONLY] 0 in
        assert (Server_progress.ErrorsRead.openfile fd |> Result.is_ok);
        String_asserter.assert_equals
          "Errors [a=2,b=1]"
          (Server_progress.ErrorsRead.read_next_errors fd |> show_read)
          "errors";
        String_asserter.assert_equals
          "Errors [c=1]"
          (Server_progress.ErrorsRead.read_next_errors fd |> show_read)
          "errors";
        String_asserter.assert_equals
          "typecheck completed [complete]"
          (Server_progress.ErrorsRead.read_next_errors fd |> show_read)
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
          "nothing yet"
          (Server_progress.ErrorsRead.read_next_errors fd |> show_read)
          "killed";
        Unix.close fd;
        Lwt.return_unit)
  in
  Lwt.return_true

let test_read_unlinked () : bool Lwt.t =
  let%lwt () =
    try_with_tmp (fun ~root ->
        let errors_file_path = ServerFiles.errors_file_path root in
        Server_progress.ErrorsWrite.new_empty_file
          ~clock:None
          ~ignore_hh_version:false
          ~cancel_reason;
        let fd = Unix.openfile errors_file_path [Unix.O_RDONLY] 0 in
        Server_progress.ErrorsWrite.unlink_at_server_stop ();
        assert (Server_progress.ErrorsRead.openfile fd |> Result.is_ok);
        String_asserter.assert_equals
          "hh_server gracefully stopped [unlink]"
          (Server_progress.ErrorsRead.read_next_errors fd |> show_read)
          "end";
        Unix.close fd;
        Lwt.return_unit)
  in
  Lwt.return_true

let test_read_unlinked_empty () : bool Lwt.t =
  let%lwt () =
    try_with_tmp (fun ~root ->
        let errors_file_path = ServerFiles.errors_file_path root in
        Server_progress.ErrorsWrite.new_empty_file
          ~clock:None
          ~ignore_hh_version:false
          ~cancel_reason;
        let fd = Unix.openfile errors_file_path [Unix.O_RDONLY] 0 in
        assert (Server_progress.ErrorsRead.openfile fd |> Result.is_ok);
        Server_progress.ErrorsWrite.unlink_at_server_stop ();
        String_asserter.assert_equals
          "hh_server gracefully stopped [unlink]"
          (Server_progress.ErrorsRead.read_next_errors fd |> show_read)
          "end";
        Unix.close fd;
        Lwt.return_unit)
  in
  Lwt.return_true

let test_read_restarted () : bool Lwt.t =
  let%lwt () =
    try_with_tmp (fun ~root ->
        let errors_file_path = ServerFiles.errors_file_path root in
        Server_progress.ErrorsWrite.new_empty_file
          ~clock:None
          ~ignore_hh_version:false
          ~cancel_reason;
        Server_progress.ErrorsWrite.report an_error;
        let fd = Unix.openfile errors_file_path [Unix.O_RDONLY] 0 in
        Server_progress.ErrorsWrite.new_empty_file
          ~clock:None
          ~ignore_hh_version:false
          ~cancel_reason;
        assert (Server_progress.ErrorsRead.openfile fd |> Result.is_ok);
        String_asserter.assert_equals
          "Errors [c=1]"
          (Server_progress.ErrorsRead.read_next_errors fd |> show_read)
          "errors";
        String_asserter.assert_equals
          "typecheck restarted due to file changes [new_empty_file]"
          (Server_progress.ErrorsRead.read_next_errors fd |> show_read)
          "end";
        Unix.close fd;
        Lwt.return_unit)
  in
  Lwt.return_true

let test_locks () : bool Lwt.t =
  (* This is just too irritating to test, so I'm not going to...
     The Server_progress.Errors functions all wait synchronously until they can acquire a lock,
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
          "malformed error"
          (Server_progress.ErrorsRead.read_next_errors fd |> show_read)
          "half";
        Unix.close fd;
        Lwt.return_unit)
  in
  Lwt.return_true

let test_read_dead_pid () : bool Lwt.t =
  let%lwt () =
    try_with_tmp (fun ~root ->
        Server_progress.ErrorsWrite.create_file_FOR_TEST ~pid:1 ~cmdline:"bogus";
        let errors_file_path = ServerFiles.errors_file_path root in
        let fd = Unix.openfile errors_file_path [Unix.O_RDONLY] 0 in
        begin
          match Server_progress.ErrorsRead.openfile fd with
          | Error
              ( Server_progress.ErrorsRead.OKilled _,
                "Errors-file is from defunct PID" ) ->
            ()
          | Ok _ -> failwith "Expected a dead-pid failure, not success"
          | Error (e, msg) ->
            failwith
              ("Expected OKilled, not ("
              ^ Server_progress.ErrorsRead.show_open_error e
              ^ ", "
              ^ msg
              ^ ")")
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
        expect_exn (fun () -> Server_progress.ErrorsWrite.report an_error);
        expect_state "Absent";
        expect_exn (fun () -> Server_progress.ErrorsWrite.complete telemetry);
        expect_state "Absent";
        Server_progress.ErrorsWrite.unlink_at_server_stop ();
        expect_state "Absent";
        Server_progress.ErrorsWrite.new_empty_file
          ~clock:None
          ~ignore_hh_version:false
          ~cancel_reason;
        expect_state "Reporting[0]";
        Lwt.return_unit)
  in
  let%lwt () =
    try_with_tmp (fun ~root:_ ->
        (* Actions from state "Reporting[0]"... *)
        Server_progress.ErrorsWrite.new_empty_file
          ~clock:None
          ~ignore_hh_version:false
          ~cancel_reason;
        expect_state "Reporting[0]";
        Server_progress.ErrorsWrite.new_empty_file
          ~clock:None
          ~ignore_hh_version:false
          ~cancel_reason;
        expect_state "Reporting[0]";
        Server_progress.ErrorsWrite.report an_error;
        expect_state "Reporting[1]";
        Server_progress.ErrorsWrite.new_empty_file
          ~clock:None
          ~ignore_hh_version:false
          ~cancel_reason;
        Server_progress.ErrorsWrite.unlink_at_server_stop ();
        expect_state "Absent";
        Server_progress.ErrorsWrite.new_empty_file
          ~clock:None
          ~ignore_hh_version:false
          ~cancel_reason;
        expect_state "Reporting[0]";
        Server_progress.ErrorsWrite.complete telemetry;
        expect_state "Closed";
        Lwt.return_unit)
  in
  let%lwt () =
    try_with_tmp (fun ~root:_ ->
        (* Actions from state "Reporting[1]" *)
        Server_progress.ErrorsWrite.new_empty_file
          ~clock:None
          ~ignore_hh_version:false
          ~cancel_reason;
        Server_progress.ErrorsWrite.report an_error;
        expect_state "Reporting[1]";
        Server_progress.ErrorsWrite.new_empty_file
          ~clock:None
          ~ignore_hh_version:false
          ~cancel_reason;
        expect_state "Reporting[0]";
        Server_progress.ErrorsWrite.report an_error;
        expect_state "Reporting[1]";
        Server_progress.ErrorsWrite.report an_error;
        expect_state "Reporting[2]";
        Server_progress.ErrorsWrite.complete telemetry;
        expect_state "Closed";
        Server_progress.ErrorsWrite.new_empty_file
          ~clock:None
          ~ignore_hh_version:false
          ~cancel_reason;
        Server_progress.ErrorsWrite.report an_error;
        expect_state "Reporting[1]";
        Server_progress.ErrorsWrite.unlink_at_server_stop ();
        expect_state "Absent";
        Lwt.return_unit)
  in
  let%lwt () =
    try_with_tmp (fun ~root:_ ->
        (* Actions from state "Closed" *)
        Server_progress.ErrorsWrite.new_empty_file
          ~clock:None
          ~ignore_hh_version:false
          ~cancel_reason;
        Server_progress.ErrorsWrite.complete telemetry;
        expect_state "Closed";
        Server_progress.ErrorsWrite.new_empty_file
          ~clock:None
          ~ignore_hh_version:false
          ~cancel_reason;
        expect_state "Reporting[0]";
        Server_progress.ErrorsWrite.complete telemetry;
        expect_state "Closed";
        expect_exn (fun () -> Server_progress.ErrorsWrite.report an_error);
        expect_state "Closed";
        expect_exn (fun () -> Server_progress.ErrorsWrite.complete telemetry);
        expect_state "Closed";
        Server_progress.ErrorsWrite.unlink_at_server_stop ();
        expect_state "Absent";
        Lwt.return_unit)
  in
  Lwt.return_true

let test_async_read_completed () : bool Lwt.t =
  let%lwt () =
    try_with_tmp (fun ~root ->
        let errors_file_path = ServerFiles.errors_file_path root in
        Server_progress.ErrorsWrite.new_empty_file
          ~clock:None
          ~ignore_hh_version:false
          ~cancel_reason;
        Server_progress.ErrorsWrite.report
          (make_errors
             [(101, "a", "hello"); (101, "a", "there"); (101, "b", "world")]);
        Server_progress.ErrorsWrite.report (make_errors [(101, "c", "oops")]);
        Server_progress.ErrorsWrite.complete telemetry;
        let fd = Unix.openfile errors_file_path [Unix.O_RDONLY] 0 in
        let _open = Server_progress.ErrorsRead.openfile fd in
        let q = Server_progress_lwt.watch_errors_file ~pid fd in
        let%lwt () = expect_qitem q "Errors [a=2,b=1]" in
        let%lwt () = expect_qitem q "Errors [c=1]" in
        let%lwt () = expect_qitem q "typecheck completed [complete]" in
        let%lwt () = expect_qitem q "closed" in
        Lwt.return_unit)
  in
  Lwt.return_true

let test_async_read_partial () : bool Lwt.t =
  let%lwt () =
    try_with_tmp (fun ~root ->
        let errors_file_path = ServerFiles.errors_file_path root in
        Server_progress.ErrorsWrite.new_empty_file
          ~clock:None
          ~ignore_hh_version:false
          ~cancel_reason;
        let fd = Unix.openfile errors_file_path [Unix.O_RDONLY] 0 in
        let _open = Server_progress.ErrorsRead.openfile fd in
        let q = Server_progress_lwt.watch_errors_file ~pid fd in
        (* initially there are no items available *)
        let%lwt () = expect_qitem q "nothing" in
        (* we'll put in one report, and after this there should be exactly one item available *)
        Server_progress.ErrorsWrite.report
          (make_errors
             [(101, "a", "hello"); (101, "a", "there"); (101, "b", "world")]);
        let%lwt () = expect_qitem q "Errors [a=2,b=1]" in
        let%lwt () = expect_qitem q "nothing" in
        (* we'll complete the file, and after this the stream should be closed *)
        Server_progress.ErrorsWrite.complete (Telemetry.create ());
        let%lwt () = expect_qitem q "typecheck completed [complete]" in
        let%lwt () = expect_qitem q "closed" in
        Lwt.return_unit)
  in
  Lwt.return_true

let test_async_read_unlinked () : bool Lwt.t =
  let%lwt () =
    try_with_tmp (fun ~root ->
        let errors_file_path = ServerFiles.errors_file_path root in
        Server_progress.ErrorsWrite.new_empty_file
          ~clock:(Some (ServerNotifier.Watchman "clock123"))
          ~ignore_hh_version:false
          ~cancel_reason;
        let fd = Unix.openfile errors_file_path [Unix.O_RDONLY] 0 in
        let open_result = Server_progress.ErrorsRead.openfile fd in
        let { Server_progress.ErrorsRead.pid; clock; _ } =
          Result.ok open_result |> Option.value_exn
        in
        assert (
          Option.equal
            ServerNotifier.equal_clock
            (Some (ServerNotifier.Watchman "clock123"))
            clock);
        assert (pid = Unix.getpid ());
        let q = Server_progress_lwt.watch_errors_file ~pid fd in
        (* we'll unlink the file, and after this the stream should be closed *)
        Server_progress.ErrorsWrite.unlink_at_server_stop ();
        let%lwt () = expect_qitem q "hh_server gracefully stopped [unlink]" in
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
            let _ = Server_progress.ErrorsRead.openfile fd in
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
        Server_progress.ErrorsWrite.new_empty_file
          ~clock:None
          ~ignore_hh_version:false
          ~cancel_reason;
        (* we'll write an incomplete file *)
        let preamble = Marshal_tools.make_preamble 100 in
        Sys_utils.append_file ~file:errors_file_path (Bytes.to_string preamble);
        let fd = Unix.openfile errors_file_path [Unix.O_RDONLY] 0 in
        let { Server_progress.ErrorsRead.pid; _ } =
          Server_progress.ErrorsRead.openfile fd
          |> Result.ok
          |> Option.value_exn
        in
        let q = Server_progress_lwt.watch_errors_file ~pid fd in
        (* because the file is incomplete, the queue should assume that the
           creating process was killed *)
        let%lwt () = expect_qitem q "malformed error" in
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
        Server_progress.ErrorsWrite.new_empty_file
          ~clock:None
          ~ignore_hh_version:false
          ~cancel_reason;
        let fd = Unix.openfile errors_file_path [Unix.O_RDONLY] 0 in
        let _open = Server_progress.ErrorsRead.openfile fd in
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
        let%lwt () = expect_qitem ~delay:60.0 q "hh_server died" in
        let%lwt () = expect_qitem q "closed" in
        Lwt.return_unit)
  in
  Lwt.return_true

let test_async_read_start () : bool Lwt.t =
  let%lwt () =
    try_with_tmp (fun ~root ->
        let errors_file_path = ServerFiles.errors_file_path root in
        let pid = Unix.getpid () in
        Server_progress.ErrorsWrite.new_empty_file
          ~clock:None
          ~ignore_hh_version:false
          ~cancel_reason;
        let fd = Unix.openfile errors_file_path [Unix.O_RDONLY] 0 in
        (* oops! we didn't call Server_progress.ErrorsRead.openfile *)
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
      error_format = Some Errors.Plain;
      force_dormant_start = false;
      from = "test";
      show_spinner = false;
      gen_saved_ignore_type_errors = false;
      ignore_hh_version = true;
      saved_state_ignore_hhconfig = true;
      paths = [];
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
      preexisting_warnings = false;
      is_interactive = false;
      warning_switches = [];
      dump_config = false;
      find_my_tests_max_distance = 1;
    }

let make_error_filter env =
  Filter_errors.Filter.make
    ~default_all:true
    ~generated_files:[]
    env.ClientEnv.warning_switches

let test_check_success () : bool Lwt.t =
  let%lwt () =
    try_with_tmp (fun ~root ->
        Server_progress.write ~include_in_logs:false "test1";
        Server_progress.ErrorsWrite.new_empty_file
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
            ServerLocalConfigLoad.default
            (make_error_filter env)
            ~partial_telemetry_ref
            ~connect_then_close
        in
        let%lwt () = Lwt_unix.sleep 1.0 in
        (* It should have picked up the errors-file, and seen that it's valid, and be just waiting *)
        assert (Lwt.is_sleeping check_future);
        (* Now we'll complete the errors file. This should make our check complete. *)
        Server_progress.ErrorsWrite.complete
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
        Server_progress.write ~include_in_logs:false "test1";
        let env = { env with ClientEnv.root } in
        let connect_then_close () = Lwt.return_unit in
        let partial_telemetry_ref = ref None in
        let check_future =
          ClientCheckStatus.go_streaming
            env
            ServerLocalConfigLoad.default
            (make_error_filter env)
            ~partial_telemetry_ref
            ~connect_then_close
        in
        let%lwt () = Lwt_unix.sleep 0.2 in
        Server_progress.ErrorsWrite.new_empty_file
          ~clock:None
          ~ignore_hh_version:false
          ~cancel_reason;
        Server_progress.ErrorsWrite.report (make_errors [(101, "c", "oops")]);
        Server_progress.ErrorsWrite.complete telemetry;
        let%lwt (exit_status, _telemetry) = check_future in
        let exit_status = Exit_status.show exit_status in
        String_asserter.assert_equals "Exit_status.Type_error" exit_status "x";
        Lwt.return_unit)
  in
  Lwt.return_true

let test_check_connect_success () : bool Lwt.t =
  let%lwt () =
    try_with_tmp (fun ~root ->
        Server_progress.write ~include_in_logs:false "test1";
        let env = { env with ClientEnv.root } in
        let (future1, trigger1) = Lwt.wait () in
        let (future2, trigger2) = Lwt.wait () in
        let connect = ref "A" in
        let connect_then_close () =
          connect := "B";
          let%lwt () = future1 in
          Server_progress.ErrorsWrite.new_empty_file
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
            ServerLocalConfigLoad.default
            (make_error_filter env)
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
        Server_progress.ErrorsWrite.unlink_at_server_stop ();
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
        Server_progress.write ~include_in_logs:false "test1";
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
            ServerLocalConfigLoad.default
            (make_error_filter env)
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

let assert_errors ~expected ~(actual : Errors.finalized_error list) =
  let expected = List.sort expected ~compare:compare_simple_error in
  let actual =
    List.fold actual ~init:[] ~f:(fun acc err ->
        ( User_error.get_code err,
          User_error.get_pos err
          |> Pos.filename
          |> String.chop_prefix_exn ~prefix:(Path.to_string root ^ "/"),
          User_error.get_messages err |> List.hd_exn |> snd )
        :: acc)
    |> List.sort ~compare:compare_simple_error
  in
  if not @@ List.equal equal_simple_error expected actual then (
    Printf.eprintf
      "Expected \n%s\nbut got\n%s"
      (show_simple_error_list expected)
      (show_simple_error_list actual);
    assert false
  )

let test_filter_warnings () : bool =
  let error_filter =
    Filter_errors.Filter.make
      ~default_all:true
      ~generated_files:[Str.regexp "gen/"]
      [
        Filter_errors.Code_off Error_codes.Warning.SketchyEquality;
        Filter_errors.Ignored_files (Str.regexp "def");
        Filter_errors.Code_off Error_codes.Warning.SketchyNullCheck;
        Filter_errors.Code_on Error_codes.Warning.SketchyEquality;
        Filter_errors.Ignored_files (Str.regexp "abc");
      ]
  in
  let errors =
    make_errors
      [
        (12001, "a", "SketchyEquality in non-ignored file. Show");
        (12003, "a", "SketchyNullCheck in non-ignored file. Hide");
        (12001, "defgh", "SketchyEquality in ignored file. Hide");
        (12004, "abcd", "other warning in ignored file. Hide");
        (4110, "abc", "non-warning in ignored file. Show");
        (4110, "gen/", "non-warning in generated file. Show");
        (12004, "gen/", "other warning in generated file. Hide");
      ]
    |> Errors.sort_and_finalize
  in
  let actual = Filter_errors.filter error_filter errors in
  let expected =
    [
      (12001, "a", "SketchyEquality in non-ignored file. Show");
      (4110, "abc", "non-warning in ignored file. Show");
      (4110, "gen/", "non-warning in generated file. Show");
    ]
  in
  assert_errors ~expected ~actual;
  true

let test_filter_warnings_generated () : bool =
  let error_filter =
    Filter_errors.Filter.make
      ~default_all:true
      ~generated_files:[Str.regexp "gen/"; Str.regexp "gen2"]
      [
        Filter_errors.Ignored_files (Str.regexp "def");
        Filter_errors.Ignored_files (Str.regexp "gen2");
        Filter_errors.Generated_files_on;
      ]
  in
  let errors =
    make_errors
      [
        (12001, "defgh", "warning in ignored file. Hide");
        (12004, "abcd", "unrelated. show");
        (4110, "gen2/", "non-warning in ignored file. Show");
        (12004, "gen/", "warning in generated file with -Wgenerated. Show");
        ( 12004,
          "gen2/",
          "warning in generated file with -Wgenerated but ignored. Hide" );
      ]
    |> Errors.sort_and_finalize
  in
  let actual = Filter_errors.filter error_filter errors in
  let expected =
    [
      (12004, "abcd", "unrelated. show");
      (4110, "gen2/", "non-warning in ignored file. Show");
      (12004, "gen/", "warning in generated file with -Wgenerated. Show");
    ]
  in
  assert_errors ~expected ~actual;
  true

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
      ("test_filter_warnings", test_filter_warnings);
      ("test_filter_warnings_generated", test_filter_warnings_generated);
    ]
