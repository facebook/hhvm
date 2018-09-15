(**
 * Copyright (c) 2018, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE fn in the "hack" directory of this source tree.
 *
 *)

open ServerEnv

let should_use options local_config =
  Option.value (ServerArgs.prechecked options)
    ~default:local_config.ServerLocalConfig.prechecked_files

let set env prechecked_files = { env with prechecked_files }

let intersect_with_master_deps ~deps ~dirty_master_deps =
  let common_deps = Typing_deps.DepSet.inter deps dirty_master_deps in
  let dirty_master_deps = Typing_deps.DepSet.diff dirty_master_deps common_deps in
  let more_deps = Typing_deps.add_all_deps common_deps in
  more_deps, dirty_master_deps

let update_rechecked_files env rechecked =
  let t = Unix.gettimeofday () in
  let add_rechecked dirty_deps =
    let rechecked_files = Relative_path.Map.fold rechecked
      ~init:dirty_deps.rechecked_files
      ~f:begin fun path _ acc ->
        Relative_path.Set.add acc path
      end
    in
    { dirty_deps with rechecked_files }
  in
  let env = set env @@ match env.prechecked_files with
  | Prechecked_files_disabled -> Prechecked_files_disabled
  | Initial_typechecking dirty_deps ->
    Initial_typechecking (add_rechecked dirty_deps)
  | Prechecked_files_ready dirty_deps ->
    Prechecked_files_ready (add_rechecked dirty_deps)
  in
  HackEventLogger.prechecked_update_rechecked t;
  env

let update_after_recheck env rechecked =
  let env = update_rechecked_files env rechecked in
    match env.full_check, env.prechecked_files with
  | Full_check_done, Initial_typechecking {
      dirty_local_deps;
      dirty_master_deps;
      rechecked_files;
      clean_local_deps;
    } ->
    let t = Unix.gettimeofday () in
    assert (Typing_deps.DepSet.is_empty clean_local_deps);
    Hh_logger.log "Finished rechecking dirty files, evaluating their fanout";
    let deps = Typing_deps.add_all_deps dirty_local_deps in
    (* Take any prechecked files that could have been affected by local changes
     * and expand them too *)
    let more_deps, dirty_master_deps =
      intersect_with_master_deps ~deps ~dirty_master_deps in
    let deps = Typing_deps.DepSet.union deps more_deps in
    let needs_recheck = Typing_deps.get_files deps in
    let needs_recheck = Relative_path.Set.diff needs_recheck rechecked_files in

    let size = Relative_path.Set.cardinal needs_recheck in
    let env = if size = 0 then env else begin
      Hh_logger.log "Adding %d files to recheck" size;
      let full_check = Full_check_started in
      let init_env = { env.init_env with needs_full_init = true } in
      { env with needs_recheck; full_check; init_env }
    end in
    let clean_local_deps = dirty_local_deps in
    let dirty_local_deps = Typing_deps.DepSet.empty in
    HackEventLogger.prechecked_evaluate_init t size;

    set env (Prechecked_files_ready {
      dirty_local_deps;
      dirty_master_deps;
      rechecked_files;
      clean_local_deps;
    })
  | _ -> env

let update_after_local_changes env changes =
  match env.prechecked_files with
  | Prechecked_files_disabled -> env
  | Initial_typechecking dirty_deps ->
    let dirty_local_deps =
      Typing_deps.DepSet.union changes dirty_deps.dirty_local_deps in
    set env (Initial_typechecking { dirty_deps with dirty_local_deps })
  | Prechecked_files_ready dirty_deps ->
    let changes = Typing_deps.DepSet.diff changes dirty_deps.clean_local_deps in
    if Typing_deps.DepSet.is_empty changes then env else
    let t = Unix.gettimeofday () in
    let clean_local_deps =
      Typing_deps.DepSet.union dirty_deps.clean_local_deps changes in
    let changes = Typing_deps.add_all_deps changes in
    let deps, dirty_master_deps = intersect_with_master_deps
      ~deps:changes
      ~dirty_master_deps:dirty_deps.dirty_master_deps in
    let needs_recheck = Typing_deps.get_files deps in
    let needs_recheck =
      Relative_path.Set.diff needs_recheck dirty_deps.rechecked_files in
    let size = Relative_path.Set.cardinal needs_recheck in
    let env = if size = 0 then env else begin
      Hh_logger.log "Adding %d files to recheck" size;
      let needs_recheck =
        Relative_path.Set.union env.needs_recheck needs_recheck in
      let full_check = match env.full_check with
        | Full_check_done -> Full_check_needed
        | x -> x
      in
      { env with needs_recheck; full_check; }
    end in
    HackEventLogger.prechecked_evaluate_incremental t size;
    set env (Prechecked_files_ready { dirty_deps with
      dirty_master_deps;
      clean_local_deps;
    })

let expand_all env =
  match env.prechecked_files with
  | Prechecked_files_disabled -> env
  | Initial_typechecking dirty_deps
  | Prechecked_files_ready dirty_deps ->
    let deps = Typing_deps.add_all_deps dirty_deps.dirty_master_deps in
    let needs_recheck = Typing_deps.get_files deps in
    let needs_recheck =
      Relative_path.Set.diff needs_recheck dirty_deps.rechecked_files in
    let env = if Relative_path.Set.is_empty needs_recheck then env else begin
      Hh_logger.log "Adding %d files to recheck after expanding all master deps"
        (Relative_path.Set.cardinal needs_recheck);
      let needs_recheck =
        Relative_path.Set.union env.needs_recheck needs_recheck in
      { env with
        needs_recheck;
        full_check = Full_check_started;
      }
    end in
    set env (Prechecked_files_ready { dirty_deps with
      dirty_master_deps = Typing_deps.DepSet.empty;
    })
