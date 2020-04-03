(*
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Core_kernel
open File_content
open String_utils
open Sys_utils
open Typing_env_types
module Inf = Typing_inference_env

module StringAnnotation = struct
  type t = string

  let pp fmt str = Format.pp_print_string fmt str
end

module PS = Full_fidelity_positioned_syntax
module PositionedTree = Full_fidelity_syntax_tree.WithSyntax (PS)

(*****************************************************************************)
(* Types, constants *)
(*****************************************************************************)

type mode =
  | Ai of Ai_options.t
  | Autocomplete
  | Autocomplete_manually_invoked
  | Ffp_autocomplete
  | Color
  | Coverage
  | Cst_search
  | Dump_symbol_info
  | Dump_inheritance
  | Errors
  | Lint
  | Dump_deps
  | Identify_symbol of int * int
  | Find_local of int * int
  | Outline
  | Dump_nast
  | Dump_stripped_tast
  | Dump_tast
  | Check_tast
  | Rewrite
  | Find_refs of int * int
  | Highlight_refs of int * int
  | Decl_compare
  | Shallow_class_diff
  | Linearization
  | Go_to_impl of int * int
  | Dump_glean_deps

type options = {
  files: string list;
  mode: mode;
  error_format: Errors.format;
  no_builtins: bool;
  max_errors: int option;
  tcopt: GlobalOptions.t;
  batch_mode: bool;
  out_extension: string;
  verbosity: int;
}

(* Canonical builtins from our hhi library *)
let hhi_builtins = Hhi.get_raw_hhi_contents ()

(* All of the stuff that hh_single_type_check relies on is sadly not contained
 * in the hhi library, so we include a very small number of magic builtins *)
let magic_builtins =
  [|
    ( "hh_single_type_check_magic.hhi",
      "<?hh\n"
      ^ "namespace {\n"
      ^ "async function gena<Tk as arraykey, Tv>(
  KeyedTraversable<Tk, Awaitable<Tv>> $awaitables,
): Awaitable<darray<Tk, Tv>>;\n"
      ^ "function hh_show(<<__AcceptDisposable>> $val) {}\n"
      ^ "function hh_show_env() {}\n"
      ^ "function hh_log_level($key, $level) {}\n"
      ^ "function hh_force_solve () {}"
      ^ "}\n"
      ^ "namespace HH\\Lib\\Tuple{\n"
      ^ "function gen();\n"
      ^ "function from_async();\n"
      ^ "}\n" );
  |]

(*****************************************************************************)
(* Helpers *)
(*****************************************************************************)

let die str =
  let oc = stderr in
  Out_channel.output_string oc str;
  Out_channel.close oc;
  exit 2

let print_error format ?(oc = stderr) l =
  let formatter =
    match format with
    | Errors.Context -> Errors.to_contextual_string
    | Errors.Raw -> (fun e -> Errors.to_string ~indent:false e)
  in
  Out_channel.output_string oc (formatter (Errors.to_absolute_for_test l))

let write_error_list format errors oc max_errors =
  let (shown_errors, dropped_errors) =
    match max_errors with
    | Some max_errors -> List.split_n errors max_errors
    | None -> (errors, [])
  in
  if errors <> [] then (
    List.iter ~f:(print_error format ~oc) shown_errors;
    match
      Errors.format_summary
        format
        errors
        (List.length dropped_errors)
        max_errors
    with
    | Some summary -> Out_channel.output_string oc summary
    | None -> ()
  ) else
    Out_channel.output_string oc "No errors\n";
  Out_channel.close oc

let print_error_list format errors max_errors =
  let (shown_errors, dropped_errors) =
    match max_errors with
    | Some max_errors -> List.split_n errors max_errors
    | None -> (errors, [])
  in
  if errors <> [] then (
    List.iter ~f:(print_error format) shown_errors;
    match
      Errors.format_summary
        format
        errors
        (List.length dropped_errors)
        max_errors
    with
    | Some summary -> Out_channel.output_string stderr summary
    | None -> ()
  ) else
    Printf.printf "No errors\n"

let print_errors format (errors : Errors.t) max_errors : unit =
  print_error_list format (Errors.get_error_list errors) max_errors

let print_errors_if_present (errors : Errors.error list) =
  if not (List.is_empty errors) then (
    let errors_output = Errors.convert_errors_to_string errors in
    Printf.printf "Errors:\n";
    List.iter errors_output (fun err_output ->
        Printf.printf "  %s\n" err_output)
  )

let parse_options () =
  let fn_ref = ref [] in
  let usage = Printf.sprintf "Usage: %s filename\n" Sys.argv.(0) in
  let mode = ref Errors in
  let no_builtins = ref false in
  let line = ref 0 in
  let log_key = ref "" in
  let log_levels = ref SMap.empty in
  let max_errors = ref None in
  let batch_mode = ref false in
  let set_mode x () =
    if !mode <> Errors then
      raise (Arg.Bad "only a single mode should be specified")
    else
      mode := x
  in
  let set_ai x = set_mode (Ai (Ai_options.prepare ~server:false x)) () in
  let error_format = ref Errors.Context in
  let forbid_nullable_cast = ref false in
  let deregister_attributes = ref None in
  let disallow_array_typehint = ref None in
  let disallow_array_literal = ref None in
  let dynamic_view = ref None in
  let auto_namespace_map = ref None in
  let unsafe_rx = ref (Some false) in
  let log_inference_constraints = ref None in
  let timeout = ref None in
  let disallow_invalid_arraykey = ref None in
  let disallow_byref_dynamic_calls = ref (Some false) in
  let disallow_byref_calls = ref (Some false) in
  let set_bool x () = x := Some true in
  let set_bool_ x () = x := true in
  let set_float_ x f = x := f in
  let shallow_class_decl = ref false in
  let out_extension = ref ".out" in
  let like_type_hints = ref false in
  let union_intersection_type_hints = ref false in
  let like_casts = ref false in
  let simple_pessimize = ref 0.0 in
  let complex_coercion = ref false in
  let disable_partially_abstract_typeconsts = ref false in
  let rust_parser_errors = ref false in
  let rust_top_level_elaborator = ref true in
  let symbolindex_file = ref None in
  let check_xhp_attribute = ref false in
  let check_redundant_generics = ref false in
  let disallow_invalid_arraykey_constraint = ref None in
  let enable_class_level_where_clauses = ref false in
  let disable_legacy_soft_typehints = ref false in
  let allow_new_attribute_syntax = ref false in
  let allow_toplevel_requires = ref false in
  let global_inference = ref false in
  let ordered_solving = ref false in
  let reinfer_types = ref [] in
  let const_static_props = ref false in
  let disable_legacy_attribute_syntax = ref false in
  let const_attribute = ref false in
  let disallow_goto = ref false in
  let const_default_func_args = ref false in
  let disallow_silence = ref false in
  let abstract_static_props = ref false in
  let disable_unset_class_const = ref false in
  let glean_service = ref (GleanOptions.service GlobalOptions.default) in
  let glean_hostname = ref (GleanOptions.hostname GlobalOptions.default) in
  let glean_port = ref (GleanOptions.port GlobalOptions.default) in
  let glean_reponame = ref (GleanOptions.reponame GlobalOptions.default) in
  let disallow_func_ptrs_in_constants = ref false in
  let error_php_lambdas = ref false in
  let disallow_discarded_nullable_awaitables = ref false in
  let disable_xhp_element_mangling = ref false in
  let disable_xhp_children_declarations = ref false in
  let enable_xhp_class_modifier = ref false in
  let verbosity = ref 0 in
  let enable_first_class_function_pointers = ref false in
  let disable_modes = ref false in
  let options =
    [
      ("--ai", Arg.String set_ai, " Run the abstract interpreter (Zoncolan)");
      ( "--deregister-attributes",
        Arg.Unit (set_bool deregister_attributes),
        " Ignore all functions with attribute '__PHPStdLib'" );
      ( "--auto-complete",
        Arg.Unit (set_mode Autocomplete),
        " Produce autocomplete suggestions as if triggered by trigger character"
      );
      ( "--auto-complete-manually-invoked",
        Arg.Unit (set_mode Autocomplete_manually_invoked),
        " Produce autocomplete suggestions as if manually triggered by user" );
      ( "--auto-namespace-map",
        Arg.String
          (fun m ->
            auto_namespace_map :=
              Some (ServerConfig.convert_auto_namespace_to_map m)),
        " Alias namespaces" );
      ( "--ffp-auto-complete",
        Arg.Unit (set_mode Ffp_autocomplete),
        " Produce autocomplete suggestions using the full-fidelity parse tree"
      );
      ("--colour", Arg.Unit (set_mode Color), " Produce colour output");
      ("--color", Arg.Unit (set_mode Color), " Produce color output");
      ("--coverage", Arg.Unit (set_mode Coverage), " Produce coverage output");
      ( "--cst-search",
        Arg.Unit (set_mode Cst_search),
        " Search the concrete syntax tree of the given file using the pattern"
        ^ " given on stdin."
        ^ " (The pattern is a JSON object adhering to the search DSL.)" );
      ( "--dump-symbol-info",
        Arg.Unit (set_mode Dump_symbol_info),
        " Dump all symbol information" );
      ( "--error-format",
        Arg.String
          (fun s ->
            match s with
            | "raw" -> error_format := Errors.Raw
            | "context" -> error_format := Errors.Context
            | _ -> print_string "Warning: unrecognized error format.\n"),
        "<raw|context> Error formatting style" );
      ("--lint", Arg.Unit (set_mode Lint), " Produce lint errors");
      ( "--no-builtins",
        Arg.Set no_builtins,
        " Don't use builtins (e.g. ConstSet)" );
      ( "--out-extension",
        Arg.String (fun s -> out_extension := s),
        "output file extension (default .out)" );
      ("--dump-deps", Arg.Unit (set_mode Dump_deps), " Print dependencies");
      ( "--dump-glean-deps",
        Arg.Unit (set_mode Dump_glean_deps),
        " Print dependencies in the Glean format" );
      ( "--dump-inheritance",
        Arg.Unit (set_mode Dump_inheritance),
        " Print inheritance" );
      ( "--identify-symbol",
        Arg.Tuple
          [
            Arg.Int (fun x -> line := x);
            Arg.Int
              (fun column -> set_mode (Identify_symbol (!line, column)) ());
          ],
        "<pos> Show info about symbol at given line and column" );
      ( "--find-local",
        Arg.Tuple
          [
            Arg.Int (fun x -> line := x);
            Arg.Int (fun column -> set_mode (Find_local (!line, column)) ());
          ],
        "<pos> Find all usages of local at given line and column" );
      ( "--max-errors",
        Arg.Int (fun num_errors -> max_errors := Some num_errors),
        " Maximum number of errors to display" );
      ("--outline", Arg.Unit (set_mode Outline), " Print file outline");
      ("--nast", Arg.Unit (set_mode Dump_nast), " Print out the named AST");
      ("--tast", Arg.Unit (set_mode Dump_tast), " Print out the typed AST");
      ("--tast-check", Arg.Unit (set_mode Check_tast), " Typecheck the tast");
      ( "--stripped-tast",
        Arg.Unit (set_mode Dump_stripped_tast),
        " Print out the typed AST, stripped of type information."
        ^ " This can be compared against the named AST to look for holes." );
      ( "--rewrite",
        Arg.Unit (set_mode Rewrite),
        " Rewrite the file after inferring types using global inference"
        ^ " (requires --global-inference)." );
      ( "--global-inference",
        Arg.Set global_inference,
        "Global type inference to infer missing type annotations." );
      ( "--ordered-solving",
        Arg.Set ordered_solving,
        "Optimized solver for type variables. Experimental." );
      ( "--reinfer-types",
        Arg.String (fun s -> reinfer_types := Str.split (Str.regexp ", *") s),
        "List of type hint to be ignored and infered again using global inference."
      );
      ( "--find-refs",
        Arg.Tuple
          [
            Arg.Int (fun x -> line := x);
            Arg.Int (fun column -> set_mode (Find_refs (!line, column)) ());
          ],
        "<pos> Find all usages of a symbol at given line and column" );
      ( "--go-to-impl",
        Arg.Tuple
          [
            Arg.Int (fun x -> line := x);
            Arg.Int (fun column -> set_mode (Go_to_impl (!line, column)) ());
          ],
        "<pos> Find all implementations of a symbol at given line and column" );
      ( "--highlight-refs",
        Arg.Tuple
          [
            Arg.Int (fun x -> line := x);
            Arg.Int (fun column -> set_mode (Highlight_refs (!line, column)) ());
          ],
        "<pos> Highlight all usages of a symbol at given line and column" );
      ( "--decl-compare",
        Arg.Unit (set_mode Decl_compare),
        " Test comparison functions used in incremental mode on declarations"
        ^ " in provided file" );
      ( "--shallow-class-diff",
        Arg.Unit (set_mode Shallow_class_diff),
        " Test shallow class comparison used in incremental mode on shallow class declarations"
      );
      ( "--forbid_nullable_cast",
        Arg.Set forbid_nullable_cast,
        " Forbid casting from nullable values." );
      ( "--disallow-array-typehint",
        Arg.Unit (set_bool disallow_array_typehint),
        " Disallow usage of array typehints." );
      ( "--disallow-array-literal",
        Arg.Unit (set_bool disallow_array_literal),
        " Disallow usage of array literals." );
      ( "--dynamic-view",
        Arg.Unit (set_bool dynamic_view),
        " Turns on dynamic view, replacing Tany with dynamic" );
      ( "--unsafe-rx",
        Arg.Unit (set_bool unsafe_rx),
        " Disables reactivity related errors" );
      ( "--mro",
        Arg.Unit (set_mode Linearization),
        " Grabs the linearization of all classes in a file." );
      ( "--log-inference-constraints",
        Arg.Unit (set_bool log_inference_constraints),
        " Log inference constraints to Scuba." );
      ( "--timeout",
        Arg.Int (fun secs -> timeout := Some secs),
        " Timeout in seconds for checking a function or a class." );
      ( "--hh-log-level",
        Arg.Tuple
          [
            Arg.String (fun x -> log_key := x);
            Arg.Int
              (fun level -> log_levels := SMap.add !log_key level !log_levels);
          ],
        " Set the log level for a key" );
      ( "--batch-files",
        Arg.Set batch_mode,
        " Typecheck each file passed in independently" );
      ( "--disallow-invalid-arraykey",
        Arg.Unit (set_bool disallow_invalid_arraykey),
        " Disallow using values that get casted to arraykey at runtime as array keys"
      );
      ( "--disallow-invalid-arraykey-constraint",
        Arg.Unit (set_bool disallow_invalid_arraykey_constraint),
        " Disallow using non-string, non-int types as array key constraints" );
      ( "--check-xhp-attribute",
        Arg.Set check_xhp_attribute,
        " Typechecks xhp required attributes" );
      ( "--disallow-byref-dynamic-calls",
        Arg.Unit (set_bool disallow_byref_dynamic_calls),
        " Disallow passing arguments by reference to dynamically called functions [e.g. $foo(&$bar)]"
      );
      ( "--disallow-byref-calls",
        Arg.Unit (set_bool disallow_byref_calls),
        " Disallow passing arguments by reference in any form [e.g. foo(&$bar)]"
      );
      ( "--shallow-class-decl",
        Arg.Set shallow_class_decl,
        " Look up class members lazily from shallow declarations" );
      ( "--union-intersection-type-hints",
        Arg.Set union_intersection_type_hints,
        " Allows union and intersection types to be written in type hint positions"
      );
      ( "--like-type-hints",
        Arg.Set like_type_hints,
        " Allows like types to be written in type hint positions" );
      ( "--like-casts",
        Arg.Set like_casts,
        " Allows like types to be written in as expressions" );
      ( "--simple-pessimize",
        Arg.Set_float simple_pessimize,
        " At coercion points, if a type is not enforceable, wrap it in like. Float argument 0.0 to 1.0 sets frequency"
      );
      ( "--complex-coercion",
        Arg.Set complex_coercion,
        " Allows complex coercions that involve like types" );
      ( "--like-types-all",
        Arg.Unit
          (fun () ->
            set_bool_ like_type_hints ();
            set_bool_ like_casts ();
            set_float_ simple_pessimize 1.0;
            set_bool_ complex_coercion ()),
        " Enables all like types features" );
      ( "--disable-partially-abstract-typeconsts",
        Arg.Set disable_partially_abstract_typeconsts,
        " Treat partially abstract type constants as concrete type constants" );
      ( "--rust-parser-errors",
        Arg.Bool (fun x -> rust_parser_errors := x),
        " Use rust parser error checker" );
      ( "--rust-lowerer",
        Arg.Bool (fun x -> rust_top_level_elaborator := x),
        " Use rust lowerer" );
      ( "--symbolindex-file",
        Arg.String (fun str -> symbolindex_file := Some str),
        " Load the symbol index from this file" );
      ( "--enable-class-level-where-clauses",
        Arg.Set enable_class_level_where_clauses,
        "Enables support for class-level where clauses" );
      ( "--disable-legacy-soft-typehints",
        Arg.Set disable_legacy_soft_typehints,
        " Disables the legacy @ syntax for soft typehints (use __Soft instead)"
      );
      ( "--allow-new-attribute-syntax",
        Arg.Set allow_new_attribute_syntax,
        " Allow the new @ attribute syntax (disables legacy soft typehints)" );
      ( "--allow-toplevel-requires",
        Arg.Set allow_toplevel_requires,
        " Allow `require()` and similar at the top-level" );
      ( "--const-static-props",
        Arg.Set const_static_props,
        " Enable static properties to be const" );
      ( "--disable-legacy-attribute-syntax",
        Arg.Set disable_legacy_attribute_syntax,
        " Disable the legacy <<...>> user attribute syntax" );
      ("--const-attribute", Arg.Set const_attribute, " Allow __Const attribute");
      ( "--disallow-goto",
        Arg.Set disallow_goto,
        " Forbid the goto operator and goto labels in the parser" );
      ( "--const-default-func-args",
        Arg.Set const_default_func_args,
        " Statically check default function arguments are constant initializers"
      );
      ( "--disallow-silence",
        Arg.Set disallow_silence,
        " Disallow the error suppression operator, @" );
      ( "--abstract-static-props",
        Arg.Set abstract_static_props,
        " Static properties can be abstract" );
      ( "--disable-unset-class-const",
        Arg.Set disable_unset_class_const,
        " Make unsetting a class const a parse error" );
      ( "--glean-service",
        Arg.String (fun str -> glean_service := str),
        " glean service name" );
      ( "--glean-hostname",
        Arg.String (fun str -> glean_hostname := str),
        " glean hostname" );
      ("--glean-port", Arg.Int (fun x -> glean_port := x), " glean port number");
      ( "--glean-reponame",
        Arg.String (fun str -> glean_reponame := str),
        "glean repo name" );
      ( "--disallow-func-ptrs-in-constants",
        Arg.Set disallow_func_ptrs_in_constants,
        " Disallow use of HH\\fun and HH\\class_meth in constants and constant initializers"
      );
      ( "--disallow-php-lambdas",
        Arg.Set error_php_lambdas,
        "Disallow php style anonymous functions." );
      ( "--disallow-discarded-nullable-awaitables",
        Arg.Set disallow_discarded_nullable_awaitables,
        "Error on using discarded nullable awaitables" );
      ( "--disable-xhp-element-mangling",
        Arg.Set disable_xhp_element_mangling,
        "Disable mangling of XHP elements :foo. That is, :foo:bar is now \\foo\\bar, not xhp_foo__bar"
      );
      ( "--disable-xhp-children-declarations",
        Arg.Set disable_xhp_children_declarations,
        "Disable XHP children declarations, e.g. children (foo, bar+)" );
      ( "--enable-xhp-class-modifier",
        Arg.Set enable_xhp_class_modifier,
        "Enable the XHP class modifier, xhp class name {} will define an xhp class."
      );
      ( "--verbose",
        Arg.Int (fun v -> verbosity := v),
        "Verbosity as an integer." );
      ( "--enable-first-class-function-pointers",
        Arg.Set enable_first_class_function_pointers,
        "Enable first class funciton pointers using <> syntax" );
      ("--disable-modes", Arg.Set disable_modes, "Treat partial as strict");
    ]
  in
  let options = Arg.align ~limit:25 options in
  Arg.parse options (fun fn -> fn_ref := fn :: !fn_ref) usage;
  let fns =
    match !fn_ref with
    | [] -> die usage
    | x -> x
  in
  let tcopt =
    GlobalOptions.make
      ?tco_unsafe_rx:!unsafe_rx
      ?po_deregister_php_stdlib:!deregister_attributes
      ?tco_disallow_array_typehint:!disallow_array_typehint
      ?tco_disallow_array_literal:!disallow_array_literal
      ?tco_dynamic_view:!dynamic_view
      ?tco_log_inference_constraints:!log_inference_constraints
      ?tco_timeout:!timeout
      ?tco_disallow_invalid_arraykey:!disallow_invalid_arraykey
      ?po_auto_namespace_map:!auto_namespace_map
      ?tco_disallow_byref_dynamic_calls:!disallow_byref_dynamic_calls
      ?tco_disallow_byref_calls:!disallow_byref_calls
      ?tco_disallow_invalid_arraykey_constraint:
        !disallow_invalid_arraykey_constraint
      ~tco_check_xhp_attribute:!check_xhp_attribute
      ~tco_check_redundant_generics:!check_redundant_generics
      ~tco_shallow_class_decl:!shallow_class_decl
      ~tco_like_type_hints:!like_type_hints
      ~tco_union_intersection_type_hints:!union_intersection_type_hints
      ~tco_like_casts:!like_casts
      ~tco_simple_pessimize:!simple_pessimize
      ~tco_complex_coercion:!complex_coercion
      ~tco_disable_partially_abstract_typeconsts:
        !disable_partially_abstract_typeconsts
      ~log_levels:!log_levels
      ~po_rust_parser_errors:!rust_parser_errors
      ~po_rust_top_level_elaborator:!rust_top_level_elaborator
      ~po_enable_class_level_where_clauses:!enable_class_level_where_clauses
      ~po_disable_legacy_soft_typehints:!disable_legacy_soft_typehints
      ~po_allow_new_attribute_syntax:!allow_new_attribute_syntax
      ~po_disallow_toplevel_requires:(not !allow_toplevel_requires)
      ~tco_const_static_props:!const_static_props
      ~tco_global_inference:!global_inference
      ~tco_ordered_solving:!ordered_solving
      ~tco_gi_reinfer_types:!reinfer_types
      ~po_disable_legacy_attribute_syntax:!disable_legacy_attribute_syntax
      ~tco_const_attribute:!const_attribute
      ~po_allow_goto:(not !disallow_goto)
      ~po_const_default_func_args:!const_default_func_args
      ~po_disallow_silence:!disallow_silence
      ~po_abstract_static_props:!abstract_static_props
      ~po_disable_unset_class_const:!disable_unset_class_const
      ~po_disallow_func_ptrs_in_constants:!disallow_func_ptrs_in_constants
      ~tco_check_attribute_locations:true
      ~tco_error_php_lambdas:!error_php_lambdas
      ~tco_disallow_discarded_nullable_awaitables:
        !disallow_discarded_nullable_awaitables
      ~glean_service:!glean_service
      ~glean_hostname:!glean_hostname
      ~glean_port:!glean_port
      ~glean_reponame:!glean_reponame
      ~po_disable_xhp_element_mangling:!disable_xhp_element_mangling
      ~po_disable_xhp_children_declarations:!disable_xhp_children_declarations
      ~po_enable_xhp_class_modifier:!enable_xhp_class_modifier
      ~po_enable_first_class_function_pointers:
        !enable_first_class_function_pointers
      ~po_disable_modes:!disable_modes
      ()
  in
  let tcopt =
    {
      tcopt with
      GlobalOptions.tco_experimental_features =
        SSet.filter
          begin
            fun x ->
            if x = GlobalOptions.tco_experimental_forbid_nullable_cast then
              !forbid_nullable_cast
            else
              true
          end
          tcopt.GlobalOptions.tco_experimental_features;
    }
  in
  (* Configure symbol index settings *)
  let namespace_map = GlobalOptions.po_auto_namespace_map tcopt in
  let sienv =
    SymbolIndex.initialize
      ~globalrev:None
      ~gleanopt:tcopt
      ~namespace_map
      ~provider_name:"LocalIndex"
      ~quiet:true
      ~ignore_hh_version:false
      ~savedstate_file_opt:!symbolindex_file
      ~workers:None
  in
  let sienv =
    {
      sienv with
      SearchUtils.sie_resolve_signatures = true;
      SearchUtils.sie_resolve_positions = true;
      SearchUtils.sie_resolve_local_decl = true;
    }
  in
  ( {
      files = fns;
      mode = !mode;
      no_builtins = !no_builtins;
      max_errors = !max_errors;
      error_format = !error_format;
      tcopt;
      batch_mode = !batch_mode;
      out_extension = !out_extension;
      verbosity = !verbosity;
    },
    sienv )

(* Make readable test output *)
let replace_color input =
  Ide_api_types.(
    match input with
    | (Some Unchecked, str) -> "<unchecked>" ^ str ^ "</unchecked>"
    | (Some Checked, str) -> "<checked>" ^ str ^ "</checked>"
    | (Some Partial, str) -> "<partial>" ^ str ^ "</partial>"
    | (None, str) -> str)

let print_colored fn type_acc =
  let content = cat (Relative_path.to_absolute fn) in
  let results = ColorFile.go content type_acc in
  if Unix.isatty Unix.stdout then
    Tty.cprint (ClientColorFile.replace_colors results)
  else
    print_string (List.map ~f:replace_color results |> String.concat ~sep:"")

let print_coverage type_acc =
  ClientCoverageMetric.go ~json:false (Some (Coverage_level_defs.Leaf type_acc))

let print_global_inference_envs ctx ~verbosity gienvs =
  let gienvs =
    Typing_global_inference.StateSubConstraintGraphs.global_tvenvs gienvs
  in
  let tco_global_inference =
    TypecheckerOptions.global_inference (Provider_context.get_tcopt ctx)
  in
  if verbosity >= 2 && tco_global_inference then
    let should_log (pos, gienv) =
      let file_relevant =
        match verbosity with
        | 1
          when Filename.check_suffix
                 (Relative_path.suffix (Pos.filename pos))
                 ".hhi" ->
          false
        | _ -> true
      in
      file_relevant && (not @@ List.is_empty @@ Inf.get_vars_g gienv)
    in
    let env = Typing_env.empty ctx Relative_path.default ~droot:None in

    List.filter gienvs ~f:should_log
    |> List.iter ~f:(fun (pos, gienv) ->
           Typing_log.log_global_inference_env pos env gienv)

let merge_global_inference_envs_opt ctx gienvs :
    Typing_global_inference.StateConstraintGraph.t option =
  if TypecheckerOptions.global_inference (Provider_context.get_tcopt ctx) then
    let open Typing_global_inference in
    let (type_map, env, state_errors) =
      StateConstraintGraph.merge_subgraphs ctx [gienvs]
    in
    (* we are not going to print type variables without any bounds *)
    let env = { env with inference_env = Inf.compress env.inference_env } in
    Some (type_map, env, state_errors)
  else
    None

let print_global_inference_env
    env ~step_name state_errors error_format max_errors =
  let print_header s =
    print_endline "";
    print_endline (String.map s (const '='));
    print_endline s;
    print_endline (String.map s (const '='))
  in
  print_header (Printf.sprintf "%sd environment" step_name);
  Typing_log.hh_show_full_env Pos.none env;

  print_header (Printf.sprintf "%s errors" step_name);
  List.iter
    (Typing_global_inference.StateErrors.elements state_errors)
    ~f:(fun (var, errl) ->
      Printf.fprintf stderr "#%d\n" var;
      print_error_list error_format errl max_errors);
  Out_channel.flush stderr

let print_merged_global_inference_env
    ~verbosity
    (gienv : Typing_global_inference.StateConstraintGraph.t option)
    error_format
    max_errors =
  if verbosity >= 1 then
    match gienv with
    | None -> ()
    | Some (_type_map, gienv, state_errors) ->
      print_global_inference_env
        gienv
        ~step_name:"Merge"
        state_errors
        error_format
        max_errors

let print_solved_global_inference_env
    ~verbosity
    (gienv : Typing_global_inference.StateSolvedGraph.t option)
    error_format
    max_errors =
  if verbosity >= 1 then
    match gienv with
    | None -> ()
    | Some (gienv, state_errors, _type_map) ->
      print_global_inference_env
        gienv
        ~step_name:"Solve"
        state_errors
        error_format
        max_errors

let solve_global_inference_env
    (gienv : Typing_global_inference.StateConstraintGraph.t) :
    Typing_global_inference.StateSolvedGraph.t =
  Typing_global_inference.StateSolvedGraph.from_constraint_graph gienv

let global_inference_merge_and_solve
    ~verbosity ?(error_format = Errors.Raw) ?max_errors ctx gienvs =
  print_global_inference_envs ctx ~verbosity gienvs;
  let gienv = merge_global_inference_envs_opt ctx gienvs in
  print_merged_global_inference_env ~verbosity gienv error_format max_errors;
  let gienv = Option.map gienv solve_global_inference_env in
  print_solved_global_inference_env ~verbosity gienv error_format max_errors;
  gienv

let check_file ctx ~verbosity errors files_info error_format max_errors =
  let (errors, tasts, genvs) =
    Relative_path.Map.fold
      files_info
      ~f:
        begin
          fun fn fileinfo (errors, tasts, genvs) ->
          let (new_tasts, new_genvs, new_errors) =
            Typing_check_utils.type_file_with_global_tvenvs ctx fn fileinfo
          in
          ( errors @ Errors.get_error_list new_errors,
            new_tasts @ tasts,
            Lazy.force new_genvs @ genvs )
        end
      ~init:(errors, [], [])
  in
  let gienvs =
    Typing_global_inference.StateSubConstraintGraphs.build ctx tasts genvs
  in
  let _gienv =
    global_inference_merge_and_solve
      ctx
      ~verbosity:(verbosity + 1)
      gienvs
      ~error_format
      ?max_errors
  in
  errors

let create_nasts ctx files_info =
  let build_nast fn _ =
    let ast = Ast_provider.get_ast ~full:true ctx fn in
    Naming.program ctx ast
  in
  Relative_path.Map.mapi ~f:build_nast files_info

let parse_and_name ctx files_contents =
  let parsed_files =
    Relative_path.Map.mapi files_contents ~f:(fun fn contents ->
        Errors.run_in_context fn Errors.Parsing (fun () ->
            let popt = Provider_context.get_tcopt ctx in
            let parsed_file =
              Full_fidelity_ast.defensive_program popt fn contents
            in
            let ast =
              let { Parser_return.ast; _ } = parsed_file in
              if ParserOptions.deregister_php_stdlib popt then
                Nast.deregister_ignored_attributes ast
              else
                ast
            in
            Ast_provider.provide_ast_hint fn ast Ast_provider.Full;
            { parsed_file with Parser_return.ast }))
  in
  let files_info =
    Relative_path.Map.mapi
      ~f:
        begin
          fun _fn parsed_file ->
          let { Parser_return.file_mode; comments; ast; _ } = parsed_file in
          (* If the feature is turned on, deregister functions with attribute
             __PHPStdLib. This does it for all functions, not just hhi files *)
          let (funs, classes, record_defs, typedefs, consts) =
            Nast.get_defs ast
          in
          {
            FileInfo.file_mode;
            funs;
            classes;
            record_defs;
            typedefs;
            consts;
            comments = Some comments;
            hash = None;
          }
        end
      parsed_files
  in
  Relative_path.Map.iter files_info (fun fn fileinfo ->
      Errors.run_in_context fn Errors.Naming (fun () ->
          let { FileInfo.funs; classes; record_defs; typedefs; consts; _ } =
            fileinfo
          in
          Naming_global.make_env
            ctx
            ~funs
            ~classes
            ~record_defs
            ~typedefs
            ~consts));
  (parsed_files, files_info)

let parse_name_and_decl ctx files_contents =
  Errors.do_ (fun () ->
      let (parsed_files, files_info) = parse_and_name ctx files_contents in
      Relative_path.Map.iter parsed_files (fun fn parsed_file ->
          Errors.run_in_context fn Errors.Decl (fun () ->
              Decl.name_and_declare_types_program
                ~sh:SharedMem.Uses
                ctx
                parsed_file.Parser_return.ast));

      files_info)

let parse_name_and_shallow_decl ctx filename file_contents :
    Shallow_decl_defs.shallow_class SMap.t =
  Errors.ignore_ (fun () ->
      let files_contents = Relative_path.Map.singleton filename file_contents in
      let (parsed_files, _) = parse_and_name ctx files_contents in
      let parsed_file = Relative_path.Map.values parsed_files |> List.hd_exn in
      parsed_file.Parser_return.ast
      |> List.filter_map ~f:(function
             | Aast.Class c -> Some (Shallow_decl.class_ ctx c)
             | _ -> None)
      |> List.fold ~init:SMap.empty ~f:(fun acc c ->
             SMap.add (snd c.Shallow_decl_defs.sc_name) c acc))

let test_shallow_class_diff popt filename =
  let filename_after = Relative_path.to_absolute filename ^ ".after" in
  let contents1 = Sys_utils.cat (Relative_path.to_absolute filename) in
  let contents2 = Sys_utils.cat filename_after in
  let decls1 = parse_name_and_shallow_decl popt filename contents1 in
  let decls2 = parse_name_and_shallow_decl popt filename contents2 in
  let decls =
    SMap.merge (fun _ a b -> Some (a, b)) decls1 decls2 |> SMap.bindings
  in
  let diffs =
    List.map decls (fun (cid, old_and_new) ->
        ( Utils.strip_ns cid,
          match old_and_new with
          | (Some c1, Some c2) -> Shallow_class_diff.diff_class c1 c2
          | _ -> ClassDiff.Major_change ))
  in
  List.iter diffs (fun (cid, diff) ->
      Format.printf "%s: %a@." cid ClassDiff.pp diff)

let add_newline contents =
  (* this is used for incremental mode to change all the positions, so we
     basically want a prepend; there's a few cases we need to handle:
     - empty file
     - header line: apppend after header
     - shebang and header: append after header
     - shebang only, no header (e.g. .hack file): append after shebang
     - no header or shebang (e.g. .hack file): prepend
  *)
  let after_shebang =
    if string_starts_with contents "#!" then
      String.index_exn contents '\n' + 1
    else
      0
  in
  let after_header =
    if
      String.length contents > after_shebang + 2
      && String.sub contents after_shebang 2 = "<?"
    then
      String.index_from_exn contents after_shebang '\n' + 1
    else
      after_shebang
  in
  String.sub contents 0 after_header
  ^ "\n"
  ^ String.sub contents after_header (String.length contents - after_header)

let get_decls defs =
  ( SSet.fold
      (fun x acc -> Decl_heap.Typedefs.find_unsafe x :: acc)
      defs.FileInfo.n_types
      [],
    SSet.fold
      (fun x acc -> Decl_heap.Funs.find_unsafe x :: acc)
      defs.FileInfo.n_funs
      [],
    SSet.fold
      (fun x acc -> Decl_heap.Classes.find_unsafe x :: acc)
      defs.FileInfo.n_classes
      [] )

let fail_comparison s =
  raise
    (Failure
       ( Printf.sprintf "Comparing %s failed!\n" s
       ^ "It's likely that you added new positions to decl types "
       ^ "without updating Decl_pos_utils.NormalizeSig\n" ))

let compare_typedefs t1 t2 =
  let t1 = Decl_pos_utils.NormalizeSig.typedef t1 in
  let t2 = Decl_pos_utils.NormalizeSig.typedef t2 in
  if t1 <> t2 then fail_comparison "typedefs"

let compare_funs f1 f2 =
  let f1 = Decl_pos_utils.NormalizeSig.fun_elt f1 in
  let f2 = Decl_pos_utils.NormalizeSig.fun_elt f2 in
  if f1 <> f2 then fail_comparison "funs"

let compare_classes c1 c2 =
  if Decl_compare.class_big_diff c1 c2 then fail_comparison "class_big_diff";

  let c1 = Decl_pos_utils.NormalizeSig.class_type c1 in
  let c2 = Decl_pos_utils.NormalizeSig.class_type c2 in
  let (_, is_unchanged) =
    Decl_compare.ClassDiff.compare c1.Decl_defs.dc_name c1 c2
  in
  if not is_unchanged then fail_comparison "ClassDiff";

  let (_, is_unchanged) = Decl_compare.ClassEltDiff.compare c1 c2 in
  if is_unchanged = `Changed then fail_comparison "ClassEltDiff"

let test_decl_compare ctx filenames builtins files_contents files_info =
  (* skip some edge cases that we don't handle now... ugly! *)
  if Relative_path.suffix filenames = "capitalization3.php" then
    ()
  else if Relative_path.suffix filenames = "capitalization4.php" then
    ()
  else
    (* do not analyze builtins over and over *)
    let files_info =
      Relative_path.Map.fold
        builtins
        ~f:
          begin
            fun k _ acc ->
            Relative_path.Map.remove acc k
          end
        ~init:files_info
    in
    let files =
      Relative_path.Map.fold
        files_info
        ~f:(fun k _ acc -> Relative_path.Set.add acc k)
        ~init:Relative_path.Set.empty
    in
    let defs =
      Relative_path.Map.fold
        files_info
        ~f:
          begin
            fun _ names1 names2 ->
            FileInfo.(merge_names (simplify names1) names2)
          end
        ~init:FileInfo.empty_names
    in
    let (typedefs1, funs1, classes1) = get_decls defs in
    (* For the purpose of this test, we can ignore other heaps *)
    Ast_provider.remove_batch files;

    let get_classes path =
      match Relative_path.Map.find_opt files_info path with
      | None -> SSet.empty
      | Some info -> SSet.of_list @@ List.map info.FileInfo.classes snd
    in
    (* We need to oldify, not remove, for ClassEltDiff to work *)
    Decl_redecl_service.oldify_type_decl
      ctx
      None
      get_classes
      ~bucket_size:1
      ~previously_oldified_defs:FileInfo.empty_names
      ~defs
      ~collect_garbage:false;

    let files_contents = Relative_path.Map.map files_contents ~f:add_newline in
    let (_, _) = parse_name_and_decl ctx files_contents in
    let (typedefs2, funs2, classes2) = get_decls defs in
    List.iter2_exn typedefs1 typedefs2 compare_typedefs;
    List.iter2_exn funs1 funs2 compare_funs;
    List.iter2_exn classes1 classes2 compare_classes;
    ()

(* Returns a list of Tast defs, along with associated type environments. *)
let compute_tasts ctx files_info interesting_files :
    Errors.t
    * ( Tast.program Relative_path.Map.t
      * Typing_inference_env.t_global_with_pos list ) =
  let _f _k nast x =
    match (nast, x) with
    | (Some nast, Some _) -> Some nast
    | _ -> None
  in
  Errors.do_ (fun () ->
      let nasts = create_nasts ctx files_info in
      (* Interesting files are usually the non hhi ones. *)
      let filter_non_interesting nasts =
        Relative_path.Map.merge nasts interesting_files (fun _k nast x ->
            match (nast, x) with
            | (Some nast, Some _) -> Some nast
            | _ -> None)
      in
      let nasts = filter_non_interesting nasts in
      let tasts_envs =
        Relative_path.Map.map
          nasts
          ~f:(Typing_toplevel.nast_to_tast_gienv ~do_tast_checks:true ctx)
      in
      let tasts = Relative_path.Map.map tasts_envs ~f:fst in
      let genvs =
        List.concat
        @@ Relative_path.Map.values
        @@ Relative_path.Map.map tasts_envs ~f:snd
      in
      (tasts, genvs))

let merge_global_inference_env_in_tast gienv tast =
  let env_merger =
    object
      inherit Tast_visitor.endo

      method! on_'en _ env =
        {
          env with
          Tast.inference_env =
            Typing_inference_env.simple_merge
              env.Tast.inference_env
              gienv.inference_env;
        }
    end
  in
  env_merger#go tast

(**
 * Compute TASTs for some files, then expand all type variables.
 *)
let compute_tasts_expand_types ctx ~verbosity files_info interesting_files =
  let (errors, (tasts, gienvs)) =
    compute_tasts ctx files_info interesting_files
  in
  let subconstraints =
    Typing_global_inference.StateSubConstraintGraphs.build
      ctx
      (List.concat (Relative_path.Map.values tasts))
      gienvs
  in
  let (tasts, gi_solved) =
    match global_inference_merge_and_solve ctx ~verbosity subconstraints with
    | None -> (tasts, None)
    | Some ((gienv, _, _) as gi_solved) ->
      let tasts =
        Relative_path.Map.map
          tasts
          (merge_global_inference_env_in_tast gienv ctx)
      in
      (tasts, Some gi_solved)
  in
  let tasts = Relative_path.Map.map tasts (Tast_expand.expand_program ctx) in
  (errors, tasts, gi_solved)

let print_tasts tasts ctx =
  let print_tast = Typing_ast_print.print_tast ctx in
  Relative_path.Map.iter tasts (fun _k (tast : Tast.program) -> print_tast tast)

let typecheck_tasts tasts tcopt (filename : Relative_path.t) =
  let env = Typing_env.empty tcopt filename ~droot:None in
  let tasts = Relative_path.Map.values tasts in
  let typecheck_tast tast =
    Errors.get_error_list (Tast_typecheck.check env tast)
  in
  List.concat_map tasts ~f:typecheck_tast

let pp_debug_deps fmt entries =
  Format.fprintf fmt "@[<v>";
  ignore
  @@ List.fold_left entries ~init:false ~f:(fun sep (obj, roots) ->
         if sep then Format.fprintf fmt "@;";
         Format.fprintf fmt "%s -> " obj;
         Format.fprintf fmt "@[<hv>";
         ignore
         @@ List.fold_left roots ~init:false ~f:(fun sep root ->
                if sep then Format.fprintf fmt ",@ ";
                Format.pp_print_string fmt root;
                true);
         Format.fprintf fmt "@]";
         true);
  Format.fprintf fmt "@]"

let show_debug_deps = Format.asprintf "%a" pp_debug_deps

let sort_debug_deps deps =
  Hashtbl.fold deps ~init:[] ~f:(fun ~key:obj ~data:set acc ->
      (obj, set) :: acc)
  |> List.sort ~compare:(fun (a, _) (b, _) -> String.compare a b)
  |> List.map ~f:(fun (obj, roots) ->
         let roots =
           HashSet.fold roots ~init:[] ~f:List.cons
           |> List.sort ~compare:String.compare
         in
         (obj, roots))

(* Note: this prints dependency graph edges in the same direction as the mapping
   which is actually stored in the shared memory table. The line "X -> Y" can be
   read, "X is used by Y", or "X is a dependency of Y", or "when X changes, Y
   must be rechecked". *)
let dump_debug_deps dbg_deps =
  dbg_deps |> sort_debug_deps |> show_debug_deps |> Printf.printf "%s\n"

let dump_debug_glean_deps
    (deps :
      ( Typing_deps.Dep.dependency Typing_deps.Dep.variant
      * Typing_deps.Dep.dependent Typing_deps.Dep.variant )
      HashSet.t) =
  let json_opt = Glean_dependency_graph.convert_deps_to_json ~deps in
  match json_opt with
  | Some json_obj ->
    Printf.printf "%s\n" (Hh_json.json_to_string ~pretty:true json_obj)
  | None -> Printf.printf "No dependencies\n"

let scan_files_for_symbol_index
    (filename : Relative_path.t)
    (sienv : SearchUtils.si_env)
    (ctx : Provider_context.t) : SearchUtils.si_env =
  let files_contents = Multifile.file_to_files filename in
  let (_, individual_file_info) = parse_name_and_decl ctx files_contents in
  let fileinfo_list = Relative_path.Map.values individual_file_info in
  let transformed_list =
    List.map fileinfo_list ~f:(fun fileinfo ->
        (filename, SearchUtils.Full fileinfo, SearchUtils.TypeChecker))
  in
  SymbolIndex.update_files ~ctx ~sienv ~paths:transformed_list

let handle_mode
    mode
    filenames
    ctx
    builtins
    files_contents
    files_info
    parse_errors
    max_errors
    error_format
    batch_mode
    out_extension
    dbg_deps
    dbg_glean_deps
    ~verbosity
    (sienv : SearchUtils.si_env) =
  let expect_single_file () : Relative_path.t =
    match filenames with
    | [x] -> x
    | _ -> die "Only single file expected"
  in
  let iter_over_files f : unit = List.iter filenames f in
  match mode with
  | Ai ai_options ->
    if parse_errors <> [] then
      List.iter ~f:(print_error error_format) parse_errors
    else
      let to_check =
        Relative_path.Map.filter files_info ~f:(fun _p i ->
            let open FileInfo in
            match i.file_mode with
            | None
            | Some Mstrict ->
              true
            | _ -> false)
      in
      let errors =
        check_file ~verbosity ctx [] to_check error_format max_errors
      in
      if errors <> [] then
        List.iter ~f:(print_error error_format) errors
      else
        Ai.do_ files_info ai_options ctx
  | Autocomplete
  | Autocomplete_manually_invoked ->
    let path = expect_single_file () in
    let contents = cat (Relative_path.to_absolute path) in
    (* Search backwards: there should only be one /real/ case. If there's multiple, *)
    (* guess that the others are preceding explanation comments *)
    let offset =
      Str.search_backward
        (Str.regexp AutocompleteTypes.autocomplete_token)
        contents
        (String.length contents)
    in
    let pos = File_content.offset_to_position contents offset in
    let is_manually_invoked = mode = Autocomplete_manually_invoked in
    let (ctx, entry) =
      Provider_context.add_entry_from_file_contents ~ctx ~path ~contents
    in
    let autocomplete_context =
      ServerAutoComplete.get_autocomplete_context
        ~file_content:contents
        ~pos
        ~is_manually_invoked
    in
    let sienv = scan_files_for_symbol_index path sienv ctx in
    let result =
      ServerAutoComplete.go_at_auto332_ctx
        ~ctx
        ~entry
        ~sienv
        ~autocomplete_context
    in
    List.iter
      ~f:
        begin
          fun r ->
          AutocompleteTypes.(Printf.printf "%s %s\n" r.res_name r.res_ty)
        end
      result.Utils.With_complete_flag.value
  | Ffp_autocomplete ->
    iter_over_files (fun path ->
        try
          let sienv = scan_files_for_symbol_index path sienv ctx in
          let (ctx, entry) = Provider_context.add_entry ~ctx ~path in
          (* TODO: Use a magic word/symbol to identify autocomplete location instead *)
          let args_regex = Str.regexp "AUTOCOMPLETE [1-9][0-9]* [1-9][0-9]*" in
          let position =
            try
              let file_text = entry.Provider_context.contents in
              let _ = Str.search_forward args_regex file_text 0 in
              let raw_flags = Str.matched_string file_text in
              match split ' ' raw_flags with
              | [_; row; column] ->
                { line = int_of_string row; column = int_of_string column }
              | _ -> failwith "Invalid test file: no flags found"
            with Caml.Not_found ->
              failwith "Invalid test file: no flags found"
          in
          let result =
            FfpAutocompleteService.auto_complete
              ctx
              entry
              position
              ~filter_by_token:true
              ~sienv
          in
          match result with
          | [] -> Printf.printf "No result found\n"
          | res ->
            List.iter res ~f:(fun r ->
                AutocompleteTypes.(Printf.printf "%s\n" r.res_name))
        with
        | Failure msg
        | Invalid_argument msg ->
          Printf.printf "%s\n" msg;
          exit 1)
  | Color ->
    Relative_path.Map.iter files_info (fun fn fileinfo ->
        if Relative_path.Map.mem builtins fn then
          ()
        else
          let (tast, _) = Typing_check_utils.type_file ctx fn fileinfo in
          let result = Coverage_level.get_levels ctx tast fn in
          print_colored fn result)
  | Coverage ->
    Relative_path.Map.iter files_info (fun fn fileinfo ->
        if Relative_path.Map.mem builtins fn then
          ()
        else
          let (tast, _) = Typing_check_utils.type_file ctx fn fileinfo in
          let type_acc = ServerCoverageMetric.accumulate_types ctx tast fn in
          print_coverage type_acc)
  | Cst_search ->
    let path = expect_single_file () in
    let (ctx, entry) = Provider_context.add_entry ~ctx ~path in
    let result =
      let open Result.Monad_infix in
      Sys_utils.read_stdin_to_string ()
      |> Hh_json.json_of_string
      |> CstSearchService.compile_pattern ctx
      >>| CstSearchService.search ctx entry
      >>| CstSearchService.result_to_json ~sort_results:true
      >>| Hh_json.json_to_string ~pretty:true
    in
    begin
      match result with
      | Ok result -> Printf.printf "%s\n" result
      | Error message ->
        Printf.printf "%s\n" message;
        exit 1
    end
  | Dump_symbol_info ->
    iter_over_files (fun filename ->
        match Relative_path.Map.find_opt files_info filename with
        | Some fileinfo ->
          let raw_result =
            SymbolInfoService.helper ctx [] [(filename, fileinfo)]
          in
          let result = SymbolInfoService.format_result raw_result in
          let result_json = ClientSymbolInfo.to_json result in
          print_endline (Hh_json.json_to_multiline result_json)
        | None -> ())
  | Lint ->
    let lint_errors =
      Relative_path.Map.fold
        files_contents
        ~init:[]
        ~f:(fun fn content lint_errors ->
          lint_errors
          @ fst (Lint.do_ (fun () -> Linting_service.lint ctx fn content)))
    in
    if lint_errors <> [] then (
      let lint_errors =
        List.sort
          ~compare:
            begin
              fun x y ->
              Pos.compare (Lint.get_pos x) (Lint.get_pos y)
            end
          lint_errors
      in
      let lint_errors = List.map ~f:Lint.to_absolute lint_errors in
      ServerLint.output_text stdout lint_errors error_format;
      exit 2
    ) else
      Printf.printf "No lint errors\n"
  | Dump_deps ->
    Relative_path.Map.iter files_info (fun fn fileinfo ->
        ignore @@ Typing_check_utils.check_defs ctx fn fileinfo);
    if Hashtbl.length dbg_deps > 0 then dump_debug_deps dbg_deps
  | Dump_glean_deps ->
    Relative_path.Map.iter files_info (fun fn fileinfo ->
        ignore @@ Typing_check_utils.check_defs ctx fn fileinfo);
    dump_debug_glean_deps dbg_glean_deps
  | Dump_inheritance ->
    let open ServerCommandTypes.Method_jumps in
    let naming_table = Naming_table.create files_info in
    Naming_table.iter naming_table Typing_deps.update_file;
    Naming_table.iter naming_table (fun fn fileinfo ->
        if Relative_path.Map.mem builtins fn then
          ()
        else (
          List.iter fileinfo.FileInfo.classes (fun (_p, class_) ->
              Printf.printf
                "Ancestors of %s and their overridden methods:\n"
                class_;
              let ancestors =
                MethodJumps.get_inheritance
                  ctx
                  class_
                  ~filter:No_filter
                  ~find_children:false
                  naming_table
                  None
              in
              ClientMethodJumps.print_readable ancestors ~find_children:false;
              Printf.printf "\n");
          Printf.printf "\n";
          List.iter fileinfo.FileInfo.classes (fun (_p, class_) ->
              Printf.printf
                "Children of %s and the methods they override:\n"
                class_;
              let children =
                MethodJumps.get_inheritance
                  ctx
                  class_
                  ~filter:No_filter
                  ~find_children:true
                  naming_table
                  None
              in
              ClientMethodJumps.print_readable children ~find_children:true;
              Printf.printf "\n")
        ))
  | Identify_symbol (line, column) ->
    let path = expect_single_file () in
    let (ctx, entry) = Provider_context.add_entry ~ctx ~path in
    (* TODO(ljw): surely this doesn't need quarantine? *)
    let result =
      Provider_utils.respect_but_quarantine_unsaved_changes ~ctx ~f:(fun () ->
          ServerIdentifyFunction.go_quarantined_absolute
            ~ctx
            ~entry
            ~line
            ~column)
    in
    begin
      match result with
      | [] -> print_endline "None"
      | result -> ClientGetDefinition.print_readable ~short_pos:true result
    end
  | Find_local (line, char) ->
    let filename = expect_single_file () in
    let (ctx, entry) = Provider_context.add_entry ~ctx ~path:filename in
    let result = ServerFindLocals.go ~ctx ~entry ~line ~char in
    let print pos = Printf.printf "%s\n" (Pos.string_no_file pos) in
    List.iter result print
  | Outline ->
    iter_over_files (fun filename ->
        let file = cat (Relative_path.to_absolute filename) in
        let results =
          FileOutline.outline (Provider_context.get_popt ctx) file
        in
        FileOutline.print ~short_pos:true results)
  | Dump_nast ->
    iter_over_files (fun filename ->
        let nasts = create_nasts ctx files_info in
        let nast = Relative_path.Map.find nasts filename in
        Printf.printf "%s\n" (Nast.show_program nast))
  | Dump_tast ->
    let (errors, tasts, _gi_solved) =
      compute_tasts_expand_types ctx ~verbosity files_info files_contents
    in
    print_errors_if_present (parse_errors @ Errors.get_error_list errors);
    print_tasts tasts ctx
  | Check_tast ->
    iter_over_files (fun filename ->
        let files_contents =
          Relative_path.Map.filter files_contents ~f:(fun k _v -> k = filename)
        in
        let (errors, tasts, _gi_solved) =
          compute_tasts_expand_types ctx ~verbosity files_info files_contents
        in
        print_tasts tasts ctx;
        if not @@ Errors.is_empty errors then (
          print_errors error_format errors max_errors;
          Printf.printf "Did not typecheck the TAST as there are typing errors.";
          exit 2
        ) else
          let tast_check_errors = typecheck_tasts tasts ctx filename in
          print_error_list error_format tast_check_errors max_errors;
          if tast_check_errors <> [] then exit 2)
  | Dump_stripped_tast ->
    iter_over_files (fun filename ->
        let files_contents =
          Relative_path.Map.filter files_contents ~f:(fun k _v -> k = filename)
        in
        let (_, (tasts, _gienvs)) =
          compute_tasts ctx files_info files_contents
        in
        let tast = Relative_path.Map.find tasts filename in
        let nast = Tast.to_nast tast in
        Printf.printf "%s\n" (Nast.show_program nast))
  | Rewrite ->
    let (errors, _tasts, gi_solved) =
      compute_tasts_expand_types ctx ~verbosity files_info files_contents
    in
    print_errors_if_present (parse_errors @ Errors.get_error_list errors);
    (match gi_solved with
    | None ->
      prerr_endline
        ( "error: no patches generated as global"
        ^ " inference is turend off (use --global-inference)" );
      exit 1
    | Some gi_solved ->
      let patches =
        ServerGlobalInference.Mode_rewrite.get_patches ~files_contents gi_solved
      in
      if List.length patches <= 0 then
        print_endline "No patches"
      else
        (* simple key-map: convert Relative_path.Map.t into an SMap.t
         * without changing the values *)
        let file_contents =
          Relative_path.Map.fold
            files_contents
            ~f:(fun fn -> SMap.add (Relative_path.suffix fn))
            ~init:SMap.empty
        in
        let patched =
          ClientRefactor.apply_patches_to_file_contents file_contents patches
        in
        let print_filename = not @@ Int.equal (SMap.cardinal patched) 1 in
        SMap.iter
          (fun fn new_contents ->
            if print_filename then Printf.printf "//// %s\n" fn;
            Out_channel.output_string stdout new_contents)
          patched)
  | Find_refs (line, column) ->
    let path = expect_single_file () in
    let naming_table = Naming_table.create files_info in
    Naming_table.iter naming_table Typing_deps.update_file;
    let genv = ServerEnvBuild.default_genv in
    let env =
      {
        (ServerEnvBuild.make_env genv.ServerEnv.config) with
        ServerEnv.naming_table;
        ServerEnv.tcopt = Provider_context.get_tcopt ctx;
      }
    in
    let include_defs = true in
    let (ctx, entry) =
      Provider_context.add_entry
        ~ctx:(Provider_utils.ctx_from_server_env env)
        ~path
    in
    let open Option.Monad_infix in
    let open ServerCommandTypes.Done_or_retry in
    let results =
      Provider_utils.respect_but_quarantine_unsaved_changes ~ctx ~f:(fun () ->
          ServerFindRefs.(
            go_from_file_ctx ~ctx ~entry ~line ~column >>= fun (name, action) ->
            go ctx action include_defs genv env
            |> map_env ~f:(to_ide name)
            |> snd
            |> function
            | Done r -> r
            | Retry ->
              failwith
              @@ "should only happen with prechecked files "
              ^ "which are not a thing in hh_single_type_check"))
    in
    ClientFindRefs.print_ide_readable results
  | Go_to_impl (line, column) ->
    let filename = expect_single_file () in
    let naming_table = Naming_table.create files_info in
    Naming_table.iter naming_table Typing_deps.update_file;
    let genv = ServerEnvBuild.default_genv in
    let env =
      {
        (ServerEnvBuild.make_env genv.ServerEnv.config) with
        ServerEnv.naming_table;
        ServerEnv.tcopt = Provider_context.get_tcopt ctx;
      }
    in
    let filename = Relative_path.to_absolute filename in
    let contents = cat filename in
    let (ctx, entry) =
      Provider_context.add_entry_from_file_contents
        ~ctx:(Provider_utils.ctx_from_server_env env)
        ~path:(Relative_path.create_detect_prefix filename)
        ~contents
    in
    Option.Monad_infix.(
      ServerCommandTypes.Done_or_retry.(
        let results =
          ServerFindRefs.go_from_file_ctx ~ctx ~entry ~line ~column
          >>= fun (name, action) ->
          ServerGoToImpl.go ~action ~genv ~env
          |> map_env ~f:(ServerFindRefs.to_ide name)
          |> snd
          |> function
          | Done r -> r
          | Retry ->
            failwith
            @@ "should only happen with prechecked files "
            ^ "which are not a thing in hh_single_type_check"
        in
        ClientFindRefs.print_ide_readable results))
  | Highlight_refs (line, column) ->
    let path = expect_single_file () in
    let (ctx, entry) = Provider_context.add_entry ~ctx ~path in
    let results =
      ServerHighlightRefs.go_quarantined ~ctx ~entry ~line ~column
    in
    ClientHighlightRefs.go results ~output_json:false
  | Errors when batch_mode ->
    (* For each file in our batch, run typechecking serially.
      Reset the heaps every time in between. *)
    iter_over_files (fun filename ->
        let oc =
          Out_channel.create (Relative_path.to_absolute filename ^ out_extension)
        in
        (* This means builtins had errors, so lets just print those if we see them *)
        if parse_errors <> [] then
          (* This closes the out channel *)
          write_error_list error_format parse_errors oc max_errors
        else (
          Typing_log.out_channel := oc;
          Provider_utils.respect_but_quarantine_unsaved_changes
            ~ctx
            ~f:(fun () ->
              let files_contents = Multifile.file_to_files filename in
              let (parse_errors, individual_file_info) =
                parse_name_and_decl ctx files_contents
              in
              let errors =
                check_file
                  ctx
                  ~verbosity
                  (Errors.get_error_list parse_errors)
                  individual_file_info
                  error_format
                  max_errors
              in
              write_error_list error_format errors oc max_errors)
        ))
  | Decl_compare when batch_mode ->
    (* For each file in our batch, run typechecking serially.
      Reset the heaps every time in between. *)
    iter_over_files (fun filename ->
        let oc =
          Out_channel.create (Relative_path.to_absolute filename ^ ".decl_out")
        in
        Provider_utils.respect_but_quarantine_unsaved_changes ~ctx ~f:(fun () ->
            let files_contents =
              Relative_path.Map.filter files_contents ~f:(fun k _v ->
                  k = filename)
            in
            let (_, individual_file_info) =
              parse_name_and_decl ctx files_contents
            in
            try
              test_decl_compare
                ctx
                filename
                builtins
                files_contents
                individual_file_info;
              Out_channel.output_string oc ""
            with e ->
              let msg = Exn.to_string e in
              Out_channel.output_string oc msg);
        Out_channel.close oc)
  | Errors ->
    (* Don't typecheck builtins *)
    let errors =
      check_file ctx ~verbosity parse_errors files_info error_format max_errors
    in
    print_error_list error_format errors max_errors;
    if errors <> [] then exit 2
  | Decl_compare ->
    let filename = expect_single_file () in
    test_decl_compare ctx filename builtins files_contents files_info
  | Shallow_class_diff ->
    print_errors_if_present parse_errors;
    let filename = expect_single_file () in
    test_shallow_class_diff ctx filename
  | Linearization ->
    if parse_errors <> [] then (
      print_error error_format (List.hd_exn parse_errors);
      exit 2
    );
    let files_info =
      Relative_path.Map.fold
        builtins
        ~f:
          begin
            fun k _ acc ->
            Relative_path.Map.remove acc k
          end
        ~init:files_info
    in
    Relative_path.Map.iter files_info ~f:(fun _file info ->
        let { FileInfo.classes; _ } = info in
        List.iter classes ~f:(fun (_, classname) ->
            Printf.printf "Linearization for class %s:\n" classname;
            let key = (classname, Decl_defs.Member_resolution) in
            let linearization = Decl_linearize.get_linearization ctx key in
            let linearization =
              Sequence.map linearization (fun mro ->
                  let name = mro.Decl_defs.mro_name in
                  let targs =
                    List.map
                      mro.Decl_defs.mro_type_args
                      (Typing_print.full_decl ctx)
                  in
                  let targs =
                    if targs = [] then
                      ""
                    else
                      "<" ^ String.concat ~sep:"," targs ^ ">"
                  in
                  Decl_defs.(
                    let modifiers =
                      [
                        ( if Option.is_some mro.mro_required_at then
                          Some "requirement"
                        else if mro.mro_via_req_extends || mro.mro_via_req_impl
                      then
                          Some "synthesized"
                        else
                          None );
                        ( if mro.mro_xhp_attrs_only then
                          Some "xhp_attrs_only"
                        else
                          None );
                        ( if mro.mro_consts_only then
                          Some "consts_only"
                        else
                          None );
                        ( if mro.mro_copy_private_members then
                          Some "copy_private_members"
                        else
                          None );
                        ( if mro.mro_passthrough_abstract_typeconst then
                          Some "PAT"
                        else
                          None );
                        Option.map mro.mro_trait_reuse ~f:(fun c ->
                            "trait reuse via " ^ c);
                      ]
                      |> List.filter_map ~f:(fun x -> x)
                      |> String.concat ~sep:", "
                    in
                    Printf.sprintf
                      "%s%s%s"
                      name
                      targs
                      ( if String.equal modifiers "" then
                        ""
                      else
                        Printf.sprintf "(%s)" modifiers )))
              |> Sequence.to_list
            in
            Printf.printf "[%s]\n" (String.concat ~sep:", " linearization)))

(*****************************************************************************)
(* Main entry point *)
(*****************************************************************************)

let decl_and_run_mode
    {
      files;
      mode;
      error_format;
      no_builtins;
      tcopt;
      max_errors;
      batch_mode;
      out_extension;
      verbosity;
    }
    (popt : TypecheckerOptions.t)
    (hhi_root : Path.t)
    (sienv : SearchUtils.si_env) : unit =
  Ident.track_names := true;
  let builtins =
    if no_builtins then
      Relative_path.Map.empty
    else
      (* Note that the regular `.hhi` files have already been written to disk
      with `Hhi.get_root ()` *)
      let magic_builtins =
        match mode with
        | Ai _ -> Array.append magic_builtins Ai.magic_builtins
        | _ -> magic_builtins
      in
      Array.iter magic_builtins ~f:(fun (file_name, file_contents) ->
          let file_path = Path.concat hhi_root file_name in
          let file = Path.to_string file_path in
          Sys_utils.try_touch ~follow_symlinks:true file;
          Sys_utils.write_file ~file file_contents);

      (* Take the builtins (file, contents) array and create relative paths *)
      Array.fold
        (Array.append magic_builtins hhi_builtins)
        ~init:Relative_path.Map.empty
        ~f:(fun acc (f, src) ->
          let f = Path.concat hhi_root f |> Path.to_string in
          Relative_path.Map.add
            acc
            ~key:(Relative_path.create Relative_path.Hhi f)
            ~data:src)
  in
  let files = List.map ~f:(Relative_path.create Relative_path.Dummy) files in
  let files_contents =
    List.fold
      files
      ~f:(fun acc filename ->
        let files_contents = Multifile.file_to_files filename in
        Relative_path.Map.union acc files_contents)
      ~init:Relative_path.Map.empty
  in
  (* Merge in builtins *)
  let files_contents_with_builtins =
    Relative_path.Map.fold
      builtins
      ~f:
        begin
          fun k src acc ->
          Relative_path.Map.add acc ~key:k ~data:src
        end
      ~init:files_contents
  in
  (* Don't declare all the filenames in batch_errors mode *)
  let to_decl =
    if batch_mode then
      builtins
    else
      files_contents_with_builtins
  in
  let dbg_deps = Hashtbl.Poly.create () in
  ( if mode = Dump_deps then
    (* In addition to actually recording the dependencies in shared memory,
     we build a non-hashed respresentation of the dependency graph
     for printing. *)
    let get_debug_trace root obj =
      let root = Typing_deps.Dep.variant_to_string root in
      let obj = Typing_deps.Dep.variant_to_string obj in
      match Hashtbl.find dbg_deps obj with
      | Some set -> HashSet.add set root
      | None ->
        let set = HashSet.create () in
        HashSet.add set root;
        Hashtbl.set dbg_deps obj set
    in
    Typing_deps.add_dependency_callback "get_debug_trace" get_debug_trace );
  let dbg_glean_deps = HashSet.create () in
  ( if mode = Dump_glean_deps then
    (* In addition to actually recording the dependencies in shared memory,
     we build a non-hashed respresentation of the dependency graph
     for printing. In the callback we receive this as dep_right uses dep_left. *)
    let get_debug_trace dep_right dep_left =
      HashSet.add dbg_glean_deps (dep_left, dep_right)
    in
    Typing_deps.add_dependency_callback "get_debug_trace" get_debug_trace );
  let ctx = Provider_context.empty_for_test ~popt ~tcopt in
  let (errors, files_info) = parse_name_and_decl ctx to_decl in
  handle_mode
    mode
    files
    ctx
    builtins
    files_contents
    files_info
    (Errors.get_error_list errors)
    max_errors
    error_format
    batch_mode
    out_extension
    dbg_deps
    dbg_glean_deps
    sienv
    ~verbosity

let main_hack ({ tcopt; _ } as opts) (sienv : SearchUtils.si_env) : unit =
  (* TODO: We should have a per file config *)
  Sys_utils.signal Sys.sigusr1 (Sys.Signal_handle Typing.debug_print_last_pos);
  EventLogger.init_fake ();

  let (_handle : SharedMem.handle) =
    SharedMem.init ~num_workers:0 GlobalConfig.default_sharedmem_config
  in
  Tempfile.with_tempdir (fun hhi_root ->
      Hhi.set_hhi_root_for_unit_test hhi_root;
      Relative_path.set_path_prefix Relative_path.Root (Path.make "/");
      Relative_path.set_path_prefix Relative_path.Hhi hhi_root;
      Relative_path.set_path_prefix Relative_path.Tmp (Path.make "tmp");
      decl_and_run_mode opts tcopt hhi_root sienv;
      TypingLogger.flush_buffers ())

(* command line driver *)
let () =
  if !Sys.interactive then
    ()
  else
    (* On windows, setting 'binary mode' avoids to output CRLF on
       stdout.  The 'text mode' would not hurt the user in general, but
       it breaks the testsuite where the output is compared to the
       expected one (i.e. in given file without CRLF). *)
    Out_channel.set_binary_mode stdout true;
  let (options, sienv) = parse_options () in
  Unix.handle_unix_error main_hack options sienv
