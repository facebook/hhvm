(*
 * Copyright (c) 2019, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Hh_prelude

(** This is the result type from attempting to load saved-state.
In the error case, [stopped_reason] is a human-facing response,
and [Lsp.Error.t] contains structured telemetry data. *)
type load_saved_state_result =
  ( Naming_table.t * Saved_state_loader.changed_files,
    ClientIdeMessage.stopped_reason * Lsp.Error.t )
  result

(** These are messages on ClientIdeDaemon's internal message-queue *)
type message =
  | ClientRequest : 'a ClientIdeMessage.tracked_t -> message
      (** ClientRequest came from ClientIdeService over stdin;
      it expects a response. *)
  | LoadedState : load_saved_state_result -> message
      (** LoadedState is posted from within ClientIdeDaemon itself once
      our attempt at loading saved-state has finished; it's picked
      up by handle_messages. *)

type message_queue = message Lwt_message_queue.t

exception Outfd_write_error

let is_outfd_write_error (exn : Exception.t) : bool =
  match Exception.unwrap exn with
  | Outfd_write_error -> true
  | _ -> false

(** istate, "initialized state", is the state the daemon after it has
finished initialization (i.e. finished loading saved state),
concerning these data-structures:
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
2. They are updated asynchronously by update_naming_tables_for_changed_file_lwt
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
type istate = {
  icommon: common_state;
  ifiles: open_files_state;
  naming_table: Naming_table.t;
      (** the forward-naming-table is constructed during initialize and updated
      during process_changed_files. It stores an in-memory map of FileInfos that
      have changed since sqlite. When a file is changed on disk, we need this to
      know which shallow decls to invalidate. Note: while the forward-naming-table
      is stored here, the reverse-naming-table is instead stored in ctx. *)
}

(** dstate, "during_init state", is the state the daemon after it has received an
init message (and has parsed config files to get popt/tcopt, has initialized
glean, as written out hhi files) but before it has loaded saved-state or processed
file updates. *)
and dstate = {
  start_time: float;
      (** When did we kick off the attempt to load saved-state? *)
  dcommon: common_state;
  dfiles: open_files_state;
}

and common_state = {
  hhi_root: Path.t;
      (** hhi_root files are written during initialize, deleted at shutdown, and
      refreshed periodically in case the tmp-cleaner has deleted them. *)
  sienv: SearchUtils.si_env;
      (** sienv provides autocomplete and find-symbols. It is constructed during
      initialization and stores a few in-memory structures such as namespace-list,
      plus in-memory deltas. It is also updated during process_changed_files. *)
  popt: ParserOptions.t;  (** parser options *)
  tcopt: TypecheckerOptions.t;  (** typechecker options *)
  local_memory: Provider_backend.local_memory;
      (** Local_memory backend; includes decl caches *)
}

and open_files_state = {
  open_files: Provider_context.entries;
      (** all open files, along with caches of their ASTs and TASTs and errors *)
  changed_files_to_process: Relative_path.Set.t;
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
  | Pending_init  (** We haven't yet received init request *)
  | During_init of dstate  (** We're working on the init request *)
  | Initialized of istate  (** Finished work on init request *)
  | Failed_init of Lsp.Error.t  (** Failed request, with root cause *)

type t = {
  message_queue: message_queue;
  state: state;
}

let state_to_log_string (state : state) : string =
  let files_to_log_string (files : open_files_state) : string =
    Printf.sprintf
      "%d open_files; %d changed_files_to_process"
      (Relative_path.Map.cardinal files.open_files)
      (Relative_path.Set.cardinal files.changed_files_to_process)
  in
  match state with
  | Pending_init -> "Pending_init"
  | During_init { dfiles; _ } ->
    Printf.sprintf "During_init(%s)" (files_to_log_string dfiles)
  | Initialized { ifiles; _ } ->
    Printf.sprintf "Initialized(%s)" (files_to_log_string ifiles)
  | Failed_init e -> Printf.sprintf "Failed_init(%s)" e.Lsp.Error.message

let log s = Hh_logger.log ("[ide-daemon] " ^^ s)

let log_debug s = Hh_logger.debug ("[ide-daemon] " ^^ s)

let set_up_hh_logger_for_client_ide_service (root : Path.t) : unit =
  (* Log to a file on disk. Note that calls to `Hh_logger` will always write to
  `stderr`; this is in addition to that. *)
  let client_ide_log_fn = ServerFiles.client_ide_log root in
  begin
    try Sys.rename client_ide_log_fn (client_ide_log_fn ^ ".old")
    with _e -> ()
  end;
  Hh_logger.set_log client_ide_log_fn;
  log "Starting client IDE service at %s" client_ide_log_fn

let write_message
    ~(out_fd : Lwt_unix.file_descr)
    ~(message : ClientIdeMessage.message_from_daemon) : unit Lwt.t =
  try%lwt
    let%lwt (_ : int) = Marshal_tools_lwt.to_fd_with_preamble out_fd message in
    Lwt.return_unit
  with Unix.Unix_error (Unix.EPIPE, _, _) -> raise Outfd_write_error

let load_saved_state
    (ctx : Provider_context.t)
    ~(root : Path.t)
    ~(naming_table_load_info :
       ClientIdeMessage.Initialize_from_saved_state.naming_table_load_info
       option) : load_saved_state_result Lwt.t =
  log "[saved-state] Starting load in root %s" (Path.to_string root);
  let%lwt result =
    try%lwt
      let%lwt result =
        match naming_table_load_info with
        | Some naming_table_load_info ->
          let open ClientIdeMessage.Initialize_from_saved_state in
          (* tests may wish to pretend there's a delay *)
          let%lwt () =
            if Float.(naming_table_load_info.test_delay > 0.0) then
              Lwt_unix.sleep naming_table_load_info.test_delay
            else
              Lwt.return_unit
          in
          (* Assume that there are no changed files on disk if we're getting
          passed the path to the saved-state directly, and that the saved-state
          corresponds to the current state of the world. *)
          let changed_files = [] in
          (* Test hook, for tests that want to get messages in before init *)
          Lwt.return_ok
            {
              Saved_state_loader.main_artifacts =
                {
                  Saved_state_loader.Naming_table_info.naming_table_path =
                    naming_table_load_info.path;
                };
              additional_info = ();
              changed_files;
              manifold_path = "<not provided>";
              corresponding_rev = "<not provided>";
              mergebase_rev = "<not provided>";
              is_cached = true;
            }
        | None ->
          let%lwt result =
            State_loader_lwt.load
              ~watchman_opts:
                Saved_state_loader.Watchman_options.{ root; sockname = None }
              ~ignore_hh_version:false
              ~saved_state_type:Saved_state_loader.Naming_table
          in
          Lwt.return result
      in
      match result with
      | Ok { Saved_state_loader.main_artifacts; changed_files; _ } ->
        let path =
          Path.to_string
            main_artifacts
              .Saved_state_loader.Naming_table_info.naming_table_path
        in
        log "[saved-state] Loading naming-table... %s" path;
        let naming_table = Naming_table.load_from_sqlite ctx path in
        log "[saved-state] Loaded naming-table.";
        (* Track how many files we have to change locally *)
        HackEventLogger.serverless_ide_local_files
          ~local_file_count:(List.length changed_files);

        Lwt.return_ok (naming_table, changed_files)
      | Error load_error ->
        (* We'll turn that load_error into a user-facing [reason], and a
        programmatic error [e] for future telemetry *)
        let reason =
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
        in
        let e =
          {
            Lsp.Error.code = Lsp.Error.UnknownErrorCode;
            message = reason.ClientIdeMessage.medium_user_message;
            data =
              Some
                (Hh_json.JSON_Object
                   [
                     ( "debug_details",
                       Hh_json.string_ reason.ClientIdeMessage.debug_details );
                   ]);
          }
        in
        Lwt.return_error (reason, e)
    with exn ->
      let exn = Exception.wrap exn in
      ClientIdeUtils.log_bug "load_exn" ~exn ~telemetry:false;
      (* We need both a user-facing "reason" and an internal error "e" *)
      let reason = ClientIdeUtils.make_bug_reason "load_exn" ~exn in
      let e = ClientIdeUtils.make_bug_error "load_exn" ~exn in
      Lwt.return_error (reason, e)
  in
  Lwt.return result

let log_startup_time (component : string) (start_time : float) : float =
  let now = Unix.gettimeofday () in
  HackEventLogger.serverless_ide_startup ~component ~start_time;
  now

let restore_hhi_root_if_necessary (istate : istate) : istate =
  if Sys.file_exists (Path.to_string istate.icommon.hhi_root) then
    istate
  else
    (* Some processes may clean up the temporary HHI directory we're using.
    Assume that such a process has deleted the directory, and re-write the HHI
    files to disk. *)
    let hhi_root = Hhi.get_hhi_root ~force_write:true () in
    log
      "Old hhi root %s no longer exists. Creating a new hhi root at %s"
      (Path.to_string istate.icommon.hhi_root)
      (Path.to_string hhi_root);
    Relative_path.set_path_prefix Relative_path.Hhi hhi_root;
    { istate with icommon = { istate.icommon with hhi_root } }

(** Deletes the hhi files we've created. *)
let remove_hhi (state : state) : unit =
  match state with
  | Pending_init
  | Failed_init _ ->
    ()
  | During_init { dcommon = { hhi_root; _ }; _ }
  | Initialized { icommon = { hhi_root; _ }; _ } ->
    let hhi_root = Path.to_string hhi_root in
    log "Removing hhi directory %s..." hhi_root;
    (try Sys_utils.rm_dir_tree hhi_root
     with exn ->
       let exn = Exception.wrap exn in
       ClientIdeUtils.log_bug "remove_hhi" ~exn ~telemetry:true)

(** initialize1 is called by handle_request upon receipt of an "init"
message from the client. It is synchronous. It sets up global variables and
glean. The remainder of init work will happen after we return... our caller
handle_request will kick off async work to load saved-state, and once done
it will stick a LoadedState message into the queue, and handle_one_message
will subsequently pick up that message and call [initialize2]. *)
let initialize1 (param : ClientIdeMessage.Initialize_from_saved_state.t) :
    dstate =
  log_debug "initialize1";
  let open ClientIdeMessage.Initialize_from_saved_state in
  let start_time = Unix.gettimeofday () in
  HackEventLogger.serverless_ide_set_root param.root;
  set_up_hh_logger_for_client_ide_service param.root;

  Relative_path.set_path_prefix Relative_path.Root param.root;
  let hhi_root = Hhi.get_hhi_root () in
  log "Extracted hhi files to directory %s" (Path.to_string hhi_root);
  Relative_path.set_path_prefix Relative_path.Hhi hhi_root;
  Relative_path.set_path_prefix Relative_path.Tmp (Path.make "/tmp");

  let server_args =
    ServerArgs.default_options_with_check_mode ~root:(Path.to_string param.root)
  in
  let server_args = ServerArgs.set_config server_args param.config in
  let (config, local_config) =
    ServerConfig.load ~silent:true ServerConfig.filename server_args
  in
  HackEventLogger.set_hhconfig_version
    (ServerConfig.version config |> Config_file.version_to_string_opt);
  HackEventLogger.set_rollout_flags
    (ServerLocalConfig.to_rollout_flags local_config);

  Provider_backend.set_local_memory_backend_with_defaults ();
  let local_memory =
    match Provider_backend.get () with
    | Provider_backend.Local_memory local_memory -> local_memory
    | _ -> failwith "expected local memory backend"
  in

  (* Use config to modify server_env with the correct symbol index *)
  let genv = ServerEnvBuild.make_genv server_args config local_config [] in
  let init_id = Random_id.short_string () in
  let { ServerEnv.tcopt; popt; gleanopt; _ } =
    (* TODO(hverr): Figure out 64-bit mode *)
    ServerEnvBuild.make_env
      ~init_id
      ~deps_mode:Typing_deps_mode.SQLiteMode
      genv.ServerEnv.config
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
        local_config.ServerLocalConfig.ide_symbolindex_search_provider
      ~quiet:local_config.ServerLocalConfig.symbolindex_quiet
      ~ignore_hh_version:false
      ~savedstate_file_opt:local_config.ServerLocalConfig.symbolindex_file
      ~workers:None
  in
  let sienv =
    {
      sienv with
      SearchUtils.sie_log_timings = true;
      SearchUtils.use_ranked_autocomplete = param.use_ranked_autocomplete;
    }
  in
  if param.use_ranked_autocomplete then AutocompleteRankService.initialize ();
  let start_time = log_startup_time "symbol_index" start_time in
  (* We only ever serve requests on files that are open. That's why our caller
  passes an initial list of open files, the ones already open in the editor
  at the time we were launched. We don't actually care about their contents
  at this stage, since updated contents will be delivered upon each request.
  (and indeed it's pointless to waste time reading existing contents off disk).
  All we care is that every open file is listed in 'open_files'. *)
  let open_files =
    param.open_files
    |> List.map ~f:(fun path ->
           path |> Path.to_string |> Relative_path.create_detect_prefix)
    |> List.map ~f:(fun path ->
           ( path,
             Provider_context.make_entry
               ~path
               ~contents:Provider_context.Raise_exn_on_attempt_to_read ))
    |> Relative_path.Map.of_list
  in
  log_debug "initialize1.done";
  {
    start_time;
    dcommon = { hhi_root; sienv; popt; tcopt; local_memory };
    dfiles =
      {
        open_files;
        changed_files_to_process = Relative_path.Set.empty;
        changed_files_denominator = 0;
      };
  }

(** initialize2 is called by handle_one_message upon receipt of a
[LoadedState] message. It sends the appropriate message on to the
client, and transitions into either [Initialized] or [Failed_init]
state. *)
let initialize2
    (out_fd : Lwt_unix.file_descr)
    (dstate : dstate)
    (load_state_result : load_saved_state_result) : state Lwt.t =
  let (_ : float) = log_startup_time "saved_state" dstate.start_time in
  log_debug "initialize2";
  match load_state_result with
  | Ok (naming_table, changed_files) ->
    let changed_files_to_process =
      Relative_path.Set.union
        dstate.dfiles.changed_files_to_process
        (Relative_path.Set.of_list changed_files)
    in
    let changed_files_denominator =
      Relative_path.Set.cardinal changed_files_to_process
    in
    let p = { ClientIdeMessage.Processing_files.total = 0; processed = 0 } in
    let%lwt () =
      write_message
        ~out_fd
        ~message:
          (ClientIdeMessage.Notification (ClientIdeMessage.Done_init (Ok p)))
    in
    let istate =
      {
        naming_table;
        icommon = dstate.dcommon;
        ifiles =
          {
            open_files = dstate.dfiles.open_files;
            changed_files_to_process;
            changed_files_denominator;
          };
      }
    in
    log_debug "initialize2.done";
    Lwt.return (Initialized istate)
  | Error (reason, e) ->
    log_debug "initialize2.error";
    let%lwt () =
      write_message
        ~out_fd
        ~message:
          (ClientIdeMessage.Notification
             (ClientIdeMessage.Done_init (Error reason)))
    in
    remove_hhi (During_init dstate);
    Lwt.return (Failed_init e)

(** An empty ctx with no entries *)
let make_empty_ctx (istate : istate) : Provider_context.t =
  (* TODO(hverr): Support 64-bit *)
  Provider_context.empty_for_tool
    ~popt:istate.icommon.popt
    ~tcopt:istate.icommon.tcopt
    ~backend:(Provider_backend.Local_memory istate.icommon.local_memory)
    ~deps_mode:Typing_deps_mode.SQLiteMode

(** Constructs a temporary ctx with just one entry. *)
let make_singleton_ctx (istate : istate) (entry : Provider_context.entry) :
    Provider_context.t =
  let ctx = make_empty_ctx istate in
  let ctx = Provider_context.add_or_overwrite_entry ~ctx entry in
  ctx

(** This funtion is about papering over a bug. Sometimes, rarely, we're
failing to receive DidOpen messages from clientLsp. Our model is to
only ever answer IDE requests on open files, so we know we'll eventually
reveive a DidClose even for them and be able to clear their TAST cache
at that time. But for now, to paper over the bug, we'll call this
function to log the event and we'll assume that we just missed a DidOpen. *)
let log_missing_open_file_BUG (path : Relative_path.t) : unit =
  let path = Relative_path.to_absolute path in
  let message = Printf.sprintf "Error: action on non-open file %s" path in
  ClientIdeUtils.log_bug message ~telemetry:true

(** Opens a file, in response to DidOpen event, by putting in a new
entry in open_files, with empty AST and TAST. If the LSP client
happened to send us two DidOpens for a file, well, we won't complain. *)
let open_file
    (files : open_files_state) (path : Relative_path.t) (contents : string) :
    open_files_state =
  let entry =
    Provider_context.make_entry
      ~path
      ~contents:(Provider_context.Provided_contents contents)
  in
  let open_files =
    Relative_path.Map.add files.open_files ~key:path ~data:entry
  in
  { files with open_files }

(** Changes a file, in response to DidChange event. For future we
might switch ClientIdeDaemon to incremental change events. But for
now, this is basically a no-op just with some error checking. *)
let change_file (files : open_files_state) (path : Relative_path.t) :
    open_files_state =
  if Relative_path.Map.mem files.open_files path then
    files
  else
    (* We'll now mark the file as opened. We'll provide empty contents for now;
    this doesn't matter since every actual future request for the file will provide
    actual contents. *)
    let () = log_missing_open_file_BUG path in
    open_file files path ""

(** Closes a file, in response to DidClose event, by removing the
entry in open_files. If the LSP client sents us multile DidCloses,
or DidClose for an unopen file, we won't complain. *)
let close_file (files : open_files_state) (path : Relative_path.t) :
    open_files_state =
  let open_files = Relative_path.Map.remove files.open_files path in
  { files with open_files }

(** Updates an existing opened file, with new contents; if the
contents haven't changed then the existing open file's AST and TAST
will be left intact; if the file wasn't already open then we
throw an exception. *)
let update_file
    (files : open_files_state)
    (document_location : ClientIdeMessage.document_location) :
    open_files_state * Provider_context.entry =
  let path =
    document_location.ClientIdeMessage.file_path
    |> Path.to_string
    |> Relative_path.create_detect_prefix
  in
  let entry =
    match
      ( document_location.ClientIdeMessage.file_contents,
        Relative_path.Map.find_opt files.open_files path )
    with
    | (Some contents, None) ->
      log_missing_open_file_BUG path;
      (* TODO(ljw): failwith "Attempted LSP operation on a non-open file" *)
      Provider_context.make_entry
        ~path
        ~contents:(Provider_context.Provided_contents contents)
    | (None, None) ->
      log_missing_open_file_BUG path;
      failwith "Attempted LSP operation on a non-open file"
    | (Some contents, Some entry)
      when Option.equal
             String.equal
             (Some contents)
             (Provider_context.get_file_contents_if_present entry) ->
      entry
    | (None, Some entry) -> entry
    | (Some contents, _) ->
      Provider_context.make_entry
        ~path
        ~contents:(Provider_context.Provided_contents contents)
  in
  let open_files =
    Relative_path.Map.add files.open_files ~key:path ~data:entry
  in
  ({ files with open_files }, entry)

(** like [update_file], but for convenience also produces a ctx for
use in typechecking. Also ensures that hhi files haven't been deleted
by tmp_cleaner, so that type-checking will succeed. *)
let update_file_ctx
    (istate : istate) (document_location : ClientIdeMessage.document_location) :
    state * Provider_context.t * Provider_context.entry =
  let istate = restore_hhi_root_if_necessary istate in
  let (ifiles, entry) = update_file istate.ifiles document_location in
  let ctx = make_singleton_ctx istate entry in
  (Initialized { istate with ifiles }, ctx, entry)

(** Simple helper. It updates the [ifiles] or [dfiles] member of Initialized
or During_init states, respectively. Will throw if you call it on any other
state. *)
let update_state_files (state : state) (files : open_files_state) : state =
  match state with
  | During_init dstate -> During_init { dstate with dfiles = files }
  | Initialized istate -> Initialized { istate with ifiles = files }
  | _ -> failwith ("Update_state_files: unexpected " ^ state_to_log_string state)

(** handle_request invariants: Messages are only ever handled serially; we never
handle one message while another is being handled. It is a bug if the client sends
anything other than [Initialize_from_saved_state] as its first message. Upon
receipt+processing of this we transition from [Pre_init] to [During_init]
and kick off some async work to load saved state. During this async work, i.e.
during [During_init], we are able to handle a few requests but will reject
others. Our caller [handle_one_message] is actually the one that transitions
us from [During_init] to either [Failed_init] or [Initialized]. Once in one
of those states, we never thereafter transition state. *)
let handle_request :
    type a.
    message_queue ->
    state ->
    string ->
    a ClientIdeMessage.t ->
    (state * (a, Lsp.Error.t) result) Lwt.t =
 fun message_queue state _tracking_id message ->
  let open ClientIdeMessage in
  match (state, message) with
  (***********************************************************)
  (************************* HANDLED IN ANY STATE ************)
  (***********************************************************)
  | (_, Verbose_to_file verbose) ->
    if verbose then
      Hh_logger.Level.set_min_level_file Hh_logger.Level.Debug
    else
      Hh_logger.Level.set_min_level_file Hh_logger.Level.Info;
    Lwt.return (state, Ok ())
  | (_, Shutdown ()) ->
    remove_hhi state;
    Lwt.return (state, Ok ())
  (***********************************************************)
  (************************* INITIALIZATION ******************)
  (***********************************************************)
  | (Pending_init, Initialize_from_saved_state param) ->
    (* Invariant: no message will be sent to us prior to this request,
    and we must send no message until we've sent this response. *)
    let open Initialize_from_saved_state in
    begin
      try
        let dstate = initialize1 param in
        (* We're going to kick off the asynchronous part of initializing now.
        Once it's done, it will appear as a LoadedState message on the queue. *)
        Lwt.async (fun () ->
            (* following method never throws *)
            (* TODO(hverr): Figure out how to support 64-bit *)
            let%lwt result =
              load_saved_state
                (Provider_context.empty_for_tool
                   ~popt:dstate.dcommon.popt
                   ~tcopt:dstate.dcommon.tcopt
                   ~backend:
                     (Provider_backend.Local_memory dstate.dcommon.local_memory)
                   ~deps_mode:Typing_deps_mode.SQLiteMode)
                ~root:param.root
                ~naming_table_load_info:param.naming_table_load_info
            in
            (* if the following push fails, that must be because the queues
            have been shut down, in which case there's nothing to do. *)
            let (_succeeded : bool) =
              Lwt_message_queue.push message_queue (LoadedState result)
            in
            Lwt.return_unit);
        Lwt.return (During_init dstate, Ok ())
      with exn ->
        let exn = Exception.wrap exn in
        let e = ClientIdeUtils.make_bug_error "initialize1" ~exn in
        (* Our caller has an exception handler. But we must handle this ourselves
        to change state to Failed_init; our caller's handler doesn't change state. *)
        (* TODO: remove_hhi *)
        Lwt.return (Failed_init e, Error e)
    end
  | (_, Initialize_from_saved_state _) ->
    failwith ("Unexpected init in " ^ state_to_log_string state)
  (***********************************************************)
  (************************* CAN HANDLE DURING INIT **********)
  (***********************************************************)
  | ( (During_init { dfiles = files; _ } | Initialized { ifiles = files; _ }),
      Disk_files_changed paths ) ->
    let paths =
      List.filter paths ~f:(fun (Changed_file path) ->
          FindUtils.file_filter path)
    in
    (* That filtered-out non-hack files *)
    let files =
      {
        files with
        changed_files_to_process =
          List.fold
            paths
            ~init:files.changed_files_to_process
            ~f:(fun acc (Changed_file path) ->
              Relative_path.Set.add
                acc
                (Relative_path.create_detect_prefix path));
        changed_files_denominator =
          files.changed_files_denominator + List.length paths;
      }
    in
    Lwt.return (update_state_files state files, Ok ())
  | ( (During_init { dfiles = files; _ } | Initialized { ifiles = files; _ }),
      Ide_file_closed file_path ) ->
    let path =
      file_path |> Path.to_string |> Relative_path.create_detect_prefix
    in
    let files = close_file files path in
    Lwt.return (update_state_files state files, Ok ())
  | ( (During_init { dfiles = files; _ } | Initialized { ifiles = files; _ }),
      Ide_file_opened { file_path; file_contents } ) ->
    let path =
      file_path |> Path.to_string |> Relative_path.create_detect_prefix
    in
    let files = open_file files path file_contents in
    Lwt.return (update_state_files state files, Ok ())
  | ( (During_init { dfiles = files; _ } | Initialized { ifiles = files; _ }),
      Ide_file_changed { Ide_file_changed.file_path; _ } ) ->
    let path =
      file_path |> Path.to_string |> Relative_path.create_detect_prefix
    in
    let files = change_file files path in
    Lwt.return (update_state_files state files, Ok ())
  (* Document Symbol *)
  | ( ( During_init { dfiles = files; dcommon = common; _ }
      | Initialized { ifiles = files; icommon = common; _ } ),
      Document_symbol document_location ) ->
    let (files, entry) = update_file files document_location in
    let result =
      FileOutline.outline_entry_no_comments ~popt:common.popt ~entry
    in
    Lwt.return (update_state_files state files, Ok result)
  (***********************************************************)
  (************************* UNABLE TO HANDLE ****************)
  (***********************************************************)
  | (During_init _, _) ->
    let e =
      {
        Lsp.Error.code = Lsp.Error.RequestCancelled;
        message = "IDE service has not yet completed init";
        data = None;
      }
    in
    Lwt.return (state, Error e)
  | (Failed_init e, _) -> Lwt.return (state, Error e)
  | (Pending_init, _) ->
    failwith
      (Printf.sprintf
         "unexpected message '%s' in state '%s'"
         (ClientIdeMessage.t_to_string message)
         (state_to_log_string state))
  (***********************************************************)
  (************************* NORMAL HANDLING AFTER INIT ******)
  (***********************************************************)
  | (Initialized istate, Hover document_location) ->
    let (state, ctx, entry) = update_file_ctx istate document_location in
    let result =
      Provider_utils.respect_but_quarantine_unsaved_changes ~ctx ~f:(fun () ->
          ServerHover.go_quarantined
            ~ctx
            ~entry
            ~line:document_location.ClientIdeMessage.line
            ~column:document_location.ClientIdeMessage.column)
    in
    Lwt.return (state, Ok result)
  (* Autocomplete *)
  | ( Initialized istate,
      Completion
        { ClientIdeMessage.Completion.document_location; is_manually_invoked }
    ) ->
    (* Update the state of the world with the document as it exists in the IDE *)
    let (state, ctx, entry) = update_file_ctx istate document_location in
    let result =
      ServerAutoComplete.go_ctx
        ~ctx
        ~entry
        ~sienv:istate.icommon.sienv
        ~is_manually_invoked
        ~line:document_location.line
        ~column:document_location.column
    in
    Lwt.return (state, Ok result)
  (* Autocomplete docblock resolve *)
  | (Initialized istate, Completion_resolve param) ->
    let ctx = make_empty_ctx istate in
    ClientIdeMessage.Completion_resolve.(
      let result =
        ServerDocblockAt.go_docblock_for_symbol
          ~ctx
          ~symbol:param.symbol
          ~kind:param.kind
      in
      Lwt.return (state, Ok result))
  (* Autocomplete docblock resolve *)
  | (Initialized istate, Completion_resolve_location param) ->
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
    let ctx = make_empty_ctx istate in
    let (ctx, entry) = Provider_context.add_entry_if_missing ~ctx ~path in
    let result =
      Provider_utils.respect_but_quarantine_unsaved_changes ~ctx ~f:(fun () ->
          ServerDocblockAt.go_docblock_ctx
            ~ctx
            ~entry
            ~line:param.document_location.line
            ~column:param.document_location.column
            ~kind:param.kind)
    in
    Lwt.return (state, Ok result)
  (* Document highlighting *)
  | (Initialized istate, Document_highlight document_location) ->
    let (state, ctx, entry) = update_file_ctx istate document_location in
    let results =
      Provider_utils.respect_but_quarantine_unsaved_changes ~ctx ~f:(fun () ->
          ServerHighlightRefs.go_quarantined
            ~ctx
            ~entry
            ~line:document_location.line
            ~column:document_location.column)
    in
    Lwt.return (state, Ok results)
  (* Signature help *)
  | (Initialized istate, Signature_help document_location) ->
    let (state, ctx, entry) = update_file_ctx istate document_location in
    let results =
      Provider_utils.respect_but_quarantine_unsaved_changes ~ctx ~f:(fun () ->
          ServerSignatureHelp.go_quarantined
            ~ctx
            ~entry
            ~line:document_location.line
            ~column:document_location.column)
    in
    Lwt.return (state, Ok results)
  (* Go to definition *)
  | (Initialized istate, Definition document_location) ->
    let (state, ctx, entry) = update_file_ctx istate document_location in
    let result =
      Provider_utils.respect_but_quarantine_unsaved_changes ~ctx ~f:(fun () ->
          ServerGoToDefinition.go_quarantined
            ~ctx
            ~entry
            ~line:document_location.ClientIdeMessage.line
            ~column:document_location.ClientIdeMessage.column)
    in
    Lwt.return (state, Ok result)
  (* Type Definition *)
  | (Initialized istate, Type_definition document_location) ->
    let (state, ctx, entry) = update_file_ctx istate document_location in
    let result =
      Provider_utils.respect_but_quarantine_unsaved_changes ~ctx ~f:(fun () ->
          ServerTypeDefinition.go_quarantined
            ~ctx
            ~entry
            ~line:document_location.ClientIdeMessage.line
            ~column:document_location.ClientIdeMessage.column)
    in
    Lwt.return (state, Ok result)
  (* Type Coverage *)
  | (Initialized istate, Type_coverage document_identifier) ->
    let document_location =
      {
        file_path = document_identifier.file_path;
        file_contents = Some document_identifier.file_contents;
        line = 0;
        column = 0;
      }
    in
    let (state, ctx, entry) = update_file_ctx istate document_location in
    let result =
      Provider_utils.respect_but_quarantine_unsaved_changes ~ctx ~f:(fun () ->
          ServerColorFile.go_quarantined ~ctx ~entry)
    in
    Lwt.return (state, Ok result)
  (* Workspace Symbol *)
  | (Initialized istate, Workspace_symbol query) ->
    (* Note: needs reverse-naming-table, hence only works in initialized
    state: for top-level queries it needs reverse-naming-table to look
    up positions; for member queries "Foo::bar" it needs it to fetch the
    decl for Foo. *)
    (* Note: we intentionally don't give results from unsaved files *)
    let ctx = make_empty_ctx istate in
    let result =
      ServerSearch.go ctx query ~kind_filter:"" istate.icommon.sienv
    in
    Lwt.return (state, Ok result)

let write_status ~(out_fd : Lwt_unix.file_descr) (state : state) : unit Lwt.t =
  match state with
  | Pending_init
  | During_init _
  | Failed_init _ ->
    Lwt.return_unit
  | Initialized { ifiles; _ } ->
    if Relative_path.Set.is_empty ifiles.changed_files_to_process then
      let%lwt () =
        write_message
          ~out_fd
          ~message:
            (ClientIdeMessage.Notification ClientIdeMessage.Done_processing)
      in
      Lwt.return_unit
    else
      let total = ifiles.changed_files_denominator in
      let processed =
        total - Relative_path.Set.cardinal ifiles.changed_files_to_process
      in
      let%lwt () =
        write_message
          ~out_fd
          ~message:
            (ClientIdeMessage.Notification
               (ClientIdeMessage.Processing_files
                  { ClientIdeMessage.Processing_files.processed; total }))
      in
      Lwt.return_unit

(** Allow to process the next file change only if we have no new events to
handle. To ensure correctness, we would have to actually process all file
change events *before* we processed any other IDE queries. However, we're
trying to maximize availability, even if occasionally we give stale
results. We can revisit this trade-off later if we decide that the stale
results are baffling users. *)
let should_process_file_change
    (in_fd : Lwt_unix.file_descr)
    (message_queue : message_queue)
    (istate : istate) : bool =
  Lwt_message_queue.is_empty message_queue
  && (not (Lwt_unix.readable in_fd))
  && not (Relative_path.Set.is_empty istate.ifiles.changed_files_to_process)

let process_one_file_change (out_fd : Lwt_unix.file_descr) (istate : istate) :
    istate Lwt.t =
  let next_file =
    Relative_path.Set.choose istate.ifiles.changed_files_to_process
  in
  let changed_files_to_process =
    Relative_path.Set.remove istate.ifiles.changed_files_to_process next_file
  in
  let { ClientIdeIncremental.naming_table; sienv; old_file_info; _ } =
    ClientIdeIncremental.update_naming_tables_for_changed_file
      ~backend:(Provider_backend.Local_memory istate.icommon.local_memory)
      ~popt:istate.icommon.popt
      ~naming_table:istate.naming_table
      ~sienv:istate.icommon.sienv
      ~path:next_file
  in
  Option.iter
    old_file_info
    ~f:
      (Provider_utils.invalidate_local_decl_caches_for_file
         istate.icommon.local_memory);
  Provider_utils.invalidate_tast_cache_of_entries istate.ifiles.open_files;
  let changed_files_denominator =
    if Relative_path.Set.is_empty changed_files_to_process then
      0
    else
      istate.ifiles.changed_files_denominator
  in
  let istate =
    {
      naming_table;
      icommon = { istate.icommon with sienv };
      ifiles =
        {
          istate.ifiles with
          changed_files_to_process;
          changed_files_denominator;
        };
    }
  in
  let%lwt () = write_status ~out_fd (Initialized istate) in
  Lwt.return istate

(** This function will either process one change that's pending,
or will await as necessary to handle one message. *)
let handle_one_message_exn
    ~(in_fd : Lwt_unix.file_descr)
    ~(out_fd : Lwt_unix.file_descr)
    ~(message_queue : message_queue)
    ~(state : state) : state option Lwt.t =
  (* The precise order of operations is to help us be responsive
  to requests, to never to await if there are pending changes to process,
  but also to await for the next thing to do:
  (1) If there's a message in [message_queue] then handle it;
  (2) Otherwise if there's a message in [in_fd] then await until it
  gets pumped into [message_queue] and then handle it;
  (3) Otherwise if there are pending file-changes then process them;
  (4) otherwise await until the next client request arrives in [in_fd]
  and gets pumped into [message_queue] and then handle it. *)
  match state with
  | Initialized istate
    when should_process_file_change in_fd message_queue istate ->
    let%lwt istate = process_one_file_change out_fd istate in
    Lwt.return_some (Initialized istate)
  | _ ->
    let%lwt message = Lwt_message_queue.pop message_queue in
    (match (state, message) with
    | (_, None) ->
      Lwt.return_none (* exit loop if message_queue has been closed *)
    | (During_init dstate, Some (LoadedState load_state_result)) ->
      let%lwt state = initialize2 out_fd dstate load_state_result in
      Lwt.return_some state
    | (_, Some (LoadedState _)) ->
      failwith ("Unexpected LoadedState in " ^ state_to_log_string state)
    | (_, Some (ClientRequest { ClientIdeMessage.tracking_id; message })) ->
      let unblocked_time = Unix.gettimeofday () in
      let%lwt (state, response) =
        try%lwt
          let%lwt (s, r) =
            handle_request message_queue state tracking_id message
          in
          Lwt.return (s, r)
        with exn ->
          (* Our caller has an exception handler which logs the exception.
          But we instead must fulfil our contract of responding to the client,
          even if we have an exception. Hence we need our own handler here. *)
          let exn = Exception.wrap exn in
          let e = ClientIdeUtils.make_bug_error "handle_request" ~exn in
          Lwt.return (state, Error e)
      in
      let%lwt () =
        write_message
          ~out_fd
          ~message:
            ClientIdeMessage.(
              Response { response; tracking_id; unblocked_time })
      in
      Lwt.return_some state)

let serve ~(in_fd : Lwt_unix.file_descr) ~(out_fd : Lwt_unix.file_descr) :
    unit Lwt.t =
  let rec flush_event_logger () : unit Lwt.t =
    let%lwt () = Lwt_unix.sleep 0.5 in
    HackEventLogger.Memory.profile_if_needed ();
    Lwt.async EventLoggerLwt.flush;
    EventLogger.recheck_disk_files ();
    flush_event_logger ()
  in
  let rec pump_stdin (message_queue : message_queue) : unit Lwt.t =
    try%lwt
      let%lwt { ClientIdeMessage.tracking_id; message } =
        Marshal_tools_lwt.from_fd_with_preamble in_fd
      in
      let is_queue_open =
        Lwt_message_queue.push
          message_queue
          (ClientRequest { ClientIdeMessage.tracking_id; message })
      in
      match message with
      | ClientIdeMessage.Shutdown () -> Lwt.return_unit
      | _ when not is_queue_open -> Lwt.return_unit
      | _ -> pump_stdin message_queue
    with e ->
      let e = Exception.wrap e in
      Lwt_message_queue.close message_queue;
      Exception.reraise e
  in
  let rec handle_messages ({ message_queue; state } : t) : unit Lwt.t =
    let%lwt next_state_opt =
      try%lwt
        let%lwt state =
          handle_one_message_exn ~in_fd ~out_fd ~message_queue ~state
        in
        Lwt.return state
      with exn ->
        let exn = Exception.wrap exn in
        ClientIdeUtils.log_bug "handle_one_message" ~exn ~telemetry:true;
        if is_outfd_write_error exn then exit 1;
        (* if out_fd is down then there's no use continuing. *)
        Lwt.return_some state
    in
    match next_state_opt with
    | None -> Lwt.return_unit (* exit loop *)
    | Some state -> handle_messages { message_queue; state }
  in
  try%lwt
    let message_queue = Lwt_message_queue.create () in
    let flusher_promise = flush_event_logger () in
    let%lwt () = handle_messages { message_queue; state = Pending_init }
    and () = pump_stdin message_queue in
    Lwt.cancel flusher_promise;
    Lwt.return_unit
  with exn ->
    let exn = Exception.wrap exn in
    ClientIdeUtils.log_bug "fatal clientIdeDaemon" ~exn ~telemetry:true;
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

  if args.ClientIdeMessage.verbose_to_stderr then
    Hh_logger.Level.set_min_level_stderr Hh_logger.Level.Debug
  else
    Hh_logger.Level.set_min_level_stderr Hh_logger.Level.Error;
  if args.ClientIdeMessage.verbose_to_file then
    Hh_logger.Level.set_min_level_file Hh_logger.Level.Debug
  else
    Hh_logger.Level.set_min_level_file Hh_logger.Level.Info;

  Lwt_main.run (serve ~in_fd ~out_fd)

let daemon_entry_point : (ClientIdeMessage.daemon_args, unit, unit) Daemon.entry
    =
  Daemon.register_entry_point "ClientIdeService" daemon_main
