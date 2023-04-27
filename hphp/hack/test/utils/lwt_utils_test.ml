open Hh_prelude
open Asserter

let test_exec_checked_basic () : bool Lwt.t =
  let%lwt process_status =
    Lwt_utils.exec_checked
      (Exec_command.For_use_in_testing_only "echo")
      [| "hello"; "world" |]
  in
  match process_status with
  | Ok { Lwt_utils.Process_success.command_line; stdout; stderr; _ } ->
    String_asserter.assert_equals
      command_line
      "echo hello world"
      "incorrect command line";
    String_asserter.assert_equals stdout "hello world\n" "incorrect stdout";
    String_asserter.assert_equals stderr "" "incorrect stderr";
    Lwt.return true
  | Error _ -> failwith "command failed"

let test_exec_checked_failing () : bool Lwt.t =
  let%lwt process_status =
    Lwt_utils.exec_checked
      (Exec_command.For_use_in_testing_only "false")
      [| "ignored-argument" |]
  in
  match process_status with
  | Ok _ -> failwith "command succeeded unexpectedly"
  | Error
      {
        Lwt_utils.Process_failure.command_line;
        process_status;
        stdout;
        stderr;
        exn;
        _;
      } ->
    assert (Option.is_none exn);
    String_asserter.assert_equals
      command_line
      "false ignored-argument"
      "incorrect command line";
    Process_status_asserter.assert_equals
      process_status
      (Unix.WEXITED 1)
      "incorrect process status";
    String_asserter.assert_equals stdout "" "incorrect stdout";
    String_asserter.assert_equals stderr "" "incorrect stderr";
    Lwt.return true

let test_exec_checked_timeout () : bool Lwt.t =
  let%lwt process_status =
    Lwt_utils.exec_checked
      ~timeout:1.0
      Exec_command.Shell
      [| "echo A; sleep 4; echo B" |]
  in
  match process_status with
  | Ok _ -> failwith "command succeeded even though it should have timed out"
  | Error { Lwt_utils.Process_failure.process_status; stdout; exn; _ } ->
    Process_status_asserter.assert_equals
      process_status
      (Unix.WSIGNALED (-7))
      "process_status";
    assert (Option.is_some exn);
    String_asserter.assert_equals "" stdout "stdout";
    Lwt.return_true

let test_exec_checked_cancel_before () : bool Lwt.t =
  let cancel = Lwt.return_unit in
  let%lwt process_status =
    Lwt_utils.exec_checked
      Exec_command.Shell
      [| "echo A; sleep 4; echo B" |]
      ~cancel
  in
  match process_status with
  | Ok _ ->
    failwith "command succeeded even though it should have been cancelled"
  | Error { Lwt_utils.Process_failure.process_status; stdout; _ } ->
    Process_status_asserter.assert_equals
      process_status
      (Unix.WSIGNALED (-7))
      "process_status";
    String_asserter.assert_equals "" stdout "stdout";
    Lwt.return_true

let test_exec_checked_cancel_during () : bool Lwt.t =
  let (cancel, cancel_source) = Lwt.wait () in
  let _ =
    Lwt.bind (Lwt_unix.sleep 1.0) (fun () ->
        Lwt.wakeup_later cancel_source ();
        Lwt.return_unit)
  in
  let%lwt process_status =
    Lwt_utils.exec_checked
      Exec_command.Shell
      [| "echo A; sleep 4; echo B" |]
      ~cancel
  in
  match process_status with
  | Ok _ ->
    failwith "command succeeded even though it should have been cancelled"
  | Error { Lwt_utils.Process_failure.process_status; stdout; exn; _ } ->
    Process_status_asserter.assert_equals
      process_status
      (Unix.WSIGNALED (-7))
      "process_status";
    String_asserter.assert_equals "A\n" stdout "stdout";
    assert (Option.is_none exn);
    Lwt.return_true

let test_exec_checked_cancel_after () : bool Lwt.t =
  let (cancel, cancel_source) = Lwt.wait () in
  let _ =
    Lwt.bind (Lwt_unix.sleep 1.0) (fun () ->
        Lwt.wakeup_later cancel_source ();
        Lwt.return_unit)
  in
  let%lwt process_status =
    Lwt_utils.exec_checked
      Exec_command.Shell
      [| "echo A; sleep 0.1; echo B" |]
      ~cancel
  in
  match process_status with
  | Ok { Lwt_utils.Process_success.stdout; _ } ->
    String_asserter.assert_equals stdout "A\nB\n" "stdout";
    Lwt.return true
  | Error _ -> failwith "command failed even though it should have succeeded"

let test_exec_checked_big_payload () : bool Lwt.t =
  (* Try to exceed the pipe buffer size to suss out any deadlocks. *)
  let payload_size = 500000 in
  let big_payload = String.init payload_size ~f:(fun _ -> 'x') in
  let%lwt process_status =
    Lwt_utils.exec_checked
      (Exec_command.For_use_in_testing_only "cat")
      [||]
      ~input:big_payload
  in
  match process_status with
  | Ok { Lwt_utils.Process_success.stdout; _ } ->
    String_asserter.assert_equals stdout big_payload "incorrect stdout";
    Lwt.return true
  | Error _ -> failwith "command failed"

let test_lwt_message_queue_basic () : bool Lwt.t =
  let q = Lwt_message_queue.create () in
  Bool_asserter.assert_equals
    (Lwt_message_queue.is_empty q)
    true
    "queue was expected to be empty";
  let%lwt () =
    let%lwt () = Lwt_unix.sleep 1.0 in
    let _success : bool = Lwt_message_queue.push q "message1" in
    let _success : bool = Lwt_message_queue.push q "message2" in
    Bool_asserter.assert_equals
      (Lwt_message_queue.is_empty q)
      false
      "queue was expected to be non-empty";
    Lwt.return_unit
  and () =
    let%lwt () =
      match%lwt Lwt_message_queue.pop q with
      | None -> failwith "expected queue to still have messages"
      | Some result ->
        String_asserter.assert_equals result "message1" "wrong message in queue";
        Lwt.return_unit
    in
    let%lwt () =
      match%lwt Lwt_message_queue.pop q with
      | None -> failwith "expected queue to still have messages"
      | Some result ->
        String_asserter.assert_equals result "message2" "wrong message in queue";
        Lwt.return_unit
    in
    Lwt.return_unit
  in
  Lwt.return true

let test_lwt_message_queue_close () : bool Lwt.t =
  let q = Lwt_message_queue.create () in
  let success = Lwt_message_queue.push q "should be dropped later" in
  Bool_asserter.assert_equals
    success
    true
    "should have pushed message successfully";
  Lwt_message_queue.close q;
  let success = Lwt_message_queue.push q "should be dropped now" in
  Bool_asserter.assert_equals success false "should have failed to push message";

  match%lwt Lwt_message_queue.pop q with
  | Some _ -> failwith "popped a message despite the queue being closed"
  | None -> Lwt.return true

let test_lwt_message_queue_length () : bool Lwt.t =
  let q = Lwt_message_queue.create () in
  let initial_length = Lwt_message_queue.length q in
  Int_asserter.assert_equals initial_length 0 "initial_length should be 0";

  let (_ : bool) = Lwt_message_queue.push q 1 in
  let (_ : bool) = Lwt_message_queue.push q 2 in
  let (_ : bool) = Lwt_message_queue.push q 3 in
  let intermediate_length = Lwt_message_queue.length q in
  Int_asserter.assert_equals
    intermediate_length
    3
    "intermediate_length should be 3";

  Lwt_message_queue.close q;
  let closed_length = Lwt_message_queue.length q in
  Int_asserter.assert_equals
    closed_length
    0
    ("the length once the queue is closed should be reset to 0, "
    ^ "regardless of the number of previously-present items");
  Lwt.return true

let wrap_lwt_test (test : string * (unit -> bool Lwt.t)) :
    string * (unit -> bool) =
  let (name, f) = test in
  let wrapped_test () : bool Lwt.t =
    let%lwt result =
      Lwt.pick
        [
          f ();
          (let timeout = 5.0 in
           let%lwt () = Lwt_unix.sleep timeout in
           failwith
             (Printf.sprintf "Test %s timed out after %f seconds" name timeout));
        ]
    in
    Lwt.return result
  in
  (name, (fun () -> Lwt_main.run (wrapped_test ())))

let () =
  Unit_test.run_all
  @@ List.map
       ~f:wrap_lwt_test
       [
         ("test Lwt_utils.exec_checked basic", test_exec_checked_basic);
         ("test Lwt_utils.exec_checked failing", test_exec_checked_failing);
         ("test Lwt_utils.exec_checked timeout", test_exec_checked_timeout);
         ( "test Lwt_utils.exec_checked cancel before",
           test_exec_checked_cancel_before );
         ( "test Lwt_utils.exec_checked cancel during",
           test_exec_checked_cancel_during );
         ( "test Lwt_utils.exec_checked cancel after",
           test_exec_checked_cancel_after );
         ( "test Lwt_utils.exec_checked big payload",
           test_exec_checked_big_payload );
         ("test Lwt_message_queue.t basic", test_lwt_message_queue_basic);
         ("test Lwt_message_queue.t close", test_lwt_message_queue_close);
         ("test Lwt_message_queue.t length", test_lwt_message_queue_length);
       ]
