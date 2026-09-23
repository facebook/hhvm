(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Hh_prelude

exception Informant_edenfs_watcher_failed of string

type repo_transition =
  | State_enter of Hg.Rev.t
  | State_leave of Hg.Rev.t
  | Changed_merge_base of Hg.Rev.t
  | Changed_commit of Hg.Rev.t
[@@deriving show]

type t =
  | Watchman of Watchman.watchman_instance ref
  | Eden of {
      instance: Edenfs_watcher.instance;
      pending: Edenfs_watcher_types.changes Queue.t;
    }

let get_change_watchman
    watchman_ref ~is_in_hg_update_state ~is_in_hg_transaction_state =
  let (watchman, change) = Watchman.get_changes !watchman_ref in
  watchman_ref := watchman;
  match change with
  | Watchman.Watchman_unavailable
  | Watchman.Watchman_synchronous _ ->
    None
  | Watchman.Watchman_pushed (Watchman.Changed_merge_base (rev, _files, _clock))
    ->
    let () = Hh_logger.log "Changed_merge_base: %s" (Hg.Rev.to_string rev) in
    Some (Changed_merge_base rev)
  | Watchman.Watchman_pushed (Watchman.State_enter (state, json))
    when String.equal state Hg_states.update ->
    is_in_hg_update_state := true;
    Option.(
      json >>= Watchman_utils.rev_in_state_change >>= fun hg_rev ->
      Hh_logger.log "State_enter: %s" (Hg.Rev.to_string hg_rev);
      Some (State_enter hg_rev))
  | Watchman.Watchman_pushed (Watchman.State_leave (state, json))
    when String.equal state Hg_states.update ->
    is_in_hg_update_state := false;
    Option.(
      json >>= Watchman_utils.rev_in_state_change >>= fun hg_rev ->
      Hh_logger.log "State_leave: %s" (Hg.Rev.to_string hg_rev);
      Some (State_leave hg_rev))
  | Watchman.Watchman_pushed (Watchman.State_enter (state, _))
    when String.equal state Hg_states.transaction ->
    is_in_hg_transaction_state := true;
    None
  | Watchman.Watchman_pushed (Watchman.State_leave (state, _))
    when String.equal state Hg_states.transaction ->
    is_in_hg_transaction_state := false;
    None
  | Watchman.Watchman_pushed (Watchman.Files_changed _)
  | Watchman.Watchman_pushed (Watchman.State_enter _)
  | Watchman.Watchman_pushed (Watchman.State_leave _) ->
    None

let get_change_eden
    instance pending ~is_in_hg_update_state ~is_in_hg_transaction_state =
  let process_change = function
    | Edenfs_watcher_types.CommitTransition { to_commit; _ } ->
      let rev = Hg.Rev.of_string to_commit in
      Hh_logger.log "Changed_commit: %s" to_commit;
      Some (Changed_commit rev)
    | Edenfs_watcher_types.StateEnter state
      when String.equal state Hg_states.update ->
      is_in_hg_update_state := true;
      None
    | Edenfs_watcher_types.StateLeave state
      when String.equal state Hg_states.update ->
      is_in_hg_update_state := false;
      None
    | Edenfs_watcher_types.StateEnter state
      when String.equal state Hg_states.transaction ->
      is_in_hg_transaction_state := true;
      None
    | Edenfs_watcher_types.StateLeave state
      when String.equal state Hg_states.transaction ->
      is_in_hg_transaction_state := false;
      None
    | Edenfs_watcher_types.FileChanges _
    | Edenfs_watcher_types.StateEnter _
    | Edenfs_watcher_types.StateLeave _ ->
      None
  in

  (match Edenfs_watcher.get_changes_async instance with
  | Ok (changes, _clock, _telemetry) -> Queue.enqueue_all pending changes
  | Error e ->
    raise
      (Informant_edenfs_watcher_failed
         (Edenfs_watcher.show_edenfs_watcher_error e)));

  match Queue.dequeue pending with
  | None -> None
  | Some c -> process_change c

let get_change t ~is_in_hg_update_state ~is_in_hg_transaction_state =
  match t with
  | Watchman watchman_ref ->
    get_change_watchman
      watchman_ref
      ~is_in_hg_update_state
      ~is_in_hg_transaction_state
  | Eden { instance; pending } ->
    get_change_eden
      instance
      pending
      ~is_in_hg_update_state
      ~is_in_hg_transaction_state

let has_more_messages t =
  match t with
  | Watchman watchman_ref ->
    (match Watchman.get_reader !watchman_ref with
    | None -> false
    | Some reader -> Buffered_line_reader.is_readable reader)
  | Eden { instance; pending } ->
    if not (Queue.is_empty pending) then
      true
    else (
      match Edenfs_watcher.get_notification_fd instance with
      | Error e ->
        raise
          (Informant_edenfs_watcher_failed
             (Edenfs_watcher.show_edenfs_watcher_error e))
      | Ok fd ->
        Buffered_line_reader.is_readable (Buffered_line_reader.create fd)
    )

let init_watchman ~watchman_debug_logging root =
  let watchman =
    (* The informant is only interested in hg state changes and Changed_merge_base notifications,
       but not actual files.
       Therefore, we use an expression term for our subscription that matches no files. *)
    let expression_terms =
      [Hh_json_helpers.AdhocJsonHelpers.strlist ["false"]]
    in
    Watchman.init
      {
        Watchman.subscribe_mode = Some Watchman.Scm_aware;
        init_timeout = Watchman.Explicit_timeout 30.;
        expression_terms;
        debug_logging = watchman_debug_logging;
        (* Should also take an arg *)
        sockname = None;
        subscription_prefix = "hh_informant_watcher";
        roots = [root];
      }
      ()
  in
  match watchman with
  | Some watchman_env ->
    Some (Watchman (ref (Watchman.Watchman_alive watchman_env)))
  | None -> None

let init_eden root =
  let settings =
    {
      Edenfs_watcher_types.root;
      watch_spec =
        { Files_to_ignore.include_extensions = []; include_file_names = [] };
      debug_logging = false;
      timeout_secs = 30;
      throttle_time_ms = 0;
      report_telemetry = false;
      state_tracking = true;
      sync_queries_obey_deferral = false;
      tracked_states = [Hg_states.update; Hg_states.transaction];
    }
  in
  match Edenfs_watcher.init settings with
  | Ok (instance, _clock) -> Some (Eden { instance; pending = Queue.create () })
  | Error e ->
    Hh_logger.log
      "Edenfs_watcher failed to init: %s"
      (Edenfs_watcher.show_edenfs_watcher_error e);
    None

let init ~use_eden ~watchman_debug_logging root =
  if use_eden then (
    match init_eden root with
    | Some t -> Some t
    | None ->
      let msg =
        "InformantNotifier failed to initialize Edenfs_watcher, falling back to Watchman"
      in
      Hh_logger.log "%s" msg;
      Hack_event_logger.edenfs_watcher_fallback ~msg;
      init_watchman ~watchman_debug_logging root
  ) else
    init_watchman ~watchman_debug_logging root
