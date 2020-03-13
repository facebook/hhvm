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
  let path =
    match file_input with
    | FileName file_name -> Relative_path.create_detect_prefix file_name
    | FileContent _ -> Relative_path.default
  in
  let (_ctx, entry) =
    Provider_context.add_entry_from_file_input ~ctx ~path ~file_input
  in
  let { Tast_provider.Compute_tast_and_errors.errors; _ } =
    Tast_provider.compute_tast_and_errors_unquarantined ~ctx ~entry
  in
  errors |> Errors.get_sorted_error_list |> List.map ~f:Errors.to_absolute
