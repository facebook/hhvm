(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

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

type 'pos t = {
  code: int;
  severity: severity;
  pos: 'pos; [@opaque]
  message: string;
  (* Normally, lint warnings and lint advice only get shown by arcanist if the
   * lines they are raised on overlap with lines changed in a diff. This
   * flag bypasses that behavior *)
  bypass_changed_lines: bool;
  autofix: (string * string * int * int) option;
}
[@@deriving show]

let (lint_list : Pos.t t list option ref) = ref None

let get_code { code; _ } = code

let get_pos { pos; _ } = pos

let add
    ?(bypass_changed_lines = false) ?(autofix = None) code severity pos message
    =
  match !lint_list with
  | Some lst ->
    let lint =
      { code; severity; pos; message; bypass_changed_lines; autofix }
    in
    lint_list := Some (lint :: lst)
  (* by default, we ignore lint errors *)
  | None -> ()

let add_lint lint =
  add
    ~bypass_changed_lines:lint.bypass_changed_lines
    ~autofix:lint.autofix
    lint.code
    lint.severity
    lint.pos
    lint.message

let to_absolute ({ pos; _ } as lint) = { lint with pos = Pos.to_absolute pos }

let to_string lint =
  let code = User_error.error_code_to_string lint.code in
  Printf.sprintf "%s\n%s (%s)" (Pos.string lint.pos) lint.message code

let to_contextual_string lint =
  let claim_color =
    match lint.severity with
    | Lint_error -> Tty.Red
    | Lint_warning -> Tty.Yellow
    | Lint_advice -> Tty.Default
  in
  User_error.make_absolute lint.code [(lint.pos, lint.message)]
  |> Contextual_error_formatter.to_string ~claim_color

let to_highlighted_string (lint : string Pos.pos t) =
  User_error.make_absolute lint.code [(lint.pos, lint.message)]
  |> Highlighted_error_formatter.to_string

let to_json { pos; code; severity; message; bypass_changed_lines; autofix } =
  let (line, scol, ecol) = Pos.info_pos pos in
  let (origin, repl, start, w) =
    match autofix with
    | Some (original, replacement, start_offset, width) ->
      ( Hh_json.JSON_String original,
        Hh_json.JSON_String replacement,
        Hh_json.int_ start_offset,
        Hh_json.int_ width )
    | None ->
      ( Hh_json.JSON_String "",
        Hh_json.JSON_String "",
        Hh_json.JSON_Null,
        Hh_json.JSON_Null )
  in
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
      ("original", origin);
      ("replacement", repl);
      ("start_offset", start);
      ("width", w);
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
