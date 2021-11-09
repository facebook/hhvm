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
