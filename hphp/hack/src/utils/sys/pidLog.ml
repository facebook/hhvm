(*
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Core_kernel
module Unix = Caml_unix

let log_oc = ref None

let enabled = ref true

let disable () = enabled := false

let init pids_file =
  assert (Option.is_none !log_oc);
  Sys_utils.with_umask 0o111 (fun () ->
      Sys_utils.mkdir_no_fail (Filename.dirname pids_file);
      let oc = Out_channel.create pids_file in
      log_oc := Some oc;
      Unix.(set_close_on_exec (descr_of_out_channel oc)))

let log ?reason ?(no_fail = false) pid =
  if !enabled then
    let pid = Sys_utils.pid_of_handle pid in
    let reason =
      match reason with
      | None -> "unknown"
      | Some s -> s
    in
    match !log_oc with
    | None when no_fail -> ()
    | None -> failwith "Can't write pid to uninitialized pids log"
    | Some oc -> Printf.fprintf oc "%d\t%s\n%!" pid reason

exception FailedToGetPids

let get_pids pids_file =
  try
    let ic = In_channel.create pids_file in
    let results = ref [] in
    begin
      try
        while true do
          let row = In_channel.input_line ic in
          match row with
          | None -> raise End_of_file
          | Some row ->
            if Str.string_match (Str.regexp "^\\([0-9]+\\)\t\\(.+\\)") row 0
            then
              let pid = int_of_string (Str.matched_group 1 row) in
              let reason = Str.matched_group 2 row in
              results := (pid, reason) :: !results
        done
      with End_of_file -> ()
    end;
    In_channel.close ic;
    List.rev !results
  with Sys_error _ -> raise FailedToGetPids

let close () =
  Option.iter !log_oc ~f:Out_channel.close;
  log_oc := None
