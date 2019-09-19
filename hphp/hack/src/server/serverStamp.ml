(*
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

(* The contract of this stamp "/tmp/hh_server/stamp" file is:
 * If someone wants to keep a cache of whether a given file typechecks
 * cleanly, then they can look for changes to the file to know that
 * their cache might have become invalid and should be re-checked.
 *
 * Only known consumer is ext_hh_client, an HHVM client which
 * exposes some APIs in the namespace HH\Client for typechecking a file.
 *
 * Note that there's only one stamp file on a system, even though there
 * might be several separate instances of hh_server.
 *)

let stamp_file = Filename.concat GlobalConfig.tmp_dir "stamp"

let touch_stamp () =
  Sys_utils.mkdir_no_fail (Filename.dirname stamp_file);
  Sys_utils.with_umask 0o111 (fun () ->
      (* Open and close the file to set its mtime. Don't use the Unix.utimes
       * function since that will fail if the stamp file doesn't exist. *)
      close_out (open_out stamp_file))

let touch_stamp_errors l1 l2 =
  (* We don't want to needlessly touch the stamp file if the error list is
   * the same and nothing has changed, but we also don't want to spend a ton
   * of time comparing huge lists of errors over and over (i.e., grind to a
   * halt in the cases when there are thousands of errors). So we cut off
   * the comparison at an arbitrary point. *)
  let rec length_greater_than n = function
    | [] -> false
    | _ when n = 0 -> true
    | _ :: l -> length_greater_than (n - 1) l
  in
  if length_greater_than 5 l1 || length_greater_than 5 l2 || l1 <> l2 then
    touch_stamp ()
