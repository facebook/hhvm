(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Hh_prelude

type t = Relative_path.t

let from_options ~paths ~paths_file =
  let relative_path_exists r = Sys.file_exists (Relative_path.to_absolute r) in
  List.concat
    [
      Option.value_map paths_file ~default:[] ~f:In_channel.read_lines
      |> List.map ~f:Relative_path.storage_of_string;
      paths
      |> List.map ~f:(fun path -> Relative_path.from_root ~suffix:path)
      |> List.filter ~f:relative_path_exists;
    ]

let from_naming_table naming_table ~failed_parsing ~exclude_hhi ~ignore_paths =
  let defs_per_file = Naming_table.to_defs_per_file naming_table in
  let defs_per_file =
    Relative_path.Set.fold
      failed_parsing
      ~f:(fun x m -> Relative_path.Map.remove m x)
      ~init:defs_per_file
  in
  Relative_path.Map.fold defs_per_file ~init:[] ~f:(fun path _ acc ->
      match Naming_table.get_file_info naming_table path with
      | None -> acc
      | Some _ ->
        let path_str = Relative_path.S.to_string path in
        if
          Relative_path.is_hhi (Relative_path.prefix path)
          && (exclude_hhi
             || String_utils.string_starts_with path_str "hhi|hsl_generated")
          || List.exists ignore_paths ~f:(fun ignore ->
                 String.equal path_str ignore)
        then
          acc
        else
          path :: acc)
