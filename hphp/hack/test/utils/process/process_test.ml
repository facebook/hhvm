open Asserter

let test_echo () =
  let process =
    Process.exec (Exec_command.For_use_in_testing_only "echo") ["hello world"]
  in
  match Process.read_and_wait_pid ~timeout:2 process with
  | Ok { Process_types.stdout; _ } ->
    let () = String_asserter.assert_equals "hello world\n" stdout "" in
    true
  | _ -> false

let test_echo_in_a_loop () =
  let rec loop acc = function
    | 0 -> acc
    | n ->
      let acc = acc && test_echo () in
      loop acc (n - 1)
  in
  (* There was a bug leaking 2 file descriptors per Process execution, and
   * running it over 500 times would run out of FDs *)
  loop true 600

let test_process_read_idempotent () =
  let process =
    Process.exec (Exec_command.For_use_in_testing_only "echo") ["hello world"]
  in
  let result = Process.read_and_wait_pid ~timeout:2 process in
  let () =
    match result with
    | Ok { Process_types.stdout; _ } ->
      String_asserter.assert_equals "hello world\n" stdout ""
    | _ -> ()
  in
  let result = Process.read_and_wait_pid ~timeout:2 process in
  match result with
  | Ok { Process_types.stdout; _ } ->
    String_asserter.assert_equals "hello world\n" stdout "";
    true
  | _ -> false

let test_env_variable () =
  let process =
    Process.exec
      (Exec_command.For_use_in_testing_only "printenv")
      ~env:(Process_types.Augment ["NAME=world"])
      []
  in
  match Process.read_and_wait_pid ~timeout:2 process with
  | Ok { Process_types.stdout; _ } ->
    let env = String_utils.split_into_lines stdout in
    let name_env =
      List.filter (fun s -> String_utils.string_starts_with s "NAME=") env
    in
    (match name_env with
    | [] -> false
    | n :: _ ->
      let () = String_asserter.assert_equals "NAME=world" n "" in
      true)
  | _ -> false

let test_process_timeout () =
  let process =
    Process.exec (Exec_command.For_use_in_testing_only "sleep") ["2"]
  in
  match Process.read_and_wait_pid ~timeout:1 process with
  | Error (Process_types.Timed_out _) -> true
  | _ -> false

let test_process_finishes_within_timeout () =
  let process =
    Process.exec (Exec_command.For_use_in_testing_only "sleep") ["1"]
  in
  match Process.read_and_wait_pid ~timeout:2 process with
  | Ok _ -> true
  | _ -> false

let test_future () =
  let future =
    Future.make
      (Process.exec (Exec_command.For_use_in_testing_only "sleep") ["1"])
      String.trim
  in
  let result = Future.get_exn future in
  let () = String_asserter.assert_equals "" result "" in
  true

let test_future_is_ready () =
  let future =
    Future.make
      (Process.exec (Exec_command.For_use_in_testing_only "sleep") ["1"])
      String.trim
  in
  (* Shouldn't be ready immediately. *)
  if Future.is_ready future then
    false
  else
    (* Is ready after sleeping. *)
    let (_, _, _) = Unix.select [] [] [] 2.0 in
    if Future.is_ready future then
      true
    else
      false

let test_future_continue_with () =
  Tempfile.with_real_tempdir (fun dir_path ->
      let fn = Path.concat dir_path "test.txt" in
      RealDisk.write_file ~file:(Path.to_string fn) ~contents:"my file contents";
      let ls_proc =
        Process.exec
          (Exec_command.For_use_in_testing_only "ls")
          [Path.to_string dir_path]
      in
      let future = Future.make ls_proc String.trim in
      let future = Future.continue_with future String.uppercase_ascii in
      String_asserter.assert_equals
        "TEST.TXT"
        (Future.get_exn future)
        "future mapping should modify results");
  true

let test_future_continue_with_future () =
  Tempfile.with_real_tempdir (fun dir_path ->
      let fn = Path.concat dir_path "test.txt" in
      RealDisk.write_file ~file:(Path.to_string fn) ~contents:"my file contents";
      let ls_proc =
        Process.exec
          (Exec_command.For_use_in_testing_only "ls")
          [Path.to_string dir_path]
      in
      let future = Future.make ls_proc String.trim in
      let future =
        Future.continue_with_future future (fun a ->
            let cat_proc =
              Process.exec
                (Exec_command.For_use_in_testing_only "cat")
                [Path.to_string (Path.concat dir_path a)]
            in
            Future.make cat_proc String.trim)
      in
      String_asserter.assert_equals
        "my file contents"
        (Future.get_exn future)
        "ls >>= cat should return file contents");
  true

let test_future_continue_and_map_err_ok () =
  Tempfile.with_real_tempdir (fun dir_path ->
      let fn = Path.concat dir_path "test.txt" in
      RealDisk.write_file ~file:(Path.to_string fn) ~contents:"my file contents";
      let ls_proc =
        Process.exec
          (Exec_command.For_use_in_testing_only "ls")
          [Path.to_string dir_path]
      in
      let future = Future.make ls_proc String.trim in
      let future =
        Future.continue_and_map_err future (fun res ->
            match res with
            | Ok str -> Ok (String.uppercase_ascii str)
            | Error _ -> Error "should not hit this point")
      in
      match Future.get_exn future with
      | Ok s ->
        String_asserter.assert_equals
          "TEST.TXT"
          s
          "the mapped result should be used, not the original"
      | Error s -> failwith s);
  true

let test_future_continue_and_map_err_error () =
  let fail_proc =
    Process.exec
      (Exec_command.For_use_in_testing_only "command_that_doesnt_exist")
      []
  in
  let future = Future.make fail_proc String.trim in
  let future =
    Future.continue_and_map_err future (fun res ->
        match res with
        | Ok _ -> Ok "should not hit this point"
        | Error _ -> Error "successfully failed")
  in
  (match Future.get_exn future with
  | Ok s -> failwith s
  | Error s ->
    String_asserter.assert_equals
      "successfully failed"
      s
      "the error message should be preserved");
  true

let test_future_long_continuation_chain_ok () =
  Tempfile.with_real_tempdir (fun dir_path ->
      let dir1 = Path.concat dir_path "dir1" in
      let dir2 = Path.concat dir_path "dir2" in
      RealDisk.mkdir_p (Path.to_string dir1);
      RealDisk.mkdir_p (Path.to_string dir2);
      let fn = Path.to_string (Path.concat dir1 "test.txt") in
      let fn2 = Path.to_string (Path.concat dir2 "test2.txt") in
      RealDisk.write_file ~file:fn ~contents:fn2;
      RealDisk.write_file ~file:fn2 ~contents:"my file contents";
      let ls_proc =
        Process.exec
          (Exec_command.For_use_in_testing_only "ls")
          [Path.to_string dir1]
      in
      let future = Future.make ls_proc String.trim in
      let future =
        Future.continue_with_future future (fun ls_result ->
            let cat_proc =
              Process.exec
                (Exec_command.For_use_in_testing_only "cat")
                [Path.to_string (Path.concat dir1 ls_result)]
            in
            Future.make cat_proc String.trim)
      in
      let future =
        Future.continue_with_future future (fun cat_result ->
            let cat_proc =
              Process.exec
                (Exec_command.For_use_in_testing_only "cat")
                [cat_result]
            in
            Future.make cat_proc String.trim)
      in
      String_asserter.assert_equals
        "my file contents"
        (Future.get_exn future)
        "ls >>= cat >>= cat should return file contents");
  true

let test_future_long_continuation_chain_error () =
  Tempfile.with_real_tempdir (fun dir_path ->
      let dir1 = Path.concat dir_path "dir1" in
      let dir2 = Path.concat dir_path "dir2" in
      RealDisk.mkdir_p (Path.to_string dir1);
      RealDisk.mkdir_p (Path.to_string dir2);
      let fn = Path.to_string (Path.concat dir1 "test.txt") in
      let fn2 = Path.to_string (Path.concat dir2 "test2.txt") in
      RealDisk.write_file ~file:fn ~contents:(fn2 ^ ".nowhere");
      RealDisk.write_file ~file:fn2 ~contents:"my file contents";
      let ls_proc =
        Process.exec
          (Exec_command.For_use_in_testing_only "ls")
          [Path.to_string dir1]
      in
      let future = Future.make ls_proc String.trim in
      let future =
        Future.continue_with_future future (fun ls_result ->
            let cat_proc =
              Process.exec
                (Exec_command.For_use_in_testing_only "cat")
                [Path.to_string (Path.concat dir1 ls_result)]
            in
            Future.make cat_proc String.trim)
      in
      let future =
        Future.continue_with_future future (fun cat_result ->
            let cat_proc =
              Process.exec
                (Exec_command.For_use_in_testing_only "cat")
                [cat_result]
            in
            Future.make cat_proc String.trim)
      in
      let future =
        Future.continue_and_map_err future (fun res ->
            match res with
            | Ok s -> Ok (Printf.sprintf "Expected no output, got '%s'" s)
            | Error _ -> Error "successfully failed")
      in
      let future =
        Future.continue_with future (fun res ->
            match res with
            | Ok s -> Ok (String.uppercase_ascii s)
            | Error s -> Error (String.uppercase_ascii s))
      in
      match Future.get_exn future with
      | Ok s -> failwith (Printf.sprintf "Expected failure, got Ok '%s'" s)
      | Error s ->
        String_asserter.assert_equals
          "SUCCESSFULLY FAILED"
          s
          "the error message should be mapped");
  true

(** Send "hello" to stdin and use sed to replace hello to world. *)
let test_stdin_input () =
  let process =
    Process.exec
      (Exec_command.For_use_in_testing_only "sed")
      ~input:"hello"
      ["s/hello/world/g"]
  in
  match Process.read_and_wait_pid ~timeout:3 process with
  | Ok { Process_types.stdout; _ } ->
    String_asserter.assert_equals
      "world"
      stdout
      "sed should replace hello with world";
    true
  | Error failure ->
    Printf.eprintf "Error %s" (Process.failure_msg failure);
    false

let print_string_main str = Printf.printf "%s\n" str

let print_string_entry =
  Process.register_entry_point "print_string_main" print_string_main

let test_entry_point () =
  let process = Process.run_entry print_string_entry "hello" in
  let result = Process.read_and_wait_pid ~timeout:10 process in
  match result with
  | Ok { Process_types.stdout; _ } ->
    let () = String_asserter.assert_equals "hello\n" stdout "" in
    true
  | _ -> false

let test_chdir () =
  let process =
    Process.exec_with_working_directory
      ~dir:"/tmp"
      (Exec_command.For_use_in_testing_only "pwd")
      []
  in
  let result = Process.read_and_wait_pid ~timeout:10 process in
  match result with
  | Ok { Process_types.stdout; _ } ->
    let () = String_asserter.assert_equals "/tmp\n" stdout "" in
    true
  | Error (Process_types.Timed_out _) ->
    Printf.eprintf "Process timed out\n";
    false
  | Error Process_types.Overflow_stdin ->
    Printf.eprintf "Unexpected error process input too large\n";
    false
  | Error (Process_types.Abnormal_exit { stdout; stderr; _ }) ->
    Printf.eprintf "Process exited abnormally\n";
    Printf.eprintf "See stdout: %s\n" stdout;
    Printf.eprintf "See stderr: %s\n" stderr;
    false

let open_an_fd () = Unix.openfile "/dev/null" [Unix.O_RDONLY] 0o440

let int_of_fd (x : Unix.file_descr) : int = Obj.magic x

let close_fds fds = List.iter Unix.close fds

(** Asserts the next opened file descriptor is exactly 1 greater
 * than the "last_fd". Repeats this "repeats" times. Accumulates all opened
 * FDs into all_fds and closes all of them at the end. *)
let rec assert_next_fd_incremental ~repeats all_fds last_fd =
  if repeats <= 0 then
    close_fds all_fds
  else
    let fd = open_an_fd () in
    let () =
      Int_asserter.assert_equals
        (int_of_fd last_fd + 1)
        (int_of_fd fd)
        "Test unexpectedly leaked a File Descriptor."
    in
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
let assert_no_fd_leaked test () =
  let first_fd = open_an_fd () in
  let result = test () in
  assert_next_fd_incremental ~repeats:1000 [first_fd] first_fd;
  result

let tests =
  [
    ("test_echo", assert_no_fd_leaked test_echo);
    ("test_echo_in_a_loop", assert_no_fd_leaked test_echo_in_a_loop);
    ( "test_process_read_idempotent",
      assert_no_fd_leaked test_process_read_idempotent );
    ("test_env_variable", assert_no_fd_leaked test_env_variable);
    ("test_process_timeout", assert_no_fd_leaked test_process_timeout);
    ( "test_process_finishes_within_timeout",
      assert_no_fd_leaked test_process_finishes_within_timeout );
    ("test_future", assert_no_fd_leaked test_future);
    ("test_future_is_ready", assert_no_fd_leaked test_future_is_ready);
    ("test_future_continue_with", assert_no_fd_leaked test_future_continue_with);
    ( "test_future_continue_with_future",
      assert_no_fd_leaked test_future_continue_with_future );
    ( "test_future_continue_and_map_err_ok",
      assert_no_fd_leaked test_future_continue_and_map_err_ok );
    ( "test_future_continue_and_map_err_error",
      assert_no_fd_leaked test_future_continue_and_map_err_error );
    ( "test_future_long_continuation_chain_ok",
      assert_no_fd_leaked test_future_long_continuation_chain_ok );
    ( "test_future_long_continuation_chain_error",
      assert_no_fd_leaked test_future_long_continuation_chain_error );
    ("test_stdin_input", assert_no_fd_leaked test_stdin_input);
    ("test_entry_point", assert_no_fd_leaked test_entry_point);
    ("test_chdir", assert_no_fd_leaked test_chdir);
  ]

let () =
  Daemon.check_entry_point ();
  Unit_test.run_all tests
