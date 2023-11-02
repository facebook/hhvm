(*
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)
open Hh_prelude
open Sys_utils
module Cls = Decl_provider.Class

(*****************************************************************************)
(* Profiling utilities *)
(*****************************************************************************)

let mean samples =
  List.fold ~init:0.0 ~f:( +. ) samples /. Float.of_int (List.length samples)

let standard_deviation mean samples =
  let sosq =
    List.fold
      ~init:0.0
      ~f:(fun acc sample ->
        let diff = sample -. mean in
        (diff *. diff) +. acc)
      samples
  in
  let sosqm = sosq /. Float.of_int (List.length samples) in
  Float.sqrt sosqm

(*****************************************************************************)
(* Types, constants *)
(*****************************************************************************)

type mode =
  | Cst_search
  | Dump_symbol_info
  | Glean_index of string
  | Glean_sym_hash
  | Dump_inheritance
  | Errors
  | Lint
  | Lint_json
  | Dump_deps
  | Dump_dep_hashes
  | Get_some_file_deps of {
      depth: int;
      full_hierarchy: bool;
    }
  | Identify_symbol of int * int
  | Ide_code_actions of {
      title_prefix: string;
      use_snippet_edits: bool;
    }
  | Find_local of int * int
  | Get_member of string
  | Outline
  | Dump_nast
  | Dump_stripped_tast
  | Dump_tast
  | Find_refs of int * int
  | Highlight_refs of int * int
  | Decl_compare
  | Shallow_class_diff
  | Go_to_impl of int * int
  | Hover of (int * int) option
  | Apply_quickfixes
  | Refactor_sound_dynamic of string * string * string
  | RemoveDeadUnsafeCasts
  | CountImpreciseTypes
  | SDT_analysis of string
  | Get_type_hierarchy
  | Map_reduce_mode

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
  custom_hhi_path: string option;
  profile_type_check_multi: int option;
  memtrace: string option;
  rust_provider_backend: bool;
  naming_table_path: string option;
  packages_config_path: string option;
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
      ^ "<<__NoAutoDynamic, __SupportDynamicType>> function hh_show<T>(<<__AcceptDisposable>> readonly T $val)[]:T {}\n"
      ^ "<<__NoAutoDynamic, __SupportDynamicType>> function hh_expect<T>(<<__AcceptDisposable>> readonly T $val)[]:T {}\n"
      ^ "<<__NoAutoDynamic, __SupportDynamicType>> function hh_expect_equivalent<T>(<<__AcceptDisposable>> readonly T $val)[]:T {}\n"
      ^ "<<__NoAutoDynamic>> function hh_show_env()[]:void {}\n"
      ^ "<<__NoAutoDynamic>> function hh_log_level(string $key, int $level)[]:void {}\n"
      ^ "<<__NoAutoDynamic>> function hh_force_solve()[]:void {}"
      ^ "<<__NoAutoDynamic>> function hh_time(string $command, string $tag = '_'):void {}\n"
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
    | Errors.Raw -> (fun e -> Raw_error_formatter.to_string e)
    | Errors.Plain -> (fun e -> Errors.to_string e)
    | Errors.Highlighted -> Highlighted_error_formatter.to_string
  in
  let absolute_errors = User_error.to_absolute l in
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
        ~displayed_count:(List.length errors)
        ~dropped_count:(Some (List.length dropped_errors))
        ~max_errors
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
        ~displayed_count:(List.length errors)
        ~dropped_count:(Some (List.length dropped_errors))
        ~max_errors
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

let load_and_parse_custom_error_config path =
  match Custom_error_config.initialize (`Absolute path) with
  | Ok cfg ->
    if not @@ List.is_empty cfg.Custom_error_config.invalid then
      eprintf
        "Encountered invalid rules with loading custom error config: \n %s\n"
      @@ String.concat ~sep:"\n"
      @@ List.map ~f:(fun Custom_error.{ name; _ } -> name)
      @@ cfg.Custom_error_config.invalid;
    Some cfg
  | Error msg ->
    eprintf
      "Encountered and error when loading custom error config: \n %s\n"
      msg;
    None

let parse_options () =
  let fn_ref = ref [] in
  let extra_builtins = ref [] in
  let usage = Printf.sprintf "Usage: %s filename\n" Sys.argv.(0) in
  let mode = ref Errors in
  let no_builtins = ref false in
  let log_levels = ref SMap.empty in
  let max_errors = ref None in
  let batch_mode = ref false in
  let set_mode x () =
    match !mode with
    | Errors -> mode := x
    | _ -> raise (Arg.Bad "only a single mode should be specified")
  in
  let config_overrides = ref [] in
  let error_format = ref Errors.Highlighted in
  let forbid_nullable_cast = ref false in
  let deregister_attributes = ref None in
  let auto_namespace_map = ref None in
  let log_inference_constraints = ref None in
  let timeout = ref None in
  let disallow_byref_dynamic_calls = ref (Some false) in
  let disallow_byref_calls = ref (Some false) in
  let set_bool x () = x := Some true in
  let set_bool_ x () = x := true in
  let rust_elab = ref false in
  let rust_provider_backend = ref false in
  let skip_hierarchy_checks = ref false in
  let skip_tast_checks = ref false in
  let skip_check_under_dynamic = ref false in
  let out_extension = ref ".out" in
  let like_type_hints = ref false in
  let union_intersection_type_hints = ref false in
  let call_coeffects = ref true in
  let local_coeffects = ref true in
  let strict_contexts = ref true in
  let check_xhp_attribute = ref false in
  let check_redundant_generics = ref false in
  let disallow_static_memoized = ref false in
  let enable_supportdyn_hint = ref false in
  let enable_class_level_where_clauses = ref false in
  let disable_legacy_soft_typehints = ref false in
  let allow_new_attribute_syntax = ref false in
  let allow_toplevel_requires = ref false in
  let const_static_props = ref false in
  let disable_legacy_attribute_syntax = ref false in
  let const_attribute = ref false in
  let const_default_func_args = ref false in
  let const_default_lambda_args = ref false in
  let disallow_silence = ref false in
  let abstract_static_props = ref false in
  let glean_reponame = ref (GleanOptions.reponame GlobalOptions.default) in
  let disallow_func_ptrs_in_constants = ref false in
  let error_php_lambdas = ref false in
  let disallow_discarded_nullable_awaitables = ref false in
  let disable_xhp_element_mangling = ref false in
  let keep_user_attributes = ref false in
  let disable_xhp_children_declarations = ref false in
  let enable_xhp_class_modifier = ref false in
  let verbosity = ref 0 in
  let disable_hh_ignore_error = ref 0 in
  let is_systemlib = ref false in
  let enable_higher_kinded_types = ref false in
  let allowed_fixme_codes_strict = ref None in
  let allowed_decl_fixme_codes = ref None in
  let method_call_inference = ref false in
  let report_pos_from_reason = ref false in
  let enable_sound_dynamic = ref true in
  let always_pessimise_return = ref false in
  let consider_type_const_enforceable = ref false in
  let interpret_soft_types_as_like_types = ref false in
  let enable_strict_string_concat_interp = ref false in
  let ignore_unsafe_cast = ref false in
  let math_new_code = ref false in
  let typeconst_concrete_concrete_error = ref false in
  let enable_strict_const_semantics = ref 0 in
  let strict_wellformedness = ref 0 in
  let meth_caller_only_public_visibility = ref true in
  let require_extends_implements_ancestors = ref false in
  let strict_value_equality = ref false in
  let expression_tree_virtualize_functions = ref false in
  let naming_table = ref None in
  let root = ref None in
  let sharedmem_config = ref SharedMem.default_config in
  let print_position = ref true in
  let enforce_sealed_subclasses = ref false in
  let everything_sdt = ref false in
  let custom_hhi_path = ref None in
  let force_allow_builtins_in_custom_hhi_path = ref false in
  let explicit_consistent_constructors = ref 0 in
  let require_types_class_consts = ref 0 in
  let type_printer_fuel =
    ref (TypecheckerOptions.type_printer_fuel GlobalOptions.default)
  in
  let profile_type_check_multi = ref None in
  let profile_top_level_definitions =
    ref (TypecheckerOptions.profile_top_level_definitions GlobalOptions.default)
  in
  let memtrace = ref None in
  let enable_global_access_check = ref false in
  let packages_config_path = ref None in
  let custom_error_config_path = ref None in
  let allow_all_files_for_module_declarations = ref true in
  let loop_iteration_upper_bound = ref None in
  let get_some_file_deps_full_hierarchy = ref false in
  let options =
    [
      ( "--config",
        Arg.String (fun s -> config_overrides := s :: !config_overrides),
        "<option=value> Set one hhconfig option; can be used multiple times" );
      ( "--no-print-position",
        Arg.Unit (fun _ -> print_position := false),
        " Don't print positions while printing TASTs and NASTs" );
      ( "--naming-table",
        Arg.String (fun s -> naming_table := Some s),
        " Naming table, to look up undefined symbols; needs --root."
        ^ " (Hint: buck2 run //hphp/hack/src/hh_naming_table_builder)" );
      ( "--root",
        Arg.String (fun s -> root := Some s),
        " Root for where to look up undefined symbols; needs --naming-table" );
      ( "--extra-builtin",
        Arg.String (fun f -> extra_builtins := f :: !extra_builtins),
        " HHI file to parse and declare" );
      ( "--refactor-sound-dynamic",
        (let refactor_analysis_mode = ref "" in
         let refactor_mode = ref "" in
         Arg.Tuple
           [
             Arg.String (( := ) refactor_analysis_mode);
             Arg.String (( := ) refactor_mode);
             Arg.String
               (fun x ->
                 batch_mode := true;
                 set_mode
                   (Refactor_sound_dynamic
                      (!refactor_analysis_mode, !refactor_mode, x))
                   ());
           ]),
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
      ( "--cst-search",
        Arg.Unit (set_mode Cst_search),
        " Search the concrete syntax tree of the given file using the pattern"
        ^ " given on stdin."
        ^ " (The pattern is a JSON object adhering to the search DSL.)" );
      ( "--dump-symbol-info",
        Arg.Unit (set_mode Dump_symbol_info),
        " Dump all symbol information" );
      ( "--glean-index",
        Arg.String (fun output_dir -> set_mode (Glean_index output_dir) ()),
        " Run indexer and output json in provided dir" );
      ( "--glean-sym-hash",
        Arg.Unit (set_mode Glean_sym_hash),
        " Print symbols hashes used by incremental indexer" );
      ( "--error-format",
        Arg.String
          (fun s ->
            match s with
            | "raw" -> error_format := Errors.Raw
            | "context" -> error_format := Errors.Context
            | "highlighted" -> error_format := Errors.Highlighted
            | "plain" -> error_format := Errors.Plain
            | _ -> print_string "Warning: unrecognized error format.\n"),
        "<raw|context|highlighted|plain> Error formatting style; (default: highlighted)"
      );
      ("--lint", Arg.Unit (set_mode Lint), " Produce lint errors");
      ("--lint-json", Arg.Unit (set_mode Lint_json), " Produce json lint output");
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
      ( "--dump-inheritance",
        Arg.Unit (set_mode Dump_inheritance),
        " Print inheritance" );
      ( "--get-some-file-deps",
        Arg.Int
          (fun depth ->
            set_mode (Get_some_file_deps { depth; full_hierarchy = false }) ()),
        " Print a list of files this file depends on. The provided integer is the depth of the traversal. Requires --root, --naming-table and --depth"
      );
      ( "--get-some-file-deps-full-hierarchy",
        Arg.Set get_some_file_deps_full_hierarchy,
        " Include all dependencies from class hierarchies in the list of files this file depends on regardless of their depth."
      );
      ( "--ide-code-actions",
        Arg.String
          (fun title_prefix ->
            set_mode
              (Ide_code_actions { title_prefix; use_snippet_edits = true })
              ()),
        "<title_prefix> Apply a code action with the given title prefix to the given file, where the selection is indicated with markers in comments (see tests)"
      );
      ( "--ide-code-actions-no-experimental-capabilities",
        Arg.String
          (fun title_prefix ->
            set_mode
              (Ide_code_actions { title_prefix; use_snippet_edits = false })
              ()),
        "<title_prefix> Like --ide-code-actions, but do not use any nonstandard LSP features (experimental capabilities)."
      );
      ( "--identify-symbol",
        (let line = ref 0 in
         Arg.Tuple
           [
             Arg.Int (( := ) line);
             Arg.Int
               (fun column -> set_mode (Identify_symbol (!line, column)) ());
           ]),
        "<pos> Show info about symbol at given line and column" );
      ( "--find-local",
        (let line = ref 0 in
         Arg.Tuple
           [
             Arg.Int (( := ) line);
             Arg.Int (fun column -> set_mode (Find_local (!line, column)) ());
           ]),
        "<pos> Find all usages of local at given line and column" );
      ( "--max-errors",
        Arg.Int (fun num_errors -> max_errors := Some num_errors),
        " Maximum number of errors to display" );
      ("--outline", Arg.Unit (set_mode Outline), " Print file outline");
      ("--nast", Arg.Unit (set_mode Dump_nast), " Print out the named AST");
      ("--tast", Arg.Unit (set_mode Dump_tast), " Print out the typed AST");
      ( "--stripped-tast",
        Arg.Unit (set_mode Dump_stripped_tast),
        " Print out the typed AST, stripped of type information."
        ^ " This can be compared against the named AST to look for holes." );
      ( "--find-refs",
        (let line = ref 0 in
         Arg.Tuple
           [
             Arg.Int (( := ) line);
             Arg.Int (fun column -> set_mode (Find_refs (!line, column)) ());
           ]),
        "<pos> Find all usages of a symbol at given line and column" );
      ( "--go-to-impl",
        Arg.Tuple
          (let line = ref 0 in
           [
             Arg.Int (( := ) line);
             Arg.Int (fun column -> set_mode (Go_to_impl (!line, column)) ());
           ]),
        "<pos> Find all implementations of a symbol at given line and column" );
      ( "--highlight-refs",
        (let line = ref 0 in
         Arg.Tuple
           [
             Arg.Int (( := ) line);
             Arg.Int
               (fun column -> set_mode (Highlight_refs (!line, column)) ());
           ]),
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
      ( "--get-member",
        Arg.String
          (fun class_and_member_id ->
            set_mode (Get_member class_and_member_id) ()),
        " Given ClassName::MemberName, fetch the decl of members with that name and print them."
      );
      ( "--log-inference-constraints",
        Arg.Unit (set_bool log_inference_constraints),
        " Log inference constraints to Scuba." );
      ( "--timeout",
        Arg.Int (fun secs -> timeout := Some secs),
        " Timeout in seconds for checking a function or a class." );
      ( "--hh-log-level",
        (let log_key = ref "" in
         Arg.Tuple
           [
             Arg.String (( := ) log_key);
             Arg.Int
               (fun level -> log_levels := SMap.add !log_key level !log_levels);
           ]),
        " Set the log level for a key" );
      ( "--batch-files",
        Arg.Set batch_mode,
        " Typecheck each file passed in independently" );
      ( "--disallow-static-memoized",
        Arg.Set disallow_static_memoized,
        " Disallow static memoized methods on non-final methods" );
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
      ( "--rust-elab",
        Arg.Set rust_elab,
        " Use the Rust implementation of naming elaboration and NAST checks" );
      ( "--rust-provider-backend",
        Arg.Set rust_provider_backend,
        " Use the Rust implementation of Provider_backend (including decl-folding)"
      );
      ( "--skip-hierarchy-checks",
        Arg.Set skip_hierarchy_checks,
        " Do not apply checks on class hierarchy (override, implements, etc)" );
      ( "--skip-tast-checks",
        Arg.Set skip_tast_checks,
        " Do not apply checks using TAST visitors" );
      ( "--skip-check-under-dynamic",
        Arg.Set skip_check_under_dynamic,
        " Do not apply second check to functions and methods under dynamic assumptions"
      );
      ( "--union-intersection-type-hints",
        Arg.Set union_intersection_type_hints,
        " Allows union and intersection types to be written in type hint positions"
      );
      ( "--like-type-hints",
        Arg.Set like_type_hints,
        " Allows like types to be written in type hint positions" );
      ( "--naive-implicit-pess",
        Arg.Unit
          (fun () ->
            set_bool_ enable_sound_dynamic ();
            set_bool_ everything_sdt ();
            set_bool_ like_type_hints ();
            set_bool_ always_pessimise_return ();
            set_bool_ consider_type_const_enforceable ();
            set_bool_ enable_supportdyn_hint ()),
        " Enables naive implicit pessimisation" );
      ( "--implicit-pess",
        Arg.Unit
          (fun () ->
            set_bool_ enable_sound_dynamic ();
            set_bool_ everything_sdt ();
            set_bool_ like_type_hints ();
            set_bool_ enable_supportdyn_hint ()),
        " Enables implicit pessimisation" );
      ( "--explicit-pess",
        Arg.String
          (fun dir ->
            set_bool_ enable_sound_dynamic ();
            set_bool_ like_type_hints ();
            set_bool_ enable_supportdyn_hint ();
            custom_hhi_path := Some dir),
        " Enables checking explicitly pessimised files. Requires path to pessimised .hhi files "
      );
      ( "--enable-supportdyn-hint",
        Arg.Set enable_supportdyn_hint,
        " Allow the supportdyn type hint" );
      ( "--enable-class-level-where-clauses",
        Arg.Set enable_class_level_where_clauses,
        " Enables support for class-level where clauses" );
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
      ( "--keep-user-attributes",
        Arg.Set keep_user_attributes,
        " Keep user attributes when parsing decls" );
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
      ( "--disable-hh-ignore-error",
        Arg.Int (( := ) disable_hh_ignore_error),
        " Forbid HH_IGNORE_ERROR comments as an alternative to HH_FIXME, or treat them as normal comments."
      );
      ( "--is-systemlib",
        Arg.Set is_systemlib,
        " Enable systemlib annotations and other internal-only features" );
      ( "--enable-higher-kinded-types",
        Arg.Set enable_higher_kinded_types,
        " Enable support for higher-kinded types" );
      ( "--allowed-fixme-codes-strict",
        Arg.String
          (fun s -> allowed_fixme_codes_strict := Some (comma_string_to_iset s)),
        " List of fixmes that are allowed in strict mode." );
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
      ( "--always-pessimise-return",
        Arg.Set always_pessimise_return,
        " Consider all return types unenforceable." );
      ( "--consider-type-const-enforceable",
        Arg.Set consider_type_const_enforceable,
        " Consider type constants to potentially be enforceable." );
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
      ( "--enable-strict-const-semantics",
        Arg.Int (fun x -> enable_strict_const_semantics := x),
        " Raise an error when a concrete constants is overridden or multiply defined"
      );
      ( "--strict-wellformedness",
        Arg.Int (fun x -> strict_wellformedness := x),
        " Re-introduce missing well-formedness checks in AST positions" );
      ( "--meth-caller-only-public-visibility",
        Arg.Bool (fun x -> meth_caller_only_public_visibility := x),
        " Controls whether meth_caller can be used on non-public methods" );
      ( "--hover",
        (let line = ref 0 in
         Arg.Tuple
           [
             Arg.Int (fun x -> line := x);
             Arg.Int (fun column -> set_mode (Hover (Some (!line, column))) ());
           ]),
        "<pos> Display hover tooltip" );
      ( "--hover-at-caret",
        Arg.Unit (fun () -> set_mode (Hover None) ()),
        " Show the hover information indicated by // ^ hover-at-caret" );
      ( "--fix",
        Arg.Unit (fun () -> set_mode Apply_quickfixes ()),
        " Apply quickfixes for all the errors in the file, and print the resulting code."
      );
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
      ( "--everything-sdt",
        Arg.Set everything_sdt,
        " Treat all classes, functions, and traits as though they are annotated with <<__SupportDynamicType>>, unless they are annotated with <<__NoAutoDynamic>>"
      );
      ( "--custom-hhi-path",
        Arg.String (fun s -> custom_hhi_path := Some s),
        " Use custom hhis" );
      ( "--force-allow-builtins-in-custom-hhi-path-FLAKEY",
        Arg.Set force_allow_builtins_in_custom_hhi_path,
        " (FLAKEY) allow builtins to be written into the custom hhi path" );
      ( "--explicit-consistent-constructors",
        Arg.Int (( := ) explicit_consistent_constructors),
        " Raise an error for <<__ConsistentConstruct>> without an explicit constructor; 1 for traits, 2 for all "
      );
      ( "--require-types-class-consts",
        Arg.Int (( := ) require_types_class_consts),
        " Raise an error for class constants missing types; 1 for abstract constants, 2 for all "
      );
      ( "--profile-type-check-twice",
        Arg.Unit (fun () -> profile_type_check_multi := Some 1),
        " Typecheck the file twice" );
      ( "--profile-type-check-multi",
        Arg.Int (fun n -> profile_type_check_multi := Some n),
        " Typecheck the files n times extra (!)" );
      ( "--profile-top-level-definitions",
        Arg.Set profile_top_level_definitions,
        " Profile typechecking of top-level definitions" );
      ( "--memtrace",
        Arg.String (fun s -> memtrace := Some s),
        " Write memtrace to this file (typical extension .ctf)" );
      ( "--type-printer-fuel",
        Arg.Int (( := ) type_printer_fuel),
        " Sets the amount of fuel that the type printer can use to display an individual type. Default: "
        ^ string_of_int
            (TypecheckerOptions.type_printer_fuel GlobalOptions.default) );
      ( "--enable-global-access-check",
        Arg.Set enable_global_access_check,
        " Run global access checker to check global writes and reads" );
      ( "--overwrite-loop-iteration-upper-bound",
        Arg.Int (fun u -> loop_iteration_upper_bound := Some u),
        " Sets the maximum number of iterations that will be used to typecheck loops"
      );
      ( "--expression-tree-virtualize-functions",
        Arg.Set expression_tree_virtualize_functions,
        " Enables function virtualization in Expression Trees" );
      ( "--remove-dead-unsafe-casts",
        Arg.Unit (fun () -> set_mode RemoveDeadUnsafeCasts ()),
        " Removes dead unsafe casts from a file" );
      ( "--count-imprecise-types",
        Arg.Unit (fun () -> set_mode CountImpreciseTypes ()),
        " Counts the number of mixed, dynamic, and nonnull types in a file" );
      ( "--sdt-analysis",
        Arg.String
          (fun command ->
            batch_mode := true;
            set_mode (SDT_analysis command) ()),
        " Analyses to support Sound Dynamic rollout" );
      ( "--packages-config-path",
        Arg.String (fun s -> packages_config_path := Some s),
        " Config file for a list of package definitions" );
      ( "--custom-error-config-path",
        Arg.String (fun s -> custom_error_config_path := Some s),
        " Config file for custom error messages" );
      ( "--get-type-hierarchy-at-caret",
        Arg.Unit (set_mode Get_type_hierarchy),
        " Produce type hierarchy at caret location" );
      ( "--map-reduce",
        Arg.Unit (set_mode Map_reduce_mode),
        " Run the map reducers and print the result" );
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

  (match !mode with
  | Get_some_file_deps { depth; _ } ->
    if Option.is_none !naming_table then
      raise (Arg.Bad "--get-some-file-deps requires --naming-table");
    if Option.is_none !root then
      raise (Arg.Bad "--get-some-file-deps requires --root");
    mode :=
      Get_some_file_deps
        { depth; full_hierarchy = !get_some_file_deps_full_hierarchy }
  | _ -> ());

  if Option.is_some !naming_table && Option.is_none !root then
    failwith "--naming-table needs --root";

  (* Unless --no-builtins, then well write into the supplied directory, which would be bad. *)
  if
    Option.is_some !custom_hhi_path
    && (not !no_builtins)
    && not !force_allow_builtins_in_custom_hhi_path
  then
    failwith "--custom-hhi-path needs --no-builtins";

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
          (Filename.concat root Config_file.file_path_relative_to_repo_root)
      in
      (* We will pick up values from .hhconfig, unless they've been overridden at the command-line. *)
      if Option.is_none !auto_namespace_map then
        auto_namespace_map :=
          config
          |> Config_file.Getters.string_opt "auto_namespace_map"
          |> Option.map ~f:ServerConfig.convert_auto_namespace_to_map;
      if Option.is_none !allowed_fixme_codes_strict then
        allowed_fixme_codes_strict :=
          config
          |> Config_file.Getters.string_opt "allowed_fixme_codes_strict"
          |> Option.map ~f:comma_string_to_iset;
      sharedmem_config :=
        ServerConfig.make_sharedmem_config
          config
          (ServerArgs.default_options ~root)
          ServerLocalConfig.default;
      (* Path.make canonicalizes it, i.e. resolves symlinks *)
      Path.make root
  in

  let tcopt : GlobalOptions.t =
    GlobalOptions.set
      ~tco_saved_state:GlobalOptions.default_saved_state
      ?po_deregister_php_stdlib:!deregister_attributes
      ?tco_log_inference_constraints:!log_inference_constraints
      ?tco_timeout:!timeout
      ?po_auto_namespace_map:!auto_namespace_map
      ?tco_disallow_byref_dynamic_calls:!disallow_byref_dynamic_calls
      ?tco_disallow_byref_calls:!disallow_byref_calls
      ~allowed_fixme_codes_strict:
        (Option.value !allowed_fixme_codes_strict ~default:ISet.empty)
      ~tco_check_xhp_attribute:!check_xhp_attribute
      ~tco_check_redundant_generics:!check_redundant_generics
      ~tco_skip_hierarchy_checks:!skip_hierarchy_checks
      ~tco_skip_tast_checks:!skip_tast_checks
      ~tco_like_type_hints:!like_type_hints
      ~tco_union_intersection_type_hints:!union_intersection_type_hints
      ~tco_strict_contexts:!strict_contexts
      ~tco_coeffects:!call_coeffects
      ~tco_coeffects_local:!local_coeffects
      ~tco_like_casts:false
      ~log_levels:!log_levels
      ~po_enable_class_level_where_clauses:!enable_class_level_where_clauses
      ~po_disable_legacy_soft_typehints:!disable_legacy_soft_typehints
      ~po_allow_new_attribute_syntax:!allow_new_attribute_syntax
      ~po_disallow_toplevel_requires:(not !allow_toplevel_requires)
      ~tco_const_static_props:!const_static_props
      ~po_disable_legacy_attribute_syntax:!disable_legacy_attribute_syntax
      ~tco_const_attribute:!const_attribute
      ~po_const_default_func_args:!const_default_func_args
      ~po_const_default_lambda_args:!const_default_lambda_args
      ~po_disallow_silence:!disallow_silence
      ~po_abstract_static_props:!abstract_static_props
      ~po_disallow_func_ptrs_in_constants:!disallow_func_ptrs_in_constants
      ~tco_check_attribute_locations:true
      ~tco_error_php_lambdas:!error_php_lambdas
      ~tco_disallow_discarded_nullable_awaitables:
        !disallow_discarded_nullable_awaitables
      ~glean_reponame:!glean_reponame
      ~po_disable_xhp_element_mangling:!disable_xhp_element_mangling
      ~po_disable_xhp_children_declarations:!disable_xhp_children_declarations
      ~po_enable_xhp_class_modifier:!enable_xhp_class_modifier
      ~po_keep_user_attributes:!keep_user_attributes
      ~po_disable_hh_ignore_error:!disable_hh_ignore_error
      ~tco_is_systemlib:!is_systemlib
      ~tco_higher_kinded_types:!enable_higher_kinded_types
      ~po_allowed_decl_fixme_codes:
        (Option.value !allowed_decl_fixme_codes ~default:ISet.empty)
      ~po_allow_unstable_features:true
      ~tco_method_call_inference:!method_call_inference
      ~tco_report_pos_from_reason:!report_pos_from_reason
      ~tco_enable_sound_dynamic:!enable_sound_dynamic
      ~tco_skip_check_under_dynamic:!skip_check_under_dynamic
      ~tco_global_access_check_enabled:!enable_global_access_check
      ~po_interpret_soft_types_as_like_types:!interpret_soft_types_as_like_types
      ~tco_enable_strict_string_concat_interp:
        !enable_strict_string_concat_interp
      ~tco_ignore_unsafe_cast:!ignore_unsafe_cast
      ~tco_math_new_code:!math_new_code
      ~tco_typeconst_concrete_concrete_error:!typeconst_concrete_concrete_error
      ~tco_enable_strict_const_semantics:!enable_strict_const_semantics
      ~tco_strict_wellformedness:!strict_wellformedness
      ~tco_meth_caller_only_public_visibility:
        !meth_caller_only_public_visibility
      ~tco_require_extends_implements_ancestors:
        !require_extends_implements_ancestors
      ~tco_strict_value_equality:!strict_value_equality
      ~tco_enforce_sealed_subclasses:!enforce_sealed_subclasses
      ~tco_everything_sdt:!everything_sdt
      ~tco_explicit_consistent_constructors:!explicit_consistent_constructors
      ~tco_require_types_class_consts:!require_types_class_consts
      ~tco_type_printer_fuel:!type_printer_fuel
      ~tco_profile_top_level_definitions:!profile_top_level_definitions
      ~tco_allow_all_files_for_module_declarations:
        !allow_all_files_for_module_declarations
      ~tco_loop_iteration_upper_bound:!loop_iteration_upper_bound
      ~tco_expression_tree_virtualize_functions:
        !expression_tree_virtualize_functions
      ~tco_rust_elab:!rust_elab
      GlobalOptions.default
  in

  let tcopt =
    let config =
      List.fold
        (List.rev !config_overrides)
        ~init:(Config_file_common.empty ())
        ~f:(fun config setting ->
          let c = Config_file_common.parse_contents setting in
          Config_file_common.apply_overrides
            ~config
            ~overrides:c
            ~log_reason:None)
    in
    ServerConfig.load_config config tcopt
  in

  Errors.allowed_fixme_codes_strict :=
    GlobalOptions.allowed_fixme_codes_strict tcopt;
  Errors.report_pos_from_reason :=
    TypecheckerOptions.report_pos_from_reason tcopt;

  let tco_experimental_features =
    tcopt.GlobalOptions.tco_experimental_features
  in
  let tco_experimental_features =
    if !forbid_nullable_cast then
      SSet.add
        TypecheckerOptions.experimental_forbid_nullable_cast
        tco_experimental_features
    else
      tco_experimental_features
  in
  let tco_experimental_features =
    if !disallow_static_memoized then
      SSet.add
        TypecheckerOptions.experimental_disallow_static_memoized
        tco_experimental_features
    else
      tco_experimental_features
  in
  let tco_experimental_features =
    if !enable_supportdyn_hint then
      SSet.add
        TypecheckerOptions.experimental_supportdynamic_type_hint
        tco_experimental_features
    else
      tco_experimental_features
  in
  let tco_experimental_features =
    if !always_pessimise_return then
      SSet.add
        TypecheckerOptions.experimental_always_pessimise_return
        tco_experimental_features
    else
      tco_experimental_features
  in
  let tco_experimental_features =
    if !consider_type_const_enforceable then
      SSet.add
        TypecheckerOptions.experimental_consider_type_const_enforceable
        tco_experimental_features
    else
      tco_experimental_features
  in

  let tcopt = { tcopt with GlobalOptions.tco_experimental_features } in
  let tco_custom_error_config =
    Option.value ~default:Custom_error_config.empty
    @@ Option.bind
         ~f:load_and_parse_custom_error_config
         !custom_error_config_path
  in
  let tcopt = GlobalOptions.{ tcopt with tco_custom_error_config } in
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
      custom_hhi_path = !custom_hhi_path;
      profile_type_check_multi = !profile_type_check_multi;
      memtrace = !memtrace;
      rust_provider_backend = !rust_provider_backend;
      naming_table_path = !naming_table;
      packages_config_path = !packages_config_path;
    },
    root,
    if !rust_provider_backend then
      SharedMem.
        {
          !sharedmem_config with
          shm_use_sharded_hashtbl = true;
          shm_cache_size =
            max !sharedmem_config.shm_cache_size (2 * 1024 * 1024 * 1024);
        }
    else
      !sharedmem_config )

let print_elapsed fn desc ~start_time =
  let elapsed_ms = Float.(Unix.gettimeofday () - start_time) *. 1000. in
  Printf.printf
    "%s: %s - %0.2fms\n"
    (Relative_path.to_absolute fn |> Filename.basename)
    desc
    elapsed_ms

let check_file ctx errors files_info ~profile_type_check_multi ~memtrace =
  let profiling = Option.is_some profile_type_check_multi in
  if profiling then
    Relative_path.Map.iter files_info ~f:(fun fn (_fileinfo : FileInfo.t) ->
        let full_ast = Ast_provider.get_ast ctx fn ~full:true in
        let start_time = Unix.gettimeofday () in
        let _ = Typing_check_job.calc_errors_and_tast ctx fn ~full_ast in
        print_elapsed fn "first typecheck+decl" ~start_time);
  let tracer =
    Option.map memtrace ~f:(fun filename ->
        Memtrace.start_tracing
          ~context:None
          ~sampling_rate:Memtrace.default_sampling_rate
          ~filename)
  in
  let add_timing fn timings closure =
    let start_cpu = Sys.time () in
    let result = Lazy.force closure in
    let elapsed_cpu_time = Sys.time () -. start_cpu in
    let add_sample = function
      | Some samples -> Some (elapsed_cpu_time :: samples)
      | None -> Some [elapsed_cpu_time]
    in
    let timings = Relative_path.Map.update fn add_sample timings in
    (result, timings)
  in
  let rec go n timings =
    let (errors, timings) =
      Relative_path.Map.fold
        files_info
        ~f:(fun fn _fileinfo (errors, timings) ->
          let full_ast = Ast_provider.get_ast ctx fn ~full:true in
          let ((new_errors, _tast), timings) =
            add_timing fn timings
            @@ lazy (Typing_check_job.calc_errors_and_tast ctx fn ~full_ast)
          in
          (errors @ Errors.get_sorted_error_list new_errors, timings))
        ~init:(errors, timings)
    in
    if n > 1 then
      go (n - 1) timings
    else
      (errors, timings)
  in
  let n_of_times_to_typecheck =
    max 1 (Option.value ~default:1 profile_type_check_multi)
  in
  let timings = Relative_path.Map.empty in
  let (errors, timings) = go n_of_times_to_typecheck timings in
  let print_elapsed_cpu_time fn samples =
    let mean = mean samples in
    Printf.printf
      "%s: %d typechecks - %f ± %f (s)\n"
      (Relative_path.to_absolute fn |> Filename.basename)
      n_of_times_to_typecheck
      mean
      (standard_deviation mean samples)
  in
  if profiling then Relative_path.Map.iter timings ~f:print_elapsed_cpu_time;
  Option.iter tracer ~f:Memtrace.stop_tracing;
  errors

let create_nasts ctx files_info =
  let build_nast fn _ =
    let (syntax_errors, ast) =
      Ast_provider.get_ast_with_error ~full:true ctx fn
    in
    let error_list = Errors.get_sorted_error_list syntax_errors in
    List.iter error_list ~f:Errors.add_error;
    Naming.program ctx ast
  in
  Relative_path.Map.mapi ~f:build_nast files_info

(** This is an almost-pure function which returns what we get out of parsing.
The only side-effect it has is on the global errors list. *)
let parse_and_name ctx files_contents =
  Relative_path.Map.mapi files_contents ~f:(fun fn contents ->
      (* Get parse errors. *)
      let () =
        Errors.run_in_context fn (fun () ->
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
            ())
      in
      match Direct_decl_utils.direct_decl_parse ctx fn with
      | None -> failwith "no file contents"
      | Some decls -> Direct_decl_utils.decls_to_fileinfo fn decls)

(** This function is used for gathering naming and parsing errors,
and the side-effect of updating the global reverse naming table (and
picking up duplicate-name errors along the way), and for the side effect
of updating the decl heap (and picking up decling errors along the way). *)
let parse_name_and_decl ctx files_contents =
  Errors.do_ (fun () ->
      let files_info = parse_and_name ctx files_contents in
      Relative_path.Map.iter files_info ~f:(fun fn fileinfo ->
          let _failed_naming_fns =
            Naming_global.ndecl_file_and_get_conflict_files ctx fn fileinfo
          in
          ());
      Relative_path.Map.iter files_info ~f:(fun fn _ ->
          Decl.make_env ~sh:SharedMem.Uses ctx fn);

      files_info)

(** This function is used solely for its side-effect of putting decls into shared-mem *)
let add_decls_to_heap ctx files_contents =
  Errors.ignore_ (fun () ->
      let files_info = parse_and_name ctx files_contents in
      Relative_path.Map.iter files_info ~f:(fun fn _ ->
          Decl.make_env ~sh:SharedMem.Uses ctx fn));
  ()

(** This function doesn't have side-effects. Its sole job is to return shallow decls. *)
let get_shallow_decls ctx filename file_contents :
    Shallow_decl_defs.shallow_class SMap.t =
  let popt = Provider_context.get_popt ctx in
  let opts = DeclParserOptions.from_parser_options popt in
  (Direct_decl_parser.parse_decls opts filename file_contents)
    .Direct_decl_parser.pf_decls
  |> List.fold ~init:SMap.empty ~f:(fun acc (name, decl) ->
         match decl with
         | Shallow_decl_defs.Class c -> SMap.add name c acc
         | _ -> acc)

let test_shallow_class_diff ctx filename =
  let filename_after = Relative_path.to_absolute filename ^ ".after" in
  let contents1 = Sys_utils.cat (Relative_path.to_absolute filename) in
  let contents2 = Sys_utils.cat filename_after in
  let decls1 = get_shallow_decls ctx filename contents1 in
  let decls2 = get_shallow_decls ctx filename contents2 in
  let decls =
    SMap.merge (fun _ a b -> Some (a, b)) decls1 decls2 |> SMap.bindings
  in
  let diffs =
    List.map decls ~f:(fun (cid, old_and_new) ->
        ( Utils.strip_ns cid,
          match old_and_new with
          | (Some c1, Some c2) ->
            Shallow_class_diff.diff_class
              ctx
              (Provider_context.get_package_info ctx)
              c1
              c2
          | (None, None) -> ClassDiff.(Major_change MajorChange.Unknown)
          | (None, Some _) -> ClassDiff.(Major_change MajorChange.Added)
          | (Some _, None) -> ClassDiff.(Major_change MajorChange.Removed) ))
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
    if String.is_prefix contents ~prefix:"#!" then
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

(* Might raise because of Option.value_exn *)
let get_decls defs =
  ( SSet.fold
      (fun x acc ->
        Option.value_exn ~message:"Decl not found" (Decl_heap.Typedefs.get x)
        :: acc)
      defs.FileInfo.n_types
      [],
    SSet.fold
      (fun x acc ->
        Option.value_exn ~message:"Decl not found" (Decl_heap.Funs.get x) :: acc)
      defs.FileInfo.n_funs
      [],
    SSet.fold
      (fun x acc ->
        Option.value_exn ~message:"Decl not found" (Decl_heap.Classes.get x)
        :: acc)
      defs.FileInfo.n_classes
      [] )

let fail_comparison s =
  raise
    (Failure
       (Printf.sprintf "Comparing %s failed!\n" s
       ^ "It's likely that you added new positions to decl types "
       ^ "without updating Decl_pos_utils.NormalizeSig\n"))

let compare_typedefs t1 t2 =
  let t1 = Decl_pos_utils.NormalizeSig.typedef t1 in
  let t2 = Decl_pos_utils.NormalizeSig.typedef t2 in
  if Poly.(t1 <> t2) then fail_comparison "typedefs"

let compare_funs f1 f2 =
  let f1 = Decl_pos_utils.NormalizeSig.fun_elt f1 in
  let f2 = Decl_pos_utils.NormalizeSig.fun_elt f2 in
  if Poly.(f1 <> f2) then fail_comparison "funs"

let compare_classes c1 c2 =
  if Decl_compare.class_big_diff c1 c2 then fail_comparison "class_big_diff";

  let c1 = Decl_pos_utils.NormalizeSig.class_type c1 in
  let c2 = Decl_pos_utils.NormalizeSig.class_type c2 in
  let is_unchanged = Decl_compare.ClassDiff.compare c1 c2 in
  if not is_unchanged then fail_comparison "ClassDiff";

  let is_unchanged = Decl_compare.ClassEltDiff.compare c1 c2 in
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
            (fun k _ acc -> Relative_path.Map.remove acc k)
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
      | Some info ->
        SSet.of_list @@ List.map info.FileInfo.classes ~f:(fun (_, x, _) -> x)
    in
    (* We need to oldify, not remove, for ClassEltDiff to work *)
    Decl_redecl_service.oldify_type_decl
      ctx
      None
      get_classes
      ~bucket_size:1
      ~defs
      ~collect_garbage:false;

    let files_contents = Relative_path.Map.map files_contents ~f:add_newline in
    add_decls_to_heap ctx files_contents;
    let (typedefs2, funs2, classes2) = get_decls defs in
    List.iter2_exn typedefs1 typedefs2 ~f:compare_typedefs;
    List.iter2_exn funs1 funs2 ~f:compare_funs;
    List.iter2_exn classes1 classes2 ~f:compare_classes;
    ()

let compute_nasts ctx files_info interesting_files =
  let nasts = create_nasts ctx files_info in
  (* Interesting files are usually the non hhi ones. *)
  let filter_non_interesting nasts =
    Relative_path.Map.merge nasts interesting_files ~f:(fun _k nast x ->
        match (nast, x) with
        | (Some nast, Some _) -> Some nast
        | _ -> None)
  in
  filter_non_interesting nasts

(* Returns a list of Tast defs, along with associated type environments. *)
let compute_tasts ?(drop_fixmed = true) ctx files_info interesting_files :
    Errors.t * Tast.program Relative_path.Map.t =
  let _f _k nast x =
    match (nast, x) with
    | (Some nast, Some _) -> Some nast
    | _ -> None
  in
  Errors.do_ ~drop_fixmed (fun () ->
      let nasts = compute_nasts ctx files_info interesting_files in
      let tasts =
        Relative_path.Map.map nasts ~f:(fun nast ->
            let tast =
              Typing_toplevel.nast_to_tast ~do_tast_checks:true ctx nast
            in
            if
              TypecheckerOptions.enable_sound_dynamic
                (Provider_context.get_tcopt ctx)
            then
              List.concat (Tast_with_dynamic.all tast)
            else
              tast.Tast_with_dynamic.under_normal_assumptions)
      in
      tasts)

let compute_tasts_by_name ?(drop_fixmed = true) ctx files_info interesting_files
    : Errors.t * Tast.by_names Relative_path.Map.t =
  let (nast_errors, nasts) =
    Errors.do_ ~drop_fixmed (fun () ->
        compute_nasts ctx files_info interesting_files)
  in
  let errors_and_tasts =
    Relative_path.Map.mapi nasts ~f:(fun fn full_ast ->
        Typing_check_job.calc_errors_and_tast ctx ~drop_fixmed fn ~full_ast)
  in
  let tasts = Relative_path.Map.map ~f:snd errors_and_tasts in
  let tast_errors =
    Relative_path.Map.values errors_and_tasts |> List.map ~f:fst
  in

  ( List.fold (nast_errors :: tast_errors) ~init:Errors.empty ~f:Errors.merge,
    tasts )

(* Given source code containing a caret marker (e.g. "^ hover-at-caret"), return
   the line and column of the position indicated. *)
let caret_pos (src : string) (marker : string) : int * int =
  let lines = String.split_lines src in
  match
    List.findi lines ~f:(fun _ line ->
        String.is_substring line ~substring:marker)
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
    failwith
      (Printf.sprintf
         "Could not find any occurrence of '%s' in source code"
         marker)

(* Given source code containing the patterns [start_marker] and [end_marker], calculate the range between the markers *)
let find_ide_range src : Ide_api_types.range =
  let start_marker = "/*range-start*/" in
  let end_marker = "/*range-end*/" in
  let lines = String.split_lines src in
  let find_line marker =
    List.findi lines ~f:(fun _ line ->
        String.is_substring line ~substring:marker)
  in
  let find_marker marker after_or_before =
    let (line_zero_indexed, start_line_src) =
      find_line marker
      |> Option.value_exn
           ~message:(Format.sprintf "couldn't find marker %s" marker)
    in
    let line = line_zero_indexed + 1 in
    let column =
      let marker_start =
        Str.search_forward (Str.regexp_string marker) start_line_src 0
      in
      let column_adjustment =
        match after_or_before with
        | `After -> String.length marker + 1
        | `Before -> 1
      in
      marker_start + column_adjustment
    in
    Ide_api_types.{ line; column }
  in
  let st = find_marker start_marker `After in
  let ed = find_marker end_marker `Before in
  Ide_api_types.{ st; ed }

(**
 * Compute TASTs for some files, then expand all type variables.
 *)
let compute_tasts_expand_types ctx files_info interesting_files =
  let (errors, tasts) = compute_tasts ctx files_info interesting_files in
  let tasts = Relative_path.Map.map tasts ~f:(Tast_expand.expand_program ctx) in
  (errors, tasts)

let decl_parse_typecheck_and_then ctx files_contents f =
  let (parse_errors, files_info) = parse_name_and_decl ctx files_contents in
  let parse_errors = Errors.get_sorted_error_list parse_errors in
  let (errors, _tasts) =
    compute_tasts_expand_types ctx files_info files_contents
  in
  let errors = parse_errors @ Errors.get_sorted_error_list errors in
  if List.is_empty errors then
    f files_info
  else
    print_errors_if_present errors

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

let handle_constraint_mode
    ~do_
    name
    opts
    ctx
    error_format
    ~iter_over_files
    ~profile_type_check_multi
    ~memtrace =
  (* Process a single typechecked file *)
  let process_file path info =
    match info.FileInfo.file_mode with
    | Some FileInfo.Mstrict ->
      let (ctx, entry) = Provider_context.add_entry_if_missing ~ctx ~path in
      let { Tast_provider.Compute_tast.tast; _ } =
        Tast_provider.compute_tast_unquarantined ~ctx ~entry
      in
      let tast = List.concat (Tast_with_dynamic.all tast) in
      do_ opts ctx tast
    | _ ->
      (* We are not interested in partial files and there is nothing in HHI
         files to analyse *)
      ()
  in
  let print_errors = List.iter ~f:(print_error ~oc:stdout error_format) in
  (* Process a multifile that is not typechecked *)
  let process_multifile filename =
    Printf.printf
      "=== %s analysis results for %s\n%!"
      name
      (Relative_path.to_absolute filename);
    let files_contents = Multifile.file_to_files filename in
    let (parse_errors, file_info) = parse_name_and_decl ctx files_contents in
    let error_list = Errors.get_sorted_error_list parse_errors in
    let check_errors =
      check_file ctx error_list file_info ~profile_type_check_multi ~memtrace
    in
    if not (List.is_empty check_errors) then
      print_errors check_errors
    else
      Relative_path.Map.iter file_info ~f:process_file
  in
  let process_multifile filename =
    Provider_utils.respect_but_quarantine_unsaved_changes ~ctx ~f:(fun () ->
        process_multifile filename)
  in
  iter_over_files process_multifile

module File_deps = struct
  type dep_kind =
    | Hierarchy  (** Dependencies that come from class hierarchies *)
    | Regular
        (** Dependencies that come from method signatures, expressions, etc. *)

  type 'a deps = {
    hierarchy: 'a;
    regular: 'a;
  }

  let scrape_class_names (ast : Nast.program) : SSet.t deps =
    (* Visitor that collects class names from AST *)
    let name_collector () =
      object
        inherit [_] Aast.iter

        val names = HashSet.create ()

        (* Note that the unit is not strictly required here, however, this way it's more in line
           with regular OCaml. *)
        method names () = HashSet.fold names ~init:SSet.empty ~f:SSet.add

        method! on_class_name _ (_p, id) = HashSet.add names id
      end
    in
    (* Visitor that invokes [name_collector] on elements of a class hierarchy
       such as extends, implements, uses clauses *)
    let class_hierarchy_visitor name_collector =
      let open Aast in
      object
        inherit [_] Aast.iter

        method! on_Class _ class_ =
          List.iter class_.c_extends ~f:(name_collector#on_hint ());
          List.iter class_.c_uses ~f:(name_collector#on_hint ());
          List.iter class_.c_implements ~f:(name_collector#on_hint ());
          List.iter class_.c_reqs ~f:(name_collector#on_class_req ());
          List.iter
            class_.c_where_constraints
            ~f:(name_collector#on_where_constraint_hint ())
      end
    in
    (* Downstream we need to treat class names relating to class hierarchies
       defined in the file differently from names simply used in method signature
       and expressions.

       Below we do two passes over AST to collect all names and names coming from
       class hierarchies. This could be done more efficiently in one pass but the
       delta in performance is pretty small and overall it is good enough for the
       downstream consumers as-is. *)
    let all_names_collector = name_collector () in
    all_names_collector#on_program () ast;
    let hierarchy_names_collector = name_collector () in
    (class_hierarchy_visitor hierarchy_names_collector)#on_program () ast;
    let hierarchy_names = hierarchy_names_collector#names () in
    let regular_names =
      SSet.diff (all_names_collector#names ()) hierarchy_names
    in
    { hierarchy = hierarchy_names; regular = regular_names }

  (** Collect files that the given [file] depend on based on used class names. *)
  let collect_some_file_dependencies ctx (file : Relative_path.t) :
      Relative_path.Set.t deps =
    let open Hh_prelude in
    let nast = Ast_provider.get_ast ctx ~full:true file in
    let names =
      Errors.ignore_ (fun () -> Naming.program ctx nast) |> scrape_class_names
    in
    let resolve_to_path names =
      SSet.fold
        (fun class_name files ->
          match Naming_provider.get_class_path ctx class_name with
          | None -> files
          | Some file -> Relative_path.Set.add files file)
        names
        Relative_path.Set.empty
    in
    {
      hierarchy = resolve_to_path names.hierarchy;
      regular = resolve_to_path names.regular;
    }

  (** Recursively collect names in files and return the files where those names
  are defined. *)
  let traverse_file_dependencies
      ctx (files : Relative_path.t list) ~(depth : int) ~(full_hierarchy : bool)
      : Relative_path.t Seq.t =
    let module Pathtbl = Stdlib.Hashtbl.Make (Relative_path.S) in
    let pending = Queue.create () in
    let visited = Pathtbl.create 17 in
    (* The code below does BFS traversal of file dependencies. When
       [--get-some-file-deps-full-hierarchy] flag is set we collect deps that
       come from class hierarchies regardless of their distance from the input
       file.

       Note that in a chain [input file -> hierarhcy dep -> regular dep] regular
       dep is considered to have distance 2 from the input file. Alternatively,
       we could assume that hierarchy deps are always at distance 0 and then in
       the same chain regular dep would be at distance 1. However, this leads to
       a blow up in the number of deps we collect and is not practical. *)
    let enqueue_file file_depth kind file =
      let should_traverse =
        (match kind with
        | Hierarchy when full_hierarchy -> true
        | _ -> Int.(file_depth < depth))
        && not (Pathtbl.mem visited file)
      in
      if should_traverse then Queue.enqueue pending (file_depth, file);
      Pathtbl.replace visited file ()
    in
    List.iter files ~f:(fun file -> enqueue_file 0 Regular file);
    let iteration = ref 0 in
    while not (Queue.is_empty pending) do
      let (file_depth, file) = Queue.dequeue_exn pending in
      let dependencies = collect_some_file_dependencies ctx file in
      Relative_path.Set.iter
        dependencies.hierarchy
        ~f:(enqueue_file (file_depth + 1) Hierarchy);
      Relative_path.Set.iter
        dependencies.regular
        ~f:(enqueue_file (file_depth + 1) Regular);
      incr iteration;
      if Int.(!iteration mod 100 = 0) then
        Hh_logger.log
          "Iteration %d: visited %d, pending %d"
          !iteration
          (Pathtbl.length visited)
          (Queue.length pending)
    done;
    Pathtbl.to_seq_keys visited
end

(** Used for testing code that generates patches. *)
let codemod
    ~files_info
    ~files_contents
    ctx
    (get_patches :
      files_info:FileInfo.t Relative_path.Map.t -> ServerRenameTypes.patch list)
    =
  let decl_parse_typecheck_and_then = decl_parse_typecheck_and_then ctx in
  let backend = Provider_context.get_backend ctx in
  (* Because we repeatedly apply the codemod, positions change. So we need to
     re-parse, decl, and typecheck the file to generated accurate patches as
     well as amend the patched test files in memory. This involves
     invalidating a number of shared heaps and providing in memory
     replacements after patching files. *)
  let invalidate_heaps_and_update_files files_info files_contents =
    let paths_to_purge =
      Relative_path.Map.keys files_contents |> Relative_path.Set.of_list
    in
    (* Purge the file, then provide its replacement, otherwise the
       replacement is dropped on the floor. *)
    File_provider.remove_batch paths_to_purge;
    Relative_path.Map.iter
      ~f:File_provider.provide_file_for_tests
      files_contents;
    Ast_provider.remove_batch paths_to_purge;
    Relative_path.Map.iter
      ~f:(fun path file_info ->
        (* Don't invalidate builtins, otherwise, we can't find them. *)
        if not (Relative_path.prefix path |> Relative_path.is_hhi) then
          Naming_global.remove_decls_using_file_info backend file_info)
      files_info
  in
  let rec go files_info files_contents =
    invalidate_heaps_and_update_files files_info files_contents;
    decl_parse_typecheck_and_then files_contents @@ fun files_info ->
    let patches = get_patches ~files_info in
    let files_contents =
      ServerRenameTypes.apply_patches_to_file_contents files_contents patches
    in
    if List.is_empty patches then
      Multifile.print_files_as_multifile files_contents
    else
      go files_info files_contents
  in
  go files_info files_contents;

  (* Typecheck after the codemod is fully applied to confirm that what we
     produce is not garbage. *)
  Printf.printf
    "\nTypechecking after the codemod... (no output after this is good news)\n";
  invalidate_heaps_and_update_files files_info files_contents;
  decl_parse_typecheck_and_then files_contents ignore

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
    ~should_print_position
    ~profile_type_check_multi
    ~memtrace
    ~verbosity =
  let expect_single_file () : Relative_path.t =
    match filenames with
    | [x] -> x
    | _ -> die "Only single file expected"
  in
  let iter_over_files f : unit = List.iter filenames ~f in
  match mode with
  | Refactor_sound_dynamic (analysis_mode, refactor_mode, element_name) ->
    let opts =
      match
        ( Refactor_sd_options.parse_analysis_mode analysis_mode,
          Refactor_sd_options.parse_refactor_mode refactor_mode )
      with
      | (Some analysis_mode, Some refactor_mode) ->
        Refactor_sd_options.mk ~analysis_mode ~refactor_mode
      | (None, _) -> die "invalid refactor_sd analysis mode"
      | (_, None) -> die "invalid refactor_sd refactor mode"
    in
    handle_constraint_mode
      ~do_:(Refactor_sd.do_ element_name)
      "Sound Dynamic"
      opts
      ctx
      error_format
      ~iter_over_files
      ~profile_type_check_multi
      ~memtrace
  | SDT_analysis command ->
    handle_constraint_mode
      ~do_:(Sdt_analysis.do_ ~command ~on_bad_command:die ~verbosity)
      "SDT"
      ()
      ctx
      error_format
      ~iter_over_files
      ~profile_type_check_multi
      ~memtrace
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
  | Glean_index out_dir ->
    if
      (not (Disk.is_directory out_dir))
      || Array.length (Sys.readdir out_dir) > 0
    then (
      Printf.printf "%s should be an empty dir\n" out_dir;
      exit 1
    ) else
      Write_symbol_info.Entrypoint.index_files ctx ~out_dir ~files:filenames
  | Glean_sym_hash ->
    List.iter
      (Write_symbol_info.Entrypoint.sym_hashes ctx ~files:filenames)
      ~f:(fun (path, hash) -> Printf.printf "%s %s\n" path (Md5.to_hex hash))
  | Lint ->
    let lint_errors =
      Relative_path.Map.fold
        files_contents
        ~init:[]
        ~f:(fun fn content lint_errors ->
          lint_errors
          @ fst (Lints_core.do_ (fun () -> Linting_main.lint ctx fn content)))
    in
    if not (List.is_empty lint_errors) then (
      let lint_errors =
        List.sort
          ~compare:
            begin
              fun x y ->
                Pos.compare (Lints_core.get_pos x) (Lints_core.get_pos y)
            end
          lint_errors
      in
      let lint_errors = List.map ~f:Lints_core.to_absolute lint_errors in
      ServerLintTypes.output_text stdout lint_errors error_format;
      exit 2
    ) else
      Printf.printf "No lint errors\n"
  | Lint_json ->
    let json_errors =
      Relative_path.Map.fold
        files_contents
        ~init:[]
        ~f:(fun fn content json_errors ->
          json_errors
          @ fst (Lints_core.do_ (fun () -> Linting_main.lint ctx fn content)))
    in
    let json_errors =
      List.sort
        ~compare:
          begin
            fun x y ->
              Pos.compare (Lints_core.get_pos x) (Lints_core.get_pos y)
          end
        json_errors
    in
    let json_errors = List.map ~f:Lints_core.to_absolute json_errors in
    ServerLintTypes.output_json ~pretty:true stdout json_errors;
    exit 2
  | Dump_deps ->
    Relative_path.Map.iter files_info ~f:(fun fn _fileinfo ->
        let full_ast = Ast_provider.get_ast ctx fn ~full:true in
        ignore @@ Typing_check_job.calc_errors_and_tast ctx fn ~full_ast);
    if Hashtbl.length dbg_deps > 0 then dump_debug_deps dbg_deps
  | Dump_dep_hashes ->
    iter_over_files (fun _ ->
        let nasts = create_nasts ctx files_info in
        Relative_path.Map.iter nasts ~f:(fun _ nast ->
            Dep_hash_to_symbol.dump nast))
  | Get_some_file_deps { depth; full_hierarchy } ->
    let file_deps =
      File_deps.traverse_file_dependencies ctx filenames ~depth ~full_hierarchy
    in
    file_deps
    |> Seq.iter (fun file ->
           Printf.printf "%s\n" (Relative_path.to_absolute file))
  | Dump_inheritance ->
    let open ServerCommandTypes.Method_jumps in
    let naming_table = Naming_table.create files_info in
    Naming_table.iter naming_table ~f:(fun fn fileinfo ->
        if Relative_path.Map.mem builtins fn then
          ()
        else (
          List.iter fileinfo.FileInfo.classes ~f:(fun (_p, class_, _) ->
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
          List.iter fileinfo.FileInfo.classes ~f:(fun (_p, class_, _) ->
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
  | Ide_code_actions { title_prefix; use_snippet_edits } ->
    let path = expect_single_file () in
    let (ctx, entry) = Provider_context.add_entry_if_missing ~ctx ~path in
    let src = Provider_context.read_file_contents_exn entry in
    let range = find_ide_range src in
    Code_actions_cli_lib.run ctx entry range ~title_prefix ~use_snippet_edits
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
    let (errors, nasts) = Errors.do_ (fun () -> create_nasts ctx files_info) in
    print_errors_if_present (Errors.get_sorted_error_list errors);

    print_nasts
      ~should_print_position
      nasts
      (Relative_path.Map.keys files_contents)
  | Dump_tast ->
    let (errors, tasts) =
      compute_tasts_expand_types ctx files_info files_contents
    in
    print_errors_if_present (parse_errors @ Errors.get_sorted_error_list errors);
    print_tasts ~should_print_position tasts ctx
  | Dump_stripped_tast ->
    iter_over_files (fun filename ->
        let files_contents =
          Relative_path.Map.filter files_contents ~f:(fun k _v ->
              Relative_path.equal k filename)
        in
        let (_, tasts) = compute_tasts ctx files_info files_contents in
        let tast = Relative_path.Map.find tasts filename in
        let nast = Tast.to_nast tast in
        Printf.printf "%s\n" (Nast.show_program nast))
  | RemoveDeadUnsafeCasts ->
    let ctx =
      Provider_context.map_tcopt ctx ~f:(fun tcopt ->
          GlobalOptions.{ tcopt with tco_populate_dead_unsafe_cast_heap = true })
    in
    let get_patches ~files_info =
      Remove_dead_unsafe_casts.get_patches
        ~is_test:true
        ~files_info
        ~fold:Relative_path.Map.fold
    in
    codemod ~files_info ~files_contents ctx get_patches
  | Find_refs (line, column) ->
    let path = expect_single_file () in
    let naming_table = Naming_table.create files_info in
    let genv = ServerEnvBuild.default_genv in
    let init_id = Random_id.short_string () in
    let env =
      {
        (ServerEnvBuild.make_env
           ~init_id
           ~deps_mode:(Typing_deps_mode.InMemoryMode None)
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
    let open ServerCommandTypes.Done_or_retry in
    Provider_utils.respect_but_quarantine_unsaved_changes ~ctx ~f:(fun () ->
        match ServerFindRefs.go_from_file_ctx ~ctx ~entry ~line ~column with
        | None -> ()
        | Some (name, action) ->
          let results =
            match
              ServerFindRefs.go
                ctx
                action
                include_defs
                ~stream_file:None
                ~hints:[]
                genv
                env
            with
            | (_env, Done r) -> ServerFindRefs.to_absolute r
            | (_env, Retry) -> failwith "didn't expect retry"
          in
          Printf.printf "%s\n" name;
          FindRefsWireFormat.CliHumanReadable.print_results (List.rev results))
  | Go_to_impl (line, column) ->
    let filename = expect_single_file () in
    let naming_table = Naming_table.create files_info in
    let genv = ServerEnvBuild.default_genv in
    let init_id = Random_id.short_string () in
    let env =
      {
        (ServerEnvBuild.make_env
           ~init_id
           ~deps_mode:(Typing_deps_mode.InMemoryMode None)
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
    let open ServerCommandTypes.Done_or_retry in
    begin
      match ServerFindRefs.go_from_file_ctx ~ctx ~entry ~line ~column with
      | None -> ()
      | Some (name, action) ->
        let results =
          match ServerGoToImpl.go ~action ~genv ~env with
          | (_env, Done r) -> ServerFindRefs.to_absolute r
          | (_env, Retry) -> failwith "didn't expect retry"
        in
        Printf.printf "%s\n" name;
        FindRefsWireFormat.CliHumanReadable.print_results (List.rev results)
    end
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
                  File_provider.(provide_file_for_tests filename contents));
              let (parse_errors, individual_file_info) =
                parse_name_and_decl ctx files_contents
              in
              let errors =
                check_file
                  ctx
                  (Errors.get_sorted_error_list parse_errors)
                  individual_file_info
                  ~profile_type_check_multi
                  ~memtrace
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
            with
            | e ->
              let msg = Exn.to_string e in
              Out_channel.output_string oc msg);
        Out_channel.close oc)
  | Errors ->
    (* Don't typecheck builtins *)
    let errors =
      check_file ctx parse_errors files_info ~profile_type_check_multi ~memtrace
    in
    print_error_list error_format errors max_errors;
    if not (List.is_empty errors) then exit 2
  | Decl_compare ->
    let filename = expect_single_file () in
    (* Might raise because of Option.value_exn *)
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
        let env =
          Typing_env_types.empty ctx Relative_path.default ~droot:None
        in
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
        in
        let abstract =
          Typing_defs.(
            match ttc.ttc_kind with
            | TCConcrete _ -> ""
            | TCAbstract _ -> "abstract ")
        in
        Printf.printf "  %stypeconst%s: %s %s\n" abstract from mid ty);
      ())
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
        caret_pos src "^ hover-at-caret"
    in
    let results = ServerHover.go_quarantined ~ctx ~entry ~line ~column in
    let formatted_results =
      List.map
        ~f:(fun r ->
          let open HoverService in
          String.concat ~sep:"\n" (r.snippet :: r.addendum))
        results
    in
    Printf.printf
      "%s\n"
      (String.concat ~sep:"\n-------------\n" formatted_results)
  | Apply_quickfixes ->
    let path = expect_single_file () in
    let (ctx, entry) = Provider_context.add_entry_if_missing ~ctx ~path in
    let (errors, _) =
      compute_tasts ~drop_fixmed:false ctx files_info files_contents
    in
    let src = Relative_path.Map.find files_contents path in

    let quickfixes =
      Errors.get_error_list ~drop_fixmed:false errors
      |> List.map ~f:(fun e ->
             (* If an error has multiple possible quickfixes, take the first. *)
             List.hd (User_error.quickfixes e))
      |> List.filter_opt
    in

    let cst = Ast_provider.compute_cst ~ctx ~entry in
    let tree = Provider_context.PositionedSyntaxTree.root cst in

    let classish_starts =
      match entry.Provider_context.source_text with
      | Some source_text ->
        Quickfix_ffp.classish_starts
          tree
          source_text
          entry.Provider_context.path
      | None -> SMap.empty
    in

    (* Print the title of each quickfix, so we can see text changes in tests. *)
    List.iter quickfixes ~f:(fun qf ->
        Printf.printf "%s\n" (Quickfix.get_title qf));

    (* Print the source code after applying all these quickfixes. *)
    Printf.printf "\n%s" (Quickfix.apply_all src classish_starts quickfixes)
  | CountImpreciseTypes ->
    let (errors, tasts) =
      compute_tasts_expand_types ctx files_info files_contents
    in
    if not @@ Errors.is_empty errors then (
      print_errors error_format errors max_errors;
      Printf.printf
        "Did not count imprecise types because there are typing errors.";
      exit 2
    ) else
      let tasts = Relative_path.Map.values tasts in
      let results =
        List.map ~f:(Count_imprecise_types.count ctx) tasts
        |> List.fold
             ~f:(SMap.union ~combine:(fun id _ -> failwith ("Clash at " ^ id)))
             ~init:SMap.empty
      in
      let json = Count_imprecise_types.json_of_results results in
      Printf.printf "%s" (Hh_json.json_to_string json)
  | Get_type_hierarchy ->
    let path = expect_single_file () in
    let (ctx, entry) = Provider_context.add_entry_if_missing ~ctx ~path in
    let src = Provider_context.read_file_contents_exn entry in
    let (line, column) = caret_pos src "^ type-hierarchy-at-caret" in
    let results =
      ServerTypeHierarchy.go_quarantined ~ctx ~entry ~line ~column
    in
    let json = ServerTypeHierarchy.json_of_results ~results in
    Printf.printf "%s" (Hh_json.json_to_string ~pretty:true json)
  | Map_reduce_mode ->
    let (errors, tasts) = compute_tasts_by_name ctx files_info files_contents in
    print_errors_if_present (parse_errors @ Errors.get_sorted_error_list errors);
    let mapped =
      Relative_path.Map.elements tasts
      |> List.map ~f:(fun (fn, tasts) -> Map_reduce.map ctx fn tasts errors)
    in
    let reduced =
      List.fold mapped ~init:Map_reduce.empty ~f:Map_reduce.reduce
    in
    let json = Map_reduce_ffi.yojson_of_t (Map_reduce.to_ffi reduced) in
    Yojson.Safe.pretty_to_channel Caml.stdout json

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
      custom_hhi_path;
      profile_type_check_multi;
      memtrace;
      rust_provider_backend;
      naming_table_path;
      packages_config_path;
    }
    (hhi_root : Path.t) : unit =
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
      let hhi_builtins =
        match custom_hhi_path with
        | None -> hhi_builtins
        | Some path -> Array.of_list (Hhi_get.get_hhis_in_dir path)
      in
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
      let canonicalize_path path =
        match Sys_utils.realpath path with
        | Some path -> Relative_path.create_detect_prefix path
        | None -> failwith ("Couldn't resolve realpath of " ^ path)
      in
      files |> List.map ~f:canonicalize_path
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
          (fun k src acc -> Relative_path.Map.add acc ~key:k ~data:src)
        end
      ~init:files_contents
  in
  Relative_path.Map.iter files_contents ~f:(fun filename contents ->
      File_provider.(provide_file_for_tests filename contents));
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
    Typing_deps.add_dependency_callback ~name:"get_debug_trace" get_debug_trace
  | _ -> ());
  let package_info =
    match packages_config_path with
    | None -> PackageInfo.empty
    | Some _ ->
      let (_errors, info) =
        PackageConfig.load_and_parse
          ~pkgs_config_abs_path:packages_config_path
          ()
      in
      info
  in
  let tcopt = { tcopt with GlobalOptions.tco_package_info = package_info } in
  let ctx =
    if rust_provider_backend then
      let backend = Hh_server_provider_backend.make tcopt in
      let () = Provider_backend.set_rust_backend backend in
      Provider_context.empty_for_tool
        ~popt:tcopt
        ~tcopt
        ~backend:(Provider_backend.get ())
        ~deps_mode:(Typing_deps_mode.InMemoryMode None)
    else
      Provider_context.empty_for_test
        ~popt:tcopt
        ~tcopt
        ~deps_mode:(Typing_deps_mode.InMemoryMode None)
  in

  (* We make the following call for the side-effect of updating ctx's "naming-table fallback"
     so it will look in the sqlite database for names it doesn't know.
     This function returns the forward naming table. *)
  let naming_table_for_root : Naming_table.t option =
    Option.map naming_table_path ~f:(fun path ->
        Naming_table.load_from_sqlite ctx path)
  in
  (* If run in naming-table mode, we first have to remove any old names from the files we're about to redeclare --
     otherwise when we declare them it'd count as a duplicate definition! *)
  Option.iter naming_table_for_root ~f:(fun naming_table_for_root ->
      Relative_path.Map.iter files_contents ~f:(fun file _content ->
          let file_info =
            Naming_table.get_file_info naming_table_for_root file
          in
          Option.iter file_info ~f:(fun file_info ->
              let ids_to_strings ids =
                List.map ids ~f:(fun (_, name, _) -> name)
              in
              Naming_global.remove_decls
                ~backend:(Provider_context.get_backend ctx)
                ~funs:(ids_to_strings file_info.FileInfo.funs)
                ~classes:(ids_to_strings file_info.FileInfo.classes)
                ~typedefs:(ids_to_strings file_info.FileInfo.typedefs)
                ~consts:(ids_to_strings file_info.FileInfo.consts)
                ~modules:(ids_to_strings file_info.FileInfo.modules))));

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
    ~should_print_position
    ~profile_type_check_multi
    ~memtrace
    ~verbosity

let main_hack opts (root : Path.t) (sharedmem_config : SharedMem.config) : unit
    =
  (* TODO: We should have a per file config *)
  Sys_utils.signal Sys.sigusr1 (Sys.Signal_handle Typing.debug_print_last_pos);
  EventLogger.init_fake ();
  ServerProgress.disable ();
  Measure.push_global ();

  let (_handle : SharedMem.handle) =
    SharedMem.init ~num_workers:0 sharedmem_config
  in
  let process custom hhi_root =
    if custom then
      let hhi_root_s = Path.to_string hhi_root in
      if Disk.file_exists hhi_root_s && Disk.is_directory hhi_root_s then
        Hhi.set_custom_hhi_root hhi_root
      else
        die ("Custom hhi directory " ^ hhi_root_s ^ " not found")
    else
      Hhi.set_hhi_root_for_unit_test hhi_root;
    Relative_path.set_path_prefix Relative_path.Root root;
    Relative_path.set_path_prefix Relative_path.Hhi hhi_root;
    Relative_path.set_path_prefix Relative_path.Tmp (Path.make "tmp");
    decl_and_run_mode opts hhi_root;
    TypingLogger.flush_buffers ()
  in
  match opts.custom_hhi_path with
  | Some hhi_root -> process true (Path.make hhi_root)
  | None -> Tempfile.with_tempdir (fun hhi_root -> process false hhi_root)

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
  let (options, root, sharedmem_config) = parse_options () in
  Unix.handle_unix_error main_hack options root sharedmem_config
