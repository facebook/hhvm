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
  Printf.sprintf "%s/%s-%s.sock" tmp_dir user root_part

let init_unix_socket www_root_path =
  unix_socket (get_path www_root_path)

let init_internal_socket =
  let counter = ref 0 in
  fun root ->
    incr counter;
    let nbr = string_of_int !counter in
    let root_part = (Path.slash_escaped_string_of_path root) in
    let sock_name = Tmp.get_dir()^"/"^root_part^"_data_"^nbr^".sock" in
    sock_name

