(*
 * Copyright (c) 2016, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

module SyntaxError = Full_fidelity_syntax_error
module Lint = Lints_core
open Hh_prelude
open Scoured_comments

(* Context of the file being parsed, as (hopefully some day read-only) state. *)
type env = {
  codegen: bool;
  php5_compat_mode: bool;
  elaborate_namespaces: bool;
  include_line_comments: bool;
  quick_mode: bool;
  (* Show errors even in quick mode.
   * Hotfix until we can properly set up saved states to surface parse errors during
   * typechecking properly. *)
  show_all_errors: bool;
  parser_options: ParserOptions.t;
  file: Relative_path.t;
  is_systemlib: bool;
}
[@@deriving show]

let make_env
    ?(codegen = false)
    ?(php5_compat_mode = false)
    ?(elaborate_namespaces = true)
    ?(include_line_comments = false)
    ?(quick_mode = false)
    ?(show_all_errors = false)
    ?(parser_options = ParserOptions.default)
    ?(is_systemlib = false)
    (file : Relative_path.t) : env =
  let parser_options = ParserOptions.with_codegen parser_options codegen in
  {
    codegen;
    php5_compat_mode;
    elaborate_namespaces;
    include_line_comments;
    quick_mode = (not codegen) && quick_mode;
    show_all_errors;
    parser_options;
    file;
    is_systemlib;
  }

let should_surface_errors env =
  (* env.show_all_errors is a hotfix until we can retool how saved states handle
   * parse errors. *)
  (not env.quick_mode) || env.show_all_errors

type aast_result = {
  fi_mode: FileInfo.mode;
  ast: (unit, unit) Aast.program;
  content: string;
  comments: Scoured_comments.t;
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
  List.iter sc.sc_error_pos ~f:(fun pos ->
      Errors.add_error Parsing_error.(to_user_error @@ Fixme_format pos));
  List.iter sc.sc_bad_ignore_pos ~f:(fun pos ->
      Errors.add_error Parsing_error.(to_user_error @@ Hh_ignore_comment pos));
  Fixme_provider.provide_disallowed_fixmes env.file sc.sc_misuses;
  if env.quick_mode then
    Fixme_provider.provide_decl_hh_fixmes env.file sc.sc_fixmes
  else
    Fixme_provider.provide_hh_fixmes env.file sc.sc_fixmes

let process_lowerer_parsing_errors
    (env : env) (lowerer_parsing_errors : (Pos.t * string) list) =
  if should_surface_errors env then
    List.iter
      ~f:(fun (pos, msg) ->
        Errors.add_error
          Parsing_error.(
            to_user_error @@ Parsing_error { pos; msg; quickfixes = [] }))
      lowerer_parsing_errors

let process_non_syntax_errors (_ : env) (errors : Errors.error list) =
  List.iter ~f:Errors.add_error errors

let process_lint_errors (_ : env) (errors : Pos.t Lint.t list) =
  List.iter ~f:Lint.add_lint errors

external rust_from_text_ffi :
  Rust_aast_parser_types.env ->
  SourceText.t ->
  (Rust_aast_parser_types.result, Rust_aast_parser_types.error) result
  = "from_text"

(** Note: this doesn't respect deregister_php_stdlib *)
external parse_ast_and_decls_ffi :
  Rust_aast_parser_types.env ->
  SourceText.t ->
  (Rust_aast_parser_types.result, Rust_aast_parser_types.error) result
  * Direct_decl_parser.parsed_file_with_hashes = "hh_parse_ast_and_decls_ffi"

let process_syntax_errors
    (env : env)
    (source_text : SourceText.t)
    (errors : Full_fidelity_syntax_error.t list) =
  let relative_pos = pos_of_error env.file source_text in

  let pos_of_offsets start_offset end_offset =
    SourceText.relative_pos env.file source_text start_offset end_offset
  in

  let report_error e =
    let quickfixes =
      List.map e.Full_fidelity_syntax_error.quickfixes ~f:(fun qf ->
          let { Full_fidelity_syntax_error.title; edits } = qf in
          let edits =
            Quickfix.Eager
              (List.map edits ~f:(fun (start_offset, end_offset, new_text) ->
                   (new_text, pos_of_offsets start_offset end_offset)))
          in
          Quickfix.make ~title ~edits)
    in

    Errors.add_error
      Parsing_error.(
        to_user_error
        @@ Parsing_error
             { pos = relative_pos e; msg = SyntaxError.message e; quickfixes })
  in
  List.iter ~f:report_error errors

let make_rust_env (env : env) : Rust_aast_parser_types.env =
  Rust_aast_parser_types.
    {
      codegen = env.codegen;
      elaborate_namespaces = env.elaborate_namespaces;
      php5_compat_mode = env.php5_compat_mode;
      include_line_comments = env.include_line_comments;
      quick_mode = env.quick_mode;
      show_all_errors = env.show_all_errors;
      is_systemlib = env.is_systemlib;
      for_debugger_eval = false;
      parser_options = env.parser_options;
      scour_comments = true;
    }

let unwrap_rust_parser_result
    st
    (rust_result :
      (Rust_aast_parser_types.result, Rust_aast_parser_types.error) result) :
    Rust_aast_parser_types.result =
  match rust_result with
  | Ok r -> r
  | Error Rust_aast_parser_types.NotAHackFile ->
    failwith
      ("Not a Hack file: " ^ Relative_path.to_absolute (SourceText.file_path st))
  | Error (Rust_aast_parser_types.ParserFatal (e, p)) ->
    raise @@ SyntaxError.ParserFatal (e, p)
  | Error (Rust_aast_parser_types.Other msg) -> failwith msg

let from_text_rust (env : env) (source_text : SourceText.t) :
    Rust_aast_parser_types.result =
  let rust_env = make_rust_env env in
  unwrap_rust_parser_result
    source_text
    (rust_from_text_ffi rust_env source_text)

(** note: this doesn't respect deregister_php_stdlib *)
let ast_and_decls_from_text_rust (env : env) (source_text : SourceText.t) :
    Rust_aast_parser_types.result * Direct_decl_parser.parsed_file_with_hashes =
  let rust_env = make_rust_env env in
  let (ast_result, decls) = parse_ast_and_decls_ffi rust_env source_text in
  let ast_result = unwrap_rust_parser_result source_text ast_result in
  (ast_result, decls)

let process_lowerer_result
    (env : env) (source_text : SourceText.t) (r : Rust_aast_parser_types.result)
    : aast_result =
  Rust_aast_parser_types.(
    process_scour_comments env r.scoured_comments;
    process_syntax_errors env source_text r.syntax_errors;
    process_lowerer_parsing_errors env r.lowerer_parsing_errors;
    process_non_syntax_errors env r.errors;
    process_lint_errors env r.lint_errors;
    {
      fi_mode = r.file_mode;
      ast = r.aast;
      content =
        (if env.codegen then
          ""
        else
          SourceText.text source_text);
      comments = r.scoured_comments;
    })

let from_text (env : env) (source_text : SourceText.t) : aast_result =
  process_lowerer_result env source_text (from_text_rust env source_text)

(** note: this doesn't respect deregister_php_stdlib *)
let ast_and_decls_from_text (env : env) (source_text : SourceText.t) :
    aast_result * Direct_decl_parser.parsed_file_with_hashes =
  let (ast_result, decls) = ast_and_decls_from_text_rust env source_text in
  let ast_result = process_lowerer_result env source_text ast_result in
  (ast_result, decls)

let from_file (env : env) : aast_result =
  let source_text = SourceText.from_file env.file in
  from_text env source_text

(*****************************************************************************(
 * Backward compatibility matter (should be short-lived)
)*****************************************************************************)

let legacy (x : aast_result) : Parser_return.t =
  {
    Parser_return.file_mode = Some x.fi_mode;
    Parser_return.comments = x.comments.sc_comments;
    Parser_return.ast = x.ast;
    Parser_return.content = x.content;
  }

let from_source_text_with_legacy
    (env : env) (source_text : Full_fidelity_source_text.t) : Parser_return.t =
  legacy @@ from_text env source_text

(** note: this doesn't respect deregister_php_stdlib *)
let ast_and_decls_from_source_text_with_legacy
    (env : env) (source_text : Full_fidelity_source_text.t) :
    Parser_return.t * Direct_decl_parser.parsed_file_with_hashes =
  let (ast_result, decls) = ast_and_decls_from_text env source_text in
  (legacy ast_result, decls)

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
    ?(elaborate_namespaces = true)
    ?(include_line_comments = false)
    parser_options
    fn
    content =
  try
    let source = Full_fidelity_source_text.make fn content in
    let env =
      make_env
        ~quick_mode:quick
        ~show_all_errors
        ~elaborate_namespaces
        ~parser_options
        ~include_line_comments
        fn
    in
    legacy @@ from_text env source
  with
  | e ->
    Rust_pointer.free_leaked_pointer ();

    (* If we fail to lower, try to just make a source text and get the file mode *)
    (* If even THAT fails, we just have to give up and return an empty php file*)
    let mode =
      try
        let source = Full_fidelity_source_text.make fn content in
        Full_fidelity_parser.parse_mode source
      with
      | _ -> None
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
    try Sys_utils.cat (Relative_path.to_absolute fn) with
    | _ -> ""
  in
  defensive_program ?quick ?show_all_errors popt fn content

(** note: this doesn't respect deregister_php_stdlib *)
let ast_and_decls_from_file
    ?(quick = false) ?(show_all_errors = false) parser_options fn =
  let content =
    try Sys_utils.cat (Relative_path.to_absolute fn) with
    | _ -> ""
  in
  try
    let source = Full_fidelity_source_text.make fn content in
    let env =
      make_env
        ~quick_mode:quick
        ~show_all_errors
        ~elaborate_namespaces:true
        ~parser_options
        ~include_line_comments:false
        fn
    in
    ast_and_decls_from_source_text_with_legacy env source
  with
  | e ->
    (* If we fail to lower, try to just make a source text and get the file mode *)
    (* If even THAT fails, we just have to give up and return an empty php file*)
    let mode =
      try
        let source = Full_fidelity_source_text.make fn content in
        Full_fidelity_parser.parse_mode source
      with
      | _ -> None
    in
    let err = Exn.to_string e in
    let fn = Relative_path.suffix fn in
    (* If we've already found a parsing error, it's okay for lowering to fail *)
    if not (Errors.currently_has_errors ()) then
      Hh_logger.log "Warning, lowering failed for %s\n  - error: %s\n" fn err;

    ( {
        Parser_return.file_mode = mode;
        Parser_return.comments = [];
        Parser_return.ast = [];
        Parser_return.content;
      },
      Direct_decl_parser.
        { pfh_mode = mode; pfh_hash = Int64.zero; pfh_decls = [] } )
