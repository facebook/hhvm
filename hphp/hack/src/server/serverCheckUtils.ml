(**
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

let get_naming_table_fallback_path genv (deptable_fn: string option): string option =
  Hh_logger.log "Figuring out naming table SQLite path";
  match genv.ServerEnv.local_config.ServerLocalConfig.naming_sqlite_path with
  | Some path ->
    Hh_logger.log "Found path: %s" path;
    Some path
  | None when genv.ServerEnv.local_config.ServerLocalConfig.enable_naming_table_fallback ->
    Hh_logger.log "No path, using dep table";
    deptable_fn
  | None ->
    Hh_logger.log "Load from blob";
    None

let extend_fast fast naming_table additional_files =
  Relative_path.Set.fold additional_files ~init:fast ~f:begin fun x acc ->
    match Relative_path.Map.get fast x with
    | None ->
        (try
           let info = Naming_table.get_file_info_unsafe naming_table x in
           let info_names = FileInfo.simplify info in
           Relative_path.Map.add acc ~key:x ~data:info_names
         with Not_found ->
           acc)
    | Some _ -> acc
  end
