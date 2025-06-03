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

type clock = ServerNotifierTypes.clock = Watchman of Watchman.clock
[@@deriving show, eq]

let watchman_clock_of_clock = function
  | ServerNotifierTypes.Watchman clock -> clock

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
      tmp_watchman_instance: t;
          (** Currently, the Edenfs_watcher-backed implementation is work in
          progress and can't actually provide any of the ServerNotifier
          functionality. In order to be able to do some basic testing, we
          actually forward all requests to this Watchman instance. *)
    }
  | MockChanges of {
      get_changes_async: unit -> changes;
      get_changes_sync: unit -> SSet.t;
    }

type indexer = (string -> bool) -> unit -> string list

(** This returns an "indexer", i.e. unit -> string list, which when invoked
will return all files under root. *)
let rec indexer (t : t) (filter : string -> bool) : unit -> string list =
  match t with
  | Dfind { root; _ }
  | IndexOnly { root; _ } ->
    Find.make_next_files ~name:"root" ~filter root
  | MockChanges _ -> failwith "indexer not mocked"
  | Watchman { wenv; num_workers; _ } ->
    let files = Watchman.get_all_files wenv in
    Bucket.make_list ~num_workers (List.filter ~f:filter files)
  | EdenfsFileWatcher { tmp_watchman_instance; _ } ->
    (* TODO(T225695144) Implement this in Edenfs_watcher *)
    indexer tmp_watchman_instance filter

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
        HackEventLogger.set_use_watchman ();
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
    let init_settings = { Edenfs_watcher.root } in
    match Edenfs_watcher.init init_settings with
    | Result.Error (Edenfs_watcher_types.EdenfsWatcherError msg) ->
      Hh_logger.log
        "Failed to initialize EdenFS watcher, failed with message:\n%s"
        msg;
      None
    | Result.Error Edenfs_watcher_types.NonEdenWWW ->
      Hh_logger.log
        "Failed to initialize EdenFS watcher, www repo %s is not on Eden"
        (Path.to_string root);
      None
    | Result.Error (Edenfs_watcher_types.LostChanges reason) ->
      Hh_logger.log
        "Failed to initialize EdenFS watcher with lost changes message, reason %s"
        reason;
      None
    | Result.Ok instance -> begin
      (* TODO(frankemrich): Add use_edenfs_file_watcher to hh_server_events
         table and HackEventLogger *)
      (* HackEventLogger.set_use_edenfs_file_watcher (); *)

      (* This is just temporary:
         For now, we also carry around a nested instance using Watchman. *)
      let watchman = try_init_watchman () in
      Option.map watchman ~f:(fun tmp_watchman_instance ->
          EdenfsFileWatcher { instance; tmp_watchman_instance })
    end
  in

  let notifier =
    match
      (ServerArgs.check_mode options, edenfs_watcher_enabled, watchman_enabled)
    with
    | (true, _, _) ->
      (* check_mode *)
      Hh_logger.log "Not using any file watching mechanism";
      IndexOnly { root }
    | (false, true, _) ->
      (* Whenever EdenFS file watching is requested it takes precedence
         over Watchman, but there is no fallback *)
      if watchman_enabled then
        Hh_logger.warn
          "Both Watchman and EdenFS file watching enabled in server config";
      (match try_init_edenfs_watcher () with
      | Some t -> t
      | None ->
        failwith "EdenFS file watching enabled, but failed to initialize.")
    | (false, false, true) ->
      (match try_init_watchman () with
      | Some t -> t
      | None -> init_dfind ())
    | (false, false, false) -> init_dfind ()
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

let convert_edenfs_watcher_changes (eden_changes : Edenfs_watcher_types.changes)
    : SSet.t =
  match eden_changes with
  | Edenfs_watcher_types.CommitTransition { file_changes; _ } ->
    (* TODO(T224461521) Need to inform ServerRevisionTracker about commit
       transition, similarly to what convert_watchman_changes does *)
    SSet.of_list file_changes
  | Edenfs_watcher_types.FileChanges file_changes ->
    (* TODO(T215219438) Need to inform ServerRevisionTracker about changed files,
       similarly to what convert_watchman_changes does *)
    SSet.of_list file_changes

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

let get_changes_sync (t : t) : SSet.t * clock option =
  let (changes, new_clock) =
    match t with
    | IndexOnly _ -> (SSet.empty, None)
    | MockChanges { get_changes_sync; _ } -> (get_changes_sync (), None)
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
      (set, None)
    | Watchman { local_config; watchman; root; _ } ->
      let (watchman', changes) =
        Watchman.get_changes_synchronously
          ~timeout:
            local_config.ServerLocalConfig.watchman
              .ServerLocalConfig.Watchman.synchronous_timeout
          !watchman
      in
      watchman := watchman';
      let changes =
        List.fold_left changes ~init:SSet.empty ~f:(fun acc c ->
            SSet.union acc (convert_watchman_changes ~root ~local_config c))
      in
      let clock = Watchman.get_clock !watchman in
      (changes, Some (ServerNotifierTypes.Watchman clock))
    | EdenfsFileWatcher { instance; _ } ->
      (* Note that this will handle all errors by raising Exit_status *)
      let (changes, _clock) =
        handle_edenfs_watcher_result (Edenfs_watcher.get_changes_sync instance)
      in
      let changes_set =
        List.fold_left changes ~init:SSet.empty ~f:(fun acc c ->
            SSet.union acc (convert_edenfs_watcher_changes c))
      in
      (* TODO(T215219438) We need to return a proper clock value here *)
      let new_clock = None in
      (changes_set, new_clock)
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
  (changes, new_clock)

let get_changes_async (t : t) : changes * clock option =
  let (changes, new_clock) =
    match t with
    | IndexOnly _ -> (SyncChanges SSet.empty, None)
    | MockChanges { get_changes_async; _ } -> (get_changes_async (), None)
    | Dfind _ ->
      let (changes, _) = get_changes_sync t in
      (SyncChanges changes, None)
    | Watchman { watchman; root; local_config; _ } ->
      let (watchman', changes) = Watchman.get_changes !watchman in
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

      (changes, Some (ServerNotifierTypes.Watchman clock))
    | EdenfsFileWatcher { instance; _ } ->
      (* Note that this will handle all errors by raising Exit_status *)
      let (changes, _clock) =
        handle_edenfs_watcher_result (Edenfs_watcher.get_changes_async instance)
      in
      let changes_set =
        List.fold_left changes ~init:SSet.empty ~f:(fun acc c ->
            SSet.union acc (convert_edenfs_watcher_changes c))
      in
      (* TODO(T215219438) We need to return a proper clock value here *)
      let new_clock = None in
      (AsyncChanges changes_set, new_clock)
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
  (changes, new_clock)

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
