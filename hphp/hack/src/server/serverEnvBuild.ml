(**
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 *
 *)


(*****************************************************************************)
(* Building the environment *)
(*****************************************************************************)
open ServerEnv

module SLC = ServerLocalConfig

let make_genv options config local_config handle =
  let root = ServerArgs.root options in
  let check_mode   = ServerArgs.check_mode options in
  Typing_deps.trace :=
    not check_mode || ServerArgs.convert options <> None ||
    ServerArgs.save_filename options <> None;
  let gc_control = ServerConfig.gc_control config in
  let workers = Some (ServerWorker.make gc_control handle) in
  let watchman_env =
    if check_mode || not local_config.SLC.use_watchman
    then None
    else Watchman.init {
      Watchman.init_timeout = local_config.SLC.watchman_init_timeout;
      subscribe_to_changes = local_config.SLC.watchman_subscribe;
      sync_directory = local_config.SLC.watchman_sync_directory;
      root = root;
    }
  in
  if Option.is_some watchman_env then Hh_logger.log "Using watchman";
  let indexer, notifier_async, notifier, wait_until_ready =
    match watchman_env with
    | Some watchman_env ->
      let indexer filter =
        let files = Watchman.get_all_files watchman_env in
        Bucket.make
          ~num_workers:GlobalConfig.nbr_procs
          ~max_size:1000
          (List.filter filter files)
      in
      (** Watchman state can change during requests (See
       * Watchamn.Watchman_dead and Watchman_alive). We need to update
       * a reference to the new instance. *)
      let watchman = ref (Watchman.Watchman_alive watchman_env) in
      let notifier_async () =
        let watchman', changes = Watchman.get_changes !watchman in
        watchman := watchman';
        let open ServerNotifierTypes in
        match changes with
        | Watchman.Watchman_unavailable -> Notifier_unavailable
        | Watchman.Watchman_pushed changes -> begin match changes with
          | Watchman.State_enter _
          | Watchman.State_leave _ ->
            Notifier_async_changes SSet.empty
          | Watchman.Files_changed changes ->
            Notifier_async_changes changes
          end
        | Watchman.Watchman_synchronous changes ->
          Notifier_synchronous_changes changes
      in
      let notifier () =
        let watchman', changes =
          (** Timeout is arbitrary. We just use 30 seconds for now. *)
          Watchman.get_changes_synchronously ~timeout:30 !watchman in
        watchman := watchman';
        changes
      in
      HackEventLogger.set_use_watchman ();
      (* The initial watch-project command blocks until watchman's crawl is
       * done, so we don't have anything else to wait for here. *)
      let wait_until_ready () = () in
      indexer, notifier_async, notifier, wait_until_ready
    | None ->
      let indexer filter = Find.make_next_files ~name:"root" ~filter root in
      let log_link = ServerFiles.dfind_log root in
      let log_file = Sys_utils.make_link_of_timestamped log_link in
      let log_fd = Daemon.fd_of_path log_file in
      let dfind = DfindLib.init
        (log_fd, log_fd) (GlobalConfig.scuba_table_name, [root]) in
      let notifier () =
        let set = begin try
          Timeout.with_timeout ~timeout:120
            ~on_timeout:(fun () -> Exit_status.(exit Dfind_unresponsive))
            ~do_:(fun t -> DfindLib.get_changes ~timeout:t dfind)
        with _ ->
          Exit_status.(exit Dfind_died)
        end in
        set
      in
      let ready = ref false in
      let wait_until_ready () =
        if !ready then ()
        else (DfindLib.wait_until_ready dfind; ready := true)
      in
      indexer, (fun() ->
        ServerNotifierTypes.Notifier_synchronous_changes (notifier ())
        ), notifier, wait_until_ready
  in
  { options;
    config;
    local_config;
    workers;
    indexer;
    notifier_async;
    notifier;
    wait_until_ready;
    debug_channels = None;
  }

(* useful in testing code *)
let default_genv =
  { options          = ServerArgs.default_options "";
    config           = ServerConfig.default_config;
    local_config     = ServerLocalConfig.default;
    workers          = None;
    indexer          = (fun _ -> fun () -> []);
    notifier_async   = (fun () ->
      ServerNotifierTypes.Notifier_synchronous_changes SSet.empty);
    notifier         = (fun () -> SSet.empty);
    wait_until_ready = (fun () -> ());
    debug_channels   = None;
  }

let make_env config =
  { tcopt          = ServerConfig.typechecker_options config;
    popt           = ServerConfig.parser_options config;
    files_info     = Relative_path.Map.empty;
    errorl         = Errors.empty;
    failed_parsing = Relative_path.Set.empty;
    failed_naming  = Relative_path.Set.empty;
    failed_decl    = Relative_path.Set.empty;
    failed_check   = Relative_path.Set.empty;
    persistent_client = None;
    last_command_time = 0.0;
    last_notifier_check_time = 0.0;
    last_idle_job_time = 0.0;
    edited_files   = Relative_path.Map.empty;
    ide_needs_parsing = Relative_path.Set.empty;
    disk_needs_parsing = Relative_path.Set.empty;
    needs_decl = Relative_path.Set.empty;
    needs_check = Relative_path.Set.empty;
    needs_full_check = false;
    diag_subscribe = None;
    recent_recheck_loop_stats = empty_recheck_loop_stats;
  }
