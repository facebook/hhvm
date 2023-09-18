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

let go file_inputs ctx :
    Errors.finalized_error list
    * Tast.program Tast_with_dynamic.t Relative_path.Map.t =
  let collect (errors_acc, tasts) file_input =
    match file_input with
    | FileName file_name ->
      let path = Relative_path.create_detect_prefix file_name in
      let (ctx, entry) = Provider_context.add_entry_if_missing ~ctx ~path in
      let { Tast_provider.Compute_tast_and_errors.errors; tast; telemetry = _ }
          =
        Tast_provider.compute_tast_and_errors_unquarantined ~ctx ~entry
      in
      let errors_acc = Errors.merge errors errors_acc in
      let tasts = Relative_path.Map.add tasts ~key:path ~data:tast in
      (errors_acc, tasts)
    | FileContent contents ->
      let (ctx, entry) =
        Provider_context.add_or_overwrite_entry_contents
          ~ctx
          ~path:Relative_path.default
          ~contents
      in
      let { Tast_provider.Compute_tast_and_errors.errors; tast; telemetry = _ }
          =
        (* Explicitly put the contents of `ctx` in a quarantine, since they
           may overwrite naming table entries. *)
        Provider_utils.respect_but_quarantine_unsaved_changes ~ctx ~f:(fun () ->
            Tast_provider.compute_tast_and_errors_unquarantined ~ctx ~entry)
      in
      let errors_acc = Errors.merge errors errors_acc in
      let tasts =
        Relative_path.Map.add tasts ~key:Relative_path.default ~data:tast
      in
      (errors_acc, tasts)
  in
  let (errors, tasts) =
    List.fold
      ~f:collect
      ~init:(Errors.empty, Relative_path.Map.empty)
      file_inputs
  in
  let errors =
    Errors.get_sorted_error_list errors |> List.map ~f:User_error.to_absolute
  in
  (errors, tasts)
