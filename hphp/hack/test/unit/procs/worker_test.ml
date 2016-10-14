let entry = Worker.register_entry_point ~restore:(fun () -> ())

let make_worker heap_handle =
  Worker.make
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

let test_simple_worker_spawn heap_handle () =
  let workers = make_worker heap_handle in
  match workers with
  | [] ->
    Printf.eprintf "Failed to create workers";
    false
  | worker :: workers ->
    let handle = Worker.call worker (fun () -> "hello") () in
    let handle = wait_until_ready handle in
    let result = Worker.get_result handle in
    if result = "hello" then true
    else false

let make_tests handle = [
  "simple_worker_spawn_test", test_simple_worker_spawn handle;
]

let () =
  Daemon.check_entry_point (); (* this call might not return *)
  let heap_handle = SharedMem.init GlobalConfig.default_sharedmem_config in
  let tests = make_tests heap_handle in
  Unit_test.run_all tests
