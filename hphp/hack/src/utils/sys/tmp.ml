(*
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

(*****************************************************************************)
(* Handling where our temporary files go *)
(*****************************************************************************)

let temp_dir parent_dir prefix =
  Sys_utils.mkdir_no_fail parent_dir;
  let tmpdir =
    Filename.concat
      parent_dir
      (Printf.sprintf "%s_%06x" prefix (Random.bits ()))
  in
  Sys_utils.mkdir_no_fail tmpdir;
  tmpdir

let hh_server_tmp_dir =
  try Sys.getenv "HH_TMPDIR" with
  | _ ->
    Path.to_string
    @@ Path.concat (Path.make Sys_utils.temp_dir_name) "hh_server"

let ( // ) = Filename.concat

let make_dir_in_tmp ~description_what_for ~root =
  let tmp = hh_server_tmp_dir in
  assert (String.length description_what_for > 0);
  let dir = tmp // description_what_for in
  let dir =
    match root with
    | None -> dir
    | Some root -> dir // Path.slash_escaped_string_of_path root
  in
  Sys_utils.mkdir_p dir;
  dir
