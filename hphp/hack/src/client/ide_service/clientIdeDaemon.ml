(*
 * Copyright (c) 2019, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Core_kernel

type message = Message : 'a ClientIdeMessage.t -> message

type message_queue = message Lwt_message_queue.t

type state =
  | Initializing
  | Failed_to_initialize of string
  | Initialized of {
      saved_state_info: Saved_state_loader.Naming_table_saved_state_info.t;
      hhi_root: Path.t;
      server_env: ServerEnv.env;
      changed_files_to_process: Path.Set.t;
    }

type t = {
  message_queue: message_queue;
  state: state;
}

let log s = Hh_logger.log ("[ide-daemon] " ^^ s)

let set_up_hh_logger_for_client_ide_service ~(root : Path.t) : unit =
  (* Log to a file on disk. Note that calls to `Hh_logger` will always write to
  `stderr`; this is in addition to that. *)
  let client_ide_log_fn = ServerFiles.client_ide_log root in
  begin
    try Sys.rename client_ide_log_fn (client_ide_log_fn ^ ".old")
    with _e -> ()
  end;
  Hh_logger.set_log
    client_ide_log_fn
    (Out_channel.create client_ide_log_fn ~append:true);
  EventLogger.init EventLogger.Event_logger_fake 0.0;
  log "Starting client IDE service at %s" client_ide_log_fn

let load_naming_table_from_saved_state_info
    (server_env : ServerEnv.env)
    (saved_state_info : Saved_state_loader.Naming_table_saved_state_info.t) :
    ServerEnv.env Lwt.t =
  let path =
    Saved_state_loader.Naming_table_saved_state_info.(
      Path.to_string saved_state_info.naming_table_path)
  in
  let naming_table =
    Naming_table.load_from_sqlite ~update_reverse_entries:false path
  in
  log "Loaded naming table from SQLite database at %s" path;
  let server_env = { server_env with ServerEnv.naming_table } in
  Lwt.return server_env

let load_saved_state
    (env : ServerEnv.env)
    ~(root : Path.t)
    ~(hhi_root : Path.t)
    ~(naming_table_saved_state_path : Path.t option) : state Lwt.t =
  log "[saved-state] Starting load in root %s" (Path.to_string root);
  let%lwt result =
    try%lwt
      let%lwt result =
        match naming_table_saved_state_path with
        | Some naming_table_saved_state_path ->
          (* Assume that there are no changed files on disk if we're getting
          passed the path to the saved-state directly, and that the saved-state
          corresponds to the current state of the world. *)
          let changed_files = [] in
          Lwt.return_ok
            ( {
                Saved_state_loader.Naming_table_saved_state_info
                .naming_table_path = naming_table_saved_state_path;
              },
              changed_files )
        | None ->
          let%lwt result =
            State_loader_lwt.load
              ~repo:root
              ~saved_state_type:Saved_state_loader.Naming_table
          in
          Lwt.return result
      in
      let%lwt new_state =
        match result with
        | Ok (saved_state_info, changed_files) ->
          log
            "[saved-state] Naming table path: %s"
            Saved_state_loader.Naming_table_saved_state_info.(
              Path.to_string saved_state_info.naming_table_path);

          let%lwt server_env =
            load_naming_table_from_saved_state_info env saved_state_info
          in
          log "[saved-state] Load succeeded";

          Lwt.return
            (Initialized
               {
                 saved_state_info;
                 hhi_root;
                 server_env;
                 changed_files_to_process = Path.Set.of_list changed_files;
               })
        | Error load_error ->
          let message = Saved_state_loader.load_error_to_string load_error in
          log "[saved-state] %s" message;
          Lwt.return (Failed_to_initialize message)
      in
      Lwt.return new_state
    with e ->
      let stack = Printexc.get_backtrace () in
      Hh_logger.exc
        e
        ~prefix:"Uncaught exception in client IDE services"
        ~stack;
      Lwt.return
        (Failed_to_initialize
           (Printf.sprintf
              "Uncaught exception in client IDE services: %s"
              stack))
  in
  Lwt.return result

let initialize
    ({
       ClientIdeMessage.Initialize_from_saved_state.root;
       naming_table_saved_state_path;
     } :
      ClientIdeMessage.Initialize_from_saved_state.t) =
  set_up_hh_logger_for_client_ide_service ~root;

  Relative_path.set_path_prefix Relative_path.Root root;
  let hhi_root = Hhi.get_hhi_root () in
  log "Extracted hhi files to directory %s" (Path.to_string hhi_root);
  Relative_path.set_path_prefix Relative_path.Hhi hhi_root;
  Relative_path.set_path_prefix Relative_path.Tmp (Path.make "/tmp");

  let server_args = ServerArgs.default_options ~root:(Path.to_string root) in
  let (server_config, server_local_config) =
    ServerConfig.load ServerConfig.filename server_args
  in
  (* NOTE: We don't want to depend on shared memory in the long-term, since
  we're only running one process and don't need to share memory with anyone. To
  remove the shared memory usage here requires refactoring our heaps to never
  write to shared memory. *)
  let (_ : SharedMem.handle) =
    SharedMem.init ~num_workers:0 (ServerConfig.sharedmem_config server_config)
  in
  let bytes_per_word = Sys.word_size / 8 in
  let words_per_mb = 1_000_000 / bytes_per_word in
  let max_size_in_words = 250 * words_per_mb in
  Provider_config.set_local_memory_backend ~max_size_in_words;

  let genv =
    ServerEnvBuild.make_genv
      server_args
      server_config
      server_local_config
      [] (* no workers *)
      None
    (* no lru_workers *)
  in
  let server_env = ServerEnvBuild.make_env genv.ServerEnv.config in
  (* We need shallow class declarations so that we can invalidate individual
  members in a class hierarchy. *)
  let server_env =
    {
      server_env with
      ServerEnv.tcopt =
        {
          server_env.ServerEnv.tcopt with
          GlobalOptions.tco_shallow_class_decl = true;
        };
    }
  in
  GlobalParserOptions.set server_env.ServerEnv.popt;
  GlobalNamingOptions.set server_env.ServerEnv.tcopt;

  (* Use server_config to modify server_env with the correct symbol index *)
  let namespace_map =
    GlobalOptions.po_auto_namespace_map server_env.ServerEnv.tcopt
  in
  let sienv =
    SymbolIndex.initialize
      ~globalrev_opt:None
      ~namespace_map
      ~provider_name:
        server_local_config.ServerLocalConfig.symbolindex_search_provider
      ~quiet:server_local_config.ServerLocalConfig.symbolindex_quiet
      ~savedstate_file_opt:
        server_local_config.ServerLocalConfig.symbolindex_file
      ~workers:None
  in
  let sienv = { sienv with SearchUtils.sie_log_timings = true } in
  let server_env =
    { server_env with ServerEnv.local_symbol_table = ref sienv }
  in
  let%lwt new_state =
    load_saved_state server_env ~root ~hhi_root ~naming_table_saved_state_path
  in
  log "Serverless IDE has completed initialization";
  Lwt.return new_state

let shutdown (state : state) : unit Lwt.t =
  match state with
  | Initializing
  | Failed_to_initialize _ ->
    log "No cleanup to be done";
    Lwt.return_unit
  | Initialized { hhi_root; _ } ->
    let hhi_root = Path.to_string hhi_root in
    log "Removing hhi directory %s..." hhi_root;
    Sys_utils.rm_dir_tree hhi_root;
    Lwt.return_unit

let make_context_from_document_location
    (server_env : ServerEnv.env)
    (document_location : ClientIdeMessage.document_location) :
    Provider_context.t * Provider_context.entry =
  let (file_path, file_input) =
    match document_location with
    | { ClientIdeMessage.file_contents = None; file_path; _ } ->
      let file_input =
        ServerCommandTypes.FileName (Path.to_string file_path)
      in
      (file_path, file_input)
    | { ClientIdeMessage.file_contents = Some file_contents; file_path; _ } ->
      let file_input = ServerCommandTypes.FileContent file_contents in
      (file_path, file_input)
  in
  let file_path =
    file_path |> Path.to_string |> Relative_path.create_detect_prefix
  in
  Provider_utils.update_context
    ~ctx:(Provider_context.empty ~tcopt:server_env.ServerEnv.tcopt)
    ~path:file_path
    ~file_input

module Handle_message_result = struct
  type 'a t =
    | Notification
    | Response of 'a
    | Error of string
end

let handle_message :
    type a.
    state -> a ClientIdeMessage.t -> (state * a Handle_message_result.t) Lwt.t
    =
 fun state message ->
  ClientIdeMessage.(
    match (state, message) with
    | (state, Shutdown ()) ->
      let%lwt () = shutdown state in
      Lwt.return (state, Handle_message_result.Response ())
    | ((Failed_to_initialize _ | Initializing), File_changed _) ->
      (* Should not happen. *)
      Lwt.return
        ( state,
          Handle_message_result.Error
            ( "IDE services could not process file change because "
            ^ "it failed to initialize or was still initializing. The caller "
            ^ "should have waited for the IDE services to become ready before "
            ^ "sending file-change notifications." ) )
    | ( Initialized ({ changed_files_to_process; _ } as state),
        File_changed path ) ->
      let changed_files_to_process =
        Path.Set.add changed_files_to_process path
      in
      let state = Initialized { state with changed_files_to_process } in
      Lwt.return (state, Handle_message_result.Notification)
    | (Initializing, Initialize_from_saved_state param) ->
      let%lwt new_state = initialize param in
      Lwt.return (new_state, Handle_message_result.Response ())
    | (Initialized _, Initialize_from_saved_state _) ->
      Lwt.return
        ( state,
          Handle_message_result.Error
            "Tried to initialize when already initialized" )
    | (Initializing, _) ->
      Lwt.return
        ( state,
          Handle_message_result.Error
            "IDE services have not yet been initialized" )
    | (Failed_to_initialize error_message, _) ->
      Lwt.return
        ( state,
          Handle_message_result.Error
            (Printf.sprintf
               "IDE services failed to initialize: %s"
               error_message) )
    | (Initialized { server_env; _ }, File_opened { file_path; file_contents })
      ->
      let path =
        file_path |> Path.to_string |> Relative_path.create_detect_prefix
      in
      let (ctx, entry) =
        Provider_utils.update_context
          ~ctx:(Provider_context.empty ~tcopt:server_env.ServerEnv.tcopt)
          ~path
          ~file_input:(ServerCommandTypes.FileContent file_contents)
      in
      (* Do a typecheck just to warm up the caches when you open a file. In the
    future, we'll actually surface typechecking errors. *)
      let _tast : Tast.program = Provider_utils.compute_tast ~ctx ~entry in
      Lwt.return (state, Handle_message_result.Response ())
    | (Initialized { server_env; _ }, Hover document_location) ->
      let (ctx, entry) =
        make_context_from_document_location server_env document_location
      in
      let result =
        Provider_utils.with_context ~ctx ~f:(fun () ->
            ServerHover.go_ctx
              ~ctx
              ~entry
              ~line:document_location.ClientIdeMessage.line
              ~column:document_location.ClientIdeMessage.column)
      in
      Lwt.return (state, Handle_message_result.Response result)
    (* Autocomplete *)
    | ( Initialized { server_env; _ },
        Completion
          {
            ClientIdeMessage.Completion.document_location =
              { ClientIdeMessage.file_path; file_contents; line; column };
            is_manually_invoked;
          } ) ->
      let path =
        file_path |> Path.to_string |> Relative_path.create_detect_prefix
      in
      let file_content =
        match file_contents with
        | Some file_contents -> file_contents
        | None -> file_path |> Path.to_string |> Sys_utils.cat_no_fail
      in
      let sienv = !(server_env.ServerEnv.local_symbol_table) in
      let matches =
        ServerAutoComplete.auto_complete_at_position_ctx
          ~line
          ~column
          ~file_content
          ~path
          ~tcopt:server_env.ServerEnv.tcopt
          ~sienv
          ~is_manually_invoked
      in
      let result =
        {
          AutocompleteTypes.completions =
            matches.Utils.With_complete_flag.value;
          char_at_pos = ' ';
          is_complete = matches.Utils.With_complete_flag.is_complete;
        }
      in
      Lwt.return (state, Handle_message_result.Response result)
    (* Autocomplete docblock resolve *)
    | (Initialized { server_env; _ }, Completion_resolve param) ->
      ClientIdeMessage.Completion_resolve.(
        let start_time = Unix.gettimeofday () in
        let result =
          ServerDocblockAt.go_docblock_for_symbol
            ~env:server_env
            ~symbol:param.symbol
            ~kind:param.kind
        in
        let sienv = !(server_env.ServerEnv.local_symbol_table) in
        ( if sienv.SearchUtils.sie_log_timings then
          let _t : float =
            Hh_logger.log_duration
              (Printf.sprintf
                 "[docblock] Search for [%s] [%s]"
                 param.symbol
                 (SearchUtils.show_si_kind param.kind))
              start_time
          in
          () );
        Lwt.return (state, Handle_message_result.Response result))
    (* Autocomplete docblock resolve *)
    | (Initialized { server_env; _ }, Completion_resolve_location param) ->
      ClientIdeMessage.Completion_resolve_location.(
        let start_time = Unix.gettimeofday () in
        let contents =
          Option.value_exn param.document_location.file_contents
        in
        let result =
          ServerDocblockAt.go_docblock_at_contents
            ~filename:(Path.to_string param.document_location.file_path)
            ~contents
            ~line:param.document_location.line
            ~column:param.document_location.column
            ~kind:param.kind
        in
        let sienv = !(server_env.ServerEnv.local_symbol_table) in
        ( if sienv.SearchUtils.sie_log_timings then
          let pathstr = Path.to_string param.document_location.file_path in
          let msg =
            Printf.sprintf
              "[docblock] Search for [%s] line [%d] column [%d] kind [%s]"
              pathstr
              param.document_location.line
              param.document_location.column
              (SearchUtils.show_si_kind param.kind)
          in
          let _t : float = Hh_logger.log_duration msg start_time in
          () );
        Lwt.return (state, Handle_message_result.Response result))
    (* Document highlighting *)
    | (Initialized { server_env; _ }, Document_highlight document_location) ->
      let (ctx, entry) =
        make_context_from_document_location server_env document_location
      in
      let results =
        Provider_utils.with_context ~ctx ~f:(fun () ->
            ServerHighlightRefs.go_ctx
              ~ctx
              ~entry
              ~line:document_location.line
              ~column:document_location.column
              ~tcopt:server_env.ServerEnv.tcopt)
      in
      Lwt.return (state, Handle_message_result.Response results)
    (* Signature help *)
    | (Initialized { server_env; _ }, Signature_help document_location) ->
      let (ctx, entry) =
        make_context_from_document_location server_env document_location
      in
      let results =
        Provider_utils.with_context ~ctx ~f:(fun () ->
            ServerSignatureHelp.go
              ~env:server_env
              ~file:entry.Provider_context.file_input
              ~line:document_location.line
              ~column:document_location.column)
      in
      Lwt.return (state, Handle_message_result.Response results)
    (* Go to definition *)
    | (Initialized { server_env; _ }, Definition document_location) ->
      let (ctx, entry) =
        make_context_from_document_location server_env document_location
      in
      let result =
        Provider_utils.with_context ~ctx ~f:(fun () ->
            ServerGoToDefinition.go_ctx
              ~ctx
              ~entry
              ~line:document_location.ClientIdeMessage.line
              ~column:document_location.ClientIdeMessage.column)
      in
      Lwt.return (state, Handle_message_result.Response result)
    (* Type Definition *)
    | (Initialized { server_env; _ }, Type_definition document_location) ->
      let (ctx, entry) =
        make_context_from_document_location server_env document_location
      in
      let result =
        Provider_utils.with_context ~ctx ~f:(fun () ->
            ServerTypeDefinition.go_ctx
              ~ctx
              ~entry
              ~line:document_location.ClientIdeMessage.line
              ~column:document_location.ClientIdeMessage.column)
      in
      Lwt.return (state, Handle_message_result.Response result)
    (* Document Symbol *)
    | (Initialized { server_env; _ }, Document_symbol document_identifier) ->
      let result =
        match document_identifier.Document_symbol.file_contents with
        | None -> []
        | Some file_contents ->
          FileOutline.outline server_env.ServerEnv.popt file_contents
      in
      Lwt.return (state, Handle_message_result.Response result)
    (* Type Coverage *)
    | (Initialized { server_env; _ }, Type_coverage document_identifier) ->
      let document_location =
        {
          file_path = document_identifier.file_path;
          file_contents = Some document_identifier.file_contents;
          line = 0;
          column = 0;
        }
      in
      let (ctx, entry) =
        make_context_from_document_location server_env document_location
      in
      let result =
        Provider_utils.with_context ~ctx ~f:(fun () ->
            ServerColorFile.go_ctx ~ctx ~entry)
      in
      Lwt.return (state, Handle_message_result.Response result))

let write_message
    ~(out_fd : Lwt_unix.file_descr)
    ~(message : ClientIdeMessage.message_from_daemon) : unit Lwt.t =
  let%lwt (_ : int) = Marshal_tools_lwt.to_fd_with_preamble out_fd message in
  Lwt.return_unit

let serve
    (type a) ~(in_fd : Lwt_unix.file_descr) ~(out_fd : Lwt_unix.file_descr) :
    unit Lwt.t =
  let rec pump_message_queue (message_queue : message_queue) : unit Lwt.t =
    try%lwt
      let%lwt (message : a ClientIdeMessage.t) =
        Marshal_tools_lwt.from_fd_with_preamble in_fd
      in
      let is_queue_open =
        Lwt_message_queue.push message_queue (Message message)
      in
      match message with
      | ClientIdeMessage.Shutdown () -> Lwt.return_unit
      | _ when not is_queue_open -> Lwt.return_unit
      | _ -> pump_message_queue message_queue
    with e ->
      let e = Exception.wrap e in
      Lwt_message_queue.close message_queue;
      Exception.reraise e
  in
  let rec handle_messages (t : t) : unit Lwt.t =
    match t with
    | {
     message_queue;
     state = Initialized ({ server_env; changed_files_to_process; _ } as state);
    }
      when Lwt_message_queue.is_empty message_queue
           && (not (Lwt_unix.readable in_fd))
           && not (Path.Set.is_empty changed_files_to_process) ->
      (* Process the next file change, but only if we have no new events to
      handle. To ensure correctness, we would have to actually process all file
      change events *before* we processed any other IDE queries. However, we're
      trying to maximize availability, even if occasionally we give stale
      results. We can revisit this trade-off later if we decide that the stale
      results are baffling users. *)
      let next_file = Path.Set.choose changed_files_to_process in
      let changed_files_to_process =
        Path.Set.remove changed_files_to_process next_file
      in
      let%lwt server_env =
        ClientIdeIncremental.process_changed_file server_env next_file
      in
      let%lwt () =
        if Path.Set.is_empty changed_files_to_process then
          let message = ClientIdeMessage.(Notification Done_processing) in
          let%lwt () = write_message ~out_fd ~message in
          Lwt.return_unit
        else
          Lwt.return_unit
      in
      let state =
        Initialized { state with server_env; changed_files_to_process }
      in
      handle_messages { t with state }
    | t ->
      let%lwt message = Lwt_message_queue.pop t.message_queue in
      (match message with
      | None -> Lwt.return_unit
      | Some (Message message) ->
        let%lwt state =
          try%lwt
            let%lwt (state, response) = handle_message t.state message in
            match response with
            | Handle_message_result.Notification ->
              (* No response needed for notifications. *)
              Lwt.return state
            | Handle_message_result.Response response ->
              let response = ClientIdeMessage.Response (Ok response) in
              let%lwt () = write_message ~out_fd ~message:response in
              Lwt.return state
            | Handle_message_result.Error message ->
              let response = ClientIdeMessage.Response (Error message) in
              let%lwt () = write_message ~out_fd ~message:response in
              Lwt.return state
          with e ->
            let stack = Printexc.get_backtrace () in
            Hh_logger.exc ~prefix:"[ide-daemon] exception: " ~stack e;
            Lwt.return t.state
        in
        handle_messages { t with state })
  in
  try%lwt
    let message_queue = Lwt_message_queue.create () in
    let%lwt () = handle_messages { message_queue; state = Initializing }
    and () = pump_message_queue message_queue in
    Lwt.return_unit
  with e ->
    let e = Exception.wrap e in
    log
      "Exception occurred while handling RPC call: %s"
      (Exception.to_string e);
    Lwt.return_unit

let daemon_main () (channels : ('a, 'b) Daemon.channel_pair) : unit =
  Printexc.record_backtrace true;
  let (ic, oc) = channels in
  let in_fd = Lwt_unix.of_unix_file_descr (Daemon.descr_of_in_channel ic) in
  let out_fd = Lwt_unix.of_unix_file_descr (Daemon.descr_of_out_channel oc) in
  Lwt_main.run (serve ~in_fd ~out_fd)

let daemon_entry_point : (unit, unit, unit) Daemon.entry =
  Daemon.register_entry_point "ClientIdeService" daemon_main
