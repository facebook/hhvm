(**
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

include ServerArgs_sig.Types

(*****************************************************************************)
(* The options from the command line *)
(*****************************************************************************)

type options = {
  ai_mode          : Ai_options.t option;
  check_mode       : bool;
  json_mode        : bool;
  root             : Path.t;
  should_detach    : bool;
  convert          : Path.t option;
  max_procs        : int;
  no_load          : bool;
  profile_log      : bool;
  load_state_canary : bool;
  with_mini_state  : mini_state_target option;
  save_filename    : string option;
  waiting_client   : Unix.file_descr option;
  watchman_debug_logging : bool;
  ignore_hh_version : bool;
  file_info_on_disk : bool;
  dynamic_view      : bool;
  gen_saved_ignore_type_errors : bool;
  prechecked       : bool option;
}

(*****************************************************************************)
(* Usage code *)
(*****************************************************************************)
let usage = Printf.sprintf "Usage: %s [WWW DIRECTORY]\n" Sys.argv.(0)

(*****************************************************************************)
(* Options *)
(*****************************************************************************)

module Messages = struct
  let debug         = " debugging mode"
  let ai            = " run ai with options"
  let check         = " check and exit"
  let json          = " output errors in json format (arc lint mode)"
  let daemon        = " detach process"
  let from_vim      = " passed from hh_client"
  let from_emacs    = " passed from hh_client"
  let from_hhclient = " passed from hh_client"
  let convert       = " adds type annotations automatically"
  let save          = " DEPRECATED"
  let save_mini     = " save mini server state to file"
  let max_procs     = " max numbers of workers"
  let no_load       = " don't load from a saved state"
  let profile_log   = " enable profile logging"
  let load_state_canary = " Look up a saved state using the hg commit" ^
                          " hash instead of the SVN rev."
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
  let with_mini_state = " init with the given saved state instead of getting" ^
                        " it by running load mini state script." ^
                        " Expects a JSON blob specified as" ^
                        mini_state_json_descr
  let waiting_client= " send message to fd/handle when server has begun \
                      \ starting and again when it's done starting"
  let watchman_debug_logging =
    " Enable debug logging on Watchman client. This is very noisy"
  let ignore_hh_version = " ignore hh_version check when loading saved states"
  let file_info_on_disk = " [experimental] store file-info in sqlite db."
  let dynamic_view      = " start with dynamic view for IDE files on by default."
  let gen_saved_ignore_type_errors = " generate a saved state even if there are type errors."
  let prechecked = " override value of \"prechecked_files\" flag from hh.conf"
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
      (fun file -> Hh_json.get_string_exn file |> Relative_path.from_root)
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
      (Core_result.ok_fst data_dump_parse_result),
      (Core_result.ok_fst from_file_parse_result) with
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
  let root          = ref "" in
  let from_vim      = ref false in
  let from_emacs    = ref false in
  let from_hhclient = ref false in
  let debug         = ref false in
  let ai_mode       = ref None in
  let check_mode    = ref false in
  let json_mode     = ref false in
  let should_detach = ref false in
  let convert_dir   = ref None  in
  let save          = ref None in
  let max_procs     = ref GlobalConfig.nbr_procs in
  let no_load       = ref false in
  let profile_log   = ref false in
  let load_state_canary = ref false in
  let with_mini_state = ref None in
  let version       = ref false in
  let watchman_debug_logging = ref false in
  let waiting_client= ref None in
  let ignore_hh     = ref false in
  let dynamic_view  = ref false in
  let gen_saved_ignore_type_errors = ref false in

  let file_info_on_disk = ref false in
  let cdir          = fun s -> convert_dir := Some s in
  let set_ai        = fun s ->
    ai_mode := Some (Ai_options.prepare ~server:true s) in
  let set_max_procs = fun s -> max_procs := min !max_procs s in
  let set_save ()   = Printf.eprintf "DEPRECATED\n"; exit 1 in
  let set_save_mini = fun s -> save := Some s in
  let set_wait      = fun fd ->
    waiting_client := Some (Handle.wrap_handle fd) in
  let set_with_mini_state = fun s -> with_mini_state := Some s in
  let prechecked = ref None in
  let options =
    ["--debug"         , Arg.Set debug           , Messages.debug;
     "--ai"            , Arg.String set_ai       , Messages.ai;
     "--check"         , Arg.Set check_mode      , Messages.check;
     "--json"          , Arg.Set json_mode       , Messages.json; (*CAREFUL!!!*)
     "--daemon"        , Arg.Set should_detach   , Messages.daemon;
     "-d"              , Arg.Set should_detach   , Messages.daemon;
     "--from-vim"      , Arg.Set from_vim        , Messages.from_vim;
     "--from-emacs"    , Arg.Set from_emacs      , Messages.from_emacs;
     "--from-hhclient" , Arg.Set from_hhclient   , Messages.from_hhclient;
     "--convert"       , Arg.String cdir         , Messages.convert;
     "--save"          , Arg.Unit set_save       , Messages.save;
     "--save-mini"     , Arg.String set_save_mini, Messages.save_mini;
     "--max-procs"     , Arg.Int set_max_procs   , Messages.max_procs;
     "--no-load"       , Arg.Set no_load         , Messages.no_load;
     "--profile-log"   , Arg.Set profile_log     , Messages.profile_log;
     "--load-state-canary", Arg.Set load_state_canary, Messages.load_state_canary;
     "--with-mini-state", Arg.String set_with_mini_state,
       Messages.with_mini_state;
     "--version"       , Arg.Set version         , "";
     "--waiting-client", Arg.Int set_wait        , Messages.waiting_client;
     "--watchman-debug-logging", Arg.Set watchman_debug_logging,
       Messages.watchman_debug_logging;
     "--ignore-hh-version", Arg.Set ignore_hh  , Messages.ignore_hh_version;
     "--file-info-on-disk", Arg.Set file_info_on_disk , Messages.file_info_on_disk;
     "--dynamic-view", Arg.Set dynamic_view,     Messages.dynamic_view;
     "--gen-saved-ignore-type-errors", Arg.Set gen_saved_ignore_type_errors, Messages.gen_saved_ignore_type_errors;
     "--prechecked",    Arg.Unit (fun () -> prechecked := Some true),
      Messages.prechecked;
     "--no-prechecked", Arg.Unit (fun () -> prechecked := Some false),
      Messages.prechecked;
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
  (* Conversion mode implies check *)
  let check_mode = check_mode || !convert_dir <> None in
  let convert = Option.map ~f:Path.make !convert_dir in
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
    json_mode     = !json_mode;
    ai_mode       = !ai_mode;
    check_mode    = check_mode;
    root          = root_path;
    should_detach = !should_detach;
    convert       = convert;
    max_procs     = !max_procs;
    no_load       = !no_load;
    profile_log   = !profile_log;
    load_state_canary = !load_state_canary;
    with_mini_state = with_mini_state;
    save_filename = !save;
    waiting_client= !waiting_client;
    watchman_debug_logging = !watchman_debug_logging;
    ignore_hh_version = !ignore_hh;
    file_info_on_disk = !file_info_on_disk;
    dynamic_view      = !dynamic_view;
    gen_saved_ignore_type_errors = !gen_saved_ignore_type_errors;
    prechecked = !prechecked;
  }

(* useful in testing code *)
let default_options ~root = {
  ai_mode = None;
  check_mode = false;
  json_mode = false;
  root = Path.make root;
  should_detach = false;
  convert = None;
  max_procs = GlobalConfig.nbr_procs;
  no_load = true;
  profile_log = false;
  load_state_canary = false;
  with_mini_state = None;
  save_filename = None;
  waiting_client = None;
  watchman_debug_logging = false;
  ignore_hh_version = false;
  file_info_on_disk = false;
  dynamic_view = false;
  gen_saved_ignore_type_errors = false;
  prechecked = None;
}

(*****************************************************************************)
(* Accessors *)
(*****************************************************************************)

let ai_mode options = options.ai_mode
let check_mode options = options.check_mode
let json_mode options = options.json_mode
let root options = options.root
let should_detach options = options.should_detach
let convert options = options.convert
let max_procs options = options.max_procs
let no_load options = options.no_load
let profile_log options = options.profile_log
let load_state_canary options = options.load_state_canary
let with_mini_state options = options.with_mini_state
let save_filename options = options.save_filename
let waiting_client options = options.waiting_client
let watchman_debug_logging options = options.watchman_debug_logging
let ignore_hh_version options = options.ignore_hh_version
let file_info_on_disk options = options.file_info_on_disk
let dynamic_view options = options.dynamic_view
let gen_saved_ignore_type_errors options = options.gen_saved_ignore_type_errors
let prechecked options = options.prechecked

(*****************************************************************************)
(* Setters *)
(*****************************************************************************)

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
    json_mode;
    ai_mode;
    check_mode;
    root;
    should_detach;
    convert;
    max_procs;
    no_load;
    profile_log;
    load_state_canary;
    with_mini_state;
    save_filename;
    waiting_client;
    watchman_debug_logging;
    ignore_hh_version;
    file_info_on_disk;
    dynamic_view;
    gen_saved_ignore_type_errors;
    prechecked;
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
    let convert_str = match convert with
      | None -> "<>"
      | Some path -> Path.to_string path in
    let save_filename_str = match save_filename with
      | None -> "<>"
      | Some path -> path in
    let prechecked_str = match prechecked with
      | None -> "<>"
      | Some b -> string_of_bool b in
    ([
      "ServerArgs.options({";
      "json_mode: "; string_of_bool json_mode; ", ";
      "ai_mode: "; ai_mode_str; ", ";
      "check_mode: "; string_of_bool check_mode; ", ";
      "root: "; Path.to_string root; ", ";
      "should_detach: "; string_of_bool should_detach; ", ";
      "convert: "; convert_str; ", ";
      "maxprocs: "; string_of_int max_procs; ", ";
      "no_load: "; string_of_bool no_load; ", ";
      "profile_log: "; string_of_bool profile_log; ", ";
      "load_state_canary: "; string_of_bool load_state_canary; ", ";
      "with_mini_state: "; mini_state_str; ", ";
      "save_filename: "; save_filename_str; ", ";
      "waiting_client: "; waiting_client_str; ", ";
      "watchman_debug_logging: "; string_of_bool watchman_debug_logging; ", ";
      "ignore_hh_version: "; string_of_bool ignore_hh_version; ", ";
      "file_info_on_disk: "; string_of_bool file_info_on_disk; ", ";
      "dynamic_view: "; string_of_bool dynamic_view; ", ";
      "gen_saved_ignore_type_errors: "; string_of_bool gen_saved_ignore_type_errors; ", ";
      "prechecked: "; prechecked_str;
      "})"
    ] |> String.concat "")
