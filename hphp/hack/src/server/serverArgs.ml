(**
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Core_kernel
include ServerArgs_sig.Types

(*****************************************************************************)
(* The options from the command line *)
(*****************************************************************************)

type options = {
  ai_mode: Ai_options.t option;
  check_mode: bool;
  config: (string * string) list;
  dynamic_view: bool;
  file_info_on_disk: bool;
  from: string;
  gen_saved_ignore_type_errors: bool;
  ignore_hh_version: bool;
  json_mode: bool;
  load_state_canary: bool;
  log_inference_constraints: bool;
  max_procs: int;
  no_load: bool;
  prechecked: bool option;
  profile_log: bool;
  root: Path.t;
  save_filename: string option;
  should_detach: bool;
  waiting_client: Unix.file_descr option;
  watchman_debug_logging: bool;
  with_mini_state: mini_state_target option;
}

(*****************************************************************************)
(* Usage code *)
(*****************************************************************************)
let usage = Printf.sprintf "Usage: %s [WWW DIRECTORY]\n" Sys.argv.(0)

(*****************************************************************************)
(* Options *)
(*****************************************************************************)

module Messages = struct
  let ai = " run ai with options"
  let check = " check and exit"
  let config = " override arbitrary value from hh.conf (format: <key>=<value>)"
  let daemon = " detach process"
  let dynamic_view = " start with dynamic view for IDE files on by default."
  let file_info_on_disk = " [experimental] a saved state option to store file info" ^
                          " (the naming table) in SQLite. Only has meaning in --saved-state mode."
  let from = " so we know who's invoking - e.g. nuclide, vim, emacs, vscode"
  let from_vim = " DEPRECATED"
  let from_emacs = " DEPRECATED"
  let from_hhclient = " DEPRECATED"
  let gen_saved_ignore_type_errors = " generate a saved state even if there are type errors."
  let ignore_hh_version = " ignore hh_version check when loading saved states"
  let json = " output errors in json format (arc lint mode)"
  let load_state_canary = " Look up a saved state using the hg commit" ^
                          " hash instead of the SVN rev."
  let log_inference_constraints = " (for hh debugging purpose only) log type" ^
    " inference constraints into external logger (e.g. Scuba)"
  let max_procs = " max numbers of workers"
  let mini_state_json_descr =
    "Either\n" ^
    "   { \"data_dump\" : <mini_state_target json> }\n" ^
    "or\n" ^
    "   { \"from_file\" : <path to file containing mini_state_target json }\n" ^
    "where mini_state_target json looks liks:\n" ^
    "   {\n" ^
    "      \"state\" : <saved state filename>\n" ^
    "      \"corresponding_base_revision\" : <SVN rev #>\n" ^
    "      \"deptable\" : <dependency table filename>\n" ^
    "      \"changes\" : [array of files changed since that saved state]\n" ^
    "   }"
  let no_load = " don't load from a saved state"
  let prechecked = " override value of \"prechecked_files\" flag from hh.conf"
  let profile_log = " enable profile logging"
  let save_mini = " save mini server state to file"
  let waiting_client= " send message to fd/handle when server has begun" ^
                      " starting and again when it's done starting"
  let watchman_debug_logging =
    " Enable debug logging on Watchman client. This is very noisy"

  let with_mini_state = " init with the given saved state instead of getting" ^
                        " it by running load mini state script." ^
                        " Expects a JSON blob specified as" ^
                        mini_state_json_descr
end

let print_json_version () =
  let open Hh_json in
  let json = JSON_Object [
    "commit", JSON_String Build_id.build_revision;
    "commit_time", int_ Build_id.build_commit_time;
    "api_version", int_ Build_id.build_api_version;
  ] in
  print_endline @@ json_to_string json

(*****************************************************************************)
(* The main entry point *)
(*****************************************************************************)

let parse_mini_state_json (json, _keytrace) =
  let prechecked_changes = Option.value ~default:[]
    (Hh_json.(get_field_opt (Access.get_array "prechecked_changes")) json) in
  let json = Hh_json.Access.return json in
  let open Hh_json.Access in
  json >>= get_string "state" >>= fun (state, _state_keytrace) ->
  json >>= get_string "corresponding_base_revision"
    >>= fun (for_base_rev, _for_base_rev_keytrace) ->
  json >>= get_string "deptable" >>= fun (deptable, _deptable_keytrace) ->
  json >>= get_array "changes" >>= fun (changes, _) ->
    let array_to_path_list = List.map
      ~f:(fun file -> Hh_json.get_string_exn file |> Relative_path.from_root)
    in
    let prechecked_changes = array_to_path_list prechecked_changes in
    let changes = array_to_path_list changes in
    return (Mini_state_target_info {
      saved_state_fn = state;
      corresponding_base_revision = for_base_rev;
      deptable_fn = deptable;
      prechecked_changes;
      changes;
    })

let verify_with_mini_state v = match !v with
  | None -> None
  | Some blob ->
    let json = Hh_json.json_of_string blob in
    let json = Hh_json.Access.return json in
    let open Hh_json.Access in
    let data_dump_parse_result =
      json
        >>= get_obj "data_dump"
        >>= parse_mini_state_json
    in
    let from_file_parse_result =
      json
        >>= get_string "from_file"
        >>= fun (filename, _filename_keytrace) ->
        let contents = Sys_utils.cat filename in
        let json = Hh_json.json_of_string contents in
        (Hh_json.Access.return json)
          >>= parse_mini_state_json
    in
    match
      (Result.ok_fst data_dump_parse_result),
      (Result.ok_fst from_file_parse_result) with
    | (`Fst (parsed_data_dump, _)), (`Fst (_parsed_from_file, _)) ->
      Hh_logger.log "Warning - %s"
        ("Parsed mini state target from both JSON blob data dump" ^
        " and from contents of file.");
      Hh_logger.log "Preferring data dump result";
      Some parsed_data_dump
    | (`Fst (parsed_data_dump, _)), (`Snd _) ->
      Some parsed_data_dump
    | (`Snd _), (`Fst (parsed_from_file, _)) ->
      Some parsed_from_file
    | (`Snd data_dump_failure), (`Snd from_file_failure) ->
      Hh_logger.log "parsing optional arg with_mini_state failed:\n%s\n%s"
        (Printf.sprintf "  data_dump failure:%s"
              (access_failure_to_string data_dump_failure))
        (Printf.sprintf "  from_file failure:%s"
              (access_failure_to_string from_file_failure));
      Hh_logger.log "See input: %s" blob;
      raise (Arg.Bad "--with-mini-state")

let parse_options () =
  let ai_mode = ref None in
  let check_mode = ref false in
  let config = ref [] in
  let dynamic_view = ref false in
  let file_info_on_disk = ref false in
  let from = ref "" in
  let from_emacs = ref false in
  let from_hhclient = ref false in
  let from_vim = ref false in
  let gen_saved_ignore_type_errors = ref false in
  let ignore_hh = ref false in
  let json_mode = ref false in
  let load_state_canary = ref false in
  let log_inference_constraints = ref false in
  let max_procs = ref GlobalConfig.nbr_procs in
  let no_load = ref false in
  let prechecked = ref None in
  let profile_log = ref false in
  let root = ref "" in
  let save = ref None in
  let should_detach = ref false in
  let version = ref false in
  let waiting_client= ref None in
  let watchman_debug_logging = ref false in
  let with_mini_state = ref None in

  let set_ai = fun s -> ai_mode := Some (Ai_options.prepare ~server:true s) in
  let set_max_procs = fun s -> max_procs := min !max_procs s in
  let set_save_mini = fun s -> save := Some s in
  let set_wait = fun fd -> waiting_client := Some (Handle.wrap_handle fd) in
  let set_with_mini_state = fun s -> with_mini_state := Some s in
  let set_from = fun s -> from := s in

  let options = [
      "--ai", Arg.String set_ai, Messages.ai;
      "--check", Arg.Set check_mode, Messages.check;
      "--config",
        Arg.String (fun s -> config := (String_utils.split2_exn '=' s) :: !config),
        Messages.config;
      "--daemon", Arg.Set should_detach, Messages.daemon;
      "--dynamic-view", Arg.Set dynamic_view, Messages.dynamic_view;
      "--file-info-on-disk", Arg.Set file_info_on_disk, Messages.file_info_on_disk;
      "--from-emacs", Arg.Set from_emacs, Messages.from_emacs;
      "--from-hhclient", Arg.Set from_hhclient, Messages.from_hhclient;
      "--from-vim", Arg.Set from_vim, Messages.from_vim;
      "--from", Arg.String set_from, Messages.from;
      "--gen-saved-ignore-type-errors",
        Arg.Set gen_saved_ignore_type_errors,
        Messages.gen_saved_ignore_type_errors;
      "--ignore-hh-version", Arg.Set ignore_hh, Messages.ignore_hh_version;
      "--json", Arg.Set json_mode, Messages.json;
      "--load-state-canary", Arg.Set load_state_canary, Messages.load_state_canary;
      "--log-inference-constraints", Arg.Set log_inference_constraints, Messages.log_inference_constraints;
      "--max-procs", Arg.Int set_max_procs, Messages.max_procs;
      "--no-load", Arg.Set no_load, Messages.no_load;
      "--no-prechecked", Arg.Unit (fun () -> prechecked := Some false), Messages.prechecked;
      "--prechecked", Arg.Unit (fun () -> prechecked := Some true), Messages.prechecked;
      "--profile-log", Arg.Set profile_log, Messages.profile_log;
      "--save-mini", Arg.String set_save_mini, Messages.save_mini;
      "--version", Arg.Set version, "";
      "--waiting-client", Arg.Int set_wait, Messages.waiting_client;
      "--watchman-debug-logging", Arg.Set watchman_debug_logging, Messages.watchman_debug_logging;
      "--with-mini-state", Arg.String set_with_mini_state, Messages.with_mini_state;
      "-d", Arg.Set should_detach, Messages.daemon;
      "-s", Arg.String set_save_mini, Messages.save_mini;
    ] in
  let options = Arg.align options in
  Arg.parse options (fun s -> root := s) usage;
  if !version then begin
    if !json_mode then print_json_version ()
    else print_endline Build_id.build_id_ohai;
    exit 0
  end;
  (* --json and --save both imply check *)
  let check_mode = !check_mode || !json_mode || !save <> None; in
  if check_mode && !waiting_client <> None then begin
    Printf.eprintf "--check is incompatible with wait modes!\n";
    Exit_status.(exit Input_error)
  end;
  let with_mini_state = verify_with_mini_state with_mini_state in
  (match !root with
  | "" ->
      Printf.eprintf "You must specify a root directory!\n";
      Exit_status.(exit Input_error)
  | _ -> ());
  let root_path = Path.make !root in
  ai_mode := (match !ai_mode with
    | Some (ai) ->
        (* ai may have json mode enabled internally, in which case,
         * it should not be disabled by hack if --json was not used *)
        if !json_mode then Some (Ai_options.set_json_mode ai true)
        else Some (ai)
    | None -> None);
  Wwwroot.assert_www_directory root_path;
  if (!gen_saved_ignore_type_errors) && not (Option.is_some (!save)) then begin
    Printf.eprintf "--ignore-type-errors is only valid when producing saved states\n%!";
    exit 1
  end;
  {
    ai_mode = !ai_mode;
    check_mode = check_mode;
    config = !config;
    dynamic_view = !dynamic_view;
    file_info_on_disk = !file_info_on_disk;
    from = !from;
    gen_saved_ignore_type_errors = !gen_saved_ignore_type_errors;
    ignore_hh_version = !ignore_hh;
    json_mode = !json_mode;
    load_state_canary = !load_state_canary;
    log_inference_constraints = !log_inference_constraints;
    max_procs = !max_procs;
    no_load = !no_load;
    prechecked = !prechecked;
    profile_log = !profile_log;
    root = root_path;
    save_filename = !save;
    should_detach = !should_detach;
    waiting_client = !waiting_client;
    watchman_debug_logging = !watchman_debug_logging;
    with_mini_state = with_mini_state;
  }

(* useful in testing code *)
let default_options ~root = {
  ai_mode = None;
  check_mode = false;
  config = [];
  dynamic_view = false;
  file_info_on_disk = false;
  from = "";
  gen_saved_ignore_type_errors = false;
  ignore_hh_version = false;
  json_mode = false;
  load_state_canary = false;
  log_inference_constraints = false;
  max_procs = GlobalConfig.nbr_procs;
  no_load = true;
  prechecked = None;
  profile_log = false;
  root = Path.make root;
  save_filename = None;
  should_detach = false;
  waiting_client = None;
  watchman_debug_logging = false;
  with_mini_state = None;
}

(*****************************************************************************)
(* Accessors *)
(*****************************************************************************)

let ai_mode options = options.ai_mode
let check_mode options = options.check_mode
let config options = options.config
let dynamic_view options = options.dynamic_view
let file_info_on_disk options = options.file_info_on_disk
let from options = options.from
let gen_saved_ignore_type_errors options = options.gen_saved_ignore_type_errors
let ignore_hh_version options = options.ignore_hh_version
let json_mode options = options.json_mode
let load_state_canary options = options.load_state_canary
let log_inference_constraints options = options.log_inference_constraints
let max_procs options = options.max_procs
let no_load options = options.no_load
let prechecked options = options.prechecked
let profile_log options = options.profile_log
let root options = options.root
let save_filename options = options.save_filename
let should_detach options = options.should_detach
let waiting_client options = options.waiting_client
let watchman_debug_logging options = options.watchman_debug_logging
let with_mini_state options = options.with_mini_state

(*****************************************************************************)
(* Setters *)
(*****************************************************************************)

let set_gen_saved_ignore_type_errors options ignore_type_errors = { options with
  gen_saved_ignore_type_errors = ignore_type_errors}
let set_no_load options is_no_load = {options with no_load = is_no_load}
let set_mini_state_target options target = match target with
  | None -> options
  | Some target ->
    { options with
      with_mini_state = Some (Informant_induced_mini_state_target target)
    }

(****************************************************************************)
(* Misc *)
(****************************************************************************)
let to_string
  {
    ai_mode;
    check_mode;
    config;
    dynamic_view;
    file_info_on_disk;
    from;
    gen_saved_ignore_type_errors;
    ignore_hh_version;
    json_mode;
    load_state_canary;
    log_inference_constraints;
    max_procs;
    no_load;
    prechecked;
    profile_log;
    root;
    save_filename;
    should_detach;
    waiting_client;
    watchman_debug_logging;
    with_mini_state;
  } =
    let ai_mode_str = match ai_mode with
      | None -> "<>"
      | Some _ -> "Some(...)" in
    let mini_state_str = match with_mini_state with
      | None -> "<>"
      | Some _ -> "MiniStateTarget(...)" in
    let waiting_client_str = match waiting_client with
      | None -> "<>"
      | Some _ -> "WaitingClient(...)" in
    let save_filename_str = match save_filename with
      | None -> "<>"
      | Some path -> path in
    let prechecked_str = match prechecked with
      | None -> "<>"
      | Some b -> string_of_bool b in
    let config_str = Printf.sprintf "[%s]"
      (String.concat ~sep:", " @@ List.map ~f:(fun (key, value) -> Printf.sprintf "%s=%s" key value) config)
    in
    ([
      "ServerArgs.options({";
        "ai_mode: "; ai_mode_str; ", ";
        "check_mode: "; string_of_bool check_mode; ", ";
        "config: "; config_str;
        "dynamic_view: "; string_of_bool dynamic_view; ", ";
        "file_info_on_disk: "; string_of_bool file_info_on_disk; ", ";
        "from: "; from; ", ";
        "gen_saved_ignore_type_errors: "; string_of_bool gen_saved_ignore_type_errors; ", ";
        "ignore_hh_version: "; string_of_bool ignore_hh_version; ", ";
        "json_mode: "; string_of_bool json_mode; ", ";
        "load_state_canary: "; string_of_bool load_state_canary; ", ";
        "log_inference_constraints: "; string_of_bool log_inference_constraints; ", ";
        "maxprocs: "; string_of_int max_procs; ", ";
        "no_load: "; string_of_bool no_load; ", ";
        "prechecked: "; prechecked_str;
        "profile_log: "; string_of_bool profile_log; ", ";
        "root: "; Path.to_string root; ", ";
        "save_filename: "; save_filename_str; ", ";
        "should_detach: "; string_of_bool should_detach; ", ";
        "waiting_client: "; waiting_client_str; ", ";
        "watchman_debug_logging: "; string_of_bool watchman_debug_logging; ", ";
        "with_mini_state: "; mini_state_str; ", ";
      "})"
    ] |> String.concat ~sep:"")
