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
  from: string;
  gen_saved_ignore_type_errors: bool;
  ignore_hh_version: bool;
  saved_state_ignore_hhconfig: bool;
  json_mode: bool;
  load_state_canary: bool;
  log_inference_constraints: bool;
  max_procs: int option;
  no_load: bool;
  prechecked: bool option;
  profile_log: bool;
  replace_state_after_saving: bool;
  root: Path.t;
  save_filename: string option;
  save_naming_filename: string option;
  save_with_spec: save_state_spec_info option;
  should_detach: bool; (* AKA, daemon mode *)
  waiting_client: Unix.file_descr option;
  watchman_debug_logging: bool;
  with_saved_state: saved_state_target option;
  allow_non_opt_build : bool;
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
  let from = " so we know who's invoking - e.g. nuclide, vim, emacs, vscode"
  let from_vim = " DEPRECATED"
  let from_emacs = " DEPRECATED"
  let from_hhclient = " DEPRECATED"
  let gen_saved_ignore_type_errors = " generate a saved state even if there are type errors."
  let ignore_hh_version = " ignore hh_version check when loading saved states"
  let saved_state_ignore_hhconfig = " ignore hhconfig hash when loading saved states"
  let json = " output errors in json format (arc lint mode)"
  let load_state_canary = " Look up a saved state using the hg commit" ^
                          " hash instead of the SVN rev."
  let log_inference_constraints = " (for hh debugging purpose only) log type" ^
    " inference constraints into external logger (e.g. Scuba)"
  let max_procs = " max numbers of workers"
  let save_state_spec_json_descr =
{|A JSON specification of how and what to save, e.g.:
  {
    "files_to_check": [
      "/some/path/prefix1",
      {
        "from_prefix_incl": "/from/path/prefix1",
        "to_prefix_excl": "/to/path/prefix1"
      },
      {
        "from_prefix_incl": "/from/path/prefix2",
        "to_prefix_excl": "/to/path/prefix2"
      },
      {
        "from_prefix_incl": "/from/path/only"
      },
      {
        "to_prefix_excl": "/to/path/only"
      }
    ],
    "filename": "/some/dir/some_filename",
    "gen_with_errors": true
  }
|}
  let saved_state_json_descr =
    "Either\n" ^
    "   { \"data_dump\" : <saved_state_target json> }\n" ^
    "or\n" ^
    "   { \"from_file\" : <path to file containing saved_state_target json }\n" ^
    "where saved_state_target JSON looks like:\n" ^
    "   {\n" ^
    "      \"state\": <saved state filename>,\n" ^
    "      \"corresponding_base_revision\" : <SVN rev #>,\n" ^
    "      \"deptable\": <dependency table filename>,\n" ^
    "      \"changes\": [array of files changed since that saved state]\n" ^
    "   }"
  let no_load = " don't load from a saved state"
  let prechecked = " override value of \"prechecked_files\" flag from hh.conf"
  let profile_log = " enable profile logging"
  let replace_state_after_saving = " if combined with --save-mini, causes the saved state" ^
                        " to replace the program state; otherwise, the state files are not" ^
                        " used after being written to disk (default: false)"
  let save_state = " save server state to file"
  let save_naming = " save naming table to file"
  let save_with_spec = " save server state given a JSON spec" ^
                        " Expects a JSON string specified as" ^
                        save_state_spec_json_descr
  let waiting_client= " send message to fd/handle when server has begun" ^
                      " starting and again when it's done starting"
  let watchman_debug_logging =
    " Enable debug logging on Watchman client. This is very noisy"

  let with_saved_state = " init with the given saved state instead of fetching it." ^
                        " Expects a JSON string specified as" ^
                        saved_state_json_descr
end

let print_json_version () =
  print_endline @@ Hh_json.json_to_string Build_id.build_version_json

(*****************************************************************************)
(* The main entry point *)
(*****************************************************************************)
let get_path (key: string) json_obj : Relative_path.t option =
  let value = Hh_json.Access.get_string key json_obj in
  match value with
  | Ok ((value: string), _keytrace) -> Some (Relative_path.from_root value)
  | Error _ -> None

let get_spec (spec_json: Hh_json.json) : files_to_check_spec =
  try
    Prefix (Hh_json.get_string_exn spec_json |> Relative_path.from_root)
  with _ ->
    let from_prefix_incl = get_path "from_prefix_incl" (spec_json, []) in
    let to_prefix_excl = get_path "to_prefix_excl" (spec_json, []) in
    Range {
      from_prefix_incl;
      to_prefix_excl;
    }

let parse_save_state_json ((json: Hh_json.json), _keytrace) =
  let open Hh_json.Access in
  let files_to_check = Option.value
    ~default:[]
    (Hh_json.(get_field_opt (get_array "files_to_check")) json) in
  let files_to_check = List.map files_to_check ~f:get_spec in
  let json = return json in
  json >>= get_string "filename" >>= fun (filename, _filename_keytrace) ->
  json >>= get_bool "gen_with_errors" >>= fun (gen_with_errors, _gen_with_errors_keytrace) ->
  return {
    files_to_check;
    filename;
    gen_with_errors;
  }

let verify_save_with_spec (v: string option) : save_state_spec_info option =
  match v with
  | None -> None
  | Some blob ->
    let open Hh_json.Access in
    let json = Hh_json.json_of_string blob in
    let json = return json in
    let parsed_spec_result = json >>= parse_save_state_json in
    match parsed_spec_result with
    | Ok ((parsed_spec: save_state_spec_info), _keytrace) ->
      Hh_logger.log "Parsed save state spec, everything's good";
      Some parsed_spec
    | Error spec_failure ->
      let message = access_failure_to_string spec_failure in
      Hh_logger.log "parsing optional arg --save-with-spec failed:\n%s" message;
      Hh_logger.log "See input: %s" blob;
      raise (Arg.Bad ("--save-with-spec " ^ message))

let parse_saved_state_json (json, _keytrace) =
  let array_to_path_list = List.map
    ~f:(fun file -> Hh_json.get_string_exn file |> Relative_path.from_root) in
  let prechecked_changes = (Hh_json.(get_field_opt (Access.get_array "prechecked_changes")) json) in
  let prechecked_changes = Option.value ~default:[] prechecked_changes in
  let json = Hh_json.Access.return json in
  let open Hh_json.Access in
  json >>= get_string "state" >>= fun (state, _state_keytrace) ->
  json >>= get_string "corresponding_base_revision"
    >>= fun (for_base_rev, _for_base_rev_keytrace) ->
  json >>= get_string "deptable" >>= fun (deptable, _deptable_keytrace) ->
  json >>= get_array "changes" >>= fun (changes, _) ->
    let prechecked_changes = array_to_path_list prechecked_changes in
    let changes = array_to_path_list changes in
    return (Saved_state_target_info {
      saved_state_fn = state;
      corresponding_base_revision = for_base_rev;
      deptable_fn = deptable;
      prechecked_changes;
      changes;
    })

let verify_with_saved_state (v: string option) : saved_state_target option =
  match v with
  | None -> None
  | Some blob ->
    let json = Hh_json.json_of_string blob in
    let json = Hh_json.Access.return json in
    let open Hh_json.Access in
    let data_dump_parse_result =
      json
        >>= get_obj "data_dump"
        >>= parse_saved_state_json
    in
    let from_file_parse_result =
      json
        >>= get_string "from_file"
        >>= fun (filename, _filename_keytrace) ->
        let contents = Sys_utils.cat filename in
        let json = Hh_json.json_of_string contents in
        (Hh_json.Access.return json)
          >>= parse_saved_state_json
    in
    match
      (Result.ok_fst data_dump_parse_result),
      (Result.ok_fst from_file_parse_result) with
    | (`Fst (parsed_data_dump, _)), (`Fst (_parsed_from_file, _)) ->
      Hh_logger.log "Warning - %s"
        ("Parsed saved state target from both JSON blob data dump" ^
        " and from contents of file.");
      Hh_logger.log "Preferring data dump result";
      Some parsed_data_dump
    | (`Fst (parsed_data_dump, _)), (`Snd _) ->
      Some parsed_data_dump
    | (`Snd _), (`Fst (parsed_from_file, _)) ->
      Some parsed_from_file
    | (`Snd data_dump_failure), (`Snd from_file_failure) ->
      Hh_logger.log "parsing optional arg with_saved_state failed:\n%s\n%s"
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
  let from = ref "" in
  let from_emacs = ref false in
  let from_hhclient = ref false in
  let from_vim = ref false in
  let gen_saved_ignore_type_errors = ref false in
  let ignore_hh = ref false in
  let saved_state_ignore_hhconfig = ref false in
  let json_mode = ref false in
  let load_state_canary = ref false in
  let log_inference_constraints = ref false in
  let max_procs = ref None in
  let no_load = ref false in
  let prechecked = ref None in
  let profile_log = ref false in
  let root = ref "" in
  let replace_state_after_saving = ref false in
  let save = ref None in
  let save_with_spec = ref None in
  let save_naming = ref None in
  let should_detach = ref false in
  let version = ref false in
  let waiting_client= ref None in
  let watchman_debug_logging = ref false in
  let with_saved_state = ref None in
  let allow_non_opt_build = ref false in

  let set_ai = fun s -> ai_mode := Some (Ai_options.prepare ~server:true s) in
  let set_max_procs = fun n -> max_procs := Some n in
  let set_save_state = fun s -> save := Some s in
  let set_save_with_spec = fun s -> save_with_spec := Some s in
  let set_save_naming = fun s -> save_naming := Some s in
  let set_wait = fun fd -> waiting_client := Some (Handle.wrap_handle fd) in
  let set_with_saved_state = fun s -> with_saved_state := Some s in
  let set_from = fun s -> from := s in

  let options = [
      "--ai", Arg.String set_ai, Messages.ai;
      "--allow-non-opt-build", Arg.Set allow_non_opt_build, "";
      "--check", Arg.Set check_mode, Messages.check;
      "--config",
        Arg.String (fun s -> config := (String_utils.split2_exn '=' s) :: !config),
        Messages.config;
      "--daemon", Arg.Set should_detach, Messages.daemon;
      "--dynamic-view", Arg.Set dynamic_view, Messages.dynamic_view;
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
      "--log-inference-constraints",
        Arg.Set log_inference_constraints,
        Messages.log_inference_constraints;
      "--max-procs", Arg.Int set_max_procs, Messages.max_procs;
      "--no-load", Arg.Set no_load, Messages.no_load;
      "--no-prechecked", Arg.Unit (fun () -> prechecked := Some false), Messages.prechecked;
      "--prechecked", Arg.Unit (fun () -> prechecked := Some true), Messages.prechecked;
      "--profile-log", Arg.Set profile_log, Messages.profile_log;
      "--replace-state-after-saving",
        Arg.Set replace_state_after_saving,
        Messages.replace_state_after_saving;
      "--save-mini", Arg.String set_save_state, Messages.save_state;
      "--save-naming", Arg.String set_save_naming, Messages.save_naming;
      "--save-state", Arg.String set_save_state, Messages.save_state;
      "--save-state-with-spec", Arg.String set_save_with_spec, Messages.save_with_spec;
      "--saved-state-ignore-hhconfig",
        Arg.Set saved_state_ignore_hhconfig,
        Messages.saved_state_ignore_hhconfig;
      "--version", Arg.Set version, "";
      "--waiting-client", Arg.Int set_wait, Messages.waiting_client;
      "--watchman-debug-logging", Arg.Set watchman_debug_logging, Messages.watchman_debug_logging;
      "--with-mini-state", Arg.String set_with_saved_state, Messages.with_saved_state;
      "-d", Arg.Set should_detach, Messages.daemon;
      "-s", Arg.String set_save_state, Messages.save_state;
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
  let save_with_spec = verify_save_with_spec !save_with_spec in
  let with_saved_state = verify_with_saved_state !with_saved_state in
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
  if (Option.is_some (!save) && Option.is_some save_with_spec) then begin
    Printf.eprintf "--save-state and --save-state-with-spec cannot be combined\n%!";
    exit 1
  end;
  if (!gen_saved_ignore_type_errors) && not (Option.is_some (!save)) then begin
    Printf.eprintf
      "--gen-saved-ignore-type-errors is only valid when combined with --save-state\n%!";
    exit 1
  end;
  {
    ai_mode = !ai_mode;
    check_mode;
    config = !config;
    dynamic_view = !dynamic_view;
    from = !from;
    gen_saved_ignore_type_errors = !gen_saved_ignore_type_errors;
    ignore_hh_version = !ignore_hh;
    saved_state_ignore_hhconfig = !saved_state_ignore_hhconfig;
    json_mode = !json_mode;
    load_state_canary = !load_state_canary;
    log_inference_constraints = !log_inference_constraints;
    max_procs = !max_procs;
    no_load = !no_load;
    prechecked = !prechecked;
    profile_log = !profile_log;
    replace_state_after_saving = !replace_state_after_saving;
    root = root_path;
    save_filename = !save;
    save_with_spec;
    save_naming_filename = !save_naming;
    should_detach = !should_detach;
    waiting_client = !waiting_client;
    watchman_debug_logging = !watchman_debug_logging;
    with_saved_state;
    allow_non_opt_build = !allow_non_opt_build;
  }

(* useful in testing code *)
let default_options ~root = {
  ai_mode = None;
  check_mode = false;
  config = [];
  dynamic_view = false;
  from = "";
  gen_saved_ignore_type_errors = false;
  ignore_hh_version = false;
  saved_state_ignore_hhconfig = false;
  json_mode = false;
  load_state_canary = false;
  log_inference_constraints = false;
  max_procs = None;
  no_load = true;
  prechecked = None;
  profile_log = false;
  replace_state_after_saving = false;
  root = Path.make root;
  save_filename = None;
  save_with_spec = None;
  save_naming_filename = None;
  should_detach = false;
  waiting_client = None;
  watchman_debug_logging = false;
  with_saved_state = None;
  allow_non_opt_build = false;
}

(*****************************************************************************)
(* Accessors *)
(*****************************************************************************)

let ai_mode options = options.ai_mode
let check_mode options = options.check_mode
let config options = options.config
let dynamic_view options = options.dynamic_view
let from options = options.from
let gen_saved_ignore_type_errors options = options.gen_saved_ignore_type_errors
let ignore_hh_version options = options.ignore_hh_version
let saved_state_ignore_hhconfig options = options.saved_state_ignore_hhconfig
let json_mode options = options.json_mode
let load_state_canary options = options.load_state_canary
let log_inference_constraints options = options.log_inference_constraints
let max_procs options = options.max_procs
let no_load options = options.no_load
let prechecked options = options.prechecked
let profile_log options = options.profile_log
let replace_state_after_saving options = options.replace_state_after_saving
let root options = options.root
let save_filename options = options.save_filename
let save_with_spec options = options.save_with_spec
let save_naming_filename options = options.save_naming_filename
let should_detach options = options.should_detach
let waiting_client options = options.waiting_client
let watchman_debug_logging options = options.watchman_debug_logging
let with_saved_state options = options.with_saved_state
let allow_non_opt_build options = options.allow_non_opt_build

(*****************************************************************************)
(* Setters *)
(*****************************************************************************)

let set_gen_saved_ignore_type_errors options ignore_type_errors = { options with
  gen_saved_ignore_type_errors = ignore_type_errors}
let set_no_load options is_no_load = {options with no_load = is_no_load}

let set_saved_state_target options target =
  match target with
  | None -> options
  | Some target ->
    { options with
      with_saved_state = Some (Informant_induced_saved_state_target target)
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
    from;
    gen_saved_ignore_type_errors;
    ignore_hh_version;
    saved_state_ignore_hhconfig;
    json_mode;
    load_state_canary;
    log_inference_constraints;
    max_procs;
    no_load;
    prechecked;
    profile_log;
    replace_state_after_saving;
    root;
    save_filename;
    save_with_spec;
    save_naming_filename;
    should_detach;
    waiting_client;
    watchman_debug_logging;
    with_saved_state;
    allow_non_opt_build;
  } =
    let ai_mode_str = match ai_mode with
      | None -> "<>"
      | Some _ -> "Some(...)" in
    let saved_state_str = match with_saved_state with
      | None -> "<>"
      | Some _ -> "SavedStateTarget(...)" in
    let waiting_client_str = match waiting_client with
      | None -> "<>"
      | Some _ -> "WaitingClient(...)" in
    let save_filename_str = match save_filename with
      | None -> "<>"
      | Some path -> path in
    let save_with_spec_str = match save_with_spec with
      | None -> "<>"
      | Some _ -> "SaveStateSpec(...)" in
    let save_naming_filename_str = match save_naming_filename with
      | None -> "<>"
      | Some path -> path in
    let prechecked_str = match prechecked with
      | None -> "<>"
      | Some b -> string_of_bool b in
    let max_procs_str = match max_procs with
      | None -> "<>"
      | Some n -> string_of_int n in
    let config_str = Printf.sprintf "[%s]"
      (String.concat ~sep:", " @@ List.map ~f:(fun (key, value) -> Printf.sprintf "%s=%s" key value) config)
    in
    ([
      "ServerArgs.options({";
        "ai_mode: "; ai_mode_str; ", ";
        "check_mode: "; string_of_bool check_mode; ", ";
        "config: "; config_str;
        "dynamic_view: "; string_of_bool dynamic_view; ", ";
        "from: "; from; ", ";
        "gen_saved_ignore_type_errors: "; string_of_bool gen_saved_ignore_type_errors; ", ";
        "ignore_hh_version: "; string_of_bool ignore_hh_version; ", ";
        "saved_state_ignore_hhconfig: "; string_of_bool saved_state_ignore_hhconfig; ", ";
        "json_mode: "; string_of_bool json_mode; ", ";
        "load_state_canary: "; string_of_bool load_state_canary; ", ";
        "log_inference_constraints: "; string_of_bool log_inference_constraints; ", ";
        "maxprocs: "; max_procs_str; ", ";
        "no_load: "; string_of_bool no_load; ", ";
        "prechecked: "; prechecked_str;
        "profile_log: "; string_of_bool profile_log; ", ";
        "replace_state_after_saving: "; string_of_bool replace_state_after_saving; ", ";
        "root: "; Path.to_string root; ", ";
        "save_filename: "; save_filename_str; ", ";
        "save_with_spec: "; save_with_spec_str; ", ";
        "save_naming_filename: "; save_naming_filename_str; ", ";
        "should_detach: "; string_of_bool should_detach; ", ";
        "waiting_client: "; waiting_client_str; ", ";
        "watchman_debug_logging: "; string_of_bool watchman_debug_logging; ", ";
        "with_saved_state: "; saved_state_str; ", ";
        "allow_non_opt_build: "; string_of_bool allow_non_opt_build; ", ";
      "})"
    ] |> String.concat ~sep:"")
