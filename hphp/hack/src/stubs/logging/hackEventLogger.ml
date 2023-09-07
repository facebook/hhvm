(*
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

module PerFileProfilingConfig = struct
  type profile_decling =
    | DeclingOff
    | DeclingTopCounts
    | DeclingAllTelemetry of { callstacks: bool }

  type t = {
    profile_log: bool;
    profile_type_check_duration_threshold: float;
    profile_type_check_memory_threshold_mb: int;
    profile_type_check_twice: bool;
    profile_decling: profile_decling;
    profile_owner: string option;
    profile_desc: string option;
    profile_slow_threshold: float;
  }

  let default =
    {
      profile_log = false;
      profile_type_check_duration_threshold = 0.05 (* seconds *);
      profile_type_check_memory_threshold_mb = 100;
      profile_type_check_twice = false;
      profile_decling = DeclingOff;
      profile_owner = None;
      profile_desc = None;
      profile_slow_threshold = 0.;
    }
end

type serialized_globals = Serialized_globals

let serialize_globals () = Serialized_globals

type rollout_flags = {
  log_saved_state_age_and_distance: bool;
  fetch_remote_old_decls: bool;
  ide_max_num_decls: int;
  ide_max_num_shallow_decls: int;
  max_typechecker_worker_memory_mb: int;
  max_workers: int;
  use_max_typechecker_worker_memory_for_decl_deferral: bool;
  specify_manifold_api_key: bool;
  populate_member_heaps: bool;
  shm_use_sharded_hashtbl: bool;
  shm_cache_size: int;
  remote_old_decls_no_limit: bool;
  use_manifold_cython_client: bool;
  disable_naming_table_fallback_loading: bool;
  use_type_alias_heap: bool;
  override_load_state_natively: bool;
  use_server_revision_tracker_v2: bool;
  rust_provider_backend: bool;
  use_hh_distc_instead_of_hulk: bool;
  consume_streaming_errors: bool;
  hh_distc_fanout_threshold: int;
  rust_elab: bool;
  ide_load_naming_table_on_disk: bool;
  ide_naming_table_update_threshold: int;
  use_compressed_dep_graph: bool;
  glean_v2: bool;
}

let flush () = ()

let deserialize_globals _ = ()

let set_use_watchman _ = ()

let set_use_full_fidelity_parser _ = ()

let set_search_chunk_size _ = ()

let set_changed_mergebase _ = ()

let set_from _ = ()

let set_hhconfig_version _ = ()

let set_rollout_group _ = ()

let set_rollout_flags _ = ()

let typechecker_exit _ _ _ ~exit_type:_ ~exit_code:_ ~exit_status:_ ~is_oom:_ =
  ()

let init
    ~root:_
    ~hhconfig_version:_
    ~init_id:_
    ~custom_columns:_
    ~informant_managed:_
    ~rollout_flags:_
    ~rollout_group:_
    ~time:_
    ~max_workers:_
    ~per_file_profiling:_ =
  ()

let init_worker
    ~root:_
    ~hhconfig_version:_
    ~init_id:_
    ~custom_columns:_
    ~rollout_flags:_
    ~rollout_group:_
    ~time:_
    ~per_file_profiling:_ =
  ()

let init_monitor
    ~from:_
    ~custom_columns:_
    ~proc_stack:_
    ~hhconfig_version:_
    ~rollout_flags:_
    ~rollout_group:_
    _
    _
    _ =
  ()

let init_batch_tool ~init_id:_ ~root:_ ~time:_ = ()

let starting_first_server _ = ()

let refuse_to_restart_server ~reason:_ ~server_state:_ ~version_matches:_ = ()

let server_receipt_to_monitor_write_exn ~server_receipt_to_monitor_file:_ _ = ()

let server_receipt_to_monitor_read_exn ~server_receipt_to_monitor_file:_ _ _ =
  ()

let init_lazy_end _ ~approach_name:_ ~init_error:_ ~init_type:_ = ()

let server_is_partially_ready () = ()

let server_is_ready _ = ()

let load_deptable_end _ = ()

let saved_state_download_and_load_done
    ~load_state_approach:_ ~success:_ ~state_result:_ _ =
  ()

let tried_to_be_hg_aware_with_precomputed_saved_state_warning _ = ()

let tried_to_load_non_existant_compressed_dep_graph _ = ()

let init_start ~experiments_config_meta:_ _ = ()

let nfs_root _ = ()

let load_state_worker_end ~is_cached:_ _ _ = ()

let vcs_changed_files_end _ _ = ()

let type_check_dirty ~start_t:_ ~dirty_count:_ ~recheck_count:_ = ()

let out_of_date _ = ()

let lock_stolen _ = ()

let client_init ~init_id:_ ~custom_columns:_ _ = ()

let serverless_ide_init ~init_id:_ = ()

let client_set_mode _ = ()

let serverless_ide_set_root _ = ()

let client_start _ = ()

let client_stop _ = ()

let client_restart ~data:_ = ()

let client_check_start () = ()

let client_check _ _ ~init_proc_stack:_ ~spinner:_ = ()

let client_check_partial _ _ ~init_proc_stack:_ ~spinner:_ = ()

let client_check_bad_exit _ _ ~init_proc_stack:_ ~spinner:_ = ()

let client_check_errors_file_restarted _ = ()

let client_lsp_method_handled
    ~root:_
    ~method_:_
    ~kind:_
    ~path_opt:_
    ~result_count:_
    ~result_extra_telemetry:_
    ~tracking_id:_
    ~start_queue_time:_
    ~start_hh_server_state:_
    ~start_handle_time:_
    ~serverless_ide_flag:_ =
  ()

let client_lsp_method_exception
    ~root:_
    ~method_:_
    ~kind:_
    ~path_opt:_
    ~tracking_id:_
    ~start_queue_time:_
    ~start_hh_server_state:_
    ~start_handle_time:_
    ~message:_
    ~data_opt:_
    ~source:_ =
  ()

let serverless_ide_bug ~message:_ ~data:_ = ()

let client_lsp_exception ~root:_ ~message:_ ~data_opt:_ ~source:_ = ()

let serverless_ide_startup ?count:_ ~start_time:_ _ = ()

let serverless_ide_load_naming_table ~start_time:_ ~local_file_count:_ _ = ()

let serverless_ide_destroy_ok _ = ()

let serverless_ide_destroy_error _ _ _ = ()

let server_hung_up
    ~external_exit_status:_
    ~client_exn:_
    ~client_stack:_
    ~server_exit_status:_
    ~server_stack:_
    ~server_msg:_ =
  ()

let client_bad_exit ~command_name:_ _ _ = ()

let glean_init ~reponame:_ ~init_time:_ ~prev_init_time:_ = ()

let glean_init_failure ~reponame:_ ~init_time:_ ~prev_init_time:_ ~e:_ = ()

let glean_query
    ~reponame:_
    ~mode:_
    ~start_time:_
    ~glean_init_time:_
    ~count:_
    ~query:_
    ~query_text:_ =
  ()

let glean_query_error
    ~reponame:_
    ~mode:_
    ~start_time:_
    ~glean_init_time:_
    ~query:_
    ~query_text:_
    ~e:_ =
  ()

let glean_fetch_namespaces ~count:_ ~start_time:_ = ()

let completion_call ~method_name:_ = ()

let glean_fetch_namespaces_error _ = ()

let ranked_autocomplete_duration ~start_time:_ = ()

let ranked_autocomplete_request_duration ~start_time:_ = ()

let monitor_dead_but_typechecker_alive () = ()

let spinner_heartbeat _ ~spinner:_ = ()

let spinner_change ~spinner:_ ~next:_ = ()

let client_established_connection _ = ()

let client_connect_once ~t_start:_ = ()

let client_connect_once_failure ~t_start:_ _ _ = ()

let client_connect_to_monitor_slow_log () = ()

let client_connect_autostart () = ()

let check_response _ = ()

let got_client_channels _ = ()

let get_client_channels_exception _ = ()

let got_persistent_client_channels _ = ()

let get_persistent_client_channels_exception _ = ()

let handled_connection _ = ()

let handle_connection_exception _ _ = ()

let handled_persistent_connection _ = ()

let handle_persistent_connection_exception _ _ ~is_fatal:_ = ()

let server_file_edited_error ~reason:_ _ = ()

let handled_command
    _ ~start_t:_ ~major_gc_time:_ ~minor_gc_time:_ ~parsed_files:_ =
  ()

let remote_scheduler_get_dirty_files_end _ _ = ()

let remote_scheduler_update_dependency_graph_end ~edges:_ _ = ()

let remote_scheduler_save_naming_end _ = ()

let credentials_check_failure _ _ = ()

let credentials_check_end _ _ = ()

let remote_worker_type_check_end _ ~start_t:_ = ()

let remote_worker_load_naming_end _ = ()

let recheck_end
    ~last_recheck_duration:_
    ~update_batch_count:_
    ~total_changed_files:_
    ~total_rechecked:_
    _ =
  ()

let indexing_end ~desc:_ _ = ()

let parsing_end _ _ ~parsed_count:_ = ()

let parsing_end_for_init _ _ ~parsed_count:_ ~desc:_ = ()

let parsing_end_for_typecheck _ _ ~parsed_count:_ = ()

let naming_costly_iter ~start_t:_ = ()

let naming_end ~count:_ _ _ = ()

let global_naming_end ~count:_ ~desc:_ ~heap_size:_ ~start_t:_ = ()

let run_search_end _ = ()

let update_search_end _ _ = ()

let naming_from_saved_state_end _ = ()

let naming_sqlite_local_changes_nonempty _ = ()

let naming_sqlite_has_changes_since_baseline _ = ()

let type_decl_end _ = ()

let remote_old_decl_end _ _ = ()

let first_redecl_end _ _ = ()

let second_redecl_end _ _ = ()

let type_check_primary_position_bug ~current_file:_ ~message:_ ~stack:_ = ()

let type_check_exn_bug ~path:_ ~pos:_ ~e:_ = ()

let invariant_violation_bug ?path:_ ?pos:_ ?data:_ ?data_int:_ ?telemetry:_ _ =
  ()

let decl_consistency_bug ?path:_ ?pos:_ ?data:_ _ = ()

let live_squiggle_diff ~uri:_ ~reason:_ ~expected_error_count:_ _ = ()

let type_check_end
    _
    ~heap_size:_
    ~started_count:_
    ~total_rechecked_count:_
    ~desc:_
    ~experiments:_
    ~start_t:_ =
  ()

let notifier_returned _ _ = ()

let load_state_exn _ = ()

let naming_table_sqlite_missing _ = ()

let prechecked_update_rechecked _ = ()

let prechecked_evaluate_init _ _ = ()

let prechecked_evaluate_incremental _ _ = ()

let check_mergebase_failed _ _ = ()

let check_mergebase_success _ = ()

let type_at_pos_batch ~start_time:_ ~num_files:_ ~num_positions:_ ~results:_ =
  ()

let worker_large_data_send ~path:_ _ = ()

let with_id ~stage:_ _ f = f ()

let with_rechecked_stats
    ~update_batch_count:_ ~total_changed_files:_ ~total_rechecked:_ f =
  f ()

let with_init_type _ f = f ()

let with_check_kind ~check_kind:_ ~check_reason:_ f = f ()

let state_loader_dirty_files _ = ()

let changed_while_parsing_end _ = ()

let saved_state_load_ok _ ~start_time:_ = ()

let saved_state_load_failure _ ~start_time:_ = ()

let saved_state_dirty_files_ok ~start_time:_ = ()

let saved_state_dirty_files_failure _ ~start_time:_ = ()

let saved_state_load_naming_table_on_disk _ ~start_time:_ = ()

let monitor_update_status _ _ = ()

let find_svn_rev_failed _ _ = ()

let revision_tracker_init_svn_rev_failed _ = ()

(** Watchman Event Watcher client running in the informant *)
let informant_watcher_not_available _ = ()

let informant_watcher_unknown_state _ = ()

let informant_watcher_mid_update_state _ = ()

let informant_watcher_settled_state _ = ()

let informant_watcher_starting_server_from_settling _ = ()

(** Server Monitor events *)
let accepting_on_socket_exception _ = ()

let malformed_build_id _ = ()

let send_fd_failure _ = ()

let typechecker_already_running _ = ()

(** Watchman Event Watcher events *)
let init_watchman_event_watcher _ _ = ()

let init_watchman_failed _ = ()

let restarting_watchman_subscription _ = ()

let watchman_uncaught_exception _ = ()

let monitor_giving_up_exception _ = ()

let processed_clients _ = ()

let invalid_mercurial_state_transition ~state:_ = ()

let server_revision_tracker_forced_reset ~telemetry:_ = ()

let search_symbol_index
    ~query_text:_
    ~max_results:_
    ~results:_
    ~kind_filter:_
    ~duration:_
    ~caller:_
    ~search_provider:_ =
  ()

let shallow_decl_errors_emitted _ = ()

let server_progress_write_exn ~server_progress_file:_ _ = ()

let server_progress_read_exn ~server_progress_file:_ _ = ()

let worker_exception _ = ()

(* Typing service events. *)

let hulk_type_check_end _ _ ~start_t:_ = ()

module ProfileTypeCheck = struct
  type stats = unit

  type batch_info = unit

  type typecheck_info = unit

  let get_stats
      ~include_current_process:_
      ~include_slightly_costly_stats:_
      ~shmem_heap_size:_
      _ =
    ()

  let get_typecheck_info
      ~init_id:_
      ~check_reason:_
      ~recheck_id:_
      ~start_hh_stats:_
      ~start_typecheck_stats:_
      ~config:_ =
    ()

  let get_batch_info
      ~typecheck_info:_
      ~worker_id:_
      ~batch_number:_
      ~batch_size:_
      ~start_batch_stats:_ =
    ()

  let process_workitem
      ~batch_info:_
      ~workitem_index:_
      ~file:_
      ~file_was_already_deferred:_
      ~decl:_
      ~error_code:_
      ~workitem_ends_under_cap:_
      ~workitem_start_stats:_
      ~workitem_end_stats:_
      ~workitem_end_second_stats:_ =
    ()

  let compute_tast ~path:_ ~telemetry:_ ~start_time:_ = ()
end

module CGroup = struct
  let error _ = ()

  let step
      ~cgroup:_
      ~step_group:_
      ~step:_
      ~start_time:_
      ~total_relative_to:_
      ~anon_relative_to:_
      ~shmem_relative_to:_
      ~file_relative_to:_
      ~total_start:_
      ~anon_start:_
      ~shmem_start:_
      ~file_start:_
      ~total_hwm:_
      ~total:_
      ~anon:_
      ~shmem:_
      ~file:_
      ~secs_at_total_gb:_
      ~secs_above_total_gb_summary:_ =
    ()
end

module Memory = struct
  let profile_if_needed () = ()
end

module ProfileDecl = struct
  let count_decl
      ~kind:_
      ~cpu_duration:_
      ~decl_id:_
      ~decl_name:_
      ~decl_origin:_
      ~decl_file:_
      ~decl_callstack:_
      ~decl_start_time:_
      ~subdecl_member_name:_
      ~subdecl_eagerness:_
      ~subdecl_callstack:_
      ~subdecl_start_time:_ =
    ()
end

module Rage = struct
  let rage_start ~rageid:_ ~desc:_ ~root:_ ~from:_ ~disk_config:_ = ()

  let rage_watchman ~rageid:_ ~items:_ = ()

  let rage
      ~rageid:_
      ~desc:_
      ~vscode_bugid:_
      ~merged_logs:_
      ~root:_
      ~from:_
      ~hhconfig_version:_
      ~disk_config:_
      ~experiments:_
      ~experiments_config_meta:_
      ~items:_
      ~result:_
      ~start_time:_ =
    ()
end

module Fanouts = struct
  let log_class
      ~class_name:_
      ~class_diff:_
      ~class_diff_category:_
      ~fanout_cardinal:_
      ~direct_references_cardinal:_
      ~descendants_cardinal:_
      ~children_cardinal:_ =
    ()

  let log ~changes_cardinal:_ ~fanout_cardinal:_ ~max_class_fanout_cardinal:_ =
    ()
end
