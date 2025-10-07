(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Hh_prelude

type changes =
  | Unavailable
      (** e.g. because DFind is not available, or watchman subscription is down *)
  | SyncChanges of SSet.t
      (** contains all changes up to the point that the notifier was invoked *)
  | AsyncChanges of SSet.t
      (** contains some of the changes up to the point that the notifier was invoked,
          but there may be more pending changes that have not been included *)

type clock = ServerNotifierTypes.clock =
  | Watchman of Watchman.clock
  | Eden of Edenfs_watcher.clock
[@@deriving show, eq]

let show_file_watcher_name = function
  | Watchman _ -> "Watchman"
  | Eden _ -> "Edenfs_watcher"

let handle_edenfs_watcher_result
    (result : ('t, Edenfs_watcher_types.edenfs_watcher_error) result) : 't =
  match result with
  | Result.Error (Edenfs_watcher_types.EdenfsWatcherError msg) ->
    Hh_logger.log "Edenfs_watcher failed with message: %s" msg;
    raise Exit_status.(Exit_with Edenfs_watcher_failed)
  | Result.Error Edenfs_watcher_types.NonEdenWWW ->
    Hh_logger.log "Edenfs_watcher failed, www repo is not on Eden";
    raise Exit_status.(Exit_with Edenfs_watcher_failed)
  | Result.Error (Edenfs_watcher_types.LostChanges reason) ->
    Hh_logger.log "Edenfs_watcher has lost track of changes, reason: %s" reason;
    raise Exit_status.(Exit_with Edenfs_watcher_lost_changes)
  | Result.Ok value -> value

type t =
  | IndexOnly of { root: Path.t }
  | Dfind of {
      root: Path.t;
      ready: bool ref;
      dfind: DfindLib.t;
    }
  | Watchman of {
      wenv: Watchman.env;
      watchman: Watchman.watchman_instance ref;
          (** Watchman state can change during requests (see Watchamn.Watchman_dead and Watchman_alive).
          This reference will be updated as necessary to the new instance. *)
      root: Path.t;
      local_config: ServerLocalConfig.t;
      num_workers: int;
    }
  | EdenfsFileWatcher of {
      instance: Edenfs_watcher.instance;
      num_workers: int;
      tmp_watchman_instance: Watchman.watchman_instance ref;
          (** Currently, the Eden notification API does not support reporting state transitions (e.g.,
          hg.update, meerkat-build enter/leave events). In the meantime, we use a Watchman instance
          that just listens to these events. *)
      root: Path.t;
      local_config: ServerLocalConfig.t;
      last_clock: Edenfs_watcher.clock ref;
          (** Clock up to which we have gotten changes from the instance.
          Concretely, clock as of the last call to any of the following Edenfs_watcher functions:
          init, get_all_files, get_changes_sync, get_changes_async. *)
    }
  | MockChanges of {
      get_changes_async: unit -> changes;
      get_changes_sync: unit -> SSet.t;
    }

type indexer = (string -> bool) -> unit -> string list

(** This returns an "indexer", i.e. unit -> string list, which when invoked
will return all files under root. *)
let indexer (t : t) (filter : string -> bool) : unit -> string list =
  match t with
  | Dfind { root; _ }
  | IndexOnly { root; _ } ->
    Find.make_next_files ~name:"root" ~filter root
  | MockChanges _ -> failwith "indexer not mocked"
  | Watchman { wenv; num_workers; _ } ->
    let files = Watchman.get_all_files wenv in
    Bucket.make_list ~num_workers (List.filter ~f:filter files)
  | EdenfsFileWatcher { instance; num_workers; last_clock; _ } ->
    let (files, new_clock, _telemetry_opt) =
      Edenfs_watcher.get_all_files instance |> handle_edenfs_watcher_result
    in
    Hh_logger.debug
      "Edenfs_watcher.get_all_files returned %d files"
      (List.length files);
    last_clock := new_clock;
    Bucket.make_list ~num_workers (List.filter ~f:filter files)

let init
    (options : ServerArgs.options)
    (local_config : ServerLocalConfig.t)
    ~(num_workers : int) : t * indexer =
  let root = ServerArgs.root options in
  let watchman_config = local_config.ServerLocalConfig.watchman in
  let watchman_enabled = watchman_config.ServerLocalConfig.Watchman.enabled in
  let edenfs_watcher_config =
    local_config.ServerLocalConfig.edenfs_file_watcher
  in
  let edenfs_watcher_enabled =
    edenfs_watcher_config.ServerLocalConfig.EdenfsFileWatcher.enabled
  in

  let init_dfind () =
    Hh_logger.log "Using dfind";
    let in_fd = Daemon.null_fd () in
    let log_link = ServerFiles.dfind_log root in
    let log_file = Sys_utils.make_link_of_timestamped log_link in
    let log_fd = Daemon.fd_of_path log_file in
    let dfind =
      DfindLib.init
        (in_fd, log_fd, log_fd)
        (GlobalConfig.scuba_table_name, [root])
    in
    HackEventLogger.set_file_watcher_dfind ();
    Dfind { root; ready = ref false; dfind }
  in

  (* helper to try to construct Watchman, or return None if failed *)
  let try_init_watchman () =
    Hh_logger.log "Using watchman";
    let ServerLocalConfig.Watchman.
          { sockname; subscribe; init_timeout; debug_logging; _ } =
      watchman_config
    in

    let wenv =
      Watchman.init
        {
          Watchman.init_timeout = Watchman.Explicit_timeout (float init_timeout);
          subscribe_mode =
            (if subscribe then
              Some Watchman.Defer_changes
            else
              None);
          expression_terms = FilesToIgnore.watchman_server_expression_terms;
          debug_logging =
            ServerArgs.watchman_debug_logging options || debug_logging;
          sockname;
          subscription_prefix = "hh_type_check_watcher";
          roots = [root];
        }
        ()
    in
    Option.map wenv ~f:(fun wenv ->
        HackEventLogger.set_file_watcher_watchman ();
        Watchman
          {
            wenv;
            watchman = ref (Watchman.Watchman_alive wenv);
            root;
            local_config;
            num_workers;
          })
  in

  let try_init_edenfs_watcher () : t option =
    Hh_logger.log "Using EdenFS file watcher";
    let watch_spec = FilesToIgnore.server_watch_spec in
    let {
      ServerLocalConfig.EdenfsFileWatcher.debug_logging;
      timeout_secs;
      throttle_time_ms;
      report_telemetry;
      _;
    } =
      local_config.edenfs_file_watcher
    in
    let init_settings =
      {
        Edenfs_watcher_types.root;
        watch_spec;
        debug_logging;
        timeout_secs;
        throttle_time_ms;
        report_telemetry;
      }
    in
    match Edenfs_watcher.init init_settings with
    | Result.Error (Edenfs_watcher_types.EdenfsWatcherError msg) ->
      Hh_logger.log
        "Failed to initialize EdenFS watcher, failed with message:\n%s"
        msg;
      HackEventLogger.edenfs_watcher_fallback ~msg;
      None
    | Result.Error Edenfs_watcher_types.NonEdenWWW ->
      let msg =
        Printf.sprintf
          "Failed to initialize EdenFS watcher, www repo %s is not on Eden"
          (Path.to_string root)
      in
      Hh_logger.log "%s" msg;
      HackEventLogger.edenfs_watcher_fallback ~msg;
      None
    | Result.Error (Edenfs_watcher_types.LostChanges reason) ->
      let msg =
        Printf.sprintf
          "Failed to initialize EdenFS watcher with lost changes message, reason %s"
          reason
      in
      Hh_logger.log "%s" msg;
      HackEventLogger.edenfs_watcher_fallback ~msg;
      None
    | Result.Ok (instance, initial_clock) ->
      let last_clock = ref initial_clock in

      (* This is just temporary:
         For now, we also carry around a Watchman instance.
         This is just used to get hg and meerkat state change updates. *)
      let ServerLocalConfig.Watchman.
            { sockname; init_timeout; debug_logging; _ } =
        watchman_config
      in
      (* This Watchman instance doesn't care about file changes at all. *)
      let expression_terms =
        [Hh_json_helpers.AdhocJsonHelpers.strlist ["false"]]
      in
      let watchman_env =
        Watchman.init
          {
            Watchman.init_timeout =
              Watchman.Explicit_timeout (float init_timeout);
            subscribe_mode = Some Watchman.Defer_changes;
            expression_terms;
            debug_logging =
              ServerArgs.watchman_debug_logging options || debug_logging;
            sockname;
            subscription_prefix = "hh_type_check_state_transition_watcher";
            roots = [root];
          }
          ()
      in
      (match watchman_env with
      | Some watchman_env ->
        let tmp_watchman_instance =
          ref (Watchman.Watchman_alive watchman_env)
        in
        HackEventLogger.set_file_watcher_edenfs ();
        Some
          (EdenfsFileWatcher
             {
               instance;
               tmp_watchman_instance;
               num_workers;
               root;
               local_config;
               last_clock;
             })
      | None ->
        let msg = "Failed to initialize Watchman event watching instance" in
        HackEventLogger.edenfs_watcher_fallback ~msg;
        None)
  in

  if edenfs_watcher_enabled && watchman_enabled then
    Hh_logger.warn
      "Both Watchman and EdenFS file watching enabled in server config, will prefer the latter";

  (* We just use these lazy values to make the initialization logic less branchy. *)
  let lazy_edenfs_watcher = lazy (try_init_edenfs_watcher ()) in
  let lazy_watchman = lazy (try_init_watchman ()) in

  let notifier =
    if ServerArgs.check_mode options then (
      (* check_mode *)
      Hh_logger.log "Not using any file watching mechanism";
      IndexOnly { root }
    ) else if
        edenfs_watcher_enabled
        && (Option.is_some @@ Lazy.force lazy_edenfs_watcher)
      then
      (* This value_exn cannot fail, we just checked that this is Some in the previous line *)
      Option.value_exn @@ Lazy.force lazy_edenfs_watcher
    else if watchman_enabled && (Option.is_some @@ Lazy.force lazy_watchman)
    then
      (* This value_exn cannot fail, we just checked that this is Some in the previous line *)
      Option.value_exn @@ Lazy.force lazy_watchman
    else
      init_dfind ()
  in

  (notifier, indexer notifier)

let init_mock
    ~(get_changes_async : unit -> changes) ~(get_changes_sync : unit -> SSet.t)
    : t =
  MockChanges { get_changes_async; get_changes_sync }

let init_null () : t =
  let f () = SyncChanges SSet.empty in
  let g () = SSet.empty in
  init_mock ~get_changes_async:f ~get_changes_sync:g

let wait_until_ready (t : t) : unit =
  match t with
  | Dfind { ready; dfind; _ } ->
    if !ready then
      ()
    else begin
      DfindLib.wait_until_ready dfind;
      ready := true
    end
  | IndexOnly _ -> ()
  | MockChanges _ -> ()
  | Watchman _ ->
    (* The initial watch-project command blocks until watchman's crawl is
       done, so we don't have anything else to wait for here. *)
    ()
  | EdenfsFileWatcher _ ->
    (* Same as for Watchman *)
    ()

(** Helper conversion function, from a single watchman-changes to a set of changed
    files. Also handles informing ServerRevisionTracker about changes *)
let convert_watchman_changes
    ~(root : Path.t)
    ~(local_config : ServerLocalConfig.t)
    (watchman_changes : Watchman.pushed_changes) : SSet.t =
  match watchman_changes with
  | Watchman.Changed_merge_base _ ->
    let () =
      Hh_logger.log "Error: Typechecker does not use Source Control Aware mode"
    in
    raise Exit_status.(Exit_with Watchman_invalid_result)
  | Watchman.State_enter (name, _metadata) ->
    if local_config.ServerLocalConfig.hg_aware then
      ServerRevisionTracker.on_state_enter name;
    SSet.empty
  | Watchman.State_leave (name, metadata) ->
    if local_config.ServerLocalConfig.hg_aware then
      ServerRevisionTracker.on_state_leave root name metadata;
    SSet.empty
  | Watchman.Files_changed changes ->
    ServerRevisionTracker.files_changed local_config (SSet.cardinal changes);
    changes

let convert_edenfs_watcher_changes
    local_config (eden_changes : Edenfs_watcher_types.changes) : SSet.t =
  let changed_files =
    match eden_changes with
    | Edenfs_watcher_types.CommitTransition { file_changes; _ } ->
      (* TODO(T224461521) Need to inform ServerRevisionTracker about commit
         transition, similarly to what convert_watchman_changes does *)
      SSet.of_list file_changes
    | Edenfs_watcher_types.FileChanges file_changes ->
      (* TODO(T215219438) Need to inform ServerRevisionTracker about changed files,
         similarly to what convert_watchman_changes does *)
      SSet.of_list file_changes
  in
  ServerRevisionTracker.files_changed local_config (SSet.cardinal changed_files);
  changed_files

(** This is only used by the Edenfs_watcher-backed implementation while it uses
    a dedicated Watchman instance for receiving state transitions. *)
let process_watchman_state_changes ~sync instance_ref root local_config : unit =
  let (watchman', changes) =
    if sync then
      let watchman_res =
        Watchman.get_changes_synchronously
          ~timeout:
            local_config.ServerLocalConfig.watchman
              .ServerLocalConfig.Watchman.synchronous_timeout
          !instance_ref
      in
      Tuple2.map_snd watchman_res ~f:(fun changes ->
          Watchman.Watchman_synchronous changes)
    else
      Watchman.get_changes !instance_ref
  in
  instance_ref := watchman';
  let changes =
    match changes with
    | Watchman.Watchman_unavailable -> []
    | Watchman.Watchman_pushed changes -> [changes]
    | Watchman.Watchman_synchronous changes -> changes
  in
  let process_changes = function
    | Watchman.State_enter (name, _metadata) ->
      if local_config.ServerLocalConfig.hg_aware then
        ServerRevisionTracker.on_state_enter name;
      MeerkatTracker.on_state_enter name
    | Watchman.State_leave (name, metadata) ->
      if local_config.ServerLocalConfig.hg_aware then
        ServerRevisionTracker.on_state_leave root name metadata;
      MeerkatTracker.on_state_leave name
    | Watchman.Files_changed files when SSet.is_empty files -> ()
    | _ ->
      Hh_logger.log
        "Got file changes or merge base change from a Watchman subscription that should never yield any";
      raise (Exit_status.Exit_with Exit_status.Watchman_invalid_result)
  in
  List.iter changes ~f:process_changes

let get_changes_sync (t : t) telemetry : SSet.t * clock option * Telemetry.t =
  let (changes, new_clock, telemetry) =
    match t with
    | IndexOnly _ -> (SSet.empty, None, telemetry)
    | MockChanges { get_changes_sync; _ } ->
      (get_changes_sync (), None, telemetry)
    | Dfind { dfind; _ } ->
      let set =
        try
          Timeout.with_timeout
            ~timeout:120
            ~on_timeout:(fun (_ : Timeout.timings) ->
              Exit.exit Exit_status.Dfind_unresponsive)
            ~do_:(fun _timeout -> DfindLib.get_changes dfind)
        with
        | _ -> Exit.exit Exit_status.Dfind_died
      in
      (set, None, telemetry)
    | Watchman { local_config; watchman; root; _ } ->
      let start_time = Unix.gettimeofday () in
      let (watchman', changes) =
        Watchman.get_changes_synchronously
          ~timeout:
            local_config.ServerLocalConfig.watchman
              .ServerLocalConfig.Watchman.synchronous_timeout
          !watchman
      in
      let telemetry =
        Telemetry.add_duration ~key:"sync_watcher" ~start_time telemetry
      in
      watchman := watchman';
      let changes =
        List.fold_left changes ~init:SSet.empty ~f:(fun acc c ->
            SSet.union acc (convert_watchman_changes ~root ~local_config c))
      in
      let clock = Watchman.get_clock !watchman in
      (changes, Some (ServerNotifierTypes.Watchman clock), telemetry)
    | EdenfsFileWatcher
        { instance; root; local_config; tmp_watchman_instance; last_clock; _ }
      ->
      process_watchman_state_changes
        ~sync:true
        tmp_watchman_instance
        root
        local_config;

      let sync_queries_obey_deferral =
        local_config.ServerLocalConfig.edenfs_file_watcher
          .sync_queries_obey_deferral
      in

      (* TODO(T226505256) Workaround for deferring changes while Meerkat is running *)
      if sync_queries_obey_deferral && MeerkatTracker.is_meerkat_running () then (
        (* This is here to slow down the interrupt mechanism while we are deferring changes:
           Edenfs_watcher itself is unaware of the deferral and will write to the notification
           file descriptor if it sees changes.
           The server sees that and calls watchman_interrupt_handler, which in turn calls
           query_notifier to check if there are actual changes, which might take us here.
           Without this delay, this will cause us to clog the CPU. *)
        Unix.sleepf 0.25;
        (SSet.empty, Some (ServerNotifierTypes.Eden !last_clock), telemetry)
      ) else
        let start_time = Unix.gettimeofday () in
        (* Note that this will handle all errors by raising Exit_status *)
        let (changes, new_clock, sync_telemetry_opt) =
          handle_edenfs_watcher_result
            (Edenfs_watcher.get_changes_sync instance)
        in
        let telemetry =
          Telemetry.add_duration ~key:"sync_watcher" ~start_time telemetry
        in
        let telemetry =
          Option.value_map
            ~default:telemetry
            sync_telemetry_opt
            ~f:(fun sync_telemetry ->
              Telemetry.object_
                ~key:"get_changes_sync"
                ~value:sync_telemetry
                telemetry)
        in
        last_clock := new_clock;
        let changes_set =
          List.fold_left changes ~init:SSet.empty ~f:(fun acc c ->
              SSet.union acc (convert_edenfs_watcher_changes local_config c))
        in
        (changes_set, Some (ServerNotifierTypes.Eden new_clock), telemetry)
  in

  if
    Hh_logger.Level.passes_min_level Hh_logger.Level.Debug
    && not (SSet.is_empty changes)
  then begin
    Hh_logger.log
      ~lvl:Hh_logger.Level.Debug
      "ServerNotifier.get_changes_sync got %d changes"
      (SSet.cardinal changes);
    SSet.iter
      (fun file ->
        Hh_logger.log
          ~lvl:Hh_logger.Level.Debug
          "ServerNotifier.get_changes_sync: changed file %s"
          file)
      changes
  end;
  (changes, new_clock, telemetry)

let get_changes_async (t : t) telemetry : changes * clock option * Telemetry.t =
  let (changes, new_clock, telemetry) =
    match t with
    | IndexOnly _ -> (SyncChanges SSet.empty, None, telemetry)
    | MockChanges { get_changes_async; _ } ->
      (get_changes_async (), None, telemetry)
    | Dfind _ ->
      let (changes, _, telemetry) = get_changes_sync t telemetry in
      (SyncChanges changes, None, telemetry)
    | Watchman { watchman; root; local_config; _ } ->
      let start_time = Unix.gettimeofday () in
      let (watchman', changes) = Watchman.get_changes !watchman in
      let telemetry =
        Telemetry.add_duration ~key:"async_watcher" ~start_time telemetry
      in
      watchman := watchman';
      let changes =
        match changes with
        | Watchman.Watchman_unavailable -> Unavailable
        | Watchman.Watchman_pushed changes ->
          AsyncChanges (convert_watchman_changes ~root ~local_config changes)
        | Watchman.Watchman_synchronous changes ->
          let accumulated_changes =
            List.fold_left changes ~init:SSet.empty ~f:(fun acc c ->
                SSet.union acc (convert_watchman_changes ~root ~local_config c))
          in
          SyncChanges accumulated_changes
      in
      let clock = Watchman.get_clock !watchman in

      (changes, Some (ServerNotifierTypes.Watchman clock), telemetry)
    | EdenfsFileWatcher
        { instance; tmp_watchman_instance; root; local_config; last_clock; _ }
      ->
      process_watchman_state_changes
        ~sync:false
        tmp_watchman_instance
        root
        local_config;

      (* TODO(T226505256) Workaround for deferring changes while Meerkat is running *)
      if MeerkatTracker.is_meerkat_running () then (
        (* This is here to slow down the interrupt mechanism while we are deferring changes:
           Edenfs_watcher itself is unaware of the deferral and will write to the notification
           file descriptor if it sees changes.
           The server sees that and calls watchman_interrupt_handler, which in turn calls
           query_notifier to check if there are actual changes, which might take us here.
           Without this delay, this will cause us to clog the CPU. *)
        Unix.sleepf 0.25;
        (AsyncChanges SSet.empty, Some (Eden !last_clock), telemetry)
      ) else
        let start_time = Unix.gettimeofday () in
        (* Note that this will handle all errors by raising Exit_status *)
        let (changes, new_clock, async_telemetry_opt) =
          handle_edenfs_watcher_result
            (Edenfs_watcher.get_changes_async instance)
        in
        let telemetry =
          Telemetry.add_duration ~key:"async_watcher" ~start_time telemetry
        in
        let telemetry =
          Option.value_map
            ~default:telemetry
            async_telemetry_opt
            ~f:(fun async_telemetry ->
              Telemetry.object_
                ~key:"get_changes_async"
                ~value:async_telemetry
                telemetry)
        in
        last_clock := new_clock;
        let changes_set =
          List.fold_left changes ~init:SSet.empty ~f:(fun acc c ->
              SSet.union acc (convert_edenfs_watcher_changes local_config c))
        in
        (AsyncChanges changes_set, Some (Eden new_clock), telemetry)
  in

  if Hh_logger.Level.passes_min_level Hh_logger.Level.Debug then begin
    let change_set =
      match changes with
      | Unavailable -> SSet.empty
      | AsyncChanges set
      | SyncChanges set ->
        set
    in
    if not (SSet.is_empty change_set) then begin
      Hh_logger.log
        ~lvl:Hh_logger.Level.Debug
        "ServerNotifier.get_changes_async got %d changes"
        (SSet.cardinal change_set);
      SSet.iter
        (fun file ->
          Hh_logger.log
            ~lvl:Hh_logger.Level.Debug
            "ServerNotifier.get_changes_async: changed file %s"
            file)
        change_set
    end
  end;
  (changes, new_clock, telemetry)

let notification_fd (t : t) : Caml_unix.file_descr option =
  match t with
  | Dfind _
  | IndexOnly _ ->
    None
  | MockChanges _ -> None
  | Watchman { watchman; _ } ->
    Option.map (Watchman.get_reader !watchman) ~f:Buffered_line_reader.get_fd
  | EdenfsFileWatcher { instance; _ } ->
    let fd_res = Edenfs_watcher.get_notification_fd instance in
    (* Note that this will handle all errors by raising Exit_status *)
    let fd = handle_edenfs_watcher_result fd_res in
    Some fd

let maybe_changes_available (t : t) : bool option =
  let fd_opt = notification_fd t in
  Option.map fd_opt ~f:(fun fd ->
      let (readable, _, _) = Caml_unix.select [fd] [] [] 0.0 in
      not (List.is_empty readable))
