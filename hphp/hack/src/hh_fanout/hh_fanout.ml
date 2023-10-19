(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Hh_prelude

type env = {
  client_id: string;
  root: Path.t;
  ignore_hh_version: bool;
  detail_level: Calculate_fanout.Detail_level.t;
  naming_table_path: Path.t option;
  dep_table_path: Path.t option;
  watchman_sockname: Path.t option;
  changed_files: Relative_path.Set.t;
  state_path: Path.t option;
}

type setup_result = {
  workers: MultiWorker.worker list;
  ctx: Provider_context.t;
}

type saved_state_result = {
  naming_table: Naming_table.t;
  naming_table_path: Path.t;
  dep_table_path: Path.t;
  errors_path: Path.t;
  saved_state_changed_files: Relative_path.Set.t;
  setup_result: setup_result;
}

type cursor_reference =
  | Cursor_reference_from_saved_state of saved_state_result
  | Cursor_reference_id of string

let create_global_env (env : env) : ServerEnv.genv =
  let server_args =
    ServerArgs.default_options_with_check_mode ~root:(Path.to_string env.root)
  in
  let (server_config, server_local_config) =
    ServerConfig.load ~silent:false server_args
  in
  ServerEnvBuild.make_genv server_args server_config server_local_config []

let set_up_global_environment (env : env) ~(deps_mode : Typing_deps_mode.t) :
    setup_result =
  let genv = create_global_env env (* no workers *) in
  let server_config = genv.ServerEnv.config in

  let popt = ServerConfig.parser_options genv.ServerEnv.config in
  let tcopt = ServerConfig.typechecker_options genv.ServerEnv.config in

  let (ctx, workers, _time_taken) =
    Batch_init.init
      ~root:env.root
      ~shmem_config:(ServerConfig.sharedmem_config server_config)
      ~popt
      ~tcopt
      ~deps_mode
      (Unix.gettimeofday ())
  in
  { workers; ctx }

let load_saved_state ~(env : env) : saved_state_result Lwt.t =
  let genv = create_global_env env in
  let ssopt =
    genv.ServerEnv.local_config.ServerLocalConfig.saved_state
    |> GlobalOptions.with_log_saved_state_age_and_distance false
  in
  let%lwt ( naming_table_path,
            naming_table_changed_files,
            dep_table_path,
            errors_path,
            dep_table_changed_files ) =
    match (env.naming_table_path, env.dep_table_path) with
    | (Some naming_table_path, Some dep_table_path) ->
      let errors_path =
        dep_table_path
        |> Path.to_string
        |> Filename.split_extension
        |> fst
        |> SaveStateService.get_errors_filename
        |> Path.make
      in
      Lwt.return (naming_table_path, [], dep_table_path, errors_path, [])
    | (Some naming_table_path, None) ->
      let%lwt dep_table_saved_state =
        State_loader_lwt.load
          ~ssopt
          ~progress_callback:(fun _ -> ())
          ~watchman_opts:
            Saved_state_loader.Watchman_options.
              { root = env.root; sockname = env.watchman_sockname }
          ~ignore_hh_version:env.ignore_hh_version
      in
      (match dep_table_saved_state with
      | Error load_error ->
        failwith
          (Printf.sprintf
             "Failed to load dep-table saved-state, and saved-state files were not manually provided on command-line: %s"
             (Saved_state_loader.LoadError.debug_details_of_error load_error))
      | Ok { Saved_state_loader.main_artifacts; changed_files; _ } ->
        let open Saved_state_loader.Naming_and_dep_table_info in
        Lwt.return
          ( naming_table_path,
            [],
            main_artifacts.dep_table_path,
            main_artifacts.errors_path,
            changed_files ))
    | (None, Some dep_table_path) ->
      let%lwt naming_table_saved_state =
        State_loader_lwt.load
          ~ssopt
          ~progress_callback:(fun _ -> ())
          ~watchman_opts:
            Saved_state_loader.Watchman_options.
              { root = env.root; sockname = env.watchman_sockname }
          ~ignore_hh_version:env.ignore_hh_version
      in
      (match naming_table_saved_state with
      | Error load_error ->
        failwith
          (Printf.sprintf
             "Failed to load naming-table saved-state, and saved-state files were not manually provided on command-line: %s"
             (Saved_state_loader.LoadError.debug_details_of_error load_error))
      | Ok { Saved_state_loader.main_artifacts; changed_files; _ } ->
        let errors_path =
          dep_table_path
          |> Path.to_string
          |> Filename.split_extension
          |> fst
          |> SaveStateService.get_errors_filename
          |> Path.make
        in
        Lwt.return
          ( main_artifacts
              .Saved_state_loader.Naming_and_dep_table_info
               .naming_sqlite_table_path,
            changed_files,
            dep_table_path,
            errors_path,
            [] ))
    | (None, None) ->
      let%lwt saved_state =
        State_loader_lwt.load
          ~ssopt
          ~progress_callback:(fun _ -> ())
          ~watchman_opts:
            Saved_state_loader.Watchman_options.
              { root = env.root; sockname = env.watchman_sockname }
          ~ignore_hh_version:env.ignore_hh_version
      in
      (match saved_state with
      | Error load_error ->
        failwith
          (Printf.sprintf
             "Failed to load naming-table saved-state, and saved-state files were not manually provided on command-line: %s"
             (Saved_state_loader.LoadError.debug_details_of_error load_error))
      | Ok { Saved_state_loader.main_artifacts; changed_files; _ } ->
        let open Saved_state_loader.Naming_and_dep_table_info in
        Lwt.return
          ( main_artifacts.naming_sqlite_table_path,
            changed_files,
            main_artifacts.dep_table_path,
            main_artifacts.errors_path,
            changed_files ))
  in
  let changed_files =
    Relative_path.Set.union
      (Relative_path.Set.of_list naming_table_changed_files)
      (Relative_path.Set.of_list dep_table_changed_files)
  in
  let changed_files =
    Relative_path.Set.filter changed_files ~f:(fun path ->
        FindUtils.file_filter (Relative_path.to_absolute path))
  in

  let deps_mode =
    Typing_deps_mode.InMemoryMode (Some (Path.to_string dep_table_path))
  in
  let setup_result = set_up_global_environment env ~deps_mode in

  let naming_table =
    Naming_table.load_from_sqlite
      setup_result.ctx
      (Path.to_string naming_table_path)
  in
  let ClientIdeIncremental.Batch.{ naming_table; sienv = _; changes = _ } =
    ClientIdeIncremental.Batch.update_naming_tables_and_si
      ~ctx:setup_result.ctx
      ~naming_table
      ~sienv:SearchUtils.quiet_si_env
      ~changes:changed_files
  in
  Lwt.return
    {
      naming_table;
      naming_table_path;
      dep_table_path;
      errors_path;
      saved_state_changed_files = changed_files;
      setup_result;
    }

let get_state_path ~(env : env) : Path.t =
  match env.state_path with
  | Some state_path -> state_path
  | None ->
    let state_path = Path.make "/tmp/hh_fanout" in
    let state_path =
      Path.concat state_path (Path.slash_escaped_string_of_path env.root)
    in
    let state_path =
      Path.concat
        state_path
        (match Build_banner.banner with
        | Some banner -> banner
        | None -> "development")
    in
    let state_path = Path.concat state_path "hh_fanout_state" in
    state_path

let make_incremental_state ~(env : env) : Incremental.state =
  let state_path = get_state_path ~env in
  Hh_logger.log "State path: %s" (Path.to_string state_path);
  Incremental.make_reference_implementation state_path

let resolve_cursor_reference
    ~(env : env)
    ~(incremental_state : Incremental.state)
    ~(previous_cursor_reference : cursor_reference) :
    Incremental.cursor * Relative_path.Set.t =
  match previous_cursor_reference with
  | Cursor_reference_from_saved_state saved_state_result ->
    let client_id =
      incremental_state#make_client_id
        {
          Incremental.client_id = env.client_id;
          ignore_hh_version = env.ignore_hh_version;
          dep_table_saved_state_path = saved_state_result.dep_table_path;
          dep_table_errors_saved_state_path = saved_state_result.errors_path;
          naming_table_saved_state_path =
            Naming_sqlite.Db_path
              (Path.to_string saved_state_result.naming_table_path);
          deps_mode =
            Provider_context.get_deps_mode saved_state_result.setup_result.ctx;
        }
    in
    let cursor =
      incremental_state#make_default_cursor client_id |> Result.ok_or_failwith
    in
    (cursor, saved_state_result.saved_state_changed_files)
  | Cursor_reference_id cursor_id ->
    let cursor =
      incremental_state#look_up_cursor
        ~client_id:(Some (Incremental.Client_id env.client_id))
        ~cursor_id
      |> Result.ok_or_failwith
    in
    (cursor, Relative_path.Set.empty)

let advance_cursor
    ~(env : env)
    ~(setup_result : setup_result)
    ~(previous_cursor : Incremental.cursor)
    ~(previous_changed_files : Relative_path.Set.t)
    ~(input_files : Relative_path.Set.t) : Incremental.cursor =
  let cursor_changed_files =
    previous_changed_files
    |> Relative_path.Set.union env.changed_files
    |> Relative_path.Set.union input_files
  in
  previous_cursor#advance
    ~detail_level:env.detail_level
    setup_result.ctx
    setup_result.workers
    cursor_changed_files

let mode_calculate
    ~(env : env) ~(input_files : Path.Set.t) ~(cursor_id : string option) :
    unit Lwt.t =
  let telemetry = Telemetry.create () in
  let incremental_state = make_incremental_state ~env in
  let%lwt (previous_cursor, previous_changed_files, setup_result) =
    match cursor_id with
    | None ->
      let%lwt saved_state_result = load_saved_state ~env in
      let previous_cursor_reference =
        Cursor_reference_from_saved_state saved_state_result
      in
      let (previous_cursor, previous_changed_files) =
        resolve_cursor_reference
          ~env
          ~incremental_state
          ~previous_cursor_reference
      in
      Lwt.return
        ( previous_cursor,
          previous_changed_files,
          saved_state_result.setup_result )
    | Some cursor_id ->
      let previous_cursor_reference = Cursor_reference_id cursor_id in
      let (previous_cursor, previous_changed_files) =
        resolve_cursor_reference
          ~env
          ~incremental_state
          ~previous_cursor_reference
      in
      let deps_mode = previous_cursor#get_deps_mode in
      let setup_result = set_up_global_environment env ~deps_mode in
      Lwt.return (previous_cursor, previous_changed_files, setup_result)
  in

  let input_files =
    Path.Set.fold input_files ~init:Relative_path.Set.empty ~f:(fun path acc ->
        let path = Relative_path.create_detect_prefix (Path.to_string path) in
        Relative_path.Set.add acc path)
  in
  let cursor =
    advance_cursor
      ~env
      ~setup_result
      ~previous_cursor
      ~previous_changed_files
      ~input_files
  in

  let calculate_fanout_result = cursor#get_calculate_fanout_result in
  let {
    Calculate_fanout.fanout_dependents = _;
    fanout_files;
    explanations;
    telemetry = calculate_fanout_telemetry;
  } =
    Option.value_exn
      calculate_fanout_result
      ~message:
        ("Internal invariant failure -- "
        ^ "produced cursor did not have an associated `Calculate_fanout.result`"
        )
  in
  let telemetry =
    Telemetry.object_
      telemetry
      ~key:"calculate_fanout"
      ~value:calculate_fanout_telemetry
  in

  let telemetry =
    Telemetry.int_
      telemetry
      ~key:"num_input_files"
      ~value:(Relative_path.Set.cardinal input_files)
  in
  let telemetry =
    Telemetry.int_
      telemetry
      ~key:"num_fanout_files"
      ~value:(Relative_path.Set.cardinal fanout_files)
  in

  let cursor_id = incremental_state#add_cursor cursor in

  let json =
    Hh_json.JSON_Object
      [
        ( "files",
          Hh_json.JSON_Array
            (fanout_files
            |> Relative_path.Set.elements
            |> List.map ~f:Relative_path.to_absolute
            |> List.map ~f:Hh_json.string_) );
        ( "explanations",
          Hh_json.JSON_Object
            (Relative_path.Map.fold explanations ~init:[] ~f:(fun k v acc ->
                 let path = Relative_path.suffix k in
                 let explanation = Calculate_fanout.explanation_to_json v in
                 (path, explanation) :: acc)) );
        ( "cursor",
          let (Incremental.Cursor_id cursor_id) = cursor_id in
          Hh_json.JSON_String cursor_id );
        ("telemetry", Telemetry.to_json telemetry);
      ]
  in
  Hh_json.json_to_multiline_output Out_channel.stdout json;
  Lwt.return_unit

let mode_calculate_errors
    ~(env : env) ~(cursor_id : string option) ~(pretty_print : bool) :
    unit Lwt.t =
  let incremental_state = make_incremental_state ~env in
  let cursor =
    match cursor_id with
    | Some cursor_id ->
      incremental_state#look_up_cursor ~client_id:None ~cursor_id
    | None ->
      incremental_state#make_default_cursor
        (Incremental.Client_id env.client_id)
  in
  let cursor =
    match cursor with
    | Error message -> failwith ("Cursor not found: " ^ message)
    | Ok cursor -> cursor
  in
  let { ctx; workers } =
    set_up_global_environment env ~deps_mode:cursor#get_deps_mode
  in

  let (errors, cursor) = cursor#calculate_errors ctx workers in
  let cursor_id =
    match cursor with
    | Some cursor -> incremental_state#add_cursor cursor
    | None ->
      let cursor_id =
        Option.value_exn
          cursor_id
          ~message:
            ("Internal invariant failure -- "
            ^ "expected a new cursor to be generated, "
            ^ "given that no cursor was passed in.")
      in
      Incremental.Cursor_id cursor_id
  in

  let error_list =
    errors |> Errors.get_sorted_error_list |> List.map ~f:User_error.to_absolute
  in
  (if pretty_print then
    ServerError.print_error_list
      stdout
      ~error_list
      ~stale_msg:None
      ~output_json:false
      ~save_state_result:None
      ~recheck_stats:None
  else
    let json =
      ServerError.get_error_list_json
        error_list
        ~save_state_result:None
        ~recheck_stats:None
    in
    let json =
      match json with
      | Hh_json.JSON_Object props ->
        let props =
          [
            ( "cursor",
              let (Incremental.Cursor_id cursor_id) = cursor_id in
              Hh_json.JSON_String cursor_id );
          ]
          @ props
        in
        Hh_json.JSON_Object props
      | _ -> failwith "Expected error JSON to be an object"
    in
    Hh_json.json_to_multiline_output Out_channel.stdout json);
  Lwt.return_unit

let detail_level_arg =
  Cmdliner.Arg.enum
    [
      ("low", Calculate_fanout.Detail_level.Low);
      ("high", Calculate_fanout.Detail_level.High);
    ]

let env
    from
    client_id
    root
    detail_level
    ignore_hh_version
    naming_table_path
    dep_table_path
    watchman_sockname
    changed_files
    state_path =
  let root =
    Wwwroot.interpret_command_line_root_parameter (Option.to_list root)
  in

  (* Interpret relative paths with respect to the root from here on. That way,
      we can write `hh_fanout --root ~/www foo/bar.php` and it will work regardless
      of the directory that we invoked this executable from. *)
  Sys.chdir (Path.to_string root);
  Relative_path.set_path_prefix Relative_path.Root root;
  Relative_path.set_path_prefix Relative_path.Hhi (Hhi.get_hhi_root ());
  Relative_path.set_path_prefix Relative_path.Tmp (Path.make "/tmp");
  let changed_files =
    changed_files
    |> Sys_utils.parse_path_list
    |> List.filter ~f:FindUtils.file_filter
    |> List.map ~f:(fun path -> Relative_path.create_detect_prefix path)
    |> Relative_path.Set.of_list
  in

  let client_id =
    (* We always require 'from'. We don't want to make the user write out a
       client ID multiple times when they're using/debugging `hh_fanout`
       interactively, so provide a default value in that case.

       Most of the time, `from` and `client_id` will be the same anyways. An
       example of reuse might occur when the IDE service wants to take advantage
       of any work that the bulk typechecker has already done with regards to
       updating the dependency graph. *)
    Option.value client_id ~default:from
  in

  let naming_table_path = Option.map ~f:Path.make naming_table_path in
  let dep_table_path = Option.map ~f:Path.make dep_table_path in
  let watchman_sockname = Option.map ~f:Path.make watchman_sockname in
  let state_path = Option.map ~f:Path.make state_path in

  {
    client_id;
    root;
    ignore_hh_version;
    detail_level;
    naming_table_path;
    dep_table_path;
    watchman_sockname;
    changed_files;
    state_path;
  }

let env_t =
  let open Cmdliner in
  let open Cmdliner.Arg in
  let from =
    let doc = "A descriptive string indicating the caller of this program." in
    required & opt (some string) None & info ["from"] ~doc ~docv:"FROM"
  in
  let client_id =
    let doc =
      String.strip
        {|
A string identifying the caller of this program.
Use the same string across multiple callers to reuse hh_fanout cursors and intermediate results.
If not provided, defaults to the value for 'from'.
|}
    in
    let docv = "CLIENT-ID" in
    value & opt (some string) None & info ["client-id"] ~doc ~docv
  in
  let root =
    let doc =
      "The root directory to run in. If not set, will attempt to locate one by searching upwards for an `.hhconfig` file."
    in
    let docv = "DIR" in
    value & opt (some string) None & info ["root"] ~doc ~docv
  in
  let detail_level =
    let doc =
      "How much debugging output to include in the result. May slow down the query. The values are `low` or `high`."
    in
    let docv = "VERBOSITY" in
    value
    & opt detail_level_arg Calculate_fanout.Detail_level.Low
    & info ["detail-level"] ~doc ~docv
  in
  let ignore_hh_version =
    let doc =
      "Skip the consistency check for the version that this program was built with versus the version of the server that built the saved-state."
    in
    value & flag & info ["ignore-hh-version"] ~doc
  in
  let naming_table_path =
    let doc = "The path to the naming table SQLite saved-state." in
    let docv = "PATH" in
    value & opt (some string) None & info ["naming-table-path"] ~doc ~docv
  in
  let dep_table_path =
    let doc = "The path to the dependency table saved-state." in
    let docv = "PATH" in
    value & opt (some string) None & info ["dep-table-path"] ~doc ~docv
  in
  let watchman_sockname =
    let doc = "The path to the Watchman socket to use." in
    let docv = "PATH" in
    value & opt (some string) None & info ["watchman-sockname"] ~doc ~docv
  in
  let changed_files =
    let doc =
      String.strip
        {|
A file which has changed since last time `hh_fanout` was invoked.
May be specified multiple times.
Not necessary for the caller to pass unless Watchman is unavailable.
|}
    in
    let docv = "PATH" in
    (* Note: I think the following can be `file` as opposed to `string`, but
       I'm staying faithful to the original CLI. *)
    value & opt_all string [] & info ["changed-file"] ~doc ~docv
  in
  let state_path =
    let doc =
      String.strip
        {|
The path to the persistent state on disk.
If not provided, will use the default path for the repository.
|}
    in
    let docv = "PATH" in
    value & opt (some string) None & info ["state-path"] ~doc ~docv
  in
  Term.(
    const env
    $ from
    $ client_id
    $ root
    $ detail_level
    $ ignore_hh_version
    $ naming_table_path
    $ dep_table_path
    $ watchman_sockname
    $ changed_files
    $ state_path)

let clean_subcommand =
  let open Cmdliner in
  let doc = "Delete any state files which hh_fanout uses from disk." in

  let run env =
    let state_path = get_state_path ~env in
    Hh_logger.log "Deleting %s" (Path.to_string state_path);
    Sys_utils.rm_dir_tree (Path.to_string state_path)
  in
  let info = Cmd.info "clean" ~doc ~sdocs:Manpage.s_common_options in
  let term = Term.(const run $ env_t) in
  Cmd.v info term

let calculate_subcommand =
  let open Cmdliner in
  let open Cmdliner.Arg in
  let doc = "Determines which files must be rechecked after a change." in

  let input_files = value & pos_all string [] & info [] ~docv:"FILENAME" in
  let cursor_id =
    let doc = "The cursor that the previous request returned." in
    value & opt (some string) None & info ["cursor"] ~doc ~docv:"CURSOR"
  in

  let run env input_files cursor_id =
    let input_files =
      input_files
      |> Sys_utils.parse_path_list
      |> List.filter ~f:FindUtils.file_filter
      |> List.map ~f:Path.make
      |> Path.Set.of_list
    in
    if Path.Set.is_empty input_files then
      Hh_logger.warn "Warning: list of input files is empty.";

    Lwt_utils.run_main (fun () -> mode_calculate ~env ~input_files ~cursor_id)
  in
  let info = Cmd.info "calculate" ~doc ~sdocs:Manpage.s_common_options in
  let term = Term.(const run $ env_t $ input_files $ cursor_id) in
  Cmd.v info term

let calculate_errors_subcommand =
  let open Cmdliner in
  let open Cmdliner.Arg in
  let doc = "Produce typechecking errors for the codebase." in

  let cursor_id =
    let doc =
      String.strip
        {|
The cursor returned by a previous request to `calculate`.
If not provided, uses the cursor corresponding to the saved-state.
|}
    in
    value & opt (some string) None & info ["cursor"] ~doc ~docv:"CURSOR"
  in
  let pretty_print =
    let doc =
      "Pretty-print the errors to stdout, rather than returning a JSON object."
    in
    value & flag & info ["pretty-print"] ~doc
  in

  let run env cursor_id pretty_print =
    Lwt_utils.run_main (fun () ->
        mode_calculate_errors ~env ~cursor_id ~pretty_print)
  in
  let info = Cmd.info "calculate-errors" ~doc ~sdocs:Manpage.s_common_options in
  let term = Term.(const run $ env_t $ cursor_id $ pretty_print) in
  Cmd.v info term

let mode_debug ~(env : env) ~(path : Path.t) ~(cursor_id : string option) :
    unit Lwt.t =
  let%lwt saved_state_result = load_saved_state ~env in
  let previous_cursor_reference =
    match cursor_id with
    | Some _ ->
      Hh_logger.warn
        ("A cursor ID was passed to `debug`, "
        ^^ "but loading from a previous cursor is not yet implemented.");
      Cursor_reference_from_saved_state saved_state_result
    | None -> Cursor_reference_from_saved_state saved_state_result
  in

  let path = Relative_path.create_detect_prefix (Path.to_string path) in
  let input_files = Relative_path.Set.singleton path in
  let incremental_state = make_incremental_state ~env in
  let (previous_cursor, previous_changed_files) =
    resolve_cursor_reference ~env ~incremental_state ~previous_cursor_reference
  in
  let cursor =
    advance_cursor
      ~env
      ~setup_result:saved_state_result.setup_result
      ~previous_cursor
      ~previous_changed_files
      ~input_files
  in
  let file_deltas = cursor#get_file_deltas in
  let new_naming_table =
    Naming_table.update_from_deltas saved_state_result.naming_table file_deltas
  in

  let cursor_id = incremental_state#add_cursor cursor in

  let json =
    Debug_fanout.go
      ~ctx:saved_state_result.setup_result.ctx
      ~workers:saved_state_result.setup_result.workers
      ~old_naming_table:saved_state_result.naming_table
      ~new_naming_table
      ~file_deltas
      ~path
    |> Debug_fanout.result_to_json
  in
  let json =
    Hh_json.JSON_Object
      [
        ( "cursor",
          let (Incremental.Cursor_id cursor_id) = cursor_id in
          Hh_json.JSON_String cursor_id );
        ("debug", json);
      ]
  in
  Hh_json.json_to_multiline_output Out_channel.stdout json;
  Lwt.return_unit

let debug_subcommand =
  let open Cmdliner in
  let open Cmdliner.Arg in
  let doc =
    "Produces debugging information about the fanout of a certain file."
  in

  let path = required & pos 0 (some string) None & info [] ~docv:"PATH" in
  let cursor_id =
    let doc = "The cursor that the previous request returned." in
    value & opt (some string) None & info ["cursor"] ~doc ~docv:"CURSOR"
  in

  let run env path cursor_id =
    let path = Path.make path in
    Lwt_utils.run_main (fun () -> mode_debug ~env ~path ~cursor_id)
  in
  let info = Cmd.info "debug" ~doc ~sdocs:Manpage.s_common_options in
  let term = Term.(const run $ env_t $ path $ cursor_id) in
  Cmd.v info term

let mode_status ~(env : env) ~(cursor_id : string) : unit Lwt.t =
  let incremental_state = make_incremental_state ~env in
  let cursor =
    incremental_state#look_up_cursor ~client_id:None ~cursor_id
    |> Result.ok_or_failwith
  in
  let fanout_calculations =
    cursor#get_calculate_fanout_results_since_last_typecheck
  in
  let%lwt () = Status.go fanout_calculations in
  Lwt.return_unit

let status_subcommand =
  let open Cmdliner in
  let open Cmdliner.Arg in
  let doc =
    "EXPERIMENTAL: Shows details about the files that need to be re-typechecked on the next `calculate-errors` call."
  in

  let cursor_id =
    let doc = "The cursor that the previous request returned." in
    required & opt (some string) None & info ["cursor"] ~doc ~docv:"CURSOR"
  in

  let run env cursor_id =
    Lwt_utils.run_main (fun () -> mode_status ~env ~cursor_id)
  in
  let info = Cmd.info "status" ~doc ~sdocs:Manpage.s_common_options in
  let term = Term.(const run $ env_t $ cursor_id) in
  Cmd.v info term

let mode_query
    ~(env : env) ~(dep_hash : Typing_deps.Dep.t) ~(include_extends : bool) :
    unit Lwt.t =
  let%lwt (saved_state_result : saved_state_result) = load_saved_state ~env in
  let json =
    Query_fanout.go
      ~ctx:saved_state_result.setup_result.ctx
      ~dep_hash
      ~include_extends
    |> Query_fanout.result_to_json
  in
  let json = Hh_json.JSON_Object [("result", json)] in
  Hh_json.json_to_multiline_output Out_channel.stdout json;
  Lwt.return_unit

let query_subcommand =
  let open Cmdliner in
  let open Cmdliner.Arg in
  let doc = "Get the edges for which the given input node is a dependency." in

  let include_extends =
    let doc =
      "Traverse the extends dependencies for this node and include them in the output as well."
    in
    value & flag & info ["include-extends"] ~doc
  in
  let dep_hash = required & pos 0 (some string) None & info [] ~docv:"HASH" in

  let run env include_extends dep_hash =
    let dep_hash = Typing_deps.Dep.of_debug_string dep_hash in
    Lwt_utils.run_main (fun () -> mode_query ~env ~dep_hash ~include_extends)
  in
  let info = Cmd.info "query" ~doc ~sdocs:Manpage.s_common_options in
  let term = Term.(const run $ env_t $ include_extends $ dep_hash) in
  Cmd.v info term

let mode_query_path
    ~(env : env) ~(source : Typing_deps.Dep.t) ~(dest : Typing_deps.Dep.t) :
    unit Lwt.t =
  let%lwt (saved_state_result : saved_state_result) = load_saved_state ~env in
  let json =
    Query_path.go ~ctx:saved_state_result.setup_result.ctx ~source ~dest
    |> Query_path.result_to_json
  in
  let json = Hh_json.JSON_Object [("result", json)] in
  Hh_json.json_to_multiline_output Out_channel.stdout json;
  Lwt.return_unit

let query_path_subcommand =
  let open Cmdliner in
  let open Cmdliner.Arg in
  let doc =
    "Find a path of dependencies edges leading from one node to another."
  in
  let man =
    [
      `S Manpage.s_description;
      `P
        (String.strip
           {|
Produces a list of nodes in the dependency graph connected by typing- or
extends-dependency edges. This is a list of n nodes, where the leading pairs
are connected by extends-dependency edges, and the last pair is connected by
a typing-dependency edge.
|});
    ]
  in

  let source =
    required & pos 0 (some string) None & info [] ~docv:"SOURCE-HASH"
  in
  let dest = required & pos 1 (some string) None & info [] ~docv:"DEST-HASH" in

  let run env source dest =
    let source = Typing_deps.Dep.of_debug_string source in
    let dest = Typing_deps.Dep.of_debug_string dest in
    Lwt_utils.run_main (fun () -> mode_query_path ~env ~source ~dest)
  in
  let info = Cmd.info "query-path" ~doc ~sdocs:Manpage.s_common_options ~man in
  let term = Term.(const run $ env_t $ source $ dest) in
  Cmd.v info term

let mode_build = Build.go

let build_subcommand =
  let open Cmdliner in
  let open Cmdliner.Arg in
  let doc = "Build the 64-bit graph from a collection of edges" in
  let man =
    [
      `S Manpage.s_description;
      `P
        (String.strip
           {|
Produces the 64-bit dependency graph from a collection of edges stored in a
set of binary files. The files containing the dependency graph edges are meant
to be produced by hh_server
|});
    ]
  in

  let allow_empty =
    let doc =
      "Do not fail when produced dependency graph is empty. By default, the tool"
      ^ " exits with a non-zero exit code when trying to produce an empty graph,"
      ^ " as most likely, this only happens when something has gone wrong (a bug)."
      ^ " However, producing empty graphs can still be useful! (E.g. in tests)"
    in
    value & flag & info ["allow-empty"] ~doc
  in
  let incremental =
    let doc =
      "Use the provided dependency graph as a base. Build a new dependency graph"
      ^ " by adding the edges in EDGES_DIR or DELTA_FILE to this graph."
    in
    value
    & opt (some string) None
    & info ["incremental"] ~doc ~docv:"INCREMENTAL_HHDG"
  in

  let edges_dir =
    let doc = "A directory containing the .bin files with all the edges." in
    value & opt (some string) None & info ["edges-dir"] ~doc ~docv:"EDGES_DIR"
  in
  let delta_file =
    let doc =
      "A file containing a dependency graph delta in binary format."
      ^ " The files should contain edges as produced by calling"
      ^ " `hh --save-state /path/to/file` (which is a special binary format)."
    in
    value & opt (some string) None & info ["delta-file"] ~doc ~docv:"DELTA_FILE"
  in
  let output =
    let doc = "Where to put the 64-bit dependency graph." in
    required & opt (some string) None & info ["output"] ~doc ~docv:"OUTPUT"
  in
  let run allow_empty incremental edges_dir delta_file output =
    Lwt_utils.run_main (fun () ->
        mode_build ~allow_empty ~incremental ~edges_dir ~delta_file ~output)
  in
  let info = Cmd.info "build" ~doc ~sdocs:Manpage.s_common_options ~man in
  let term =
    Term.(
      const run $ allow_empty $ incremental $ edges_dir $ delta_file $ output)
  in
  Cmd.v info term

let mode_dep_graph_stats = Dep_graph_stats.go

let dep_graph_stats_subcommand =
  let open Cmdliner in
  let open Cmdliner.Arg in
  let doc = "Calculate some statistics for the 64-bit dependency graph" in
  let man =
    [
      `S Manpage.s_description;
      `P
        (String.strip
           {|
Calculate a bunch of statistics for a given 64-bit dependency graph.
|});
    ]
  in

  let dep_graph =
    let doc = "Path to a 64-bit dependency graph." in
    required
    & opt (some string) None
    & info ["dep-graph"] ~doc ~docv:"DEP_GRAPH"
  in
  let run dep_graph =
    Lwt_utils.run_main (fun () -> mode_dep_graph_stats ~dep_graph)
  in
  let info =
    Cmd.info "dep-graph-stats" ~doc ~sdocs:Manpage.s_common_options ~man
  in
  let term = Term.(const run $ dep_graph) in
  Cmd.v info term

let mode_dep_graph_is_subgraph = Dep_graph_is_subgraph.go

let dep_graph_is_subgraph_subcommand =
  let open Cmdliner in
  let open Cmdliner.Arg in
  let doc = "Check whether SUB is a subgraph of SUPER" in
  let man =
    [
      `S Manpage.s_description;
      `P
        (String.strip
           {|
Check whether a 64-bit dependency graph is a subgraph of an other graph.
|});
    ]
  in

  let dep_graph_sub =
    let doc = "Path to smallest 64-bit dependency graph." in
    required & opt (some string) None & info ["sub"] ~doc ~docv:"SUB"
  in
  let dep_graph_super =
    let doc = "Path to largest 64-bit dependency graph." in
    required & opt (some string) None & info ["super"] ~doc ~docv:"SUPER"
  in
  let run sub super =
    Lwt_utils.run_main (fun () -> mode_dep_graph_is_subgraph ~sub ~super)
  in
  let info =
    Cmd.info "dep-graph-is-subgraph" ~doc ~sdocs:Manpage.s_common_options ~man
  in
  let term = Term.(const run $ dep_graph_sub $ dep_graph_super) in
  Cmd.v info term

let default_subcommand =
  let open Cmdliner in
  let sdocs = Manpage.s_common_options in
  Term.(ret (const (`Help (`Pager, None))), Cmd.info "hh_fanout" ~sdocs)

let () =
  EventLogger.init EventLogger.Event_logger_fake 0.0;
  Daemon.check_entry_point ();
  Folly.ensure_folly_init ();
  let cmds =
    [
      build_subcommand;
      calculate_subcommand;
      calculate_errors_subcommand;
      clean_subcommand;
      debug_subcommand;
      dep_graph_is_subgraph_subcommand;
      dep_graph_stats_subcommand;
      query_subcommand;
      query_path_subcommand;
      status_subcommand;
    ]
  in
  let (default, default_info) = default_subcommand in
  let group = Cmdliner.Cmd.group ~default default_info cmds in
  Stdlib.exit (Cmdliner.Cmd.eval group)
