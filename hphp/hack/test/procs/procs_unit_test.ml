let entry = Worker.register_entry_point ~restore:(fun _ -> ())

let sum acc elements =
  List.fold_left (fun acc elem -> acc + elem) acc elements

let multi_worker_list workers () =
  let work = [1;2;3;4;5;6;7;8;9;10;11;12;13] in
  let expected = List.length work * (List.length work + 1) / 2 in
  let result =
    MultiWorker.call (Some workers)
      ~job:sum
      ~merge:(+)
      ~neutral:0
      ~next:(Bucket.make ~num_workers:20 work)
  in
  Printf.printf "Got %d\n" result;
  result = expected

let multi_worker_bucket workers () =
  let buckets = 20 in
  let next =
    let counter = ref 1
    in
    fun () ->
      let current = !counter in
      counter := !counter + 1;
      if current <= buckets then Bucket.Job current else Bucket.Done
  in
  let expected = buckets * (buckets + 1) / 2 in
  let result =
    MultiWorker.call (Some workers)
      ~job:(+)
      ~merge:(+)
      ~neutral:0
      ~next:next
  in
  Printf.printf "Got %d\n" result;
  result = expected

let multi_worker_of_n_buckets workers () =
  let buckets = 20 in
  let split ~bucket = bucket + 1 in
  let expected = buckets * (buckets + 1) / 2 in
  let open Bucket in
  let do_work _ bucket =
    assert (bucket.work = bucket.bucket + 1);
    bucket.work
  in
  let result =
    MultiWorker.call (Some workers)
      ~job:do_work
      ~merge:(+)
      ~neutral:0
      ~next:(make_n_buckets ~buckets ~split)
  in
  Printf.printf "Got %d\n" result;
  result = expected

let tests =
  [
    "multi_worker_list", multi_worker_list;
    "multi_worker_bucket", multi_worker_bucket;
    "multi_worker_of_n_buckets", multi_worker_of_n_buckets;
  ]

let try_finalize f x finally y =
  let res = try f x with exn -> finally y; raise exn in
  finally y;
  res

let cleanup _ =
  Worker.killall ()

let () =
  Daemon.check_entry_point (); (* this call might not return *)
  let config = ServerConfig.default_config in
  let shared_config = ServerConfig.(sharedmem_config config) in
  let handle = SharedMem.init shared_config in
  let workers = Worker.make handle entry 10
      ServerConfig.(gc_control config) handle
  in
  SharedMem.connect handle ~is_master:true;
  try_finalize
    Unit_test.run_all (List.map (fun (n, t) -> n, t workers) tests)
    cleanup handle
