open Asserter

let test_echo () =
  let process = Process.exec "echo" [ "hello world"; ] in
  match Process.read_and_wait_pid ~timeout:2 process with
  | Result.Ok (result, _err) ->
    let () = String_asserter.assert_equals "hello world\n" result "" in
    true
  | _ ->
    false

let test_process_read_idempotent () =
  let process = Process.exec "echo" [ "hello world"; ] in
  let result = Process.read_and_wait_pid ~timeout:2 process in
  let () = match result with
    | Result.Ok (result, _err) ->
      String_asserter.assert_equals "hello world\n" result ""
    | _ ->
      ()
  in
  let result = Process.read_and_wait_pid ~timeout:2 process in
  match result with
  | Result.Ok (result, _err) ->
    String_asserter.assert_equals "hello world\n" result "";
    true
  | _ ->
    false

let test_env_variable () =
  let process = Process.exec "printenv" ~env:[ "NAME=world" ] [ ] in
  match Process.read_and_wait_pid ~timeout:2 process with
  | Result.Ok (result, _stderr) ->
    let () = String_asserter.assert_equals "NAME=world\n" result "" in
    true
  | _ ->
    false

let test_process_timeout () =
  let process = Process.exec "sleep" [ "2"; ] in
  match Process.read_and_wait_pid ~timeout:1 process with
  | Result.Error (Process_types.Timed_out _) ->
    true
  | _ ->
    false

let test_process_finishes_within_timeout () =
  let process = Process.exec "sleep" [ "1"; ] in
  match Process.read_and_wait_pid ~timeout:2 process with
  | Result.Ok _ ->
    true
  | _ ->
    false

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

(** Send "hello" to stdin and use sed to replace hello to world. *)
let test_stdin_input () =
  let process = Process.exec "sed" ~input:"hello" [ "s/hello/world/g"; ] in
  match Process.read_and_wait_pid ~timeout:3 process with
  | Result.Ok (msg, _) ->
    String_asserter.assert_equals "world" msg
      "sed should replace hello with world";
    true
  | Result.Error failure ->
    Printf.eprintf "Error %s" (Process.failure_msg failure);
    false

let print_string_main str =
  Printf.printf "%s\n" str

let print_string_entry = Process.register_entry_point
  "print_string_main" print_string_main

let test_entry_point () =
  let process = Process.run_entry print_string_entry "hello" in
  let result = Process.read_and_wait_pid ~timeout:10 process in
  match result with
  | Result.Ok (out, _err) ->
    let () = String_asserter.assert_equals "hello\n" out "" in
    true
  | _ ->
    false

let test_chdir () =
  let process = Process.exec ~cwd:"/tmp" "pwd" [] in
  let result = Process.read_and_wait_pid ~timeout:10 process in
  match result with
  | Result.Ok (out, _err) ->
    let () = String_asserter.assert_equals "/tmp\n" out "" in
    true
  | Result.Error (Process_types.Timed_out _) ->
    Printf.eprintf "Process timed out\n";
    false
  | Result.Error Process_types.Process_aborted_input_too_large ->
    Printf.eprintf "Unexpected error process input too large\n";
    false
  | Result.Error (Process_types.Process_exited_abnormally
    (_, stdout, stderr)) ->
      Printf.eprintf "Process exited abnormally\n";
      Printf.eprintf "See stdout: %s\n" stdout;
      Printf.eprintf "See stderr: %s\n" stderr;
      false

let tests = [
  ("test_echo", test_echo);
  ("test_process_read_idempotent", test_process_read_idempotent);
  ("test_env_variable", test_env_variable);
  ("test_process_timeout", test_process_timeout);
  ("test_process_finishes_within_timeout",
    test_process_finishes_within_timeout);
  ("test_future", test_future);
  ("test_future_is_ready", test_future_is_ready);
  ("test_stdin_input", test_stdin_input);
  ("test_entry_point", test_entry_point);
  ("test_chdir", test_chdir);
]

let () =
  Daemon.check_entry_point ();
  Unit_test.run_all tests
