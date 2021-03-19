open Hh_prelude

let entry =
  WorkerController.register_entry_point ~restore:(fun _ ~(worker_id : int) ->
      Hh_logger.set_id (Printf.sprintf "procs_test_utils %d" worker_id))

let try_finalize f x finally y =
  let res =
    try f x
    with exn ->
      finally y;
      raise exn
  in
  finally y;
  res

let use_worker_clones =
  Array.length Sys.argv < 2 || not (String.equal Sys.argv.(1) "NO_CLONES")

let make_workers n =
  let handle = SharedMem.init ~num_workers:n SharedMem.default_config in
  let workers =
    MultiWorker.make
      (not use_worker_clones) (* longlived_workers *)
      handle
      entry
      n
      GlobalConfig.gc_control
      handle
  in
  workers

let cleanup () = WorkerController.force_quit_all ()

let run_interrupter limit =
  let (fd_in, fd_out) = Unix.pipe () in
  let interrupter_pid =
    match Unix.fork () with
    | 0 ->
      Unix.close fd_in;
      let rec aux x =
        match x with
        | Some 0 -> exit 0
        | _ ->
          let written = Unix.write fd_out (Bytes.of_string "!") 0 1 in
          assert (written = 1);
          aux (Option.map x (fun x -> x - 1))
      in
      aux limit
    | pid -> pid
  in
  Unix.close fd_out;
  (fd_in, interrupter_pid)

let read_exclamation_mark fd =
  let exclamation_mark = Bytes.create 1 in
  let read = Unix.read fd exclamation_mark 0 1 in
  assert (read = 1 && String.equal (Bytes.to_string exclamation_mark) "!")
