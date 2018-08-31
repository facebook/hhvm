(**
 * Copyright (c) 2018, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE fn in the "hack" directory of this source tree.
 *
 *)

open ServerEnv

let set env prechecked_files = { env with prechecked_files }

let update_after_recheck env =
    match env.full_check, env.prechecked_files with
  | Full_check_done, Initial_typechecking { dirty_local_deps } ->
    Hh_logger.log "Finished rechecking dirty files, evaluating their fanout";
    let deps = Typing_deps.add_all_deps dirty_local_deps in
    let needs_recheck = Typing_deps.get_files deps in

    let env = if Relative_path.Set.is_empty needs_recheck then env else begin
      Hh_logger.log "Adding %d files to recheck"
        (Relative_path.Set.cardinal needs_recheck);
      let full_check = Full_check_started in
      let init_env = { env.init_env with needs_full_init = true } in
      { env with needs_recheck; full_check; init_env }
    end in
    set env Prechecked_files_ready
  | _ -> env
