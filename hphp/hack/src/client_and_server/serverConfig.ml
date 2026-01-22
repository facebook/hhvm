(*
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

(**
 * Parses and gathers information from the .hhconfig in the repo.
 *)

open Hh_prelude
open Config_file.Getters
open Reordered_argument_collections
open ServerLocalConfig

type t = {
  version: Config_file.version; [@printer (fun fmt _ -> fprintf fmt "version")]
  load_script_timeout: int;
  (* in seconds *)
  (* Configures only the workers. Workers can have more relaxed GC configs as
   * they are short-lived processes *)
  gc_control: Gc.control; [@printer (fun fmt _ -> fprintf fmt "control")]
  sharedmem_config: SharedMem.config;
  tc_options: TypecheckerOptions.t;
  parser_options: ParserOptions.t;
  glean_options: GleanOptions.t;
  symbol_write_options: SymbolWriteOptions.t;
  formatter_override: Path.t option;
  config_hash: string option;
  (* A list of regexps for paths to ignore *)
  ignored_paths: string list;
  (* A list of extra paths to search for declarations *)
  extra_paths: Path.t list;
  warn_on_non_opt_build: bool;
  (* clientIdeDaemon will fall back to performing a full index of files to construct a naming table if it fails to load one. *)
  ide_fall_back_to_full_index: bool;
  naming_table_compression_level: int;
  naming_table_compression_threads: int;
  warnings_generated_files: string list;
      (** List of regex patterns for file paths for which any warning will be ignored.
        Useful to ignore warnings from certain generated files.
        Passing `-Wgenerated` on the command line will override this behavior
        and show warnings from those files.
        These are sent to Rust via FFI so are not stored as regexes here.
         *)
}
[@@deriving show]

let repo_config_path =
  Relative_path.from_root ~suffix:Config_file.file_path_relative_to_repo_root

let is_compatible c1 c2 =
  (* This comparison can eventually be made more complex; we may not always
   * need to restart hh_server, e.g. changing the path to the load script
   * is immaterial*)
  Poly.equal c1 c2

let make_gc_control config =
  let { Gc.Control.minor_heap_size; space_overhead; _ } =
    GlobalConfig.gc_control
  in
  let minor_heap_size =
    int_ "gc_minor_heap_size" ~default:minor_heap_size config
  in
  let space_overhead =
    int_ "gc_space_overhead" ~default:space_overhead config
  in
  { GlobalConfig.gc_control with Gc.Control.minor_heap_size; space_overhead }

let make_sharedmem_config config local_config =
  let { SharedMem.global_size; heap_size; shm_min_avail; _ } =
    SharedMem.default_config
  in
  let shm_dirs = local_config.ServerLocalConfig.shm_dirs in
  let global_size = int_ "sharedmem_global_size" ~default:global_size config in
  let heap_size = int_ "sharedmem_heap_size" ~default:heap_size config in
  let hash_table_pow = int_ "sharedmem_hash_table_pow" ~default:18 config in
  let log_level = int_ "sharedmem_log_level" ~default:0 config in
  let sample_rate = float_ "sharedmem_sample_rate" ~default:0.0 config in
  let compression = int_ "sharedmem_compression" ~default:0 config in
  let shm_dirs = string_list "sharedmem_dirs" ~default:shm_dirs config in
  let shm_use_sharded_hashtbl =
    bool_
      "shm_use_sharded_hashtbl"
      ~default:local_config.ServerLocalConfig.shm_use_sharded_hashtbl
      config
  in
  let shm_cache_size =
    int_
      "shm_cache_size"
      ~default:local_config.ServerLocalConfig.shm_cache_size
      config
  in
  let shm_min_avail =
    int_ "sharedmem_minimum_available" ~default:shm_min_avail config
  in
  {
    SharedMem.global_size;
    heap_size;
    hash_table_pow;
    log_level;
    sample_rate;
    shm_dirs;
    shm_use_sharded_hashtbl;
    shm_cache_size;
    shm_min_avail;
    compression;
  }

let config_list_regexp = Str.regexp "[, \t]+"

let process_experimental sl =
  match List.map sl ~f:String.lowercase with
  | ["false"] -> SSet.empty
  | ["true"] -> TypecheckerOptions.experimental_all
  | features -> List.fold_left features ~f:SSet.add ~init:SSet.empty

let config_experimental_tc_features config =
  Option.map
    (Config_file.Getters.string_opt "enable_experimental_tc_features" config)
    ~f:(fun list_str ->
      let sl = Str.split config_list_regexp list_str in
      process_experimental sl)

let config_experimental_stx_features config =
  Option.map
    (Config_file.Getters.string_opt "enable_experimental_stx_features" config)
    ~f:(fun str ->
      let json = Hh_json.json_of_string ~strict:true str in
      let pairs = Hh_json.get_object_exn json in
      SMap.of_list
        (List.map pairs ~f:Experimental_features.parse_experimental_feature))

let process_migration_flags sl =
  match sl with
  | ["false"] -> SSet.empty
  | ["true"] -> TypecheckerOptions.migration_flags_all
  | flags ->
    List.iter flags ~f:(fun s ->
        if not (SSet.mem TypecheckerOptions.migration_flags_all s) then
          failwith ("invalid migration flag: " ^ s));
    List.fold_left flags ~f:SSet.add ~init:SSet.empty

let config_tc_migration_flags config =
  Option.map
    (Config_file.Getters.string_opt "enable_tc_migration_flags" config)
    ~f:(fun list_str ->
      Str.split config_list_regexp list_str
      |> List.map ~f:String.lowercase
      |> process_migration_flags)

let convert_paths str =
  let json = Hh_json.json_of_string ~strict:true str in
  let l = Hh_json.get_array_exn json in
  List.filter_map
    ~f:(fun s ->
      match s with
      | Hh_json.JSON_String path -> Some path
      | _ -> None)
    l

let process_ignored_paths config =
  Config_file.Getters.string_opt "ignored_paths" config
  |> Option.value_map ~f:convert_paths ~default:[]

let maybe_relative_path fn =
  (* Note: this is not the same as calling realpath; the cwd is not
   * necessarily the same as hh_server's root!!! *)
  Path.make
    begin
      if Filename.is_relative fn then
        Relative_path.(to_absolute (from_root ~suffix:fn))
      else
        fn
    end

let process_extra_paths config =
  match Config_file.Getters.string_opt "extra_paths" config with
  | Some s -> Str.split config_list_regexp s |> List.map ~f:maybe_relative_path
  | _ -> []

let process_untrusted_mode config =
  match Config_file.Getters.string_opt "untrusted_mode" config with
  | Some s ->
    if bool_of_string s then
      let blacklist =
        [
          (* out of tree file access*)
          "extra_paths";
          (* potential resource abuse *)
          "language_feature_logging";
        ]
      in
      let prefix_blacklist =
        [(* potential resource abuse *) "gc_"; "sharedmem_"]
      in
      let invalid_keys =
        List.filter (Config_file.keys config) ~f:(fun ck ->
            let ck = String.lowercase ck in
            let exact_match =
              List.find ~f:(fun bli -> String.equal bli ck) blacklist
            in
            let prefix_match =
              List.find
                ~f:(fun blp -> String.is_prefix ck ~prefix:blp)
                prefix_blacklist
            in
            match (exact_match, prefix_match) with
            | (None, None) -> false
            | _ -> true)
      in
      if not (List.is_empty invalid_keys) then
        failwith
          ("option not permitted in untrusted_mode: "
          ^ String.concat ~sep:", " invalid_keys)
      else
        failwith "untrusted_mode can only be enabled, not disabled"
  | _ -> ()

let extract_auto_namespace_element ns_map element =
  match element with
  | (source, Hh_json.JSON_String target) -> (source, target) :: ns_map
  | _ ->
    (* This means the JSON we received is incorrect *)
    ns_map

let convert_auto_namespace_to_map map =
  let json = Hh_json.json_of_string ~strict:true map in
  let pairs = Hh_json.get_object_exn json in
  (* We do a fold instead of a map to filter
   * out the incorrect entrie as we look at each item *)
  List.fold_left ~init:[] ~f:extract_auto_namespace_element pairs

let prepare_auto_namespace_map config =
  Option.map
    (Config_file.Getters.string_opt "auto_namespace_map" config)
    ~f:convert_auto_namespace_to_map

let extract_log_level = function
  | (log_key, Hh_json.JSON_Number log_level) -> begin
    match int_of_string_opt log_level with
    | Some log_level -> (log_key, log_level)
    | None -> failwith "non-integer log level value"
  end
  | _ -> failwith "non-integer log level value"

let convert_log_levels_to_map map =
  let json = Hh_json.json_of_string ~strict:true map in
  let pairs = Hh_json.get_object_exn json in
  List.map ~f:extract_log_level pairs |> SMap.of_list

let prepare_log_levels config =
  Option.map
    (Config_file.Getters.string_opt "log_levels" config)
    ~f:convert_log_levels_to_map

let prepare_iset config config_name =
  Option.map
    (Config_file.Getters.string_opt config_name config)
    ~f:(fun list_str ->
      Str.split config_list_regexp list_str
      |> List.map ~f:int_of_string
      |> List.fold_right ~init:ISet.empty ~f:ISet.add)

let prepare_allowed_decl_fixme_codes config =
  prepare_iset config "allowed_decl_fixme_codes"

let reasons_config_opt config =
  Option.bind (string_opt "extended_reasons" config) ~f:(fun data_str ->
      if String.equal data_str "debug" then
        Some GlobalOptions.Debug
      else if String.equal data_str "legacy" then
        Some GlobalOptions.Legacy
      else
        Option.map ~f:(fun n -> GlobalOptions.Extended n)
        @@ int_of_string_opt data_str)

let load_config (config : Config_file_common.t) (options : GlobalOptions.t) :
    GlobalOptions.t =
  let ( >?? ) x y = Option.value x ~default:y in
  let po_opt = options.GlobalOptions.po in
  let experimental_features = config_experimental_stx_features config in
  let po =
    ParserOptions.
      {
        (* These aren't set in the config file, so init them to their defaults. *)
        hhvm_compat_mode = po_opt.hhvm_compat_mode;
        allow_unstable_features = po_opt.allow_unstable_features;
        hhi_mode = po_opt.hhi_mode;
        codegen = po_opt.codegen;
        stack_size = po_opt.stack_size;
        no_parser_readonly_check = po_opt.no_parser_readonly_check;
        unwrap_concurrent = po_opt.unwrap_concurrent;
        (* Never use the legacy mode for hh_server *)
        use_legacy_experimental_feature_config = false;
        (* The remainder are set in the config file *)
        is_systemlib = bool_opt "is_systemlib" config >?? po_opt.is_systemlib;
        disable_lval_as_an_expression =
          bool_opt "disable_lval_as_an_expression" config
          >?? po_opt.disable_lval_as_an_expression;
        disable_legacy_soft_typehints =
          bool_opt "disable_legacy_soft_typehints" config
          >?? po_opt.disable_legacy_soft_typehints;
        disable_legacy_attribute_syntax =
          bool_opt "disable_legacy_attribute_syntax" config
          >?? po_opt.disable_legacy_attribute_syntax;
        const_default_func_args =
          bool_opt "const_default_func_args" config
          >?? po_opt.const_default_func_args;
        const_default_lambda_args =
          bool_opt "const_default_lambda_args" config
          >?? po_opt.const_default_lambda_args;
        const_static_props =
          bool_opt "const_static_props" config >?? po_opt.const_static_props;
        abstract_static_props =
          bool_opt "abstract_static_props" config
          >?? po_opt.abstract_static_props;
        disallow_func_ptrs_in_constants =
          bool_opt "disallow_func_ptrs_in_constants" config
          >?? po_opt.disallow_func_ptrs_in_constants;
        disable_xhp_element_mangling =
          bool_opt "disable_xhp_element_mangling" config
          >?? po_opt.disable_xhp_element_mangling;
        disable_xhp_children_declarations =
          bool_opt "disable_xhp_children_declarations" config
          >?? po_opt.disable_xhp_children_declarations;
        enable_xhp_class_modifier =
          bool_opt "enable_xhp_class_modifier" config
          >?? po_opt.enable_xhp_class_modifier;
        interpret_soft_types_as_like_types =
          bool_opt "interpret_soft_types_as_like_types" config
          >?? po_opt.interpret_soft_types_as_like_types;
        disallow_static_constants_in_default_func_args =
          bool_opt "disallow_static_constants_in_default_func_args" config
          >?? po_opt.disallow_static_constants_in_default_func_args;
        auto_namespace_map =
          prepare_auto_namespace_map config >?? po_opt.auto_namespace_map;
        everything_sdt =
          bool_opt "everything_sdt" config >?? po_opt.everything_sdt;
        keep_user_attributes =
          bool_opt "keep_user_attributes" config >?? po_opt.keep_user_attributes;
        deregister_php_stdlib =
          bool_opt "deregister_php_stdlib" config
          >?? po_opt.deregister_php_stdlib;
        union_intersection_type_hints =
          bool_opt "union_intersection_type_hints" config
          >?? po_opt.union_intersection_type_hints;
        disallow_silence =
          bool_opt "disallow_silence" config >?? po_opt.disallow_silence;
        allowed_decl_fixme_codes =
          prepare_allowed_decl_fixme_codes config
          >?? po_opt.allowed_decl_fixme_codes;
        disable_hh_ignore_error =
          int_opt "disable_hh_ignore_error" config
          >?? po_opt.disable_hh_ignore_error;
        experimental_features =
          experimental_features >?? po_opt.experimental_features;
        (* If there was no experimental features status list in configuration,
           consider all existing experimental features to be released and hence
           usable. *)
        consider_unspecified_experimental_features_released =
          Option.is_none experimental_features;
        package_info = po_opt.package_info;
        package_support_multifile_tests =
          bool_opt "package_support_multifile_tests" config
          >?? po_opt.package_support_multifile_tests;
        enable_class_pointer_hint =
          bool_opt "enable_class_pointer_hint" config
          >?? po_opt.enable_class_pointer_hint;
        disallow_non_annotated_memoize =
          bool_opt "disallow_non_annotated_memoize" config
          >?? po_opt.disallow_non_annotated_memoize;
        treat_non_annotated_memoize_as_kbic =
          bool_opt "treat_non_annotated_memoize_as_kbic" config
          >?? po_opt.treat_non_annotated_memoize_as_kbic;
        ignore_string_methods =
          bool_opt "ignore_string_methods" config
          >?? po_opt.ignore_string_methods;
      }
  in
  GlobalOptions.set
    ~po
    ?tco_language_feature_logging:(bool_opt "language_feature_logging" config)
    ?tco_timeout:(int_opt "timeout" config)
    ?tco_constraint_array_index_assign:
      (bool_opt "constraint_array_index_assign" config)
    ?tco_constraint_method_call:(bool_opt "constraint_method_call" config)
    ?code_agnostic_fixme:(bool_opt "code_agnostic_fixme" config)
    ?allowed_fixme_codes_strict:
      (prepare_iset config "allowed_fixme_codes_strict")
    ?tco_experimental_features:(config_experimental_tc_features config)
    ?tco_migration_flags:(config_tc_migration_flags config)
    ?tco_coeffects:(bool_opt "call_coeffects" config)
    ?tco_coeffects_local:(bool_opt "local_coeffects" config)
    ?tco_like_casts:(bool_opt "like_casts" config)
    ?tco_check_xhp_attribute:(bool_opt "check_xhp_attribute" config)
    ?tco_check_redundant_generics:(bool_opt "check_redundant_generics" config)
    ?tco_disallow_unresolved_type_variables:
      (bool_opt "disallow_unresolved_type_variables" config)
    ?tco_locl_cache_capacity:(int_opt "locl_cache_capacity" config)
    ?tco_locl_cache_node_threshold:(int_opt "locl_cache_node_threshold" config)
    ?po_disallow_toplevel_requires:
      (bool_opt "disallow_toplevel_requires" config)
    ?tco_const_attribute:(bool_opt "const_attribute" config)
    ?tco_check_attribute_locations:(bool_opt "check_attribute_locations" config)
    ?tco_type_refinement_partition_shapes:
      (bool_opt "type_refinement_partition_shapes" config)
    ?glean_reponame:(string_opt "glean_reponame" config)
    ?symbol_write_index_inherited_members:
      (bool_opt "symbol_write_index_inherited_members" config)
    ?symbol_write_ownership:(bool_opt "symbol_write_ownership" config)
    ?symbol_write_root_path:(string_opt "symbol_write_root_path" config)
    ?symbol_write_hhi_path:(string_opt "symbol_write_hhi_path" config)
    ?symbol_write_ignore_paths:
      (string_list_opt "symbol_write_ignore_paths" config)
    ?symbol_write_index_paths:
      (string_list_opt "symbol_write_index_paths" config)
    ?symbol_write_index_paths_file:
      (string_opt "symbol_write_index_paths_file" config)
    ?symbol_write_index_paths_file_output:
      (string_opt "symbol_write_index_paths_file_output" config)
    ?symbol_write_include_hhi:(bool_opt "symbol_write_include_hhi" config)
    ?symbol_write_sym_hash_in:(string_opt "symbol_write_sym_hash_in" config)
    ?symbol_write_exclude_out:(string_opt "symbol_write_exclude_out" config)
    ?symbol_write_referenced_out:
      (string_opt "symbol_write_referenced_out" config)
    ?symbol_write_reindexed_out:(string_opt "symbol_write_reindexed_out" config)
    ?symbol_write_sym_hash_out:(bool_opt "symbol_write_sym_hash_out" config)
    ?tco_error_php_lambdas:(bool_opt "error_php_lambdas" config)
    ?tco_disallow_discarded_nullable_awaitables:
      (bool_opt "disallow_discarded_nullable_awaitables" config)
    ?tco_typecheck_sample_rate:(float_opt "typecheck_sample_rate" config)
    ?tco_pessimise_builtins:(bool_opt "pessimise_builtins" config)
    ?tco_enable_no_auto_dynamic:(bool_opt "enable_no_auto_dynamic" config)
    ?tco_skip_check_under_dynamic:(bool_opt "skip_check_under_dynamic" config)
    ?tco_enable_function_references:
      (bool_opt "enable_function_references" config)
    ?tco_ignore_unsafe_cast:(bool_opt "ignore_unsafe_cast" config)
    ?tco_allowed_expression_tree_visitors:
      (Option.map
         (string_list_opt "allowed_expression_tree_visitors" config)
         ~f:(fun l -> List.map l ~f:Utils.add_ns))
    ?tco_typeconst_concrete_concrete_error:
      (bool_opt "typeconst_concrete_concrete_error" config)
    ?tco_meth_caller_only_public_visibility:
      (bool_opt "meth_caller_only_public_visibility" config)
    ?tco_require_extends_implements_ancestors:
      (bool_opt "require_extends_implements_ancestors" config)
    ?tco_strict_value_equality:(bool_opt "strict_value_equality" config)
    ?tco_enforce_sealed_subclasses:(bool_opt "enforce_sealed_subclasses" config)
    ?tco_implicit_inherit_sdt:(bool_opt "implicit_inherit_sdt" config)
    ?tco_repo_stdlib_path:(string_opt "repo_stdlib_path" config)
    ?tco_explicit_consistent_constructors:
      (int_opt "explicit_consistent_constructors" config)
    ?tco_require_types_class_consts:
      (int_opt "require_types_tco_require_types_class_consts" config)
    ?tco_check_bool_for_condition:(int_opt "check_bool_for_condition" config)
    ?tco_type_printer_fuel:(int_opt "type_printer_fuel" config)
    ?tco_profile_top_level_definitions:
      (bool_opt "profile_top_level_definitions" config)
    ?tco_typecheck_if_name_matches_regexp:
      (string_opt "typecheck_if_name_matches_regexp" config)
    ?log_levels:(prepare_log_levels config)
    ?tco_allowed_files_for_module_declarations:
      (string_list_opt "allowed_files_for_module_declarations" config)
    ?tco_allow_all_files_for_module_declarations:
      (bool_opt "allow_all_files_for_module_declarations" config)
    ?tco_populate_dead_unsafe_cast_heap:
      (bool_opt "populate_dead_unsafe_cast_heap" config)
    ?dump_tast_hashes:(bool_opt "dump_tast_hashes" config)
    ?warnings_default_all:(bool_opt "warnings_default_all" config)
    ?tco_allowed_files_for_ignore_readonly:
      (string_list_opt "allowed_files_for_ignore_readonly" config)
    ?tco_package_allow_typedef_violations:
      (bool_opt "package_allow_typedef_violations" config)
    ?tco_package_allow_classconst_violations:
      (bool_opt "package_allow_classconst_violations" config)
    ?tco_package_allow_reifiable_tconst_violations:
      (bool_opt "package_allow_reifiable_tconst_violations" config)
    ?tco_package_allow_reified_generics_violations:
      (bool_opt "package_allow_reified_generics_violations" config)
    ?tco_package_allow_all_tconst_violations:
      (bool_opt "package_allow_all_tconst_violations" config)
    ?tco_package_allow_all_generics_violations:
      (bool_opt "package_allow_all_generics_violations" config)
    ?tco_package_allow_function_pointers_violations:
      (bool_opt "package_allow_function_pointers_violations" config)
    ?tco_package_exclude_patterns:
      (string_list_opt "package_exclude_patterns" config)
    ?tco_extended_reasons:(reasons_config_opt config)
    ?tco_disable_physical_equality:(bool_opt "disable_physical_equality" config)
    ?re_no_cache:(bool_opt "re_no_cache" config)
    ?hh_distc_should_disable_trace_store:
      (bool_opt "hh_distc_should_disable_trace_store" config)
    ?hh_distc_exponential_backoff_num_retries:
      (int_opt "hh_distc_exponential_backoff_num_retries" config)
    ?tco_enable_abstract_method_optional_parameters:
      (bool_opt "enable_abstract_method_optional_parameters" config)
    ?hack_warnings:
      (bool_opt "hack_warnings" config
      |> Option.map ~f:(function
             | true -> GlobalOptions.All_except []
             | false -> GlobalOptions.NNone))
    ?recursive_case_types:(bool_opt "recursive_case_types" config)
    ?class_sub_classname:(bool_opt "class_sub_classname" config)
    ?class_class_type:(bool_opt "class_class_type" config)
    ?needs_concrete:(bool_opt "needs_concrete" config)
    ?needs_concrete_override_check:
      (bool_opt "needs_concrete_override_check" config)
    ?allow_class_string_cast:(bool_opt "allow_class_string_cast" config)
    ?class_pointer_ban_classname_new:
      (int_opt "class_pointer_ban_classname_new" config)
    ?class_pointer_ban_classname_type_structure:
      (int_opt "class_pointer_ban_classname_type_structure" config)
    ?class_pointer_ban_classname_static_meth:
      (int_opt "class_pointer_ban_classname_static_meth" config)
    ?class_pointer_ban_classname_class_const:
      (int_opt "class_pointer_ban_classname_class_const" config)
    ?class_pointer_ban_class_array_key:
      (bool_opt "class_pointer_ban_class_array_key" config)
    ?tco_poly_function_pointers:(bool_opt "poly_function_pointers" config)
    ?tco_check_packages:(bool_opt "check_packages" config)
    ?tco_package_config_disable_transitivity_check:
      (bool_opt "package_config_disable_transitivity_check" config)
    ?tco_allow_require_package_on_interface_methods:
      (bool_opt "allow_require_package_on_interface_methods" config)
    options

(** Load local config from the following sources:

  * /etc/hh.conf
  * overridden by Knobs
  * overridden by ExperimentsConfig
  * overridden by command line overrides

If not silent, then prints what it's doing to stderr. *)
let load_local_config
    (config : Config_file_common.t)
    (version : Config_file_version.version)
    ~(command_line_overrides : Config_file_common.t)
    ~silent
    ~from : ServerLocalConfig.t =
  let current_rolled_out_flag_idx =
    int_ "current_saved_state_rollout_flag_index" ~default:0 config
  in
  let deactivate_saved_state_rollout =
    bool_ "deactivate_saved_state_rollout" ~default:false config
  in
  ServerLocalConfigLoad.load
    ~silent
    ~current_version:version
    ~current_rolled_out_flag_idx
    ~deactivate_saved_state_rollout
    ~from
    ~overrides:command_line_overrides

let load ~silent ~from ~(cli_config_overrides : (string * string) list) :
    t * ServerLocalConfig.t =
  let command_line_overrides = Config_file.of_list cli_config_overrides in
  let hhconfig_abs_path = Relative_path.to_absolute repo_config_path in
  let hhconfig = Config_file.parse_hhconfig hhconfig_abs_path in
  let config =
    Config_file.apply_overrides
      ~config:hhconfig
      ~overrides:command_line_overrides
      ~log_reason:None
  in
  process_untrusted_mode config;
  let version =
    Config_file.parse_version (Config_file.Getters.string_opt "version" config)
  in
  let local_config =
    load_local_config config version ~command_line_overrides ~silent ~from
  in
  Option.iter
    ~f:Config_file_common.set_pkgconfig_path
    (string_opt "packages_config_path" config);
  let pkgs_config_abs_path =
    Relative_path.(
      to_absolute
      @@ from_root ~suffix:(Config_file_common.get_pkgconfig_path ()))
  in
  let config_hash =
    Config_file_common.hash
      hhconfig
      ~hhconfig_contents:(Config_file.cat_hhconfig_file hhconfig_abs_path)
      ~pkgconfig_contents:(Config_file.cat_packages_file pkgs_config_abs_path)
  in
  let global_opts_without_package_info =
    let tco_custom_error_config = CustomErrorConfig.load_and_parse () in
    let local_config_opts =
      GlobalOptions.set
        ~po:
          GlobalOptions.
            {
              default.po with
              ParserOptions.allow_unstable_features =
                local_config.ServerLocalConfig.allow_unstable_features;
            }
        ?so_naming_sqlite_path:local_config.naming_sqlite_path
        ?tco_log_large_fanouts_threshold:
          local_config.log_large_fanouts_threshold
        ~tco_remote_old_decls_no_limit:
          local_config.ServerLocalConfig.remote_old_decls_no_limit
        ~tco_fetch_remote_old_decls:
          local_config.ServerLocalConfig.fetch_remote_old_decls
        ~tco_populate_member_heaps:
          local_config.ServerLocalConfig.populate_member_heaps
        ~tco_skip_hierarchy_checks:
          local_config.ServerLocalConfig.skip_hierarchy_checks
        ~tco_skip_tast_checks:local_config.ServerLocalConfig.skip_tast_checks
        ~tco_saved_state:local_config.ServerLocalConfig.saved_state
        ~tco_log_inference_constraints:
          local_config.ServerLocalConfig.log_inference_constraints
        ~tco_global_access_check_enabled:
          local_config.ServerLocalConfig.enable_global_access_check
        ~dump_tast_hashes:local_config.dump_tast_hashes
        ~dump_tasts:local_config.dump_tasts
        ~tco_custom_error_config
        ~tco_sticky_quarantine:local_config.lsp_sticky_quarantine
        ~tco_lsp_invalidation:local_config.lsp_invalidation
        ~tco_autocomplete_sort_text:local_config.autocomplete_sort_text
        ~hack_warnings:
          (if local_config.hack_warnings then
            GlobalOptions.All_except
              (int_list_opt "disabled_warnings" config
              |> Option.value ~default:[])
          else
            GlobalOptions.NNone)
        ~warnings_default_all:local_config.warnings_default_all
        ~warnings_in_sandcastle:local_config.warnings_in_sandcastle
        ~warnings_generated_files:
          (string_list "warnings_generated_files" ~default:[] config)
        ~hh_distc_exponential_backoff_num_retries:
          local_config.hh_distc_exponential_backoff_num_retries
        ~fanout_strip_class_location:local_config.fanout_strip_class_location
        GlobalOptions.default
    in
    load_config config local_config_opts
  in
  Hh_logger.log "Parsing and loading packages config at %s" pkgs_config_abs_path;
  let package_info =
    PackageConfig.load_and_parse
      ~strict:local_config.ServerLocalConfig.package_config_strict_validation
      ~disable_transitivity_check:
        global_opts_without_package_info
          .tco_package_config_disable_transitivity_check
      ~pkgs_config_abs_path
  in
  PackageInfo.log_package_info package_info;
  let global_opts =
    GlobalOptions.set
      ~po:
        GlobalOptions.{ global_opts_without_package_info.po with package_info }
      global_opts_without_package_info
  in

  Diagnostics.allowed_fixme_codes_strict :=
    GlobalOptions.allowed_fixme_codes_strict global_opts;
  Diagnostics.code_agnostic_fixme :=
    GlobalOptions.code_agnostic_fixme global_opts;
  ( {
      version;
      load_script_timeout =
        (* Since we use the unix alarm() for our timeouts, a timeout value of 0 means
         * to wait indefinitely *)
        int_ "load_script_timeout" ~default:0 config;
      gc_control = make_gc_control config;
      sharedmem_config = make_sharedmem_config config local_config;
      tc_options = global_opts;
      parser_options = global_opts.GlobalOptions.po;
      glean_options = global_opts;
      symbol_write_options = global_opts;
      formatter_override =
        Option.map
          (Config_file.Getters.string_opt "formatter_override" config)
          ~f:maybe_relative_path;
      config_hash = Some config_hash;
      ignored_paths = process_ignored_paths config;
      extra_paths = process_extra_paths config;
      warn_on_non_opt_build =
        bool_ "warn_on_non_opt_build" ~default:false config;
      ide_fall_back_to_full_index =
        bool_ "ide_fall_back_to_full_index" ~default:true config;
      naming_table_compression_level =
        int_ "naming_table_compression_level" ~default:6 config;
      naming_table_compression_threads =
        int_ "naming_table_compression_threads" ~default:1 config;
      warnings_generated_files =
        string_list "warnings_generated_files" ~default:[] config;
    },
    local_config )

(* useful in testing code *)
let default_config =
  {
    version = Config_file.Opaque_version None;
    load_script_timeout = 0;
    gc_control = GlobalConfig.gc_control;
    sharedmem_config = SharedMem.default_config;
    tc_options = TypecheckerOptions.default;
    glean_options = GleanOptions.default;
    symbol_write_options = SymbolWriteOptions.default;
    parser_options = ParserOptions.default;
    formatter_override = None;
    config_hash = None;
    ignored_paths = [];
    extra_paths = [];
    warn_on_non_opt_build = false;
    ide_fall_back_to_full_index = false;
    naming_table_compression_level = 6;
    naming_table_compression_threads = 1;
    warnings_generated_files = [];
  }

let set_parser_options config popt = { config with parser_options = popt }

let set_tc_options config tcopt = { config with tc_options = tcopt }

let set_glean_options config gleanopt = { config with glean_options = gleanopt }

let set_symbol_write_options config swriteopt =
  { config with symbol_write_options = swriteopt }

let gc_control config = config.gc_control

let sharedmem_config config = config.sharedmem_config

let typechecker_options config = config.tc_options

let parser_options config = config.parser_options

let glean_options config = config.glean_options

let symbol_write_options config = config.symbol_write_options

let formatter_override config = config.formatter_override

let config_hash config = config.config_hash

let ignored_paths config = config.ignored_paths |> List.map ~f:Str.regexp

let extra_paths config = config.extra_paths

let version config = config.version

let warn_on_non_opt_build config = config.warn_on_non_opt_build

let ide_fall_back_to_full_index config = config.ide_fall_back_to_full_index

let warnings_generated_files config = config.warnings_generated_files

let update_config_with_ai_options config local_config ai_options =
  match ai_options with
  | None -> (config, local_config)
  | Some ai ->
    ( {
        config with
        sharedmem_config =
          Ai_options.modify_shared_mem ai config.sharedmem_config;
      },
      {
        local_config with
        watchman =
          {
            local_config.watchman with
            Watchman.enabled = false;
            subscribe = false;
          };
        edenfs_file_watcher =
          {
            local_config.edenfs_file_watcher with
            EdenfsFileWatcher.enabled = false;
          };
        interrupt_on_file_changes = false;
        interrupt_on_client = false;
        trace_parsing = false;
      } )
