(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Hh_prelude
open Hh_json

type t = {
  path: Relative_path.t;
  fanout: bool;
}

let from_file path = { path; fanout = true }

let parse_line s =
  if String.is_prefix s ~prefix:"{" then
    match json_of_string s with
    | JSON_Object [("path", JSON_String p); ("fanout", JSON_Bool f)] ->
      { path = Relative_path.storage_of_string p; fanout = f }
    | _ -> failwith "Can't parse paths file"
  else
    (* for backward compatibility *)
    { path = Relative_path.storage_of_string s; fanout = false }

let from_options ~paths ~paths_file =
  let relative_path_exists r = Sys.file_exists (Relative_path.to_absolute r) in
  List.concat
    [
      Option.value_map paths_file ~default:[] ~f:In_channel.read_lines
      |> List.map ~f:parse_line;
      paths
      |> List.map ~f:(fun path -> Relative_path.from_root ~suffix:path)
      |> List.filter ~f:relative_path_exists
      |> List.map ~f:from_file;
    ]

let from_naming_table naming_table ~exclude_hhi ~ignore_paths =
  let defs_per_file = Naming_table.to_defs_per_file naming_table in
  Relative_path.Map.fold defs_per_file ~init:[] ~f:(fun path _ acc ->
      match Naming_table.get_file_info naming_table path with
      | None -> acc
      | Some _ ->
        let path_str = Relative_path.S.to_string path in
        if
          Relative_path.is_hhi (Relative_path.prefix path)
          && (exclude_hhi
             || String.is_prefix path_str ~prefix:"hhi|hsl_generated")
          || List.exists ignore_paths ~f:(fun ignore ->
                 String.equal path_str ignore)
        then
          acc
        else
          from_file path :: acc)
