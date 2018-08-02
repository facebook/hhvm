(**
 * Copyright (c) 2016, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

 (**
  *
  * Usage: full_fidelity_parse [OPTIONS] [FILES]
  *
  * --full-fidelity-json
  * --full-fidelity-errors
  * --full-fidelity-errors-all
  * --full-fidelity-s-expression
  * --full-fidelity-ast-s-expression
  * --original-parser-errors
  * --original-parser-s-expression
  * --program-text
  * --pretty-print
  * --show-file-name
  *
  * TODO: Parser for things other than scripts:
  *       types, expressions, statements, declarations, etc.
  *)

module Schema = Full_fidelity_schema
module SyntaxError = Full_fidelity_syntax_error
module SyntaxTree = Full_fidelity_syntax_tree
  .WithSyntax(Full_fidelity_positioned_syntax)
module SourceText = Full_fidelity_source_text
module ParserErrors = Full_fidelity_parser_errors
  .WithSyntax(Full_fidelity_positioned_syntax)
module DebugPos = Debug.WithSyntax(Full_fidelity_positioned_syntax)

module FullFidelityParseArgs = struct

  type t =
  {
    (* Output options *)
    full_fidelity_json : bool;
    full_fidelity_text_json : bool;
    full_fidelity_dot : bool;
    full_fidelity_dot_edges : bool;
    full_fidelity_errors : bool;
    full_fidelity_errors_all : bool;
    full_fidelity_s_expr : bool;
    full_fidelity_ast_s_expr : bool;
    original_parser_errors : bool;
    original_parser_s_expr : bool;
    program_text : bool;
    pretty_print : bool;
    schema: bool;
    show_file_name : bool;
    (* Configuring the parser *)
    is_hh_file : bool;
    codegen : bool;
    php5_compat_mode : bool;
    elaborate_namespaces : bool;
    include_line_comments : bool;
    keep_errors : bool;
    quick_mode : bool;
    lower_coroutines : bool;
    enable_hh_syntax : bool;
    fail_open : bool;
    force_hh_syntax : bool;
    (* Defining the input *)
    files : string list;
  }

  let make
    full_fidelity_json
    full_fidelity_text_json
    full_fidelity_dot
    full_fidelity_dot_edges
    full_fidelity_errors
    full_fidelity_errors_all
    full_fidelity_s_expr
    full_fidelity_ast_s_expr
    original_parser_errors
    original_parser_s_expr
    program_text
    pretty_print
    schema
    is_hh_file
    codegen
    php5_compat_mode
    elaborate_namespaces
    include_line_comments
    keep_errors
    quick_mode
    lower_coroutines
    enable_hh_syntax
    fail_open
    force_hh_syntax
    show_file_name
    files = {
    full_fidelity_json;
    full_fidelity_dot;
    full_fidelity_dot_edges;
    full_fidelity_text_json;
    full_fidelity_errors;
    full_fidelity_errors_all;
    full_fidelity_s_expr;
    full_fidelity_ast_s_expr;
    original_parser_errors;
    original_parser_s_expr;
    program_text;
    pretty_print;
    schema;
    is_hh_file;
    codegen;
    php5_compat_mode;
    elaborate_namespaces;
    include_line_comments;
    keep_errors;
    quick_mode;
    lower_coroutines;
    enable_hh_syntax;
    fail_open;
    force_hh_syntax;
    show_file_name;
    files }

  let parse_args () =
    let usage = Printf.sprintf "Usage: %s [OPTIONS] filename\n" Sys.argv.(0) in
    let full_fidelity_json = ref false in
    let set_full_fidelity_json () = full_fidelity_json := true in
    let full_fidelity_text_json = ref false in
    let set_full_fidelity_text_json () = full_fidelity_text_json := true in
    let full_fidelity_dot = ref false in
    let set_full_fidelity_dot () = full_fidelity_dot := true in
    let full_fidelity_dot_edges = ref false in
    let set_full_fidelity_dot_edges () = full_fidelity_dot_edges := true in
    let full_fidelity_errors = ref false in
    let set_full_fidelity_errors () = full_fidelity_errors := true in
    let full_fidelity_errors_all = ref false in
    let set_full_fidelity_errors_all () =
      full_fidelity_errors_all := true in
    let full_fidelity_s_expr = ref false in
    let full_fidelity_ast_s_expr = ref false in
    let set_full_fidelity_s_expr () = full_fidelity_s_expr := true in
    let set_full_fidelity_ast_s_expr () = full_fidelity_ast_s_expr := true in
    let original_parser_errors = ref false in
    let set_original_parser_errors () = original_parser_errors := true in
    let original_parser_s_expr = ref false in
    let set_original_parser_s_expr () = original_parser_s_expr := true in
    let program_text = ref false in
    let set_program_text () = program_text := true in
    let pretty_print = ref false in
    let set_pretty_print () = pretty_print := true in
    let schema = ref false in
    let set_schema () = schema := true in
    let is_hh_file = ref false in
    let codegen = ref false in
    let php5_compat_mode = ref false in
    let elaborate_namespaces = ref true in
    let include_line_comments = ref false in
    let keep_errors = ref true in
    let quick_mode = ref false in
    let lower_coroutines = ref true in
    let enable_hh_syntax = ref false in
    let fail_open = ref true in
    let force_hh_syntax = ref false in
    let show_file_name = ref false in
    let set_show_file_name () = show_file_name := true in
    let files = ref [] in
    let push_file file = files := file :: !files in
    let options =  [
      (* modes *)
      "--full-fidelity-json",
        Arg.Unit set_full_fidelity_json,
        "Displays the full-fidelity parse tree in JSON format.";
      "--full-fidelity-text-json",
        Arg.Unit set_full_fidelity_text_json,
        "Displays the full-fidelity parse tree in JSON format with token text.";
      "--full-fidelity-dot",
        Arg.Unit set_full_fidelity_dot,
        "Displays the full-fidelity parse tree in GraphViz DOT format.";
      "--full-fidelity-dot-edges",
        Arg.Unit set_full_fidelity_dot_edges,
        "Displays the full-fidelity parse tree in GraphViz DOT format with edge labels.";
      "--full-fidelity-errors",
        Arg.Unit set_full_fidelity_errors,
        "Displays the full-fidelity parser errors, if any.
Some errors may be filtered out.";
      "--full-fidelity-errors-all",
        Arg.Unit set_full_fidelity_errors_all,
        "Displays the full-fidelity parser errors, if any.
No errors are filtered out.";
      "--full-fidelity-s-expression",
        Arg.Unit set_full_fidelity_s_expr,
        "Displays the full-fidelity parse tree in S-expression format.";
      "--full-fidelity-ast-s-expression",
        Arg.Unit set_full_fidelity_ast_s_expr,
        "Displays the AST produced by the FFP in S-expression format.";
      "--original-parser-errors",
        Arg.Unit set_original_parser_errors,
        "Displays the original parse tree errors, if any.";
      "--original-parser-s-expression",
        Arg.Unit set_original_parser_s_expr,
        "Displays the original parse tree in S-expression format.";
      "--program-text",
        Arg.Unit set_program_text,
        "Displays the text of the given file.";
      "--pretty-print",
        Arg.Unit set_pretty_print,
        "Displays the text of the given file after pretty-printing.";
      "--schema",
        Arg.Unit set_schema,
        "Displays the parser version and schema of nodes.";
      "--is-hh-file",
        Arg.Set is_hh_file,
        "Set the is_hh_file option for the parser.";
      "--no-is-hh-file",
        Arg.Clear is_hh_file,
        "Unset the is_hh_file option for the parser.";
      "--codegen",
        Arg.Set codegen,
        "Set the codegen option for the parser.";
      "--no-codegen",
        Arg.Clear codegen,
        "Unset the codegen option for the parser.";
      "--php5-compat-mode",
        Arg.Set php5_compat_mode,
        "Set the php5_compat_mode option for the parser.";
      "--no-php5-compat-mode",
        Arg.Clear php5_compat_mode,
        "Unset the php5_compat_mode option for the parser.";
      "--elaborate-namespaces",
        Arg.Set elaborate_namespaces,
        "Set the elaborate_namespaces option for the parser.";
      "--no-elaborate-namespaces",
        Arg.Clear elaborate_namespaces,
        "Unset the elaborate_namespaces option for the parser.";
      "--include-line-comments",
        Arg.Set include_line_comments,
        "Set the include_line_comments option for the parser.";
      "--no-include-line-comments",
        Arg.Clear include_line_comments,
        "Unset the include_line_comments option for the parser.";
      "--keep-errors",
        Arg.Set keep_errors,
        "Set the keep_errors option for the parser.";
      "--no-keep-errors",
        Arg.Clear keep_errors,
        "Unset the keep_errors option for the parser.";
      "--quick-mode",
        Arg.Set quick_mode,
        "Set the quick_mode option for the parser.";
      "--no-quick-mode",
        Arg.Clear quick_mode,
        "Unset the quick_mode option for the parser.";
      "--lower-coroutines",
        Arg.Set lower_coroutines,
        "Set the lower_coroutines option for the parser.";
      "--no-lower-coroutines",
        Arg.Clear lower_coroutines,
        "Unset the lower_coroutines option for the parser.";
      "--fail-open",
        Arg.Set fail_open,
        "Set the fail_open option for the parser.";
      "--no-fail-open",
        Arg.Clear fail_open,
        "Unset the fail_open option for the parser.";
      "--force-hh-syntax",
        Arg.Set force_hh_syntax,
        "Force hh syntax for the parser.";
      "--show-file-name",
        Arg.Unit set_show_file_name,
        "Displays the file name.";
      ] in
    Arg.parse options push_file usage;
    make
      !full_fidelity_json
      !full_fidelity_text_json
      !full_fidelity_dot
      !full_fidelity_dot_edges
      !full_fidelity_errors
      !full_fidelity_errors_all
      !full_fidelity_s_expr
      !full_fidelity_ast_s_expr
      !original_parser_errors
      !original_parser_s_expr
      !program_text
      !pretty_print
      !schema
      !is_hh_file
      !codegen
      !php5_compat_mode
      !elaborate_namespaces
      !include_line_comments
      !keep_errors
      !quick_mode
      !lower_coroutines
      !enable_hh_syntax
      !fail_open
      !force_hh_syntax
      !show_file_name
      (List.rev !files)
end

open FullFidelityParseArgs

let print_error error = error
  |> Errors.to_absolute
  |> Errors.to_string
  |> output_string stdout

(* Prints a single FFP error. *)
let print_full_fidelity_error source_text error =
  let text = SyntaxError.to_positioned_string
    error (SourceText.offset_to_position source_text) in
  Printf.printf "%s\n" text

(* Computes and prints list of all FFP errors from syntax pass and parser pass.
 * Specifying all_errors=false will attempt to filter out duplicate errors. *)
let print_full_fidelity_errors ~source_text ~error_env =
  let errors = ParserErrors.parse_errors error_env in
  List.iter (print_full_fidelity_error source_text) errors

let handle_existing_file args filename =
  let popt = ParserOptions.default in
  let popt = ParserOptions.with_hh_syntax_for_hhvm popt
    (args.codegen && args.enable_hh_syntax) in

  if args.force_hh_syntax then begin
    Full_fidelity_lexer.Env.set ~force_hh:true ~enable_xhp:true;
  end;

  (* Parse with the full fidelity parser *)
  let file = Relative_path.create Relative_path.Dummy filename in
  let source_text = SourceText.from_file file in
  let _, mode = Full_fidelity_parser.get_language_and_mode source_text in
  let env = Full_fidelity_parser_env.make ?mode () in
  let syntax_tree = SyntaxTree.make ~env source_text in
  let editable = SyntaxTransforms.editable_from_positioned syntax_tree in

  (* Parse with the original parser *)
  let (original_errors, original_parse) = Errors.do_
    begin
      fun () -> Parser_hack.from_file popt file
    end in

  if args.show_file_name then begin
    Printf.printf "%s\n" filename
  end;
  if args.program_text then begin
    let text = Full_fidelity_editable_syntax.text editable in
    Printf.printf "%s\n" text
  end;
  if args.pretty_print then begin
    let pretty = Libhackfmt.format_tree syntax_tree in
    Printf.printf "%s\n" pretty
  end;

  let print_errors =
       args.codegen
    || args.full_fidelity_errors
    || args.full_fidelity_errors_all
  in
  if print_errors then begin
    let level = if args.full_fidelity_errors_all
      then ParserErrors.Maximum
      else ParserErrors.Typical in
    let hhvm_compat_mode = if args.codegen
      then ParserErrors.HHVMCompat
      else ParserErrors.NoCompat in
    let error_env = ParserErrors.make_env syntax_tree
      ~level
      ~hhvm_compat_mode
      ~codegen:args.codegen
    in
    print_full_fidelity_errors ~source_text ~error_env
  end;

  if args.full_fidelity_s_expr then begin
    let root = SyntaxTree.root syntax_tree in
    let str = DebugPos.dump_syntax root in
    Printf.printf "%s" str
  end;
  if args.full_fidelity_ast_s_expr then begin
    let module Lowerer = Full_fidelity_ast in
    let env =
      Lowerer.make_env
        ~codegen:args.codegen
        ~php5_compat_mode:args.php5_compat_mode
        ~elaborate_namespaces:args.elaborate_namespaces
        ~include_line_comments:args.include_line_comments
        ~keep_errors:args.keep_errors
        ~quick_mode:args.quick_mode
        ~lower_coroutines:args.lower_coroutines
        ~enable_hh_syntax:args.enable_hh_syntax
        ~parser_options:popt
        ~fail_open:args.fail_open
        ~is_hh_file:args.is_hh_file
        file
    in
    let res = Lowerer.from_file env in
    let str = Debug.dump_ast @@ Ast.AProgram res.Lowerer.ast in
    Printf.printf "%s" str
  end;
  if args.original_parser_errors then begin
    Errors.iter_error_list print_error original_errors
  end;
  if args.original_parser_s_expr then begin
    let ast = Ast.AProgram original_parse.Parser_return.ast in
    let str = Debug.dump_ast ast in
    Printf.printf "%s\n" str
  end;
  if args.full_fidelity_json then begin
    let json = SyntaxTree.to_json syntax_tree in
    let str = Hh_json.json_to_string json in
    Printf.printf "%s\n" str
  end;
  if args.full_fidelity_text_json then begin
    let json = Full_fidelity_editable_syntax.to_json editable in
    let str = Hh_json.json_to_string json in
    Printf.printf "%s\n" str
  end;
  if args.full_fidelity_dot then begin
    let dot = Full_fidelity_editable_syntax.to_dot editable false in
    Printf.printf "%s\n" dot
  end;
  if args.full_fidelity_dot_edges then begin
    let dot = Full_fidelity_editable_syntax.to_dot editable true in
    Printf.printf "%s\n" dot
  end

let handle_file args filename =
  if Path.file_exists (Path.make filename) then
    handle_existing_file args filename
  else
    Printf.printf "File %s does not exist.\n" filename

let rec main args files =
  if args.schema then begin
    let schema = Schema.schema_as_json() in
    Printf.printf "%s\n" schema
  end;
  match files with
  | [] -> ()
  | file :: tail ->
    begin
      Unix.handle_unix_error (handle_file args) file;
      main args tail
    end

let () =
   let args = parse_args () in
   EventLogger.init EventLogger.Event_logger_fake 0.0;
   let _ = SharedMem.init GlobalConfig.default_sharedmem_config in
   main args args.files
