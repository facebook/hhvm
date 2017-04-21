(**
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 *
 *)

(**
 * Parses and gathers information from the .hhconfig in the repo.
 *)

open Core
open Config_file.Getters
open Reordered_argument_collections

type t = {
  load_script      : Path.t option;
  load_script_timeout : int; (* in seconds *)

  load_mini_script : Path.t option;

  (** Script to call to prefetch a saved state. Expected invocation is:
    *   state_prefetcher_script <svn revision number>
    *
   * Which is expected to put a saved state into the correct place which the
   * above load_script will be able to use.
   *)
  state_prefetcher_script : Path.t option;

  (* Configures only the workers. Workers can have more relaxed GC configs as
   * they are short-lived processes *)
  gc_control       : Gc.control;
  sharedmem_config : SharedMem.config;
  tc_options       : TypecheckerOptions.t;
  parser_options   : ParserOptions.t;
  formatter_override : Path.t option;
}

let filename = Relative_path.concat Relative_path.Root ".hhconfig"

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
    | features ->
      begin
        List.iter features ~f:(fun s ->
          if not (SSet.mem TypecheckerOptions.experimental_all s)
          then failwith ("invalid experimental feature " ^ s));
        List.fold_left features ~f:SSet.add ~init:SSet.empty
      end

let config_experimental_tc_features config =
  match SMap.get config "enable_experimental_tc_features" with
    | None -> SSet.empty
    | Some s ->
      let sl = Str.split config_list_regexp s in
      process_experimental sl

let maybe_relative_path fn =
  (* Note: this is not the same as calling realpath; the cwd is not
   * necessarily the same as hh_server's root!!! *)
  Path.make begin
    if Filename.is_relative fn
    then Relative_path.(to_absolute (concat Root fn))
    else fn
  end

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

let load config_filename options =
  let config = Config_file.parse (Relative_path.to_absolute config_filename) in
  let local_config = ServerLocalConfig.load () in
  let load_script =
    Option.map (SMap.get config "load_script") maybe_relative_path in
  (* Since we use the unix alarm() for our timeouts, a timeout value of 0 means
   * to wait indefinitely *)
  let load_script_timeout = int_ "load_script_timeout" ~default:0 config in
  let load_mini_script =
    Option.map (SMap.get config "load_mini_script") maybe_relative_path in
  let state_prefetcher_script =
    Option.map (SMap.get config "state_prefetcher_script") maybe_relative_path in
  let formatter_override =
    Option.map (SMap.get config "formatter_override") maybe_relative_path in
  let global_opts = GlobalOptions.make
    (bool_ "assume_php" ~default:true config)
    (bool_ "safe_array" ~default:false config)
    (bool_ "safe_vector_array" ~default:false config)
    (config_user_attributes config)
    (config_experimental_tc_features config)
    (prepare_auto_namespace_map config)
  in
  {
    load_script = load_script;
    load_script_timeout = load_script_timeout;
    load_mini_script = load_mini_script;
    state_prefetcher_script = state_prefetcher_script;
    gc_control = make_gc_control config;
    sharedmem_config = make_sharedmem_config config options local_config;
    tc_options = global_opts;
    parser_options = global_opts;
    formatter_override = formatter_override;
  }, local_config

(* useful in testing code *)
let default_config = {
  load_script = None;
  load_script_timeout = 0;
  load_mini_script = None;
  state_prefetcher_script = None;
  gc_control = GlobalConfig.gc_control;
  sharedmem_config = GlobalConfig.default_sharedmem_config;
  tc_options = TypecheckerOptions.default;
  parser_options = ParserOptions.default;
  formatter_override = None;
}

let set_parser_options config popt = { config with parser_options = popt }
let set_tc_options config tcopt = { config with tc_options = tcopt }
let load_script config = config.load_script
let load_script_timeout config = config.load_script_timeout
let load_mini_script config = config.load_mini_script
let gc_control config = config.gc_control
let sharedmem_config config = config.sharedmem_config
let typechecker_options config = config.tc_options
let parser_options config = config.parser_options
let formatter_override config = config.formatter_override
