open Hh_prelude

let entry =
  WorkerControllerEntryPoint.register ~restore:(fun () ~(worker_id : int) ->
      Hh_logger.set_id (Printf.sprintf "worker_test %d" worker_id))

let num_workers = 2

let make_worker ?call_wrapper ~longlived_workers heap_handle =
  WorkerController.make
    ?call_wrapper
    ~longlived_workers
    ~saved_state:()
    ~entry
    num_workers
    ~gc_control:(Gc.get ())
    ~heap_handle

let rec wait_until_ready handle =
  let { WorkerController.readys; waiters = _; ready_fds = _ } =
    WorkerController.select [handle] []
  in
  match readys with
  | [] -> wait_until_ready handle
  | ready :: _ -> ready

(** If "f x" throws, we exit the program with a custom exit code. *)
let catch_exception_and_custom_exit_wrapper : 'x 'b. ('x -> 'b) -> 'x -> 'b =
 (fun f x -> (try f x with _ -> exit 17))

let call_and_verify_result worker f x expected =
  let result =
    WorkerController.call worker f x
    |> wait_until_ready
    |> WorkerController.get_result
  in
  String.equal result expected

(** This is just like the test_worker_uncaught_exception_exits_with_2 test
 * except we add a call_wapper to the worker. It catches all exceptions and
 * makes the worker exit with code 17. *)
let test_wrapped_worker_with_custom_exit use_clones heap_handle () =
  let workers =
    make_worker
      ~call_wrapper:
        { WorkerController.wrap = catch_exception_and_custom_exit_wrapper }
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
     | WorkerController.Worker_failed
         (_, WorkerController.Worker_quit (Unix.WEXITED i))
     ->
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
     | WorkerController.Worker_failed
         (_, WorkerController.Worker_quit (Unix.WEXITED i))
     ->
       i = 2)

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
      "wrapped_worker_with_custom_exit"
      test_wrapped_worker_with_custom_exit

let () =
  Daemon.check_entry_point ();

  (* this call might not return *)
  let heap_handle = SharedMem.init ~num_workers SharedMem.default_config in
  let tests = make_tests heap_handle in
  Unit_test.run_all tests
