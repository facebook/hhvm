open Asserter

let test_exception_pp () =
  let prefix = "Bang!!!" in
  let err_inst = Future.create_default_error_instance prefix in
  let gv ~timeout:_ = Error err_inst in
  let future = Future.make (gv, (fun _ -> true)) in
  try
    Future.get_exn future;
    failwith "Expected future to throw an exception"
  with
  | exn ->
    let e = Exception.wrap exn in
    Bool_asserter.assert_equals
      true
      (String.starts_with ~prefix @@ Exception.to_string e)
      "Expected error to be rendered correctly";
    true

let test_delayed_future () =
  let future = Future.delayed_value ~delays:3 "Delayed value" in
  Bool_asserter.assert_equals false (Future.is_ready future) "First delay";
  Bool_asserter.assert_equals false (Future.is_ready future) "Second delay";
  Bool_asserter.assert_equals false (Future.is_ready future) "Third delay";
  Bool_asserter.assert_equals
    true
    (Future.is_ready future)
    "After third delay should be ready";
  String_asserter.assert_equals
    "Delayed value"
    (Future.get_exn future)
    "retrieve delayed value";
  true

let test_continue_delayed_future_with () =
  let future = Future.delayed_value ~delays:1 "Delayed value" in
  let future = Future.continue_with future @@ fun result -> result in
  Bool_asserter.assert_equals false (Future.is_ready future) "Delay!";
  Bool_asserter.assert_equals
    true
    (Future.is_ready future)
    "After the delay, the continuation should execute";
  String_asserter.assert_equals
    "Delayed value"
    (Future.get_exn future)
    "The delayed value must match the expected";
  true

let test_exception_in_future_continuation () =
  let on_error_count = ref 0 in
  let make_future (i : int) : int Future.t =
    let future =
      if i mod 2 = 0 then
        Future.of_value 5
      else
        Future.continue_with (Future.of_value ()) @@ fun _ ->
        failwith "Does it fire?"
    in
    Future.on_error future @@ fun error ->
    ignore error;
    on_error_count := !on_error_count + 1
  in

  let successful_future = make_future 4 in

  Asserter.Bool_asserter.assert_equals
    true
    (Future.is_ready successful_future)
    "Successful future must be ready";

  Asserter.Int_asserter.assert_equals
    0
    !on_error_count
    "On error shouldn't have been called";

  let () =
    match Future.get successful_future with
    | Ok result -> Asserter.Int_asserter.assert_equals 5 result "must be 5"
    | Error error -> failwith (Future.error_to_string error)
  in
  let failing_future = make_future 3 in

  Asserter.Bool_asserter.assert_equals
    true
    (Future.is_ready failing_future)
    "Failing future must be ready";

  Asserter.Int_asserter.assert_equals
    1
    !on_error_count
    "On error should have been called";

  let () =
    match Future.get failing_future with
    | Ok _result -> failwith "Expected a failure"
    | Error error ->
      Asserter.String_asserter.assert_equals
        "Continuation_raised(Failure(\"Does it fire?\"))"
        (Future.error_to_string error)
        "expecting the correct error"
  in
  true

let test_of_error () =
  let on_error_count = ref 0 in
  let make_future (i : int) : int Future.t =
    let future =
      if i mod 2 = 0 then
        Future.of_value 5
      else
        Future.of_error "Does it fail?"
    in
    Future.on_error future @@ fun error ->
    ignore error;
    on_error_count := !on_error_count + 1
  in
  let () =
    match Future.get (make_future 4) with
    | Ok result -> Asserter.Int_asserter.assert_equals 5 result "must be 5"
    | Error error -> failwith (Future.error_to_string error)
  in
  Asserter.Int_asserter.assert_equals
    0
    !on_error_count
    "On error shouldn't have been called";
  let () =
    match Future.get (make_future 3) with
    | Ok _result -> failwith "Expected a failure"
    | Error error ->
      Asserter.String_asserter.assert_equals
        "Continuation_raised(Failure(\"Does it fail?\"))"
        (Future.error_to_string error)
        "expecting the correct error"
  in
  Asserter.Int_asserter.assert_equals
    1
    !on_error_count
    "On error should have been called";
  true

(* This test verifies that all continuations and result handlers are called
    exactly once when calling `Future`'s `is_ready` and `get` functions. *)
let test_bound_future_idempotency () =
  let initial_future_id = ref 0 in
  let initial_future () =
    Printf.printf "initial_future future\n";
    initial_future_id := !initial_future_id + 1;
    Future.of_value "initial_future"
  in

  let next_future_id = ref 0 in
  let next_future previous_result =
    Printf.printf "next future: %s\n" previous_result;
    next_future_id := !next_future_id + 1;
    Future.of_value "next_future"
  in

  let penultimate_future_id = ref 0 in
  let penultimate_future previous_result =
    Printf.printf "penultimate future: %s\n" previous_result;
    penultimate_future_id := !penultimate_future_id + 1;
    Future.of_value "penultimate_future"
  in

  let ultimate_future_id = ref 0 in
  let ultimate_future =
    Future.continue_with_future (initial_future ()) @@ fun result ->
    Printf.printf "Continue with next future; result: %s\n" result;
    let penultimate_future =
      Future.continue_with_future (next_future result) @@ fun result ->
      Printf.printf "Continue with penultimate future; result: %s\n" result;
      penultimate_future result
    in
    Future.continue_with penultimate_future @@ fun result ->
    ultimate_future_id := !ultimate_future_id + 1;
    Printf.printf "continue_with: %s\n" result;
    "ultimate_future"
  in

  let is_future_ready = Future.is_ready ultimate_future in
  Bool_asserter.assert_equals
    true
    is_future_ready
    "The future should be ready immediately";

  let get_and_assert_value future ~expected =
    match Future.get future with
    | Ok actual ->
      String_asserter.assert_equals
        expected
        actual
        "The actual value must match the expected"
    | Error error ->
      failwith (Printf.sprintf "Error! %s\n" (Future.error_to_string error))
  in

  get_and_assert_value ultimate_future ~expected:"ultimate_future";

  (* Now, try getting the value out of the future a second time *)
  get_and_assert_value ultimate_future ~expected:"ultimate_future";

  let rec verify_ids ids =
    match ids with
    | [] -> ()
    | (id, name) :: rest ->
      Int_asserter.assert_equals
        1
        id
        (Printf.sprintf "'%s' should only be called once!" name);

      verify_ids rest
  in

  verify_ids
    [
      (!initial_future_id, "initial_future");
      (!next_future_id, "next_future");
      (!penultimate_future_id, "penultimate_future");
      (!ultimate_future_id, "ultimate_future");
    ];

  true

(** Verify that cleanup really reaps the child instead of leaving a zombie. *)
let assert_reaped pid =
  try
    let (_ : int * Unix.process_status) = Unix.waitpid [Unix.WNOHANG] pid in
    failwith "Process was not reaped"
  with
  | Unix.Unix_error (Unix.ECHILD, _, _) -> ()

(** Verify that descriptors cannot leak when a cancelled process is cleaned up. *)
let assert_descriptors_closed process =
  List.iter
    (fun fd_ref ->
      match !fd_ref with
      | None -> ()
      | Some _ -> failwith "Process descriptor was not closed")
    [
      process.Process_types.stdin_fd;
      process.Process_types.stdout_fd;
      process.Process_types.stderr_fd;
    ]

(** Cancellation stops and reaps a direct child, closes its descriptors, and
    leaves a terminal lifecycle so a repeated cleanup cannot signal its PID.
    A child that ignores SIGTERM must be stopped by the SIGKILL fallback;
    the requested grace period must elapse first. Omitting the graceful signal
    must kill the child directly. *)
let test_process_termination ~graceful_signal ~grace_period ~ignore_sigterm () =
  (* Ignored signals survive exec, avoiding a race with child initialization. *)
  let previous_handler =
    Sys.signal
      Sys.sigterm
      (if ignore_sigterm then
        Sys.Signal_ignore
      else
        Sys.Signal_default)
  in
  let process =
    Utils.try_finally
      ~f:(fun () ->
        Process.exec (Exec_command.For_use_in_testing_only "sleep") ["600"])
      ~finally:(fun () -> Sys.set_signal Sys.sigterm previous_handler)
  in
  let expected_signal =
    match graceful_signal with
    | Some signal when not ignore_sigterm -> signal
    | Some _
    | None ->
      Sys.sigkill
  in
  Utils.try_finally
    ~f:(fun () ->
      let pid =
        match !(process.Process_types.lifecycle) with
        | Process_types.Lifecycle_running { pid } -> pid
        | Process_types.Lifecycle_exited _
        | Process_types.Lifecycle_killed_due_to_overflow_stdin ->
          failwith "Expected a running child"
      in
      let started_at = Unix.gettimeofday () in
      Process.terminate ~graceful_signal ~grace_period process;
      if ignore_sigterm && Option.is_some graceful_signal then
        assert (Unix.gettimeofday () -. started_at >= grace_period);
      Process.terminate ~graceful_signal ~grace_period process;
      assert_reaped pid;
      assert_descriptors_closed process;
      match !(process.Process_types.lifecycle) with
      | Process_types.Lifecycle_exited (Unix.WSIGNALED signal)
        when signal = expected_signal ->
        ()
      | Process_types.Lifecycle_exited _
      | Process_types.Lifecycle_running _
      | Process_types.Lifecycle_killed_due_to_overflow_stdin ->
        failwith
          ("Expected termination by " ^ Sys_utils.name_of_signal expected_signal))
    ~finally:(fun () ->
      Process.terminate ~graceful_signal:None ~grace_period:0.0 process);
  true

(** A failed preliminary signal must not prevent SIGKILL, reaping, or descriptor
    cleanup, and its error must propagate after those steps finish. *)
let test_process_termination_after_signal_failure () =
  let process =
    Process.exec (Exec_command.For_use_in_testing_only "sleep") ["600"]
  in
  Utils.try_finally
    ~f:(fun () ->
      let pid =
        match !(process.Process_types.lifecycle) with
        | Process_types.Lifecycle_running { pid } -> pid
        | Process_types.Lifecycle_exited _
        | Process_types.Lifecycle_killed_due_to_overflow_stdin ->
          failwith "Expected a running child"
      in
      (* An invalid signal fails while the child is still running. *)
      let raised =
        try
          Process.terminate
            ~graceful_signal:(Some 1024)
            ~grace_period:1.0
            process;
          false
        with
        | Unix.Unix_error (Unix.EINVAL, _, _) -> true
      in
      assert raised;
      assert_reaped pid;
      assert_descriptors_closed process;
      match !(process.Process_types.lifecycle) with
      | Process_types.Lifecycle_exited (Unix.WSIGNALED signal)
        when signal = Sys.sigkill ->
        ()
      | Process_types.Lifecycle_exited _
      | Process_types.Lifecycle_running _
      | Process_types.Lifecycle_killed_due_to_overflow_stdin ->
        failwith "Expected the SIGKILL step to run after the signal failure")
    ~finally:(fun () ->
      Process.terminate ~graceful_signal:None ~grace_period:0.0 process);
  true

(** Cleanup after normal completion preserves the command's result and closes
    descriptors without trying to signal an already-reaped PID. *)
let test_process_completed_cleanup () =
  let process =
    Process.exec (Exec_command.For_use_in_testing_only "printf") ["finished"]
  in
  Utils.try_finally
    ~f:(fun () ->
      let deadline = Unix.gettimeofday () +. 10.0 in
      let rec wait () =
        if not (Process.is_ready process) then (
          if Unix.gettimeofday () > deadline then
            failwith "Process fixture timed out";
          let (_
                : Unix.file_descr list
                  * Unix.file_descr list
                  * Unix.file_descr list) =
            Unix.select [] [] [] 0.01
          in
          wait ()
        )
      in
      wait ();
      let result = Process.read_and_wait_pid ~timeout:0 process in
      (match result with
      | Ok { Process_types.stdout; _ } ->
        String_asserter.assert_equals "finished" stdout "Command output"
      | Error failure -> failwith (Process.failure_msg failure));
      Process.terminate
        ~graceful_signal:(Some Sys.sigterm)
        ~grace_period:1.0
        process;
      Process.terminate ~graceful_signal:None ~grace_period:0.0 process;
      assert_descriptors_closed process;
      match !(process.Process_types.lifecycle) with
      | Process_types.Lifecycle_exited (Unix.WEXITED 0) -> ()
      | Process_types.Lifecycle_exited _
      | Process_types.Lifecycle_running _
      | Process_types.Lifecycle_killed_due_to_overflow_stdin ->
        failwith "Expected successful completion")
    ~finally:(fun () ->
      Process.terminate ~graceful_signal:None ~grace_period:0.0 process);
  true

let tests =
  [
    ("test_exception_pp", test_exception_pp);
    ("test_delayed_future", test_delayed_future);
    ("test_continue_delayed_future_with", test_continue_delayed_future_with);
    ("test_of_error", test_of_error);
    ( "test_exception_in_future_continuation",
      test_exception_in_future_continuation );
    ("test_bound_future_idempotency", test_bound_future_idempotency);
    ( "test_process_termination",
      test_process_termination
        ~graceful_signal:(Some Sys.sigterm)
        ~grace_period:1.0
        ~ignore_sigterm:false );
    ( "test_process_termination_fallback",
      test_process_termination
        ~graceful_signal:(Some Sys.sigterm)
        ~grace_period:2.0
        ~ignore_sigterm:true );
    ( "test_process_termination_without_signal",
      test_process_termination
        ~graceful_signal:None
        ~grace_period:0.0
        ~ignore_sigterm:false );
    ( "test_process_termination_after_signal_failure",
      test_process_termination_after_signal_failure );
    ("test_process_completed_cleanup", test_process_completed_cleanup);
  ]

let () =
  Daemon.check_entry_point ();
  Unit_test.run_all tests
