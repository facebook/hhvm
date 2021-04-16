(*
 * Copyright (c) 2018, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)
open Hh_prelude
open Hh_json

let go file_name ctx =
  let dep_edges = HashSet.create () in
  let single_remote_execution_trace root obj =
    let dependent = Typing_deps.Dep.variant_to_string root in
    let dependency = Typing_deps.Dep.variant_to_string obj in
    HashSet.add
      dep_edges
      (Hh_json.JSON_Object
         [
           ("dependent", JSON_String dependent);
           ("dependency", JSON_String dependency);
         ])
  in
  Typing_deps.add_dependency_callback
    "single_remote_execution_trace"
    single_remote_execution_trace;
  let errors =
    let path = Relative_path.create_detect_prefix file_name in
    let (ctx, entry) = Provider_context.add_entry_if_missing ~ctx ~path in
    let { Tast_provider.Compute_tast_and_errors.errors; _ } =
      Tast_provider.compute_tast_and_errors_unquarantined ~ctx ~entry
    in
    errors
  in
  let errors =
    errors |> Errors.get_sorted_error_list |> List.map ~f:Errors.to_absolute
  in
  let json_string =
    JSON_Array (HashSet.fold dep_edges ~init:[] ~f:(fun x acc -> x :: acc))
  in
  Printf.eprintf "DEPS: %s\n" (Hh_json.json_to_string json_string);
  (errors, Hh_json.json_to_string json_string)
