(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)
open Hh_prelude

(** These severity levels are based on those provided by Arcanist. "Advice"
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
  bypass_changed_lines: bool;
      (** Normally, lint warnings and lint advice only get shown by arcanist if the
       * lines they are raised on overlap with lines changed in a diff. This
       * flag bypasses that behavior *)
  autofix: (string * Pos.t) option;
  check_status: Tast.check_status option;
}
[@@deriving show]

let (lint_list : Pos.t t list option ref) = ref None

let get_code { code; _ } = code

let get_pos { pos; _ } = pos

let add
    ?(check_status = None)
    ?(bypass_changed_lines = false)
    ?(autofix = None)
    code
    severity
    pos
    message =
  match !lint_list with
  | Some lst ->
    let lint =
      {
        code;
        severity;
        pos;
        message;
        bypass_changed_lines;
        autofix;
        check_status;
      }
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

let to_json { pos; code; severity; message; bypass_changed_lines; autofix; _ } =
  let (line, scol, ecol) = Pos.info_pos pos in
  let (origin, replacement, start, w) =
    match autofix with
    | Some (replacement, replacement_pos) ->
      let path = Pos.filename (Pos.to_absolute replacement_pos) in
      let lines = Errors.read_lines path in
      let src = String.concat ~sep:"\n" lines in
      let original = Pos.get_text_from_pos ~content:src replacement_pos in
      let (start_offset, end_offset) = Pos.info_raw replacement_pos in
      let width = end_offset - start_offset in
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
      ("replacement", replacement);
      ("start_offset", start);
      ("width", w);
    ]

(* If the check_status field is available in a lint, we expect that the
   lint is a true positive only if the same lint was produced under both
   dynamic and normal assumptions. This helper function filters out the
   remaining ones. *)
let filter_out_unsound_lints lints =
  let module LintMap = WrappedMap.Make (struct
    type t = int * Pos.t [@@deriving ord]
  end) in
  let module CheckStatusParity = struct
    type t = {
      under_normal_assumptions: bool;
      under_dynamic_assumptions: bool;
    }

    let default =
      { under_normal_assumptions = false; under_dynamic_assumptions = false }

    let set_normal parity_opt =
      let set parity = Some { parity with under_normal_assumptions = true } in
      Option.value parity_opt ~default |> set

    let set_dynamic parity_opt =
      let set parity = Some { parity with under_dynamic_assumptions = true } in
      Option.value parity_opt ~default |> set

    let is_paired parity =
      parity.under_dynamic_assumptions && parity.under_normal_assumptions
  end in
  let lint_parity =
    List.fold
      ~f:(fun m lint ->
        let update = LintMap.update (lint.code, lint.pos) in
        match lint.check_status with
        | Some Tast.CUnderNormalAssumptions ->
          update CheckStatusParity.set_normal m
        | Some Tast.CUnderDynamicAssumptions ->
          update CheckStatusParity.set_dynamic m
        | _ -> m)
      ~init:LintMap.empty
      lints
  in
  let should_keep_lint = function
    | { check_status = Some Tast.CUnderDynamicAssumptions; _ } ->
      (* There are three cases to consider:
           1. we are in any unsound lint rule: this cannot be the case as
              `check_status` is set to non-None.
           2. we are in a sound lint rule where the lint is only produced under
              dynamic assumptions: the result is not reliable, so it should be
              eliminated.
           3. we are in a sound lint rule where the lint is produced under both
              dynamic and normal assumptions: the lint is reliable, but it is
              duplicated, so we arbitrarily eliminate the one produced under
              dynamic assumptions and keep the one under normal assumptions. *)
      false
    | { code; pos; _ } -> begin
      match LintMap.find_opt (code, pos) lint_parity with
      | Some parity -> CheckStatusParity.is_paired parity
      | None ->
        (* Either it was checked once and no parity is needed or is a lint that
           doesn't use sound linting. *)
        true
    end
  in
  List.filter ~f:should_keep_lint lints

let do_ f =
  let list_copy = !lint_list in
  lint_list := Some [];
  let result = f () in
  let out =
    match !lint_list with
    | Some lst -> lst
    | None -> assert false
  in
  let out = filter_out_unsound_lints out in
  lint_list := list_copy;
  (List.rev out, result)
