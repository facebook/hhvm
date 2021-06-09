(*
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Hh_prelude

[@@@warning "-52"]

(* we have no alternative but to depend on Sys_error strings *)

let log genv msg_thunk =
  Option.iter genv.ServerEnv.debug_channels ~f:(fun (_ic, oc) ->
      (* The input is read using input_line, hence we append a trailing
       * newline *)
      (* We use a thunk so that expensive computations don't slow down the
       * server when the debug listener isn't attached *)
      let msg = msg_thunk () in
      try
        Out_channel.output_string oc (Hh_json.json_to_string msg ^ "\n");
        Out_channel.flush oc
      with Sys_error "Broken pipe" ->
        Hh_logger.log "Debug listener has gone away.";
        genv.ServerEnv.debug_channels <- None)

[@@@warning "+52"] (* CARE! scope of suppression should be only 'log' *)

let info genv msg =
  Hh_json.(
    let msg =
      JSON_Object [("type", JSON_String "info"); ("data", JSON_String msg)]
    in
    log genv (fun () -> msg))

let say_hello genv = info genv "hello"
