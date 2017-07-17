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

let parse_function_or_method_id ~func_action ~meth_action name =
  let pieces = Str.split (Str.regexp "::") name in
  try
    match pieces with
    | class_name :: method_name :: _ -> meth_action class_name method_name
    | fun_name :: _ -> func_action fun_name
    | _ -> raise Exit
  with _ ->
    Printf.eprintf "Invalid input\n";
    raise Exit_status.(Exit_with Input_error)

let get_list_files conn: string list =
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
    force_dormant_start = args.force_dormant_start;
    retries = Some args.retries;
    retry_if_init = args.retry_if_init;
    expiry = args.timeout;
    no_load = args.no_load;
    profile_log = args.profile_log;
    ai_mode = args.ai_mode;
    progress_callback = ClientConnect.tty_progress_reporter;
    do_post_handoff_handshake = true;
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

let warn_unsaved_changes () =
  (* Make sure any buffered diagnostics are printed before printing this
     warning. *)
  flush stdout;
  Tty.cprintf (Tty.Bold Tty.Yellow) "Warning: " ~out_channel:stderr;
  prerr_endline
{|there is an editor connected to the Hack server.
The errors below may reflect your unsaved changes in the editor.|}

let main args =
  let mode_s = ClientEnv.mode_to_string args.mode in
  HackEventLogger.client_set_from args.from;
  HackEventLogger.client_set_mode mode_s;

  HackEventLogger.client_check ();

  let exit_status =
    match args.mode with
    | MODE_LIST_FILES ->
      let conn = connect args in
      let infol = get_list_files conn in
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
      let action =
        parse_function_or_method_id
          ~meth_action:(fun class_name method_name ->
            FindRefsService.Member
              (class_name, FindRefsService.Method method_name))
          ~func_action:(fun fun_name -> FindRefsService.Function fun_name)
          name
      in
      let results = rpc args @@ Rpc.FIND_REFS action in
      ClientFindRefs.go results args.output_json;
      Exit_status.No_error
    | MODE_IDE_FIND_REFS arg ->
      let line, char = parse_position_string arg in
      let include_defs = false in
      let content =
        ServerUtils.FileContent (Sys_utils.read_stdin_to_string ()) in
      let results =
        rpc args @@ Rpc.IDE_FIND_REFS (content, line, char, include_defs) in
      ClientFindRefs.go_ide results args.output_json;
      Exit_status.No_error
    | MODE_IDE_HIGHLIGHT_REFS arg ->
      let line, char = parse_position_string arg in
      let content =
        ServerUtils.FileContent (Sys_utils.read_stdin_to_string ()) in
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
    | MODE_IDENTIFY_SYMBOL1 arg
    | MODE_IDENTIFY_SYMBOL2 arg
    | MODE_IDENTIFY_SYMBOL3 arg ->
      let line, char = parse_position_string arg in
      let content =
        ServerUtils.FileContent (Sys_utils.read_stdin_to_string ()) in
      let result = rpc args @@ Rpc.IDENTIFY_FUNCTION (content, line, char) in
      ClientGetDefinition.go result args.output_json;
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
      let ty = rpc args @@ Rpc.INFER_TYPE (fn, line, char) in
      ClientTypeAtPos.go ty args.output_json;
      Exit_status.No_error
    | MODE_AUTO_COMPLETE ->
      let content = Sys_utils.read_stdin_to_string () in
      let results = rpc args @@ Rpc.AUTOCOMPLETE content in
      ClientAutocomplete.go results args.output_json;
      Exit_status.No_error
    | MODE_OUTLINE | MODE_OUTLINE2 ->
      let content = Sys_utils.read_stdin_to_string () in
      let results = FileOutline.outline
      (**
       * TODO: Don't use default parser options.
       *
       * Parser options enables certain features (such as namespace aliasing)
       * Thus, for absolute correctness of outlining, we need to use the same
       * parser options that the server uses. But this client request doesn't
       * hit the server at all. So either change this to a server RPC, or
       * ask the server what its parser options are, or parse the
       * options from the .hhconfig file (needs to be the same hhconfig file the
       * server used).
       * *)
        ParserOptions.default content in
      ClientOutline.go results args.output_json;
      Exit_status.No_error
    | MODE_METHOD_JUMP_CHILDREN class_ ->
      let filter = MethodJumps.No_filter in
      let results = rpc args @@ Rpc.METHOD_JUMP (class_, filter, true) in
      ClientMethodJumps.go results true args.output_json;
      Exit_status.No_error
    | MODE_METHOD_JUMP_ANCESTORS (class_, filter) ->
      let filter =
        match MethodJumps.string_filter_to_method_jump_filter filter with
        | Some filter -> filter
        | None ->
          Printf.eprintf "Invalid method jump filter %s\n" filter;
          raise Exit_status.(Exit_with Input_error)
      in
      let results = rpc args @@ Rpc.METHOD_JUMP (class_, filter, false) in
      ClientMethodJumps.go results false args.output_json;
      Exit_status.No_error
    | MODE_STATUS ->
      let ignore_ide = ClientMessages.ignore_ide_from args.from in
      let {
        Rpc.Server_status.liveness;
        has_unsaved_changes;
        error_list;
      } = rpc args (Rpc.STATUS ignore_ide) in
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
        Option.iter stale_msg ~f:(fun msg -> Printf.printf "%s" msg);
        if has_unsaved_changes then warn_unsaved_changes ()
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
    | MODE_IGNORE_FIXMES files ->
      let conn = connect args in
      let {
        Rpc.Ignore_fixmes_result.has_unsaved_changes;
        error_list;
      } = ServerCommand.rpc conn @@ Rpc.IGNORE_FIXMES files in
      if args.output_json || args.from <> "" || error_list = []
      then begin
        let oc = if args.output_json then stderr else stdout in
        ServerError.print_errorl None args.output_json error_list oc
      end else begin
        List.iter error_list ClientCheckStatus.print_error_color;
        if has_unsaved_changes then warn_unsaved_changes ()
      end;
      if error_list = [] then Exit_status.No_error else Exit_status.Type_error
    | MODE_FORMAT (from, to_) ->
      let content = Sys_utils.read_stdin_to_string () in
      let result =
        rpc args @@ Rpc.FORMAT (content, from, to_) in
      ClientFormat.go result args.output_json;
      Exit_status.No_error
    | MODE_TRACE_AI name ->
      let action =
        parse_function_or_method_id
          ~meth_action:(fun class_name method_name ->
            Ai.TraceService.Member
              (class_name, Ai.TraceService.Method method_name))
          ~func_action:(fun fun_name -> Ai.TraceService.Function fun_name)
          name
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
    | MODE_FULL_FIDELITY_SCHEMA ->
      let schema = Full_fidelity_schema.schema_as_json() in
      print_string schema;
      Exit_status.No_error
    | MODE_INFER_RETURN_TYPE name ->
      let action =
        parse_function_or_method_id
          ~func_action:(fun fun_name -> InferReturnTypeService.Function fun_name)
          ~meth_action:(fun class_name meth_name ->
            InferReturnTypeService.Method (class_name, meth_name))
          name
      in
      let result = rpc args @@ Rpc.INFER_RETURN_TYPE action in
      match result with
      | Result.Error error_str ->
        Printf.eprintf "%s\n" error_str;
        raise Exit_status.(Exit_with Input_error)
      | Result.Ok ty ->
        if args.output_json then
          print_endline Hh_json.(json_to_string (JSON_String ty))
        else
          print_endline ty;
        Exit_status.No_error
  in
  HackEventLogger.client_check_finish exit_status;
  exit_status
