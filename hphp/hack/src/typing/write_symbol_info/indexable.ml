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

let from_options ~paths ~paths_file ~include_hhi =
  let hhi_root = Path.make (Relative_path.path_of_prefix Relative_path.Hhi) in
  let hhi_files =
    match include_hhi with
    | false -> []
    | true ->
      Hhi.get_raw_hhi_contents ()
      |> Array.to_list
      |> List.filter ~f:(fun (fn, _) ->
             not (String.is_prefix fn ~prefix:"hsl_generated"))
      |> List.map ~f:(fun (fn, _) ->
             from_file
               (Relative_path.create
                  Relative_path.Hhi
                  Path.(to_string (concat hhi_root fn))))
  in
  let relative_path_exists r = Sys.file_exists (Relative_path.to_absolute r) in
  List.concat
    [
      Option.value_map paths_file ~default:[] ~f:In_channel.read_lines
      |> List.map ~f:parse_line;
      hhi_files;
      paths
      |> List.map ~f:(fun path -> Relative_path.from_root ~suffix:path)
      |> List.filter ~f:relative_path_exists
      |> List.map ~f:from_file;
    ]

let from_naming_table naming_table ~include_hhi ~ignore_paths =
  let defs_per_file = Naming_table.to_defs_per_file naming_table in
  Relative_path.Map.fold defs_per_file ~init:[] ~f:(fun path _ acc ->
      match Naming_table.get_file_info naming_table path with
      | None -> acc
      | Some _ ->
        let path_str = Relative_path.S.to_string path in
        if
          Relative_path.is_hhi (Relative_path.prefix path)
          && ((not include_hhi)
             || String.is_prefix path_str ~prefix:"hhi|hsl_generated")
          || List.exists ignore_paths ~f:(fun ignore ->
                 String.equal path_str ignore)
        then
          acc
        else
          from_file path :: acc)
