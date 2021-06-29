(*
 * Copyright (c) 2018, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)
open Hh_prelude

let go (filenames : string list) ctx =
  let dep_edges = HashSet.create () in
  let multi_remote_execution_trace root obj =
    if
      not
        (Typing_deps.idep_exists (Provider_context.get_deps_mode ctx) root obj)
    then
      HashSet.add dep_edges (root, obj)
  in
  Typing_deps.add_dependency_callback
    "multi_remote_execution_trace"
    multi_remote_execution_trace;
  (* filter out non-existent files *)
  let paths =
    List.filter_map filenames ~f:(fun file ->
        Sys_utils.realpath file
        |> Option.map ~f:Relative_path.create_detect_prefix)
  in
  let errors =
    List.fold_left paths ~init:Errors.empty ~f:(fun acc path ->
        let (ctx, entry) = Provider_context.add_entry_if_missing ~ctx ~path in
        let { Tast_provider.Compute_tast_and_errors.errors; _ } =
          Tast_provider.compute_tast_and_errors_unquarantined ~ctx ~entry
        in
        Errors.merge acc errors)
  in
  let errors = Base64.encode_exn ~pad:false (Marshal.to_string errors []) in
  let dep_edges = HashSet.to_list dep_edges in
  let dep_edges =
    Base64.encode_exn ~pad:false (Marshal.to_string dep_edges [])
  in
  (errors, dep_edges)
