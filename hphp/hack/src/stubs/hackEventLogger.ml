(**
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

let set_use_watchman _ = ()
let set_use_tiny_state _ = ()
let set_use_full_fidelity_parser _ = ()
let set_lazy_incremental _ = ()
let set_search_chunk_size _ = ()
let bad_exit _ _ _ ~is_oom:_ = ()
let init
  ~exit_on_parent_exit:_
  ~root:_
  ~init_id:_
  ~informant_managed:_
  ~devinfra_saved_state_lookup:_
  ~time:_
  ~saved_state_load_type:_
  ~use_sql:_
  ~search_chunk_size:_
  ~max_workers:_
  ~max_bucket_size:_
  ~load_tiny_state:_
  ~use_full_fidelity_parser:_
  ~interrupt_on_watchman:_
  ~interrupt_on_client:_
  = ()
let init_monitor
  ?exit_on_parent_exit:_
  ~search_chunk_size:_
  ~devinfra_saved_state_lookup:_
  _ _ _ _ = ()
let init_end ~state_distance:_ ~approach_name:_ ~init_error:_ _ _ = ()
let starting_first_server _ = ()
let init_lazy_end
  ~informant_use_xdb:_
  ~state_distance:_
  ~approach_name:_
  ~init_error:_ _ = ()
let init_really_end
  ~informant_use_xdb:_
  ~state_distance:_
  ~approach_name:_
  ~init_error:_ _ = ()
let load_deptable_end _ = ()
let nfs_root _ = ()
let load_mini_worker_end ~is_cached:_ _ _ = ()
let vcs_changed_files_end _ _ = ()
let type_check_dirty ~start_t:_ ~dirty_count:_ ~recheck_count:_ = ()
let out_of_date _ = ()
let lock_stolen _ = ()
let client_init ?exit_on_parent_exit:_ _ = ()
let client_set_from _ = ()
let client_set_mode _ = ()
let client_check () = ()
let client_build _ _ = ()
let client_start _ = ()
let client_stop _ = ()
let client_restart _ = ()
let client_build_begin_work _ _ = ()
let client_build_finish
  ~rev_changed:_ ~build_type:_ ~request_id:_ ~exit_status:_ = ()
let client_check_finish _ = ()
let client_lsp_method_handled
  ~root:_ ~method_:_ ~kind:_ ~start_queue_time:_ ~start_hh_server_state:_ ~start_handle_time:_
  ~json:_ ~json_response:_ = ()
let client_lsp_method_exception
  ~root:_ ~method_:_ ~kind:_ ~start_queue_time:_ ~start_hh_server_state:_ ~start_handle_time:_
  ~json:_ ~message:_ ~stack:_ ~source:_ = ()
let client_lsp_exception
  ~root:_ ~message:_ ~stack:_ ~source:_ = ()
let client_bad_exit ~command _ = ()
let monitor_dead_but_typechecker_alive () = ()
let client_connect_to_monitor_timeout () = ()
let client_established_connection _ = ()
let client_establish_connection_exception _ = ()
let client_connect_once _ = ()
let client_connect_once_busy _ = ()
let check_response _ = ()
let got_client_channels _ = ()
let get_client_channels_exception _ = ()
let got_persistent_client_channels _ = ()
let get_persistent_client_channels_exception _ = ()
let handled_connection _ = ()
let handle_connection_exception _ = ()
let handled_persistent_connection _ = ()
let handle_persistent_connection_exception _ = ()
let handled_command _ _ = ()
let build_differs _ _ _ = ()
let build_same _ _ = ()
let recheck_end _ _ _ _ = ()
let indexing_end _ = ()
let parsing_end _ _ ~parsed_count:_ = ()
let updating_deps_end _ = ()
let naming_end _ = ()
let global_naming_end _ = ()
let run_search_end _ = ()
let send_hg_end _ = ()
let wait_hg_end _ = ()
let update_search_end _ = ()
let fast_naming_end _ = ()
let type_decl_end _ = ()
let first_redecl_end _ _ = ()
let second_redecl_end _ _ = ()
let type_check_end _ _ _ = ()
let notifier_returned _ _ = ()
let load_mini_exn _ = ()
let hg_cat_exn _ = ()
let with_id ~stage:_ _ f = f ()
let with_rechecked_stats _ _ _ f = f ()
let with_init_type _ f = f ()
let with_check_kind _ f = f ()
let state_loader_dirty_files _ = ()

(** Informant events *)
let init_informant_prefetcher_runner ?exit_on_parent_exit _ = ()
let informant_decision_on_saved_state
  ~start_t:_ ~state_distance:_ ~incremental_distance:_ = ()
let informant_induced_kill _ = ()
let informant_induced_restart _ = ()
let informant_no_xdb_result _ = ()
let informant_prefetcher_success _ = ()
let informant_prefetcher_failed _ _ = ()
let informant_prefetcher_timed_out _ = ()
let informant_state_leave _ = ()
let find_svn_rev_failed _ _ = ()
let find_svn_rev_success _ = ()
let find_xdb_match_failed _ _ = ()
let find_xdb_match_success _ = ()
let find_xdb_match_timed_out _ = ()
let informant_find_saved_state_failed _ = ()
let informant_find_saved_state_success ~distance:_ _ = ()
let revision_tracker_init_svn_rev_failed _ = ()
let xdb_malformed_result _ = ()

(** Watchman Event Watcher client running in the informant *)
let informant_watcher_not_available _ = ()
let informant_watcher_unknown_state _ = ()
let informant_watcher_mid_update_state _ = ()
let informant_watcher_settled_state _ = ()
let informant_watcher_starting_server_from_settling _ = ()

(** Server Monitor events *)
let accepting_on_socket_exception _ = ()
let ack_and_handoff_exception _ = ()
let accepted_client_fd _ = ()
let client_connection_sent _ = ()
let malformed_build_id _ = ()
let send_fd_failure _ = ()
let typechecker_already_running _ = ()

(** Watchman Event Watcher events *)
let init_watchman_event_watcher _ _ = ()
let init_watchman_failed _ = ()
let restarting_watchman_subscription _ = ()
let uncaught_exception _ = ()
let processed_clients _ = ()
