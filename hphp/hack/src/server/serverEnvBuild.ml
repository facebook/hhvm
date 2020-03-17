(*
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

(*****************************************************************************)
(* Building the environment *)
(*****************************************************************************)
module Hh_bucket = Bucket
open Core_kernel
open ServerEnv
module SLC = ServerLocalConfig
module J = Hh_json_helpers.AdhocJsonHelpers

let hg_dirname = J.strlist ["dirname"; ".hg"]

let git_dirname = J.strlist ["dirname"; ".git"]

let svn_dirname = J.strlist ["dirname"; ".svn"]

let watchman_expression_terms =
  [
    J.strlist ["type"; "f"];
    J.pred "anyof"
    @@ [
         J.strlist ["name"; ".hhconfig"];
         J.pred "anyof"
         @@ [
              J.strlist ["suffix"; "php"];
              J.strlist ["suffix"; "phpt"];
              J.strlist ["suffix"; "hack"];
              J.strlist ["suffix"; "hck"];
              J.strlist ["suffix"; "hh"];
              J.strlist ["suffix"; "hhi"];
              J.strlist ["suffix"; "xhp"];
            ];
       ];
    J.pred "not" @@ [J.pred "anyof" @@ [hg_dirname; git_dirname; svn_dirname]];
  ]

type changes_mode =
  | Check_mode
  | Dfind_mode
  | Watchman_mode of Watchman.env

let make_genv options config local_config workers =
  let root = ServerArgs.root options in
  let check_mode = ServerArgs.check_mode options in
  Typing_deps.trace :=
    (not check_mode)
    || ServerArgs.save_filename options <> None
    || ServerArgs.save_with_spec options <> None;

  (* The number of workers is set both in hh.conf and as an optional server argument.
    if the two numbers given in argument and in hh.conf are different, we always take the minimum
    of the two.
  *)
  let ( >>= ) = Option.( >>= ) in
  let since_clockspec =
    ServerArgs.with_saved_state options >>= function
    | ServerArgs.Saved_state_target_info _ -> None
    | ServerArgs.Informant_induced_saved_state_target target ->
      target.ServerMonitorUtils.watchman_mergebase >>= fun mb ->
      Some mb.ServerMonitorUtils.watchman_clock
  in
  let watchman_env =
    if check_mode then (
      Hh_logger.log "Not using dfind or watchman";
      Check_mode
    ) else if not local_config.SLC.use_watchman then (
      Hh_logger.log "Using dfind";
      Dfind_mode
    ) else (
      Hh_logger.log "Using watchman";
      let watchman_env =
        Watchman.init
          ?since_clockspec
          {
            Watchman.init_timeout =
              Watchman.Explicit_timeout
                (float local_config.SLC.watchman_init_timeout);
            subscribe_mode =
              ( if local_config.SLC.watchman_subscribe then
                Some Watchman.Defer_changes
              else
                None );
            expression_terms = watchman_expression_terms;
            debug_logging =
              ServerArgs.watchman_debug_logging options
              || local_config.SLC.watchman_debug_logging;
            subscription_prefix = "hh_type_check_watcher";
            roots = [root];
          }
          ()
      in
      match watchman_env with
      | Some watchman_env -> Watchman_mode watchman_env
      | None -> Dfind_mode
    )
  in
  let max_bucket_size = local_config.SLC.max_bucket_size in
  Hh_bucket.set_max_bucket_size max_bucket_size;
  let ( indexer,
        notifier_async,
        notifier_async_reader,
        notifier,
        wait_until_ready,
        options ) =
    match watchman_env with
    | Watchman_mode watchman_env ->
      let indexer filter =
        let files = Watchman.get_all_files watchman_env in
        Hh_bucket.make_list
          ~num_workers:(List.length workers)
          ~max_size:max_bucket_size
          (List.filter ~f:filter files)
      in
      (* Watchman state can change during requests (See
       * Watchamn.Watchman_dead and Watchman_alive). We need to update
       * a reference to the new instance. *)
      let watchman = ref (Watchman.Watchman_alive watchman_env) in
      ServerNotifierTypes.(
        let on_changes = function
          | Watchman.Changed_merge_base _ ->
            let () =
              Hh_logger.log
                "Error: Typechecker does not use Source Control Aware mode"
            in
            raise Exit_status.(Exit_with Watchman_invalid_result)
          | Watchman.State_enter (name, metadata) ->
            if local_config.SLC.hg_aware then
              ServerRevisionTracker.on_state_enter name;
            Notifier_state_enter (name, metadata)
          | Watchman.State_leave (name, metadata) ->
            if local_config.SLC.hg_aware then
              ServerRevisionTracker.on_state_leave root name metadata;
            Notifier_state_leave (name, metadata)
          | Watchman.Files_changed changes ->
            ServerRevisionTracker.files_changed
              local_config
              (SSet.cardinal changes);
            Notifier_async_changes changes
        in
        let concat_changes_list =
          List.fold_left
            ~f:
              begin
                fun acc changes ->
                match on_changes changes with
                | Notifier_unavailable
                | Notifier_state_enter _
                | Notifier_state_leave _ ->
                  acc
                | Notifier_synchronous_changes changes
                | Notifier_async_changes changes ->
                  SSet.union acc changes
              end
            ~init:SSet.empty
        in
        let notifier_async () =
          let (watchman', changes) = Watchman.get_changes !watchman in
          watchman := watchman';
          match changes with
          | Watchman.Watchman_unavailable -> Notifier_unavailable
          | Watchman.Watchman_pushed changes -> on_changes changes
          | Watchman.Watchman_synchronous changes ->
            Notifier_synchronous_changes (concat_changes_list changes)
        in
        let notifier_async_reader () = Watchman.get_reader !watchman in
        let notifier () =
          let (watchman', changes) =
            (* Timeout is arbitrary. We just use 30 seconds for now. *)
            Watchman.get_changes_synchronously
              ~timeout:local_config.SLC.watchman_synchronous_timeout
              !watchman
          in
          watchman := watchman';
          concat_changes_list changes
        in
        HackEventLogger.set_use_watchman ();

        (* The initial watch-project command blocks until watchman's crawl is
         * done, so we don't have anything else to wait for here. *)
        let wait_until_ready () = () in
        ( indexer,
          notifier_async,
          notifier_async_reader,
          notifier,
          wait_until_ready,
          options ))
    | Dfind_mode ->
      (* Failed to start Watchman subscription. Clear out the watchman_mergebase
       * inside the Informant-directed target mini state since it is no longer
       * usable during init. *)
      let options =
        match ServerArgs.with_saved_state options with
        | None -> options
        | Some (ServerArgs.Saved_state_target_info _) -> options
        | Some (ServerArgs.Informant_induced_saved_state_target target) ->
          ServerArgs.set_saved_state_target
            options
            (Some { target with ServerMonitorUtils.watchman_mergebase = None })
      in
      let indexer filter = Find.make_next_files ~name:"root" ~filter root in
      let in_fd = Daemon.null_fd () in
      let log_link = ServerFiles.dfind_log root in
      let log_file = Sys_utils.make_link_of_timestamped log_link in
      let log_fd = Daemon.fd_of_path log_file in
      let dfind =
        DfindLib.init
          (in_fd, log_fd, log_fd)
          (GlobalConfig.scuba_table_name, [root])
      in
      let notifier () =
        let set =
          try
            Timeout.with_timeout
              ~timeout:120
              ~on_timeout:(fun () -> Exit_status.(exit Dfind_unresponsive))
              ~do_:(fun t -> DfindLib.get_changes ~timeout:t dfind)
          with _ -> Exit_status.(exit Dfind_died)
        in
        set
      in
      let ready = ref false in
      let wait_until_ready () =
        if !ready then
          ()
        else (
          DfindLib.wait_until_ready dfind;
          ready := true
        )
      in
      ( indexer,
        (fun () ->
          ServerNotifierTypes.Notifier_synchronous_changes (notifier ())),
        (fun () -> None),
        notifier,
        wait_until_ready,
        options )
    | Check_mode ->
      let wait_until_ready () = () in
      let notifier () = SSet.empty in
      let notifier_async () =
        ServerNotifierTypes.Notifier_synchronous_changes (notifier ())
      in
      let notifier_async_reader () = None in
      let indexer filter = Find.make_next_files ~name:"root" ~filter root in
      ( indexer,
        notifier_async,
        notifier_async_reader,
        notifier,
        wait_until_ready,
        options )
  in
  {
    options;
    config;
    local_config;
    workers = Some workers;
    indexer;
    notifier_async;
    notifier_async_reader;
    notifier;
    wait_until_ready;
    debug_channels = None;
  }

(* useful in testing code *)
let default_genv =
  {
    options = ServerArgs.default_options "";
    config = ServerConfig.default_config;
    local_config = ServerLocalConfig.default;
    workers = None;
    indexer = (fun _ () -> []);
    notifier_async =
      (fun () -> ServerNotifierTypes.Notifier_synchronous_changes SSet.empty);
    notifier_async_reader = (fun () -> None);
    notifier = (fun () -> SSet.empty);
    wait_until_ready = (fun () -> ());
    debug_channels = None;
  }

let make_env ?init_id config =
  let tcopt = ServerConfig.typechecker_options config in
  let max_batch_size = TypecheckerOptions.remote_max_batch_size tcopt in
  let min_batch_size = TypecheckerOptions.remote_min_batch_size tcopt in
  {
    tcopt;
    popt = ServerConfig.parser_options config;
    gleanopt = ServerConfig.glean_options config;
    swriteopt = ServerConfig.symbol_write_options config;
    naming_table = Naming_table.empty;
    typing_service =
      {
        delegate_state =
          Typing_service_delegate.create ~max_batch_size ~min_batch_size ();
        enabled = false;
      };
    errorl = Errors.empty;
    failed_naming = Relative_path.Set.empty;
    persistent_client = None;
    ide_idle = false;
    last_command_time = 0.0;
    last_notifier_check_time = 0.0;
    last_idle_job_time = 0.0;
    editor_open_files = Relative_path.Set.empty;
    ide_needs_parsing = Relative_path.Set.empty;
    disk_needs_parsing = Relative_path.Set.empty;
    needs_phase2_redecl = Relative_path.Set.empty;
    needs_recheck = Relative_path.Set.empty;
    full_recheck_on_file_changes = Not_paused;
    remote = false;
    full_check = Full_check_done;
    changed_files = Relative_path.Set.empty;
    prechecked_files = Prechecked_files_disabled;
    can_interrupt = true;
    interrupt_handlers = (fun _ _ -> []);
    pending_command_needs_writes = None;
    persistent_client_pending_command_needs_full_check = None;
    default_client_pending_command_needs_full_check = None;
    init_env =
      {
        approach_name = "";
        ci_info = None;
        init_error = None;
        init_id =
          (match init_id with
          | Some init_id -> init_id
          | None -> Random_id.short_string ());
        init_start_t = Unix.gettimeofday ();
        init_type = "";
        mergebase = None;
        needs_full_init = false;
        recheck_id = None;
        state_distance = None;
      };
    diag_subscribe = None;
    recent_recheck_loop_stats = empty_recheck_loop_stats;
    last_recheck_info = None;
    local_symbol_table = SearchUtils.default_si_env;
  }
