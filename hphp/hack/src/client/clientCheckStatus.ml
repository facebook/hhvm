(*
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Core_kernel
open Ocaml_overrides
open ServerCommandTypes
open String_utils
module C = Tty

let print_reason_color
    ~(first : bool) ~(code : int) ((p, s) : Pos.absolute * string) =
  let (line, start, end_) = Pos.info_pos p in
  let code_clr = C.Normal C.Yellow in
  let err_clr =
    if first then
      C.Bold C.Red
    else
      C.Normal C.Green
  in
  let file_clr =
    if first then
      C.Bold C.Red
    else
      C.Normal C.Red
  in
  let line_clr = C.Normal C.Yellow in
  let col_clr = C.Normal C.Cyan in
  let to_print_code =
    if not first then
      []
    else
      [
        (C.Normal C.Default, " (");
        (code_clr, Errors.error_code_to_string code);
        (C.Normal C.Default, ")");
      ]
  in
  let to_print =
    [
      (line_clr, string_of_int line);
      (C.Normal C.Default, ":");
      (col_clr, string_of_int start);
      (C.Normal C.Default, ",");
      (col_clr, string_of_int end_);
      (C.Normal C.Default, ": ");
      (err_clr, s);
    ]
    @ to_print_code
    @ [(C.Normal C.Default, "\n")]
  in
  if not first then
    Printf.printf "  "
  else
    ();
  if Unix.isatty Unix.stdout then
    let cwd = Filename.concat (Sys.getcwd ()) "" in
    let file_path =
      [(file_clr, lstrip (Pos.filename p) cwd); (C.Normal C.Default, ":")]
    in
    C.cprint (file_path @ to_print)
  else
    let strings = List.map to_print (fun (_, x) -> x) in
    Printf.printf "%s:" (Pos.filename p);
    List.iter strings (Printf.printf "%s")

let print_error_color e =
  let code = Errors.get_code e in
  let msg_list = Errors.to_list e in
  print_reason_color ~first:true ~code (List.hd_exn msg_list);
  List.iter (List.tl_exn msg_list) (print_reason_color ~first:false ~code)

let print_error_contextual e =
  Printf.printf "%s" (Errors.to_contextual_string e)

let is_stale_msg liveness =
  match liveness with
  | Stale_status ->
    Some
      ( "(but this may be stale, probably due to"
      ^ " watchman being unresponsive)\n" )
  | Live_status -> None

let warn_unsaved_changes () =
  (* Make sure any buffered diagnostics are printed before printing this
     warning. *)
  Out_channel.flush stdout;
  Tty.cprintf (Tty.Bold Tty.Yellow) "Warning: " ~out_channel:stderr;
  prerr_endline
    {|there is an editor connected to the Hack server.
The errors above may reflect your unsaved changes in the editor.|}

let go status output_json from error_format max_errors =
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
  ( if output_json || from <> "" || error_list = [] then
    (* this should really go to stdout but we need to adapt the various
     * IDE plugins first *)
    let oc =
      if output_json then
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
      | Errors.Context -> print_error_contextual
      | Errors.Raw -> print_error_color
    in
    List.iter error_list f;
    Option.iter
      (Errors.format_summary error_format error_list dropped_count max_errors)
      ~f:(fun msg -> Printf.printf "%s" msg);
    Option.iter stale_msg ~f:(fun msg -> Printf.printf "%s" msg);
    if has_unsaved_changes then warn_unsaved_changes () );

  (* don't indicate errors in exit code for warnings; warnings shouldn't break
   * CI *)
  if List.exists ~f:(fun e -> Errors.get_severity e = Errors.Error) error_list
  then
    Exit_status.Type_error
  else
    Exit_status.No_error
