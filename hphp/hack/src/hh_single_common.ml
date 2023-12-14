(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Hh_prelude

(** Finds an arbitrary naming-table within the saved-states directory,
or fails if none can be found. *)
let find_naming_table_or_fail () : string =
  let dir = ServerFiles.saved_state_download_dir in
  let candidates =
    Disk.readdir dir
    |> Array.to_list
    |> List.map ~f:(fun s ->
           Printf.sprintf "%s/%s/hh_mini_saved_state_naming.sql" dir s)
    |> List.filter ~f:Sys.file_exists
  in
  match candidates with
  | [] -> failwith ("--root needs --naming-table, or to find one in " ^ dir)
  | naming_table :: _ ->
    Printf.eprintf
      "WARNING: --naming-table wasn't specified, so guessing this one will work (out of %d total candidates):\n%s\n"
      (List.length candidates)
      naming_table;
    naming_table
