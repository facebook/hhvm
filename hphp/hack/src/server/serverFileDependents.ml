(*
 * Copyright (c) 2019, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE fn in the "hack" directory of this source tree.
 *
 *)

open Hh_prelude
module DepSet = Typing_deps.DepSet
module Dep = Typing_deps.Dep

(* Given the naming table, a list of relative paths,
 *   find the list of relative paths of all dependencies *)
let deps_of_paths ctx workers naming_table relative_paths =
  let deps_mode = Provider_context.get_deps_mode ctx in
  let find_dependencies acc paths =
    let fileinfos =
      List.filter_map ~f:(Naming_table.get_file_info naming_table) paths
    in
    let initial_deps =
      List.fold_left fileinfos ~init:(DepSet.make ()) ~f:(fun acc fileinfo ->
          let deps =
            Typing_deps.deps_of_file_info fileinfo |> Typing_deps.DepSet.of_list
          in
          DepSet.union acc deps)
    in
    DepSet.union acc (Typing_deps.add_all_deps deps_mode initial_deps)
  in
  let all_deps =
    MultiWorker.call
      workers
      ~job:find_dependencies
      ~neutral:(DepSet.make ())
      ~merge:DepSet.union
      ~next:(MultiWorker.next workers relative_paths)
  in
  all_deps |> Naming_provider.get_files ctx |> Relative_path.Set.elements

let go (genv : ServerEnv.genv) (env : ServerEnv.env) (filenames : string list) =
  let ctx = Provider_utils.ctx_from_server_env env in
  let workers = genv.ServerEnv.workers in
  let naming_table = env.ServerEnv.naming_table in
  let paths = List.map ~f:Relative_path.create_detect_prefix filenames in
  let all_deps = deps_of_paths ctx workers naming_table paths in
  List.map ~f:Relative_path.to_absolute all_deps
