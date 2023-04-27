(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

let init
    (ctx : Provider_context.t)
    (workers : MultiWorker.worker list option)
    ~(worker_key : string)
    ~(nonce : Int64.t)
    ~(check_id : string)
    ~(ci_info : Ci_util.info option Future.t option)
    ~(init_id : string)
    ~(init_start_t : float)
    ~(bin_root : Path.t)
    ~(root : Path.t)
    ~(cache_remote_decls : bool)
    ~(use_shallow_decls_saved_state : bool)
    ~(saved_state_manifold_path : string option)
    ~(shallow_decls_manifold_path : string option) : unit =
  let (server
        : (module RemoteWorker.RemoteServerApi
             with type naming_table = Naming_table.t option)) =
    ServerApi.make_remote_server_api ctx workers root
  in
  let (worker_env : Naming_table.t option RemoteWorker.work_env) =
    RemoteWorker.make_env
      ctx
      ~bin_root
      ~check_id
      ~ci_info
      ~init_id
      ~init_start_t
      ~key:worker_key
      ~nonce
      ~root
      ~cache_remote_decls
      ~use_shallow_decls_saved_state
      ~saved_state_manifold_path
      ~shallow_decls_manifold_path
      server
  in
  RemoteWorker.go worker_env
