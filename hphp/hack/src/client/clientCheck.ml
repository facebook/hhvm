(**
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Hh_core
open ClientEnv
open Utils
open ClientRefactor
open Ocaml_overrides

module Cmd = ServerCommand
module Rpc = ServerCommandTypes
module SyntaxTree = Full_fidelity_syntax_tree
  .WithSyntax(Full_fidelity_minimal_syntax)

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
      Printf.printf "File not found: %s\n" file;
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

let connect ?(use_priority_pipe=false) args =
  ClientConnect.connect { ClientConnect.
    root = args.root;
    autostart = args.autostart;
    force_dormant_start = args.force_dormant_start;
    retries = Some args.retries;
    expiry = args.timeout;
    no_load = args.no_load;
    watchman_debug_logging = args.watchman_debug_logging;
    profile_log = args.profile_log;
    ai_mode = args.ai_mode;
    progress_callback = ClientConnect.tty_progress_reporter ();
    do_post_handoff_handshake = true;
    ignore_hh_version = args.ignore_hh_version;
    use_priority_pipe;
    prechecked = args.prechecked;
  }

(* This is a function, because server closes the connection after each command,
 * so we need to be able to reconnect to retry. *)
type connect_fun = unit -> ClientConnect.conn

let rpc
  (args : ClientEnv.client_check_env)
  (command : 'a ServerCommandTypes.t)
  (call : connect_fun -> 'a ServerCommandTypes.t -> 'b)
: 'b =
  let use_priority_pipe =
    (not @@ ServerCommand.rpc_command_needs_full_check command) &&
    (not @@ ServerCommand.rpc_command_needs_writes command)
  in
  let conn = fun () -> connect args ~use_priority_pipe in
  call conn @@ command

let rpc_with_retry
  (args : ClientEnv.client_check_env)
  (command : 'a ServerCommandTypes.Done_or_retry.t ServerCommandTypes.t)
: 'a =
  rpc args command ClientConnect.rpc_with_retry

let rpc
  (args : ClientEnv.client_check_env)
  (command : 'a ServerCommandTypes.t)
: 'a =
  rpc args command (fun conn -> ClientConnect.rpc (conn ()))

let main args =
  let mode_s = ClientEnv.mode_to_string args.mode in
  HackEventLogger.client_set_from args.from;
  HackEventLogger.client_set_mode mode_s;

  HackEventLogger.client_check ();

  let exit_status =
    match args.mode with
    | MODE_LIST_FILES ->
      let ClientConnect.{channels; _} = connect args in
      let infol = get_list_files channels in
      List.iter infol (Printf.printf "%s\n");
      Exit_status.No_error
    | MODE_LIST_MODES ->
      let ClientConnect.{channels = ic, oc; _} = connect args in
      Cmd.stream_request oc ServerCommandTypes.LIST_MODES;
      begin try
        while true do print_endline (Timeout.input_line ic) done;
      with End_of_file -> () end;
      Exit_status.No_error
    | MODE_COLORING file ->
      let file_input = match file with
        | "-" ->
          let content = Sys_utils.read_stdin_to_string () in
          ServerCommandTypes.FileContent content
        | _ ->
          let file = expand_path file in
          ServerCommandTypes.FileName file
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
        rpc_with_retry args @@ Rpc.FIND_REFS (ServerCommandTypes.Find_refs.Class name) in
      ClientFindRefs.go results args.output_json;
      Exit_status.No_error
    | MODE_FIND_REFS name ->
      let open ServerCommandTypes.Find_refs in
      let action =
        parse_function_or_method_id
          ~meth_action:(fun class_name method_name ->
            Member
              (class_name, Method method_name))
          ~func_action:(fun fun_name -> Function fun_name)
          name
      in
      let results = rpc_with_retry args @@ Rpc.FIND_REFS action in
      ClientFindRefs.go results args.output_json;
      Exit_status.No_error
    | MODE_IDE_FIND_REFS arg ->
      let line, char = parse_position_string arg in
      let include_defs = false in
      let content = Sys_utils.read_stdin_to_string () in
      let labelled_file =
        ServerCommandTypes.LabelledFileContent { filename = ""; content; } in
      let results =
        rpc_with_retry args @@ Rpc.IDE_FIND_REFS (labelled_file, line, char, include_defs) in
      ClientFindRefs.go_ide results args.output_json;
      Exit_status.No_error
    | MODE_IDE_HIGHLIGHT_REFS arg ->
      let line, char = parse_position_string arg in
      let content =
        ServerCommandTypes.FileContent (Sys_utils.read_stdin_to_string ()) in
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
      let conn = fun () -> connect args in
      ClientRefactor.go conn args ref_mode before after;
      Exit_status.No_error
    | MODE_IDE_REFACTOR arg ->
      let conn = fun () -> connect args in
      let tpos = Str.split (Str.regexp ":") arg in
      let filename, line, char, new_name =
        try
          match tpos with
          | [filename; line; char; new_name] ->
            let filename = expand_path filename in
              filename,
              int_of_string line,
              int_of_string char,
              new_name
          | _ -> raise Exit
        with _ ->
          Printf.eprintf "Invalid input\n";
          raise Exit_status.(Exit_with Input_error)
      in
      ClientRefactor.go_ide conn args filename line char new_name;
      Exit_status.No_error
    | MODE_IDENTIFY_SYMBOL1 arg
    | MODE_IDENTIFY_SYMBOL2 arg
    | MODE_IDENTIFY_SYMBOL3 arg ->
      let line, char = parse_position_string arg in
      let content =
        ServerCommandTypes.FileContent (Sys_utils.read_stdin_to_string ()) in
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
              ServerCommandTypes.FileName fn, int_of_string line, int_of_string char
          | [line; char] ->
              let content = Sys_utils.read_stdin_to_string () in
              ServerCommandTypes.FileContent content,
              int_of_string line,
              int_of_string char
          | _ -> raise Exit
        with _ ->
          Printf.eprintf "Invalid position\n";
          raise Exit_status.(Exit_with Input_error)
      in
      let ty = rpc args @@ Rpc.INFER_TYPE (fn, line, char, args.dynamic_view) in
      ClientTypeAtPos.go ty args.output_json;
      Exit_status.No_error
    | MODE_TYPE_AT_POS_BATCH positions ->
      let positions = List.map positions begin fun pos ->
        try
          match Str.split (Str.regexp ":") pos with
          | [filename; line; char] ->
              expand_path filename, int_of_string line, int_of_string char, None
          | [filename; start_line; start_char; end_line; end_char] ->
              let filename = expand_path filename in
              let start_line = int_of_string start_line in
              let start_char = int_of_string start_char in
              let end_line = int_of_string end_line in
              let end_char = int_of_string end_char in
              (filename, start_line, start_char, Some (end_line, end_char))
          | _ -> raise Exit
        with _ ->
          Printf.eprintf "Invalid position\n";
          raise Exit_status.(Exit_with Input_error)
      end in
      let responses = rpc args @@ Rpc.INFER_TYPE_BATCH (positions, args.dynamic_view) in
      List.iter responses print_endline;
      Exit_status.No_error
    | MODE_TYPED_FULL_FIDELITY_PARSE filename ->
      let fn =
        try
          (expand_path filename)
        with _ ->
          Printf.eprintf "Invalid filename: %s\n" filename;
          raise Exit_status.(Exit_with Input_error)
      in
      let result = rpc args @@ Rpc.TYPED_AST (fn) in
      print_endline result;
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
      let filter = ServerCommandTypes.Method_jumps.No_filter in
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
    | MODE_METHOD_JUMP_ANCESTORS_BATCH (classes, filter) ->
      let filter =
        match MethodJumps.string_filter_to_method_jump_filter filter with
        | Some filter -> filter
        | None ->
          Printf.eprintf "Invalid method jump filter %s\n" filter;
          raise Exit_status.(Exit_with Input_error)
      in
      let results = rpc args @@ Rpc.METHOD_JUMP_BATCH (classes, filter) in
      ClientMethodJumps.go results false args.output_json;
      Exit_status.No_error
    | MODE_IN_MEMORY_DEP_TABLE_SIZE ->
      let result = rpc args @@ Rpc.IN_MEMORY_DEP_TABLE_SIZE in
      ClientResultPrinter.Int_printer.go result args.output_json;
      Exit_status.No_error
    | MODE_SAVE_STATE path ->
      let () = Sys_utils.mkdir_p (Filename.dirname path) in
      (** Convert to real path because Client and Server may have
       * different cwd and we want to use the client processes' cwd. *)
      let path = Path.make path in
      let result = rpc args @@ Rpc.SAVE_STATE (Path.to_string path) in
      ClientResultPrinter.Int_printer.go result args.output_json;
      Exit_status.No_error
    | MODE_STATUS ->
      let ignore_ide = ClientMessages.ignore_ide_from args.from in
      if args.prechecked = Some false then rpc args (Rpc.NO_PRECHECKED_FILES);
      let status = rpc args (Rpc.STATUS ignore_ide) in
      ClientCheckStatus.go status args.output_json args.from
    | MODE_STATUS_SINGLE filename ->
      let file_input = match filename with
        | "-" ->
          ServerCommandTypes.FileContent (Sys_utils.read_stdin_to_string ())
        | _ ->
          ServerCommandTypes.FileName (expand_path filename)
      in
      let error_list = rpc args (Rpc.STATUS_SINGLE file_input) in
      let status = {
        error_list;
        Rpc.Server_status.liveness = Rpc.Live_status;
        has_unsaved_changes = false;
      } in
      ClientCheckStatus.go status args.output_json args.from
    | MODE_SHOW classname ->
      let ClientConnect.{channels = ic, oc; _} = connect args in
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
    | MODE_LINT_STDIN filename ->
      begin match Sys_utils.realpath filename with
      | None ->
        prerr_endlinef "Could not find file '%s'" filename;
        Exit_status.Input_error
      | Some filename ->
        let contents = Sys_utils.read_stdin_to_string () in
        let results = rpc args @@ Rpc.LINT_STDIN
          { ServerCommandTypes.filename; contents } in
        ClientLint.go results args.output_json;
        Exit_status.No_error
      end
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
      (* we need to confirm that the server is not already started
       * in a non-no-load (yes-load) state
       *)
      let conn = connect args in
      let response = ClientConnect.rpc conn @@
          Rpc.REMOVE_DEAD_FIXMES codes in
      begin match response with
      | `Error msg ->
        (Printf.eprintf "%s\n" msg; Exit_status.Type_error)
      | `Ok patches -> begin
        if args.output_json
        then print_patches_json patches
        else apply_patches patches;
        Exit_status.No_error
      end end
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
        let syntax_tree = SyntaxTree.make source_text in
        let json = SyntaxTree.to_json syntax_tree in
        Hh_json.json_to_string json in
      ClientFullFidelityParse.go results;
      Exit_status.No_error
    | MODE_FULL_FIDELITY_SCHEMA ->
      let schema = Full_fidelity_schema.schema_as_json() in
      print_string schema;
      Exit_status.No_error
    | MODE_INFER_RETURN_TYPE name ->
      let open ServerCommandTypes.Infer_return_type in
      let action =
        parse_function_or_method_id
          ~func_action:(fun fun_name -> Function fun_name)
          ~meth_action:(fun class_name meth_name ->
            Method (class_name, meth_name))
          name
      in
      let result = rpc args @@ Rpc.INFER_RETURN_TYPE action in
      begin match result with
      | Error error_str ->
        Printf.eprintf "%s\n" error_str;
        raise Exit_status.(Exit_with Input_error)
      | Ok ty ->
        if args.output_json then
          print_endline Hh_json.(json_to_string (JSON_String ty))
        else
          print_endline ty;
        Exit_status.No_error
      end
    | MODE_CST_SEARCH files_to_search ->
      let sort_results = args.sort_results in
      let input = Sys_utils.read_stdin_to_string () |> Hh_json.json_of_string in
      let result = rpc args @@ Rpc.CST_SEARCH { Rpc.
        sort_results;
        input;
        files_to_search;
      } in
      begin match result with
      | Ok result ->
        print_endline (Hh_json.json_to_string result);
        Exit_status.No_error
      | Error error -> print_endline error;
        raise Exit_status.(Exit_with Input_error)
      end
  in
  HackEventLogger.client_check_finish exit_status;
  exit_status
