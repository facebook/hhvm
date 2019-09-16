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
module TNBody = Typing_naming_body

module StringAnnotation = struct
  type t = string

  let pp fmt str = Format.pp_print_string fmt str
end

module PS = Full_fidelity_positioned_syntax
module PositionedTree = Full_fidelity_syntax_tree.WithSyntax (PS)
module TS = Full_fidelity_typed_positioned_syntax
module TypedTree = Full_fidelity_syntax_tree.WithSyntax (TS)

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
  | Dump_typed_full_fidelity_json
  | Find_refs of int * int
  | Highlight_refs of int * int
  | Decl_compare
  | Linearization

type options = {
  files: string list;
  mode: mode;
  error_format: Errors.format;
  no_builtins: bool;
  max_errors: int option;
  tcopt: GlobalOptions.t;
  batch_mode: bool;
  out_extension: string;
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

let exit_on_parent_exit () = Parent.exit_on_parent_exit 10 60

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
  let safe_array = ref (Some false) in
  let safe_vector_array = ref (Some false) in
  let forbid_nullable_cast = ref false in
  let deregister_attributes = ref None in
  let disallow_ambiguous_lambda = ref None in
  let disallow_array_typehint = ref None in
  let disallow_array_literal = ref None in
  let no_fallback_in_namespaces = ref None in
  let dynamic_view = ref None in
  let allow_array_as_tuple = ref (Some false) in
  let allow_user_attributes = ref None in
  let disallow_unset_on_varray = ref None in
  let auto_namespace_map = ref None in
  let unsafe_rx = ref (Some false) in
  let log_inference_constraints = ref None in
  let new_inference_lambda = ref (Some false) in
  let timeout = ref None in
  let disallow_invalid_arraykey = ref None in
  let typecheck_xhp_cvars = ref (Some false) in
  let ignore_collection_expr_type_arguments = ref (Some false) in
  let allow_ref_param_on_constructor = ref (Some false) in
  let disallow_byref_dynamic_calls = ref (Some false) in
  let disallow_byref_calls = ref (Some false) in
  let set_bool x () = x := Some true in
  let shallow_class_decl = ref false in
  let out_extension = ref ".out" in
  let like_type_hints = ref false in
  let like_casts = ref false in
  let pessimize_types = ref false in
  let simple_pessimize = ref 0.0 in
  let coercion_from_dynamic = ref false in
  let coercion_from_union = ref false in
  let complex_coercion = ref false in
  let disable_partially_abstract_typeconsts = ref false in
  let rust_parser_errors = ref false in
  let symbolindex_file = ref None in
  let check_xhp_attribute = ref false in
  let disallow_invalid_arraykey_constraint = ref None in
  let enable_class_level_where_clauses = ref false in
  let enable_constant_visibility_modifiers = ref false in
  let disable_legacy_soft_typehints = ref false in
  let allow_new_attribute_syntax = ref false in
  let allow_toplevel_requires = ref false in
  let infer_missing = ref GlobalOptions.InferMissing.Deactivated in
  let const_static_props = ref false in
  let disable_legacy_attribute_syntax = ref false in
  let const_attribute = ref false in
  let disallow_goto = ref false in
  let const_default_func_args = ref false in
  let disallow_silence = ref false in
  let abstract_static_props = ref false in
  let disable_halt_compiler = ref false in
  let disable_unset_class_const = ref false in
  let options =
    [
      ("--ai", Arg.String set_ai, " Run the abstract interpreter (Zoncolan)");
      ( "--allow-user-attributes",
        Arg.Unit (set_bool allow_user_attributes),
        " Allow all user attributes" );
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
      ( "--typed-full-fidelity-json",
        Arg.Unit (set_mode Dump_typed_full_fidelity_json),
        " (mode) show full fidelity parse tree with types in json format." );
      ( "--find-refs",
        Arg.Tuple
          [
            Arg.Int (fun x -> line := x);
            Arg.Int (fun column -> set_mode (Find_refs (!line, column)) ());
          ],
        "<pos> Find all usages of a symbol at given line and column" );
      ( "--highlight-refs",
        Arg.Tuple
          [
            Arg.Int (fun x -> line := x);
            Arg.Int
              (fun column -> set_mode (Highlight_refs (!line, column)) ());
          ],
        "<pos> Highlight all usages of a symbol at given line and column" );
      ( "--decl-compare",
        Arg.Unit (set_mode Decl_compare),
        " Test comparison functions used in incremental mode on declarations"
        ^ " in provided file" );
      ( "--safe_array",
        Arg.Unit (set_bool safe_array),
        " Enforce array subtyping relationships so that array<T> and array<Tk, Tv> are each subtypes of array but not vice-versa."
      );
      ( "--safe_vector_array",
        Arg.Unit (set_bool safe_vector_array),
        " Enforce array subtyping relationships so that array<T> is not a of array<int, T>."
      );
      ( "--forbid_nullable_cast",
        Arg.Set forbid_nullable_cast,
        " Forbid casting from nullable values." );
      ( "--disallow-ambiguous-lambda",
        Arg.Unit (set_bool disallow_ambiguous_lambda),
        " Disallow definition of lambdas that require use-site checking." );
      ( "--disallow-array-typehint",
        Arg.Unit (set_bool disallow_array_typehint),
        " Disallow usage of array typehints." );
      ( "--disallow-array-literal",
        Arg.Unit (set_bool disallow_array_literal),
        " Disallow usage of array literals." );
      ( "--no-fallback-in-namespaces",
        Arg.Unit (set_bool no_fallback_in_namespaces),
        " Treat foo() as namespace\\foo() and MY_CONST as namespace\\MY_CONST."
      );
      ( "--dynamic-view",
        Arg.Unit (set_bool dynamic_view),
        " Turns on dynamic view, replacing Tany with dynamic" );
      ( "--allow-array-as-tuple",
        Arg.Unit (set_bool allow_array_as_tuple),
        " Allow tuples to be passed as untyped arrays and vice versa" );
      ( "--disallow-unset-on-varray",
        Arg.Unit (set_bool disallow_unset_on_varray),
        " Disallow unsetting indices from varrays" );
      ( "--unsafe-rx",
        Arg.Unit (set_bool unsafe_rx),
        " Disables reactivity related errors" );
      ( "--mro",
        Arg.Unit (set_mode Linearization),
        " Grabs the linearization of all classes in a file." );
      ( "--log-inference-constraints",
        Arg.Unit (set_bool log_inference_constraints),
        " Log inference constraints to Scuba." );
      ( "--new-inference-lambda",
        Arg.Unit (set_bool new_inference_lambda),
        " Type inference of unannotated lambdas by constraint generation." );
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
      ( "--check-xhp-cvar-arity",
        Arg.Unit (set_bool typecheck_xhp_cvars),
        "Typechecks xhp cvar arity" );
      ( "--check-xhp-attribute",
        Arg.Set check_xhp_attribute,
        "Typechecks xhp required attributes" );
      ( "--ignore-collection-expr-type-arguments",
        Arg.Unit (set_bool ignore_collection_expr_type_arguments),
        "Typechecker ignores type arguments to vec<T>[...] style expressions"
      );
      ( "--allow-ref-param-on-constructor",
        Arg.Unit (set_bool allow_ref_param_on_constructor),
        "Allow class constructors to take reference parameters" );
      ( "--disallow-byref-dynamic-calls",
        Arg.Unit (set_bool disallow_byref_dynamic_calls),
        "Disallow passing arguments by reference to dynamically called functions [e.g. $foo(&$bar)]"
      );
      ( "--disallow-byref-calls",
        Arg.Unit (set_bool disallow_byref_calls),
        "Disallow passing arguments by reference in any form [e.g. foo(&$bar)]"
      );
      ( "--shallow-class-decl",
        Arg.Set shallow_class_decl,
        "Look up class members lazily from shallow declarations" );
      ( "--like-type-hints",
        Arg.Set like_type_hints,
        "Allows like types to be written in type hint positions" );
      ( "--like-casts",
        Arg.Set like_casts,
        "Allows like types to be written in as expressions" );
      ( "--pessimize-types",
        Arg.Set pessimize_types,
        "When unenforceable types are encountered, convert them to like types"
      );
      ( "--simple-pessimize",
        Arg.Set_float simple_pessimize,
        "At coercion points, if a type is not enforceable, wrap it in like. Float argument 0.0 to 1.0 sets frequency"
      );
      ( "--coercion-from-dynamic",
        Arg.Set coercion_from_dynamic,
        "Allows coercion from dynamic to enforceable types at positions that HHVM enforces"
      );
      ( "--coercion-from-union",
        Arg.Set coercion_from_union,
        "Allows coercion from union types" );
      ( "--complex-coercion",
        Arg.Set complex_coercion,
        "Allows complex coercions that involve like types" );
      ( "--disable-partially-abstract-typeconsts",
        Arg.Set disable_partially_abstract_typeconsts,
        "Treat partially abstract type constants as concrete type constants" );
      ( "--rust-parser-errors",
        Arg.Bool (fun x -> rust_parser_errors := x),
        "Use rust parser error checker" );
      ( "--symbolindex-file",
        Arg.String (fun str -> symbolindex_file := Some str),
        "Load the symbol index from this file" );
      ( "--enable-class-level-where-clauses",
        Arg.Set enable_class_level_where_clauses,
        "Enables support for class-level where clauses" );
      ( "--enable-constant-visibility-modifiers",
        Arg.Set enable_constant_visibility_modifiers,
        "Enable constant visibility modifiers" );
      ( "--disable-legacy-soft-typehints",
        Arg.Set disable_legacy_soft_typehints,
        "Disables the legacy @ syntax for soft typehints (use __Soft instead)"
      );
      ( "--allow-new-attribute-syntax",
        Arg.Set allow_new_attribute_syntax,
        "Allow the new @ attribute syntax (disables legacy soft typehints)" );
      ( "--allow-toplevel-requires",
        Arg.Set allow_toplevel_requires,
        "Allow `require()` and similar at the top-level" );
      ( "--infer-missing",
        Arg.String
          (fun s -> infer_missing := GlobalOptions.InferMissing.from_string s),
        "<return|params|global> Deduce more information about unannotated types in the tast"
      );
      ( "--const-static-props",
        Arg.Set const_static_props,
        "Enable static properties to be const" );
      ( "--disable-legacy-attribute-syntax",
        Arg.Set disable_legacy_attribute_syntax,
        "Disable the legacy <<...>> user attribute syntax" );
      ("--const-attribute", Arg.Set const_attribute, "Allow __Const attribute");
      ( "--disallow-goto",
        Arg.Set disallow_goto,
        "Forbid the goto operator and goto labels in the parser" );
      ( "--const-default-func-args",
        Arg.Set const_default_func_args,
        "Statically check default function arguments are constant initializers"
      );
      ( "--disallow-silence",
        Arg.Set disallow_silence,
        "Disallow the error suppression operator, @" );
      ( "--abstract-static-props",
        Arg.Set abstract_static_props,
        "Static properties can be abstract" );
      ( "--disable-unset-class-const",
        Arg.Set disable_unset_class_const,
        "Make unsetting a class const a parse error" );
      ( "--disable-halt-compiler",
        Arg.Set disable_halt_compiler,
        "Disable using PHP __halt_compiler()" );
    ]
  in
  let options = Arg.align ~limit:25 options in
  Arg.parse options (fun fn -> fn_ref := fn :: !fn_ref) usage;
  let fns =
    match !fn_ref with
    | [] -> die usage
    | x -> x
  in
  let not_ = Option.map ~f:not in
  let tcopt =
    GlobalOptions.make
      ?tco_unsafe_rx:!unsafe_rx
      ?tco_safe_array:!safe_array
      ?tco_safe_vector_array:!safe_vector_array
      ?po_deregister_php_stdlib:!deregister_attributes
      ?tco_disallow_ambiguous_lambda:!disallow_ambiguous_lambda
      ?tco_disallow_array_typehint:!disallow_array_typehint
      ?tco_disallow_array_literal:!disallow_array_literal
      ?tco_dynamic_view:!dynamic_view
      ?tco_disallow_array_as_tuple:(not_ !allow_array_as_tuple)
      ?tco_disallow_unset_on_varray:!disallow_unset_on_varray
      ?tco_log_inference_constraints:!log_inference_constraints
      ?tco_new_inference_lambda:!new_inference_lambda
      ?tco_timeout:!timeout
      ?tco_disallow_invalid_arraykey:!disallow_invalid_arraykey
      ?po_auto_namespace_map:!auto_namespace_map
      ?tco_typecheck_xhp_cvars:!typecheck_xhp_cvars
      ?tco_ignore_collection_expr_type_arguments:
        !ignore_collection_expr_type_arguments
      ?tco_disallow_byref_dynamic_calls:!disallow_byref_dynamic_calls
      ?tco_disallow_byref_calls:!disallow_byref_calls
      ?tco_disallow_invalid_arraykey_constraint:
        !disallow_invalid_arraykey_constraint
      ~tco_check_xhp_attribute:!check_xhp_attribute
      ~tco_shallow_class_decl:!shallow_class_decl
      ~tco_like_type_hints:!like_type_hints
      ~tco_like_casts:!like_casts
      ~tco_pessimize_types:!pessimize_types
      ~tco_simple_pessimize:!simple_pessimize
      ~tco_coercion_from_dynamic:!coercion_from_dynamic
      ~tco_coercion_from_union:!coercion_from_union
      ~tco_complex_coercion:!complex_coercion
      ~tco_disable_partially_abstract_typeconsts:
        !disable_partially_abstract_typeconsts
      ~log_levels:!log_levels
      ~po_rust_parser_errors:!rust_parser_errors
      ~po_enable_class_level_where_clauses:!enable_class_level_where_clauses
      ~po_enable_constant_visibility_modifiers:
        !enable_constant_visibility_modifiers
      ~po_disable_legacy_soft_typehints:!disable_legacy_soft_typehints
      ~po_allow_new_attribute_syntax:!allow_new_attribute_syntax
      ~po_disallow_toplevel_requires:(not !allow_toplevel_requires)
      ~tco_const_static_props:!const_static_props
      ~tco_infer_missing:!infer_missing
      ~po_disable_legacy_attribute_syntax:!disable_legacy_attribute_syntax
      ~tco_const_attribute:!const_attribute
      ~po_allow_goto:(not !disallow_goto)
      ~po_const_default_func_args:!const_default_func_args
      ~po_disallow_silence:!disallow_silence
      ~po_abstract_static_props:!abstract_static_props
      ~po_disable_unset_class_const:!disable_unset_class_const
      ~po_disable_halt_compiler:!disable_halt_compiler
      ~tco_check_attribute_locations:true
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
      ~globalrev_opt:None
      ~namespace_map
      ~provider_name:"LocalIndex"
      ~quiet:true
      ~savedstate_file_opt:!symbolindex_file
      ~workers:None
  in
  let sienv = { sienv with SearchUtils.sie_resolve_signatures = true } in
  ( {
      files = fns;
      mode = !mode;
      no_builtins = !no_builtins;
      max_errors = !max_errors;
      error_format = !error_format;
      tcopt;
      batch_mode = !batch_mode;
      out_extension = !out_extension;
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
  ClientCoverageMetric.go
    ~json:false
    (Some (Coverage_level_defs.Leaf type_acc))

let check_file opts errors files_info =
  Relative_path.Map.fold
    files_info
    ~f:
      begin
        fun fn fileinfo errors ->
        errors
        @ Errors.get_error_list
            (Typing_check_utils.check_defs opts fn fileinfo)
      end
    ~init:errors

let create_nasts files_info =
  let build_nast fn _ =
    let ast = Ast_provider.get_ast ~full:true fn in
    Naming.program ast
  in
  Relative_path.Map.mapi ~f:build_nast files_info

let parse_name_and_decl popt files_contents =
  Errors.do_ (fun () ->
      let parsed_files =
        Relative_path.Map.mapi files_contents ~f:(fun fn contents ->
            Errors.run_in_context fn Errors.Parsing (fun () ->
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
              let { Parser_return.file_mode; comments; ast; _ } =
                parsed_file
              in
              (* If the feature is turned on, deregister functions with attribute
        __PHPStdLib. This does it for all functions, not just hhi files *)
              let (funs, classes, typedefs, consts) = Nast.get_defs ast in
              {
                FileInfo.file_mode;
                funs;
                classes;
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
              let { FileInfo.funs; classes; typedefs; consts; _ } = fileinfo in
              NamingGlobal.make_env ~funs ~classes ~typedefs ~consts));

      Relative_path.Map.iter parsed_files (fun fn parsed_file ->
          Errors.run_in_context fn Errors.Decl (fun () ->
              Decl.name_and_declare_types_program parsed_file.Parser_return.ast));

      files_info)

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
  let f1 = Decl_pos_utils.NormalizeSig.fun_type f1 in
  let f2 = Decl_pos_utils.NormalizeSig.fun_type f2 in
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

let test_decl_compare filenames popt builtins files_contents files_info =
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
        ~f:begin
             fun k _ acc -> Relative_path.Map.remove acc k
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
      match Relative_path.Map.get files_info path with
      | None -> SSet.empty
      | Some info -> SSet.of_list @@ List.map info.FileInfo.classes snd
    in
    (* We need to oldify, not remove, for ClassEltDiff to work *)
    Decl_redecl_service.oldify_type_decl
      None
      get_classes
      ~bucket_size:1
      FileInfo.empty_names
      defs
      ~collect_garbage:false;

    let files_contents = Relative_path.Map.map files_contents ~f:add_newline in
    let (_, _) = parse_name_and_decl popt files_contents in
    let (typedefs2, funs2, classes2) = get_decls defs in
    List.iter2_exn typedefs1 typedefs2 compare_typedefs;
    List.iter2_exn funs1 funs2 compare_funs;
    List.iter2_exn classes1 classes2 compare_classes;
    ()

(* Returns a list of Tast defs, along with associated type environments. *)
let compute_tasts opts files_info interesting_files :
    Errors.t * Tast.program Relative_path.Map.t =
  let _f _k nast x =
    match (nast, x) with
    | (Some nast, Some _) -> Some nast
    | _ -> None
  in
  Errors.do_ (fun () ->
      let nasts = create_nasts files_info in
      (* Interesting files are usually the non hhi ones. *)
      let filter_non_interesting nasts =
        Relative_path.Map.merge nasts interesting_files (fun _k nast x ->
            match (nast, x) with
            | (Some nast, Some _) -> Some nast
            | _ -> None)
      in
      let nasts = filter_non_interesting nasts in
      Relative_path.Map.map nasts ~f:(Typing.nast_to_tast opts))

(**
 * Compute TASTs for some files, then expand all type variables.
 *)
let compute_tasts_expand_types opts files_info interesting_files =
  let (errors, tasts) = compute_tasts opts files_info interesting_files in
  let tasts = Relative_path.Map.map tasts Tast_expand.expand_program in
  (errors, tasts)

let print_tasts tasts tcopt =
  let dummy_filename = Relative_path.default in
  let env = Typing_env.empty tcopt dummy_filename ~droot:None in
  let print_pos_and_ty (pos, ty) =
    Format.asprintf "(%a, %s)" Pos.pp pos (Typing_print.full_strip_ns env ty)
  in
  let pp_fb fmt fb =
    let s =
      match fb with
      | Tast.HasUnsafeBlocks -> "Has unsafe blocks"
      | Tast.NoUnsafeBlocks -> "No unsafe blocks"
    in
    Format.pp_print_string fmt s
  in
  let pp_en fmt _ = Format.pp_print_string fmt "()" in
  let pp_ex fmt ex = Format.pp_print_string fmt (print_pos_and_ty ex) in
  let pp_hi fmt ty =
    Format.asprintf "(%s)" (Typing_print.full_strip_ns env ty)
    |> Format.pp_print_string fmt
  in
  Relative_path.Map.iter tasts (fun _k (tast : Tast.program) ->
      Printf.printf "%s\n" (Aast.show_program pp_ex pp_fb pp_en pp_hi tast))

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
  Caml.Hashtbl.fold (fun obj set acc -> (obj, set) :: acc) deps []
  |> List.sort ~compare:(fun (a, _) (b, _) -> String.compare a b)
  |> List.map ~f:(fun (obj, roots) ->
         let roots =
           HashSet.fold List.cons roots [] |> List.sort ~compare:String.compare
         in
         (obj, roots))

(* Note: this prints dependency graph edges in the same direction as the mapping
   which is actually stored in the shared memory table. The line "X -> Y" can be
   read, "X is used by Y", or "X is a dependency of Y", or "when X changes, Y
   must be rechecked". *)
let dump_debug_deps dbg_deps =
  dbg_deps |> sort_debug_deps |> show_debug_deps |> Printf.printf "%s\n"

let scan_files_for_symbol_index
    (filename : Relative_path.t)
    (popt : ParserOptions.t)
    (sienv : SearchUtils.si_env) : SearchUtils.si_env =
  let files_contents = Multifile.file_to_files filename in
  let (_, individual_file_info) = parse_name_and_decl popt files_contents in
  let fileinfo_list = Relative_path.Map.values individual_file_info in
  let transformed_list =
    List.map fileinfo_list ~f:(fun fileinfo ->
        (filename, SearchUtils.Full fileinfo, SearchUtils.TypeChecker))
  in
  let sienv_ref = ref sienv in
  SymbolIndex.update_files
    ~sienv:sienv_ref
    ~workers:None
    ~paths:transformed_list;
  !sienv_ref

let handle_mode
    mode
    filenames
    tcopt
    popt
    builtins
    files_contents
    files_info
    parse_errors
    max_errors
    error_format
    batch_mode
    out_extension
    dbg_deps
    (sienv : SearchUtils.si_env) =
  let expect_single_file () : Relative_path.t =
    match filenames with
    | [x] -> x
    | _ -> die "Only single file expected"
  in
  let iter_over_files f : unit = List.iter filenames f in
  match mode with
  | Ai _ -> ()
  | Autocomplete
  | Autocomplete_manually_invoked ->
    let filename = expect_single_file () in
    let sienv = scan_files_for_symbol_index filename popt sienv in
    let token = "AUTO332" in
    let token_len = String.length token in
    let file = cat (Relative_path.to_absolute filename) in
    (* Search backwards: there should only be one /real/ case. If there's multiple, *)
    (* guess that the others are preceding explanation comments *)
    let offset =
      Str.search_backward (Str.regexp token) file (String.length file)
    in
    let pos = File_content.offset_to_position file offset in
    let file =
      Str.string_before file offset ^ Str.string_after file (offset + token_len)
    in
    let is_manually_invoked = mode = Autocomplete_manually_invoked in
    let result =
      ServerAutoComplete.auto_complete_at_position
        ~tcopt
        ~pos
        ~is_manually_invoked
        ~file_content:file
        ~sienv
    in
    List.iter
      ~f:
        begin
          fun r ->
          AutocompleteTypes.(Printf.printf "%s %s\n" r.res_name r.res_ty)
        end
      result.Utils.With_complete_flag.value
  | Ffp_autocomplete ->
    iter_over_files (fun filename ->
        try
          let sienv = scan_files_for_symbol_index filename popt sienv in
          let file_text = cat (Relative_path.to_absolute filename) in
          (* TODO: Use a magic word/symbol to identify autocomplete location instead *)
          let args_regex = Str.regexp "AUTOCOMPLETE [1-9][0-9]* [1-9][0-9]*" in
          let position =
            try
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
              tcopt
              file_text
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
          let (tast, _) = Typing_check_utils.type_file tcopt fn fileinfo in
          let result = Coverage_level.get_levels tast fn in
          print_colored fn result)
  | Coverage ->
    Relative_path.Map.iter files_info (fun fn fileinfo ->
        if Relative_path.Map.mem builtins fn then
          ()
        else
          let (tast, _) = Typing_check_utils.type_file tcopt fn fileinfo in
          let type_acc = ServerCoverageMetric.accumulate_types tast fn in
          print_coverage type_acc)
  | Cst_search ->
    let filename = expect_single_file () in
    let fileinfo =
      match Relative_path.Map.get files_info filename with
      | Some fileinfo -> fileinfo
      | None ->
        failwith
          (Printf.sprintf
             "Missing fileinfo for path %s"
             (Relative_path.to_absolute filename))
    in
    Result.Monad_infix.(
      let result =
        Sys_utils.read_stdin_to_string ()
        |> Hh_json.json_of_string
        |> CstSearchService.compile_pattern
        >>| CstSearchService.search tcopt filename fileinfo
        >>| CstSearchService.result_to_json ~sort_results:true
        >>| Hh_json.json_to_string ~pretty:true
      in
      begin
        match result with
        | Ok result -> Printf.printf "%s\n" result
        | Error message ->
          Printf.printf "%s\n" message;
          exit 1
      end)
  | Dump_symbol_info ->
    iter_over_files (fun filename ->
        match Relative_path.Map.get files_info filename with
        | Some fileinfo ->
          let raw_result =
            SymbolInfoService.helper tcopt [] [(filename, fileinfo)]
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
          @ fst (Lint.do_ (fun () -> Linting_service.lint tcopt fn content)))
    in
    if lint_errors <> [] then (
      let lint_errors =
        List.sort
          ~compare:
            begin
              fun x y -> Pos.compare (Lint.get_pos x) (Lint.get_pos y)
            end
          lint_errors
      in
      let lint_errors = List.map ~f:Lint.to_absolute lint_errors in
      ServerLint.output_text stdout lint_errors error_format;
      exit 2
    ) else
      Printf.printf "No lint errors\n"
  | Dump_deps ->
    (* Don't typecheck builtins *)
    let files_info =
      Relative_path.Map.fold
        builtins
        ~f:begin
             fun k _ acc -> Relative_path.Map.remove acc k
           end
        ~init:files_info
    in
    Relative_path.Map.iter files_info (fun fn fileinfo ->
        ignore @@ Typing_check_utils.check_defs tcopt fn fileinfo);
    if Caml.Hashtbl.length dbg_deps > 0 then dump_debug_deps dbg_deps
  | Dump_inheritance ->
    ServerCommandTypes.Method_jumps.(
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
                    class_
                    ~filter:No_filter
                    ~find_children:true
                    naming_table
                    None
                in
                ClientMethodJumps.print_readable children ~find_children:true;
                Printf.printf "\n")
          )))
  | Identify_symbol (line, column) ->
    let path = expect_single_file () in
    let file_input =
      ServerCommandTypes.FileName (Relative_path.to_absolute path)
    in
    let (ctx, entry) =
      Provider_utils.update_context
        ~ctx:(Provider_context.empty ~tcopt)
        ~path
        ~file_input
    in
    let result =
      Provider_utils.with_context ~ctx ~f:(fun () ->
          ServerIdentifyFunction.go_ctx_absolute ~ctx ~entry ~line ~column)
    in
    begin
      match result with
      | [] -> print_endline "None"
      | result -> ClientGetDefinition.print_readable ~short_pos:true result
    end
  | Find_local (line, column) ->
    let filename = expect_single_file () in
    let file = cat (Relative_path.to_absolute filename) in
    let result = ServerFindLocals.go popt filename file line column in
    let print pos = Printf.printf "%s\n" (Pos.string_no_file pos) in
    List.iter result print
  | Outline ->
    iter_over_files (fun filename ->
        let file = cat (Relative_path.to_absolute filename) in
        let results = FileOutline.outline popt file in
        FileOutline.print ~short_pos:true results)
  | Dump_nast ->
    iter_over_files (fun filename ->
        let nasts = create_nasts files_info in
        let nast = Relative_path.Map.find filename nasts in
        Printf.printf "%s\n" (Nast.show_program nast))
  | Dump_tast ->
    let (errors, tasts) =
      compute_tasts_expand_types tcopt files_info files_contents
    in
    (match parse_errors @ Errors.get_error_list errors with
    | [] -> ()
    | errors ->
      Printf.printf "Errors:\n";
      List.iter errors (fun err ->
          List.iter (Errors.to_list err) (fun (pos, msg) ->
              Format.printf "  %a %s" Pos.pp pos msg;
              Format.print_newline ())));
    print_tasts tasts tcopt
  | Check_tast ->
    iter_over_files (fun filename ->
        let files_contents =
          Relative_path.Map.filter files_contents ~f:(fun k _v -> k = filename)
        in
        let (errors, tasts) =
          compute_tasts_expand_types tcopt files_info files_contents
        in
        print_tasts tasts tcopt;
        if not @@ Errors.is_empty errors then (
          print_errors error_format errors max_errors;
          Printf.printf
            "Did not typecheck the TAST as there are typing errors.";
          exit 2
        ) else
          let tast_check_errors = typecheck_tasts tasts tcopt filename in
          print_error_list error_format tast_check_errors max_errors;
          if tast_check_errors <> [] then exit 2)
  | Dump_typed_full_fidelity_json ->
    iter_over_files (fun filename ->
        (*
      Ideally we'd reuse ServerTypedAst.go here. Unfortunately relative file
      paths are not compatible between hh_single_type_check and server modules.
      So we copy here instead.
    *)
        (* get the typed ast *)
        let files_contents =
          Relative_path.Map.filter files_contents ~f:(fun k _v -> k = filename)
        in
        let (_, tasts) = compute_tasts tcopt files_info files_contents in
        (* get the parse tree *)
        let source_text = Full_fidelity_source_text.from_file filename in
        let positioned_tree = PositionedTree.make source_text in
        Relative_path.Map.iter tasts ~f:(fun _k tast ->
            let typed_tree =
              ServerTypedAst.create_typed_parse_tree
                ~filename
                ~positioned_tree
                ~tast
            in
            let result = ServerTypedAst.typed_parse_tree_to_json typed_tree in
            Printf.printf "%s\n" (Hh_json.json_to_string result)))
  | Dump_stripped_tast ->
    iter_over_files (fun filename ->
        let files_contents =
          Relative_path.Map.filter files_contents ~f:(fun k _v -> k = filename)
        in
        let (_, tasts) = compute_tasts tcopt files_info files_contents in
        let tast = Relative_path.Map.find_unsafe tasts filename in
        let nast = Tast.to_nast tast in
        Printf.printf "%s\n" (Nast.show_program nast))
  | Find_refs (line, column) ->
    let filename = expect_single_file () in
    let naming_table = Naming_table.create files_info in
    Naming_table.iter naming_table Typing_deps.update_file;
    let genv = ServerEnvBuild.default_genv in
    let env =
      {
        (ServerEnvBuild.make_env genv.ServerEnv.config) with
        ServerEnv.naming_table;
        ServerEnv.tcopt;
      }
    in
    let filename = Relative_path.to_absolute filename in
    let content = cat filename in
    let include_defs = true in
    let labelled_file =
      ServerCommandTypes.LabelledFileContent { filename; content }
    in
    Option.Monad_infix.(
      ServerCommandTypes.Done_or_retry.(
        let results =
          ServerFindRefs.(
            go_from_file (labelled_file, line, column) env
            >>= fun (name, action) ->
            go action include_defs genv env
            |> map_env ~f:(to_ide name)
            |> snd
            |> function
            | Done r -> r
            | Retry ->
              failwith
              @@ "should only happen with prechecked files "
              ^ "which are not a thing in hh_single_type_check")
        in
        ClientFindRefs.print_ide_readable results))
  | Highlight_refs (line, column) ->
    let filename = expect_single_file () in
    let file = cat (Relative_path.to_absolute filename) in
    let results = ServerHighlightRefs.go (file, line, column) tcopt in
    ClientHighlightRefs.go results ~output_json:false
  | Errors when batch_mode ->
    (* For each file in our batch, run typechecking serially.
      Reset the heaps every time in between. *)
    iter_over_files (fun filename ->
        let oc =
          Out_channel.create
            (Relative_path.to_absolute filename ^ out_extension)
        in
        (* This means builtins had errors, so lets just print those if we see them *)
        if parse_errors <> [] then
          (* This closes the out channel *)
          write_error_list error_format parse_errors oc max_errors
        else (
          Typing_log.out_channel := oc;
          ServerIdeUtils.make_local_changes ();
          let files_contents = Multifile.file_to_files filename in
          let (parse_errors, individual_file_info) =
            parse_name_and_decl popt files_contents
          in
          let errors =
            check_file
              tcopt
              (Errors.get_error_list parse_errors)
              individual_file_info
          in
          write_error_list error_format errors oc max_errors;
          ServerIdeUtils.revert_local_changes ()
        ))
  | Decl_compare when batch_mode ->
    (* For each file in our batch, run typechecking serially.
      Reset the heaps every time in between. *)
    iter_over_files (fun filename ->
        let oc =
          Out_channel.create (Relative_path.to_absolute filename ^ ".decl_out")
        in
        ServerIdeUtils.make_local_changes ();
        let files_contents =
          Relative_path.Map.filter files_contents ~f:(fun k _v -> k = filename)
        in
        let (_, individual_file_info) =
          parse_name_and_decl popt files_contents
        in
        (try
           test_decl_compare
             filename
             popt
             builtins
             files_contents
             individual_file_info;
           Out_channel.output_string oc ""
         with e ->
           let msg = Exn.to_string e in
           Out_channel.output_string oc msg);
        ServerIdeUtils.revert_local_changes ();
        Out_channel.close oc)
  | Errors ->
    (* Don't typecheck builtins *)
    let errors = check_file tcopt parse_errors files_info in
    print_error_list error_format errors max_errors;
    if errors <> [] then exit 2
  | Decl_compare ->
    let filename = expect_single_file () in
    test_decl_compare filename popt builtins files_contents files_info
  | Linearization ->
    if parse_errors <> [] then (
      print_error error_format (List.hd_exn parse_errors);
      exit 2
    );
    let files_info =
      Relative_path.Map.fold
        builtins
        ~f:begin
             fun k _ acc -> Relative_path.Map.remove acc k
           end
        ~init:files_info
    in
    Relative_path.Map.iter files_info ~f:(fun _file info ->
        let { FileInfo.classes; _ } = info in
        List.iter classes ~f:(fun (_, classname) ->
            Printf.printf "Linearization for class %s:\n" classname;
            let linearization = Decl_linearize.get_linearization classname in
            let linearization =
              Sequence.map linearization (fun mro ->
                  let name = mro.Decl_defs.mro_name in
                  let targs =
                    List.map
                      mro.Decl_defs.mro_type_args
                      (Typing_print.full_decl tcopt)
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
                      ( if modifiers = "" then
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
    }
    (popt : TypecheckerOptions.t)
    (hhi_root : Path.t)
    (sienv : SearchUtils.si_env) : unit =
  Ident.track_names := true;
  let builtins =
    if no_builtins then
      Relative_path.Map.empty
    else (
      (* Note that the regular `.hhi` files have already been written to disk
      with `Hhi.get_root ()` *)
      Array.iter magic_builtins ~f:(fun (file_name, file_contents) ->
          let file_path = Path.concat hhi_root file_name in
          let file = Path.to_string file_path in
          Sys_utils.try_touch ~follow_symlinks:true file;
          Sys_utils.write_file ~file file_contents);

      (* Take the builtins (file, contents) array and create relative paths *)
      Caml.Array.fold_left
        begin
          fun acc (f, src) ->
          let f = Path.concat hhi_root f |> Path.to_string in
          Relative_path.Map.add
            acc
            ~key:(Relative_path.create Relative_path.Hhi f)
            ~data:src
        end
        Relative_path.Map.empty
        (Array.append magic_builtins hhi_builtins)
    )
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
      ~f:begin
           fun k src acc -> Relative_path.Map.add acc ~key:k ~data:src
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
  let dbg_deps = Caml.Hashtbl.create 0 in
  ( if mode = Dump_deps then
    (* In addition to actually recording the dependencies in shared memory,
     we build a non-hashed respresentation of the dependency graph
     for printing. *)
    let get_debug_trace root obj =
      let root = Typing_deps.Dep.to_string root in
      let obj = Typing_deps.Dep.to_string obj in
      match Caml.Hashtbl.find_opt dbg_deps obj with
      | Some set -> HashSet.add set root
      | None ->
        let set = HashSet.create 1 in
        HashSet.add set root;
        Caml.Hashtbl.replace dbg_deps obj set
    in
    Typing_deps.add_dependency_callback "get_debug_trace" get_debug_trace );
  let (errors, files_info) = parse_name_and_decl popt to_decl in
  handle_mode
    mode
    files
    tcopt
    popt
    builtins
    files_contents
    files_info
    (Errors.get_error_list errors)
    max_errors
    error_format
    batch_mode
    out_extension
    dbg_deps
    sienv

let main_hack ({ files; mode; tcopt; _ } as opts) (sienv : SearchUtils.si_env)
    : unit =
  (* TODO: We should have a per file config *)
  Sys_utils.signal Sys.sigusr1 (Sys.Signal_handle Typing.debug_print_last_pos);
  EventLogger.init ~exit_on_parent_exit EventLogger.Event_logger_fake 0.0;

  let (_handle : SharedMem.handle) =
    SharedMem.init ~num_workers:0 GlobalConfig.default_sharedmem_config
  in
  Tempfile.with_tempdir (fun hhi_root ->
      Hhi.set_hhi_root_for_unit_test hhi_root;
      Relative_path.set_path_prefix Relative_path.Root (Path.make "/");
      Relative_path.set_path_prefix Relative_path.Hhi hhi_root;
      Relative_path.set_path_prefix Relative_path.Tmp (Path.make "tmp");
      GlobalParserOptions.set tcopt;
      GlobalNamingOptions.set tcopt;
      match mode with
      | Ai ai_options ->
        begin
          match files with
          | [filename] ->
            let filecontents =
              filename
              |> Relative_path.create Relative_path.Dummy
              |> Multifile.file_to_files
            in
            Ai.do_ Typing_check_utils.type_file filecontents ai_options tcopt
          | _ -> die "Ai mode does not support multiple files"
        end
      | _ ->
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
