(*
 * Copyright (c) 2018, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)
open Hh_core
open ServerCommandTypes

let go file_input ctx =
  let errors =
    match file_input with
    | FileName file_name ->
      let path = Relative_path.create_detect_prefix file_name in
      let (ctx, entry) = Provider_context.add_entry_if_missing ~ctx ~path in
      let { Tast_provider.Compute_tast_and_errors.errors; _ } =
        Tast_provider.compute_tast_and_errors_unquarantined ~ctx ~entry
      in
      errors
    | FileContent contents ->
      let (ctx, entry) =
        Provider_context.add_or_overwrite_entry_contents
          ~ctx
          ~path:Relative_path.default
          ~contents
      in
      let { Tast_provider.Compute_tast_and_errors.errors; _ } =
        (* Explicitly put the contents of `ctx` in a quarantine, since they
        may overwrite naming table entries. *)
        Provider_utils.respect_but_quarantine_unsaved_changes ~ctx ~f:(fun () ->
            Tast_provider.compute_tast_and_errors_unquarantined ~ctx ~entry)
      in
      errors
  in
  errors |> Errors.get_sorted_error_list |> List.map ~f:Errors.to_absolute
