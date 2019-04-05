(**
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Core_kernel
open ClientEnv
open Utils
open ClientRefactor
open Ocaml_overrides

module Cmd = ServerCommandLwt
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
    from = args.from;
    autostart = args.autostart;
    force_dormant_start = args.force_dormant_start;
    retries = Some args.retries;
    expiry = args.timeout;
    no_load = args.no_load;
    watchman_debug_logging = args.watchman_debug_logging;
    log_inference_constraints = args.log_inference_constraints;
    profile_log = args.profile_log;
    ai_mode = args.ai_mode;
    progress_callback = ClientConnect.tty_progress_reporter ();
    do_post_handoff_handshake = true;
    ignore_hh_version = args.ignore_hh_version;
    saved_state_ignore_hhconfig = args.saved_state_ignore_hhconfig;
    use_priority_pipe;
    prechecked = args.prechecked;
    config = args.config;
  }

(* This is a function, because server closes the connection after each command,
 * so we need to be able to reconnect to retry. *)
type connect_fun = unit -> ClientConnect.conn Lwt.t

let rpc
  (args : ClientEnv.client_check_env)
  (command : 'a ServerCommandTypes.t)
  (call : connect_fun -> 'a ServerCommandTypes.t -> 'b Lwt.t)
: 'b Lwt.t =
  let use_priority_pipe =
    (not @@ ServerCommand.rpc_command_needs_full_check command) &&
    (not @@ ServerCommand.rpc_command_needs_writes command)
  in
  let conn = fun () -> connect args ~use_priority_pipe in
  let%lwt result = call conn @@ command in
  Lwt.return result

let rpc_with_retry
  (args : ClientEnv.client_check_env)
  (command : 'a ServerCommandTypes.Done_or_retry.t ServerCommandTypes.t)
: 'a Lwt.t =
  let%lwt result = rpc args command ClientConnect.rpc_with_retry in
  Lwt.return result

let rpc
  (args : ClientEnv.client_check_env)
  (command : 'a ServerCommandTypes.t)
: 'a Lwt.t =
  rpc args command (fun conn_f command ->
    let%lwt conn = conn_f () in
    let%lwt result = ClientConnect.rpc conn command in
    Lwt.return result
  )

let parse_positions positions =
  List.map positions begin fun pos ->
  try
    match Str.split (Str.regexp ":") pos with
    | [filename; line; char] ->
        expand_path filename, int_of_string line, int_of_string char
    | _ -> raise Exit
  with _ ->
    Printf.eprintf "Invalid position\n";
    raise Exit_status.(Exit_with Input_error)
  end
let main (args : client_check_env) : Exit_status.t Lwt.t =
  let mode_s = ClientEnv.mode_to_string args.mode in
  HackEventLogger.set_from args.from;
  HackEventLogger.client_set_mode mode_s;

  HackEventLogger.client_check ();

  let%lwt exit_status =
    match args.mode with
    | MODE_LIST_FILES ->
      let%lwt infol = rpc args @@ Rpc.LIST_FILES_WITH_ERRORS  in
      List.iter infol (Printf.printf "%s\n");
      Lwt.return Exit_status.No_error
    | MODE_COLORING file ->
      let file_input = match file with
        | "-" ->
          let content = Sys_utils.read_stdin_to_string () in
          ServerCommandTypes.FileContent content
        | _ ->
          let file = expand_path file in
          ServerCommandTypes.FileName file
      in
      let%lwt pos_level_l = rpc args @@ Rpc.COVERAGE_LEVELS file_input in
      ClientColorFile.go file_input args.output_json pos_level_l;
      Lwt.return Exit_status.No_error
    | MODE_COVERAGE file ->
      let%lwt counts_opt =
        rpc args @@ Rpc.COVERAGE_COUNTS (expand_path file) in
      ClientCoverageMetric.go ~json:args.output_json counts_opt;
      Lwt.return Exit_status.No_error
    | MODE_FIND_CLASS_REFS name ->
      let%lwt results =
        rpc_with_retry args @@ Rpc.FIND_REFS (ServerCommandTypes.Find_refs.Class name) in
      ClientFindRefs.go results args.output_json;
      Lwt.return Exit_status.No_error
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
      let%lwt results = rpc_with_retry args @@ Rpc.FIND_REFS action in
      ClientFindRefs.go results args.output_json;
      Lwt.return Exit_status.No_error
    | MODE_GEN_HOT_CLASSES (threshold, filename) ->
      let%lwt content = rpc args @@ Rpc.GEN_HOT_CLASSES threshold in
      begin try%lwt
        let oc = Pervasives.open_out filename in
        Out_channel.output_string oc content;
        Pervasives.close_out oc;
        Lwt.return Exit_status.No_error
      with exn ->
        Printf.eprintf "Failed to save hot classes file: %s\n" @@
          Exn.to_string exn;
        Printexc.print_backtrace stderr;
        Lwt.return Exit_status.Uncaught_exception
      end
    | MODE_IDE_FIND_REFS arg ->
      let line, char = parse_position_string arg in
      let include_defs = false in
      let content = Sys_utils.read_stdin_to_string () in
      let labelled_file =
        ServerCommandTypes.LabelledFileContent { filename = ""; content; } in
      let%lwt results =
        rpc_with_retry args @@ Rpc.IDE_FIND_REFS (labelled_file, line, char, include_defs) in
      ClientFindRefs.go_ide results args.output_json;
      Lwt.return Exit_status.No_error
    | MODE_IDE_HIGHLIGHT_REFS arg ->
      let line, char = parse_position_string arg in
      let content =
        ServerCommandTypes.FileContent (Sys_utils.read_stdin_to_string ()) in
      let%lwt results =
        rpc args @@ Rpc.IDE_HIGHLIGHT_REFS (content, line, char) in
      ClientHighlightRefs.go results ~output_json:args.output_json;
      Lwt.return Exit_status.No_error
    | MODE_DUMP_SYMBOL_INFO files ->
      let%lwt conn = connect args in
      let%lwt () = ClientSymbolInfo.go conn files expand_path in
      Lwt.return Exit_status.No_error
    | MODE_REFACTOR (ref_mode, before, after) ->
      let conn = fun () -> connect args in
      let%lwt () = ClientRefactor.go conn args ref_mode before after in
      Lwt.return Exit_status.No_error
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
      let%lwt () =
        ClientRefactor.go_ide conn args filename line char new_name in
      Lwt.return Exit_status.No_error
    | MODE_IDENTIFY_SYMBOL1 arg
    | MODE_IDENTIFY_SYMBOL2 arg
    | MODE_IDENTIFY_SYMBOL3 arg ->
      let line, char = parse_position_string arg in
      let content =
        ServerCommandTypes.FileContent (Sys_utils.read_stdin_to_string ()) in
      let%lwt result =
        rpc args @@ Rpc.IDENTIFY_FUNCTION (content, line, char) in
      ClientGetDefinition.go result args.output_json;
      Lwt.return Exit_status.No_error
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
      let%lwt ty =
        rpc args @@ Rpc.INFER_TYPE (fn, line, char, args.dynamic_view) in
      ClientTypeAtPos.go ty args.output_json;
      Lwt.return Exit_status.No_error
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
      let%lwt responses =
        rpc args @@ Rpc.INFER_TYPE_BATCH (positions, args.dynamic_view) in
      List.iter responses print_endline;
      Lwt.return Exit_status.No_error
    | MODE_FUN_DEPS_AT_POS_BATCH positions ->
      let positions = parse_positions positions in
      let%lwt responses =
        rpc args @@ Rpc.FUN_DEPS_BATCH (positions, args.dynamic_view) in
      List.iter responses print_endline;
      Lwt.return Exit_status.No_error
    | MODE_FUN_IS_LOCALLABLE_AT_POS_BATCH positions ->
      let positions = parse_positions positions in
      let%lwt responses =
        rpc args @@ Rpc.FUN_IS_LOCALLABLE_BATCH positions in
      List.iter responses print_endline;
      Lwt.return Exit_status.No_error
    | MODE_TYPED_FULL_FIDELITY_PARSE filename ->
      let fn =
        try
          (expand_path filename)
        with _ ->
          Printf.eprintf "Invalid filename: %s\n" filename;
          raise Exit_status.(Exit_with Input_error)
      in
      let%lwt result = rpc args @@ Rpc.TYPED_AST (fn) in
      print_endline result;
      Lwt.return Exit_status.No_error
    | MODE_AUTO_COMPLETE ->
      let content = Sys_utils.read_stdin_to_string () in
      let%lwt results = rpc args @@ Rpc.AUTOCOMPLETE content in
      ClientAutocomplete.go results args.output_json;
      Lwt.return Exit_status.No_error
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
      Lwt.return Exit_status.No_error
    | MODE_METHOD_JUMP_CHILDREN class_ ->
      let filter = ServerCommandTypes.Method_jumps.No_filter in
      let%lwt results = rpc args @@ Rpc.METHOD_JUMP (class_, filter, true) in
      ClientMethodJumps.go results true args.output_json;
      Lwt.return Exit_status.No_error
    | MODE_METHOD_JUMP_ANCESTORS (class_, filter) ->
      let filter =
        match MethodJumps.string_filter_to_method_jump_filter filter with
        | Some filter -> filter
        | None ->
          Printf.eprintf "Invalid method jump filter %s\n" filter;
          raise Exit_status.(Exit_with Input_error)
      in
      let%lwt results = rpc args @@ Rpc.METHOD_JUMP (class_, filter, false) in
      ClientMethodJumps.go results false args.output_json;
      Lwt.return Exit_status.No_error
    | MODE_METHOD_JUMP_ANCESTORS_BATCH (classes, filter) ->
      let filter =
        match MethodJumps.string_filter_to_method_jump_filter filter with
        | Some filter -> filter
        | None ->
          Printf.eprintf "Invalid method jump filter %s\n" filter;
          raise Exit_status.(Exit_with Input_error)
      in
      let%lwt results = rpc args @@ Rpc.METHOD_JUMP_BATCH (classes, filter) in
      ClientMethodJumps.go results false args.output_json;
      Lwt.return Exit_status.No_error
    | MODE_IN_MEMORY_DEP_TABLE_SIZE ->
      let%lwt result = rpc args @@ Rpc.IN_MEMORY_DEP_TABLE_SIZE in
      ClientResultPrinter.Int_printer.go result args.output_json;
      Lwt.return Exit_status.No_error
    | MODE_SAVE_STATE path ->
      let () = Sys_utils.mkdir_p (Filename.dirname path) in
      (** Convert to real path because Client and Server may have
       * different cwd and we want to use the client processes' cwd. *)
      let path = Path.make path in
      let%lwt result = rpc args @@ Rpc.SAVE_STATE (
        Path.to_string path,
        args.gen_saved_ignore_type_errors,
        args.file_info_on_disk,
        args.replace_state_after_saving) in
      ClientResultPrinter.Int_printer.go result args.output_json;
      Lwt.return Exit_status.No_error
    | MODE_STATUS ->
      let ignore_ide = ClientMessages.ignore_ide_from args.from in
      let%lwt () =
        if args.prechecked = Some false
        then rpc args (Rpc.NO_PRECHECKED_FILES)
        else Lwt.return_unit
      in
      let%lwt status = rpc args (Rpc.STATUS ignore_ide) in
      let exit_status =
        ClientCheckStatus.go status args.output_json args.from args.error_format in
      Lwt.return exit_status
    | MODE_STATUS_SINGLE filename ->
      let file_input = match filename with
        | "-" ->
          ServerCommandTypes.FileContent (Sys_utils.read_stdin_to_string ())
        | _ ->
          ServerCommandTypes.FileName (expand_path filename)
      in
      let%lwt error_list = rpc args (Rpc.STATUS_SINGLE file_input) in
      let status = {
        error_list;
        Rpc.Server_status.liveness = Rpc.Live_status;
        has_unsaved_changes = false;
      } in
      let exit_status =
        ClientCheckStatus.go status args.output_json args.from args.error_format in
      Lwt.return exit_status
    | MODE_SEARCH (query, type_) ->
      let%lwt results = rpc args @@ Rpc.SEARCH (query, type_) in
      ClientSearch.go results args.output_json;
      Lwt.return Exit_status.No_error
    | MODE_LINT ->
      let fnl = List.filter_map args.lint_paths
        (fun fn -> match Sys_utils.realpath fn with
          | Some path -> Some path
          | None -> prerr_endlinef "Could not find file '%s'" fn; None)
      in
      begin
        match args.lint_paths with
        | [] ->
           prerr_endline "No lint errors (0 files checked)!";
           prerr_endline "Note: --lint expects a list of filenames to check.";
           Lwt.return Exit_status.No_error
        | _ ->
           let%lwt results = rpc args @@ Rpc.LINT fnl in
           ClientLint.go results args.output_json args.error_format;
           Lwt.return Exit_status.No_error
      end
    | MODE_LINT_STDIN filename ->
      begin match Sys_utils.realpath filename with
      | None ->
        prerr_endlinef "Could not find file '%s'" filename;
        Lwt.return Exit_status.Input_error
      | Some filename ->
        let contents = Sys_utils.read_stdin_to_string () in
        let%lwt results = rpc args @@ Rpc.LINT_STDIN
          { ServerCommandTypes.filename; contents } in
        ClientLint.go results args.output_json args.error_format;
        Lwt.return Exit_status.No_error
      end
    | MODE_LINT_ALL code ->
      let%lwt results = rpc args @@ Rpc.LINT_ALL code in
      ClientLint.go results args.output_json args.error_format;
      Lwt.return Exit_status.No_error
    | MODE_LINT_XCONTROLLER filename ->
      begin try
        match Sys_utils.realpath filename with
        | None ->
          prerr_endlinef "Could not find file '%s'" filename;
          raise Exit
        | Some filename ->
          let files =
            Sys_utils.cat_no_fail filename |>
            Sys_utils.split_lines |>
            List.filter_map ~f:begin fun filename ->
              let res = Sys_utils.realpath filename in
              if Option.is_none res then begin
                prerr_endlinef "Could not find file '%s'" filename;
                raise Exit
              end;
              res
            end
          in
          let%lwt results = rpc args @@ Rpc.LINT_XCONTROLLER files in
          ClientLint.go results args.output_json args.error_format;
          Lwt.return Exit_status.No_error
        with Exit -> Lwt.return Exit_status.Input_error
      end
    | MODE_CREATE_CHECKPOINT x ->
      let%lwt () = rpc args @@ Rpc.CREATE_CHECKPOINT x in
      Lwt.return Exit_status.No_error
    | MODE_RETRIEVE_CHECKPOINT x ->
      let%lwt results = rpc args @@ Rpc.RETRIEVE_CHECKPOINT x in
      begin
        match results with
        | Some results ->
            List.iter results print_endline;
            Lwt.return Exit_status.No_error
        | None ->
            Lwt.return Exit_status.Checkpoint_error
      end
    | MODE_DELETE_CHECKPOINT x ->
      let%lwt results = rpc args @@ Rpc.DELETE_CHECKPOINT x in
      if results then
        Lwt.return Exit_status.No_error
      else
        Lwt.return Exit_status.Checkpoint_error
    | MODE_STATS ->
      let%lwt stats = rpc args @@ Rpc.STATS in
      print_string @@ Hh_json.json_to_multiline (Stats.to_json stats);
      Lwt.return Exit_status.No_error
    | MODE_REMOVE_DEAD_FIXMES codes ->
      (* we need to confirm that the server is not already started
       * in a non-no-load (yes-load) state
       *)
      let%lwt conn = connect args in
      let%lwt response = ClientConnect.rpc conn @@
          Rpc.REMOVE_DEAD_FIXMES codes in
      begin match response with
      | `Error msg ->
        Printf.eprintf "%s\n" msg;
        Lwt.return Exit_status.Type_error
      | `Ok patches -> begin
        if args.output_json
        then print_patches_json patches
        else apply_patches patches;
        Lwt.return Exit_status.No_error
      end end
    | MODE_REWRITE_LAMBDA_PARAMETERS files ->
      let%lwt conn = connect args in
      let%lwt patches = ClientConnect.rpc conn @@
          Rpc.REWRITE_LAMBDA_PARAMETERS files in
      if args.output_json
      then print_patches_json patches
      else apply_patches patches;
      Lwt.return Exit_status.No_error
    | MODE_FORMAT (from, to_) ->
      let content = Sys_utils.read_stdin_to_string () in
      let%lwt result =
        rpc args @@ Rpc.FORMAT (content, from, to_) in
      ClientFormat.go result args.output_json;
      Lwt.return Exit_status.No_error
    | MODE_AI_QUERY json ->
      let%lwt results = rpc args @@ Rpc.AI_QUERY json in
      ClientAiQuery.go results;
      Lwt.return Exit_status.No_error
    | MODE_FULL_FIDELITY_PARSE file ->
      (* We can cheaply do this on the client today, but we might want to
      do it on the server and cache the results in the future. *)
      let do_it_on_server = false in
      let%lwt results = if do_it_on_server then
        rpc args @@ Rpc.DUMP_FULL_FIDELITY_PARSE file
      else
        let file = Relative_path.create Relative_path.Dummy file in
        let source_text = Full_fidelity_source_text.from_file file in
        let syntax_tree = SyntaxTree.make source_text in
        let json = SyntaxTree.to_json syntax_tree in
        Lwt.return (Hh_json.json_to_string json) in
      ClientFullFidelityParse.go results;
      Lwt.return Exit_status.No_error
    | MODE_FULL_FIDELITY_SCHEMA ->
      let schema = Full_fidelity_schema.schema_as_json() in
      print_string schema;
      Lwt.return Exit_status.No_error
    | MODE_INFER_RETURN_TYPE name ->
      let open ServerCommandTypes.Infer_return_type in
      let action =
        parse_function_or_method_id
          ~func_action:(fun fun_name -> Function fun_name)
          ~meth_action:(fun class_name meth_name ->
            Method (class_name, meth_name))
          name
      in
      let%lwt result = rpc args @@ Rpc.INFER_RETURN_TYPE action in
      begin match result with
      | Error error_str ->
        Printf.eprintf "%s\n" error_str;
        raise Exit_status.(Exit_with Input_error)
      | Ok ty ->
        if args.output_json then
          print_endline Hh_json.(json_to_string (JSON_String ty))
        else
          print_endline ty;
        Lwt.return Exit_status.No_error
      end
    | MODE_CST_SEARCH files_to_search ->
      let sort_results = args.sort_results in
      let input = Sys_utils.read_stdin_to_string () |> Hh_json.json_of_string in
      let%lwt result = rpc args @@ Rpc.CST_SEARCH { Rpc.
        sort_results;
        input;
        files_to_search;
      } in
      begin match result with
      | Ok result ->
        print_endline (Hh_json.json_to_string result);
        Lwt.return Exit_status.No_error
      | Error error -> print_endline error;
        raise Exit_status.(Exit_with Input_error)
      end
  in
  HackEventLogger.client_check_finish exit_status;
  Lwt.return exit_status
