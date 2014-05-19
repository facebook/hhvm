(**
 * Copyright (c) 2014, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 *
 *)



let init_socket port =
  if port = 0 then None else
  let sock = Unix.socket Unix.PF_INET Unix.SOCK_STREAM 0 in
  let _ = Unix.setsockopt sock Unix.SO_REUSEADDR true in
  let _ = Unix.bind sock (
    Unix.ADDR_INET (Unix.inet_addr_any, port)) in
  let _ = Unix.listen sock 10 in
  Some sock

(* Initializes the unix domain socket *)
let unix_socket sock_name =
  try
    if Sys.file_exists sock_name then Sys.remove sock_name;
    let sock = Unix.socket Unix.PF_UNIX Unix.SOCK_STREAM 0 in
    let _ = Unix.setsockopt sock Unix.SO_REUSEADDR true in
    let _ = Unix.bind sock (Unix.ADDR_UNIX sock_name) in
    let _ = Unix.listen sock 10 in
    sock
  with Unix.Unix_error (err, _, _) ->
    Printf.fprintf stderr "%s\n" (Unix.error_message err);
    exit 1

let get_path ?user:(user=None) root =
  let tmp_dir = Tmp.get_dir ~user () in
  let user = match user with
  | Some user -> user
  | None -> Sys.getenv "USER" in
  let root_part = (Path.slash_escaped_string_of_path root) in 
  let shortened_root_part = if String.length root_part > 50
    then begin
      let len = String.length root_part in
      let prefix = String.sub root_part 0 5 in
      let suffix = String.sub root_part (len - 5) 5 in
      let digest = Digest.to_hex (Digest.string root_part) in
      prefix ^ "." ^ digest ^ "." ^ suffix
    end else root_part in
  Printf.sprintf "%s/%s-%s.sock" tmp_dir user shortened_root_part

let init_unix_socket www_root_path =
  unix_socket (get_path www_root_path)
