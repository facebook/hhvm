(*
 * Copyright (c) 2019, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Core_kernel

type message = Message : 'a ClientIdeMessage.tracked_t -> message

type message_queue = message Lwt_message_queue.t

(** Here are some invariants for initialized_state, concerning these data-structures:
1. forward-naming-table-delta stored in naming_table
2. reverse-naming-table-delta-and-cache stored in local_memory
3. entries with source text, stored in open_files
3. cached ASTs and TASTs, stored in open_files
4. shallow-decl-cache, folded-decl-cache, linearization-cache stored in local-memory

There are two concepts to understand.
1. "Singleton context" (ctx). When processing IDE requests for a file, we create
   a context object in which that entry's source text and AST and TAST
   are present in the context, and no others are.
2. "Quarantine with respect to an entry". We enter a quarantine while
   computing the TAST for a singleton context entry. The invariants within
   the quarantine are different from those without.

The key algorithms which read from these data-structures are:
1. Ast_provider.get_ast will fetch the cached AST for an entry in ctx, or if
   the entry is present but as yet lacks an AST then it will parse and cache,
   or if it's lookinng for the AST of a file not in ctx then it will parse
   off disk but decline to cache.
2. Naming_provider.get_* will get_ast for ctx entry to see if symbol is there.
   If not it will look in reverse-delta-and-cache or read from sqlite
   and store the answer back in reverse-delta-and-cache. But if the answer
   to that fallback was a file in ctx, then it will say that the symbol's
   not defined.
3. Shallow_classes_provider.get_* will look it up in shallow-decl-cache, and otherwise
   will ask Naming_provider and Ast_provider for the AST, will compute shallow decl,
   and will store it in shallow-decl-cache
4. Linearization_provider.get_* will look it up in linearization-cache. The
   decl_provider reads and writes linearizations via the linearization_provider.
5. Decl_provider.get_* will look it up in folded-decl-cache, computing it if
   not there using shallow and linearization provider, and store it back in folded-decl-cache
6. Tast_provider.compute* is only ever called on entries. It returns the cached
   TAST if present; otherwise, it runs normal type-checking-and-inference, relies
   upon all the other providers, and writes the answer back in the entry's TAST cache.

The invariants for forward and reverse naming tables:
1. These tables only ever reflect truth about disk files; they are unaffected
   by open_file entries.
2. They are updated asynchronously by update_naming_tables_for_changed_file
   in response to DidChangeWatchedFile events. Thus, we might be asked to fetch
   a shallow decl even before the naming-tables have been fully updated.
   We might for instance read the naming-table and try to fetch a shallow
   decl from a file that doesn't even exist on disk any more.

The invariants for AST, TAST, shallow, folded-decl and linearization caches:
1. AST, if present, reflects the AST of its entry's source text,
   and is a "full" AST (not decl-only), and has errors.
2. TAST, if present, reflects the TAST of its entry's source text computed
   against the on-disk state of all other files
3. Outside a quarantine, all entries in shallow cache are correct as of disk
   (at least as far as asynchronous file updates have been processed).
4. Likewise, all entries in folded+linearization caches are correct as
   of disk.
5. We only ever enter quarantine with respect to one single entry.
   For the duration of the quarantine, an AST for that entry,
   if present, is correct as of the entry's source text.
6. Likewise any shallow decls for an entry are correct as of its source text.
   Moreover, if shallow decls for an entry are present, then the entry's AST
   is present and contains those symbols.
7. Any shallow decls not for the entry are correct as of disk.
8. During quarantine, the shallow-decl of all other files is correct as of disk.
9. The entry's TAST, along with every single decl and linearization,
   are correct as of this entry's source text plus every other file off disk.

Here are the algorithms we use that satisfy those invariants.
1. Upon a disk-file-change, we invalidate all TASTs (satisfying invariant 2).
   We use the forward-naming-table to find all "old" symbols that were
   defined in the file prior to the disk change, and invalidate those
   shallow decls (satisfying invariant 3). We invalidate all
  folded+linearization caches (satisfying invariant 4). Invariant 1 is N/A.
2. Upon an editor change to a file, we invalidate the entry's AST and TAST
   (satisfying invariant 1).
3. Upon request for a TAST of a file, we create a singleton context for
   that entry, and enter quarantine as follows. We parse the file and
   cache its AST and invalidate shallow decls for all symbols inside
   this new AST (satisfying invariant 6). We invalidate all decls
   and linearizations (satisfying invariant 9). Subsequent fetches,
   thanks to the "key algorithms for reading these datastructures" (above)
   will only cache things in accordance with invariants 6,7,8,9.
4. We leave quarantine as follows. We invalidate shallow decls for
   all symbols in the entry's AST; thanks to invariant 5, this will
   fulfill invariant 3. We invalidate all decls and linearizations
  (satisfying invariant 4).
*)
type initialized_state = {
  hhi_root: Path.t;
      (** hhi_root files are written during initialize, deleted at shutdown, and
  refreshed periodically in case the tmp-cleaner has deleted them. *)
  naming_table: Naming_table.t;
      (** the forward-naming-table is constructed during initialize and updated
  during process_changed_files. It stores an in-memory map of FileInfos that
  have changed since sqlite. When a file is changed on disk, we need this to
  know which shallow decls to invalidate. Note: while the forward-naming-table
  is stored here, the reverse-naming-table is instead stored in ctx. *)
  sienv: SearchUtils.si_env;
      (** sienv provides autocomplete and find-symbols. It is constructed during
  initialization and stores a few in-memory structures such as namespace-list,
  plus in-memory deltas. It is also updated during process_changed_files. *)
  popt: ParserOptions.t;  (** parser options *)
  tcopt: TypecheckerOptions.t;  (** typechecker options *)
  local_memory: Provider_backend.local_memory;
      (** Local_memory backend; includes decl caches *)
  open_files: Provider_context.entries;
      (** all open files, along with caches of their ASTs and TASTs and errors *)
  changed_files_to_process: Path.Set.t;
      (** changed_files_to_process is grown during File_changed events, and steadily
  whittled down one by one in `serve` as we get around to processing them
  via `process_changed_files`. *)
  changed_files_denominator: int;
      (** the user likes to see '5/10' for how many changed files has been processed
  in the current batch of changes. The denominator counts up for every new file
  that has to be processed, until the batch ends - i.e. changed_files_to_process
  becomes empty - and we reset the denominator. *)
}

type state =
  | Initializing
  | Failed_to_initialize of ClientIdeMessage.error_data
  | Initialized of initialized_state

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
  log "Starting client IDE service at %s" client_ide_log_fn

let load_saved_state
    (ctx : Provider_context.t)
    ~(root : Path.t)
    ~(naming_table_saved_state_path : Path.t option) :
    ( Naming_table.t * Saved_state_loader.changed_files,
      ClientIdeMessage.error_data )
    Lwt_result.t =
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
              ~ignore_hh_version:false
              ~saved_state_type:Saved_state_loader.Naming_table
          in
          Lwt.return result
      in
      match result with
      | Ok (saved_state_info, changed_files) ->
        let path =
          Path.to_string
            saved_state_info
              .Saved_state_loader.Naming_table_saved_state_info
               .naming_table_path
        in
        log "[saved-state] Loading naming-table... %s" path;
        let naming_table = Naming_table.load_from_sqlite ctx path in
        log "[saved-state] Loaded naming-table.";
        (* Track how many files we have to change locally *)
        HackEventLogger.serverless_ide_local_files
          ~local_file_count:(List.length changed_files);

        Lwt.return_ok (naming_table, changed_files)
      | Error load_error ->
        Lwt.return_error
          ClientIdeMessage.
            {
              short_user_message =
                Saved_state_loader.short_user_message_of_error load_error;
              medium_user_message =
                Saved_state_loader.medium_user_message_of_error load_error;
              long_user_message =
                Saved_state_loader.long_user_message_of_error load_error;
              debug_details =
                Saved_state_loader.debug_details_of_error load_error;
              is_actionable = Saved_state_loader.is_error_actionable load_error;
            }
    with e ->
      let stack = e |> Exception.wrap |> Exception.get_backtrace_string in
      let prefix = "Uncaught exception in client IDE services" in
      Hh_logger.exc e ~prefix ~stack;
      let debug_details = prefix ^ ": " ^ Exn.to_string e in
      Lwt.return_error (ClientIdeMessage.make_error_data debug_details ~stack)
  in
  Lwt.return result

let log_startup_time (component : string) (start_time : float) : float =
  let now = Unix.gettimeofday () in
  HackEventLogger.serverless_ide_startup ~component ~start_time;
  now

let initialize
    ({
       ClientIdeMessage.Initialize_from_saved_state.root;
       naming_table_saved_state_path;
       use_ranked_autocomplete;
       config;
       open_files;
     } :
      ClientIdeMessage.Initialize_from_saved_state.t) :
    (state, ClientIdeMessage.error_data) Lwt_result.t =
  let start_time = Unix.gettimeofday () in
  HackEventLogger.serverless_ide_set_root root;
  set_up_hh_logger_for_client_ide_service ~root;

  Relative_path.set_path_prefix Relative_path.Root root;
  let hhi_root = Hhi.get_hhi_root () in
  log "Extracted hhi files to directory %s" (Path.to_string hhi_root);
  Relative_path.set_path_prefix Relative_path.Hhi hhi_root;
  Relative_path.set_path_prefix Relative_path.Tmp (Path.make "/tmp");

  let server_args = ServerArgs.default_options ~root:(Path.to_string root) in
  let server_args = ServerArgs.set_config server_args config in
  let (server_config, server_local_config) =
    ServerConfig.load ServerConfig.filename server_args
  in
  let hhconfig_version =
    server_config |> ServerConfig.version |> Config_file.version_to_string_opt
  in
  HackEventLogger.set_hhconfig_version hhconfig_version;

  (* NOTE: We don't want to depend on shared memory in the long-term, since
  we're only running one process and don't need to share memory with anyone. To
  remove the shared memory usage here requires refactoring our heaps to never
  write to shared memory. *)
  let (_ : SharedMem.handle) =
    SharedMem.init ~num_workers:0 (ServerConfig.sharedmem_config server_config)
  in

  Provider_backend.set_local_memory_backend_with_defaults ();
  let local_memory =
    match Provider_backend.get () with
    | Provider_backend.Local_memory local_memory -> local_memory
    | _ -> failwith "expected local memory backend"
  in

  (* Use server_config to modify server_env with the correct symbol index *)
  let genv =
    ServerEnvBuild.make_genv server_args server_config server_local_config []
  in
  let { ServerEnv.tcopt; popt; gleanopt; _ } =
    ServerEnvBuild.make_env genv.ServerEnv.config
  in

  (* We need shallow class declarations so that we can invalidate individual
  members in a class hierarchy. *)
  let tcopt = { tcopt with GlobalOptions.tco_shallow_class_decl = true } in

  let start_time = log_startup_time "basic_startup" start_time in
  let sienv =
    SymbolIndex.initialize
      ~globalrev:None
      ~gleanopt
      ~namespace_map:(GlobalOptions.po_auto_namespace_map tcopt)
      ~provider_name:
        server_local_config.ServerLocalConfig.symbolindex_search_provider
      ~quiet:server_local_config.ServerLocalConfig.symbolindex_quiet
      ~ignore_hh_version:false
      ~savedstate_file_opt:
        server_local_config.ServerLocalConfig.symbolindex_file
      ~workers:None
  in
  let sienv =
    {
      sienv with
      SearchUtils.sie_log_timings = true;
      SearchUtils.use_ranked_autocomplete;
    }
  in
  let start_time = log_startup_time "symbol_index" start_time in
  if use_ranked_autocomplete then AutocompleteRankService.initialize ();
  let%lwt load_state_result =
    load_saved_state
      (Provider_context.empty_for_tool
         ~popt
         ~tcopt
         ~backend:(Provider_backend.Local_memory local_memory))
      ~root
      ~naming_table_saved_state_path
  in
  let _ = log_startup_time "saved_state" start_time in
  match load_state_result with
  | Ok (naming_table, changed_files) ->
    (* We only ever serve requests on files that are open. That's why our caller
    passes an initial list of open files, the ones already open in the editor
    at the time we were launched. We don't actually care about their contents
    at this stage, since updated contents will be delivered upon each request.
    (and indeed it's pointless to waste time reading existing contents off disk).
    All we care is that every open file is listed in 'open_files'. *)
    let open_files =
      open_files
      |> List.map ~f:(fun path ->
             path |> Path.to_string |> Relative_path.create_detect_prefix)
      |> List.map ~f:(fun path ->
             (path, Provider_context.make_entry ~path ~contents:""))
      |> Relative_path.Map.of_list
    in
    let state =
      Initialized
        {
          hhi_root;
          naming_table;
          sienv;
          changed_files_to_process = Path.Set.of_list changed_files;
          popt;
          tcopt;
          local_memory;
          open_files;
          changed_files_denominator = List.length changed_files;
        }
    in
    log "Serverless IDE has completed initialization";
    Lwt.return_ok state
  | Error error_data ->
    log "Serverless IDE failed to initialize";
    Lwt.return_error error_data

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

let restore_hhi_root_if_necessary (state : initialized_state) :
    initialized_state =
  if Sys.file_exists (Path.to_string state.hhi_root) then
    state
  else
    (* Some processes may clean up the temporary HHI directory we're using.
    Assume that such a process has deleted the directory, and re-write the HHI
    files to disk. *)
    let hhi_root = Hhi.get_hhi_root ~force_write:true () in
    log
      "Old hhi root %s no longer exists. Creating a new hhi root at %s"
      (Path.to_string state.hhi_root)
      (Path.to_string hhi_root);
    Relative_path.set_path_prefix Relative_path.Hhi hhi_root;
    { state with hhi_root }

(** An empty ctx with no entries *)
let make_empty_ctx (initialized_state : initialized_state) : Provider_context.t
    =
  Provider_context.empty_for_tool
    ~popt:initialized_state.popt
    ~tcopt:initialized_state.tcopt
    ~backend:(Provider_backend.Local_memory initialized_state.local_memory)

(** Constructs a temporary ctx with just one entry. *)
let make_singleton_ctx
    (initialized_state : initialized_state) (entry : Provider_context.entry) :
    Provider_context.t =
  let ctx = make_empty_ctx initialized_state in
  let ctx = Provider_context.add_existing_entry ~ctx entry in
  ctx

(** Opens a file, in response to DidOpen event, by putting in a new
entry in open_files, with empty AST and TAST. If the LSP client
happened to send us two DidOpens for a file, well, we won't complain. *)
let open_file
    (initialized_state : initialized_state)
    (path : Relative_path.t)
    (contents : string) : state =
  let initialized_state = restore_hhi_root_if_necessary initialized_state in
  let entry = Provider_context.make_entry ~path ~contents in
  let open_files =
    Relative_path.Map.add initialized_state.open_files path entry
  in
  Initialized { initialized_state with open_files }

(** Closes a file, in response to DidClose event, by removing the
entry in open_files. If the LSP client sents us multile DidCloses,
or DidClose for an unopen file, we won't complain. *)
let close_file (initialized_state : initialized_state) (path : Relative_path.t)
    : state =
  let open_files = Relative_path.Map.remove initialized_state.open_files path in
  Initialized { initialized_state with open_files }

(** Updates an existing opened file, with new contents; if the
contents haven't changed then the existing open file's AST and TAST
will be left intact; if the file wasn't already open then we
throw an exception. *)
let update_file
    (initialized_state : initialized_state)
    (document_location : ClientIdeMessage.document_location) :
    state * Provider_context.t * Provider_context.entry =
  let path =
    document_location.ClientIdeMessage.file_path
    |> Path.to_string
    |> Relative_path.create_detect_prefix
  in
  let entry =
    match
      ( document_location.ClientIdeMessage.file_contents,
        Relative_path.Map.find_opt initialized_state.open_files path )
    with
    | (_, None) ->
      failwith
        ( "Attempted LSP operation on a non-open file"
        ^ Exception.get_current_callstack_string 99 )
    | (Some contents, Some entry)
      when entry.Provider_context.contents = contents ->
      entry
    | (None, Some entry) -> entry
    | (Some contents, _) -> Provider_context.make_entry ~path ~contents
  in
  let open_files =
    Relative_path.Map.add initialized_state.open_files path entry
  in
  let ctx = make_singleton_ctx initialized_state entry in
  (Initialized { initialized_state with open_files }, ctx, entry)

module Handle_message_result = struct
  type 'a t =
    | Notification
    | Response of 'a
    | Error of ClientIdeMessage.error_data
end

let handle_message :
    type a.
    state ->
    string ->
    a ClientIdeMessage.t ->
    (state * a Handle_message_result.t) Lwt.t =
 fun state _tracking_id message ->
  let open ClientIdeMessage in
  match (state, message) with
  | (state, Shutdown ()) ->
    let%lwt () = shutdown state in
    Lwt.return (state, Handle_message_result.Response ())
  | (_, Verbose verbose) ->
    if verbose then
      Hh_logger.Level.set_min_level Hh_logger.Level.Debug
    else
      Hh_logger.Level.set_min_level Hh_logger.Level.Info;
    Lwt.return (state, Handle_message_result.Notification)
  | ((Failed_to_initialize _ | Initializing), File_changed _) ->
    (* Should not happen. *)
    let user_message =
      "IDE services could not process file change because "
      ^ "it failed to initialize or was still initializing. The caller "
      ^ "should have waited for the IDE services to become ready before "
      ^ "sending file-change notifications."
    in
    let stack = Exception.get_current_callstack_string 99 in
    let error_data = ClientIdeMessage.make_error_data user_message ~stack in
    Lwt.return (state, Handle_message_result.Error error_data)
  | (Initialized initialized_state, File_changed path) ->
    (* Only invalidate when a hack file changes *)
    if FindUtils.file_filter (Path.to_string path) then
      let changed_files_to_process =
        Path.Set.add initialized_state.changed_files_to_process path
      in
      let changed_files_denominator =
        initialized_state.changed_files_denominator + 1
      in
      let state =
        Initialized
          {
            initialized_state with
            changed_files_to_process;
            changed_files_denominator;
          }
      in
      Lwt.return (state, Handle_message_result.Notification)
    else
      Lwt.return (state, Handle_message_result.Notification)
  | (Initializing, Initialize_from_saved_state param) ->
    let%lwt result = initialize param in
    begin
      match result with
      | Ok state ->
        let num_changed_files_to_process =
          match state with
          | Initialized { changed_files_to_process; _ } ->
            Path.Set.cardinal changed_files_to_process
          | _ -> 0
        in
        let results =
          {
            ClientIdeMessage.Initialize_from_saved_state
            .num_changed_files_to_process;
          }
        in
        Lwt.return (state, Handle_message_result.Response results)
      | Error error_data ->
        Lwt.return
          ( Failed_to_initialize error_data,
            Handle_message_result.Error error_data )
    end
  | (Initialized _, Initialize_from_saved_state _) ->
    let error_data =
      ClientIdeMessage.make_error_data
        "Tried to initialize when already initialized"
        ~stack:(Exception.get_current_callstack_string 100)
    in
    Lwt.return (state, Handle_message_result.Error error_data)
  | (Initializing, _) ->
    let error_data =
      ClientIdeMessage.make_error_data
        "IDE services have not yet been initialized"
        ~stack:(Exception.get_current_callstack_string 100)
    in
    Lwt.return (state, Handle_message_result.Error error_data)
  | (Failed_to_initialize error_data, _) ->
    let error_data =
      {
        error_data with
        debug_details = "Failed to initialize: " ^ error_data.debug_details;
      }
    in
    Lwt.return (state, Handle_message_result.Error error_data)
  | (Initialized initialized_state, File_closed file_path) ->
    let path =
      file_path |> Path.to_string |> Relative_path.create_detect_prefix
    in
    let state = close_file initialized_state path in
    Lwt.return (state, Handle_message_result.Notification)
  | (Initialized initialized_state, File_opened { file_path; file_contents }) ->
    let path =
      file_path |> Path.to_string |> Relative_path.create_detect_prefix
    in
    let state = open_file initialized_state path file_contents in
    Lwt.return (state, Handle_message_result.Notification)
  | (Initialized initialized_state, Hover document_location) ->
    let (state, ctx, entry) = update_file initialized_state document_location in
    let result =
      Provider_utils.respect_but_quarantine_unsaved_changes ~ctx ~f:(fun () ->
          ServerHover.go_quarantined
            ~ctx
            ~entry
            ~line:document_location.ClientIdeMessage.line
            ~column:document_location.ClientIdeMessage.column)
    in
    Lwt.return (state, Handle_message_result.Response result)
  (* Autocomplete *)
  | ( Initialized initialized_state,
      Completion
        { ClientIdeMessage.Completion.document_location; is_manually_invoked }
    ) ->
    (* Update the state of the world with the document as it exists in the IDE *)
    let (state, ctx, entry) = update_file initialized_state document_location in
    let result =
      ServerAutoComplete.go_ctx
        ~ctx
        ~entry
        ~sienv:initialized_state.sienv
        ~is_manually_invoked
        ~line:document_location.line
        ~column:document_location.column
    in
    Lwt.return (state, Handle_message_result.Response result)
  (* Autocomplete docblock resolve *)
  | (Initialized initialized_state, Completion_resolve param) ->
    let ctx = make_empty_ctx initialized_state in
    ClientIdeMessage.Completion_resolve.(
      let result =
        ServerDocblockAt.go_docblock_for_symbol
          ~ctx
          ~symbol:param.symbol
          ~kind:param.kind
      in
      Lwt.return (state, Handle_message_result.Response result))
  (* Autocomplete docblock resolve *)
  | (Initialized initialized_state, Completion_resolve_location param) ->
    (* We're given a location but it often won't be an opened file.
    We will only serve autocomplete docblocks as of truth on disk.
    Hence, we construct temporary entry to reflect the file which
    contained the target of the resolve. *)
    let open ClientIdeMessage.Completion_resolve_location in
    let path =
      param.document_location.ClientIdeMessage.file_path
      |> Path.to_string
      |> Relative_path.create_detect_prefix
    in
    let ctx = make_empty_ctx initialized_state in
    let (ctx, entry) = Provider_context.add_entry ~ctx ~path in
    let result =
      Provider_utils.respect_but_quarantine_unsaved_changes ~ctx ~f:(fun () ->
          ServerDocblockAt.go_docblock_ctx
            ~ctx
            ~entry
            ~line:param.document_location.line
            ~column:param.document_location.column
            ~kind:param.kind)
    in
    Lwt.return (state, Handle_message_result.Response result)
  (* Document highlighting *)
  | (Initialized initialized_state, Document_highlight document_location) ->
    let (state, ctx, entry) = update_file initialized_state document_location in
    let results =
      Provider_utils.respect_but_quarantine_unsaved_changes ~ctx ~f:(fun () ->
          ServerHighlightRefs.go_quarantined
            ~ctx
            ~entry
            ~line:document_location.line
            ~column:document_location.column)
    in
    Lwt.return (state, Handle_message_result.Response results)
  (* Signature help *)
  | (Initialized initialized_state, Signature_help document_location) ->
    let (state, ctx, entry) = update_file initialized_state document_location in
    let results =
      Provider_utils.respect_but_quarantine_unsaved_changes ~ctx ~f:(fun () ->
          ServerSignatureHelp.go_quarantined
            ~ctx
            ~entry
            ~line:document_location.line
            ~column:document_location.column)
    in
    Lwt.return (state, Handle_message_result.Response results)
  (* Go to definition *)
  | (Initialized initialized_state, Definition document_location) ->
    let (state, ctx, entry) = update_file initialized_state document_location in
    let result =
      Provider_utils.respect_but_quarantine_unsaved_changes ~ctx ~f:(fun () ->
          ServerGoToDefinition.go_quarantined
            ~ctx
            ~entry
            ~line:document_location.ClientIdeMessage.line
            ~column:document_location.ClientIdeMessage.column)
    in
    Lwt.return (state, Handle_message_result.Response result)
  (* Type Definition *)
  | (Initialized initialized_state, Type_definition document_location) ->
    let (state, ctx, entry) = update_file initialized_state document_location in
    let result =
      Provider_utils.respect_but_quarantine_unsaved_changes ~ctx ~f:(fun () ->
          ServerTypeDefinition.go_quarantined
            ~ctx
            ~entry
            ~line:document_location.ClientIdeMessage.line
            ~column:document_location.ClientIdeMessage.column)
    in
    Lwt.return (state, Handle_message_result.Response result)
  (* Document Symbol *)
  | (Initialized initialized_state, Document_symbol document_location) ->
    let (state, ctx, entry) = update_file initialized_state document_location in
    let result = FileOutline.outline_ctx ~ctx ~entry in
    Lwt.return (state, Handle_message_result.Response result)
  (* Type Coverage *)
  | (Initialized initialized_state, Type_coverage document_identifier) ->
    let document_location =
      {
        file_path = document_identifier.file_path;
        file_contents = Some document_identifier.file_contents;
        line = 0;
        column = 0;
      }
    in
    let (state, ctx, entry) = update_file initialized_state document_location in
    let result =
      Provider_utils.respect_but_quarantine_unsaved_changes ~ctx ~f:(fun () ->
          ServerColorFile.go_quarantined ~ctx ~entry)
    in
    Lwt.return (state, Handle_message_result.Response result)

let write_message
    ~(out_fd : Lwt_unix.file_descr)
    ~(message : ClientIdeMessage.message_from_daemon) : unit Lwt.t =
  let%lwt (_ : int) = Marshal_tools_lwt.to_fd_with_preamble out_fd message in
  Lwt.return_unit

let write_status ~(out_fd : Lwt_unix.file_descr) (state : state) : unit Lwt.t =
  match state with
  | Initializing
  | Failed_to_initialize _ ->
    Lwt.return_unit
  | Initialized { changed_files_to_process; changed_files_denominator; _ } ->
    if Path.Set.is_empty changed_files_to_process then
      let%lwt () =
        write_message
          ~out_fd
          ~message:
            (ClientIdeMessage.Notification ClientIdeMessage.Done_processing)
      in
      Lwt.return_unit
    else
      let total = changed_files_denominator in
      let processed = total - Path.Set.cardinal changed_files_to_process in
      let%lwt () =
        write_message
          ~out_fd
          ~message:
            (ClientIdeMessage.Notification
               (ClientIdeMessage.Processing_files
                  { ClientIdeMessage.Processing_files.processed; total }))
      in
      Lwt.return_unit

let serve ~(in_fd : Lwt_unix.file_descr) ~(out_fd : Lwt_unix.file_descr) :
    unit Lwt.t =
  let rec flush_event_logger () : unit Lwt.t =
    let%lwt () = Lwt_unix.sleep 0.5 in
    Lwt.async EventLoggerLwt.flush;
    flush_event_logger ()
  in
  let rec pump_message_queue (message_queue : message_queue) : unit Lwt.t =
    try%lwt
      let%lwt { ClientIdeMessage.tracking_id; message } =
        Marshal_tools_lwt.from_fd_with_preamble in_fd
      in
      let is_queue_open =
        Lwt_message_queue.push
          message_queue
          (Message { ClientIdeMessage.tracking_id; message })
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
     state =
       Initialized
         ( {
             naming_table;
             sienv;
             changed_files_to_process;
             local_memory;
             popt;
             open_files;
             _;
           } as state );
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
      let%lwt { ClientIdeIncremental.naming_table; sienv; old_file_info; _ } =
        try%lwt
          ClientIdeIncremental.update_naming_tables_for_changed_file
            ~backend:(Provider_backend.Local_memory local_memory)
            ~popt
            ~naming_table
            ~sienv
            ~path:next_file
        with exn ->
          let e = Exception.wrap exn in
          HackEventLogger.uncaught_exception exn;
          Hh_logger.exception_
            e
            ~prefix:
              (Printf.sprintf
                 "Uncaught exception when processing changed file: %s"
                 (Path.to_string next_file));
          Lwt.return
            {
              ClientIdeIncremental.naming_table;
              sienv;
              old_file_info = None;
              new_file_info = None;
            }
      in
      Option.iter
        old_file_info
        ~f:(Provider_utils.invalidate_local_decl_caches_for_file local_memory);
      Provider_utils.invalidate_tast_cache_of_entries open_files;
      let%lwt state =
        if Path.Set.is_empty changed_files_to_process then
          Lwt.return
            (Initialized
               {
                 state with
                 naming_table;
                 sienv;
                 changed_files_to_process;
                 changed_files_denominator = 0;
               })
        else
          Lwt.return
            (Initialized
               { state with naming_table; sienv; changed_files_to_process })
      in
      let%lwt () = write_status ~out_fd state in
      handle_messages { t with state }
    | t ->
      let%lwt message = Lwt_message_queue.pop t.message_queue in
      (match message with
      | None -> Lwt.return_unit
      | Some (Message { ClientIdeMessage.tracking_id; message }) ->
        let unblocked_time = Unix.gettimeofday () in
        let%lwt state =
          try%lwt
            let%lwt (state, response) =
              handle_message t.state tracking_id message
            in
            match response with
            | Handle_message_result.Notification ->
              (* No response needed for notifications. *)
              Lwt.return state
            | Handle_message_result.Response response ->
              let message =
                ClientIdeMessage.Response
                  { ClientIdeMessage.response = Ok response; unblocked_time }
              in
              let%lwt () = write_message ~out_fd ~message in
              Lwt.return state
            | Handle_message_result.Error error_data ->
              let message =
                ClientIdeMessage.Response
                  {
                    ClientIdeMessage.response = Error error_data;
                    unblocked_time;
                  }
              in
              let%lwt () = write_message ~out_fd ~message in
              Lwt.return state
          with e ->
            let stack = e |> Exception.wrap |> Exception.get_backtrace_string in
            let prefix = "Exception while handling message" in
            Hh_logger.exc e ~prefix ~stack;
            let debug_details = prefix ^ ": " ^ Exn.to_string e in
            let error_data =
              ClientIdeMessage.make_error_data debug_details ~stack
            in

            (* If we were responding to a message, but threw an exception, write
            that exception as a response. *)
            let message =
              ClientIdeMessage.Response
                { ClientIdeMessage.response = Error error_data; unblocked_time }
            in
            let%lwt () = write_message ~out_fd ~message in
            Lwt.return t.state
        in
        handle_messages { t with state })
  in
  try%lwt
    let message_queue = Lwt_message_queue.create () in
    let flusher_promise = flush_event_logger () in
    let%lwt () = handle_messages { message_queue; state = Initializing }
    and () = pump_message_queue message_queue in
    Lwt.cancel flusher_promise;
    Lwt.return_unit
  with e ->
    let e = Exception.wrap e in
    log "Exception occurred while handling RPC call: %s" (Exception.to_string e);
    Lwt.return_unit

let daemon_main
    (args : ClientIdeMessage.daemon_args)
    (channels : ('a, 'b) Daemon.channel_pair) : unit =
  Printexc.record_backtrace true;
  let (ic, oc) = channels in
  let in_fd = Lwt_unix.of_unix_file_descr (Daemon.descr_of_in_channel ic) in
  let out_fd = Lwt_unix.of_unix_file_descr (Daemon.descr_of_out_channel oc) in
  let daemon_init_id =
    Printf.sprintf
      "%s.%s"
      args.ClientIdeMessage.init_id
      (Random_id.short_string ())
  in
  HackEventLogger.serverless_ide_init ~init_id:daemon_init_id;
  Hh_logger.Level.set_min_level_file Hh_logger.Level.Info;
  Hh_logger.Level.set_min_level_stderr Hh_logger.Level.Error;
  if args.ClientIdeMessage.verbose then
    Hh_logger.Level.set_min_level Hh_logger.Level.Debug;
  Lwt_main.run (serve ~in_fd ~out_fd)

let daemon_entry_point : (ClientIdeMessage.daemon_args, unit, unit) Daemon.entry
    =
  Daemon.register_entry_point "ClientIdeService" daemon_main
