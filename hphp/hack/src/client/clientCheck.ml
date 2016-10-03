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
open ClientEnv
open Utils
open ClientRefactor

module Cmd = ServerCommand
module Rpc = ServerCommandTypes

let get_list_files conn (args:client_check_env): string list =
  let ic, oc = conn in
  Cmd.stream_request oc ServerCommandTypes.LIST_FILES;
  let res = ref [] in
  try
    while true do
      res := (Timeout.input_line ic) :: !res
    done;
    assert false
  with End_of_file -> !res

let print_all ic =
  try
    while true do
      Printf.printf "%s\n" (Timeout.input_line ic);
    done
  with End_of_file -> ()

let expand_path file =
  let path = Path.make file in
  if Path.file_exists path
  then Path.to_string path
  else
    let file = Filename.concat (Sys.getcwd()) file in
    let path = Path.make file in
    if Path.file_exists path
    then Path.to_string path
    else begin
      Printf.printf "File not found\n";
      exit 2
    end

let parse_position_string arg =
  let tpos = Str.split (Str.regexp ":") arg in
  try
    match tpos with
    | [line; char] ->
        int_of_string line, int_of_string char
    | _ -> raise Exit
  with _ ->
    Printf.eprintf "Invalid position\n";
    raise Exit_status.(Exit_with Input_error)

let connect args =
  ClientConnect.connect { ClientConnect.
    root = args.root;
    autostart = args.autostart;
    retries = Some args.retries;
    retry_if_init = args.retry_if_init;
    expiry = args.timeout;
    no_load = args.no_load;
    to_ide = false;
    ai_mode = args.ai_mode;
  }

let rpc args command =
  let conn = connect args in
  Cmd.rpc conn @@ command

let is_stale_msg liveness =
   match liveness with
    | Rpc.Stale_status ->
      Some ("(but this may be stale, probably due to" ^
      " watchman being unresponsive)\n")
    | Rpc.Live_status -> None

let main args =
  let mode_s = ClientEnv.mode_to_string args.mode in
  HackEventLogger.client_set_from args.from;
  HackEventLogger.client_set_mode mode_s;

  HackEventLogger.client_check ();

  let exit_status =
    match args.mode with
    | MODE_LIST_FILES ->
      let conn = connect args in
      let infol = get_list_files conn args in
      List.iter infol (Printf.printf "%s\n");
      Exit_status.No_error
    | MODE_LIST_MODES ->
      let ic, oc = connect args in
      Cmd.stream_request oc ServerCommandTypes.LIST_MODES;
      begin try
        while true do print_endline (Timeout.input_line ic) done;
      with End_of_file -> () end;
      Exit_status.No_error
    | MODE_COLORING file ->
      let file_input = match file with
        | "-" ->
          let content = Sys_utils.read_stdin_to_string () in
          ServerUtils.FileContent content
        | _ ->
          let file = expand_path file in
          ServerUtils.FileName file
      in
      let pos_level_l = rpc args @@ Rpc.COVERAGE_LEVELS file_input in
      ClientColorFile.go file_input args.output_json pos_level_l;
      Exit_status.No_error
    | MODE_COVERAGE file ->
      let counts_opt =
        rpc args @@ Rpc.COVERAGE_COUNTS (expand_path file) in
      ClientCoverageMetric.go ~json:args.output_json counts_opt;
      Exit_status.No_error
    | MODE_FIND_CLASS_REFS name ->
      let results =
        rpc args @@ Rpc.FIND_REFS (FindRefsService.Class name) in
      ClientFindRefs.go results args.output_json;
      Exit_status.No_error
    | MODE_FIND_REFS name ->
      let pieces = Str.split (Str.regexp "::") name in
      let action =
        try
          match pieces with
          | class_name :: method_name :: _ ->
              FindRefsService.Member
                (class_name, FindRefsService.Method method_name)
          | method_name :: _ -> FindRefsService.Function method_name
          | _ -> raise Exit
        with _ ->
          Printf.eprintf "Invalid input\n";
          raise Exit_status.(Exit_with Input_error)
      in
      let results = rpc args @@ Rpc.FIND_REFS action in
      ClientFindRefs.go results args.output_json;
      Exit_status.No_error
    | MODE_IDE_FIND_REFS arg ->
      let line, char = parse_position_string arg in
      let content = Sys_utils.read_stdin_to_string () in
      let results =
        rpc args @@ Rpc.IDE_FIND_REFS (content, line, char) in
      ClientFindRefs.go results args.output_json;
      Exit_status.No_error
    | MODE_IDE_HIGHLIGHT_REFS arg ->
      let line, char = parse_position_string arg in
      let content = Sys_utils.read_stdin_to_string () in
      let results =
        rpc args @@ Rpc.IDE_HIGHLIGHT_REFS (content, line, char) in
      ClientHighlightRefs.go results ~output_json:args.output_json;
      Exit_status.No_error
    | MODE_DUMP_SYMBOL_INFO files ->
      let conn = connect args in
      ClientSymbolInfo.go conn files expand_path;
      Exit_status.No_error
    | MODE_DUMP_AI_INFO files ->
      let conn = connect args in
      ClientAiInfo.go conn files expand_path;
      Exit_status.No_error
    | MODE_FIND_DEPENDENT_FILES files ->
      let conn = connect args in
      ClientFindDependentFiles.go conn files expand_path;
      Exit_status.No_error
    | MODE_REFACTOR (ref_mode, before, after) ->
      let conn = connect args in
      ClientRefactor.go conn args ref_mode before after;
      Exit_status.No_error
    | MODE_IDENTIFY_FUNCTION arg ->
      let line, char = parse_position_string arg in
      let content = Sys_utils.read_stdin_to_string () in
      let result =
        rpc args @@ Rpc.IDENTIFY_FUNCTION (content, line, char) in
      let result = match List.hd result with
        | Some (result, _) -> Utils.strip_ns result.SymbolOccurrence.name
        | _ -> ""
      in
      print_endline result;
      Exit_status.No_error
    | MODE_GET_DEFINITION arg ->
      let line, char = parse_position_string arg in
      let content = Sys_utils.read_stdin_to_string () in
      let result =
        rpc args @@ Rpc.IDENTIFY_FUNCTION (content, line, char) in
      ClientGetDefinition.go result args.output_json;
      Exit_status.No_error
    | MODE_GET_DEFINITION_BY_ID id ->
      let result = rpc args @@ Rpc.GET_DEFINITION_BY_ID id in
      ClientOutline.print_definition result args.output_json;
      Exit_status.No_error
    | MODE_TYPE_AT_POS arg ->
      let tpos = Str.split (Str.regexp ":") arg in
      let fn, line, char =
        try
          match tpos with
          | [filename; line; char] ->
              let fn = expand_path filename in
              ServerUtils.FileName fn, int_of_string line, int_of_string char
          | [line; char] ->
              let content = Sys_utils.read_stdin_to_string () in
              ServerUtils.FileContent content,
              int_of_string line,
              int_of_string char
          | _ -> raise Exit
        with _ ->
          Printf.eprintf "Invalid position\n";
          raise Exit_status.(Exit_with Input_error)
      in
      let pos, ty = rpc args @@ Rpc.INFER_TYPE (fn, line, char) in
      ClientTypeAtPos.go pos ty args.output_json;
      Exit_status.No_error
    | MODE_ARGUMENT_INFO arg ->
      let tpos = Str.split (Str.regexp ":") arg in
      let line, char =
        try
          match tpos with
          | [line; char] ->
              int_of_string line, int_of_string char
          | _ -> raise Exit
        with _ ->
          Printf.eprintf "Invalid position\n";
          raise Exit_status.(Exit_with Input_error)
      in
      let content = Sys_utils.read_stdin_to_string () in
      let results =
        rpc args @@ Rpc.ARGUMENT_INFO (content, line, char) in
      ClientArgumentInfo.go results args.output_json;
      Exit_status.No_error
    | MODE_AUTO_COMPLETE ->
      let content = Sys_utils.read_stdin_to_string () in
      let results = rpc args @@ Rpc.AUTOCOMPLETE content in
      ClientAutocomplete.go results args.output_json;
      Exit_status.No_error
    | MODE_OUTLINE ->
      let content = Sys_utils.read_stdin_to_string () in
      let results = FileOutline.outline content in
      ClientOutline.go_legacy results args.output_json;
      Exit_status.No_error
    | MODE_OUTLINE2 ->
      let content = Sys_utils.read_stdin_to_string () in
      let results = FileOutline.outline content in
      ClientOutline.go results args.output_json;
      Exit_status.No_error
    | MODE_METHOD_JUMP_CHILDREN class_ ->
      let results = rpc args @@ Rpc.METHOD_JUMP (class_, true) in
      ClientMethodJumps.go results true args.output_json;
      Exit_status.No_error
    | MODE_METHOD_JUMP_ANCESTORS class_ ->
      let results = rpc args @@ Rpc.METHOD_JUMP (class_, false) in
      ClientMethodJumps.go results false args.output_json;
      Exit_status.No_error
    | MODE_STATUS ->
      let liveness, error_list = rpc args Rpc.STATUS in
      let stale_msg = is_stale_msg liveness in
      if args.output_json || args.from <> "" || error_list = []
      then begin
        (* this should really go to stdout but we need to adapt the various
         * IDE plugins first *)
        let oc = if args.output_json then stderr else stdout in
        ServerError.print_errorl
          stale_msg args.output_json error_list oc
      end else begin
        List.iter error_list ClientCheckStatus.print_error_color;
        Option.iter stale_msg ~f:(fun msg -> Printf.printf "%s" msg)
      end;
      if error_list = [] then Exit_status.No_error else Exit_status.Type_error
    | MODE_SHOW classname ->
      let ic, oc = connect args in
      Cmd.stream_request oc (ServerCommandTypes.SHOW classname);
      print_all ic;
      Exit_status.No_error
    | MODE_SEARCH (query, type_) ->
      let results = rpc args @@ Rpc.SEARCH (query, type_) in
      ClientSearch.go results args.output_json;
      Exit_status.No_error
    | MODE_LINT fnl ->
      let fnl = List.fold_left fnl ~f:begin fun acc fn ->
        match Sys_utils.realpath fn with
        | Some path -> path :: acc
        | None ->
            prerr_endlinef "Could not find file '%s'" fn;
            acc
      end ~init:[] in
      let results = rpc args @@ Rpc.LINT fnl in
      ClientLint.go results args.output_json;
      Exit_status.No_error
    | MODE_LINT_ALL code ->
      let results = rpc args @@ Rpc.LINT_ALL code in
      ClientLint.go results args.output_json;
      Exit_status.No_error
    | MODE_CREATE_CHECKPOINT x ->
      rpc args @@ Rpc.CREATE_CHECKPOINT x;
      Exit_status.No_error
    | MODE_RETRIEVE_CHECKPOINT x ->
      let results = rpc args @@ Rpc.RETRIEVE_CHECKPOINT x in
      begin
        match results with
        | Some results ->
            List.iter results print_endline;
            Exit_status.No_error
        | None ->
            Exit_status.Checkpoint_error
      end
    | MODE_DELETE_CHECKPOINT x ->
      if rpc args @@ Rpc.DELETE_CHECKPOINT x then
        Exit_status.No_error
      else
        Exit_status.Checkpoint_error
    | MODE_STATS ->
      let stats = rpc args @@ Rpc.STATS in
      print_string @@ Hh_json.json_to_multiline (Stats.to_json stats);
      Exit_status.No_error
    | MODE_REMOVE_DEAD_FIXMES codes ->
      let conn = connect args in
      let patches = ServerCommand.rpc conn @@
        Rpc.REMOVE_DEAD_FIXMES codes in
      let file_map = List.fold_left patches
        ~f:map_patches_to_filename ~init:SMap.empty in
      if args.output_json
      then print_patches_json file_map
      else apply_patches file_map;
      Exit_status.No_error
    | MODE_FIND_LVAR_REFS arg ->
      let line, char = parse_position_string arg in
      let content = Sys_utils.read_stdin_to_string () in
      let results =
        rpc args @@ Rpc.FIND_LVAR_REFS (content, line, char) in
      ClientFindLocals.go results args.output_json;
      Exit_status.No_error
    | MODE_GET_METHOD_NAME arg ->
      let line, char = parse_position_string arg in
      let content = Sys_utils.read_stdin_to_string () in
      let result =
        rpc args @@ Rpc.IDENTIFY_FUNCTION (content, line, char) in
      ClientGetMethodName.go (List.hd result) args.output_json;
      Exit_status.No_error
    | MODE_FORMAT (from, to_) ->
      let content = Sys_utils.read_stdin_to_string () in
      let result =
        rpc args @@ Rpc.FORMAT (content, from, to_) in
      ClientFormat.go result args.output_json;
      Exit_status.No_error
    | MODE_TRACE_AI name ->
      let pieces = Str.split (Str.regexp "::") name in
      let action =
        try
          match pieces with
          | class_name :: method_name :: _ ->
              Ai.TraceService.Member (class_name,
                  Ai.TraceService.Method method_name)
          | method_name :: _ -> Ai.TraceService.Function method_name
          | _ -> raise Exit
        with _ ->
          Printf.eprintf "Invalid input\n";
          raise Exit_status.(Exit_with Input_error)
      in
      let results = rpc args @@ Rpc.TRACE_AI action in
      ClientTraceAi.go results args.output_json;
      Exit_status.No_error
    | MODE_AI_QUERY json ->
      let results = rpc args @@ Rpc.AI_QUERY json in
      ClientAiQuery.go results;
      Exit_status.No_error
    | MODE_FULL_FIDELITY_PARSE file ->
      (* We can cheaply do this on the client today, but we might want to
      do it on the server and cache the results in the future. *)
      let do_it_on_server = false in
      let results = if do_it_on_server then
        rpc args @@ Rpc.DUMP_FULL_FIDELITY_PARSE file
      else
        let file = Relative_path.create Relative_path.Dummy file in
        let source_text = Full_fidelity_source_text.from_file file in
        let syntax_tree = Full_fidelity_syntax_tree.make source_text in
        let json = Full_fidelity_syntax_tree.to_json syntax_tree in
        Hh_json.json_to_string json in
      ClientFullFidelityParse.go results;
      Exit_status.No_error
  in
  HackEventLogger.client_check_finish exit_status;
  exit_status
