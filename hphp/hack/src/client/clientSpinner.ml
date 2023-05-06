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

type message =
  | Init
      (** There has not yet been a call to [report (Some s)], so there's nothing to show *)
  | Message of {
      text: string;
          (** This reflects the most recent call to [report (Some text)].
          It is displayed as "hh_server is busy: [<s>] <spinner>". *)
      is_hidden: bool;
          (** This is true when [report None] has been called more recently than [report Some].
          It means that the text won't be displayed on stderr.
          (However, the text is still used for heartbeat and logging). *)
    }

type state = {
  message: message;
  to_stderr: bool;  (** should this be displayed on stderr? *)
  angery_reaccs_only: bool; (* do we use emoji_chars or ascii_chars? *)
  index: int;  (** which of the four chars is used as spinner *)
}

(** The current state of the spinner. Invariant: whatever this current state implies
is reflected in stderr -- either some message, or nothing. *)
let state : state ref =
  ref
    { message = Init; to_stderr = false; angery_reaccs_only = false; index = 0 }

let get_latest_report () : string option =
  match !state.message with
  | Message { text; is_hidden = false } -> Some text
  | Message { text; is_hidden = true } -> Some ("[hidden] " ^ text)
  | Init -> None

(** Used so that when we Hh_logger.log a spinner change, we can show wall-time. *)
let start_time : float = Unix.gettimeofday ()

let start_heartbeat_telemetry () : unit =
  let rec loop n : 'a =
    let%lwt _ = Lwt_unix.sleep 1.0 in
    HackEventLogger.spinner_heartbeat n (get_latest_report ());
    loop (n + 1)
  in
  let _future = loop 1 in
  ()

(** This updates stderr to reflect new_state. This might involve clearing
the old stderr (if there was anything on it), and displaying a new thing
on stderr (if needed). *)
let update_stderr (old_state : state) ~(new_state : state) : unit =
  let was_present =
    match old_state.message with
    | Message { is_hidden = false; _ } when old_state.to_stderr -> true
    | _ -> false
  in
  if was_present then Tty.print_clear_line stderr;
  match new_state.message with
  | Message { is_hidden = false; text } when new_state.to_stderr ->
    let chars =
      if new_state.angery_reaccs_only then
        emoji_chars
      else
        ascii_chars
    in
    let spinner = Array.get chars new_state.index in
    Tty.eprintf "hh_server is busy: [%s] %s%!" text spinner
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
 fun next ->
  if Option.is_none !animator then animator := Some (animate ());
  let message =
    match (!state.message, next) with
    | (Message { text = prev; _ }, Some next) when String.equal prev next ->
      (* text is unchanged. But if it was hidden before, now let it be shown *)
      Message { text = next; is_hidden = false }
    | (_, Some next) ->
      (* text has changed *)
      Hh_logger.log
        "spinner %0.1fs: [%s]"
        (Unix.gettimeofday () -. start_time)
        next;
      Message { text = next; is_hidden = false }
    | (Message { text; _ }, None) ->
      (* pre-existing text gets hidden *)
      Message { text; is_hidden = true }
    | (Init, None) ->
      (* no-op, because we're hiding when no text has yet been reported *)
      Init
  in
  let new_state = { !state with message; to_stderr; angery_reaccs_only } in
  update_stderr !state ~new_state;
  state := new_state;
  ()
