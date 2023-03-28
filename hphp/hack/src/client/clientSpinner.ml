(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Hh_prelude

let ascii_chars = [| "-"; "\\"; "|"; "/" |]

(** Some terminals display the emoji using only one column, even though they
may take up two columns, and put the cursor immediately after it in an
illegible manner. We append an extra space to separate the cursor from the emoji. *)
let emoji_chars =
  [|
    "\xF0\x9F\x98\xA1 ";
    (* Angry Face *)
    "\xF0\x9F\x98\x82 ";
    (* Face With Tears of Joy *)
    "\xF0\x9F\xA4\x94 ";
    (* Thinking Face *)
    "\xF0\x9F\x92\xAF ";
    (* Hundred Points *)
  |]

type state = {
  message: string option;  (** "hh_server is busy: [<message>] <spinner>" *)
  to_stderr: bool;  (** should this be displayed on stderr? *)
  angery_reaccs_only: bool; (* do we use emoji_chars or ascii_chars? *)
  index: int;  (** which of the four chars is used as spinner *)
}

(** The current state of the spinner. Invariant: whatever this current state implies
is reflected in stderr -- either some message, or nothing. *)
let state : state ref =
  ref
    { message = None; to_stderr = false; angery_reaccs_only = false; index = 0 }

let start_heartbeat_telemetry () : unit =
  let rec loop n : 'a =
    let%lwt _ = Lwt_unix.sleep 1.0 in
    HackEventLogger.spinner_heartbeat n !state.message;
    loop (n + 1)
  in
  let _future = loop 1 in
  ()

(** This updates stderr to reflect new_state. This might involve clearing
the old stderr (if there was anything on it), and displaying a new thing
on stderr (if needed). *)
let update_stderr (old_state : state) ~(new_state : state) : unit =
  let was_present = old_state.to_stderr && Option.is_some old_state.message in
  if was_present then Tty.print_clear_line stderr;
  match new_state.message with
  | Some message when new_state.to_stderr ->
    let chars =
      if new_state.angery_reaccs_only then
        emoji_chars
      else
        ascii_chars
    in
    let spinner = Array.get chars new_state.index in
    Tty.eprintf "hh_server is busy: [%s] %s%!" message spinner
  | _ -> ()

(** Never-ending background process to update the spinner on stderr once a second.
This is kicked off by the first time someone calls [report]. *)
let rec animate () : unit Lwt.t =
  let%lwt () = Lwt_unix.sleep 1.0 in
  let new_state = { !state with index = (!state.index + 1) % 4 } in
  update_stderr !state ~new_state;
  state := new_state;
  animate ()

(** Mutable field so that [report] knows whether it needs to kick off [animate] in the background. *)
let animator : unit Lwt.t option ref = ref None

let report ~(to_stderr : bool) ~(angery_reaccs_only : bool) :
    string option -> unit =
 fun message ->
  if Option.is_none !animator then animator := Some (animate ());
  if
    Option.is_some message
    && not (Option.equal String.equal !state.message message)
  then
    Hh_logger.log "spinner: [%s]" (Option.value_exn message);
  let new_state = { !state with message; to_stderr; angery_reaccs_only } in
  update_stderr !state ~new_state;
  state := new_state;
  ()
