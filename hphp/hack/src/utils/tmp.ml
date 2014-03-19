(**
 * Copyright (c) 2014, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 *
 *)


(*****************************************************************************)
(* Handling where our temporary files go *)
(*****************************************************************************)

let get_dir ?user:(user=None) () =
  let user = match user with
    | None -> Sys.getenv "USER"
    | Some user -> user in
  let tmp_dir = Filename.temp_dir_name ^ "/hh_server_"^user in
  if not (Sys.file_exists tmp_dir)
  then Unix.mkdir tmp_dir 0o777;
  tmp_dir
