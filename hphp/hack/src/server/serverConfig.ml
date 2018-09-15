(**
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

open Hh_core
open Config_file.Getters
open Reordered_argument_collections

type t = {
  version : string option;

  load_script_timeout : int; (* in seconds *)

  (* Configures only the workers. Workers can have more relaxed GC configs as
   * they are short-lived processes *)
  gc_control       : Gc.control;
  sharedmem_config : SharedMem.config;
  tc_options       : TypecheckerOptions.t;
  parser_options   : ParserOptions.t;
  formatter_override : Path.t option;
  config_hash      : string option;
  (* A list of regexps for paths to ignore *)
  ignored_paths    : string list;
  (* A list of extra paths to search for declarations *)
  extra_paths      : Path.t list;
  (* A list of regexps for paths to ignore for typechecking coroutines *)
  coroutine_whitelist_paths : string list;

  (* what version of the typechecker is targetted *)
  forward_compatibility_level : ForwardCompatibilityLevel.t;
}

let filename = Relative_path.from_root Config_file.file_path_relative_to_repo_root

let is_compatible c1 c2 =
  (* This comparison can eventually be made more complex; we may not always
   * need to restart hh_server, e.g. changing the path to the load script
   * is immaterial*)
  c1 = c2

let make_gc_control config =
  let {Gc.minor_heap_size; space_overhead; _} = GlobalConfig.gc_control in
  let minor_heap_size =
    int_ "gc_minor_heap_size" ~default:minor_heap_size config in
  let space_overhead =
    int_ "gc_space_overhead" ~default:space_overhead config in
  { GlobalConfig.gc_control with Gc.minor_heap_size; space_overhead; }

let make_sharedmem_config config options local_config =
  let { SharedMem.
    global_size;
    heap_size;
    shm_min_avail;
    _;
  } = GlobalConfig.default_sharedmem_config in
  let shm_dirs = local_config.ServerLocalConfig.shm_dirs in

  let global_size = int_ "sharedmem_global_size" ~default:global_size config in
  let heap_size = int_ "sharedmem_heap_size" ~default:heap_size config in
  let dep_table_pow = int_ "sharedmem_dep_table_pow" ~default:17 config in
  let hash_table_pow = int_ "sharedmem_hash_table_pow" ~default:18 config in
  let log_level = int_ "sharedmem_log_level" ~default:0 config in
  let shm_dirs = string_list
    ~delim:(Str.regexp ",")
    "sharedmem_dirs"
    ~default:shm_dirs
    config in
  let shm_min_avail =
    int_ "sharedmem_minimum_available" ~default:shm_min_avail config in

  let global_size, heap_size, dep_table_pow, hash_table_pow =
    match ServerArgs.ai_mode options with
    | None -> global_size, heap_size, dep_table_pow, hash_table_pow
    | Some ai_options ->
      Ai.modify_shared_mem_sizes
        global_size
        heap_size
        dep_table_pow
        hash_table_pow
        ai_options in

  { SharedMem.
      global_size;
      heap_size;
      dep_table_pow;
      hash_table_pow;
      log_level;
      shm_dirs;
      shm_min_avail;
  }

let config_list_regexp = (Str.regexp "[, \t]+")

let config_user_attributes config =
  match SMap.get config "user_attributes" with
    | None -> None
    | Some s ->
      let custom_attrs = Str.split config_list_regexp s in
      Some (List.fold_left custom_attrs ~f:SSet.add ~init:SSet.empty)

let process_experimental sl =
  match List.map sl String.lowercase_ascii with
    | ["false"] -> SSet.empty
    | ["true"] -> TypecheckerOptions.experimental_all
    | features -> List.fold_left features ~f:SSet.add ~init:SSet.empty

let config_experimental_tc_features config =
  match SMap.get config "enable_experimental_tc_features" with
    | None -> SSet.empty
    | Some s ->
      let sl = Str.split config_list_regexp s in
      process_experimental sl

let process_migration_flags sl =
  match sl with
  | ["false"] -> SSet.empty
  | ["true"] -> TypecheckerOptions.migration_flags_all
  | flags ->
    begin
      List.iter flags ~f:(fun s ->
        if not (SSet.mem TypecheckerOptions.migration_flags_all s)
        then failwith ("invalid migration flag: " ^ s));
      List.fold_left flags ~f:SSet.add ~init:SSet.empty
    end

let config_tc_migration_flags config =
  SMap.get config "enable_tc_migration_flags"
  |> Option.value_map ~f:(Str.split config_list_regexp) ~default:[]
  |> List.map ~f:String.lowercase_ascii
  |> process_migration_flags


let convert_paths str =
  let json = Hh_json.json_of_string ~strict:true str in
  let l = Hh_json.get_array_exn json in
  List.filter_map ~f:(fun s ->
      match s with
      | Hh_json.JSON_String path -> Some path
      | _ -> None
    ) l

let process_ignored_paths config =
  SMap.get config "ignored_paths"
  |> Option.value_map ~f:convert_paths ~default:[]

let process_forward_compatibility_level config =
  SMap.get config "forward_compatibility_level"
  |> Option.value_map ~f:ForwardCompatibilityLevel.from_string ~default:ForwardCompatibilityLevel.HEAD

let maybe_relative_path fn =
  (* Note: this is not the same as calling realpath; the cwd is not
   * necessarily the same as hh_server's root!!! *)
  Path.make begin
    if Filename.is_relative fn
    then Relative_path.(to_absolute (from_root fn))
    else fn
  end

let process_extra_paths config =
  match SMap.get config "extra_paths" with
  | Some s -> Str.split config_list_regexp s |> List.map ~f:maybe_relative_path
  | _ -> []

let process_coroutine_whitelist_paths config =
  SMap.get config "coroutine_whitelist_paths"
  |> Option.value_map ~f:convert_paths ~default:[]

let process_untrusted_mode config =
  match SMap.get config "untrusted_mode" with
  | Some s ->
    if bool_of_string s
    then
      let blacklist = [
        (* out of tree file access*)
        "extra_paths";
        (* potential resource abuse *)
        "language_feature_logging";
      ] in
      let prefix_blacklist = [
        (* potential resource abuse *)
        "gc_";
        "sharedmem_";
      ] in
      let invalid_keys = SMap.filter ~f:(fun ck _ ->
        let ck = String.lowercase_ascii ck in
        let exact_match = List.find ~f:(fun bli -> bli = ck) blacklist in
        let prefix_match = List.find
          ~f:(fun blp -> String_utils.string_starts_with ck blp)
          prefix_blacklist
        in
        match exact_match, prefix_match with
          | (None, None) -> false
          | _ -> true
      ) config |> SMap.keys in
      if not (List.is_empty invalid_keys)
      then failwith ("option not permitted in untrusted_mode: "^(String.concat ", " invalid_keys))
    else failwith "untrusted_mode can only be enabled, not disabled"
  | _ -> ()

let extract_auto_namespace_element ns_map element =
  match element with
    | (source, Hh_json.JSON_String target) ->
       (source, target)::ns_map
    | _ -> ns_map (* This means the JSON we received is incorrect *)

let convert_auto_namespace_to_map map =
  let json = Hh_json.json_of_string ~strict:true map in
  let pairs = Hh_json.get_object_exn json in
  (* We do a fold instead of a map to filter
   * out the incorrect entrie as we look at each item *)
  List.fold_left ~init:[] ~f:extract_auto_namespace_element pairs

let prepare_auto_namespace_map config =
  Option.value_map
    (SMap.get config "auto_namespace_map")
    ~default:[]
    ~f:convert_auto_namespace_to_map

let prepare_ignored_fixme_codes config =
  SMap.get config "ignored_fixme_codes"
  |> Option.value_map ~f:(Str.split config_list_regexp) ~default:[]
  |> List.map ~f:int_of_string
  |> List.fold_right ~init:Errors.default_ignored_fixme_codes ~f:ISet.add

let load config_filename options =
  let config_hash, config = Config_file.parse (Relative_path.to_absolute config_filename) in
  process_untrusted_mode config;
  let local_config = ServerLocalConfig.load ~silent:false in
  let version = SMap.get config "version" in
  let ignored_paths = process_ignored_paths config in
  let extra_paths = process_extra_paths config in
  let coroutine_whitelist_paths = process_coroutine_whitelist_paths config in
  (* Since we use the unix alarm() for our timeouts, a timeout value of 0 means
   * to wait indefinitely *)
  let load_script_timeout = int_ "load_script_timeout" ~default:0 config in
  let formatter_override =
    Option.map (SMap.get config "formatter_override") maybe_relative_path in
  let forward_compat_level = process_forward_compatibility_level config in
  let global_opts = GlobalOptions.make
    (bool_ "assume_php" ~default:true config)
    (bool_ "safe_array" ~default:true config)
    (bool_ "safe_vector_array" ~default:true config)
    (bool_ "deregister_php_stdlib" ~default:false config)
    (* Although it's locally configured, the use_full_fidelity_parser flag needs
     * to end up in the parser options to reach all consumers of it.
     *)
    local_config.ServerLocalConfig.use_full_fidelity_parser
    (config_user_attributes config)
    (config_experimental_tc_features config)
    (config_tc_migration_flags config)
    false (* typechecker dynamic_view option to set Tany as Tdynamic, off by default *)
    (bool_ "disallow_array_as_tuple" ~default:false config)
    (prepare_auto_namespace_map config)
    (bool_ "disallow_ambiguous_lambda" ~default:false config)
    (bool_ "disallow_array_typehint" ~default:false config)
    (bool_ "disallow_array_literal" ~default:false config)
    (bool_ "untyped_nonstrict_lambda_parameters" ~default:false config)
    (bool_ "disallow_return_by_ref" ~default:false config)
    (bool_ "disallow_array_cell_pass_by_ref" ~default:false config)
    (bool_ "language_feature_logging" ~default:false config)
    (bool_ "unsafe_rx" ~default:true config)
    (bool_ "disable_primitive_refinement" ~default:false config)
    (bool_ "disallow_implicit_returns_in_non_void_functions"
       ~default:false config)
    (bool_ "disallow_unset_on_varray" ~default:false config)
    (bool_ "disallow_scrutinee_case_value_type_mismatch"
       ~default:true config)
    (prepare_ignored_fixme_codes config)
    forward_compat_level
  in
  Errors.ignored_fixme_codes :=
    (GlobalOptions.ignored_fixme_codes global_opts);
  {
    version = version;
    load_script_timeout = load_script_timeout;
    gc_control = make_gc_control config;
    sharedmem_config = make_sharedmem_config config options local_config;
    tc_options = global_opts;
    parser_options = global_opts;
    formatter_override = formatter_override;
    config_hash = config_hash;
    ignored_paths = ignored_paths;
    extra_paths = extra_paths;
    coroutine_whitelist_paths = coroutine_whitelist_paths;
    forward_compatibility_level = forward_compat_level;
  }, local_config

(* useful in testing code *)
let default_config = {
  version = None;
  load_script_timeout = 0;
  gc_control = GlobalConfig.gc_control;
  sharedmem_config = GlobalConfig.default_sharedmem_config;
  tc_options = TypecheckerOptions.default;
  parser_options = ParserOptions.default;
  formatter_override = None;
  config_hash = None;
  ignored_paths = [];
  extra_paths = [];
  coroutine_whitelist_paths = [];
  forward_compatibility_level = ForwardCompatibilityLevel.HEAD;
}

let set_parser_options config popt = { config with parser_options = popt }
let set_tc_options config tcopt = { config with tc_options = tcopt }
let gc_control config = config.gc_control
let sharedmem_config config = config.sharedmem_config
let typechecker_options config = config.tc_options
let parser_options config = config.parser_options
let formatter_override config = config.formatter_override
let config_hash config = config.config_hash
let ignored_paths config = config.ignored_paths
let extra_paths config = config.extra_paths
let coroutine_whitelist_paths config = config.coroutine_whitelist_paths
let version config = config.version
let forward_compatibility_level config = config.forward_compatibility_level
