(*
 * Copyright (c) 2016, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

module SyntaxError = Full_fidelity_syntax_error
open Core_kernel
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
  lower_coroutines: bool;
  fail_open: bool;
  parser_options: ParserOptions.t;
  fi_mode: FileInfo.mode;
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
    ?(lower_coroutines = true)
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
    lower_coroutines;
    parser_options;
    fi_mode = FileInfo.Mpartial;
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
include Full_fidelity_ast_types
module SourceText = Full_fidelity_source_text
module DeclModeSC_ = DeclModeSmartConstructors.WithSyntax (PositionedSyntax)

module DeclModeSC = DeclModeSC_.WithRustParser (struct
  type r = PositionedSyntax.t

  type t = bool list

  let rust_parse = Rust_parser_ffi.parse_positioned_with_decl_mode_sc
end)

module DeclModeParser_ = Full_fidelity_parser.WithSyntax (PositionedSyntax)
module DeclModeParser = DeclModeParser_.WithSmartConstructors (DeclModeSC)

(* Creates a relative position out of the error and the given path and source text. *)
let pos_of_error path source_text error =
  SourceText.relative_pos
    path
    source_text
    (SyntaxError.start_offset error)
    (SyntaxError.end_offset error)

let parse_text (env : env) (source_text : SourceText.t) :
    FileInfo.mode option * PositionedSyntaxTree.t =
  let mode = Full_fidelity_parser.parse_mode source_text in
  let quick_mode =
    (not env.codegen)
    &&
    match mode with
    | None
    | Some FileInfo.Mdecl
    | Some FileInfo.Mphp ->
      true
    | _ -> env.quick_mode
  in
  (* DANGER: Needs to be kept in sync with other logic in this file, ensuring
       that the tree created here is later passed to ParserErrors. This can
       currently leak memory when an exception is thrown between parsing and
       error checking
     *)
  let leak_rust_tree = true in
  let tree =
    let env' =
      Full_fidelity_parser_env.make
        ~hhvm_compat_mode:env.codegen
        ~codegen:env.codegen
        ~php5_compat_mode:env.php5_compat_mode
        ~disable_nontoplevel_declarations:
          (GlobalOptions.po_disable_nontoplevel_declarations env.parser_options)
        ~leak_rust_tree
        ~disable_legacy_soft_typehints:
          (GlobalOptions.po_disable_legacy_soft_typehints env.parser_options)
        ~allow_new_attribute_syntax:
          (GlobalOptions.po_allow_new_attribute_syntax env.parser_options)
        ~disable_legacy_attribute_syntax:
          (GlobalOptions.po_disable_legacy_attribute_syntax env.parser_options)
        ?mode
        ~enable_xhp_class_modifier:
          (GlobalOptions.po_enable_xhp_class_modifier env.parser_options)
        ()
    in
    if quick_mode then
      let parser = DeclModeParser.make env' source_text in
      let (parser, root, rust_tree) = DeclModeParser.parse_script parser in
      let errors = DeclModeParser.errors parser in
      PositionedSyntaxTree.create source_text root rust_tree errors mode false
    else
      PositionedSyntaxTree.make ~env:env' source_text
  in
  (mode, tree)

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
  List.iter ~f:Errors.add_error_with_check errors

let process_lint_errors (_ : env) (errors : Relative_path.t Lint.t list) =
  List.iter ~f:Lint.add_lint errors

let elaborate_top_level_defs env aast =
  if
    (not (ParserOptions.rust_top_level_elaborator env.parser_options))
    && env.elaborate_namespaces
  then
    Namespaces.elaborate_toplevel_defs env.parser_options aast
  else
    aast

external rust_from_text_ffi :
  Rust_aast_parser_types.env ->
  SourceText.t ->
  (Rust_aast_parser_types.result, Rust_aast_parser_types.error) result
  = "from_text"

let rewrite_coroutines source_text script =
  Full_fidelity_editable_positioned_syntax.from_positioned_syntax script
  |> Ppl_class_rewriter.rewrite_ppl_classes
  |> Coroutine_lowerer.lower_coroutines
  |> Full_fidelity_editable_positioned_syntax.text
  |> SourceText.make (SourceText.file_path source_text)

(*
rewrite_coroutines_for_rust is invoked from Rust. It takes `source_text` and `script` and
returns original source and rewritten source.

Why does it need to take `source_text`? It only requires `file_path`, why not just pass file_path
from Rust side?

`source_text` contains raw source string, the string won't be copied when Rust receives it for the sake of perf.
This assumes Ocaml GC won't collect it during Rust call. `rewrite_coroutines_for_rust` will be called from Rust during
a function call to Rust. Ocaml GC may collect/promote it. Passing it to Ocaml and return it(possiblely not smae ptr)
back to Rust can avoid segfault.
*)
let rewrite_coroutines_for_rust source_text script =
  (source_text, rewrite_coroutines source_text script)

let () =
  Callback.register "rewrite_coroutines_for_rust" rewrite_coroutines_for_rust

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
        lower_coroutines = env.lower_coroutines;
        fail_open = env.fail_open;
        parser_options = env.parser_options;
      }
  in
  match rust_from_text_ffi rust_env source_text with
  | Ok r -> r
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
        ast = elaborate_top_level_defs env aast;
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

let aast_to_tast aast =
  let tany = Typing_defs.mk (Typing_reason.Rnone, Typing_defs.make_tany ()) in
  let endo =
    object
      inherit [_] Aast.map

      method on_'ex _ pos = (pos, tany)

      method on_'fb _ _ = Tast.HasUnsafeBlocks

      method on_'en _ _ = Tast.dummy_saved_env

      method on_'hi _ _ = tany
    end
  in
  endo#on_program () aast

let tast_to_aast tast =
  let endo =
    object
      inherit [_] Aast.map

      method on_'ex _ (pos, _) = pos

      method on_'fb _ _ = ()

      method on_'en _ _ = ()

      method on_'hi _ _ = ()
    end
  in
  endo#on_program () tast

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

(**
 * Converts a legacy ast (ast.ml) into a typed ast (tast.ml / aast.ml)
 * so that codegen and typing both operate on the same ast structure.
 *
 * There are some errors that are not valid hack but are still expected
 * to produce valid bytecode. hh_single_compile is expected to catch
 * these errors.
 *)
let from_text_to_empty_tast (env : env) (source_text : SourceText.t) :
    Rust_aast_parser_types.tast_result =
  let result = from_text_rust env source_text in
  Rust_aast_parser_types.
    {
      result with
      aast =
        Result.map
          ~f:(fun x -> aast_to_tast (elaborate_top_level_defs env x))
          result.aast;
    }

(*****************************************************************************(
 * Backward compatibility matter (should be short-lived)
)*****************************************************************************)

let legacy (x : aast_result) : Parser_return.t =
  {
    Parser_return.file_mode =
      Option.some_if (x.fi_mode <> FileInfo.Mphp) x.fi_mode;
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

let defensive_from_file_with_default_popt ?quick ?show_all_errors fn =
  defensive_from_file ?quick ?show_all_errors ParserOptions.default fn

let defensive_program_with_default_popt
    ?quick ?show_all_errors ?fail_open ?elaborate_namespaces fn content =
  defensive_program
    ?quick
    ?show_all_errors
    ?fail_open
    ?elaborate_namespaces
    ParserOptions.default
    fn
    content
