(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Core_kernel

let root (value_ref : string ref) : string * Arg.spec * string =
  ( "--root",
    Arg.String
      (fun s ->
        let hhconfig = s ^ "/.hhconfig" in
        if not (Sys.file_exists hhconfig) then
          failwith ("--root incorrect. not found: " ^ hhconfig);
        value_ref := s),
    " root of project, where .hhconfig is" )

let only (cmd : string) : string -> unit =
  let has_been_invoked = ref false in
  fun s ->
    if !has_been_invoked || s <> cmd then
      failwith ("unexpected " ^ s)
    else
      has_been_invoked := true

let tmp_path_of_root (root : string) (ext : string) : string =
  (* TODO: move this into GlobalConfig out of ServerFiles *)
  Sys_utils.mkdir_no_fail GlobalConfig.tmp_dir;
  let root_part = Path.slash_escaped_string_of_path (Path.make root) in
  Filename.concat GlobalConfig.tmp_dir (Printf.sprintf "%s%s" root_part ext)

let prototype_lock_file (root : string) : string =
  tmp_path_of_root root ".prototype.lock"

let prototype_sock_file (root : string) : string =
  tmp_path_of_root root ".prototype.sock"
