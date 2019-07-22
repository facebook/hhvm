(**
 * Copyright (c) 2019, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)
let () =
  let file_paths = ref [] in
  let parse_only = ref false in
  let options = [
    "--file-path", Arg.Rest (fun s -> file_paths := s :: !file_paths), "";
    "--parse-only", Arg.Set parse_only, "";
  ] in
  Arg.parse options (fun _ -> ()) "";

  List.iter (fun file_path ->
    let php5_compat_mode, hhvm_compat_mode = true, true in
    let filename = file_path |> Relative_path.create Relative_path.Dummy in
    let text = Sys_utils.cat file_path in
    let result = if !parse_only then
      match Facts_parser.from_text ~php5_compat_mode ~hhvm_compat_mode ~filename ~text with
      | Some _ -> "true"
      | _ -> "false"
    else
      match Facts_parser.extract_as_json ~php5_compat_mode ~hhvm_compat_mode ~filename ~text with
      | Some(hh_json) -> Hh_json.json_to_multiline ~sort_keys:true hh_json
      | None -> "{}"
    in
    Printf.printf "%s\n" result;
  ) !file_paths
