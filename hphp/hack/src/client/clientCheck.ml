(*
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Hh_prelude
open Ocaml_overrides
module SyntaxTree =
  Full_fidelity_syntax_tree.WithSyntax (Full_fidelity_positioned_syntax)

module SaveStateResultPrinter = ClientResultPrinter.Make (struct
  type t = SaveStateServiceTypes.save_state_result

  let to_string t =
    Printf.sprintf
      "Dependency table edges added: %d"
      t.SaveStateServiceTypes.dep_table_edges_added

  let to_json (t : SaveStateServiceTypes.save_state_result) =
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

let print_refs (results : (string * Pos.absolute) list) ~(json : bool) : unit =
  if json then
    FindRefsWireFormat.HackAst.to_string results |> print_endline
  else
    FindRefsWireFormat.CliHumanReadable.print_results results

let print_find_my_tests_result result ~(json : bool) : unit =
  let module FMT = ServerCommandTypes.Find_my_tests in
  if json then
    let result_obj =
      Hh_json.(
        JSON_Array
          (List.map result ~f:(fun (entry : FMT.result_entry) ->
               JSON_Object
                 [
                   ("file_path", JSON_String entry.FMT.file_path);
                   ("distance", JSON_Number (Int.to_string entry.FMT.distance));
                 ])))
    in
    print_endline (Hh_json.json_to_multiline result_obj)
  else
    List.iter result ~f:(fun file -> print_endline file.FMT.file_path)

let parse_name_or_member_id ~name_only_action ~name_and_member_action name =
  let pieces = Str.split (Str.regexp "::") name in
  let default_namespace str =
    match Str.first_chars str 1 with
    | "\\" -> str
    | _ -> "\\" ^ str
  in
  try
    match pieces with
    | class_name :: member_name :: _ ->
      name_and_member_action (default_namespace class_name) member_name
    | name :: _ -> name_only_action (default_namespace name)
    | _ -> raise Exit
  with
  | _ ->
    Printf.eprintf "Invalid input\n";
    raise Exit_status.(Exit_with Input_error)

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

let parse_position_string ~(split_on : string) arg =
  let tpos = Str.split (Str.regexp split_on) arg in
  try
    match tpos with
    | [line; char] -> (int_of_string line, int_of_string char)
    | _ -> raise Exit
  with
  | _ ->
    Printf.eprintf "Invalid position\n";
    raise Exit_status.(Exit_with Input_error)

let connect
    ?(use_priority_pipe = false) ?(do_post_handoff_handshake = true) args :
    ClientConnect.conn Lwt.t =
  let {
    ClientEnv.root;
    from;
    autostart;
    force_dormant_start;
    deadline;
    no_load;
    watchman_debug_logging;
    show_spinner;
    ignore_hh_version;
    save_64bit;
    save_human_readable_64bit_dep_map;
    saved_state_ignore_hhconfig;
    prechecked;
    mini_state;
    config;
    allow_non_opt_build;
    custom_hhi_path;
    custom_telemetry_data;
    preexisting_warnings;
    error_format = _;
    gen_saved_ignore_type_errors = _;
    paths = _;
    max_errors = _;
    mode = _;
    output_json = _;
    sort_results = _;
    stdin_name = _;
    desc = _;
    is_interactive = _;
    warning_switches = _;
    dump_config = _;
    find_my_tests_max_distance = _;
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
        progress_callback =
          ClientSpinner.report
            ~to_stderr:show_spinner
            ~angery_reaccs_only:(ClientMessages.angery_reaccs_only ());
        do_post_handoff_handshake;
        ignore_hh_version;
        save_64bit;
        save_human_readable_64bit_dep_map;
        saved_state_ignore_hhconfig;
        use_priority_pipe;
        prechecked;
        mini_state;
        config;
        custom_hhi_path;
        custom_telemetry_data;
        allow_non_opt_build;
        preexisting_warnings;
      })

(* This is a function, because server closes the connection after each command,
 * so we need to be able to reconnect to retry. *)
type connect_fun = unit -> ClientConnect.conn Lwt.t

let connect_then_close (args : ClientEnv.client_check_env) : unit Lwt.t =
  let%lwt ClientConnect.{ channels = (_ic, oc); _ } =
    connect args ~use_priority_pipe:true ~do_post_handoff_handshake:false
  in
  Out_channel.close oc;
  (* The connection is derived from [Unix.open_connection]. Its docs explain:
     "The two channels returned by [open_connection] share a descriptor
      to a socket.  Therefore, when the connection is over, you should
      call {!Stdlib.close_out} on the output channel, which will also close
      the underlying socket.  Do not call {!Stdlib.close_in} on the input
      channel; it will be collected by the GC eventually." *)
  Lwt.return_unit

let rpc_with_connection
    (args : ClientEnv.client_check_env)
    (command : 'a ServerCommandTypes.t)
    (call : connect_fun -> desc:string -> 'a ServerCommandTypes.t -> 'b Lwt.t) :
    'b Lwt.t =
  let use_priority_pipe = ServerCommandTypes.use_priority_pipe command in
  let conn () = connect args ~use_priority_pipe in
  let%lwt result = call conn ~desc:args.desc @@ command in
  Lwt.return result

let rpc_with_retry
    (args : ClientEnv.client_check_env)
    (command : 'a ServerCommandTypes.Done_or_retry.t ServerCommandTypes.t) :
    'a Lwt.t =
  let%lwt result =
    rpc_with_connection args command ClientConnect.rpc_with_retry
  in
  Lwt.return result

let rpc_with_retry_list
    (args : ClientEnv.client_check_env)
    (command : 'a ServerCommandTypes.Done_or_retry.t list ServerCommandTypes.t)
    : 'a list Lwt.t =
  let%lwt result =
    rpc_with_connection args command ClientConnect.rpc_with_retry_list
  in
  Lwt.return result

let rpc
    (args : ClientEnv.client_check_env) (command : 'result ServerCommandTypes.t)
    : ('result * Telemetry.t) Lwt.t =
  rpc_with_connection args command (fun conn_f ~desc command ->
      let%lwt conn = conn_f () in
      let%lwt (result, telemetry) = ClientConnect.rpc conn ~desc command in
      Lwt.return (result, telemetry))

let parse_positions positions =
  List.map positions ~f:(fun pos ->
      try
        match Str.split (Str.regexp ":") pos with
        | [filename; line; char] ->
          (expand_path filename, int_of_string line, int_of_string char)
        | _ -> raise Exit
      with
      | _ ->
        Printf.eprintf "Invalid position\n";
        raise Exit_status.(Exit_with Input_error))

(* Filters and prints errors when a path is not a realpath *)
let filter_real_paths ~allow_directories paths =
  List.filter_map paths ~f:(fun fn ->
      match Sys_utils.realpath fn with
      | Some path ->
        if (not allow_directories) && Disk.is_directory fn then begin
          Utils.prerr_endlinef
            "Path is a directory, only files are allowed: '%s'"
            fn;
          None
        end else
          Some path
      | None ->
        Utils.prerr_endlinef "Could not find file '%s'" fn;
        None)

let main_internal
    (args : ClientEnv.client_check_env)
    (config : ServerConfig.t)
    (local_config : ServerLocalConfig.t)
    (partial_telemetry_ref : Telemetry.t option ref) :
    (Exit_status.t * Telemetry.t) Lwt.t =
  match args.mode with
  | ClientEnv.MODE_STATUS ->
    let prechecked = Option.value args.prechecked ~default:true in
    let%lwt ((), telemetry1) =
      if prechecked then
        Lwt.return ((), Telemetry.create ())
      else
        rpc args ServerCommandTypes.NO_PRECHECKED_FILES
    in
    let error_filter =
      Filter_errors.Filter.make
        ~default_all:local_config.ServerLocalConfig.warnings_default_all
        ~generated_files:(ServerConfig.warnings_generated_files config)
        args.warning_switches
    in
    (* We don't do streaming errors under [output_json]: our contract
       with the outside world is that if a caller uses [output_json] then they
       will never see [Exit_status.Typecheck_restarted], which streaming might show.

       We don't do streaming errors under [not prechecked]. That's because the
       [go_streaming] contract is to report on a typecheck that reflects all *file*
       changes up until now; it has no guarantee that the typecheck will reflects our
       preceding call to ServerCommandTypes.NO_PRECHECKED_FILES. *)
    let use_streaming =
      local_config.ServerLocalConfig.consume_streaming_errors
      && (not args.output_json)
      && prechecked
      && not (Sandcastle.is_sandcastle ())
    in
    if use_streaming then
      ClientCheckStatus.go_streaming
        args
        local_config
        error_filter
        ~partial_telemetry_ref
        ~connect_then_close:(fun () -> connect_then_close args)
    else
      let%lwt (status, telemetry) =
        rpc
          args
          (ServerCommandTypes.STATUS
             { max_errors = args.max_errors; error_filter })
      in
      let exit_status =
        ClientCheckStatus.go
          status
          args.error_format
          ~output_json:args.output_json
          ~max_errors:args.max_errors
          ~is_interactive:args.is_interactive
      in
      let telemetry =
        telemetry
        |> Telemetry.bool_ ~key:"streaming" ~value:false
        |> Telemetry.object_ ~key:"no_prechecked" ~value:telemetry1
        |> Telemetry.object_opt
             ~key:"last_recheck_stats"
             ~value:status.ServerCommandTypes.Server_status.last_recheck_stats
      in
      Lwt.return (exit_status, telemetry)
  | ClientEnv.(
      MODE_STATUS_SINGLE { filenames; show_tast; preexisting_warnings }) ->
    let file_input filename =
      match filename with
      | "-" ->
        ServerCommandTypes.FileContent (Sys_utils.read_stdin_to_string ())
      | _ -> ServerCommandTypes.FileName (expand_path filename)
    in
    let file_inputs = List.map ~f:file_input filenames in
    let error_filter =
      Filter_errors.Filter.make
        ~default_all:local_config.warnings_default_all
        ~generated_files:(ServerConfig.warnings_generated_files config)
        args.warning_switches
    in
    let%lwt (((error_list, dropped_count), tasts), telemetry) =
      rpc
        args
        (ServerCommandTypes.STATUS_SINGLE
           {
             file_names = file_inputs;
             max_errors = args.max_errors;
             error_filter;
             return_expanded_tast = show_tast;
             preexisting_warnings;
           })
    in
    (match tasts with
    | None -> ()
    | Some tasts ->
      Printf.printf "TAST hashes:\n\n";
      Relative_path.Map.map tasts ~f:Tast.program_by_names
      |> Tast_hashes.hash_tasts_by_file
      |> Relative_path.Map.yojson_of_t Tast_hashes.yojson_of_by_names
      |> Yojson.Safe.pretty_to_channel Stdlib.stdout;
      Printf.printf
        "\n\n\nTASTs:\n\n%s\n%!"
        (Relative_path.Map.show (Tast_with_dynamic.pp Tast.pp_program) tasts);
      ());

    let status =
      {
        error_list;
        dropped_count;
        ServerCommandTypes.Server_status.liveness =
          ServerCommandTypes.Live_status;
        last_recheck_stats = None;
        file_watcher_clock = None;
      }
    in
    let exit_status =
      ClientCheckStatus.go
        status
        args.error_format
        ~is_interactive:args.is_interactive
        ~output_json:args.output_json
        ~max_errors:args.max_errors
    in
    Lwt.return (exit_status, telemetry)
  | ClientEnv.MODE_LIST_FILES ->
    let%lwt (infol, telemetry) =
      rpc args @@ ServerCommandTypes.LIST_FILES_WITH_ERRORS
    in
    List.iter infol ~f:(Printf.printf "%s\n");
    Lwt.return (Exit_status.No_error, telemetry)
  | ClientEnv.MODE_FIND_CLASS_REFS name ->
    let%lwt results =
      rpc_with_retry args
      @@ ServerCommandTypes.FIND_REFS (ServerCommandTypes.Find_refs.Class name)
    in
    print_refs results ~json:args.output_json;
    Lwt.return (Exit_status.No_error, Telemetry.create ())
  | ClientEnv.MODE_FIND_REFS name ->
    let open ServerCommandTypes.Find_refs in
    let pieces = Str.split (Str.regexp "|") name in
    let (kind, name) =
      match pieces with
      | [name] -> (None, name)
      | [kind; name] -> (Some kind, name)
      | _ ->
        Printf.eprintf "Invalid input\n";
        raise Exit_status.(Exit_with Input_error)
    in
    let action =
      parse_name_or_member_id
        ~name_and_member_action:(fun class_name member_name ->
          let member =
            match kind with
            | Some "Method"
            | None ->
              Method member_name
            | Some "Property" -> Property member_name
            | Some "Class_const" -> Class_const member_name
            | Some "Typeconst" -> Typeconst member_name
            | Some _ -> raise Exit_status.(Exit_with Input_error)
          in
          Member (class_name, member))
        ~name_only_action:(fun name ->
          match kind with
          | Some "Function"
          | None ->
            Function name
          | Some "Class" -> Class name
          | Some "ExplicitClass" -> ExplicitClass name
          | Some "GConst" -> GConst name
          | Some _ -> raise Exit_status.(Exit_with Input_error))
        name
    in
    let%lwt results =
      rpc_with_retry args @@ ServerCommandTypes.FIND_REFS action
    in
    print_refs results ~json:args.output_json;
    Lwt.return (Exit_status.No_error, Telemetry.create ())
  | ClientEnv.MODE_GO_TO_IMPL_CLASS class_name ->
    let%lwt results =
      rpc_with_retry args
      @@ ServerCommandTypes.GO_TO_IMPL
           (ServerCommandTypes.Find_refs.Class class_name)
    in
    print_refs results ~json:args.output_json;
    Lwt.return (Exit_status.No_error, Telemetry.create ())
  | ClientEnv.MODE_GO_TO_IMPL_METHOD name ->
    let action =
      parse_name_or_member_id
        ~name_and_member_action:(fun class_name method_name ->
          ServerCommandTypes.Find_refs.Member
            (class_name, ServerCommandTypes.Find_refs.Method method_name))
        ~name_only_action:(fun fun_name ->
          ServerCommandTypes.Find_refs.Function fun_name)
        name
    in
    (match action with
    | ServerCommandTypes.Find_refs.Member _ ->
      let%lwt results =
        rpc_with_retry args @@ ServerCommandTypes.GO_TO_IMPL action
      in
      print_refs results ~json:args.output_json;
      Lwt.return (Exit_status.No_error, Telemetry.create ())
    | _ ->
      Printf.eprintf "Invalid input\n";
      Lwt.return (Exit_status.Input_error, Telemetry.create ()))
  | ClientEnv.MODE_HACK_TO_NOTEBOOK ->
    let exit_status = Notebook_convert.hack_to_notebook () in
    Lwt.return (exit_status, Telemetry.create ())
  | ClientEnv.MODE_IDE_FIND_REFS_BY_SYMBOL arg ->
    let%lwt results =
      rpc_with_retry args @@ ServerCommandTypes.IDE_FIND_REFS_BY_SYMBOL arg
    in
    FindRefsWireFormat.IdeShellout.to_string results |> print_endline;
    Lwt.return (Exit_status.No_error, Telemetry.create ())
  | ClientEnv.MODE_IDE_GO_TO_IMPL_BY_SYMBOL arg ->
    let%lwt results =
      rpc_with_retry args @@ ServerCommandTypes.IDE_GO_TO_IMPL_BY_SYMBOL arg
    in
    FindRefsWireFormat.IdeShellout.to_string results |> print_endline;
    Lwt.return (Exit_status.No_error, Telemetry.create ())
  | ClientEnv.MODE_DUMP_SYMBOL_INFO files ->
    let%lwt conn = connect args in
    let%lwt () = ClientSymbolInfo.go conn ~desc:args.desc files expand_path in
    Lwt.return (Exit_status.No_error, Telemetry.create ())
  | ClientEnv.MODE_RENAME ((ref_mode : ClientEnv.rename_mode), before, after) ->
    let conn () = connect args in
    let%lwt () =
      ClientRename.go conn ~desc:args.desc args ref_mode ~before ~after
    in
    Lwt.return (Exit_status.No_error, Telemetry.create ())
  | ClientEnv.MODE_IDE_RENAME_BY_SYMBOL arg ->
    let open ServerCommandTypes in
    let (new_name, action, symbol_definition) = Rename.string_to_args arg in
    let%lwt results =
      rpc_with_retry args
      @@ ServerCommandTypes.IDE_RENAME_BY_SYMBOL
           (action, new_name, symbol_definition)
    in
    begin
      match results with
      | Ok patches ->
        ClientRename.go_ide_from_patches patches args.output_json;
        Lwt.return (Exit_status.No_error, Telemetry.create ())
      | Error _msg -> raise Exit_status.(Exit_with Input_error)
    end
  | ClientEnv.MODE_IDENTIFY_SYMBOL arg ->
    if not args.output_json then begin
      Printf.eprintf "Must use --json\n%!";
      raise Exit_status.(Exit_with Input_error)
    end;
    let%lwt (result, telemetry) =
      rpc args @@ ServerCommandTypes.IDENTIFY_SYMBOL arg
    in
    let definition_to_json (d : string SymbolDefinition.t) : Hh_json.json =
      Hh_json.JSON_Object
        [
          ("full_name", SymbolDefinition.full_name d |> Hh_json.string_);
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
  | ClientEnv.MODE_IDENTIFY_SYMBOL1 arg
  | ClientEnv.MODE_IDENTIFY_SYMBOL2 arg
  | ClientEnv.MODE_IDENTIFY_SYMBOL3 arg ->
    let (line, char) = parse_position_string ~split_on:":" arg in
    let pos = File_content.Position.from_one_based line char in
    let file =
      match args.stdin_name with
      | None -> ""
      | Some f -> expand_path f
    in
    let content =
      ServerCommandTypes.FileContent (Sys_utils.read_stdin_to_string ())
    in
    let%lwt (result, telemetry) =
      rpc args @@ ServerCommandTypes.IDENTIFY_FUNCTION (file, content, pos)
    in
    ClientGetDefinition.go result args.output_json;
    Lwt.return (Exit_status.No_error, telemetry)
  | ClientEnv.MODE_TYPE_AT_POS arg ->
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
      with
      | _ ->
        Printf.eprintf "Invalid position\n";
        raise Exit_status.(Exit_with Input_error)
    in
    let pos = File_content.Position.from_one_based line char in
    let%lwt (ty, telemetry) =
      rpc args @@ ServerCommandTypes.INFER_TYPE (fn, pos)
    in
    ClientTypeAtPos.go ty args.output_json;
    Lwt.return (Exit_status.No_error, telemetry)
  | ClientEnv.MODE_TYPE_AT_POS_BATCH positions ->
    let positions =
      List.map positions ~f:(fun pos ->
          try
            match Str.split (Str.regexp ":") pos with
            | [filename; line; char] ->
              ( expand_path filename,
                File_content.Position.from_one_based
                  (int_of_string line)
                  (int_of_string char),
                None )
            | [filename; start_line; start_char; end_line; end_char] ->
              let filename = expand_path filename in
              let start_line = int_of_string start_line in
              let start_char = int_of_string start_char in
              let end_line = int_of_string end_line in
              let end_char = int_of_string end_char in
              let start_pos =
                File_content.Position.from_one_based start_line start_char
              in
              let end_pos =
                File_content.Position.from_one_based end_line end_char
              in
              (filename, start_pos, Some end_pos)
            | _ -> raise Exit
          with
          | _ ->
            Printf.eprintf "Invalid position\n";
            raise Exit_status.(Exit_with Input_error))
    in
    let%lwt (responses, telemetry) =
      rpc args @@ ServerCommandTypes.INFER_TYPE_BATCH positions
    in
    List.iter responses ~f:print_endline;
    Lwt.return (Exit_status.No_error, telemetry)
  | ClientEnv.MODE_IS_SUBTYPE ->
    let stdin = Sys_utils.read_stdin_to_string () in
    let%lwt (response, telemetry) =
      rpc args @@ ServerCommandTypes.IS_SUBTYPE stdin
    in
    (match response with
    | Ok str ->
      Printf.printf "%s" str;
      Lwt.return (Exit_status.No_error, telemetry)
    | Error str ->
      Printf.eprintf "%s" str;
      raise Exit_status.(Exit_with Input_error))
  | ClientEnv.MODE_TYPE_ERROR_AT_POS arg ->
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
      with
      | _ ->
        Printf.eprintf
          "Invalid position; expected an argument of the form [filename]:[line]:[column] or [line]:[column]\n";
        raise Exit_status.(Exit_with Input_error)
    in
    let%lwt (ty, telemetry) =
      rpc args @@ ServerCommandTypes.INFER_TYPE_ERROR (fn, line, char)
    in
    ClientTypeErrorAtPos.go ty args.output_json;
    Lwt.return (Exit_status.No_error, telemetry)
  | ClientEnv.MODE_TAST_HOLES arg ->
    let parse_hole_filter = function
      | "any" -> Some ServerCommandTypes.Tast_hole.Any
      | "typing" -> Some ServerCommandTypes.Tast_hole.Typing
      | "cast" -> Some ServerCommandTypes.Tast_hole.Cast
      | _ -> None
    in

    let (filename, hole_src_opt) =
      try
        match Str.(split (regexp ":") arg) with
        | [filename; filter_str] ->
          let fn = expand_path filename in
          (match parse_hole_filter filter_str with
          | Some filter -> (ServerCommandTypes.FileName fn, filter)
          | _ -> raise Exit)
        | [part] ->
          (match parse_hole_filter part with
          | Some src_opt ->
            let content = Sys_utils.read_stdin_to_string () in
            (ServerCommandTypes.FileContent content, src_opt)
          | _ ->
            let fn = expand_path part in
            (* No hole source specified; default to `Typing` *)
            (ServerCommandTypes.FileName fn, ServerCommandTypes.Tast_hole.Typing))
        | _ -> raise Exit
      with
      | Exit ->
        Printf.eprintf
          "Invalid argument; expected an argument of the form [filename](:[any|typing|cast])? or [any|typing|cast]\n";
        raise Exit_status.(Exit_with Input_error)
      | exn ->
        let e = Exception.wrap exn in
        Printf.eprintf
          "An unexpected error occurred: %s"
          (Exception.get_ctor_string e);
        Exception.reraise e
    in
    let%lwt (ty, telemetry) =
      rpc args @@ ServerCommandTypes.TAST_HOLES (filename, hole_src_opt)
    in
    ClientTastHoles.go ty ~print_file:false args.output_json;
    Lwt.return (Exit_status.No_error, telemetry)
  | ClientEnv.MODE_TAST_HOLES_BATCH (file : string) ->
    let files =
      expand_path file
      |> Sys_utils.read_file
      |> Bytes.to_string
      |> String.strip
      |> String.split ~on:'\n'
      |> List.map ~f:expand_path
    in
    let%lwt (holes, telemetry) =
      rpc args @@ ServerCommandTypes.TAST_HOLES_BATCH files
    in
    ClientTastHoles.go holes ~print_file:true args.output_json;
    Lwt.return (Exit_status.No_error, telemetry)
  | ClientEnv.MODE_FUN_DEPS_AT_POS_BATCH positions ->
    let positions = parse_positions positions in
    let%lwt (responses, telemetry) =
      rpc args @@ ServerCommandTypes.FUN_DEPS_BATCH positions
    in
    List.iter responses ~f:print_endline;
    Lwt.return (Exit_status.No_error, telemetry)
  | ClientEnv.MODE_DEPS_OUT_AT_POS_BATCH positions ->
    let positions = parse_positions positions in
    let%lwt (responses, telemetry) =
      rpc args @@ ServerCommandTypes.DEPS_OUT_BATCH positions
    in
    List.iter responses ~f:print_endline;
    Lwt.return (Exit_status.No_error, telemetry)
  | ClientEnv.MODE_OUTLINE
  | ClientEnv.MODE_OUTLINE2 ->
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
  | ClientEnv.MODE_METHOD_JUMP_CHILDREN class_ ->
    let filter = ServerCommandTypes.Method_jumps.No_filter in
    let%lwt (results, telemetry) =
      rpc args @@ ServerCommandTypes.METHOD_JUMP (class_, filter, true)
    in
    ClientMethodJumps.go results true args.output_json;
    Lwt.return (Exit_status.No_error, telemetry)
  | ClientEnv.MODE_METHOD_JUMP_ANCESTORS (class_, filter) ->
    let filter =
      match MethodJumps.string_filter_to_method_jump_filter filter with
      | Some filter -> filter
      | None ->
        Printf.eprintf "Invalid method jump filter %s\n" filter;
        raise Exit_status.(Exit_with Input_error)
    in
    let%lwt (results, telemetry) =
      rpc args @@ ServerCommandTypes.METHOD_JUMP (class_, filter, false)
    in
    ClientMethodJumps.go results false args.output_json;
    Lwt.return (Exit_status.No_error, telemetry)
  | ClientEnv.MODE_METHOD_JUMP_ANCESTORS_BATCH (classes, filter) ->
    let filter =
      match MethodJumps.string_filter_to_method_jump_filter filter with
      | Some filter -> filter
      | None ->
        Printf.eprintf "Invalid method jump filter %s\n" filter;
        raise Exit_status.(Exit_with Input_error)
    in
    let%lwt (results, telemetry) =
      rpc args @@ ServerCommandTypes.METHOD_JUMP_BATCH (classes, filter)
    in
    ClientMethodJumps.go results false args.output_json;
    Lwt.return (Exit_status.No_error, telemetry)
  | ClientEnv.MODE_NOTEBOOK_TO_HACK { notebook_number; notebook_header } ->
    let exit_status =
      Notebook_convert.notebook_to_hack ~notebook_number ~header:notebook_header
    in
    Lwt.return (exit_status, Telemetry.create ())
  | ClientEnv.MODE_IN_MEMORY_DEP_TABLE_SIZE ->
    let%lwt (result, telemetry) =
      rpc args @@ ServerCommandTypes.IN_MEMORY_DEP_TABLE_SIZE
    in
    ClientResultPrinter.Int_printer.go result args.output_json;
    Lwt.return (Exit_status.No_error, telemetry)
  | ClientEnv.MODE_SAVE_NAMING path ->
    let () = Sys_utils.mkdir_p (Filename.dirname path) in
    let path = Path.make path in
    let%lwt (result, telemetry) =
      rpc args @@ ServerCommandTypes.SAVE_NAMING (Path.to_string path)
    in
    SaveNamingResultPrinter.go result args.output_json;
    Lwt.return (Exit_status.No_error, telemetry)
  | ClientEnv.MODE_SAVE_STATE path ->
    let () = Sys_utils.mkdir_p (Filename.dirname path) in
    (* Convert to real path because Client and Server may have
     * different cwd and we want to use the client processes' cwd. *)
    let path = Path.make path in
    let%lwt (result, telemetry) =
      rpc args
      @@ ServerCommandTypes.SAVE_STATE
           (Path.to_string path, args.gen_saved_ignore_type_errors)
    in
    SaveStateResultPrinter.go result args.output_json;
    Lwt.return (Exit_status.No_error, telemetry)
  | ClientEnv.MODE_SEARCH query ->
    if not (String.equal query "this_is_just_to_check_liveness_of_hh_server")
    then begin
      prerr_endline
        "Usage: hh --search this_is_just_to_check_liveness_of_hh_server";
      Lwt.return (Exit_status.Input_error, Telemetry.create ())
    end else begin
      let%lwt ((), telemetry) = rpc args @@ ServerCommandTypes.CHECK_LIVENESS in
      if args.output_json then print_endline "[]";
      Lwt.return (Exit_status.No_error, telemetry)
    end
  | ClientEnv.MODE_LINT ->
    let fnl = filter_real_paths ~allow_directories:false args.paths in
    begin
      match args.paths with
      | [] ->
        prerr_endline "No lint errors (0 files checked)!";
        prerr_endline "Note: --lint expects a list of filenames to check.";
        Lwt.return (Exit_status.No_error, Telemetry.create ())
      | _ ->
        let%lwt (results, telemetry) =
          rpc args @@ ServerCommandTypes.LINT fnl
        in
        let error_format = Errors.format_or_default args.error_format in
        ClientLint.go results args.output_json error_format;
        Lwt.return (Exit_status.No_error, telemetry)
    end
  | ClientEnv.MODE_SERVER_RAGE ->
    let open ServerRageTypes in
    let open Hh_json in
    if not args.output_json then begin
      Printf.eprintf "Must use --json\n%!";
      raise Exit_status.(Exit_with Input_error)
    end;
    (* Our json output format is read by clientRage.ml *)
    let make_item { title; data } =
      JSON_Object [("name", JSON_String title); ("contents", JSON_String data)]
    in
    let%lwt (items, telemetry) = rpc args ServerCommandTypes.RAGE in
    json_to_string (JSON_Array (List.map items ~f:make_item)) |> print_endline;
    Lwt.return (Exit_status.No_error, telemetry)
  | ClientEnv.MODE_LINT_STDIN filename -> begin
    match Sys_utils.realpath filename with
    | None ->
      Utils.prerr_endlinef "Could not find file '%s'" filename;
      Lwt.return (Exit_status.Input_error, Telemetry.create ())
    | Some filename ->
      let contents = Sys_utils.read_stdin_to_string () in
      let%lwt (results, telemetry) =
        rpc args
        @@ ServerCommandTypes.LINT_STDIN
             { ServerCommandTypes.filename; contents }
      in
      let error_format = Errors.format_or_default args.error_format in
      ClientLint.go results args.output_json error_format;
      Lwt.return (Exit_status.No_error, telemetry)
  end
  | ClientEnv.MODE_LINT_ALL code ->
    let%lwt (results, telemetry) =
      rpc args @@ ServerCommandTypes.LINT_ALL code
    in
    let error_format = Errors.format_or_default args.error_format in
    ClientLint.go results args.output_json error_format;
    Lwt.return (Exit_status.No_error, telemetry)
  | ClientEnv.MODE_STATS ->
    let%lwt (stats, telemetry) = rpc args @@ ServerCommandTypes.STATS in
    print_string @@ Hh_json.json_to_multiline (Stats.to_json stats);
    Lwt.return (Exit_status.No_error, telemetry)
  | ClientEnv.MODE_REMOVE_DEAD_FIXMES codes ->
    let%lwt conn = connect args in
    let%lwt (response, telemetry) =
      ClientConnect.rpc conn ~desc:args.desc
      @@ ServerCommandTypes.REMOVE_DEAD_FIXMES codes
    in
    begin
      match response with
      | `Error msg ->
        Printf.eprintf "%s\n" msg;
        Lwt.return (Exit_status.Type_error, telemetry)
      | `Ok patches ->
        if args.output_json then
          ClientRename.print_patches_json patches
        else
          ClientRename.apply_patches patches;
        Lwt.return (Exit_status.No_error, telemetry)
    end
  | ClientEnv.MODE_REMOVE_DEAD_UNSAFE_CASTS ->
    let error_filter =
      Filter_errors.Filter.make
        ~default_all:local_config.ServerLocalConfig.warnings_default_all
        ~generated_files:(ServerConfig.warnings_generated_files config)
        args.warning_switches
    in
    let status_cmd =
      ServerCommandTypes.STATUS { max_errors = args.max_errors; error_filter }
    in
    let rec go () =
      let%lwt (response, telemetry) =
        rpc args @@ ServerCommandTypes.REMOVE_DEAD_UNSAFE_CASTS
      in
      match response with
      | `Error msg ->
        Printf.eprintf "%s\n" msg;
        Lwt.return (Exit_status.Type_error, telemetry)
      | `Ok patches ->
        ClientRename.apply_patches patches;
        if List.is_empty patches then
          Lwt.return (Exit_status.No_error, telemetry)
        else
          let%lwt _ = rpc args status_cmd in
          go ()
    in
    go ()
  | ClientEnv.MODE_REWRITE_DECLARATIONS ->
    (*
    * HHVM uses existence of this file to indicate it is paused, which
    * happens when HHVM is at a breakpoint.
    * Is also read by the language server:
    * https://www.internalfb.com/code/fbsource/[248c997e7acd028f9980cc9f82113106cc56b63d]/fbcode/hphp/hack/src/client/clientLsp.ml. *)
    let file_whose_existence_indicates_hhvm_is_paused =
      Option.map (Sys.getenv_opt "HOME") ~f:(fun home_dir ->
          home_dir ^ "/.vscode-sockets/hhvm-paused")
    in
    DeclarationsRewriter.start
      (Random.State.make_self_init ())
      ~file_whose_existence_indicates_hhvm_is_paused;
    Lwt.return (Exit_status.No_error, Telemetry.create ())
  | ClientEnv.MODE_REWRITE_LAMBDA_PARAMETERS files ->
    let%lwt conn = connect args in
    let%lwt (patches, telemetry) =
      ClientConnect.rpc conn ~desc:args.desc
      @@ ServerCommandTypes.REWRITE_LAMBDA_PARAMETERS files
    in
    if args.output_json then
      ClientRename.print_patches_json patches
    else
      ClientRename.apply_patches patches;
    Lwt.return (Exit_status.No_error, telemetry)
  | ClientEnv.MODE_FULL_FIDELITY_PARSE file ->
    (* We can cheaply do this on the client today, but we might want to
       do it on the server and cache the results in the future. *)
    let do_it_on_server = false in
    let%lwt (results, telemetry) =
      if do_it_on_server then
        rpc args @@ ServerCommandTypes.DUMP_FULL_FIDELITY_PARSE file
      else
        let file = Relative_path.create Relative_path.Dummy file in
        let source_text = Full_fidelity_source_text.from_file file in
        let syntax_tree = SyntaxTree.make source_text in
        let json = SyntaxTree.to_json syntax_tree in
        Lwt.return (Hh_json.json_to_string json, Telemetry.create ())
    in
    ClientFullFidelityParse.go results;
    Lwt.return (Exit_status.No_error, telemetry)
  | ClientEnv.MODE_FULL_FIDELITY_SCHEMA ->
    let schema = Full_fidelity_schema.schema_as_json () in
    print_string schema;
    Lwt.return (Exit_status.No_error, Telemetry.create ())
  | ClientEnv.MODE_CST_SEARCH files_to_search ->
    let sort_results = args.sort_results in
    let input = Sys_utils.read_stdin_to_string () |> Hh_json.json_of_string in
    let%lwt (result, telemetry) =
      rpc args
      @@ ServerCommandTypes.CST_SEARCH
           { ServerCommandTypes.sort_results; input; files_to_search }
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
  | ClientEnv.MODE_FILE_LEVEL_DEPENDENCIES ->
    let paths = filter_real_paths ~allow_directories:true args.paths in
    let%lwt (responses, telemetry) =
      rpc args @@ ServerCommandTypes.FILE_DEPENDENTS paths
    in
    if args.output_json then
      Hh_json.(
        let json_path_list =
          List.map responses ~f:(fun path -> JSON_String path)
        in
        let output = JSON_Object [("dependents", JSON_Array json_path_list)] in
        print_endline @@ json_to_string output)
    else
      List.iter responses ~f:(Printf.printf "%s\n");
    Lwt.return (Exit_status.No_error, telemetry)
  | ClientEnv.MODE_VERBOSE verbose ->
    let%lwt ((), telemetry) = rpc args @@ ServerCommandTypes.VERBOSE verbose in
    Lwt.return (Exit_status.No_error, telemetry)
  | ClientEnv.MODE_DEPS_IN_AT_POS_BATCH positions ->
    let positions = parse_positions positions in
    let%lwt results =
      rpc_with_retry_list args @@ ServerCommandTypes.DEPS_IN_BATCH positions
    in
    List.iter results ~f:(fun s -> print_refs s ~json:true);
    Lwt.return (Exit_status.No_error, Telemetry.create ())
  | ClientEnv.MODE_FIND_MY_TESTS symbols ->
    let%lwt (result, telemtry) =
      rpc args
      @@ ServerCommandTypes.FIND_MY_TESTS
           (args.find_my_tests_max_distance, symbols)
    in
    (match result with
    | Ok fmt_result ->
      print_find_my_tests_result fmt_result ~json:args.output_json;
      Lwt.return (Exit_status.No_error, telemtry)
    | Error error ->
      Printf.eprintf "%s\n" error;
      Lwt.return (Exit_status.Input_error, telemtry))

let rec flush_event_logger () : unit Lwt.t =
  let%lwt () = Lwt_unix.sleep 1.0 in
  let%lwt () = EventLoggerLwt.flush () in
  flush_event_logger ()

let main
    (args : ClientEnv.client_check_env)
    (config : ServerConfig.t)
    (local_config : ServerLocalConfig.t)
    ~(init_proc_stack : string list option) : _ =
  HackEventLogger.client_set_mode
    (ClientEnv.Variants_of_client_mode.to_name args.mode);

  HackEventLogger.client_check_start ();
  ClientSpinner.start_heartbeat_telemetry ();
  Lwt.dont_wait flush_event_logger (fun _exn -> ());
  let partial_telemetry_ref = ref None in

  try
    (* Note: SIGINT exception handler is typically raised from [run_main], not from
       the lwt code that's running in [main_internal]. Thus, [Lwt_utils.run_main]'s caller is
       the only place that we can deal with it. This also motivates the use of a reference
       [partial_telemetry_ref], since there's no way for a return value to survive SIGINT. *)
    let (exit_status, telemetry) =
      Lwt_utils.run_main (fun () ->
          main_internal args config local_config partial_telemetry_ref)
    in
    let spinner = ClientSpinner.get_latest_report () in
    HackEventLogger.client_check exit_status telemetry ~init_proc_stack ~spinner;
    Hh_logger.log "CLIENT_CHECK %s" (Exit_status.show exit_status);
    Exit.exit exit_status
  with
  | exn ->
    let e = Exception.wrap exn in
    let spinner = ClientSpinner.get_latest_report () in
    (* hide the spinner *)
    ClientSpinner.report ~to_stderr:false ~angery_reaccs_only:false None;
    let exit_status =
      match exn with
      | Exit_status.Exit_with exit_status ->
        (* we assume that whoever raised [Exit_with] had the decency to print an explanation *)
        exit_status
      | _ ->
        (* if it was uncaught, then presumably no one else has printed it, so it's up to us.
           Let's include lots of details, including stack trace. *)
        let exit_status = Exit_status.Uncaught_exception e in
        Printf.eprintf "%s\n%!" (Exit_status.show_expanded exit_status);
        exit_status
    in
    begin
      match (exit_status, !partial_telemetry_ref) with
      | (Exit_status.(Interrupted | Client_broken_pipe), Some telemetry) ->
        (* [Interrupted] is raised by a SIGINT exit-handler installed in hh_client.
           [Client_broken_pipe] is raised in [clientCheckStatus.go_streaming] when
           it can't write errors to the pipe. *)
        HackEventLogger.client_check_partial
          exit_status
          telemetry
          ~init_proc_stack
          ~spinner;
        Hh_logger.log
          "CLIENT_CHECK_PARTIAL [%s] %s"
          (Option.value_map spinner ~f:fst ~default:"")
          (Exit_status.show exit_status)
      | _ ->
        HackEventLogger.client_check_bad_exit
          exit_status
          e
          ~init_proc_stack
          ~spinner;
        Hh_logger.log
          "CLIENT_CHECK_EXIT [%s] %s"
          (Option.value_map spinner ~f:fst ~default:"")
          (Exit_status.show_expanded exit_status)
    end;
    Exit.exit exit_status
