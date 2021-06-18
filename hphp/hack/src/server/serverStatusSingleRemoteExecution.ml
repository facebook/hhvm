(*
 * Copyright (c) 2018, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

let go file_name ctx =
  let dep_edges = HashSet.create () in
  let single_remote_execution_trace root obj =
    HashSet.add dep_edges (root, obj)
  in
  Typing_deps.add_dependency_callback
    "single_remote_execution_trace"
    single_remote_execution_trace;
  let errors =
    match Sys_utils.realpath file_name with
    | None -> Errors.empty
    | _ ->
      let path = Relative_path.create_detect_prefix file_name in
      let (ctx, entry) = Provider_context.add_entry_if_missing ~ctx ~path in
      let { Tast_provider.Compute_tast_and_errors.errors; _ } =
        Tast_provider.compute_tast_and_errors_unquarantined ~ctx ~entry
      in
      errors
  in
  let errors = Base64.encode_exn ~pad:false (Marshal.to_string errors []) in
  let dep_edges = HashSet.to_list dep_edges in
  let dep_edges =
    Base64.encode_exn ~pad:false (Marshal.to_string dep_edges [])
  in
  (errors, dep_edges)
