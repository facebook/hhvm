(*
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Hh_core

(* These severity levels are based on those provided by Arcanist. "Advice"
 * means notify the user of the lint without requiring confirmation if the lint
 * is benign; "Warning" will raise a confirmation prompt if the lint applies to
 * a line that was changed in the given diff; and "Error" will always raise a
 * confirmation prompt, regardless of where the lint occurs in the file. *)
type severity =
  | Lint_error
  | Lint_warning
  | Lint_advice
[@@deriving show]

let string_of_severity = function
  | Lint_error -> "error"
  | Lint_warning -> "warning"
  | Lint_advice -> "advice"

type 'a t = {
  code: int;
  severity: severity;
  pos: 'a Pos.pos; [@opaque]
  message: string;
  (* Normally, lint warnings and lint advice only get shown by arcanist if the
   * lines they are raised on overlap with lines changed in a diff. This
   * flag bypasses that behavior *)
  bypass_changed_lines: bool;
  autofix: string * string;
}
[@@deriving show]

let (lint_list : Relative_path.t t list option ref) = ref None

let get_code { code; _ } = code

let get_pos { pos; _ } = pos

let add
    ?(bypass_changed_lines = false)
    ?(autofix = ("", ""))
    code
    severity
    pos
    message =
  match !lint_list with
  | Some lst ->
    let lint =
      { code; severity; pos; message; bypass_changed_lines; autofix }
    in
    lint_list := Some (lint :: lst)
  (* by default, we ignore lint errors *)
  | None -> ()

let to_absolute ({ pos; _ } as lint) = { lint with pos = Pos.to_absolute pos }

let to_string lint =
  let code = Errors.error_code_to_string lint.code in
  Printf.sprintf "%s\n%s (%s)" (Pos.string lint.pos) lint.message code

let to_contextual_string lint =
  let color =
    match lint.severity with
    | Lint_error -> Tty.apply_color (Tty.Bold Tty.Red)
    | Lint_warning -> Tty.apply_color (Tty.Bold Tty.Yellow)
    | Lint_advice -> Tty.apply_color (Tty.Bold Tty.White)
  in
  let heading =
    Printf.sprintf
      "%s %s"
      (color (Errors.error_code_to_string lint.code))
      (Tty.apply_color (Tty.Bold Tty.White) lint.message)
  in
  let fn = Errors.format_filename lint.pos in
  let (ctx, msg) =
    Errors.format_message "" lint.pos ~is_first:true ~col_width:None
  in
  Printf.sprintf "%s\n%s\n%s\n%s\n" heading fn ctx msg

let to_json
    {
      pos;
      code;
      severity;
      message;
      bypass_changed_lines;
      autofix = (original, replacement);
    } =
  let (line, scol, ecol) = Pos.info_pos pos in
  Hh_json.JSON_Object
    [
      ("descr", Hh_json.JSON_String message);
      ("severity", Hh_json.JSON_String (string_of_severity severity));
      ("path", Hh_json.JSON_String (Pos.filename pos));
      ("line", Hh_json.int_ line);
      ("start", Hh_json.int_ scol);
      ("end", Hh_json.int_ ecol);
      ("code", Hh_json.int_ code);
      ("bypass_changed_lines", Hh_json.JSON_Bool bypass_changed_lines);
      ("original", Hh_json.JSON_String original);
      ("replacement", Hh_json.JSON_String replacement);
    ]

let do_ f =
  let list_copy = !lint_list in
  lint_list := Some [];
  let result = f () in
  let out =
    match !lint_list with
    | Some lst -> lst
    | None -> assert false
  in
  lint_list := list_copy;
  (List.rev out, result)
