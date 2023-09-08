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
open Hh_prelude
open ServerEnv

let make_genv options config local_config workers =
  Typing_deps.trace :=
    (not (ServerArgs.check_mode options))
    || Option.is_some (ServerArgs.save_filename options);
  let (notifier, indexer) =
    ServerNotifier.init options local_config ~num_workers:(List.length workers)
  in
  {
    options;
    config;
    local_config;
    workers = Some workers;
    notifier;
    indexer;
    debug_channels = None;
  }

(* useful in testing code *)
let default_genv =
  {
    options = ServerArgs.default_options ~root:"";
    config = ServerConfig.default_config;
    local_config = ServerLocalConfig.default;
    workers = None;
    notifier = ServerNotifier.init_null ();
    indexer = (fun _ () -> []);
    debug_channels = None;
  }

let make_env ~init_id ~deps_mode config : ServerEnv.env =
  {
    tcopt = ServerConfig.typechecker_options config;
    popt = ServerConfig.parser_options config;
    gleanopt = ServerConfig.glean_options config;
    swriteopt = ServerConfig.symbol_write_options config;
    naming_table = Naming_table.empty;
    deps_mode;
    errorl = Errors.empty;
    failed_naming = Relative_path.Set.empty;
    last_command_time = 0.0;
    last_notifier_check_time = 0.0;
    last_idle_job_time = 0.0;
    disk_needs_parsing = Relative_path.Set.empty;
    clock = None;
    needs_phase2_redecl = Relative_path.Set.empty;
    needs_recheck = Relative_path.Set.empty;
    full_recheck_on_file_changes = Not_paused;
    full_check_status = Full_check_done;
    changed_files = Relative_path.Set.empty;
    prechecked_files = Prechecked_files_disabled;
    can_interrupt = true;
    interrupt_handlers = (fun _ _ -> []);
    nonpersistent_client_pending_command_needs_full_check = None;
    why_needs_server_type_check = ("init", "");
    init_env =
      {
        approach_name = "";
        ci_info = None;
        init_error = None;
        init_id;
        init_start_t = Unix.gettimeofday ();
        init_type = "";
        mergebase = None;
        why_needed_full_check = None;
        recheck_id = None;
        saved_state_delta = None;
        naming_table_manifold_path = None;
      };
    last_recheck_loop_stats = RecheckLoopStats.empty ~recheck_id:"<none>";
    last_recheck_loop_stats_for_actual_work = None;
  }
