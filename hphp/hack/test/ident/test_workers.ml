module Hh_bucket = Bucket (* Bucket is shadowed by Core_kernel *)

open Hh_prelude

module IntKey = struct
  type t = int

  let to_string = string_of_int

  let compare = ( - )
end

module Ids =
  SharedMem.NoCache (SharedMem.Immediate) (IntKey)
    (struct
      type t = int array

      let prefix = Prefix.make ()

      let description = "Test_Ids"
    end)

let entry =
  WorkerControllerEntryPoint.register ~restore:(fun () ~(worker_id : int) ->
      Hh_logger.set_id (Printf.sprintf "test_workers %d" worker_id))

let () =
  Daemon.check_entry_point ();

  let num_workers = 4 in
  let handle =
    SharedMem.init
      ~num_workers
      {
        SharedMem.global_size = 0;
        heap_size = 10 * 1024 * 1024;
        (* 10 MiB *)
        dep_table_pow = 0;
        hash_table_pow = 14;
        (* 256 KiB *)
        shm_dirs = [];
        shm_min_avail = 0;
        log_level = 0;
        sample_rate = 0.0;
        compression = 0;
      }
  in
  let workers =
    MultiWorker.make
      ?call_wrapper:None
      ~longlived_workers:false
      ~saved_state:()
      ~entry
      ~nbr_procs:num_workers
      ~gc_control:(Gc.get ())
      ~heap_handle:handle
  in
  let num_jobs = 100 in
  let ids_per_job = 100 in
  (* Ensure all ids within a job increase monotonically. *)
  let job () job_id =
    let ids = Array.create ~len:ids_per_job 0 in
    let prev = ref 0 in
    for i = 0 to ids_per_job - 1 do
      let id = Ident.tmp () in
      ids.(i) <- id;
      assert (!prev < id);
      prev := id
    done;
    Printf.printf "Adding job %d\n" job_id;
    Ids.add job_id ids;
    ()
  in
  (* Also add some ids from the master process *)
  let () = job () 0 in
  let job_id = ref 1 in
  let next () =
    let i = !job_id in
    if i < num_jobs then (
      incr job_id;
      Hh_bucket.Job i
    ) else
      Hh_bucket.Done
  in
  let merge () () = () in
  MultiWorker.call (Some workers) ~neutral:() ~job ~merge ~next;

  (* Ensure ids are globally unique. *)
  let all_ids_len = num_jobs * ids_per_job in
  let all_ids = Array.create ~len:all_ids_len 0 in
  for job_id = 0 to num_jobs - 1 do
    (* Might raise {!SharedMem.Shared_mem_not_found} *)
    let ids = Ids.find_unsafe job_id in
    for i = 0 to ids_per_job - 1 do
      all_ids.((job_id * ids_per_job) + i) <- ids.(i)
    done
  done;

  Array.sort ~compare:( - ) all_ids;

  let prev = ref 0 in
  for i = 0 to all_ids_len - 1 do
    let id = all_ids.(i) in
    assert (!prev < id);
    prev := id
  done;

  ()
