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
  * --full-fidelity-json-parse-tree
  * --full-fidelity-errors
  * --full-fidelity-errors-all
  * --full-fidelity-ast-s-expression
  * --program-text
  * --pretty-print
  * --pretty-print-json
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

module FullFidelityParseArgs = struct
  type t = {
    (* Output options *)
    full_fidelity_json_parse_tree: bool;
    full_fidelity_json: bool;
    full_fidelity_text_json: bool;
    full_fidelity_dot: bool;
    full_fidelity_dot_edges: bool;
    full_fidelity_errors: bool;
    full_fidelity_errors_all: bool;
    full_fidelity_ast_s_expr: bool;
    program_text: bool;
    pretty_print: bool;
    pretty_print_json: bool;
    generate_hhi: bool;
    schema: bool;
    show_file_name: bool;
    (* Configuring the parser *)
    codegen: bool;
    php5_compat_mode: bool;
    elaborate_namespaces: bool;
    include_line_comments: bool;
    keep_errors: bool;
    quick_mode: bool;
    fail_open: bool;
    (* Defining the input *)
    files: string list;
    disable_lval_as_an_expression: bool;
    enable_class_level_where_clauses: bool;
    disable_legacy_soft_typehints: bool;
    allow_new_attribute_syntax: bool;
    disable_legacy_attribute_syntax: bool;
    const_default_func_args: bool;
    const_default_lambda_args: bool;
    const_static_props: bool;
    abstract_static_props: bool;
    disallow_func_ptrs_in_constants: bool;
    error_php_lambdas: bool;
    disallow_discarded_nullable_awaitables: bool;
    disable_xhp_element_mangling: bool;
    allow_unstable_features: bool;
    enable_xhp_class_modifier: bool;
    disallow_hash_comments: bool;
    disallow_fun_and_cls_meth_pseudo_funcs: bool;
    disallow_inst_meth: bool;
    ignore_missing_json: bool;
  }

  let make
      full_fidelity_json_parse_tree
      full_fidelity_json
      full_fidelity_text_json
      full_fidelity_dot
      full_fidelity_dot_edges
      full_fidelity_errors
      full_fidelity_errors_all
      full_fidelity_ast_s_expr
      program_text
      pretty_print
      pretty_print_json
      generate_hhi
      schema
      codegen
      php5_compat_mode
      elaborate_namespaces
      include_line_comments
      keep_errors
      quick_mode
      fail_open
      show_file_name
      files
      disable_lval_as_an_expression
      enable_class_level_where_clauses
      disable_legacy_soft_typehints
      allow_new_attribute_syntax
      disable_legacy_attribute_syntax
      const_default_func_args
      const_default_lambda_args
      const_static_props
      abstract_static_props
      disallow_func_ptrs_in_constants
      error_php_lambdas
      disallow_discarded_nullable_awaitables
      disable_xhp_element_mangling
      allow_unstable_features
      enable_xhp_class_modifier
      disallow_hash_comments
      disallow_fun_and_cls_meth_pseudo_funcs
      disallow_inst_meth
      ignore_missing_json =
    {
      full_fidelity_json_parse_tree;
      full_fidelity_json;
      full_fidelity_dot;
      full_fidelity_dot_edges;
      full_fidelity_text_json;
      full_fidelity_errors;
      full_fidelity_errors_all;
      full_fidelity_ast_s_expr;
      program_text;
      pretty_print;
      pretty_print_json;
      generate_hhi;
      schema;
      codegen;
      php5_compat_mode;
      elaborate_namespaces;
      include_line_comments;
      keep_errors;
      quick_mode;
      fail_open;
      show_file_name;
      files;
      disable_lval_as_an_expression;
      enable_class_level_where_clauses;
      disable_legacy_soft_typehints;
      allow_new_attribute_syntax;
      disable_legacy_attribute_syntax;
      const_default_func_args;
      const_default_lambda_args;
      const_static_props;
      abstract_static_props;
      disallow_func_ptrs_in_constants;
      error_php_lambdas;
      disallow_discarded_nullable_awaitables;
      disable_xhp_element_mangling;
      allow_unstable_features;
      enable_xhp_class_modifier;
      disallow_hash_comments;
      disallow_fun_and_cls_meth_pseudo_funcs;
      disallow_inst_meth;
      ignore_missing_json;
    }

  let parse_args () =
    let usage = Printf.sprintf "Usage: %s [OPTIONS] filename\n" Sys.argv.(0) in
    let full_fidelity_json_parse_tree = ref false in
    let set_full_fidelity_json_parse_tree () =
      full_fidelity_json_parse_tree := true
    in
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
    let full_fidelity_ast_s_expr = ref false in
    let set_full_fidelity_ast_s_expr () = full_fidelity_ast_s_expr := true in
    let program_text = ref false in
    let set_program_text () = program_text := true in
    let pretty_print = ref false in
    let set_pretty_print () = pretty_print := true in
    let pretty_print_json = ref false in
    let set_pretty_print_json () = pretty_print_json := true in
    let generate_hhi = ref false in
    let set_generate_hhi () = generate_hhi := true in
    let schema = ref false in
    let set_schema () = schema := true in
    let codegen = ref false in
    let php5_compat_mode = ref false in
    let elaborate_namespaces = ref true in
    let include_line_comments = ref false in
    let keep_errors = ref true in
    let quick_mode = ref false in
    let enable_hh_syntax = ref false in
    let fail_open = ref true in
    let show_file_name = ref false in
    let disable_lval_as_an_expression = ref false in
    let set_show_file_name () = show_file_name := true in
    let files = ref [] in
    let push_file file = files := file :: !files in
    let enable_class_level_where_clauses = ref false in
    let disable_legacy_soft_typehints = ref false in
    let allow_new_attribute_syntax = ref false in
    let disable_legacy_attribute_syntax = ref false in
    let const_default_func_args = ref false in
    let const_default_lambda_args = ref false in
    let const_static_props = ref false in
    let abstract_static_props = ref false in
    let disallow_func_ptrs_in_constants = ref false in
    let error_php_lambdas = ref false in
    let disallow_discarded_nullable_awaitables = ref false in
    let disable_xhp_element_mangling = ref false in
    let allow_unstable_features = ref false in
    let enable_xhp_class_modifier = ref false in
    let disallow_hash_comments = ref false in
    let disallow_fun_and_cls_meth_pseudo_funcs = ref false in
    let disallow_inst_meth = ref false in
    let ignore_missing_json = ref false in
    let options =
      [
        (* modes *)
        ( "--full-fidelity-json-parse-tree",
          Arg.Unit set_full_fidelity_json_parse_tree,
          "Displays the full-fidelity parse tree in JSON format." );
        ( "--full-fidelity-json",
          Arg.Unit set_full_fidelity_json,
          "Displays the source text, FFP schema version, and parse tree in JSON format."
        );
        ( "--full-fidelity-text-json",
          Arg.Unit set_full_fidelity_text_json,
          "Displays the source text, FFP schema version, and parse tree (with trivia) in JSON format."
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
        ( "--full-fidelity-ast-s-expression",
          Arg.Unit set_full_fidelity_ast_s_expr,
          "Displays the AST produced by the FFP in S-expression format." );
        ( "--program-text",
          Arg.Unit set_program_text,
          "Displays the text of the given file." );
        ( "--generate-hhi",
          Arg.Unit set_generate_hhi,
          "Generate and display a .hhi file for the given input file." );
        ( "--pretty-print",
          Arg.Unit set_pretty_print,
          "Displays the text of the given file after pretty-printing." );
        ( "--pretty-print-json",
          Arg.Unit set_pretty_print_json,
          "Pretty print JSON output." );
        ( "--schema",
          Arg.Unit set_schema,
          "Displays the parser version and schema of nodes." );
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
        ( "--disable-lval-as-an-expression",
          Arg.Set disable_lval_as_an_expression,
          "Disable lval as an expression." );
        ( "--enable-class-level-where-clauses",
          Arg.Set enable_class_level_where_clauses,
          "Enables support for class-level where clauses" );
        ( "--disable-legacy-soft-typehints",
          Arg.Set disable_legacy_soft_typehints,
          "Disables the legacy @ syntax for soft typehints (use __Soft instead)"
        );
        ( "--allow-new-attribute-syntax",
          Arg.Set allow_new_attribute_syntax,
          "Allow the new @ attribute syntax (disables legacy soft typehints)" );
        ( "--disable-legacy-attribute-syntax",
          Arg.Set disable_legacy_attribute_syntax,
          "Disable the legacy <<...>> user attribute syntax" );
        ( "--const-default-func-args",
          Arg.Set const_default_func_args,
          "Statically check default function arguments are constant initializers"
        );
        ( "--const-default-lambda-args",
          Arg.Set const_default_lambda_args,
          " Statically check default lambda args are constant."
          ^ " Produces a subset of errors of const-default-func-args" );
        ( "--const-static-props",
          Arg.Set const_static_props,
          "Enable static properties to be const" );
        ( "--abstract-static-props",
          Arg.Set abstract_static_props,
          "Enable abstract static properties" );
        ( "--disallow-func-ptrs-in-constants",
          Arg.Set disallow_func_ptrs_in_constants,
          "Disallow use of HH\\fun and HH\\class_meth in constants and constant initializers"
        );
        ( "--error-php-lambdas",
          Arg.Set error_php_lambdas,
          "Report errors on php style anonymous functions" );
        ( "--disallow-discarded-nullable-awaitables",
          Arg.Set disallow_discarded_nullable_awaitables,
          "Error on using discarded nullable awaitables" );
        ( "--enable-first-class-function-pointers",
          Arg.Unit (fun () -> ()),
          "Deprecated - delete in coordination with HackAST" );
        ( "--enable-xhp-class-modifier",
          Arg.Set enable_xhp_class_modifier,
          "Enables the 'xhp class foo' syntax" );
        ( "--disable-xhp-element-mangling",
          Arg.Set disable_xhp_element_mangling,
          "Disable mangling of XHP elements :foo. That is, :foo:bar is now \\foo\\bar, not xhp_foo__bar"
        );
        ( "--allow-unstable-features",
          Arg.Set allow_unstable_features,
          "Enables the __EnableUnstableFeatures attribute" );
        ( "--disallow-hash-comments",
          Arg.Set disallow_hash_comments,
          "Disables hash-style(#) comments (except hashbangs)" );
        ( "--disallow-fun-and-cls-meth-pseudo-funcs",
          Arg.Set disallow_fun_and_cls_meth_pseudo_funcs,
          "Disables parsing of fun() and class_meth()" );
        ( "--disallow-inst-meth",
          Arg.Set disallow_inst_meth,
          "Disabled parsing of inst_meth()" );
        ( "--ignore-missing-json",
          Arg.Set ignore_missing_json,
          "Ignore missing nodes in JSON output" );
      ]
    in
    Arg.parse options push_file usage;
    let modes =
      [
        !full_fidelity_json_parse_tree;
        !full_fidelity_json;
        !full_fidelity_text_json;
        !full_fidelity_dot;
        !full_fidelity_dot_edges;
        !full_fidelity_errors;
        !full_fidelity_errors_all;
        !full_fidelity_ast_s_expr;
        !program_text;
        !pretty_print;
        !schema;
      ]
    in
    if not (List.exists (fun x -> x) modes) then
      full_fidelity_errors_all := true;
    make
      !full_fidelity_json_parse_tree
      !full_fidelity_json
      !full_fidelity_text_json
      !full_fidelity_dot
      !full_fidelity_dot_edges
      !full_fidelity_errors
      !full_fidelity_errors_all
      !full_fidelity_ast_s_expr
      !program_text
      !pretty_print
      !pretty_print_json
      !generate_hhi
      !schema
      !codegen
      !php5_compat_mode
      !elaborate_namespaces
      !include_line_comments
      !keep_errors
      !quick_mode
      !fail_open
      !show_file_name
      (List.rev !files)
      !disable_lval_as_an_expression
      !enable_class_level_where_clauses
      !disable_legacy_soft_typehints
      !allow_new_attribute_syntax
      !disable_legacy_attribute_syntax
      !const_default_func_args
      !const_default_lambda_args
      !const_static_props
      !abstract_static_props
      !disallow_func_ptrs_in_constants
      !error_php_lambdas
      !disallow_discarded_nullable_awaitables
      !disable_xhp_element_mangling
      !allow_unstable_features
      !enable_xhp_class_modifier
      !disallow_hash_comments
      !disallow_fun_and_cls_meth_pseudo_funcs
      !disallow_inst_meth
      !ignore_missing_json
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

let print_ast_check_errors errors =
  let error_list = Errors.get_error_list errors in
  List.iter
    (fun e ->
      let text = Errors.to_string (Errors.to_absolute e) in
      if
        Core_kernel.String.is_substring text SyntaxError.this_in_static
        || Core_kernel.String.is_substring text SyntaxError.toplevel_await_use
      then
        Printf.eprintf "%s\n%!" text)
    error_list

let handle_existing_file args filename =
  let popt = ParserOptions.default in
  let popt = ParserOptions.with_codegen popt args.codegen in
  let popt =
    ParserOptions.with_disable_lval_as_an_expression
      popt
      args.disable_lval_as_an_expression
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
    ParserOptions.with_const_default_func_args popt args.const_default_func_args
  in
  let popt =
    ParserOptions.with_const_default_lambda_args
      popt
      args.const_default_lambda_args
  in
  let popt =
    ParserOptions.with_const_static_props popt args.const_static_props
  in
  let popt =
    ParserOptions.with_abstract_static_props popt args.abstract_static_props
  in
  let popt =
    ParserOptions.with_disallow_func_ptrs_in_constants
      popt
      args.disallow_func_ptrs_in_constants
  in
  let popt =
    ParserOptions.with_disable_xhp_element_mangling
      popt
      args.disable_xhp_element_mangling
  in
  let popt =
    ParserOptions.with_allow_unstable_features popt args.allow_unstable_features
  in
  let popt =
    ParserOptions.with_enable_xhp_class_modifier
      popt
      args.enable_xhp_class_modifier
  in
  let popt =
    ParserOptions.with_disallow_hash_comments popt args.disallow_hash_comments
  in
  let popt =
    ParserOptions.with_disallow_fun_and_cls_meth_pseudo_funcs
      popt
      args.disallow_fun_and_cls_meth_pseudo_funcs
  in
  let popt =
    ParserOptions.with_disallow_inst_meth popt args.disallow_inst_meth
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
      ~leak_rust_tree:print_errors
      ~disallow_hash_comments:args.disallow_hash_comments
      ~disallow_fun_and_cls_meth_pseudo_funcs:
        args.disallow_fun_and_cls_meth_pseudo_funcs
      ~disallow_inst_meth:args.disallow_inst_meth
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
  ( if args.generate_hhi then
    let hhi = Generate_hhi.go editable in
    Printf.printf "%s\n" hhi );

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
    let dump_needed = args.full_fidelity_ast_s_expr in
    let lowered =
      if dump_needed || print_errors then
        let env =
          Full_fidelity_ast.make_env
            ~codegen:args.codegen
            ~php5_compat_mode:args.php5_compat_mode
            ~elaborate_namespaces:args.elaborate_namespaces
            ~include_line_comments:args.include_line_comments
            ~keep_errors:(args.keep_errors || print_errors)
            ~quick_mode:args.quick_mode
            ~parser_options:popt
            ~fail_open:args.fail_open
            file
        in
        try
          let (errors, res) =
            Errors.do_ (fun () -> Full_fidelity_ast.from_file_with_legacy env)
          in
          if print_errors then print_ast_check_errors errors;
          Some res
        with _ when print_errors -> None
      else
        None
    in
    match lowered with
    | Some res ->
      if dump_needed then
        let ast = res.Parser_return.ast in
        let str = Nast.show_program ast in
        Printf.printf "%s\n" str
    | None -> ()
  end;
  ( if args.full_fidelity_json_parse_tree then
    let json =
      SyntaxTree.parse_tree_to_json
        ~ignore_missing:args.ignore_missing_json
        syntax_tree
    in
    let str = Hh_json.json_to_string json ~pretty:args.pretty_print_json in
    Printf.printf "%s\n" str );
  ( if args.full_fidelity_json then
    let json =
      SyntaxTree.to_json ~ignore_missing:args.ignore_missing_json syntax_tree
    in
    let str = Hh_json.json_to_string json ~pretty:args.pretty_print_json in
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
  EventLogger.init_fake ();
  let handle = SharedMem.init ~num_workers:0 SharedMem.default_config in
  ignore (handle : SharedMem.handle);
  main args args.files
