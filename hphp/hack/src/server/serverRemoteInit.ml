(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

let init
    (workers : MultiWorker.worker list option)
    (tcopt : TypecheckerOptions.t)
    ~(worker_key : string)
    ~(check_id : string)
    ~(bin_root : Path.t)
    ~(root : Path.t) : Errors.t * float =
  let t = Unix.gettimeofday () in
  (* Set up the type checking callback *)
  let type_check files_to_check =
    Typing_check_service.(
      let check_info =
        {
          init_id = Random_id.short_string ();
          recheck_id = None;
          profile_log = false;
          profile_type_check_twice = false;
        }
      in
      let delegate_state = Typing_service_delegate.create () in
      let (errors, _delegate_state) =
        go
          workers
          delegate_state
          tcopt
          Relative_path.Set.empty
          files_to_check
          ~memory_cap:None
          ~check_info
      in
      errors)
  in
  (* Prepare the input for the remote worker *)
  let (worker_env : RemoteWorker.work_env) =
    RemoteWorker.
      {
        bin_root;
        key = worker_key;
        check_id;
        root;
        timeout = 9999;
        type_check = Some type_check;
      }
  in
  Hh_logger.log "Remote: begin type checking";
  let errors = RemoteWorker.go worker_env in
  let hs = SharedMem.heap_size () in
  Hh_logger.log "Heap size: %d" hs;

  (errors, Hh_logger.log_duration "Finished type checking" t)
