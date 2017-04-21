let entry = Worker.register_entry_point ~restore:(fun () -> ())

let make_worker ?call_wrapper heap_handle =
  Worker.make
    ?call_wrapper
    ~saved_state:()
    ~entry
    ~nbr_procs:2
    ~gc_control:(Gc.get())
    ~heap_handle

let rec wait_until_ready handle =
  let { Worker.readys; waiters } = Worker.select [handle] in
  match readys with
  | [] -> wait_until_ready handle
  | ready :: _ ->
    ready

(** If "f x" throws, we exit the program with a custom exit code. *)
let catch_exception_and_custom_exit_wrapper: 'x 'b. ('x -> 'b) -> 'x -> 'b = fun f x ->
  try f x with
  | _ -> exit 17

let call_and_verify_result worker f x expected =
  let result = Worker.call worker f x
    |> wait_until_ready
    |> Worker.get_result
  in
  result = expected

(** This is just like the test_worker_uncaught_exception_exits_with_2 test
 * except we add a call_wapper to the worker. It catches all exceptions and
 * makes the worker exit with code 17. *)
let test_wrapped_worker_with_custom_exit heap_handle () =
  let workers = make_worker
    ~call_wrapper:{ Worker.wrap = catch_exception_and_custom_exit_wrapper }
    heap_handle in
  match workers with
  | [] ->
    Printf.eprintf "Failed to create workers";
    false
  | worker :: workers ->
    try call_and_verify_result worker
    (fun () -> raise (Failure "oops")) () "dummy" with
    | Worker.Worker_exited_abnormally i ->
      i = 17

let test_worker_uncaught_exception_exits_with_2 heap_handle () =
  let workers = make_worker heap_handle in
  match workers with
  | [] ->
    Printf.eprintf "Failed to create workers";
    false
  | worker :: workers ->
    try call_and_verify_result worker (fun () -> raise (Failure "oops"))
    () "dummy" with
    | Worker.Worker_exited_abnormally i ->
      i = 2

let test_simple_worker_spawn heap_handle () =
  let workers = make_worker heap_handle in
  match workers with
  | [] ->
    Printf.eprintf "Failed to create workers";
    false
  | worker :: workers ->
    call_and_verify_result worker (fun () -> "hello") () "hello"

let make_tests handle = [
  "simple_worker_spawn_test",
    test_simple_worker_spawn handle;
  "worker_uncaught_exception_exits_with_2",
    test_worker_uncaught_exception_exits_with_2 handle;
  "wrapped_worker_with_custom_exit",
    test_wrapped_worker_with_custom_exit handle;
]

let () =
  Daemon.check_entry_point (); (* this call might not return *)
  let heap_handle = SharedMem.init GlobalConfig.default_sharedmem_config in
  let tests = make_tests heap_handle in
  Unit_test.run_all tests
