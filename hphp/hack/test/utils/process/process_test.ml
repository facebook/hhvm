open Asserter

let test_echo () =
  let process = Process.exec "echo" [ "hello world"; ] in
  let status, result, _err = Process.read_and_close_pid process in
  let () = Process_status_asserter.assert_equals (Unix.WEXITED 0) status "" in
  let () = String_asserter.assert_equals "hello world\n" result "" in
  true

let test_env_variable () =
  let process = Process.exec "printenv" ~env:[ "NAME=world" ] [ ] in
  let status, result, _err = Process.read_and_close_pid process in
  let () = Process_status_asserter.assert_equals (Unix.WEXITED 0) status "" in
  let () = String_asserter.assert_equals "NAME=world\n" result "" in
  true

let tests = [
  ("test_echo", test_echo);
  ("test_env_variable", test_env_variable);
]

let () =
  Unit_test.run_all tests
