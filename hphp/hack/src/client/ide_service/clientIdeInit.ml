(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Hh_prelude

type init_error = ClientIdeMessage.stopped_reason * Lsp.Error.t

type init_ok = {
  naming_table: Naming_table.t;
  sienv: SearchUtils.si_env;
  changed_files: Saved_state_loader.changed_files;
}

type init_result = (init_ok, init_error) result

(** If [naming_table_load_info] is Some path, load from that path (and deem that there are no changed files since).
Otherwise, attempt to download via [State_loader_lwt.load]. *)
let init_via_initialize_or_download
    (ctx : Provider_context.t)
    ~(local_config : ServerLocalConfig.t)
    ~(root : Path.t)
    ~(naming_table_load_info :
       ClientIdeMessage.Initialize_from_saved_state.naming_table_load_info
       option)
    ~(ignore_hh_version : bool) :
    (Naming_table.t * Saved_state_loader.changed_files, init_error) result Lwt.t
    =
  try%lwt
    let start_time = Unix.gettimeofday () in
    let%lwt artifacts_and_changed_files_result =
      match naming_table_load_info with
      | Some naming_table_load_info ->
        let open ClientIdeMessage.Initialize_from_saved_state in
        (* tests may wish to pretend there's a delay *)
        let%lwt () = Lwt_unix.sleep naming_table_load_info.test_delay in
        Lwt.return_ok
          {
            Saved_state_loader.main_artifacts =
              {
                Saved_state_loader.Naming_table_info.naming_table_path =
                  naming_table_load_info.path;
              };
            additional_info = ();
            changed_files = [];
            (* the "passed-save-state-directly" testing path doesn't also support changed-files *)
            manifold_path = "<not provided>";
            corresponding_rev = "<not provided>";
            mergebase_rev = "<not provided>";
            is_cached = true;
          }
      | None ->
        let ssopt =
          TypecheckerOptions.saved_state (Provider_context.get_tcopt ctx)
        in
        let%lwt load_result =
          if local_config.ServerLocalConfig.ide_should_use_hack_64_distc then
            let%lwt load_result =
              State_loader_lwt.load
                ~ssopt
                ~progress_callback:(fun _ -> ())
                ~watchman_opts:
                  Saved_state_loader.Watchman_options.{ root; sockname = None }
                ~ignore_hh_version
                ~saved_state_type:Saved_state_loader.Naming_and_dep_table_distc
            in
            match load_result with
            | Ok
                {
                  Saved_state_loader.main_artifacts;
                  changed_files;
                  manifold_path;
                  corresponding_rev;
                  mergebase_rev;
                  is_cached;
                  additional_info = _;
                } ->
              let main_artifacts =
                {
                  Saved_state_loader.Naming_table_info.naming_table_path =
                    main_artifacts
                      .Saved_state_loader.Naming_and_dep_table_info
                       .naming_sqlite_table_path;
                }
              in
              Lwt.return
                (Ok
                   {
                     Saved_state_loader.main_artifacts;
                     additional_info = ();
                     manifold_path;
                     changed_files;
                     corresponding_rev;
                     mergebase_rev;
                     is_cached;
                   })
            | Error e -> Lwt.return (Error e)
          else
            State_loader_lwt.load
              ~ssopt
              ~progress_callback:(fun _ -> ())
              ~watchman_opts:
                Saved_state_loader.Watchman_options.{ root; sockname = None }
              ~ignore_hh_version
              ~saved_state_type:Saved_state_loader.Naming_table
        in
        Lwt.return load_result
    in
    match artifacts_and_changed_files_result with
    | Ok { Saved_state_loader.main_artifacts; changed_files; _ } ->
      let path =
        Path.to_string
          main_artifacts.Saved_state_loader.Naming_table_info.naming_table_path
      in
      let naming_table = Naming_table.load_from_sqlite ctx path in
      (* Track how many files we have to change locally *)
      HackEventLogger.serverless_ide_local_files
        ~local_file_count:(List.length changed_files);
      HackEventLogger.serverless_ide_load_naming_table ~start_time;
      Lwt.return_ok (naming_table, changed_files)
    | Error load_error ->
      (* We'll turn that load_error into a user-facing [reason], and a
         programmatic error [e] for future telemetry *)
      let reason =
        ClientIdeMessage.
          {
            short_user_message =
              Saved_state_loader.LoadError.short_user_message_of_error
                load_error;
            medium_user_message =
              Saved_state_loader.LoadError.medium_user_message_of_error
                load_error;
            long_user_message =
              Saved_state_loader.LoadError.long_user_message_of_error load_error;
            debug_details =
              Saved_state_loader.LoadError.debug_details_of_error load_error;
            is_actionable =
              Saved_state_loader.LoadError.is_error_actionable load_error;
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
  with
  | exn ->
    let exn = Exception.wrap exn in
    ClientIdeUtils.log_bug "load_exn" ~exn ~telemetry:false;
    (* We need both a user-facing "reason" and an internal error "e" *)
    let reason = ClientIdeUtils.make_bug_reason "load_exn" ~exn in
    let e = ClientIdeUtils.make_bug_error "load_exn" ~exn in
    Lwt.return_error (reason, e)

(** Performs a full index of [root], building a naming table in [output]
and then loading it. Also returns [si_addenda] from what we gathered
during the full index. *)
let init_via_build
    (ctx : Provider_context.t)
    ~(root : Path.t)
    ~(hhi_root : Path.t)
    ~(output : Path.t) :
    ( Naming_table.t * Naming_table_builder_ffi_externs.si_addenda,
      init_error )
    result
    Lwt.t =
  let progress =
    Naming_table_builder_ffi_externs.build
      ~www:root
      ~custom_hhi_path:hhi_root
      ~output
  in
  let rec poll_build_until_complete_exn () :
      Naming_table_builder_ffi_externs.build_result Lwt.t =
    match Naming_table_builder_ffi_externs.poll_exn progress with
    | Some build_result -> Lwt.return build_result
    | None ->
      let%lwt () = Lwt_unix.sleep 0.1 in
      poll_build_until_complete_exn ()
  in
  let%lwt build_result =
    try%lwt
      let%lwt r = poll_build_until_complete_exn () in
      Lwt.return_ok r
    with
    | exn ->
      let e = Exception.wrap exn in
      Lwt.return_error e
  in
  match build_result with
  | Ok
      {
        Naming_table_builder_ffi_externs.exit_status = 0;
        time_taken_secs = _;
        si_addenda;
      } ->
    let naming_table =
      Naming_table.load_from_sqlite ctx (Path.to_string output)
    in
    Lwt.return_ok (naming_table, si_addenda)
  | Ok { Naming_table_builder_ffi_externs.exit_status; time_taken_secs = _; _ }
    ->
    let data =
      Some
        (Hh_json.JSON_Object
           [("naming_table_builder_exit_status", Hh_json.int_ exit_status)])
    in
    let reason =
      ClientIdeUtils.make_bug_reason "full_index_non_zero_exit" ~data
    in
    let error =
      ClientIdeUtils.make_bug_error "full_index_non_zero_exit" ~data
    in
    Lwt.return_error (reason, error)
  | Error e ->
    let reason = ClientIdeUtils.make_bug_reason "full_index_exn" ~exn:e in
    let e = ClientIdeUtils.make_bug_error "full_index_exn" ~exn:e in
    Lwt.return_error (reason, e)

let init
    ~(config : ServerConfig.t)
    ~(local_config : ServerLocalConfig.t)
    ~(param : ClientIdeMessage.Initialize_from_saved_state.t)
    ~(hhi_root : Path.t)
    ~(local_memory : Provider_backend.local_memory)
    ~(notify_callback_in_case_of_fallback_to_full_init : unit -> unit Lwt.t) :
    init_result Lwt.t =
  let popt = ServerConfig.parser_options config in
  let tcopt = ServerConfig.typechecker_options config in
  let ctx =
    Provider_context.empty_for_tool
      ~popt
      ~tcopt
      ~backend:(Provider_backend.Local_memory local_memory)
      ~deps_mode:(Typing_deps_mode.InMemoryMode None)
      ~package_info:Package.Info.empty
  in
  let open ClientIdeMessage.Initialize_from_saved_state in
  let sienv =
    SymbolIndex.initialize
      ~globalrev:None
      ~gleanopt:(ServerConfig.glean_options config)
      ~namespace_map:(ParserOptions.auto_namespace_map tcopt)
      ~provider_name:
        local_config.ServerLocalConfig.ide_symbolindex_search_provider
      ~quiet:local_config.ServerLocalConfig.symbolindex_quiet
      ~savedstate_file_opt:local_config.ServerLocalConfig.symbolindex_file
      ~workers:None
  in

  (*
   * See if we can use an existing file off disk
   *)
  let%lwt load_existing_result =
    if local_config.ServerLocalConfig.ide_load_naming_table_on_disk then begin
      let saved_state_type =
        if local_config.ServerLocalConfig.ide_should_use_hack_64_distc then
          Saved_state_loader.Naming_and_dep_table_distc
        else
          Saved_state_loader.Naming_and_dep_table
      in
      let%lwt get_metadata_result =
        State_loader_lwt.get_project_metadata
          ~opts:(TypecheckerOptions.saved_state tcopt)
          ~progress_callback:(fun _ -> ())
          ~repo:param.root
          ~ignore_hh_version:param.ignore_hh_version
          ~saved_state_type
      in
      match get_metadata_result with
      | Ok (project_metadata, _telemetry) -> begin
        let%lwt load_off_disk_result =
          State_loader_lwt.load_arbitrary_naming_table_from_disk
            ~saved_state_type
            ~project_metadata
            ~threshold:
              local_config.ServerLocalConfig.ide_naming_table_update_threshold
            ~root:param.root
        in
        match load_off_disk_result with
        | Ok (path, changed_files) -> Lwt.return_ok (path, changed_files)
        | Error err ->
          Hh_logger.log "existing disk ss? - load error %s" err;
          Lwt.return_error ()
      end
      | Error (load_error, _telemetry) ->
        Hh_logger.log
          "existing disk ss? - metadata error %s"
          (Saved_state_loader.LoadError.medium_user_message_of_error load_error);
        Lwt.return_error ()
    end else begin
      Hh_logger.log "existing disk ss? - ide_load_naming_table_on_disk=false";
      Lwt.return_error ()
    end
  in

  match load_existing_result with
  | Ok (naming_table_path, changed_files) ->
    let naming_table =
      Naming_table.load_from_sqlite ctx (Path.to_string naming_table_path)
    in
    Lwt.return_ok { naming_table; sienv; changed_files }
  | Error () -> begin
    (*
     * Failing that, see if we can use a path in the LSP initialize request, or download one
     *)
    let%lwt download_result =
      init_via_initialize_or_download
        ctx
        ~local_config
        ~root:param.root
        ~naming_table_load_info:param.naming_table_load_info
        ~ignore_hh_version:param.ignore_hh_version
    in
    match download_result with
    | Ok (naming_table, changed_files) ->
      Lwt.return_ok { naming_table; sienv; changed_files }
    | Error e when not (ServerConfig.ide_fall_back_to_full_index config) ->
      Lwt.return_error e
    | Error (_reason, e) -> begin
      (*
       * Failing that, can we build a full index ourselves?
       *)
      let message =
        Printf.sprintf
          "Falling back to full index naming table build because naming table load failed: %s"
          e.Lsp.Error.message
      in
      ClientIdeUtils.log_bug message ~data:e.Lsp.Error.data ~telemetry:true;
      let%lwt () = notify_callback_in_case_of_fallback_to_full_init () in
      let%lwt full_index_result =
        init_via_build
          ctx
          ~root:param.root
          ~hhi_root
          ~output:(Path.make @@ ServerFiles.client_ide_naming_table param.root)
      in
      match full_index_result with
      | Ok (naming_table, si_addenda) ->
        let paths =
          List.map si_addenda ~f:(fun (path, addenda) ->
              (path, addenda, SearchUtils.TypeChecker))
        in
        let sienv = SymbolIndexCore.update_from_addenda ~sienv ~paths in
        Lwt.return_ok { naming_table; sienv; changed_files = [] }
      | Error e -> Lwt.return_error e
    end
  end
