(**
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 *
 *)

open Core
open Config_file.Getters
open Reordered_argument_collections

type t = {
  load_script      : Path.t option;
  load_script_timeout : int; (* in seconds *)

  load_mini_script : Path.t option;

  (* Configures only the workers. Workers can have more relaxed GC configs as
   * they are short-lived processes *)
  gc_control       : Gc.control;
  sharedmem_config : SharedMem.config;
  tc_options       : TypecheckerOptions.t;
  parser_options   : ParserOptions.t;
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
  let process_experimental sl =
    match List.map sl String.lowercase with
    | ["false"] -> SSet.empty
    | ["true"] -> TypecheckerOptions.experimental_all
    | features ->
      begin
        List.iter features ~f:(fun s ->
          if not (SSet.mem TypecheckerOptions.experimental_all s)
          then failwith ("invalid experimental feature " ^ s));
        List.fold_left features ~f:SSet.add ~init:SSet.empty
      end in
  let tcopts = { TypecheckerOptions.
    tco_assume_php = bool_ "assume_php" ~default:true config;
    tco_unsafe_xhp = bool_ "unsafe_xhp" ~default:false config;
    tco_user_attrs = config_user_attributes config;
    tco_experimental_features =
      process_experimental (string_list
        ~delim:(Str.regexp ",")
        "enable_experimental_tc_features"
        ~default:[]
        config);
  } in
  let popts = { ParserOptions.
    po_auto_namespace_map = prepare_auto_namespace_map config;
  } in
  {
    load_script = load_script;
    load_script_timeout = load_script_timeout;
    load_mini_script = load_mini_script;
    gc_control = make_gc_control config;
    sharedmem_config = make_sharedmem_config config options local_config;
    tc_options = tcopts;
    parser_options = popts;
  }, local_config

(* useful in testing code *)
let default_config = {
  load_script = None;
  load_script_timeout = 0;
  load_mini_script = None;
  gc_control = GlobalConfig.gc_control;
  sharedmem_config = GlobalConfig.default_sharedmem_config;
  tc_options = TypecheckerOptions.default;
  parser_options = ParserOptions.default;
}

let load_script config = config.load_script
let load_script_timeout config = config.load_script_timeout
let load_mini_script config = config.load_mini_script
let gc_control config = config.gc_control
let sharedmem_config config = config.sharedmem_config
let typechecker_options config = config.tc_options
let parser_options config = config.parser_options
