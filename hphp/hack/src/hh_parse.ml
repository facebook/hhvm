(*
 * Copyright (c) 2016, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

(**
  *
  * Usage: hh_parse [OPTIONS] [FILES]
  *
  * --full-fidelity-json
  * --full-fidelity-errors
  * --full-fidelity-errors-all
  * --full-fidelity-s-expression
  * --full-fidelity-ast-s-expression
  * --program-text
  * --pretty-print
  * --show-file-name
  *
  * TODO: Parser for things other than scripts:
  *       types, expressions, statements, declarations, etc.
  *)

module Schema = Full_fidelity_schema
module SyntaxError = Full_fidelity_syntax_error
module SyntaxTree =
  Full_fidelity_syntax_tree.WithSyntax (Full_fidelity_positioned_syntax)
module SourceText = Full_fidelity_source_text
module ParserErrors =
  Full_fidelity_parser_errors.WithSyntax (Full_fidelity_positioned_syntax)
module DebugPos = Debug.WithSyntax (Full_fidelity_positioned_syntax)

module FullFidelityParseArgs = struct
  type t = {
    (* Output options *)
    full_fidelity_json: bool;
    full_fidelity_text_json: bool;
    full_fidelity_dot: bool;
    full_fidelity_dot_edges: bool;
    full_fidelity_errors: bool;
    full_fidelity_errors_all: bool;
    full_fidelity_s_expr: bool;
    full_fidelity_ast_s_expr: bool;
    program_text: bool;
    pretty_print: bool;
    schema: bool;
    show_file_name: bool;
    (* Configuring the parser *)
    is_hh_file: bool;
    codegen: bool;
    php5_compat_mode: bool;
    elaborate_namespaces: bool;
    include_line_comments: bool;
    keep_errors: bool;
    quick_mode: bool;
    lower_coroutines: bool;
    fail_open: bool;
    (* Defining the input *)
    files: string list;
    dump_nast: bool;
    disable_lval_as_an_expression: bool;
    rust_parser_errors: bool;
    enable_constant_visibility_modifiers: bool;
    enable_class_level_where_clauses: bool;
    disable_legacy_soft_typehints: bool;
    allow_new_attribute_syntax: bool;
    disable_legacy_attribute_syntax: bool;
    const_default_func_args: bool;
    const_static_props: bool;
    abstract_static_props: bool;
    disable_halt_compiler: bool;
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
      fail_open
      show_file_name
      files
      dump_nast
      disable_lval_as_an_expression
      rust_parser_errors
      enable_constant_visibility_modifiers
      enable_class_level_where_clauses
      disable_legacy_soft_typehints
      allow_new_attribute_syntax
      disable_legacy_attribute_syntax
      const_default_func_args
      const_static_props
      abstract_static_props
      disable_halt_compiler =
    {
      full_fidelity_json;
      full_fidelity_dot;
      full_fidelity_dot_edges;
      full_fidelity_text_json;
      full_fidelity_errors;
      full_fidelity_errors_all;
      full_fidelity_s_expr;
      full_fidelity_ast_s_expr;
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
      fail_open;
      show_file_name;
      files;
      dump_nast;
      disable_lval_as_an_expression;
      rust_parser_errors;
      enable_constant_visibility_modifiers;
      enable_class_level_where_clauses;
      disable_legacy_soft_typehints;
      allow_new_attribute_syntax;
      disable_legacy_attribute_syntax;
      const_default_func_args;
      const_static_props;
      abstract_static_props;
      disable_halt_compiler;
    }

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
    let set_full_fidelity_errors_all () = full_fidelity_errors_all := true in
    let full_fidelity_s_expr = ref false in
    let full_fidelity_ast_s_expr = ref false in
    let set_full_fidelity_s_expr () = full_fidelity_s_expr := true in
    let set_full_fidelity_ast_s_expr () = full_fidelity_ast_s_expr := true in
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
    let show_file_name = ref false in
    let dump_nast = ref false in
    let disable_lval_as_an_expression = ref false in
    let set_show_file_name () = show_file_name := true in
    let files = ref [] in
    let push_file file = files := file :: !files in
    let rust_parser_errors = ref false in
    let enable_constant_visibility_modifiers = ref false in
    let enable_class_level_where_clauses = ref false in
    let disable_legacy_soft_typehints = ref false in
    let allow_new_attribute_syntax = ref false in
    let disable_legacy_attribute_syntax = ref false in
    let const_default_func_args = ref false in
    let const_static_props = ref false in
    let abstract_static_props = ref false in
    let disable_halt_compiler = ref false in
    let options =
      [
        (* modes *)
          ( "--full-fidelity-json",
            Arg.Unit set_full_fidelity_json,
            "Displays the full-fidelity parse tree in JSON format." );
        ( "--full-fidelity-text-json",
          Arg.Unit set_full_fidelity_text_json,
          "Displays the full-fidelity parse tree in JSON format with token text."
        );
        ( "--full-fidelity-dot",
          Arg.Unit set_full_fidelity_dot,
          "Displays the full-fidelity parse tree in GraphViz DOT format." );
        ( "--full-fidelity-dot-edges",
          Arg.Unit set_full_fidelity_dot_edges,
          "Displays the full-fidelity parse tree in GraphViz DOT format with edge labels."
        );
        ( "--full-fidelity-errors",
          Arg.Unit set_full_fidelity_errors,
          "Displays the full-fidelity parser errors, if any.
Some errors may be filtered out."
        );
        ( "--full-fidelity-errors-all",
          Arg.Unit set_full_fidelity_errors_all,
          "Displays the full-fidelity parser errors, if any.
No errors are filtered out."
        );
        ( "--full-fidelity-s-expression",
          Arg.Unit set_full_fidelity_s_expr,
          "Displays the full-fidelity parse tree in S-expression format." );
        ( "--full-fidelity-ast-s-expression",
          Arg.Unit set_full_fidelity_ast_s_expr,
          "Displays the AST produced by the FFP in S-expression format." );
        ( "--program-text",
          Arg.Unit set_program_text,
          "Displays the text of the given file." );
        ( "--pretty-print",
          Arg.Unit set_pretty_print,
          "Displays the text of the given file after pretty-printing." );
        ( "--schema",
          Arg.Unit set_schema,
          "Displays the parser version and schema of nodes." );
        ( "--is-hh-file",
          Arg.Set is_hh_file,
          "Set the is_hh_file option for the parser." );
        ( "--no-is-hh-file",
          Arg.Clear is_hh_file,
          "Unset the is_hh_file option for the parser." );
        ("--codegen", Arg.Set codegen, "Set the codegen option for the parser.");
        ( "--no-codegen",
          Arg.Clear codegen,
          "Unset the codegen option for the parser." );
        ( "--php5-compat-mode",
          Arg.Set php5_compat_mode,
          "Set the php5_compat_mode option for the parser." );
        ( "--no-php5-compat-mode",
          Arg.Clear php5_compat_mode,
          "Unset the php5_compat_mode option for the parser." );
        ( "--elaborate-namespaces",
          Arg.Set elaborate_namespaces,
          "Set the elaborate_namespaces option for the parser." );
        ( "--no-elaborate-namespaces",
          Arg.Clear elaborate_namespaces,
          "Unset the elaborate_namespaces option for the parser." );
        ( "--include-line-comments",
          Arg.Set include_line_comments,
          "Set the include_line_comments option for the parser." );
        ( "--no-include-line-comments",
          Arg.Clear include_line_comments,
          "Unset the include_line_comments option for the parser." );
        ( "--keep-errors",
          Arg.Set keep_errors,
          "Set the keep_errors option for the parser." );
        ( "--no-keep-errors",
          Arg.Clear keep_errors,
          "Unset the keep_errors option for the parser." );
        ( "--quick-mode",
          Arg.Set quick_mode,
          "Set the quick_mode option for the parser." );
        ( "--no-quick-mode",
          Arg.Clear quick_mode,
          "Unset the quick_mode option for the parser." );
        ( "--lower-coroutines",
          Arg.Set lower_coroutines,
          "Set the lower_coroutines option for the parser." );
        ( "--no-lower-coroutines",
          Arg.Clear lower_coroutines,
          "Unset the lower_coroutines option for the parser." );
        ( "--fail-open",
          Arg.Set fail_open,
          "Set the fail_open option for the parser." );
        ( "--no-fail-open",
          Arg.Clear fail_open,
          "Unset the fail_open option for the parser." );
        ("--force-hh-syntax", Arg.Set enable_hh_syntax, "Ignored. Do not use.");
        ( "--show-file-name",
          Arg.Unit set_show_file_name,
          "Displays the file name." );
        ( "--dump-nast",
          Arg.Set dump_nast,
          "Converts the legacy AST to a NAST and prints it." );
        ( "--disable-lval-as-an-expression",
          Arg.Set disable_lval_as_an_expression,
          "Disable lval as an expression." );
        ( "--rust-parser-errors",
          Arg.Bool (fun x -> rust_parser_errors := x),
          "Use the parser errors written in Rust instead of OCaml one" );
        ( "--enable-constant-visibility-modifiers",
          Arg.Set enable_constant_visibility_modifiers,
          "Require constants to have visibility modifiers" );
        ( "--enable-class-level-where-clauses",
          Arg.Set enable_class_level_where_clauses,
          "Enables support for class-level where clauses" );
        ( "--disable-legacy-soft-typehints",
          Arg.Set disable_legacy_soft_typehints,
          "Disables the legacy @ syntax for soft typehints (use __Soft instead)"
        );
        ( "--allow-new-attribute-syntax",
          Arg.Set allow_new_attribute_syntax,
          "Allow the new @ attribute syntax (disables legacy soft typehints)"
        );
        ( "--disable-legacy-attribute-syntax",
          Arg.Set disable_legacy_attribute_syntax,
          "Disable the legacy <<...>> user attribute syntax" );
        ( "--const-default-func-args",
          Arg.Set const_default_func_args,
          "Statically check default function arguments are constant initializers"
        );
        ( "--const-static-props",
          Arg.Set const_static_props,
          "Enable static properties to be const" );
        ( "--abstract-static-props",
          Arg.Set abstract_static_props,
          "Enable abstract static properties" );
        ( "--disable-halt-compiler",
          Arg.Set disable_halt_compiler,
          "Disable using PHP __halt_compiler()" );
      ]
    in
    Arg.parse options push_file usage;
    let modes =
      [
        !full_fidelity_json;
        !full_fidelity_text_json;
        !full_fidelity_dot;
        !full_fidelity_dot_edges;
        !full_fidelity_errors;
        !full_fidelity_errors_all;
        !full_fidelity_s_expr;
        !full_fidelity_ast_s_expr;
        !program_text;
        !pretty_print;
        !schema;
      ]
    in
    if not (List.exists (fun x -> x) modes) then
      full_fidelity_errors_all := true;
    make
      !full_fidelity_json
      !full_fidelity_text_json
      !full_fidelity_dot
      !full_fidelity_dot_edges
      !full_fidelity_errors
      !full_fidelity_errors_all
      !full_fidelity_s_expr
      !full_fidelity_ast_s_expr
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
      !fail_open
      !show_file_name
      (List.rev !files)
      !dump_nast
      !disable_lval_as_an_expression
      !rust_parser_errors
      !enable_constant_visibility_modifiers
      !enable_class_level_where_clauses
      !disable_legacy_soft_typehints
      !allow_new_attribute_syntax
      !disable_legacy_attribute_syntax
      !const_default_func_args
      !const_static_props
      !abstract_static_props
      !disable_halt_compiler
end

open FullFidelityParseArgs

(* Prints a single FFP error. *)
let print_full_fidelity_error source_text error =
  let text =
    SyntaxError.to_positioned_string
      error
      (SourceText.offset_to_position source_text)
  in
  Printf.printf "%s\n" text

let handle_existing_file args filename =
  let popt = ParserOptions.default in
  let popt = ParserOptions.with_codegen popt args.codegen in
  let popt =
    ParserOptions.with_disable_lval_as_an_expression
      popt
      args.disable_lval_as_an_expression
  in
  let popt =
    ParserOptions.with_enable_constant_visibility_modifiers
      popt
      args.enable_constant_visibility_modifiers
  in
  let popt =
    {
      popt with
      GlobalOptions.po_enable_class_level_where_clauses =
        args.enable_class_level_where_clauses;
    }
  in
  let popt =
    ParserOptions.with_disable_legacy_soft_typehints
      popt
      args.disable_legacy_soft_typehints
  in
  let popt =
    ParserOptions.with_allow_new_attribute_syntax
      popt
      args.allow_new_attribute_syntax
  in
  let popt =
    ParserOptions.with_disable_legacy_attribute_syntax
      popt
      args.disable_legacy_attribute_syntax
  in
  let popt =
    ParserOptions.with_const_default_func_args
      popt
      args.const_default_func_args
  in
  let popt =
    ParserOptions.with_const_static_props popt args.const_static_props
  in
  let popt =
    ParserOptions.with_abstract_static_props popt args.abstract_static_props
  in
  let popt =
    ParserOptions.with_disable_halt_compiler popt args.disable_halt_compiler
  in
  (* Parse with the full fidelity parser *)
  let file = Relative_path.create Relative_path.Dummy filename in
  let source_text = SourceText.from_file file in
  let mode = Full_fidelity_parser.parse_mode source_text in
  let print_errors =
    args.codegen || args.full_fidelity_errors || args.full_fidelity_errors_all
  in
  let env =
    Full_fidelity_parser_env.make
      ~disable_lval_as_an_expression:args.disable_lval_as_an_expression
      ~disable_legacy_soft_typehints:args.disable_legacy_soft_typehints
      ~allow_new_attribute_syntax:args.allow_new_attribute_syntax
      ~disable_legacy_attribute_syntax:
        args.disable_legacy_attribute_syntax
        (* When print_errors is true, the leaked tree will be passed to ParserErrors,
         * which will consume it. *)
      ~leak_rust_tree:(args.rust_parser_errors && print_errors)
      ?mode
      ()
  in
  let syntax_tree = SyntaxTree.make ~env source_text in
  let editable = SyntaxTransforms.editable_from_positioned syntax_tree in
  if args.show_file_name then Printf.printf "%s\n" filename;
  ( if args.program_text then
    let text = Full_fidelity_editable_syntax.text editable in
    Printf.printf "%s\n" text );
  ( if args.pretty_print then
    let pretty = Libhackfmt.format_tree syntax_tree in
    Printf.printf "%s\n" pretty );
  ( if args.full_fidelity_s_expr then
    let root = SyntaxTree.root syntax_tree in
    let str = DebugPos.dump_syntax root in
    Printf.printf "%s\n" str );

  let dump_needed = args.full_fidelity_ast_s_expr || args.dump_nast in
  let lowered =
    if dump_needed || print_errors then
      let env =
        Full_fidelity_ast.make_env
          ~codegen:args.codegen
          ~php5_compat_mode:args.php5_compat_mode
          ~elaborate_namespaces:args.elaborate_namespaces
          ~include_line_comments:args.include_line_comments
          ~keep_errors:(args.keep_errors && not print_errors)
          ~quick_mode:args.quick_mode
          ~lower_coroutines:args.lower_coroutines
          ~parser_options:popt
          ~fail_open:args.fail_open
          ~is_hh_file:args.is_hh_file
          file
      in
      try Some (Full_fidelity_ast.from_file env)
      with _ when print_errors -> None
    else
      None
  in
  ( if print_errors then
    let level =
      if args.full_fidelity_errors_all then
        ParserErrors.Maximum
      else
        ParserErrors.Typical
    in
    let hhvm_compat_mode =
      if args.codegen then
        ParserErrors.HHVMCompat
      else
        ParserErrors.NoCompat
    in
    let error_env =
      ParserErrors.make_env
        syntax_tree
        ~level
        ~hhvm_compat_mode
        ~codegen:args.codegen
        ~parser_options:popt
    in
    let errors = ParserErrors.parse_errors error_env in
    List.iter (print_full_fidelity_error source_text) errors );
  begin
    match lowered with
    | Some res ->
      let ast = res.Full_fidelity_ast.ast in
      if print_errors then
        Ast_check.check_program ast
        |> List.iter (print_full_fidelity_error source_text);
      if dump_needed then
        let str =
          if args.dump_nast then
            Nast.show_program (Ast_to_nast.convert ast)
          else
            Debug.dump_ast (Ast.AProgram ast)
        in
        Printf.printf "%s\n" str
    | None -> ()
  end;
  ( if args.full_fidelity_json then
    let json = SyntaxTree.to_json syntax_tree in
    let str = Hh_json.json_to_string json in
    Printf.printf "%s\n" str );
  ( if args.full_fidelity_text_json then
    let json = Full_fidelity_editable_syntax.to_json editable in
    let str = Hh_json.json_to_string json in
    Printf.printf "%s\n" str );
  ( if args.full_fidelity_dot then
    let dot = Full_fidelity_editable_syntax.to_dot editable false in
    Printf.printf "%s\n" dot );
  if args.full_fidelity_dot_edges then
    let dot = Full_fidelity_editable_syntax.to_dot editable true in
    Printf.printf "%s\n" dot

let handle_file args filename =
  if Path.file_exists (Path.make filename) then
    handle_existing_file args filename
  else
    Printf.printf "File %s does not exist.\n" filename

let rec main args files =
  ( if args.schema then
    let schema = Schema.schema_as_json () in
    Printf.printf "%s\n" schema );
  match files with
  | [] -> ()
  | file :: tail ->
    Unix.handle_unix_error (handle_file args) file;
    main args tail

let () =
  let args = parse_args () in
  EventLogger.init EventLogger.Event_logger_fake 0.0;
  let handle =
    SharedMem.init ~num_workers:0 GlobalConfig.default_sharedmem_config
  in
  ignore (handle : SharedMem.handle);
  main args args.files
