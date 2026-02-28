(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

let worker_name_fmt : (_, unit, string) format =
  "worker_process_%d_out_of_%d_for_server_pid_%d"

let is_worker_process_name worker_name =
  let worker_name_regexp =
    Printf.sprintf worker_name_fmt 0 0 0
    |> Str.global_replace (Str.regexp "0") {|[0-9]+|}
    |> Str.regexp
  in
  Str.string_match worker_name_regexp worker_name 0

let is_worker_process () =
  if Array.length Sys.argv < 2 then
    false
  else
    let worker_name = Sys.argv.(1) in
    is_worker_process_name worker_name
