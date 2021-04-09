(*
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Hh_prelude
open ClientEnv
open Utils
open ClientRefactor
open Ocaml_overrides
module Rpc = ServerCommandTypes
module SyntaxTree =
  Full_fidelity_syntax_tree.WithSyntax (Full_fidelity_positioned_syntax)

module SaveStateResultPrinter = ClientResultPrinter.Make (struct
  type t = SaveStateServiceTypes.save_state_result

  let to_string t =
    Printf.sprintf
      "Dependency table edges added: %d"
      t.SaveStateServiceTypes.dep_table_edges_added

  let to_json t =
    Hh_json.JSON_Object (ServerError.get_save_state_result_props_json t)
end)

module SaveNamingResultPrinter = ClientResultPrinter.Make (struct
  type t = SaveStateServiceTypes.save_naming_result

  let to_string t =
    Printf.sprintf
      "Files added: %d, symbols added: %d"
      t.SaveStateServiceTypes.nt_files_added
      t.SaveStateServiceTypes.nt_symbols_added

  let to_json t =
    Hh_json.JSON_Object
      [
        ( "files_added",
          Hh_json.JSON_Number
            (string_of_int t.SaveStateServiceTypes.nt_files_added) );
        ( "symbols_added",
          Hh_json.JSON_Number
            (string_of_int t.SaveStateServiceTypes.nt_symbols_added) );
      ]
end)

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

let print_all ic =
  try
    while true do
      Printf.printf "%s\n" (Timeout.input_line ic)
    done
  with End_of_file -> ()

let expand_path file =
  let path = Path.make file in
  if Path.file_exists path then
    Path.to_string path
  else
    let file = Filename.concat (Sys.getcwd ()) file in
    let path = Path.make file in
    if Path.file_exists path then
      Path.to_string path
    else (
      Printf.printf "File not found: %s\n" file;
      exit 2
    )

let parse_position_string arg =
  let tpos = Str.split (Str.regexp ":") arg in
  try
    match tpos with
    | [line; char] -> (int_of_string line, int_of_string char)
    | _ -> raise Exit
  with _ ->
    Printf.eprintf "Invalid position\n";
    raise Exit_status.(Exit_with Input_error)

let connect ?(use_priority_pipe = false) args =
  let {
    root;
    from;
    autostart;
    force_dormant_start;
    deadline;
    no_load;
    watchman_debug_logging;
    log_inference_constraints;
    profile_log;
    remote;
    ai_mode;
    show_spinner;
    ignore_hh_version;
    save_64bit;
    saved_state_ignore_hhconfig;
    prechecked;
    mini_state;
    config;
    allow_non_opt_build;
    custom_telemetry_data;
    dynamic_view = _;
    error_format = _;
    gen_saved_ignore_type_errors = _;
    paths = _;
    max_errors = _;
    mode = _;
    output_json = _;
    replace_state_after_saving = _;
    sort_results = _;
    stdin_name = _;
    desc = _;
  } =
    args
  in
  ClientConnect.(
    connect
      {
        root;
        from;
        autostart;
        force_dormant_start;
        deadline;
        no_load;
        watchman_debug_logging;
        log_inference_constraints;
        profile_log;
        remote;
        ai_mode;
        progress_callback =
          Option.some_if show_spinner (ClientConnect.tty_progress_reporter ());
        do_post_handoff_handshake = true;
        ignore_hh_version;
        save_64bit;
        saved_state_ignore_hhconfig;
        use_priority_pipe;
        prechecked;
        mini_state;
        config;
        custom_telemetry_data;
        allow_non_opt_build;
      })

(* This is a function, because server closes the connection after each command,
 * so we need to be able to reconnect to retry. *)
type connect_fun = unit -> ClientConnect.conn Lwt.t

let rpc
    (args : ClientEnv.client_check_env)
    (command : 'a ServerCommandTypes.t)
    (call : connect_fun -> desc:string -> 'a ServerCommandTypes.t -> 'b Lwt.t) :
    'b Lwt.t =
  let use_priority_pipe =
    (not @@ ServerCommand.rpc_command_needs_full_check command)
    && (not @@ ServerCommand.rpc_command_needs_writes command)
  in
  let conn () = connect args ~use_priority_pipe in
  let%lwt result = call conn ~desc:args.desc @@ command in
  Lwt.return result

let rpc_with_retry
    (args : ClientEnv.client_check_env)
    (command : 'a ServerCommandTypes.Done_or_retry.t ServerCommandTypes.t) :
    'a Lwt.t =
  let%lwt result = rpc args command ClientConnect.rpc_with_retry in
  Lwt.return result

let rpc
    (args : ClientEnv.client_check_env) (command : 'result ServerCommandTypes.t)
    : ('result * Telemetry.t) Lwt.t =
  rpc args command (fun conn_f ~desc command ->
      let%lwt conn = conn_f () in
      let%lwt (result, telemetry) = ClientConnect.rpc conn ~desc command in
      Lwt.return (result, telemetry))

let parse_positions positions =
  List.map positions (fun pos ->
      try
        match Str.split (Str.regexp ":") pos with
        | [filename; line; char] ->
          (expand_path filename, int_of_string line, int_of_string char)
        | _ -> raise Exit
      with _ ->
        Printf.eprintf "Invalid position\n";
        raise Exit_status.(Exit_with Input_error))

(* Filters and prints errors when a path is not a realpath *)
let filter_real_paths paths =
  List.filter_map paths ~f:(fun fn ->
      match Sys_utils.realpath fn with
      | Some path -> Some path
      | None ->
        prerr_endlinef "Could not find file '%s'" fn;
        None)

let main (args : client_check_env) : Exit_status.t Lwt.t =
  let mode_s = ClientEnv.mode_to_string args.mode in
  HackEventLogger.set_from args.from;
  HackEventLogger.client_set_mode mode_s;

  HackEventLogger.client_check ();

  let%lwt (exit_status, telemetry) =
    match args.mode with
    | MODE_LIST_FILES ->
      let%lwt (infol, telemetry) = rpc args @@ Rpc.LIST_FILES_WITH_ERRORS in
      List.iter infol (Printf.printf "%s\n");
      Lwt.return (Exit_status.No_error, telemetry)
    | MODE_COLORING file ->
      let file_input =
        match file with
        | "-" ->
          let content = Sys_utils.read_stdin_to_string () in
          ServerCommandTypes.FileContent content
        | _ ->
          let file = expand_path file in
          ServerCommandTypes.FileName file
      in
      let%lwt (pos_level_l, telemetry) =
        rpc args @@ Rpc.COVERAGE_LEVELS (file, file_input)
      in
      ClientColorFile.go file_input args.output_json pos_level_l;
      Lwt.return (Exit_status.No_error, telemetry)
    | MODE_COVERAGE file ->
      let%lwt (counts_opt, telemetry) =
        rpc args @@ Rpc.COVERAGE_COUNTS (expand_path file)
      in
      ClientCoverageMetric.go ~json:args.output_json counts_opt;
      Lwt.return (Exit_status.No_error, telemetry)
    | MODE_FIND_CLASS_REFS name ->
      let%lwt results =
        rpc_with_retry args
        @@ Rpc.FIND_REFS (ServerCommandTypes.Find_refs.Class name)
      in
      ClientFindRefs.go results args.output_json;
      Lwt.return (Exit_status.No_error, Telemetry.create ())
    | MODE_FIND_REFS name ->
      let open ServerCommandTypes.Find_refs in
      let action =
        parse_function_or_method_id
          ~meth_action:(fun class_name method_name ->
            Member (class_name, Method method_name))
          ~func_action:(fun fun_name -> Function fun_name)
          name
      in
      let%lwt results = rpc_with_retry args @@ Rpc.FIND_REFS action in
      ClientFindRefs.go results args.output_json;
      Lwt.return (Exit_status.No_error, Telemetry.create ())
    | MODE_GEN_HOT_CLASSES (threshold, filename) ->
      let%lwt (content, telemetry) =
        rpc args @@ Rpc.GEN_HOT_CLASSES threshold
      in
      let oc = Stdlib.open_out filename in
      Out_channel.output_string oc content;
      Stdlib.close_out oc;
      Lwt.return (Exit_status.No_error, telemetry)
    | MODE_GO_TO_IMPL_CLASS class_name ->
      let%lwt results =
        rpc_with_retry args
        @@ Rpc.GO_TO_IMPL (ServerCommandTypes.Find_refs.Class class_name)
      in
      ClientFindRefs.go results args.output_json;
      Lwt.return (Exit_status.No_error, Telemetry.create ())
    | MODE_GO_TO_IMPL_CLASS_REMOTE class_name ->
      let results =
        Glean_dependency_graph.go_to_implementation ~class_name ~globalrev:""
      in
      HashSet.iter results ~f:(fun cls -> Printf.printf "%s\n" cls);
      Printf.printf "%d total results\n" (HashSet.length results);
      Lwt.return (Exit_status.No_error, Telemetry.create ())
    | MODE_GO_TO_IMPL_METHOD name ->
      let action =
        parse_function_or_method_id
          ~meth_action:(fun class_name method_name ->
            ServerCommandTypes.Find_refs.Member
              (class_name, ServerCommandTypes.Find_refs.Method method_name))
          ~func_action:(fun fun_name ->
            ServerCommandTypes.Find_refs.Function fun_name)
          name
      in
      (match action with
      | ServerCommandTypes.Find_refs.Member _ ->
        let%lwt results = rpc_with_retry args @@ Rpc.GO_TO_IMPL action in
        ClientFindRefs.go results args.output_json;
        Lwt.return (Exit_status.No_error, Telemetry.create ())
      | _ ->
        Printf.eprintf "Invalid input\n";
        Lwt.return (Exit_status.Input_error, Telemetry.create ()))
    | MODE_IDE_FIND_REFS arg ->
      let (line, char) = parse_position_string arg in
      let include_defs = false in
      let content = Sys_utils.read_stdin_to_string () in
      let labelled_file =
        ServerCommandTypes.LabelledFileContent { filename = ""; content }
      in
      let%lwt results =
        rpc_with_retry args
        @@ Rpc.IDE_FIND_REFS (labelled_file, line, char, include_defs)
      in
      ClientFindRefs.go_ide results args.output_json;
      Lwt.return (Exit_status.No_error, Telemetry.create ())
    | MODE_IDE_HIGHLIGHT_REFS arg ->
      let (line, char) = parse_position_string arg in
      let content =
        ServerCommandTypes.FileContent (Sys_utils.read_stdin_to_string ())
      in
      let%lwt (results, telemetry) =
        rpc args @@ Rpc.IDE_HIGHLIGHT_REFS ("", content, line, char)
      in
      ClientHighlightRefs.go results ~output_json:args.output_json;
      Lwt.return (Exit_status.No_error, telemetry)
    | MODE_DUMP_SYMBOL_INFO files ->
      let%lwt conn = connect args in
      let%lwt () = ClientSymbolInfo.go conn ~desc:args.desc files expand_path in
      Lwt.return (Exit_status.No_error, Telemetry.create ())
    | MODE_REFACTOR (ref_mode, before, after) ->
      let conn () = connect args in
      let%lwt () =
        ClientRefactor.go conn ~desc:args.desc args ref_mode before after
      in
      Lwt.return (Exit_status.No_error, Telemetry.create ())
    | MODE_IDE_REFACTOR arg ->
      let conn () = connect args in
      let tpos = Str.split (Str.regexp ":") arg in
      let (filename, line, char, new_name) =
        try
          match tpos with
          | [filename; line; char; new_name] ->
            let filename = expand_path filename in
            (filename, int_of_string line, int_of_string char, new_name)
          | _ -> raise Exit
        with _ ->
          Printf.eprintf "Invalid input\n";
          raise Exit_status.(Exit_with Input_error)
      in
      let%lwt () =
        ClientRefactor.go_ide
          conn
          ~desc:args.desc
          args
          filename
          line
          char
          new_name
      in
      Lwt.return (Exit_status.No_error, Telemetry.create ())
    | MODE_EXTRACT_STANDALONE name ->
      let open ServerCommandTypes.Extract_standalone in
      let target =
        parse_function_or_method_id
          ~meth_action:(fun class_name method_name ->
            Method (class_name, method_name))
          ~func_action:(fun fun_name -> Function fun_name)
          name
      in
      let%lwt (pretty_printed_dependencies, telemetry) =
        rpc args @@ Rpc.EXTRACT_STANDALONE target
      in
      print_endline pretty_printed_dependencies;
      Lwt.return (Exit_status.No_error, telemetry)
    | MODE_CONCATENATE_ALL ->
      let paths = filter_real_paths args.paths in
      let%lwt (single_file, telemetry) =
        rpc args @@ Rpc.CONCATENATE_ALL paths
      in
      print_endline single_file;
      Lwt.return (Exit_status.No_error, telemetry)
    | MODE_IDENTIFY_SYMBOL arg ->
      if not args.output_json then begin
        Printf.eprintf "Must use --json\n%!";
        raise Exit_status.(Exit_with Input_error)
      end;
      let%lwt (result, telemetry) = rpc args @@ Rpc.IDENTIFY_SYMBOL arg in
      let definition_to_json (d : string SymbolDefinition.t) : Hh_json.json =
        Hh_json.JSON_Object
          [
            ("full_name", d.SymbolDefinition.full_name |> Hh_json.string_);
            ("pos", d.SymbolDefinition.pos |> Pos.json);
            ( "kind",
              d.SymbolDefinition.kind
              |> SymbolDefinition.string_of_kind
              |> Hh_json.string_ );
          ]
      in
      let definitions = List.map result ~f:definition_to_json in
      Hh_json.json_to_string (Hh_json.JSON_Array definitions) |> print_endline;
      Lwt.return (Exit_status.No_error, telemetry)
    | MODE_IDENTIFY_SYMBOL1 arg
    | MODE_IDENTIFY_SYMBOL2 arg
    | MODE_IDENTIFY_SYMBOL3 arg ->
      let (line, char) = parse_position_string arg in
      let file =
        match args.stdin_name with
        | None -> ""
        | Some f -> expand_path f
      in
      let content =
        ServerCommandTypes.FileContent (Sys_utils.read_stdin_to_string ())
      in
      let%lwt (result, telemetry) =
        rpc args @@ Rpc.IDENTIFY_FUNCTION (file, content, line, char)
      in
      ClientGetDefinition.go result args.output_json;
      Lwt.return (Exit_status.No_error, telemetry)
    | MODE_TYPE_AT_POS arg ->
      let tpos = Str.split (Str.regexp ":") arg in
      let (fn, line, char) =
        try
          match tpos with
          | [filename; line; char] ->
            let fn = expand_path filename in
            ( ServerCommandTypes.FileName fn,
              int_of_string line,
              int_of_string char )
          | [line; char] ->
            let content = Sys_utils.read_stdin_to_string () in
            ( ServerCommandTypes.FileContent content,
              int_of_string line,
              int_of_string char )
          | _ -> raise Exit
        with _ ->
          Printf.eprintf "Invalid position\n";
          raise Exit_status.(Exit_with Input_error)
      in
      let%lwt (ty, telemetry) =
        rpc args @@ Rpc.INFER_TYPE (fn, line, char, args.dynamic_view)
      in
      ClientTypeAtPos.go ty args.output_json;
      Lwt.return (Exit_status.No_error, telemetry)
    | MODE_TYPE_AT_POS_BATCH positions ->
      let positions =
        List.map positions (fun pos ->
            try
              match Str.split (Str.regexp ":") pos with
              | [filename; line; char] ->
                ( expand_path filename,
                  int_of_string line,
                  int_of_string char,
                  None )
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
              raise Exit_status.(Exit_with Input_error))
      in
      let%lwt (responses, telemetry) =
        rpc args @@ Rpc.INFER_TYPE_BATCH (positions, args.dynamic_view)
      in
      List.iter responses print_endline;
      Lwt.return (Exit_status.No_error, telemetry)
    | MODE_TYPE_ERROR_AT_POS arg ->
      let tpos = Str.split (Str.regexp ":") arg in
      let (fn, line, char) =
        try
          match tpos with
          | [filename; line; char] ->
            let fn = expand_path filename in
            ( ServerCommandTypes.FileName fn,
              int_of_string line,
              int_of_string char )
          | [line; char] ->
            let content = Sys_utils.read_stdin_to_string () in
            ( ServerCommandTypes.FileContent content,
              int_of_string line,
              int_of_string char )
          | _ -> raise Exit
        with _ ->
          Printf.eprintf
            "Invalid position; expected an argument of the form [filename]:[line]:[column] or [line]:[column]\n";
          raise Exit_status.(Exit_with Input_error)
      in
      let%lwt (ty, telemetry) =
        rpc args @@ Rpc.INFER_TYPE_ERROR (fn, line, char)
      in
      ClientTypeErrorAtPos.go ty args.output_json;
      Lwt.return (Exit_status.No_error, telemetry)
    | MODE_FUN_DEPS_AT_POS_BATCH positions ->
      let positions = parse_positions positions in
      let%lwt (responses, telemetry) =
        rpc args @@ Rpc.FUN_DEPS_BATCH (positions, args.dynamic_view)
      in
      List.iter responses print_endline;
      Lwt.return (Exit_status.No_error, telemetry)
    | MODE_AUTO_COMPLETE ->
      let content = Sys_utils.read_stdin_to_string () in
      let%lwt (results, telemetry) =
        rpc args @@ Rpc.COMMANDLINE_AUTOCOMPLETE content
      in
      ClientAutocomplete.go results args.output_json;
      Lwt.return (Exit_status.No_error, telemetry)
    | MODE_OUTLINE
    | MODE_OUTLINE2 ->
      let (_handle : SharedMem.handle) =
        SharedMem.init ~num_workers:0 SharedMem.default_config
      in
      let content = Sys_utils.read_stdin_to_string () in
      let results =
        FileOutline.outline
          (*
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
          ParserOptions.default
          content
      in
      ClientOutline.go results args.output_json;
      Lwt.return (Exit_status.No_error, Telemetry.create ())
    | MODE_METHOD_JUMP_CHILDREN class_ ->
      let filter = ServerCommandTypes.Method_jumps.No_filter in
      let%lwt (results, telemetry) =
        rpc args @@ Rpc.METHOD_JUMP (class_, filter, true)
      in
      ClientMethodJumps.go results true args.output_json;
      Lwt.return (Exit_status.No_error, telemetry)
    | MODE_METHOD_JUMP_ANCESTORS (class_, filter) ->
      let filter =
        match MethodJumps.string_filter_to_method_jump_filter filter with
        | Some filter -> filter
        | None ->
          Printf.eprintf "Invalid method jump filter %s\n" filter;
          raise Exit_status.(Exit_with Input_error)
      in
      let%lwt (results, telemetry) =
        rpc args @@ Rpc.METHOD_JUMP (class_, filter, false)
      in
      ClientMethodJumps.go results false args.output_json;
      Lwt.return (Exit_status.No_error, telemetry)
    | MODE_METHOD_JUMP_ANCESTORS_BATCH (classes, filter) ->
      let filter =
        match MethodJumps.string_filter_to_method_jump_filter filter with
        | Some filter -> filter
        | None ->
          Printf.eprintf "Invalid method jump filter %s\n" filter;
          raise Exit_status.(Exit_with Input_error)
      in
      let%lwt (results, telemetry) =
        rpc args @@ Rpc.METHOD_JUMP_BATCH (classes, filter)
      in
      ClientMethodJumps.go results false args.output_json;
      Lwt.return (Exit_status.No_error, telemetry)
    | MODE_IN_MEMORY_DEP_TABLE_SIZE ->
      let%lwt (result, telemetry) = rpc args @@ Rpc.IN_MEMORY_DEP_TABLE_SIZE in
      ClientResultPrinter.Int_printer.go result args.output_json;
      Lwt.return (Exit_status.No_error, telemetry)
    | MODE_SAVE_NAMING path ->
      let () = Sys_utils.mkdir_p (Filename.dirname path) in
      let path = Path.make path in
      let%lwt (result, telemetry) =
        rpc args @@ Rpc.SAVE_NAMING (Path.to_string path)
      in
      SaveNamingResultPrinter.go result args.output_json;
      Lwt.return (Exit_status.No_error, telemetry)
    | MODE_SAVE_STATE path ->
      let () = Sys_utils.mkdir_p (Filename.dirname path) in
      (* Convert to real path because Client and Server may have
       * different cwd and we want to use the client processes' cwd. *)
      let path = Path.make path in
      let%lwt (result, telemetry) =
        rpc args
        @@ Rpc.SAVE_STATE
             ( Path.to_string path,
               args.gen_saved_ignore_type_errors,
               args.replace_state_after_saving )
      in
      SaveStateResultPrinter.go result args.output_json;
      Lwt.return (Exit_status.No_error, telemetry)
    | MODE_STATUS ->
      let ignore_ide = ClientMessages.ignore_ide_from args.from in
      let%lwt ((), telemetry1) =
        match args.prechecked with
        | Some false -> rpc args Rpc.NO_PRECHECKED_FILES
        | _ -> Lwt.return ((), Telemetry.create ())
      in
      let%lwt (status, telemetry) =
        rpc
          args
          (Rpc.STATUS
             { ignore_ide; max_errors = args.max_errors; remote = args.remote })
      in
      let exit_status =
        ClientCheckStatus.go
          status
          args.output_json
          args.from
          args.error_format
          args.max_errors
      in
      Lwt.return
        ( exit_status,
          Telemetry.object_ telemetry ~key:"no_prechecked" ~value:telemetry1 )
    | MODE_STATUS_SINGLE filename ->
      let file_input =
        match filename with
        | "-" ->
          ServerCommandTypes.FileContent (Sys_utils.read_stdin_to_string ())
        | _ -> ServerCommandTypes.FileName (expand_path filename)
      in
      let%lwt ((error_list, dropped_count), telemetry) =
        rpc args (Rpc.STATUS_SINGLE (file_input, args.max_errors))
      in
      let status =
        {
          error_list;
          dropped_count;
          Rpc.Server_status.liveness = Rpc.Live_status;
          has_unsaved_changes = false;
          last_recheck_stats = None;
        }
      in
      let exit_status =
        ClientCheckStatus.go
          status
          args.output_json
          args.from
          args.error_format
          args.max_errors
      in
      Lwt.return (exit_status, telemetry)
    | MODE_SEARCH (query, type_) ->
      let%lwt (results, telemetry) = rpc args @@ Rpc.SEARCH (query, type_) in
      ClientSearch.go results args.output_json;
      Lwt.return (Exit_status.No_error, telemetry)
    | MODE_LINT ->
      let fnl = filter_real_paths args.paths in
      begin
        match args.paths with
        | [] ->
          prerr_endline "No lint errors (0 files checked)!";
          prerr_endline "Note: --lint expects a list of filenames to check.";
          Lwt.return (Exit_status.No_error, Telemetry.create ())
        | _ ->
          let%lwt (results, telemetry) = rpc args @@ Rpc.LINT fnl in
          ClientLint.go results args.output_json args.error_format;
          Lwt.return (Exit_status.No_error, telemetry)
      end
    | MODE_SERVER_RAGE ->
      let open ServerRageTypes in
      let open Hh_json in
      if not args.output_json then begin
        Printf.eprintf "Must use --json\n%!";
        raise Exit_status.(Exit_with Input_error)
      end;
      (* Our json output format is read by clientRage.ml *)
      let make_item { title; data } =
        JSON_Object
          [("name", JSON_String title); ("contents", JSON_String data)]
      in
      let%lwt (items, telemetry) = rpc args Rpc.RAGE in
      json_to_string (JSON_Array (List.map items ~f:make_item)) |> print_endline;
      Lwt.return (Exit_status.No_error, telemetry)
    | MODE_LINT_STDIN filename ->
      begin
        match Sys_utils.realpath filename with
        | None ->
          prerr_endlinef "Could not find file '%s'" filename;
          Lwt.return (Exit_status.Input_error, Telemetry.create ())
        | Some filename ->
          let contents = Sys_utils.read_stdin_to_string () in
          let%lwt (results, telemetry) =
            rpc args @@ Rpc.LINT_STDIN { ServerCommandTypes.filename; contents }
          in
          ClientLint.go results args.output_json args.error_format;
          Lwt.return (Exit_status.No_error, telemetry)
      end
    | MODE_LINT_ALL code ->
      let%lwt (results, telemetry) = rpc args @@ Rpc.LINT_ALL code in
      ClientLint.go results args.output_json args.error_format;
      Lwt.return (Exit_status.No_error, telemetry)
    | MODE_LINT_XCONTROLLER filename ->
      begin
        try
          match Sys_utils.realpath filename with
          | None ->
            prerr_endlinef "Could not find file '%s'" filename;
            raise Exit
          | Some filename ->
            let files =
              Sys_utils.cat_no_fail filename
              |> Sys_utils.split_lines
              |> List.filter_map ~f:(fun filename ->
                     let res = Sys_utils.realpath filename in
                     if Option.is_none res then (
                       prerr_endlinef "Could not find file '%s'" filename;
                       raise Exit
                     );
                     res)
            in
            let%lwt (results, telemetry) =
              rpc args @@ Rpc.LINT_XCONTROLLER files
            in
            ClientLint.go results args.output_json args.error_format;
            Lwt.return (Exit_status.No_error, telemetry)
        with Exit -> Lwt.return (Exit_status.Input_error, Telemetry.create ())
      end
    | MODE_CREATE_CHECKPOINT x ->
      let%lwt ((), telemetry) = rpc args @@ Rpc.CREATE_CHECKPOINT x in
      Lwt.return (Exit_status.No_error, telemetry)
    | MODE_RETRIEVE_CHECKPOINT x ->
      let%lwt (results, telemetry) = rpc args @@ Rpc.RETRIEVE_CHECKPOINT x in
      begin
        match results with
        | Some results ->
          List.iter results print_endline;
          Lwt.return (Exit_status.No_error, telemetry)
        | None -> Lwt.return (Exit_status.Checkpoint_error, telemetry)
      end
    | MODE_DELETE_CHECKPOINT x ->
      let%lwt (results, telemetry) = rpc args @@ Rpc.DELETE_CHECKPOINT x in
      let exit_status =
        if results then
          Exit_status.No_error
        else
          Exit_status.Checkpoint_error
      in
      Lwt.return (exit_status, telemetry)
    | MODE_STATS ->
      let%lwt (stats, telemetry) = rpc args @@ Rpc.STATS in
      print_string @@ Hh_json.json_to_multiline (Stats.to_json stats);
      Lwt.return (Exit_status.No_error, telemetry)
    | MODE_REMOVE_DEAD_FIXMES codes ->
      (* we need to confirm that the server is not already started
       * in a non-no-load (yes-load) state
       *)
      let%lwt conn = connect args in
      let%lwt (response, telemetry) =
        ClientConnect.rpc conn ~desc:args.desc @@ Rpc.REMOVE_DEAD_FIXMES codes
      in
      begin
        match response with
        | `Error msg ->
          Printf.eprintf "%s\n" msg;
          Lwt.return (Exit_status.Type_error, telemetry)
        | `Ok patches ->
          if args.output_json then
            print_patches_json patches
          else
            apply_patches patches;
          Lwt.return (Exit_status.No_error, telemetry)
      end
    | MODE_REWRITE_LAMBDA_PARAMETERS files ->
      let%lwt conn = connect args in
      let%lwt (patches, telemetry) =
        ClientConnect.rpc conn ~desc:args.desc
        @@ Rpc.REWRITE_LAMBDA_PARAMETERS files
      in
      if args.output_json then
        print_patches_json patches
      else
        apply_patches patches;
      Lwt.return (Exit_status.No_error, telemetry)
    | MODE_REWRITE_TYPE_PARAMS_TYPE files ->
      let%lwt conn = connect args in
      let%lwt (patches, telemetry) =
        ClientConnect.rpc conn ~desc:args.desc
        @@ Rpc.REWRITE_TYPE_PARAMS_TYPE files
      in
      if args.output_json then
        print_patches_json patches
      else
        apply_patches patches;
      Lwt.return (Exit_status.No_error, telemetry)
    | MODE_FORMAT (from, to_) ->
      let content = Sys_utils.read_stdin_to_string () in
      let%lwt (result, telemetry) =
        rpc args @@ Rpc.FORMAT (content, from, to_)
      in
      ClientFormat.go result args.output_json;
      Lwt.return (Exit_status.No_error, telemetry)
    | MODE_AI_QUERY json ->
      let%lwt (results, telemetry) = rpc args @@ Rpc.AI_QUERY json in
      ClientAiQuery.go results;
      Lwt.return (Exit_status.No_error, telemetry)
    | MODE_FULL_FIDELITY_PARSE file ->
      (* We can cheaply do this on the client today, but we might want to
      do it on the server and cache the results in the future. *)
      let do_it_on_server = false in
      let%lwt (results, telemetry) =
        if do_it_on_server then
          rpc args @@ Rpc.DUMP_FULL_FIDELITY_PARSE file
        else
          let file = Relative_path.create Relative_path.Dummy file in
          let source_text = Full_fidelity_source_text.from_file file in
          let syntax_tree = SyntaxTree.make source_text in
          let json = SyntaxTree.to_json syntax_tree in
          Lwt.return (Hh_json.json_to_string json, Telemetry.create ())
      in
      ClientFullFidelityParse.go results;
      Lwt.return (Exit_status.No_error, telemetry)
    | MODE_FULL_FIDELITY_SCHEMA ->
      let schema = Full_fidelity_schema.schema_as_json () in
      print_string schema;
      Lwt.return (Exit_status.No_error, Telemetry.create ())
    | MODE_CST_SEARCH files_to_search ->
      let sort_results = args.sort_results in
      let input = Sys_utils.read_stdin_to_string () |> Hh_json.json_of_string in
      let%lwt (result, telemetry) =
        rpc args @@ Rpc.CST_SEARCH { Rpc.sort_results; input; files_to_search }
      in
      begin
        match result with
        | Ok result ->
          print_endline (Hh_json.json_to_string result);
          Lwt.return (Exit_status.No_error, telemetry)
        | Error error ->
          print_endline error;
          raise Exit_status.(Exit_with Input_error)
      end
    | MODE_FILE_DEPENDENTS ->
      let paths = filter_real_paths args.paths in
      let%lwt (responses, telemetry) = rpc args @@ Rpc.FILE_DEPENDENTS paths in
      if args.output_json then
        Hh_json.(
          let json_path_list =
            List.map responses ~f:(fun path -> JSON_String path)
          in
          let output =
            JSON_Object [("dependents", JSON_Array json_path_list)]
          in
          print_endline @@ json_to_string output)
      else
        List.iter responses ~f:(Printf.printf "%s\n");
      Lwt.return (Exit_status.No_error, telemetry)
    | MODE_BIGCODE arg ->
      let filename = arg in
      let fn =
        try expand_path filename
        with _ ->
          Printf.eprintf "Invalid filename: %s\n" filename;
          raise Exit_status.(Exit_with Input_error)
      in
      let%lwt (result, telemetry) = rpc args @@ Rpc.BIGCODE fn in
      print_endline result;
      Lwt.return (Exit_status.No_error, telemetry)
    | MODE_PAUSE pause ->
      let%lwt ((), telemetry) = rpc args @@ Rpc.PAUSE pause in
      if pause then (
        print_endline
          "Paused the automatic triggering of full checks upon file changes.";
        print_endline "To manually trigger a full check, do `hh check`.";
        print_endline "***PAUSE MODE CURRENTLY GETS RESET UPON REBASES.***";
        print_endline "***PAUSE MODE IS EXPERIMENTAL. USE AT YOUR OWN RISK.***"
      ) else
        print_endline
          "Resumed the automatic triggering of full checks upon file changes.";
      Lwt.return (Exit_status.No_error, telemetry)
    | MODE_GLOBAL_INFERENCE (submode, files) ->
      let%lwt conn = connect args in
      let%lwt (results, telemetry) =
        ClientConnect.rpc conn ~desc:args.desc
        @@ Rpc.GLOBAL_INFERENCE (submode, files)
      in
      (match results with
      | ServerGlobalInferenceTypes.RError error -> print_endline error
      | ServerGlobalInferenceTypes.RRewrite patches ->
        if args.output_json then
          print_patches_json patches
        else
          apply_patches patches
      | _ -> ());
      Lwt.return (Exit_status.No_error, telemetry)
    | MODE_VERBOSE verbose ->
      let%lwt ((), telemetry) = rpc args @@ Rpc.VERBOSE verbose in
      Lwt.return (Exit_status.No_error, telemetry)
  in
  HackEventLogger.client_check_finish exit_status telemetry;
  Lwt.return exit_status
