(*
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Hh_prelude
open String_utils
open Sys_utils
open Typing_env_types
module Inf = Typing_inference_env
module Cls = Decl_provider.Class
module PS = Full_fidelity_positioned_syntax
module PositionedTree = Full_fidelity_syntax_tree.WithSyntax (PS)

(*****************************************************************************)
(* Types, constants *)
(*****************************************************************************)

type mode =
  | Ifc of string * string
  | Color
  | Coverage
  | Cst_search
  | Dump_symbol_info
  | Dump_inheritance
  | Errors
  | Lint
  | Dump_deps
  | Dump_dep_hashes
  | Identify_symbol of int * int
  | Find_local of int * int
  | Get_member of string
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
  | Hover of (int * int) option

type options = {
  files: string list;
  extra_builtins: string list;
  mode: mode;
  error_format: Errors.format;
  no_builtins: bool;
  max_errors: int option;
  tcopt: GlobalOptions.t;
  batch_mode: bool;
  out_extension: string;
  verbosity: int;
  should_print_position: bool;
}

(** If the user passed --root, then all pathnames have to be canonicalized.
The fact of whether they passed --root is kind of stored inside Relative_path
global variables: the Relative_path.(path_of_prefix Root) is either "/"
if they failed to pass something, or the thing that they passed. *)
let use_canonical_filenames () =
  not (String.equal "/" (Relative_path.path_of_prefix Relative_path.Root))

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
    | Errors.Context -> (fun e -> Contextual_error_formatter.to_string e)
    | Errors.Raw -> (fun e -> Errors.to_string e)
    | Errors.Highlighted -> Highlighted_error_formatter.to_string
  in
  let absolute_errors = Errors.to_absolute l in
  Out_channel.output_string oc (formatter absolute_errors)

let write_error_list format errors oc max_errors =
  let (shown_errors, dropped_errors) =
    match max_errors with
    | Some max_errors -> List.split_n errors max_errors
    | None -> (errors, [])
  in
  if not (List.is_empty errors) then (
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
  if not (List.is_empty errors) then (
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
  print_error_list format (Errors.get_sorted_error_list errors) max_errors

let print_errors_if_present (errors : Errors.error list) =
  if not (List.is_empty errors) then (
    let errors_output = Errors.convert_errors_to_string errors in
    Printf.printf "Errors:\n";
    List.iter errors_output ~f:(fun err_output ->
        Printf.printf "  %s\n" err_output)
  )

let comma_string_to_iset (s : string) : ISet.t =
  Str.split (Str.regexp ", *") s |> List.map ~f:int_of_string |> ISet.of_list

let parse_options () =
  let fn_ref = ref [] in
  let extra_builtins = ref [] in
  let usage = Printf.sprintf "Usage: %s filename\n" Sys.argv.(0) in
  let mode = ref Errors in
  let no_builtins = ref false in
  let line = ref 0 in
  let log_key = ref "" in
  let log_levels = ref SMap.empty in
  let max_errors = ref None in
  let batch_mode = ref false in
  let set_mode x () =
    match !mode with
    | Errors -> mode := x
    | _ -> raise (Arg.Bad "only a single mode should be specified")
  in
  let ifc_mode = ref "" in
  let set_ifc lattice =
    set_mode (Ifc (!ifc_mode, lattice)) ();
    batch_mode := true
  in
  let error_format = ref Errors.Highlighted in
  let forbid_nullable_cast = ref false in
  let deregister_attributes = ref None in
  let disallow_array_typehint = ref None in
  let disallow_array_literal = ref None in
  let dynamic_view = ref None in
  let auto_namespace_map = ref None in
  let log_inference_constraints = ref None in
  let timeout = ref None in
  let disallow_byref_dynamic_calls = ref (Some false) in
  let disallow_byref_calls = ref (Some false) in
  let set_bool x () = x := Some true in
  let set_bool_ x () = x := true in
  let set_float_ x f = x := f in
  let shallow_class_decl = ref false in
  let out_extension = ref ".out" in
  let like_type_hints = ref false in
  let union_intersection_type_hints = ref false in
  let call_coeffects = ref true in
  let local_coeffects = ref true in
  let strict_contexts = ref true in
  let like_casts = ref false in
  let simple_pessimize = ref 0.0 in
  let complex_coercion = ref false in
  let disable_partially_abstract_typeconsts = ref false in
  let disallow_partially_abstract_typeconst_definitions = ref false in
  let rust_parser_errors = ref false in
  let symbolindex_file = ref None in
  let check_xhp_attribute = ref false in
  let check_redundant_generics = ref false in
  let disallow_invalid_arraykey_constraint = ref None in
  let enable_class_level_where_clauses = ref false in
  let disallow_trait_reuse = ref None in
  let disable_legacy_soft_typehints = ref false in
  let allow_new_attribute_syntax = ref false in
  let allow_toplevel_requires = ref false in
  let global_inference = ref false in
  let ordered_solving = ref false in
  let reinfer_types = ref [] in
  let const_static_props = ref false in
  let disable_legacy_attribute_syntax = ref false in
  let const_attribute = ref false in
  let const_default_func_args = ref false in
  let const_default_lambda_args = ref false in
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
  let disable_modes = ref false in
  let disable_hh_ignore_error = ref false in
  let enable_systemlib_annotations = ref false in
  let enable_higher_kinded_types = ref false in
  let allowed_fixme_codes_strict = ref None in
  let allowed_fixme_codes_partial = ref None in
  let codes_not_raised_partial = ref None in
  let allowed_decl_fixme_codes = ref None in
  let method_call_inference = ref false in
  let report_pos_from_reason = ref false in
  let enable_sound_dynamic = ref false in
  let disallow_hash_comments = ref false in
  let disallow_fun_and_cls_meth_pseudo_funcs = ref false in
  let disallow_inst_meth = ref false in
  let use_direct_decl_parser = ref false in
  let disable_enum_classes = ref false in
  let enable_enum_supertyping = ref false in
  let interpret_soft_types_as_like_types = ref false in
  let enable_strict_string_concat_interp = ref false in
  let ignore_unsafe_cast = ref false in
  let math_new_code = ref false in
  let typeconst_concrete_concrete_error = ref false in
  let meth_caller_only_public_visibility = ref true in
  let require_extends_implements_ancestors = ref false in
  let strict_value_equality = ref false in
  let naming_table = ref None in
  let root = ref None in
  let sharedmem_config = ref SharedMem.default_config in
  let print_position = ref true in
  let enforce_sealed_subclasses = ref false in
  let options =
    [
      ( "--no-print-position",
        Arg.Unit (fun _ -> print_position := false),
        " Don't print positions while printing TASTs and NASTs" );
      ( "--naming-table",
        Arg.String (fun s -> naming_table := Some s),
        " Naming table, to look up undefined symbols; needs --root" );
      ( "--root",
        Arg.String (fun s -> root := Some s),
        " Root for where to look up undefined symbols; needs --naming-table" );
      ( "--extra-builtin",
        Arg.String (fun f -> extra_builtins := f :: !extra_builtins),
        " HHI file to parse and declare" );
      ( "--ifc",
        Arg.Tuple [Arg.String (fun m -> ifc_mode := m); Arg.String set_ifc],
        " Run the flow analysis" );
      ( "--deregister-attributes",
        Arg.Unit (set_bool deregister_attributes),
        " Ignore all functions with attribute '__PHPStdLib'" );
      ( "--auto-namespace-map",
        Arg.String
          (fun m ->
            auto_namespace_map :=
              Some (ServerConfig.convert_auto_namespace_to_map m)),
        " Alias namespaces" );
      ( "--no-call-coeffects",
        Arg.Unit (fun () -> call_coeffects := false),
        " Turns off call coeffects" );
      ( "--no-local-coeffects",
        Arg.Unit (fun () -> local_coeffects := false),
        " Turns off local coeffects" );
      ( "--no-strict-contexts",
        Arg.Unit (fun () -> strict_contexts := false),
        " Do not enforce contexts to be defined within Contexts namespace" );
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
            | "highlighted" -> error_format := Errors.Highlighted
            | _ -> print_string "Warning: unrecognized error format.\n"),
        "<raw|context|highlighted> Error formatting style" );
      ("--lint", Arg.Unit (set_mode Lint), " Produce lint errors");
      ( "--no-builtins",
        Arg.Set no_builtins,
        " Don't use builtins (e.g. ConstSet); implied by --root" );
      ( "--out-extension",
        Arg.String (fun s -> out_extension := s),
        " output file extension (default .out)" );
      ("--dump-deps", Arg.Unit (set_mode Dump_deps), " Print dependencies");
      ( "--dump-dep-hashes",
        Arg.Unit (set_mode Dump_dep_hashes),
        " Print dependency hashes" );
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
        " Global type inference to infer missing type annotations." );
      ( "--ordered-solving",
        Arg.Set ordered_solving,
        " Optimized solver for type variables. Experimental." );
      ( "--reinfer-types",
        Arg.String (fun s -> reinfer_types := Str.split (Str.regexp ", *") s),
        " List of type hint to be ignored and infered again using global inference."
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
      ( "--get-member",
        Arg.String
          (fun class_and_member_id ->
            set_mode (Get_member class_and_member_id) ()),
        " Given ClassName::MemberName, fetch the decl of members with that name and print them."
      );
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
      ( "--disallow-partially-abstract-typeconst-definitions",
        Arg.Set disallow_partially_abstract_typeconst_definitions,
        " Raise error when partially abstract type constant is defined" );
      ( "--rust-parser-errors",
        Arg.Bool (fun x -> rust_parser_errors := x),
        " Use rust parser error checker" );
      ( "--symbolindex-file",
        Arg.String (fun str -> symbolindex_file := Some str),
        " Load the symbol index from this file" );
      ( "--enable-class-level-where-clauses",
        Arg.Set enable_class_level_where_clauses,
        " Enables support for class-level where clauses" );
      ( "--disallow-trait-reuse",
        Arg.Unit (set_bool disallow_trait_reuse),
        " Forbid a class from using a trait already used in a parent class" );
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
      ( "--const-default-func-args",
        Arg.Set const_default_func_args,
        " Statically check default function arguments are constant initializers"
      );
      ( "--const-default-lambda-args",
        Arg.Set const_default_lambda_args,
        " Statically check default lambda args are constant."
        ^ " Produces a subset of errors of const-default-func-args" );
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
        " glean repo name" );
      ( "--disallow-func-ptrs-in-constants",
        Arg.Set disallow_func_ptrs_in_constants,
        " Disallow use of HH\\fun and HH\\class_meth in constants and constant initializers"
      );
      ( "--disallow-php-lambdas",
        Arg.Set error_php_lambdas,
        " Disallow php style anonymous functions." );
      ( "--disallow-discarded-nullable-awaitables",
        Arg.Set disallow_discarded_nullable_awaitables,
        " Error on using discarded nullable awaitables" );
      ( "--disable-xhp-element-mangling",
        Arg.Set disable_xhp_element_mangling,
        " Disable mangling of XHP elements :foo. That is, :foo:bar is now \\foo\\bar, not xhp_foo__bar"
      );
      ( "--disable-xhp-children-declarations",
        Arg.Set disable_xhp_children_declarations,
        " Disable XHP children declarations, e.g. children (foo, bar+)" );
      ( "--enable-xhp-class-modifier",
        Arg.Set enable_xhp_class_modifier,
        " Enable the XHP class modifier, xhp class name {} will define an xhp class."
      );
      ( "--verbose",
        Arg.Int (fun v -> verbosity := v),
        " Verbosity as an integer." );
      ("--disable-modes", Arg.Set disable_modes, " Treat partial as strict");
      ( "--disable-hh-ignore-error",
        Arg.Set disable_hh_ignore_error,
        " Treat HH_IGNORE_ERROR comments as normal comments" );
      ( "--enable-systemlib-annotations",
        Arg.Set enable_systemlib_annotations,
        " Enable systemlib annotations" );
      ( "--enable-higher-kinded-types",
        Arg.Set enable_higher_kinded_types,
        " Enable support for higher-kinded types" );
      ( "--allowed-fixme-codes-strict",
        Arg.String
          (fun s -> allowed_fixme_codes_strict := Some (comma_string_to_iset s)),
        " List of fixmes that are allowed in strict mode." );
      ( "--allowed-fixme-codes-partial",
        Arg.String
          (fun s ->
            allowed_fixme_codes_partial := Some (comma_string_to_iset s)),
        " List of fixmes that are allowed in partial mode." );
      ( "--codes-not-raised-partial",
        Arg.String
          (fun s -> codes_not_raised_partial := Some (comma_string_to_iset s)),
        " List of error codes that are not raised in partial mode." );
      ( "--allowed-decl-fixme-codes",
        Arg.String
          (fun s -> allowed_decl_fixme_codes := Some (comma_string_to_iset s)),
        " List of fixmes that are allowed in declarations." );
      ( "--method-call-inference",
        Arg.Set method_call_inference,
        " Infer constraints for method calls. NB: incompatible with like types."
      );
      ( "--report-pos-from-reason",
        Arg.Set report_pos_from_reason,
        " Flag errors whose position is derived from reason information in types."
      );
      ( "--enable-sound-dynamic-type",
        Arg.Set enable_sound_dynamic,
        " Enforce sound dynamic types.  Experimental." );
      ( "--disallow-hash-comments",
        Arg.Set disallow_hash_comments,
        " Disallow #-style comments (besides hashbangs)." );
      ( "--disallow-fun-and-cls-meth-pseudo-funcs",
        Arg.Set disallow_fun_and_cls_meth_pseudo_funcs,
        " Disable parsing of fun() and class_meth()." );
      ( "--disallow-inst-meth",
        Arg.Set disallow_inst_meth,
        " Disable parsing of inst_meth()." );
      ( "--use-direct-decl-parser",
        Arg.Set use_direct_decl_parser,
        " Use direct decl parser" );
      ( "--disable-enum-classes",
        Arg.Set disable_enum_classes,
        " Disable the enum classes extension." );
      ( "--enable-enum-supertyping",
        Arg.Set enable_enum_supertyping,
        " Enable the enum supertyping extension." );
      ( "--interpret-soft-types-as-like-types",
        Arg.Set interpret_soft_types_as_like_types,
        " Types declared with <<__Soft>> (runtime logs but doesn't throw) become like types."
      );
      ( "--enable-strict-string-concat-interp",
        Arg.Set enable_strict_string_concat_interp,
        " Require arguments are arraykey types in string concatenation and interpolation."
      );
      ( "--ignore-unsafe-cast",
        Arg.Set ignore_unsafe_cast,
        " Ignore unsafe_cast and retain the original type of the expression" );
      ( "--math-new-code",
        Arg.Set math_new_code,
        " Use a new error code for math operations: addition, subtraction, division, multiplication, exponentiation"
      );
      ( "--typeconst-concrete-concrete-error",
        Arg.Set typeconst_concrete_concrete_error,
        " Raise an error when a concrete type constant is overridden by a concrete type constant in a child class."
      );
      ( "--meth-caller-only-public-visibility",
        Arg.Bool (fun x -> meth_caller_only_public_visibility := x),
        " Controls whether meth_caller can be used on non-public methods" );
      ( "--hover",
        Arg.Tuple
          [
            Arg.Int (fun x -> line := x);
            Arg.Int (fun column -> set_mode (Hover (Some (!line, column))) ());
          ],
        "<pos> Display hover tooltip" );
      ( "--hover-at-caret",
        Arg.Unit (fun () -> set_mode (Hover None) ()),
        " Show the hover information indicated by // ^ hover-at-caret" );
      ( "--require-extends-implements-ancestors",
        Arg.Set require_extends_implements_ancestors,
        " Consider `require extends` and `require implements` as ancestors when checking a class"
      );
      ( "--strict-value-equality",
        Arg.Set strict_value_equality,
        " Emit an error when \"==\" or \"!=\" is used to compare values that are incompatible types."
      );
      ( "--enable-sealed-subclasses",
        Arg.Set enforce_sealed_subclasses,
        " Require all __Sealed arguments to be subclasses" );
    ]
  in

  (* Sanity check that all option descriptions are well-formed. *)
  List.iter options ~f:(fun (_, _, description) ->
      if
        String.is_prefix description ~prefix:" "
        || String.is_prefix description ~prefix:"<"
      then
        ()
      else
        failwith
          (Printf.sprintf
             "Descriptions should start with <foo> or a leading space, got: %S"
             description));

  let options = Arg.align ~limit:25 options in
  Arg.parse options (fun fn -> fn_ref := fn :: !fn_ref) usage;
  let fns =
    match (!fn_ref, !mode) with
    | ([], Get_member _) -> []
    | ([], _) -> die usage
    | (x, _) -> x
  in
  let is_ifc_mode =
    match !mode with
    | Ifc _ -> true
    | _ -> false
  in

  (* --root implies certain things... *)
  let root =
    match !root with
    | None -> Path.make "/" (* if none specified, we use this dummy *)
    | Some root ->
      if Option.is_none !naming_table then
        failwith "--root needs --naming-table";
      (* builtins are already provided by project at --root, so we shouldn't provide our own *)
      no_builtins := true;
      (* Following will throw an exception if .hhconfig not found *)
      let (_config_hash, config) =
        Config_file.parse_hhconfig
          ~silent:true
          (Filename.concat root Config_file.file_path_relative_to_repo_root)
      in
      (* We will pick up values from .hhconfig, unless they've been overridden at the command-line. *)
      if Option.is_none !auto_namespace_map then
        auto_namespace_map :=
          SMap.find_opt "auto_namespace_map" config
          |> Option.map ~f:ServerConfig.convert_auto_namespace_to_map;
      if Option.is_none !allowed_fixme_codes_strict then
        allowed_fixme_codes_strict :=
          SMap.find_opt "allowed_fixme_codes_strict" config
          |> Option.map ~f:comma_string_to_iset;
      sharedmem_config :=
        ServerConfig.make_sharedmem_config
          config
          (ServerArgs.default_options ~root)
          ServerLocalConfig.default;
      (* Path.make canonicalizes it, i.e. resolves symlinks *)
      Path.make root
  in

  let tcopt =
    GlobalOptions.make
      ?po_disable_array_typehint:(Some false)
      ?po_deregister_php_stdlib:!deregister_attributes
      ?tco_disallow_array_typehint:!disallow_array_typehint
      ?tco_disallow_array_literal:!disallow_array_literal
      ?tco_dynamic_view:!dynamic_view
      ?tco_log_inference_constraints:!log_inference_constraints
      ?tco_timeout:!timeout
      ?po_auto_namespace_map:!auto_namespace_map
      ?tco_disallow_byref_dynamic_calls:!disallow_byref_dynamic_calls
      ?tco_disallow_byref_calls:!disallow_byref_calls
      ~allowed_fixme_codes_strict:
        (Option.value !allowed_fixme_codes_strict ~default:ISet.empty)
      ~allowed_fixme_codes_partial:
        (Option.value !allowed_fixme_codes_partial ~default:ISet.empty)
      ~codes_not_raised_partial:
        (Option.value !codes_not_raised_partial ~default:ISet.empty)
      ?tco_disallow_invalid_arraykey_constraint:
        !disallow_invalid_arraykey_constraint
      ?tco_disallow_trait_reuse:!disallow_trait_reuse
      ~tco_check_xhp_attribute:!check_xhp_attribute
      ~tco_check_redundant_generics:!check_redundant_generics
      ~tco_shallow_class_decl:!shallow_class_decl
      ~tco_like_type_hints:!like_type_hints
      ~tco_union_intersection_type_hints:!union_intersection_type_hints
      ~tco_strict_contexts:!strict_contexts
      ~tco_coeffects:!call_coeffects
      ~tco_coeffects_local:!local_coeffects
      ~tco_like_casts:!like_casts
      ~tco_simple_pessimize:!simple_pessimize
      ~tco_complex_coercion:!complex_coercion
      ~tco_disable_partially_abstract_typeconsts:
        !disable_partially_abstract_typeconsts
      ~tco_disallow_partially_abstract_typeconst_definitions:
        !disallow_partially_abstract_typeconst_definitions
      ~log_levels:!log_levels
      ~po_rust_parser_errors:!rust_parser_errors
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
      ~po_const_default_func_args:!const_default_func_args
      ~po_const_default_lambda_args:!const_default_lambda_args
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
      ~po_disable_modes:!disable_modes
      ~po_disable_hh_ignore_error:!disable_hh_ignore_error
      ~tco_enable_systemlib_annotations:!enable_systemlib_annotations
      ~tco_higher_kinded_types:!enable_higher_kinded_types
      ~po_allowed_decl_fixme_codes:
        (Option.value !allowed_decl_fixme_codes ~default:ISet.empty)
      ~po_allow_unstable_features:true
      ~tco_method_call_inference:!method_call_inference
      ~tco_report_pos_from_reason:!report_pos_from_reason
      ~tco_enable_sound_dynamic:!enable_sound_dynamic
      ~po_disallow_hash_comments:!disallow_hash_comments
      ~po_disallow_fun_and_cls_meth_pseudo_funcs:
        !disallow_fun_and_cls_meth_pseudo_funcs
      ~po_disallow_inst_meth:!disallow_inst_meth
      ~tco_ifc_enabled:
        ( if is_ifc_mode then
          ["/"]
        else
          [] )
      ~tco_use_direct_decl_parser:!use_direct_decl_parser
      ~po_enable_enum_classes:(not !disable_enum_classes)
      ~po_enable_enum_supertyping:!enable_enum_supertyping
      ~po_interpret_soft_types_as_like_types:!interpret_soft_types_as_like_types
      ~tco_enable_strict_string_concat_interp:
        !enable_strict_string_concat_interp
      ~tco_ignore_unsafe_cast:!ignore_unsafe_cast
      ~tco_math_new_code:!math_new_code
      ~tco_typeconst_concrete_concrete_error:!typeconst_concrete_concrete_error
      ~tco_meth_caller_only_public_visibility:
        !meth_caller_only_public_visibility
      ~tco_require_extends_implements_ancestors:
        !require_extends_implements_ancestors
      ~tco_strict_value_equality:!strict_value_equality
      ~tco_enforce_sealed_subclasses:!enforce_sealed_subclasses
      ()
  in
  Errors.allowed_fixme_codes_strict :=
    GlobalOptions.allowed_fixme_codes_strict tcopt;
  Errors.allowed_fixme_codes_partial :=
    GlobalOptions.allowed_fixme_codes_partial tcopt;
  Errors.codes_not_raised_partial :=
    GlobalOptions.codes_not_raised_partial tcopt;
  Errors.report_pos_from_reason :=
    GlobalOptions.tco_report_pos_from_reason tcopt;
  let tcopt =
    {
      tcopt with
      GlobalOptions.tco_experimental_features =
        SSet.filter
          begin
            fun x ->
            if
              String.equal x GlobalOptions.tco_experimental_forbid_nullable_cast
            then
              !forbid_nullable_cast
            (* Only enable infer flows in IFC mode *)
            else if String.equal x GlobalOptions.tco_experimental_infer_flows
            then
              is_ifc_mode
            else
              true
          end
          tcopt.GlobalOptions.tco_experimental_features;
    }
  in
  ( {
      files = fns;
      extra_builtins = !extra_builtins;
      mode = !mode;
      no_builtins = !no_builtins;
      max_errors = !max_errors;
      error_format = !error_format;
      tcopt;
      batch_mode = !batch_mode;
      out_extension = !out_extension;
      verbosity = !verbosity;
      should_print_position = !print_position;
    },
    root,
    !naming_table,
    !sharedmem_config )

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
    print_endline (String.map s ~f:(const '='));
    print_endline s;
    print_endline (String.map s ~f:(const '='))
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
  let gienv = Option.map gienv ~f:solve_global_inference_env in
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
          ( errors @ Errors.get_sorted_error_list new_errors,
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

(** This is an almost-pure function which returns what we get out of parsing.
The only side-effect it has is on the global errors list. *)
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
  (parsed_files, files_info)

(** This function is used for gathering naming and parsing errors,
and the side-effect of updating the global reverse naming table (and
picking up duplicate-name errors along the way), and for the side effect
of updating the decl heap (and picking up decling errors along the way). *)
let parse_name_and_decl ctx files_contents =
  Errors.do_ (fun () ->
      let (parsed_files, files_info) = parse_and_name ctx files_contents in
      Relative_path.Map.iter files_info ~f:(fun fn fileinfo ->
          let (errors, _failed_naming_fns) =
            Naming_global.ndecl_file_error_if_already_bound ctx fn fileinfo
          in
          Errors.merge_into_current errors);
      Relative_path.Map.iter parsed_files ~f:(fun fn _ ->
          Errors.run_in_context fn Errors.Decl (fun () ->
              Decl.make_env ~sh:SharedMem.Uses ctx fn));

      files_info)

(** This function is used solely for its side-effect of putting decls into shared-mem *)
let add_decls_to_heap ctx files_contents =
  Errors.ignore_ (fun () ->
      let (parsed_files, _files_info) = parse_and_name ctx files_contents in
      Relative_path.Map.iter parsed_files ~f:(fun fn _ ->
          Errors.run_in_context fn Errors.Decl (fun () ->
              Decl.make_env ~sh:SharedMem.Uses ctx fn)));
  ()

(** This function doesn't have side-effects. Its sole job is to return shallow decls. *)
let get_shallow_decls ctx filename file_contents :
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
  let decls1 = get_shallow_decls popt filename contents1 in
  let decls2 = get_shallow_decls popt filename contents2 in
  let decls =
    SMap.merge (fun _ a b -> Some (a, b)) decls1 decls2 |> SMap.bindings
  in
  let diffs =
    List.map decls ~f:(fun (cid, old_and_new) ->
        ( Utils.strip_ns cid,
          match old_and_new with
          | (Some c1, Some c2) -> Shallow_class_diff.diff_class c1 c2
          | _ -> ClassDiff.Major_change ))
  in
  List.iter diffs ~f:(fun (cid, diff) ->
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
      && String.equal (String.sub contents ~pos:after_shebang ~len:2) "<?"
    then
      String.index_from_exn contents after_shebang '\n' + 1
    else
      after_shebang
  in
  String.sub contents ~pos:0 ~len:after_header
  ^ "\n"
  ^ String.sub
      contents
      ~pos:after_header
      ~len:(String.length contents - after_header)

(* Might raise {!SharedMem.Shared_mem_not_found} *)
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
  if Poly.(t1 <> t2) then fail_comparison "typedefs"

let compare_funs f1 f2 =
  let f1 = Decl_pos_utils.NormalizeSig.fun_elt f1 in
  let f2 = Decl_pos_utils.NormalizeSig.fun_elt f2 in
  if Poly.(f1 <> f2) then fail_comparison "funs"

let compare_classes mode c1 c2 =
  if Decl_compare.class_big_diff c1 c2 then fail_comparison "class_big_diff";

  let c1 = Decl_pos_utils.NormalizeSig.class_type c1 in
  let c2 = Decl_pos_utils.NormalizeSig.class_type c2 in
  let (_, is_unchanged) =
    Decl_compare.ClassDiff.compare mode c1.Decl_defs.dc_name c1 c2
  in
  if not is_unchanged then fail_comparison "ClassDiff";

  let (_, is_unchanged) = Decl_compare.ClassEltDiff.compare mode c1 c2 in
  match is_unchanged with
  | `Changed -> fail_comparison "ClassEltDiff"
  | _ -> ()

let test_decl_compare ctx filenames builtins files_contents files_info =
  (* skip some edge cases that we don't handle now... ugly! *)
  if String.equal (Relative_path.suffix filenames) "capitalization3.php" then
    ()
  else if String.equal (Relative_path.suffix filenames) "capitalization4.php"
  then
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
      | Some info -> SSet.of_list @@ List.map info.FileInfo.classes ~f:snd
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
    add_decls_to_heap ctx files_contents;
    let (typedefs2, funs2, classes2) = get_decls defs in
    let deps_mode = Provider_context.get_deps_mode ctx in
    List.iter2_exn typedefs1 typedefs2 ~f:compare_typedefs;
    List.iter2_exn funs1 funs2 ~f:compare_funs;
    List.iter2_exn classes1 classes2 ~f:(compare_classes deps_mode);
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
        Relative_path.Map.merge nasts interesting_files ~f:(fun _k nast x ->
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

(* Given source code containing the string "^ hover-at-caret", return
   the line and column of the position indicated. *)
let hover_at_caret_pos (src : string) : int * int =
  let lines = String.split_lines src in
  match
    List.findi lines ~f:(fun _ line ->
        String.is_substring line ~substring:"^ hover-at-caret")
  with
  | Some (line_num, line_src) ->
    let col_num =
      String.lfindi line_src ~f:(fun _ c ->
          match c with
          | '^' -> true
          | _ -> false)
    in
    (line_num, Option.value_exn col_num + 1)
  | None ->
    failwith "Could not find any occurrence of ^ hover-at-caret in source code"

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
          ~f:(merge_global_inference_env_in_tast gienv ctx)
      in
      (tasts, Some gi_solved)
  in
  let tasts = Relative_path.Map.map tasts ~f:(Tast_expand.expand_program ctx) in
  (errors, tasts, gi_solved)

let print_nasts ~should_print_position nasts filenames =
  List.iter filenames ~f:(fun filename ->
      match Relative_path.Map.find_opt nasts filename with
      | None ->
        Printf.eprintf
          "Could not find nast for file %s\n"
          (Relative_path.show filename);
        Printf.eprintf "Available nasts:\n";
        Relative_path.Map.iter nasts ~f:(fun path _ ->
            Printf.eprintf "  %s\n" (Relative_path.show path))
      | Some nast ->
        Printf.printf "\n\n%s:\n\n" (Relative_path.show filename);
        if should_print_position then
          Naming_ast_print.print_nast nast
        else
          Naming_ast_print.print_nast_without_position nast)

let print_tasts ~should_print_position tasts ctx =
  Relative_path.Map.iter tasts ~f:(fun _k (tast : Tast.program) ->
      if should_print_position then
        Typing_ast_print.print_tast ctx tast
      else
        Typing_ast_print.print_tast_without_position ctx tast)

let typecheck_tasts tasts tcopt (filename : Relative_path.t) =
  let env = Typing_env.empty tcopt filename ~droot:None in
  let tasts = Relative_path.Map.values tasts in
  let typecheck_tast tast =
    Errors.get_sorted_error_list (Tast_typecheck.check env tast)
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
  let json_opt = Glean_dependency_graph_convert.convert_deps_to_json ~deps in
  match json_opt with
  | Some json_obj ->
    Printf.printf "%s\n" (Hh_json.json_to_string ~pretty:true json_obj)
  | None -> Printf.printf "No dependencies\n"

let dump_dep_hashes (nast : Nast.program) : unit =
  let process_variant x =
    let open Typing_deps in
    let dep = Dep.make Typing_deps_mode.Hash64Bit x in
    Printf.printf "%s %s\n" (Dep.to_hex_string dep) (Dep.variant_to_string x)
  in
  let handler =
    let open Typing_deps.Dep in
    let open Aast in
    object
      inherit [_] Aast.iter as super

      method! on_fun_ env x =
        process_variant @@ Fun (snd x.f_name);
        super#on_fun_ env x

      method! on_method_ cls x =
        process_variant
        @@
        if x.m_static then
          SMethod (Option.value_exn cls, snd x.m_name)
        else
          Method (Option.value_exn cls, snd x.m_name);
        super#on_method_ cls x

      method! on_class_ _cls x =
        process_variant @@ Type (snd x.c_name);
        process_variant @@ Cstr (snd x.c_name);
        process_variant @@ Extends (snd x.c_name);
        process_variant @@ AllMembers (snd x.c_name);
        super#on_class_ (Some (snd x.c_name)) x

      method! on_class_const cls x =
        process_variant @@ Const (Option.value_exn cls, snd x.cc_id);
        super#on_class_const cls x

      method! on_class_typeconst_def cls x =
        process_variant @@ Const (Option.value_exn cls, snd x.c_tconst_name);
        super#on_class_typeconst_def cls x

      method! on_class_var cls x =
        process_variant
        @@
        if x.cv_is_static then
          SProp (Option.value_exn cls, snd x.cv_id)
        else
          Prop (Option.value_exn cls, snd x.cv_id);
        super#on_class_var cls x

      method! on_typedef _cls x =
        process_variant @@ Type (snd x.t_name);
        super#on_typedef (Some (snd x.t_name)) x

      method! on_gconst cls x =
        process_variant @@ GConst (snd x.cst_name);
        super#on_gconst cls x

      method! on_record_def _cls x =
        process_variant @@ Type (snd x.rd_name);
        super#on_record_def (Some (snd x.rd_name)) x
    end
  in
  handler#on_program None nast

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
    ~should_print_position
    ~verbosity =
  let expect_single_file () : Relative_path.t =
    match filenames with
    | [x] -> x
    | _ -> die "Only single file expected"
  in
  let iter_over_files f : unit = List.iter filenames ~f in
  match mode with
  | Ifc (mode, lattice) ->
    (* Timing mode is same as check except we print out the time it takes to
       analyse the file. *)
    let (mode, should_time) =
      if String.equal mode "time" then
        ("check", true)
      else
        (mode, false)
    in
    let ifc_opts =
      match Ifc_options.parse ~mode ~lattice with
      | Ok opts -> opts
      | Error e -> die ("could not parse IFC options: " ^ e)
    in
    let time f =
      let start_time = Unix.gettimeofday () in
      let result = Lazy.force f in
      let elapsed_time = Unix.gettimeofday () -. start_time in
      if should_time then Printf.printf "Duration: %f\n" elapsed_time;
      result
    in
    let print_errors = List.iter ~f:(print_error ~oc:stdout error_format) in
    let process_file filename =
      Printf.printf
        "=== IFC analysis results for %s\n%!"
        (Relative_path.to_absolute filename);
      let files_contents = Multifile.file_to_files filename in
      let (parse_errors, file_info) = parse_name_and_decl ctx files_contents in
      let check_errors =
        let error_list = Errors.get_sorted_error_list parse_errors in
        check_file ~verbosity ctx error_list file_info error_format max_errors
      in
      if not (List.is_empty check_errors) then
        print_errors check_errors
      else
        try
          let ifc_errors = time @@ lazy (Ifc_main.do_ ifc_opts file_info ctx) in
          if not (List.is_empty ifc_errors) then print_errors ifc_errors
        with exc ->
          (* get the backtrace before doing anything else to be sure
             to get the one corresponding to exc *)
          let backtrace = Stdlib.Printexc.get_backtrace () in
          Stdlib.Printexc.register_printer (function
              | Ifc_types.IFCError err ->
                Some
                  ( Printf.sprintf "IFCError(%s)"
                  @@ Ifc_types.show_ifc_error_ty err )
              | _ -> None);
          Printf.printf
            "Uncaught exception: %s\n%s"
            (Stdlib.Printexc.to_string exc)
            backtrace
    in
    iter_over_files (fun filename ->
        Provider_utils.respect_but_quarantine_unsaved_changes ~ctx ~f:(fun () ->
            process_file filename))
  | Color ->
    Relative_path.Map.iter files_info ~f:(fun fn fileinfo ->
        if Relative_path.Map.mem builtins fn then
          ()
        else
          let (tast, _) = Typing_check_utils.type_file ctx fn fileinfo in
          let result = Coverage_level.get_levels ctx tast fn in
          match result with
          | Ok result -> print_colored fn result
          | Error () ->
            failwith
              ("HH_FIXMEs not found for path " ^ Relative_path.to_absolute fn))
  | Coverage ->
    Relative_path.Map.iter files_info ~f:(fun fn fileinfo ->
        if Relative_path.Map.mem builtins fn then
          ()
        else
          let (tast, _) = Typing_check_utils.type_file ctx fn fileinfo in
          let type_acc =
            ServerCoverageMetricUtils.accumulate_types ctx tast fn
          in
          print_coverage type_acc)
  | Cst_search ->
    let path = expect_single_file () in
    let (ctx, entry) = Provider_context.add_entry_if_missing ~ctx ~path in
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
        | Some _fileinfo ->
          let raw_result = SymbolInfoServiceUtils.helper ctx [] [filename] in
          let result = SymbolInfoServiceUtils.format_result raw_result in
          let result_json =
            ServerCommandTypes.Symbol_info_service.to_json result
          in
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
    if not (List.is_empty lint_errors) then (
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
      ServerLintTypes.output_text stdout lint_errors error_format;
      exit 2
    ) else
      Printf.printf "No lint errors\n"
  | Dump_deps ->
    Relative_path.Map.iter files_info ~f:(fun fn fileinfo ->
        ignore @@ Typing_check_utils.check_defs ctx fn fileinfo);
    if Hashtbl.length dbg_deps > 0 then dump_debug_deps dbg_deps
  | Dump_dep_hashes ->
    iter_over_files (fun _ ->
        let nasts = create_nasts ctx files_info in
        Relative_path.Map.iter nasts ~f:(fun _ nast -> dump_dep_hashes nast))
  | Dump_glean_deps ->
    Relative_path.Map.iter files_info ~f:(fun fn fileinfo ->
        ignore @@ Typing_check_utils.check_defs ctx fn fileinfo);
    dump_debug_glean_deps dbg_glean_deps
  | Dump_inheritance ->
    let open ServerCommandTypes.Method_jumps in
    let deps_mode = Provider_context.get_deps_mode ctx in
    let naming_table = Naming_table.create files_info in
    Naming_table.iter naming_table ~f:(Typing_deps.Files.update_file deps_mode);
    Naming_table.iter naming_table ~f:(fun fn fileinfo ->
        if Relative_path.Map.mem builtins fn then
          ()
        else (
          List.iter fileinfo.FileInfo.classes ~f:(fun (_p, class_) ->
              Printf.printf
                "Ancestors of %s and their overridden methods:\n"
                class_;
              let ancestors =
                (* Might raise {!Naming_table.File_info_not_found} *)
                MethodJumps.get_inheritance
                  ctx
                  class_
                  ~filter:No_filter
                  ~find_children:false
                  naming_table
                  None
              in
              ServerCommandTypes.Method_jumps.print_readable
                ancestors
                ~find_children:false;
              Printf.printf "\n");
          Printf.printf "\n";
          List.iter fileinfo.FileInfo.classes ~f:(fun (_p, class_) ->
              Printf.printf
                "Children of %s and the methods they override:\n"
                class_;
              let children =
                (* Might raise {!Naming_table.File_info_not_found} *)
                MethodJumps.get_inheritance
                  ctx
                  class_
                  ~filter:No_filter
                  ~find_children:true
                  naming_table
                  None
              in
              ServerCommandTypes.Method_jumps.print_readable
                children
                ~find_children:true;
              Printf.printf "\n")
        ))
  | Identify_symbol (line, column) ->
    let path = expect_single_file () in
    let (ctx, entry) = Provider_context.add_entry_if_missing ~ctx ~path in
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
    let (ctx, entry) =
      Provider_context.add_entry_if_missing ~ctx ~path:filename
    in
    let result = ServerFindLocals.go ~ctx ~entry ~line ~char in
    let print pos = Printf.printf "%s\n" (Pos.string_no_file pos) in
    List.iter result ~f:print
  | Outline ->
    iter_over_files (fun filename ->
        let file = cat (Relative_path.to_absolute filename) in
        let results =
          FileOutline.outline (Provider_context.get_popt ctx) file
        in
        FileOutline.print ~short_pos:true results)
  | Dump_nast ->
    let nasts = create_nasts ctx files_info in
    print_nasts
      ~should_print_position
      nasts
      (Relative_path.Map.keys files_contents)
  | Dump_tast ->
    let (errors, tasts, _gi_solved) =
      compute_tasts_expand_types ctx ~verbosity files_info files_contents
    in
    print_errors_if_present (parse_errors @ Errors.get_sorted_error_list errors);
    print_tasts ~should_print_position tasts ctx
  | Check_tast ->
    iter_over_files (fun filename ->
        let files_contents =
          Relative_path.Map.filter files_contents ~f:(fun k _v ->
              Relative_path.equal k filename)
        in
        let (errors, tasts, _gi_solved) =
          compute_tasts_expand_types ctx ~verbosity files_info files_contents
        in
        print_tasts ~should_print_position tasts ctx;
        if not @@ Errors.is_empty errors then (
          print_errors error_format errors max_errors;
          Printf.printf "Did not typecheck the TAST as there are typing errors.";
          exit 2
        ) else
          let tast_check_errors = typecheck_tasts tasts ctx filename in
          print_error_list error_format tast_check_errors max_errors;
          if not (List.is_empty tast_check_errors) then exit 2)
  | Dump_stripped_tast ->
    iter_over_files (fun filename ->
        let files_contents =
          Relative_path.Map.filter files_contents ~f:(fun k _v ->
              Relative_path.equal k filename)
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
    print_errors_if_present (parse_errors @ Errors.get_sorted_error_list errors);
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
          ServerRefactorTypes.apply_patches_to_file_contents
            file_contents
            patches
        in
        let print_filename = not @@ Int.equal (SMap.cardinal patched) 1 in
        SMap.iter
          (fun fn new_contents ->
            if print_filename then Printf.printf "//// %s\n" fn;
            Out_channel.output_string stdout new_contents)
          patched)
  | Find_refs (line, column) ->
    let path = expect_single_file () in
    let deps_mode = Provider_context.get_deps_mode ctx in
    let naming_table = Naming_table.create files_info in
    Naming_table.iter naming_table ~f:(Typing_deps.Files.update_file deps_mode);
    let genv = ServerEnvBuild.default_genv in
    let init_id = Random_id.short_string () in
    (* TODO(hverr): Figure out 64-bit *)
    let env =
      {
        (ServerEnvBuild.make_env
           ~init_id
           ~deps_mode:Typing_deps_mode.SQLiteMode
           genv.ServerEnv.config)
        with
        ServerEnv.naming_table;
        ServerEnv.tcopt = Provider_context.get_tcopt ctx;
      }
    in
    let include_defs = true in
    let (ctx, entry) =
      Provider_context.add_entry_if_missing
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
    ClientFindRefsPrint.print_ide_readable results
  | Go_to_impl (line, column) ->
    let filename = expect_single_file () in
    let deps_mode = Provider_context.get_deps_mode ctx in
    let naming_table = Naming_table.create files_info in
    Naming_table.iter naming_table ~f:(Typing_deps.Files.update_file deps_mode);
    let genv = ServerEnvBuild.default_genv in
    let init_id = Random_id.short_string () in
    (* TODO(hverr): Figure out 64-bit mode *)
    let env =
      {
        (ServerEnvBuild.make_env
           ~init_id
           ~deps_mode:Typing_deps_mode.SQLiteMode
           genv.ServerEnv.config)
        with
        ServerEnv.naming_table;
        ServerEnv.tcopt = Provider_context.get_tcopt ctx;
      }
    in
    let filename = Relative_path.to_absolute filename in
    let contents = cat filename in
    let (ctx, entry) =
      Provider_context.add_or_overwrite_entry_contents
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
        ClientFindRefsPrint.print_ide_readable results))
  | Highlight_refs (line, column) ->
    let path = expect_single_file () in
    let (ctx, entry) = Provider_context.add_entry_if_missing ~ctx ~path in
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
        if not (List.is_empty parse_errors) then
          (* This closes the out channel *)
          write_error_list error_format parse_errors oc max_errors
        else (
          Typing_log.out_channel := oc;
          Provider_utils.respect_but_quarantine_unsaved_changes
            ~ctx
            ~f:(fun () ->
              let files_contents = Multifile.file_to_files filename in
              Relative_path.Map.iter files_contents ~f:(fun filename contents ->
                  File_provider.(provide_file filename (Disk contents)));
              let (parse_errors, individual_file_info) =
                parse_name_and_decl ctx files_contents
              in
              let errors =
                check_file
                  ctx
                  ~verbosity
                  (Errors.get_sorted_error_list parse_errors)
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
                  Relative_path.equal k filename)
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
    if not (List.is_empty errors) then exit 2
  | Decl_compare ->
    let filename = expect_single_file () in
    (* Might raise {!SharedMem.Shared_mem_not_found} *)
    test_decl_compare ctx filename builtins files_contents files_info
  | Shallow_class_diff ->
    print_errors_if_present parse_errors;
    let filename = expect_single_file () in
    test_shallow_class_diff ctx filename
  | Get_member class_and_member_id ->
    let (cid, mid) =
      match Str.split (Str.regexp "::") class_and_member_id with
      | [cid; mid] -> (cid, mid)
      | _ ->
        failwith
          (Printf.sprintf "Invalid --get-member ID: %S" class_and_member_id)
    in
    let cid = Utils.add_ns cid in
    (match Decl_provider.get_class ctx cid with
    | None -> Printf.printf "No class named %s\n" cid
    | Some cls ->
      let ty_to_string ty =
        let env = Typing_env.empty ctx Relative_path.default ~droot:None in
        Typing_print.full_strip_ns_decl env ty
      in
      let print_class_element member_type get mid =
        match get cls mid with
        | None -> ()
        | Some ce ->
          let abstract =
            if Typing_defs.get_ce_abstract ce then
              "abstract "
            else
              ""
          in
          let origin = ce.Typing_defs.ce_origin in
          let from =
            if String.equal origin cid then
              ""
            else
              Printf.sprintf " from %s" (Utils.strip_ns origin)
          in
          Printf.printf
            "  %s%s%s: %s\n"
            abstract
            member_type
            from
            (ty_to_string (Lazy.force ce.Typing_defs.ce_type))
      in
      Printf.printf "%s::%s\n" cid mid;
      print_class_element "method" Cls.get_method mid;
      print_class_element "static method" Cls.get_smethod mid;
      print_class_element "property" Cls.get_prop mid;
      print_class_element "static property" Cls.get_sprop mid;
      print_class_element "static property" Cls.get_sprop ("$" ^ mid);
      (match Cls.get_const cls mid with
      | None -> ()
      | Some cc ->
        let abstract =
          Typing_defs.(
            match cc.cc_abstract with
            | CCAbstract _ -> "abstract "
            | CCConcrete -> "")
        in
        let origin = cc.Typing_defs.cc_origin in
        let from =
          if String.equal origin cid then
            ""
          else
            Printf.sprintf " from %s" (Utils.strip_ns origin)
        in
        let ty = ty_to_string cc.Typing_defs.cc_type in
        Printf.printf "  %sconst%s: %s\n" abstract from ty);
      (match Cls.get_typeconst cls mid with
      | None -> ()
      | Some ttc ->
        let origin = ttc.Typing_defs.ttc_origin in
        let from =
          if String.equal origin cid then
            ""
          else
            Printf.sprintf " from %s" (Utils.strip_ns origin)
        in
        let ty =
          let open Typing_defs in
          match ttc.ttc_kind with
          | TCConcrete { tc_type = ty } -> "= " ^ ty_to_string ty
          | TCAbstract
              {
                atc_as_constraint = as_cstr;
                atc_super_constraint = _;
                atc_default = default;
              } ->
            String.concat
              ~sep:" "
              (List.filter_map
                 [
                   Option.map as_cstr ~f:(fun ty -> "as " ^ ty_to_string ty);
                   Option.map default ~f:(fun ty -> "= " ^ ty_to_string ty);
                 ]
                 ~f:(fun x -> x))
          | TCPartiallyAbstract { patc_constraint; patc_type } ->
            String.concat
              ~sep:" "
              [
                "as " ^ ty_to_string patc_constraint;
                "= " ^ ty_to_string patc_type;
              ]
        in
        let abstract =
          Typing_defs.(
            match ttc.ttc_kind with
            | TCConcrete _ -> ""
            | TCAbstract _ -> "abstract "
            | TCPartiallyAbstract _ -> "partially abstract ")
        in
        Printf.printf "  %stypeconst%s: %s %s\n" abstract from mid ty);
      ())
  | Linearization ->
    if not (List.is_empty parse_errors) then (
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
        let is_first = ref true in
        List.iter classes ~f:(fun (_, classname) ->
            if not !is_first then Printf.printf "\n";
            is_first := false;
            let { Decl_linearize.lin_members; Decl_linearize.lin_ancestors } =
              Decl_linearize.get_linearizations ctx classname
            in
            let display mro =
              let name = Utils.strip_ns mro.Decl_defs.mro_name in
              let env =
                Typing_env.empty ctx Relative_path.default ~droot:None
              in
              let targs =
                List.map
                  mro.Decl_defs.mro_type_args
                  ~f:(Typing_print.full_strip_ns_decl env)
              in
              let targs =
                if List.is_empty targs then
                  ""
                else
                  "<" ^ String.concat ~sep:", " targs ^ ">"
              in
              Decl_defs.(
                let modifiers =
                  [
                    ( if Option.is_some mro.mro_required_at then
                      Some "requirement"
                    else if
                    is_set mro_via_req_extends mro.mro_flags
                    || is_set mro_via_req_impl mro.mro_flags
                  then
                      Some "synthesized"
                    else
                      None );
                    ( if is_set mro_xhp_attrs_only mro.mro_flags then
                      Some "xhp_attrs_only"
                    else
                      None );
                    ( if is_set mro_consts_only mro.mro_flags then
                      Some "consts_only"
                    else
                      None );
                    ( if is_set mro_copy_private_members mro.mro_flags then
                      Some "copy_private_members"
                    else
                      None );
                    ( if is_set mro_passthrough_abstract_typeconst mro.mro_flags
                    then
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
                    Printf.sprintf " (%s)" modifiers ))
            in
            let member_linearization =
              Sequence.map lin_members ~f:display |> Sequence.to_list
            in
            let ancestor_linearization =
              Sequence.map lin_ancestors ~f:display |> Sequence.to_list
            in
            Printf.printf "Member Linearization %s:\n" classname;
            List.iter member_linearization ~f:(Printf.printf "  %s\n");
            Printf.printf "Ancestor Linearization %s:\n" classname;
            List.iter ancestor_linearization ~f:(Printf.printf "  %s\n")))
  | Hover pos_given ->
    let filename = expect_single_file () in
    let (ctx, entry) =
      Provider_context.add_entry_if_missing ~ctx ~path:filename
    in
    let (line, column) =
      match pos_given with
      | Some (line, column) -> (line, column)
      | None ->
        let src = Provider_context.read_file_contents_exn entry in
        hover_at_caret_pos src
    in
    let results = ServerHover.go_quarantined ~ctx ~entry ~line ~column in
    let print result =
      Printf.printf "%s\n" (HoverService.string_of_result result)
    in
    List.iter results ~f:print

(*****************************************************************************)
(* Main entry point *)
(*****************************************************************************)

let decl_and_run_mode
    {
      files;
      extra_builtins;
      mode;
      error_format;
      no_builtins;
      tcopt;
      max_errors;
      batch_mode;
      out_extension;
      verbosity;
      should_print_position;
    }
    (popt : TypecheckerOptions.t)
    (hhi_root : Path.t)
    (naming_table_path : string option) : unit =
  Ident.track_names := true;
  let builtins =
    if no_builtins then
      Relative_path.Map.empty
    else
      let extra_builtins =
        let add_file_content map filename =
          Relative_path.create Relative_path.Dummy filename
          |> Multifile.file_to_file_list
          |> List.map ~f:(fun (path, contents) ->
                 (Filename.basename (Relative_path.suffix path), contents))
          |> List.unordered_append map
        in
        extra_builtins
        |> List.fold ~f:add_file_content ~init:[]
        |> Array.of_list
      in
      let magic_builtins = Array.append magic_builtins extra_builtins in
      (* Check that magic_builtin filenames are unique *)
      let () =
        let n_of_builtins = Array.length magic_builtins in
        let n_of_unique_builtins =
          Array.to_list magic_builtins
          |> List.map ~f:fst
          |> SSet.of_list
          |> SSet.cardinal
        in
        if n_of_builtins <> n_of_unique_builtins then
          die "Multiple magic builtins share the same base name.\n"
      in
      Array.iter magic_builtins ~f:(fun (file_name, file_contents) ->
          let file_path = Path.concat hhi_root file_name in
          let file = Path.to_string file_path in
          Sys_utils.try_touch
            (Sys_utils.Touch_existing { follow_symlinks = true })
            file;
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
  let files =
    if use_canonical_filenames () then
      files
      |> List.map ~f:Sys_utils.realpath
      |> List.map ~f:(fun s -> Option.value_exn s)
      |> List.map ~f:Relative_path.create_detect_prefix
    else
      files |> List.map ~f:(Relative_path.create Relative_path.Dummy)
  in
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
  Relative_path.Map.iter files_contents ~f:(fun filename contents ->
      File_provider.(provide_file filename (Disk contents)));
  (* Don't declare all the filenames in batch_errors mode *)
  let to_decl =
    if batch_mode then
      builtins
    else
      files_contents_with_builtins
  in
  let dbg_deps = Hashtbl.Poly.create () in
  (match mode with
  | Dump_deps ->
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
        Hashtbl.set dbg_deps ~key:obj ~data:set
    in
    Typing_deps.add_dependency_callback "get_debug_trace" get_debug_trace
  | _ -> ());
  let dbg_glean_deps = HashSet.create () in
  (match mode with
  | Dump_glean_deps ->
    (* In addition to actually recording the dependencies in shared memory,
     we build a non-hashed respresentation of the dependency graph
     for printing. In the callback we receive this as dep_right uses dep_left. *)
    let get_debug_trace dep_right dep_left =
      HashSet.add dbg_glean_deps (dep_left, dep_right)
    in
    Typing_deps.add_dependency_callback "get_debug_trace" get_debug_trace
  | _ -> ());
  (* TODO(hverr): Should we switch this to 64-bit *)
  let ctx =
    Provider_context.empty_for_test
      ~popt
      ~tcopt
      ~deps_mode:Typing_deps_mode.SQLiteMode
  in
  (* We make the following call for the side-effect of updating ctx's "naming-table fallback"
  so it will look in the sqlite database for names it doesn't know.
  This function returns the forward naming table, but we don't care about that;
  it's only needed for tools that process file changes, to know in the event
  of a file-change which old symbols used to be defined in the file. *)
  let _naming_table_for_root : Naming_table.t option =
    Option.map naming_table_path ~f:(fun path ->
        Naming_table.load_from_sqlite ctx path)
  in
  let (errors, files_info) = parse_name_and_decl ctx to_decl in
  handle_mode
    mode
    files
    ctx
    builtins
    files_contents
    files_info
    (Errors.get_sorted_error_list errors)
    max_errors
    error_format
    batch_mode
    out_extension
    dbg_deps
    dbg_glean_deps
    ~should_print_position
    ~verbosity

let main_hack
    ({ tcopt; _ } as opts)
    (root : Path.t)
    (naming_table : string option)
    (sharedmem_config : SharedMem.config) : unit =
  (* TODO: We should have a per file config *)
  Sys_utils.signal Sys.sigusr1 (Sys.Signal_handle Typing.debug_print_last_pos);
  EventLogger.init_fake ();

  let (_handle : SharedMem.handle) =
    SharedMem.init ~num_workers:0 sharedmem_config
  in
  Tempfile.with_tempdir (fun hhi_root ->
      Hhi.set_hhi_root_for_unit_test hhi_root;
      Relative_path.set_path_prefix Relative_path.Root root;
      Relative_path.set_path_prefix Relative_path.Hhi hhi_root;
      Relative_path.set_path_prefix Relative_path.Tmp (Path.make "tmp");
      decl_and_run_mode opts tcopt hhi_root naming_table;
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
  let (options, root, naming_table, sharedmem_config) = parse_options () in
  Unix.handle_unix_error main_hack options root naming_table sharedmem_config
