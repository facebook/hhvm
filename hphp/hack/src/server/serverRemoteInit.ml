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
    ~(check_id : string)
    ~(recli_version : string)
    ~(transport_channel : string option)
    ~(file_system_mode : ArtifactStore.file_system_mode)
    ~(ci_info : Ci_util.info option Future.t option)
    ~(init_id : string)
    ~(init_start_t : float)
    ~(bin_root : Path.t)
    ~(root : Path.t) : unit =
  let (server
        : (module RemoteWorker.RemoteServerApi
             with type naming_table = Naming_table.t option)) =
    ServerApi.make_remote_server_api ctx workers
  in
  let artifact_store_config =
    let open ArtifactStore in
    let config =
      default_config ~recli_version ~temp_dir:(Path.make GlobalConfig.tmp_dir)
    in
    { config with mode = file_system_mode }
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
      ~transport_channel
      ~root
      artifact_store_config
      server
  in
  RemoteWorker.go worker_env
