module Hack_bucket = Bucket
open Hh_prelude
module Bucket = Hack_bucket
open Procs_test_utils

let num_workers = 10

let num_jobs = 100000

module IntVal = struct
  type t = int

  let description = "Test_IntVal"
end

module TestHeap =
  SharedMem.Heap
    (SharedMem.ImmediateBackend (SharedMem.NonEvictable)) (StringKey)
    (IntVal)

let sum acc x = acc + x

let do_work (acc : int) (jobs : int list) : int =
  (* Since worker clones are forked from the same process that doesn't do any calls to
   * random, the per-clone sequences are not really random without injecting a
   * bit more of enthropy. *)
  Random.self_init ();

  (* Ensure that at least some workers will get killed in the middle of the
   * job (with high probability) *)
  if Random.int 100 = 0 then Unix.sleep 1;
  List.fold_left jobs ~init:acc ~f:(fun acc job ->
      TestHeap.add (string_of_int job) job;
      sum acc job)

let rec make_work acc = function
  | 0 -> acc
  | x -> make_work (x :: acc) (x - 1)

let make_work () = make_work [] num_jobs

let run_interrupter fd_in fd_out =
  Unix.close fd_in;
  while true do
    Unix.sleepf (0.5 +. Random.float 0.1);
    let written = Unix.write fd_out (Bytes.of_string "!") 0 1 in
    assert (written = 1)
  done;
  assert false

let interrupt_handler fd acc =
  let exclamation_mark = Bytes.create 1 in
  (try
     while true do
       let (ready, _, _) = Unix.select [fd] [] [] 0.0 in
       if List.is_empty ready then raise Stdlib.Not_found;
       let read = Unix.read fd exclamation_mark 0 1 in
       assert (read = 1 && String.equal (Bytes.to_string exclamation_mark) "!")
     done
   with
  | Stdlib.Not_found -> ());
  ( acc,
    MultiThreadedCall.Cancel
      {
        MultiThreadedCall.user_message = "cancel";
        log_message = "";
        timestamp = Unix.gettimeofday ();
      } )

let rec run_until_done fd_in workers (acc, iterations) = function
  | [] -> (acc, iterations)
  | work ->
    Hh_logger.log "Left: %d" (List.length work);
    let (result, (), unfinished_and_reason) =
      MultiWorker.call_with_interrupt
        (Some workers)
        ~job:do_work
        ~merge:sum
        ~neutral:0
        ~next:(Bucket.make ~num_workers ~max_size:10 work)
        ~interrupt:
          {
            MultiThreadedCall.handlers =
              (fun () -> [(fd_in, interrupt_handler fd_in)]);
            env = ();
          }
    in
    let unfinished =
      match unfinished_and_reason with
      | Some (unfinished, _reason) -> List.concat unfinished
      | None -> []
    in
    run_until_done fd_in workers (sum acc result, iterations + 1) unfinished

(* Might raise because of Option.value_exn *)
let rec sum_memory acc = function
  | [] -> acc
  | x :: xs ->
    sum_memory (acc + Option.value_exn (TestHeap.get (string_of_int x))) xs

let multi_worker_cancel workers () =
  let work = make_work () in
  let (fd_in, fd_out) = Unix.pipe () in
  let interrupter_pid =
    match Unix.fork () with
    | 0 -> run_interrupter fd_in fd_out
    | pid -> pid
  in
  Unix.close fd_out;
  let t = Unix.gettimeofday () in
  let (result, iterations) = run_until_done fd_in workers (0, 0) work in
  let duration = Unix.gettimeofday () -. t in
  let memory_result = sum_memory 0 work in
  Unix.close fd_in;
  ignore @@ Unix.waitpid [] interrupter_pid;

  let expected = num_jobs * (num_jobs + 1) / 2 in
  (* behold... MATH! *)
  Printf.printf
    "Result: %d, memory result: %d in %f\n"
    memory_result
    result
    duration;

  (* Check that at least some cancellations have happened *)
  assert (iterations > 1);
  assert (result = expected && memory_result = expected);
  true

let tests = [("multi_worker_cancel", multi_worker_cancel)]

let () =
  Daemon.check_entry_point ();
  let workers = make_workers num_workers in
  try_finalize
    Unit_test.run_all
    (List.map ~f:(fun (n, t) -> (n, t workers)) tests)
    cleanup
    ()
