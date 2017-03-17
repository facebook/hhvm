open Asserter

let test_echo () =
  let process = Process.exec "echo" [ "hello world"; ] in
  let status, result, _err = Process.read_and_wait_pid process in
  let () = Process_status_asserter.assert_equals (Unix.WEXITED 0) status "" in
  let () = String_asserter.assert_equals "hello world\n" result "" in
  true

let test_process_read_idempotent () =
  let process = Process.exec "echo" [ "hello world"; ] in
  let status, result, _err = Process.read_and_wait_pid process in
  let () = Process_status_asserter.assert_equals (Unix.WEXITED 0) status "" in
  let () = String_asserter.assert_equals "hello world\n" result "" in
  (** Can read_and_wait_pid again. *)
  let status, result, _err = Process.read_and_wait_pid process in
  let () = Process_status_asserter.assert_equals (Unix.WEXITED 0) status "" in
  let () = String_asserter.assert_equals "hello world\n" result "" in
  true

let test_env_variable () =
  let process = Process.exec "printenv" ~env:[ "NAME=world" ] [ ] in
  let status, result, _err = Process.read_and_wait_pid process in
  let () = Process_status_asserter.assert_equals (Unix.WEXITED 0) status "" in
  let () = String_asserter.assert_equals "NAME=world\n" result "" in
  true

let test_future () =
  let future = Future.make (Process.exec "sleep" [ "1" ]) String.trim in
  let result = Future.get future in
  let () = String_asserter.assert_equals "" result "" in
  true

let test_future_is_ready () =
  let future = Future.make (Process.exec "sleep" [ "1" ]) String.trim in
  (** Shouldn't be ready immediately. *)
  if Future.is_ready future then
    false
  else
    (** Is ready after sleeping. *)
    let _, _, _ = Unix.select [] [] [] 2.0 in
    if Future.is_ready future then
      true
    else
      false

let tests = [
  ("test_echo", test_echo);
  ("test_process_read_idempotent", test_process_read_idempotent);
  ("test_env_variable", test_env_variable);
  ("test_future", test_future);
  ("test_future_is_ready", test_future_is_ready);
]

let () =
  Unit_test.run_all tests
