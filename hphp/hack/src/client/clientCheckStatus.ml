(*
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Hh_prelude
open ServerCommandTypes

let print_error_raw e = Printf.printf "%s" (Raw_error_formatter.to_string e)

let print_error_plain e = Printf.printf "%s" (Errors.to_string e)

let print_error_contextual e =
  Printf.printf "%s" (Contextual_error_formatter.to_string e)

let print_error_highlighted e =
  Printf.printf "%s" (Highlighted_error_formatter.to_string e)

let is_stale_msg liveness =
  match liveness with
  | Stale_status ->
    Some
      ("(but this may be stale, probably due to"
      ^ " watchman being unresponsive)\n")
  | Live_status -> None

let warn_unsaved_changes () =
  (* Make sure any buffered diagnostics are printed before printing this
     warning. *)
  Out_channel.flush stdout;
  Tty.cprintf (Tty.Bold Tty.Yellow) "Warning: " ~out_channel:stderr;
  prerr_endline
    {|there is an editor connected to the Hack server.
The errors above may reflect your unsaved changes in the editor.|}

let go status (output_json, prefer_stdout) from error_format max_errors =
  let {
    Server_status.liveness;
    has_unsaved_changes;
    error_list;
    dropped_count;
    last_recheck_stats;
  } =
    status
  in
  let stale_msg = is_stale_msg liveness in
  (if output_json || (not (String.equal from "")) || List.is_empty error_list
  then
    (* this should really go to stdout but we need to adapt the various
     * IDE plugins first *)
    let oc =
      if output_json && not prefer_stdout then
        stderr
      else
        stdout
    in
    ServerError.print_error_list
      oc
      ~stale_msg
      ~output_json
      ~error_list
      ~save_state_result:None
      ~recheck_stats:last_recheck_stats
  else
    let f =
      match error_format with
      | Errors.Raw -> print_error_raw
      | Errors.Plain -> print_error_plain
      | Errors.Context -> print_error_contextual
      | Errors.Highlighted -> print_error_highlighted
    in
    List.iter error_list ~f;
    Option.iter
      (Errors.format_summary error_format error_list dropped_count max_errors)
      ~f:(fun msg -> Printf.printf "%s" msg);
    Option.iter stale_msg ~f:(fun msg -> Printf.printf "%s" msg);
    if has_unsaved_changes then warn_unsaved_changes ());

  if List.is_empty error_list then
    Exit_status.No_error
  else
    Exit_status.Type_error
