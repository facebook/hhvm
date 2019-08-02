open Asserter

let test_basic_exec_checked () : bool Lwt.t =
  let%lwt process_status =
    Lwt_utils.exec_checked "echo" [|"hello"; "world"|]
  in
  match process_status with
  | Ok { Lwt_utils.Process_success.command_line; stdout; stderr } ->
    String_asserter.assert_equals
      command_line
      "echo hello world"
      "incorrect command line";
    String_asserter.assert_equals stdout "hello world\n" "incorrect stdout";
    String_asserter.assert_equals stderr "" "incorrect stderr";
    Lwt.return true
  | Error _ -> failwith "command failed"

let test_failing_exec_checked () : bool Lwt.t =
  let%lwt process_status =
    Lwt_utils.exec_checked "false" [|"ignored-argument"|]
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
      } ->
    assert (exn = None);
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

let test_exec_checked_big_payload () : bool Lwt.t =
  (* Try to exceed the pipe buffer size to suss out any deadlocks. *)
  let payload_size = 500000 in
  let big_payload = String.init payload_size (fun _ -> 'x') in
  let%lwt process_status =
    Lwt_utils.exec_checked "cat" [||] ~input:big_payload
  in
  match process_status with
  | Ok { Lwt_utils.Process_success.stdout; _ } ->
    String_asserter.assert_equals stdout big_payload "incorrect stdout";
    Lwt.return true
  | Error _ -> failwith "command failed"

let test_basic_lwt_message_queue () : bool Lwt.t =
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
        String_asserter.assert_equals
          result
          "message1"
          "wrong message in queue";
        Lwt.return_unit
    in
    let%lwt () =
      match%lwt Lwt_message_queue.pop q with
      | None -> failwith "expected queue to still have messages"
      | Some result ->
        String_asserter.assert_equals
          result
          "message2"
          "wrong message in queue";
        Lwt.return_unit
    in
    Lwt.return_unit
  in
  Lwt.return true

let test_close_lwt_message_queue () : bool Lwt.t =
  let q = Lwt_message_queue.create () in
  let success = Lwt_message_queue.push q "should be dropped later" in
  Bool_asserter.assert_equals
    success
    true
    "should have pushed message successfully";
  Lwt_message_queue.close q;
  let success = Lwt_message_queue.push q "should be dropped now" in
  Bool_asserter.assert_equals
    success
    false
    "should have failed to push message";

  match%lwt Lwt_message_queue.pop q with
  | Some _ -> failwith "popped a message despite the queue being closed"
  | None -> Lwt.return true

let wrap_lwt_test (test : string * (unit -> bool Lwt.t)) :
    string * (unit -> bool) =
  let (name, f) = test in
  let wrapped_test () : bool Lwt.t =
    let%lwt result =
      Lwt.pick
        [ f ();
          (let timeout = 5.0 in
           let%lwt () = Lwt_unix.sleep timeout in
           failwith
             (Printf.sprintf "Test %s timed out after %f seconds" name timeout))
        ]
    in
    Lwt.return result
  in
  (name, (fun () -> Lwt_main.run (wrapped_test ())))

let () =
  Unit_test.run_all
  @@ List.map
       wrap_lwt_test
       [ ("test basic Lwt_utils.exec_checked", test_basic_exec_checked);
         ("test failing Lwt_utils.exec_checked", test_failing_exec_checked);
         ( "test Lwt_utils.exec_checked big payload",
           test_exec_checked_big_payload );
         ("test basic Lwt_message_queue.t", test_basic_lwt_message_queue);
         ("test close Lwt_message_queue.t", test_close_lwt_message_queue) ]
