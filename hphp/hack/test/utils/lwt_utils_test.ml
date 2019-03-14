open Asserter

let test_basic_exec_checked (): bool Lwt.t =
  let%lwt process_status = Lwt_utils.exec_checked
    "echo" [| "hello"; "world" |] in
  match process_status with
  | Ok { Lwt_utils.Process_success.command_line; stdout; stderr } ->
    String_asserter.assert_equals
      command_line
      "echo hello world"
      "incorrect command line";
    String_asserter.assert_equals
      stdout
      "hello world\n"
      "incorrect stdout";
    String_asserter.assert_equals
      stderr
      ""
      "incorrect stderr";
    Lwt.return true
  | Error _ ->
    failwith "command failed"

let test_failing_exec_checked (): bool Lwt.t =
  let%lwt process_status = Lwt_utils.exec_checked
    "false" [| "ignored-argument" |] in
  match process_status with
  | Ok _ ->
    failwith "command succeeded unexpectedly"
  | Error { Lwt_utils.Process_failure.
      command_line;
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
    String_asserter.assert_equals
      stdout
      ""
      "incorrect stdout";
    String_asserter.assert_equals
      stderr
      ""
      "incorrect stderr";
    Lwt.return true

let test_exec_checked_big_payload (): bool Lwt.t =
  (* Try to exceed the pipe buffer size to suss out any deadlocks. *)
  let payload_size = 500000 in
  let big_payload = String.init payload_size (fun _ -> 'x') in
  let%lwt process_status = Lwt_utils.exec_checked
    "cat" [||] ~input:big_payload in
  match process_status with
  | Ok { Lwt_utils.Process_success.stdout; _ } ->
    String_asserter.assert_equals
      stdout
      big_payload
      "incorrect stdout";
    Lwt.return true
  | Error _ ->
    failwith "command failed"

let wrap_lwt_test
    (test: string * (unit -> bool Lwt.t))
    : (string * (unit -> bool)) =
  let (name, f) = test in
  let wrapped_test (): bool Lwt.t =
    let%lwt result = Lwt.pick [
      f ();
      begin
        let timeout = 5.0 in
        let%lwt () = Lwt_unix.sleep timeout in
        failwith (Printf.sprintf
          "Test %s timed out after %f seconds"
          name
          timeout)
      end;
    ] in
    Lwt.return result
  in
  (name, fun () -> Lwt_main.run (wrapped_test ()))

let () =
  Unit_test.run_all @@ List.map wrap_lwt_test [
    ("test basic exec_checked", test_basic_exec_checked);
    ("test failing exec_checked", test_failing_exec_checked);
    ("test exec_checked big payload", test_exec_checked_big_payload);
  ]
