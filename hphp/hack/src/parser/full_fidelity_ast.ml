(*
 * Copyright (c) 2016, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

module SyntaxError = Full_fidelity_syntax_error
open Hh_prelude
open Scoured_comments

(* Context of the file being parsed, as (hopefully some day read-only) state. *)
type env = {
  codegen: bool;
  php5_compat_mode: bool;
  elaborate_namespaces: bool;
  include_line_comments: bool;
  keep_errors: bool;
  quick_mode: bool;
  (* Show errors even in quick mode. Does not override keep_errors. Hotfix
   * until we can properly set up saved states to surface parse errors during
   * typechecking properly. *)
  show_all_errors: bool;
  fail_open: bool;
  parser_options: ParserOptions.t;
  file: Relative_path.t;
  disable_global_state_mutation: bool;
}
[@@deriving show]

let make_env
    ?(codegen = false)
    ?(php5_compat_mode = false)
    ?(elaborate_namespaces = true)
    ?(include_line_comments = false)
    ?(keep_errors = true)
    ?(quick_mode = false)
    ?(show_all_errors = false)
    ?(fail_open = true)
    ?(parser_options = ParserOptions.default)
    ?(disable_global_state_mutation = false)
    (file : Relative_path.t) : env =
  let parser_options = ParserOptions.with_codegen parser_options codegen in
  {
    codegen;
    php5_compat_mode;
    elaborate_namespaces;
    include_line_comments;
    keep_errors;
    quick_mode = (not codegen) && quick_mode;
    show_all_errors;
    parser_options;
    fail_open;
    file;
    disable_global_state_mutation;
  }

let should_surface_errors env =
  (* env.show_all_errors is a hotfix until we can retool how saved states handle
   * parse errors. *)
  ((not env.quick_mode) || env.show_all_errors) && env.keep_errors

type aast_result = {
  fi_mode: FileInfo.mode;
  ast: (Pos.t, unit, unit, unit) Aast.program;
  content: string;
  file: Relative_path.t;
  comments: Scoured_comments.t;
  (*
    lowpri_errors_ are exposed to test, it shouldn't be used in prod
  *)
  lowpri_errors_: (Pos.t * string) list ref;
}

(* TODO: Make these not default to positioned_syntax *)
module SourceText = Full_fidelity_source_text

(* Creates a relative position out of the error and the given path and source text. *)
let pos_of_error path source_text error =
  SourceText.relative_pos
    path
    source_text
    (SyntaxError.start_offset error)
    (SyntaxError.end_offset error)

let process_scour_comments (env : env) (sc : Scoured_comments.t) =
  List.iter sc.sc_error_pos ~f:Errors.fixme_format;
  if (not env.disable_global_state_mutation) && env.keep_errors then (
    Fixme_provider.provide_disallowed_fixmes env.file sc.sc_misuses;
    if env.quick_mode then
      Fixme_provider.provide_decl_hh_fixmes env.file sc.sc_fixmes
    else
      Fixme_provider.provide_hh_fixmes env.file sc.sc_fixmes
  )

let process_lowpri_errors (env : env) (lowpri_errors : (Pos.t * string) list) =
  if should_surface_errors env then
    List.iter ~f:Errors.parsing_error lowpri_errors

let process_non_syntax_errors (_ : env) (errors : Errors.error list) =
  List.iter ~f:Errors.add_error errors

let process_lint_errors (_ : env) (errors : Pos.t Lint.t list) =
  List.iter ~f:Lint.add_lint errors

external rust_from_text_ffi :
  Rust_aast_parser_types.env ->
  SourceText.t ->
  (Rust_aast_parser_types.result, Rust_aast_parser_types.error) result
  = "from_text"

let process_syntax_errors
    (env : env)
    (source_text : SourceText.t)
    (errors : Full_fidelity_syntax_error.t list) =
  let relative_pos = pos_of_error env.file source_text in
  let report_error e =
    Errors.parsing_error (relative_pos e, SyntaxError.message e)
  in
  List.iter ~f:report_error errors

let from_text_rust (env : env) (source_text : SourceText.t) :
    Rust_aast_parser_types.result =
  let rust_env =
    Rust_aast_parser_types.
      {
        codegen = env.codegen;
        elaborate_namespaces = env.elaborate_namespaces;
        php5_compat_mode = env.php5_compat_mode;
        include_line_comments = env.include_line_comments;
        keep_errors = env.keep_errors;
        quick_mode = env.quick_mode;
        show_all_errors = env.show_all_errors;
        fail_open = env.fail_open;
        parser_options = env.parser_options;
      }
  in
  match rust_from_text_ffi rust_env source_text with
  | Ok r -> r
  | Error Rust_aast_parser_types.NotAHackFile -> failwith "Not a Hack file"
  | Error (Rust_aast_parser_types.ParserFatal (e, p)) ->
    raise @@ SyntaxError.ParserFatal (e, p)
  | Error (Rust_aast_parser_types.Other msg) -> failwith msg

let process_lowerer_result
    (env : env) (source_text : SourceText.t) (r : Rust_aast_parser_types.result)
    : aast_result =
  Rust_aast_parser_types.(
    process_scour_comments env r.scoured_comments;
    process_syntax_errors env source_text r.syntax_errors;
    process_lowpri_errors env r.lowpri_errors;
    process_non_syntax_errors env r.errors;
    process_lint_errors env r.lint_errors;
    match r.aast with
    | Error msg -> failwith msg
    | Ok aast ->
      {
        fi_mode = r.file_mode;
        ast = aast;
        content =
          ( if env.codegen then
            ""
          else
            SourceText.text source_text );
        comments = r.scoured_comments;
        file = env.file;
        lowpri_errors_ = ref r.lowpri_errors;
      })

let from_text (env : env) (source_text : SourceText.t) : aast_result =
  process_lowerer_result env source_text (from_text_rust env source_text)

let from_file (env : env) : aast_result =
  let source_text = SourceText.from_file env.file in
  from_text env source_text

type aast_to_nast_env = { mutable last_pos: Pos.t }

let aast_to_nast aast =
  let i _ x = x in
  let endo =
    object (self)
      inherit [_] Aast.map

      method check_pos env pos =
        if Pos.equal pos Pos.none then
          Pos.none
        else if Pos.equal pos env.last_pos then
          env.last_pos
        else (
          env.last_pos <- pos;
          pos
        )

      method! on_pos = self#check_pos

      method on_'fb _ _ = Nast.NamedWithUnsafeBlocks

      method on_'ex = self#check_pos

      method on_'hi = i

      method on_'en = i
    end
  in
  endo#on_program { last_pos = Pos.none } aast

(*****************************************************************************(
 * Backward compatibility matter (should be short-lived)
)*****************************************************************************)

let legacy (x : aast_result) : Parser_return.t =
  {
    Parser_return.file_mode = Some x.fi_mode;
    Parser_return.comments = x.comments.sc_comments;
    Parser_return.ast = aast_to_nast x.ast;
    Parser_return.content = x.content;
  }

let from_source_text_with_legacy
    (env : env) (source_text : Full_fidelity_source_text.t) : Parser_return.t =
  legacy @@ from_text env source_text

let from_text_with_legacy (env : env) (content : string) : Parser_return.t =
  let source_text = SourceText.make env.file content in
  from_source_text_with_legacy env source_text

let from_file_with_legacy env = legacy (from_file env)

(******************************************************************************(
 * For cut-over purposes only; this should be removed as soon as Parser_hack
 * is removed.
)******************************************************************************)

let defensive_program
    ?(quick = false)
    ?(show_all_errors = false)
    ?(fail_open = false)
    ?(keep_errors = false)
    ?(elaborate_namespaces = true)
    ?(include_line_comments = false)
    parser_options
    fn
    content =
  try
    let source = Full_fidelity_source_text.make fn content in
    (* If we fail open, we don't want errors. *)
    let env =
      make_env
        ~fail_open
        ~quick_mode:quick
        ~show_all_errors
        ~elaborate_namespaces
        ~keep_errors:(keep_errors || not fail_open)
        ~parser_options
        ~include_line_comments
        fn
    in
    legacy @@ from_text env source
  with e ->
    Rust_pointer.free_leaked_pointer ();

    (* If we fail to lower, try to just make a source text and get the file mode *)
    (* If even THAT fails, we just have to give up and return an empty php file*)
    let mode =
      try
        let source = Full_fidelity_source_text.make fn content in
        Full_fidelity_parser.parse_mode source
      with _ -> None
    in
    let err = Exn.to_string e in
    let fn = Relative_path.suffix fn in
    (* If we've already found a parsing error, it's okay for lowering to fail *)
    if not (Errors.currently_has_errors ()) then
      Hh_logger.log "Warning, lowering failed for %s\n  - error: %s\n" fn err;

    {
      Parser_return.file_mode = mode;
      Parser_return.comments = [];
      Parser_return.ast = [];
      Parser_return.content;
    }

let defensive_from_file ?quick ?show_all_errors popt fn =
  let content =
    (try Sys_utils.cat (Relative_path.to_absolute fn) with _ -> "")
  in
  defensive_program ?quick ?show_all_errors popt fn content
