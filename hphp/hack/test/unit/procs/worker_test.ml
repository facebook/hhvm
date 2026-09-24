open Hh_prelude

let entry =
  Worker_controller_entry_point.register ~restore:(fun () ~(worker_id : int) ->
      Hh_logger.set_id (Printf.sprintf "worker_test %d" worker_id))

let num_workers = 2

let make_worker ?call_wrapper ~longlived_workers heap_handle =
  Worker_controller.make
    ?call_wrapper
    ~longlived_workers
    ~saved_state:()
    ~entry
    num_workers
    ~gc_control:(Gc.get ())
    ~heap_handle

let rec wait_until_ready handle =
  let { Worker_controller.readys; waiters = _; ready_fds = _ } =
    Worker_controller.select [handle] []
  in
  match readys with
  | [] -> wait_until_ready handle
  | ready :: _ -> ready

(** If "f x" throws, we exit the program with a custom exit code. *)
let catch_exception_and_custom_exit_wrapper : 'x 'b. ('x -> 'b) -> 'x -> 'b =
 fun f x ->
  try f x with
  | _ -> exit 17

let call_and_verify_result worker f x expected =
  let result =
    Worker_controller.call worker f x
    |> wait_until_ready
    |> Worker_controller.get_result
  in
  String.equal result expected

(** This is just like the test_worker_uncaught_exception_exits_with_2 test
 * except we add a call_wapper to the worker. It catches all exceptions and
 * makes the worker exit with code 17. *)
let test_wrapped_worker_with_custom_exit use_clones heap_handle () =
  let workers =
    make_worker
      ~call_wrapper:
        { Worker_controller.wrap = catch_exception_and_custom_exit_wrapper }
      ~longlived_workers:(not use_clones)
      heap_handle
  in
  match workers with
  | [] ->
    Printf.eprintf "Failed to create workers";
    false
  | worker :: _ ->
    (try
       call_and_verify_result
         worker
         (fun () -> raise (Failure "oops"))
         ()
         "dummy"
     with
    | Worker_controller.Worker_failed
        (_, Worker_controller.Worker_quit (Unix.WEXITED i)) ->
      i = 17)

let test_worker_uncaught_exception_exits_with_2 use_clones heap_handle () =
  let workers = make_worker ~longlived_workers:use_clones heap_handle in
  match workers with
  | [] ->
    Printf.eprintf "Failed to create workers";
    false
  | worker :: _ ->
    (try
       call_and_verify_result
         worker
         (fun () -> raise (Failure "oops"))
         ()
         "dummy"
     with
    | Worker_controller.Worker_failed
        (_, Worker_controller.Worker_quit (Unix.WEXITED i)) ->
      i = 2)

let test_job_marshal_exception_exits_with_2 use_clones heap_handle () =
  let workers = make_worker ~longlived_workers:use_clones heap_handle in
  match workers with
  | [] ->
    Printf.eprintf "Failed to create workers";
    false
  | worker :: _ ->
    (try
       call_and_verify_result
         worker
         (fun () -> raise Marshal_tools.Reading_Payload_Exception)
         ()
         "dummy"
     with
    | Worker_controller.Worker_failed
        (_, Worker_controller.Worker_quit (Unix.WEXITED i)) ->
      i = 2)

let test_controller_channel_closure_is_controller_death () =
  let (input_fd, controller_fd) = Unix.pipe () in
  let output_fd = Daemon.null_fd () in
  let controller_fd_is_open = ref true in
  let close_no_fail fd =
    try Unix.close fd with
    | Unix.Unix_error (Unix.EBADF, _, _) -> ()
  in
  let close_controller_fd () =
    if !controller_fd_is_open then (
      controller_fd_is_open := false;
      close_no_fail controller_fd
    )
  in
  Utils.try_finally
    ~f:(fun () ->
      let preamble = Marshal_tools.make_preamble 2 in
      let preamble_size = Bytes.length preamble in
      let preamble_bytes_written =
        Unix.write controller_fd preamble 0 preamble_size
      in
      let payload = Bytes.of_string "x" in
      let payload_bytes_written = Unix.write controller_fd payload 0 1 in
      close_controller_fd ();
      let outcome =
        Utils.try_finally
          ~f:(fun () -> Worker.For_test.read_and_process_job input_fd output_fd)
          ~finally:(fun () -> Measure.pop_global () |> ignore)
      in
      preamble_bytes_written = preamble_size
      && payload_bytes_written = 1
      && Poly.(outcome = `Controller_has_died))
    ~finally:(fun () ->
      close_no_fail input_fd;
      close_controller_fd ();
      close_no_fail output_fd)

let test_simple_worker_spawn use_clones heap_handle () =
  let workers = make_worker ~longlived_workers:use_clones heap_handle in
  match workers with
  | [] ->
    Printf.eprintf "Failed to create workers";
    false
  | worker :: _ -> call_and_verify_result worker (fun () -> "hello") () "hello"

let make_tests handle =
  let make_test name fn =
    [(name, fn true handle); ("no_clones_" ^ name, fn false handle)]
  in
  make_test "simple_worker_spawn_test" test_simple_worker_spawn
  @ make_test
      "worker_uncaught_exception_exits_with_2"
      test_worker_uncaught_exception_exits_with_2
  @ make_test
      "job_marshal_exception_exits_with_2"
      test_job_marshal_exception_exits_with_2
  @ make_test
      "wrapped_worker_with_custom_exit"
      test_wrapped_worker_with_custom_exit
  @ [
      ( "controller_channel_closure_is_controller_death",
        test_controller_channel_closure_is_controller_death );
    ]

let () =
  Daemon.check_entry_point ();

  (* this call might not return *)
  let heap_handle = Shared_mem.init ~num_workers Shared_mem.default_config in
  let tests = make_tests heap_handle in
  Unit_test.run_all tests
