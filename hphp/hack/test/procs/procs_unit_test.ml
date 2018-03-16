open Procs_test_utils

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

let multi_worker_one_worker_throws _workers () =
  (** When a worker fails, the rest in the same MultiWorker call can't
   * be reused right now. So let's use fresh workers for this unit test
   * instead of corrupted the shared workers. *)
  let workers = make_workers 10 in
  let split ~bucket = bucket + 1 in
  let open Bucket in
  let do_work _ bucket =
    if (bucket.work = 5) then
      (** Bucket number 5 exits abnormally. *)
      exit 3
    else
      bucket.work
  in
  try begin
    let _result =
      MultiWorker.call (Some workers)
        ~job:do_work
        ~merge:(+)
        ~neutral:0
        ~next:(make_n_buckets ~buckets:20 ~split)
    in
    false
  end with
  | MultiThreadedCall.Coalesced_failures [ Unix.WEXITED 3 ] ->
    true

let multi_worker_with_failure_handler _workers () =
  (** Spawn new workers since some will be failing and the rest non-reusable. *)
  let workers = make_workers 10 in
  let split ~bucket =
    (** We want the first 5 buckets to all have exited while the MultiWorker result
     * is being collected. But that's racy. Since the work is fanned out
     * statically, we can ensure the first 5 have all "failed completely" by
     * sleeping when handing out the 6th job. *)
    if bucket = 5 then
      let () = Unix.sleep 1 in
      bucket + 1
    else
      bucket + 1 in
  let open Bucket in
  let do_work _ _ =
    (** Each bucket exits abnormally with exit code 3*)
    exit 3
  in
  try
    let _ = MultiWorker.call
      (Some workers)
      ~job:do_work
      ~merge:(+)
      ~neutral:0
      ~next:(make_n_buckets ~buckets:10 ~split)
    in
    Printf.eprintf "Expected MultiWorker.call to throw, but it didn't!\n";
    false
  with
  | MultiThreadedCall.Coalesced_failures failures ->
    (** Every single bucket failed, and we should have processed all of them since
     * we have 10 worker processes. *)
    assert ((List.length failures) = 5);
    let sum = List.fold_left (
      fun acc e -> match e with
      | Unix.WEXITED 3 ->
        acc + 3
      | _ ->
        failwith "Unexpected worker exit") 0 failures
    in
    (** Sum of all exit codes (each exit 3) is 15. *)
    sum = 15

let tests =
  [
    "multi_worker_list", multi_worker_list;
    "multi_worker_bucket", multi_worker_bucket;
    "multi_worker_of_n_buckets", multi_worker_of_n_buckets;
    "multi_worker_one_worker_throws", multi_worker_one_worker_throws;
    "multi_worker_with_failure_handler", multi_worker_with_failure_handler;
  ]

let () =
  Daemon.check_entry_point (); (* this call might not return *)
  let workers = make_workers 10 in
  try_finalize
    Unit_test.run_all (List.map (fun (n, t) -> n, t workers) tests)
    cleanup ()
