(*
 * Copyright (c) 2018, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)
open Hh_prelude
open ServerCommandTypes

(* Helper function to process a single file input *)
let process_file_input ctx ~error_filter file_input =
  match file_input with
  | FileName file_name ->
    let path = Relative_path.create_detect_prefix file_name in
    let (ctx, entry) = Provider_context.add_entry_if_missing ~ctx ~path in
    let {
      Tast_provider.Compute_tast_and_errors.diagnostics;
      tast;
      telemetry = _;
    } =
      Tast_provider.compute_tast_and_errors_unquarantined
        ~ctx
        ~entry
        ~error_filter
    in
    (diagnostics, Relative_path.Map.singleton path tast)
  | FileContent contents ->
    let (ctx, entry) =
      Provider_context.add_or_overwrite_entry_contents
        ~ctx
        ~path:Relative_path.default
        ~contents
    in
    let {
      Tast_provider.Compute_tast_and_errors.diagnostics;
      tast;
      telemetry = _;
    } =
      (* Explicitly put the contents of `ctx` in a quarantine, since they
         may overwrite naming table entries. *)
      Provider_utils.respect_but_quarantine_unsaved_changes ~ctx ~f:(fun () ->
          Tast_provider.compute_tast_and_errors_unquarantined
            ~ctx
            ~entry
            ~error_filter)
    in
    (diagnostics, Relative_path.Map.singleton Relative_path.default tast)

(* Single-threaded implementation *)
let go_sequential file_inputs ctx ~return_expanded_tast ~error_filter :
    Diagnostics.t * Tast.program Tast_with_dynamic.t Relative_path.Map.t =
  let collect (errors_acc, tasts) file_input =
    let (errors, file_tast) = process_file_input ctx ~error_filter file_input in
    let errors_acc = Diagnostics.merge errors errors_acc in
    let tasts =
      if return_expanded_tast then
        Relative_path.Map.union tasts file_tast
      else
        Relative_path.Map.empty
    in
    (errors_acc, tasts)
  in
  List.fold
    ~f:collect
    ~init:(Diagnostics.empty, Relative_path.Map.empty)
    file_inputs

(* Multi-threaded worker job function *)
let worker_job ctx ~error_filter (_worker_id, acc) file_inputs =
  List.fold
    ~f:(fun (errors_acc, tasts) file_input ->
      let (errors, file_tast) =
        process_file_input ctx ~error_filter file_input
      in
      let errors_acc = Diagnostics.merge errors errors_acc in
      let tasts = Relative_path.Map.union tasts file_tast in
      (errors_acc, tasts))
    ~init:acc
    file_inputs

(* Merge function for combining results from workers *)
let merge_results ~return_expanded_tast (_worker_id, result) acc =
  let (errors1, tasts1) = result in
  let (errors2, tasts2) = acc in
  let merged_errors = Diagnostics.merge errors1 errors2 in
  let merged_tasts =
    if return_expanded_tast then
      Relative_path.Map.union tasts1 tasts2
    else
      Relative_path.Map.empty
  in
  (merged_errors, merged_tasts)

(* Parallel implementation *)
let go_parallel workers file_inputs ctx ~return_expanded_tast ~error_filter :
    Diagnostics.t * Tast.program Tast_with_dynamic.t Relative_path.Map.t =
  let job = worker_job ctx ~error_filter in
  let next = MultiWorker.next workers file_inputs in
  MultiWorker.call_with_worker_id
    workers
    ~job
    ~merge:(merge_results ~return_expanded_tast)
    ~neutral:(Diagnostics.empty, Relative_path.Map.empty)
    ~next

(* Main entry point that decides whether to use parallel or sequential processing *)
let go workers file_inputs ctx ~return_expanded_tast ~error_filter :
    Diagnostics.t * Tast.program Tast_with_dynamic.t Relative_path.Map.t =
  let num_files = List.length file_inputs in
  (* Use parallel processing if there are enough files to justify the overhead *)
  if num_files >= 10 then
    go_parallel workers file_inputs ctx ~return_expanded_tast ~error_filter
  else
    go_sequential file_inputs ctx ~return_expanded_tast ~error_filter
