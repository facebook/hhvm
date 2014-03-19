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
(* dfind is a binary we call whenever we want to know if something changed   *)
(*****************************************************************************)
open Utils
open ServerEnv


(*****************************************************************************)
(* dfind doesn't always pick up the token 
 * however, it seems that in some cases, despite the fact that the token was
 * not picked up, dfind is still healthy.
 * So here is our strategy:
 * 1) retry calling dfind
 * 2) every five times, retry calling dfind from scratch
 * 3) after 20 times, kill the server
 * It should make things a lot more resilient to failure, in the mean time
 * we will try to figure out what is going on.
 *)
 (*****************************************************************************)

let dfind_proc = ref None
let dfind_pid = ref None

let dfind_init root =
  let proc, pid = DfindLib.start (Path.string_of_path root) in
  PidLog.log ~reason:(Some "dfind") pid;
  dfind_proc := Some proc;
  dfind_pid := Some pid

let rec dfind genv (root:Path.path) retries =
  (match !dfind_proc with
  | None -> assert false
  | Some x -> 
      DfindEnv.SSet.fold SSet.add (DfindLib.get_changes x) SSet.empty)

let dfind genv root = dfind genv root 20

let rec get_updates_ (acc_php, acc_js) genv root =
  let diff = dfind genv root in
  let diff_php = SSet.filter Find.is_php_path diff in
  let diff_js = SSet.filter Find.is_js_path diff in
  if SSet.is_empty diff
  then acc_php, acc_js
  else begin
    let acc_php = SSet.union diff_php acc_php in
    let acc_js = SSet.union diff_js acc_js in
    get_updates_ (acc_php, acc_js) genv root
  end

let get_updates genv root = get_updates_ (SSet.empty, SSet.empty) genv root
