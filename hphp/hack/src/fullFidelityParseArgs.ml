(*
 * Copyright (c) 2016, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

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
  quick_mode: bool;
  (* Defining the input *)
  files: string list;
  disable_lval_as_an_expression: bool;
  enable_class_level_where_clauses: bool;
  disable_legacy_soft_typehints: bool;
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
  ignore_missing_json: bool;
  disallow_static_constants_in_default_func_args: bool;
  disallow_direct_superglobals_refs: bool;
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
    quick_mode
    show_file_name
    files
    disable_lval_as_an_expression
    enable_class_level_where_clauses
    disable_legacy_soft_typehints
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
    ignore_missing_json
    disallow_static_constants_in_default_func_args
    disallow_direct_superglobals_refs =
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
    quick_mode;
    show_file_name;
    files;
    disable_lval_as_an_expression;
    enable_class_level_where_clauses;
    disable_legacy_soft_typehints;
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
    ignore_missing_json;
    disallow_static_constants_in_default_func_args;
    disallow_direct_superglobals_refs;
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
  let quick_mode = ref false in
  let enable_hh_syntax = ref false in
  let show_file_name = ref false in
  let disable_lval_as_an_expression = ref false in
  let set_show_file_name () = show_file_name := true in
  let files = ref [] in
  let push_file file = files := file :: !files in
  let enable_class_level_where_clauses = ref false in
  let disable_legacy_soft_typehints = ref false in
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
  let ignore_missing_json = ref false in
  let disallow_static_constants_in_default_func_arg = ref false in
  let disallow_direct_superglobals_refs = ref false in
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
      ( "--quick-mode",
        Arg.Set quick_mode,
        "Set the quick_mode option for the parser." );
      ( "--no-quick-mode",
        Arg.Clear quick_mode,
        "Unset the quick_mode option for the parser." );
      ("--fail-open", Arg.Unit (fun () -> ()), "Unused.");
      ("--no-fail-open", Arg.Unit (fun () -> ()), "Unused");
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
      ( "--ignore-missing-json",
        Arg.Set ignore_missing_json,
        "Ignore missing nodes in JSON output" );
      ( "--disallow-static-constants-in-default-func-args",
        Arg.Set disallow_static_constants_in_default_func_arg,
        "Disallow `static::*` in default arguments for functions" );
      ( "--disallow-direct-superglobals-refs",
        Arg.Set disallow_direct_superglobals_refs,
        "Disallow accessing superglobals via their variable names" );
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
  if not (List.exists (fun x -> x) modes) then full_fidelity_errors_all := true;
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
    !quick_mode
    !show_file_name
    (List.rev !files)
    !disable_lval_as_an_expression
    !enable_class_level_where_clauses
    !disable_legacy_soft_typehints
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
    !ignore_missing_json
    !disallow_static_constants_in_default_func_arg
    !disallow_direct_superglobals_refs

let to_parser_options args =
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
    ParserOptions.with_disallow_static_constants_in_default_func_args
      popt
      args.disallow_static_constants_in_default_func_args
  in
  let popt =
    ParserOptions.with_disallow_direct_superglobals_refs
      popt
      args.disallow_direct_superglobals_refs
  in
  popt

let to_parser_env args ~leak_rust_tree ~mode =
  Full_fidelity_parser_env.make
    ~disable_lval_as_an_expression:args.disable_lval_as_an_expression
    ~disable_legacy_soft_typehints:args.disable_legacy_soft_typehints
    ~disable_legacy_attribute_syntax:args.disable_legacy_attribute_syntax
    ~leak_rust_tree
    ?mode
    ()
