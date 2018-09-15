open Asserter

let test_echo () =
  let process = Process.exec "echo" [ "hello world"; ] in
  match Process.read_and_wait_pid ~timeout:2 process with
  | Ok (result, _err) ->
    let () = String_asserter.assert_equals "hello world\n" result "" in
    true
  | _ ->
    false

let test_echo_in_a_loop () =
  let rec loop acc = function
  | 0 -> acc
  | n ->
    let acc = acc && (test_echo ()) in
    loop acc (n-1)
  in
  (* There was a bug leaking 2 file descriptors per Process execution, and
   * running it over 500 times would run out of FDs *)
  loop true 600

let test_process_read_idempotent () =
  let process = Process.exec "echo" [ "hello world"; ] in
  let result = Process.read_and_wait_pid ~timeout:2 process in
  let () = match result with
    | Ok (result, _err) ->
      String_asserter.assert_equals "hello world\n" result ""
    | _ ->
      ()
  in
  let result = Process.read_and_wait_pid ~timeout:2 process in
  match result with
  | Ok (result, _err) ->
    String_asserter.assert_equals "hello world\n" result "";
    true
  | _ ->
    false

let test_env_variable () =
  let process = Process.exec_with_replacement_env "printenv" ~env:[ "NAME=world" ] [ ] in
  match Process.read_and_wait_pid ~timeout:2 process with
  | Ok (result, _stderr) ->
    let () = String_asserter.assert_equals "NAME=world\n" result "" in
    true
  | _ ->
    false

let test_process_timeout () =
  let process = Process.exec "sleep" [ "2"; ] in
  match Process.read_and_wait_pid ~timeout:1 process with
  | Error (Process_types.Timed_out _) ->
    true
  | _ ->
    false

let test_process_finishes_within_timeout () =
  let process = Process.exec "sleep" [ "1"; ] in
  match Process.read_and_wait_pid ~timeout:2 process with
  | Ok _ ->
    true
  | _ ->
    false

let test_future () =
  let future = Future.make (Process.exec "sleep" [ "1" ]) String.trim in
  let result = Future.get_exn future in
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

let test_delayed_future () =
  let future = Future.delayed_value ~delays:3 "Delayed value" in
  Bool_asserter.assert_equals false (Future.is_ready future) "First delay";
  Bool_asserter.assert_equals false (Future.is_ready future) "Second delay";
  Bool_asserter.assert_equals false (Future.is_ready future) "Third delay";
  Bool_asserter.assert_equals true (Future.is_ready future)
    "After third delay should be ready";
  String_asserter.assert_equals "Delayed value" (Future.get_exn future)
    "retrieve delayed value";
  true

(** Send "hello" to stdin and use sed to replace hello to world. *)
let test_stdin_input () =
  let process = Process.exec "sed" ~input:"hello" [ "s/hello/world/g"; ] in
  match Process.read_and_wait_pid ~timeout:3 process with
  | Ok (msg, _) ->
    String_asserter.assert_equals "world" msg
      "sed should replace hello with world";
    true
  | Error failure ->
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
  | Ok (out, _err) ->
    let () = String_asserter.assert_equals "hello\n" out "" in
    true
  | _ ->
    false

let test_chdir () =
  let process = Process.exec ~cwd:"/tmp" "pwd" [] in
  let result = Process.read_and_wait_pid ~timeout:10 process in
  match result with
  | Ok (out, _err) ->
    let () = String_asserter.assert_equals "/tmp\n" out "" in
    true
  | Error (Process_types.Timed_out _) ->
    Printf.eprintf "Process timed out\n";
    false
  | Error Process_types.Process_aborted_input_too_large ->
    Printf.eprintf "Unexpected error process input too large\n";
    false
  | Error (Process_types.Process_exited_abnormally
    (_, stdout, stderr)) ->
      Printf.eprintf "Process exited abnormally\n";
      Printf.eprintf "See stdout: %s\n" stdout;
      Printf.eprintf "See stderr: %s\n" stderr;
      false

let open_an_fd () =
  Unix.openfile "/dev/null" [Unix.O_RDONLY] 0o440

let int_of_fd (x : Unix.file_descr) : int = Obj.magic x

let close_fds fds =
  List.iter Unix.close fds

(** Asserts the next opened file descriptor is exactly 1 greater
 * than the "last_fd". Repeats this "repeats" times. Accumulates all opened
 * FDs into all_fds and closes all of them at the end. *)
let rec assert_next_fd_incremental ~repeats all_fds last_fd =
  if repeats <= 0 then
    close_fds all_fds
  else
    let fd = open_an_fd () in
    let () = Int_asserter.assert_equals ((int_of_fd last_fd) + 1) (int_of_fd fd)
      "Test unexpectedly leaked a File Descriptor." in
    assert_next_fd_incremental ~repeats:(repeats - 1) (fd :: all_fds) fd

(** Opens one FD before running a test, and then opens 1000 FDs after the test
 * and asserts that all the FD numbers are sequential (i.e. no holes).
 *
 * This catches leaks during the test because if the test opened n FDs
 * and closed none of them, then the first opened file descriptor after the
 * test finishes will be number n+1 instead of one greater than the first
 * one (the one opened before running the test). If instead the test failed
 * to close the k'th descriptor, then this will discover after k
 * iterations that the next FD opened is non-sequential.
 *)
let assert_no_fd_leaked test = fun () ->
  let first_fd = open_an_fd () in
  let result = test () in
  assert_next_fd_incremental ~repeats:1000 [first_fd] first_fd;
  result

let tests = [
  ("test_echo", assert_no_fd_leaked test_echo);
  ("test_echo_in_a_loop", assert_no_fd_leaked test_echo_in_a_loop);
  ("test_process_read_idempotent", assert_no_fd_leaked test_process_read_idempotent);
  ("test_env_variable", assert_no_fd_leaked test_env_variable);
  ("test_process_timeout", assert_no_fd_leaked test_process_timeout);
  ("test_process_finishes_within_timeout",
    assert_no_fd_leaked test_process_finishes_within_timeout);
  ("test_future", assert_no_fd_leaked test_future);
  ("test_future_is_ready", assert_no_fd_leaked test_future_is_ready);
  ("test_stdin_input", assert_no_fd_leaked test_stdin_input);
  ("test_entry_point", assert_no_fd_leaked test_entry_point);
  ("test_chdir", assert_no_fd_leaked test_chdir);
]

let () =
  Daemon.check_entry_point ();
  Unit_test.run_all tests
