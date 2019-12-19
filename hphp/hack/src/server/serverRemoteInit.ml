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
    ~(root : Path.t) : unit =
  let (server
        : (module RemoteWorker.RemoteServerApi
             with type naming_table = Naming_table.t option)) =
    ServerApi.make_remote_server_api workers tcopt
  in
  let (worker_env : Naming_table.t option RemoteWorker.work_env) =
    RemoteWorker.
      {
        bin_root;
        key = worker_key;
        check_id;
        naming_table_base = None;
        root;
        timeout = 600;
        server;
      }
  in
  RemoteWorker.go worker_env
