(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Hh_prelude

type init_result = {
  naming_table: Naming_table.t;
  sienv: SearchUtils.si_env;
  changed_files: Saved_state_loader.changed_files;
}

type 'a outcome =
  | Success of 'a
  | Failure of ClientIdeMessage.rich_error
  | Skip of string

let error_from_load_error (load_error : Saved_state_loader.LoadError.t) :
    ClientIdeMessage.rich_error =
  let data =
    Some
      (Hh_json.JSON_Object
         [
           ( "debug_details",
             Hh_json.string_
               (Saved_state_loader.LoadError.debug_details_of_error load_error)
           );
         ])
  in
  let reason =
    ClientIdeUtils.make_rich_error
      (Saved_state_loader.LoadError.category_of_error load_error)
      ~data
  in
  (* [make_rich_error] returns a vague "IDE isn't working" message.
     If load_error was actionable, then we can get more specific information
     out of the load_error, although it's no longer IDE-specific. Instead
     it says things like "zstd fault stopped Hack from working". We'll
     get the best of both worlds with a vague IDE-specific message if
     it's not actionable, or a precise non-IDE-specific message if it is. *)
  if Saved_state_loader.LoadError.is_error_actionable load_error then
    {
      reason with
      ClientIdeMessage.is_actionable = true;
      short_user_message =
        Saved_state_loader.LoadError.short_user_message_of_error load_error;
      medium_user_message =
        Saved_state_loader.LoadError.medium_user_message_of_error load_error;
      long_user_message =
        Saved_state_loader.LoadError.long_user_message_of_error load_error;
    }
  else
    reason

(** LSP initialize method might directly contain the path of a saved-state to use.
Note: this path doesn't yet support changed_files. *)
let init_via_lsp
    ~(naming_table_load_info :
       ClientIdeMessage.Initialize_from_saved_state.naming_table_load_info
       option) : Path.t outcome Lwt.t =
  match naming_table_load_info with
  | None -> Lwt.return (Skip "no naming_table_load_info")
  | Some naming_table_load_info ->
    let open ClientIdeMessage.Initialize_from_saved_state in
    (* tests may wish to pretend there's a delay *)
    let%lwt () = Lwt_unix.sleep naming_table_load_info.test_delay in
    Lwt.return (Success naming_table_load_info.path)

(** We might find a saved-state available for download, and download it. *)
let init_via_fetch
    (ctx : Provider_context.t) ~(root : Path.t) ~(ignore_hh_version : bool) :
    (Path.t * Saved_state_loader.changed_files) outcome Lwt.t =
  let ssopt = TypecheckerOptions.saved_state (Provider_context.get_tcopt ctx) in
  let%lwt load_result =
    State_loader_lwt.load
      ~ssopt
      ~progress_callback:(fun _ -> ())
      ~watchman_opts:
        Saved_state_loader.Watchman_options.{ root; sockname = None }
      ~ignore_hh_version
  in
  match load_result with
  | Ok { Saved_state_loader.main_artifacts; changed_files; _ } ->
    let path =
      main_artifacts
        .Saved_state_loader.Naming_and_dep_table_info.naming_sqlite_table_path
    in
    Lwt.return (Success (path, changed_files))
  | Error load_error -> Lwt.return (Failure (error_from_load_error load_error))

(** Performs a full index of [root], building a naming table on disk,
and then loading it. Also returns [si_addenda] from what we gathered
during the full index. *)
let init_via_build
    ~(config : ServerConfig.t) ~(root : Path.t) ~(hhi_root : Path.t) :
    (Path.t * SymbolIndexCore.paths_with_addenda) outcome Lwt.t =
  let path = Path.make (ServerFiles.client_ide_naming_table root) in
  let rec poll_build_until_complete_exn progress =
    match Naming_table_builder_ffi_externs.poll_exn progress with
    | Some build_result -> Lwt.return build_result
    | None ->
      let%lwt () = Lwt_unix.sleep 0.1 in
      poll_build_until_complete_exn progress
  in

  if not (ServerConfig.ide_fall_back_to_full_index config) then begin
    Lwt.return (Skip "ide_fallback_to_full_index=false")
  end else begin
    let progress =
      Naming_table_builder_ffi_externs.build
        ~www:root
        ~custom_hhi_path:hhi_root
        ~output:path
    in
    let%lwt build_result = poll_build_until_complete_exn progress in
    match build_result with
    | {
     Naming_table_builder_ffi_externs.exit_status = 0;
     time_taken_secs = _;
     si_addenda;
    } ->
      let paths_with_addenda =
        List.map si_addenda ~f:(fun (path, addenda) ->
            (path, addenda, SearchUtils.TypeChecker))
      in
      Lwt.return (Success (path, paths_with_addenda))
    | { Naming_table_builder_ffi_externs.exit_status; time_taken_secs = _; _ }
      ->
      let data =
        Some
          (Hh_json.JSON_Object
             [("naming_table_builder_exit_status", Hh_json.int_ exit_status)])
      in
      Lwt.return
        (Failure (ClientIdeUtils.make_rich_error "full_index_error" ~data))
  end

let init_via_find
    (ctx : Provider_context.t)
    ~(local_config : ServerLocalConfig.t)
    ~(root : Path.t)
    ~(ignore_hh_version : bool) :
    (Path.t * Saved_state_loader.changed_files) outcome Lwt.t =
  if not local_config.ServerLocalConfig.ide_load_naming_table_on_disk then begin
    Lwt.return (Skip "ide_load_naming_table_on_disk=false")
  end else begin
    let%lwt get_metadata_result =
      State_loader_lwt.get_project_metadata
        ~opts:(Provider_context.get_tcopt ctx |> TypecheckerOptions.saved_state)
        ~progress_callback:(fun _ -> ())
        ~repo:root
        ~ignore_hh_version
    in
    match get_metadata_result with
    | Error (load_error, _telemetry) ->
      Lwt.return (Failure (error_from_load_error load_error))
    | Ok (project_metadata, _telemetry) -> begin
      let%lwt load_off_disk_result =
        State_loader_lwt.load_arbitrary_naming_table_from_disk
          ~project_metadata
          ~threshold:
            local_config.ServerLocalConfig.ide_naming_table_update_threshold
          ~root
      in
      match load_off_disk_result with
      | Ok (path, changed_files) -> Lwt.return (Success (path, changed_files))
      | Error err ->
        let data = Some (Hh_json.JSON_Object [("err", Hh_json.string_ err)]) in
        Lwt.return
          (Failure (ClientIdeUtils.make_rich_error "find_failed" ~data))
    end
  end

(** This is a helper function used to add logging+telemetry to each
attempt at initializing the symbol table. Used like this:
[let%lwt result = attempt ... |> map_attempt key ~telemetry ~prev ~f]
Here [attempt ...] returns an Lwt.t representing the attempt that has been
kicked off. The return of [map_attempt] is a wrapper promise around that
underlying promise, one which adds logging and telemetry, including
any unhandled exceptions. *)
let map_attempt
    (type a)
    (key : string)
    (ctx : Provider_context.t)
    ~(prev : Telemetry.t * ClientIdeMessage.rich_error)
    ~(f : a -> Path.t * Saved_state_loader.changed_files * SearchUtils.si_env)
    (promise : a outcome Lwt.t) :
    ( Telemetry.t
      * Naming_table.t
      * Saved_state_loader.changed_files
      * SearchUtils.si_env,
      Telemetry.t * ClientIdeMessage.rich_error )
    result
    Lwt.t =
  let start_time = Unix.gettimeofday () in
  let (telemetry, prev_error) = prev in
  Hh_logger.log "Init: %s?" key;
  try%lwt
    let%lwt result = promise in
    match result with
    | Success a ->
      let (path, changed_files, sienv) = f a in
      Hh_logger.log
        "Init: %s ok, at %s, %d changed_files"
        key
        (Path.to_string path)
        (List.length changed_files);
      let naming_table =
        Naming_table.load_from_sqlite ctx (Path.to_string path)
      in
      let telemetry =
        telemetry
        |> Telemetry.string_ ~key ~value:"winner"
        |> Telemetry.duration ~start_time
      in
      Lwt.return_ok (telemetry, naming_table, changed_files, sienv)
    | Skip reason ->
      Hh_logger.log "Init: %s skipped - %s" key reason;
      let telemetry = telemetry |> Telemetry.string_ ~key ~value:reason in
      Lwt.return_error (telemetry, prev_error)
    | Failure reason ->
      let { ClientIdeMessage.category; data; _ } = reason in
      let data = Option.value data ~default:Hh_json.JSON_Null in
      Hh_logger.log
        "Init: %s error - %s - %s"
        key
        category
        (Hh_json.json_to_string data);
      let telemetry =
        telemetry
        |> Telemetry.object_
             ~key
             ~value:
               (Telemetry.create ()
               |> Telemetry.duration ~start_time
               |> Telemetry.string_ ~key:"error" ~value:category
               |> Telemetry.json_ ~key:"data" ~value:data)
      in
      Lwt.return_error (telemetry, reason)
  with
  | exn ->
    let e = Exception.wrap exn in
    (* unhandled exception from the attempt itself, or from loading the found sqlite *)
    let reason = ClientIdeUtils.make_rich_error ~e (key ^ " exn") in
    Hh_logger.log "Init: %s exn - %s" key (Exception.to_string e);
    let telemetry =
      telemetry
      |> Telemetry.object_
           ~key
           ~value:
             (Telemetry.create ()
             |> Telemetry.duration ~start_time
             |> Telemetry.exception_ ~e)
    in
    Lwt.return_error (telemetry, reason)

let init
    ~(config : ServerConfig.t)
    ~(local_config : ServerLocalConfig.t)
    ~(param : ClientIdeMessage.Initialize_from_saved_state.t)
    ~(hhi_root : Path.t)
    ~(local_memory : Provider_backend.local_memory) :
    (init_result, ClientIdeMessage.rich_error) result Lwt.t =
  let start_time = Unix.gettimeofday () in
  let open ClientIdeMessage.Initialize_from_saved_state in
  let root = param.root in
  let ignore_hh_version = param.ignore_hh_version in
  let naming_table_load_info = param.naming_table_load_info in
  let popt = ServerConfig.parser_options config in
  let tcopt = ServerConfig.typechecker_options config in

  let ctx =
    Provider_context.empty_for_tool
      ~popt
      ~tcopt
      ~backend:(Provider_backend.Local_memory local_memory)
      ~deps_mode:(Typing_deps_mode.InMemoryMode None)
  in
  let sienv =
    SymbolIndex.initialize
      ~gleanopt:(ServerConfig.glean_options config)
      ~namespace_map:(ParserOptions.auto_namespace_map tcopt)
      ~provider_name:
        local_config.ServerLocalConfig.ide_symbolindex_search_provider
      ~quiet:local_config.ServerLocalConfig.symbolindex_quiet
  in

  (* We will make several attempts using different techniques, in order, to try
     to find a saved-state, and use the first attempt to succeed.
     The goal of this code is (1) rich telemetry about why each attempt failed,
     (2) in case none of them succeed, then return a user-facing
     [ClientIdeMessage.rich_error] that has the most pertinent information for the user.

     An attempt might either skip (e.g. if config flags disable it), or it might fail.
     The way I've solved goal (2) is that we'll display a user-facing message
     about the final failed attempt.
     There are some subtle interactions here worth drawing attention to, based on how
     various libraries were written that we call into:
     1. [init_via_find] returns Skip if ide_load_naming_table_on_disk=false, but it
        returns Failure if there were no saved-states on disk, or none were recent enough,
        or if it found a candidate but experienced a runtime failure trying to use it.
     2. [init_via_fetch] doesn't ever return Skip; it only uses Failure. This makes
        sense in cases like manifold crash or zstd crash, but it's a bit odd in cases
        like the user modified .hhconfig so no saved-state is possible, or the user
        is in a repository that doesn't even have saved-states.

     How does all this fit together to satisfy goal (2)? If we're in a repository that
     allows building (i.e. its .hhconfig lacks "ide_fallback_to_full_index=false") then
     the user-facing message upon overall failure will only ever be about the failure to build.
     If we're in a repository that doesn't allow building, then the user-facing message
     message upon overall failure will only ever be about the failure to load saved-state.

     How does it satisfy goal (1)? Inside [map_attempt], if there was a "skip" then we
     only store brief telemetry about it. But if there was a "failure" then we store
     very detailed telemetry. Once ide_load_naming_table_on_disk is fully rolled out,
     we'll end up storing detailed telemetry about every single "failed to find on disk".
     That's good! I fully hope and expect that 99% of users will find something on disk
     and be good. But if that fails, we'll always store rich information about every
     "failed to fetch". That's also what we want. *)

  (* As mentioned, Skip will result in the previous error message falling through.
     So we have to seed with an initial [prev] -- even though some of the attempts never
     return Skip and so this [prev] never makes its way through to the end! *)
  let prev =
    (Telemetry.create (), ClientIdeUtils.make_rich_error "trying...")
  in

  (* Specified in the LSP initialize message? *)
  let%lwt result =
    init_via_lsp ~naming_table_load_info
    |> map_attempt "lsp" ctx ~prev ~f:(fun path -> (path, [], sienv))
  in
  (* Find on disk? *)
  let%lwt result =
    match result with
    | Ok _ -> Lwt.return result
    | Error prev ->
      init_via_find ctx ~local_config ~root ~ignore_hh_version
      |> map_attempt "find" ctx ~prev ~f:(fun (path, changed_files) ->
             (path, changed_files, sienv))
  in
  (* Fetch? *)
  let%lwt result =
    match result with
    | Ok _ -> Lwt.return result
    | Error prev ->
      init_via_fetch ctx ~root ~ignore_hh_version
      |> map_attempt "fetch" ctx ~prev ~f:(fun (path, changed_files) ->
             (path, changed_files, sienv))
  in
  (* Build? *)
  let%lwt result =
    match result with
    | Ok _ -> Lwt.return result
    | Error prev ->
      init_via_build ~config ~root ~hhi_root
      |> map_attempt "build" ctx ~prev ~f:(fun (path, paths_with_addenda) ->
             let sienv =
               SymbolIndexCore.update_from_addenda ~sienv ~paths_with_addenda
             in
             (path, [], sienv))
  in

  match result with
  | Ok (telemetry, naming_table, changed_files, sienv) ->
    HackEventLogger.serverless_ide_load_naming_table
      ~start_time
      ~local_file_count:(Some (List.length changed_files))
      telemetry;
    Lwt.return_ok { naming_table; sienv; changed_files }
  | Error (telemetry, e) ->
    HackEventLogger.serverless_ide_load_naming_table
      ~start_time
      ~local_file_count:None
      telemetry;
    Lwt.return_error e
