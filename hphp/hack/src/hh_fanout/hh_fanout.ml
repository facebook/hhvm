(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

module Hh_daemon = Daemon
open Core

type env = {
  from: string;
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
}

type previous_cursor =
  | Previous_cursor_from_saved_state of saved_state_result
  | Previous_cursor_id of string

let set_up_global_environment (env : env) : setup_result =
  let server_args =
    ServerArgs.default_options_with_check_mode ~root:(Path.to_string env.root)
  in
  let (server_config, server_local_config) =
    ServerConfig.load ~silent:false ServerConfig.filename server_args
  in
  let genv =
    ServerEnvBuild.make_genv server_args server_config server_local_config []
    (* no workers *)
  in
  let init_id = Random_id.short_string () in
  let server_env = ServerEnvBuild.make_env ~init_id genv.ServerEnv.config in
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

  let (ctx, workers, _time_taken) =
    Batch_init.init
      ~root:env.root
      ~shmem_config:(ServerConfig.sharedmem_config server_config)
      ~popt:server_env.ServerEnv.popt
      ~tcopt:server_env.ServerEnv.tcopt
      (Unix.gettimeofday ())
  in
  { workers; ctx }

let load_saved_state ~(env : env) ~(setup_result : setup_result) :
    saved_state_result Lwt.t =
  let%lwt (naming_table_path, naming_table_changed_files) =
    match env.naming_table_path with
    | Some naming_table_path -> Lwt.return (naming_table_path, [])
    | None ->
      let%lwt naming_table_saved_state =
        State_loader_lwt.load
          ~watchman_opts:
            Saved_state_loader.Watchman_options.
              { root = env.root; sockname = env.watchman_sockname }
          ~ignore_hh_version:env.ignore_hh_version
          ~saved_state_type:Saved_state_loader.Naming_table
      in
      (match naming_table_saved_state with
      | Error load_error ->
        failwith
          (Printf.sprintf
             "Failed to load naming-table saved-state, and saved-state files were not manually provided on command-line: %s"
             (Saved_state_loader.debug_details_of_error load_error))
      | Ok { Saved_state_loader.saved_state_info; changed_files; _ } ->
        Lwt.return
          ( saved_state_info
              .Saved_state_loader.Naming_table_info.naming_table_path,
            changed_files ))
  and (dep_table_path, errors_path, dep_table_changed_files) =
    match env.dep_table_path with
    | Some dep_table_path ->
      let errors_path =
        dep_table_path
        |> Path.to_string
        |> Filename.split_extension
        |> fst
        |> SaveStateService.get_errors_filename
        |> Path.make
      in
      Lwt.return (dep_table_path, errors_path, [])
    | None ->
      let%lwt dep_table_saved_state =
        State_loader_lwt.load
          ~watchman_opts:
            Saved_state_loader.Watchman_options.
              { root = env.root; sockname = env.watchman_sockname }
          ~ignore_hh_version:env.ignore_hh_version
          ~saved_state_type:Saved_state_loader.Naming_and_dep_table
      in
      (match dep_table_saved_state with
      | Error load_error ->
        failwith
          (Printf.sprintf
             "Failed to load dep-table saved-state, and saved-state files were not manually provided on command-line: %s"
             (Saved_state_loader.debug_details_of_error load_error))
      | Ok { Saved_state_loader.saved_state_info; changed_files; _ } ->
        Lwt.return
          ( saved_state_info
              .Saved_state_loader.Naming_and_dep_table_info.dep_table_path,
            saved_state_info
              .Saved_state_loader.Naming_and_dep_table_info.errors_path,
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

  let naming_table =
    Naming_table.load_from_sqlite
      setup_result.ctx
      (Path.to_string naming_table_path)
  in
  let naming_table =
    Relative_path.Set.fold
      changed_files
      ~init:naming_table
      ~f:(fun path naming_table ->
        let { ClientIdeIncremental.naming_table; _ } =
          ClientIdeIncremental.update_naming_tables_for_changed_file
            ~naming_table
            ~sienv:SearchUtils.quiet_si_env
            ~backend:(Provider_context.get_backend setup_result.ctx)
            ~popt:(Provider_context.get_popt setup_result.ctx)
            ~path
        in
        naming_table)
  in
  Lwt.return
    {
      naming_table;
      naming_table_path;
      dep_table_path;
      errors_path;
      saved_state_changed_files = changed_files;
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
  let state_path = get_state_path env in
  Hh_logger.log "State path: %s" (Path.to_string state_path);
  Incremental.make_reference_implementation state_path

let advance_cursor
    ~(env : env)
    ~(setup_result : setup_result)
    ~(incremental_state : Incremental.state)
    ~(previous_cursor : previous_cursor)
    ~(input_files : Relative_path.Set.t) : Incremental.cursor =
  let (cursor, cursor_changed_files) =
    match previous_cursor with
    | Previous_cursor_from_saved_state saved_state_result ->
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
          }
      in
      let cursor =
        incremental_state#make_default_cursor client_id |> Result.ok_or_failwith
      in
      (cursor, saved_state_result.saved_state_changed_files)
    | Previous_cursor_id cursor_id ->
      let cursor =
        incremental_state#look_up_cursor
          ~client_id:(Some (Incremental.Client_id env.client_id))
          ~cursor_id
        |> Result.ok_or_failwith
      in
      (cursor, Relative_path.Set.empty)
  in
  let cursor_changed_files =
    cursor_changed_files
    |> Relative_path.Set.union env.changed_files
    |> Relative_path.Set.union input_files
  in
  cursor#advance
    ~detail_level:env.detail_level
    setup_result.ctx
    setup_result.workers
    cursor_changed_files

let mode_calculate
    ~(env : env) ~(input_files : Path.Set.t) ~(cursor_id : string option) :
    unit Lwt.t =
  let telemetry = Telemetry.create () in
  let setup_result = set_up_global_environment env in
  let%lwt previous_cursor =
    match cursor_id with
    | None ->
      let%lwt saved_state_result = load_saved_state ~env ~setup_result in
      Lwt.return (Previous_cursor_from_saved_state saved_state_result)
    | Some cursor_id -> Lwt.return (Previous_cursor_id cursor_id)
  in

  let input_files =
    Path.Set.fold input_files ~init:Relative_path.Set.empty ~f:(fun path acc ->
        let path = Relative_path.create_detect_prefix (Path.to_string path) in
        Relative_path.Set.add acc path)
  in
  let incremental_state = make_incremental_state ~env in
  let cursor =
    advance_cursor
      ~env
      ~setup_result
      ~incremental_state
      ~previous_cursor
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
        ( "Internal invariant failure -- "
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
            ( fanout_files
            |> Relative_path.Set.elements
            |> List.map ~f:Relative_path.to_absolute
            |> List.map ~f:Hh_json.string_ ) );
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
  let { ctx; workers } = set_up_global_environment env in
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

  let (errors, cursor) = cursor#calculate_errors ctx workers in
  let cursor_id =
    match cursor with
    | Some cursor -> incremental_state#add_cursor cursor
    | None ->
      let cursor_id =
        Option.value_exn
          cursor_id
          ~message:
            ( "Internal invariant failure -- "
            ^ "expected a new cursor to be generated, "
            ^ "given that no cursor was passed in." )
      in
      Incremental.Cursor_id cursor_id
  in

  let error_list =
    errors |> Errors.get_sorted_error_list |> List.map ~f:Errors.to_absolute
  in
  ( if pretty_print then
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
    Hh_json.json_to_multiline_output Out_channel.stdout json );
  Lwt.return_unit

let detail_level_arg =
  Command.Arg_type.create (fun x ->
      match x with
      | "low" -> Calculate_fanout.Detail_level.Low
      | "high" -> Calculate_fanout.Detail_level.High
      | other ->
        Printf.eprintf
          "Invalid detail level: %s (valid values are 'low', 'high')"
          other;
        exit 1)

let dep_hash_arg = Command.Arg_type.create Typing_deps.Dep.of_debug_string

let path_arg = Command.Arg_type.create Path.make

let parse_env () =
  let open Command.Param in
  let open Command.Let_syntax in
  let%map from =
    flag
      "--from"
      (required string)
      ~doc:"FROM A descriptive string indicating the caller of this program."
  and client_id =
    flag
      "--client-id"
      (optional string)
      ~doc:
        ( "CLIENT-ID A string identifying the caller of this program. "
        ^ "Use the same string across multiple callers to reuse hh_fanout cursors and intermediate results. "
        ^ "If not provided, defaults to the value for 'from'." )
  and root =
    flag
      "--root"
      (optional string)
      ~doc:
        "DIR The root directory to run in. If not set, will attempt to locate one by searching upwards for an `.hhconfig` file."
  and detail_level =
    flag
      "--detail-level"
      (optional_with_default Calculate_fanout.Detail_level.Low detail_level_arg)
      ~doc:
        "VERBOSITY How much debugging output to include in the result. May slow down the query."
  and ignore_hh_version =
    flag
      "--ignore-hh-version"
      no_arg
      ~doc:
        "Skip the consistency check for the version that this program was built with versus the version of the server that built the saved-state."
  and naming_table_path =
    flag
      "--naming-table-path"
      (optional path_arg)
      ~doc:"PATH The path to the naming table SQLite saved-state."
  and dep_table_path =
    flag
      "--dep-table-path"
      (optional path_arg)
      ~doc:"PATH The path to the dependency table saved-state."
  and watchman_sockname =
    flag
      "--watchman-sockname"
      (optional path_arg)
      ~doc:"PATH The path to the Watchman socket to use."
  and changed_files =
    flag
      "--changed-file"
      (listed string)
      ~doc:
        ( "PATH A file which has changed since last time `hh_fanout` was invoked. "
        ^ "May be specified multiple times. "
        ^ "Not necessary for the caller to pass unless Watchman is unavailable."
        )
  and state_path =
    flag
      "--state-path"
      (optional path_arg)
      ~doc:
        ( "PATH The path to the persistent state on disk. "
        ^ "If not provided, will use the default path for the repository." )
  in
  let root =
    match root with
    | Some root -> Path.make root
    | None -> Wwwroot.get None
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
    match client_id with
    | Some client_id -> client_id
    | None -> from
  in

  {
    from;
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

let clean_subcommand =
  let open Command.Let_syntax in
  Command.basic
    ~summary:"Delete any state files which hh_fanout uses from disk."
    (let%map env = parse_env () in
     fun () ->
       let state_path = get_state_path env in
       Hh_logger.log "Deleting %s" (Path.to_string state_path);
       Sys_utils.rm_dir_tree (Path.to_string state_path))

let calculate_subcommand =
  let open Command.Param in
  let open Command.Let_syntax in
  Command.basic
    ~summary:"Determines which files must be rechecked after a change"
    (let%map env = parse_env ()
     and input_files = anon (sequence ("filename" %: string))
     and cursor_id =
       flag
         "--cursor"
         (optional string)
         ~doc:"CURSOR The cursor that the previous request returned."
     in

     let input_files =
       input_files
       |> Sys_utils.parse_path_list
       |> List.filter ~f:FindUtils.file_filter
       |> List.map ~f:Path.make
       |> Path.Set.of_list
     in
     if Path.Set.is_empty input_files then
       Hh_logger.warn "Warning: list of input files is empty.";

     (fun () -> Lwt_main.run (mode_calculate ~env ~input_files ~cursor_id)))

let calculate_errors_subcommand =
  let open Command.Param in
  let open Command.Let_syntax in
  Command.basic
    ~summary:"Produce typechecking errors for the codebase"
    (let%map env = parse_env ()
     and cursor_id =
       flag
         "--cursor"
         (optional string)
         ~doc:
           ( "CURSOR The cursor returned by a previous request to `calculate`. "
           ^ "If not provided, uses the cursor corresponding to the saved-state."
           )
     and pretty_print =
       flag
         "--pretty-print"
         no_arg
         ~doc:
           "Pretty-print the errors to stdout, rather than returning a JSON object."
     in

     fun () ->
       Lwt_main.run (mode_calculate_errors ~env ~cursor_id ~pretty_print))

let mode_debug ~(env : env) ~(path : Path.t) ~(cursor_id : string option) :
    unit Lwt.t =
  let ({ ctx; workers } as setup_result) = set_up_global_environment env in
  let%lwt ({ naming_table = old_naming_table; _ } as saved_state_result) =
    load_saved_state ~env ~setup_result
  in
  let previous_cursor =
    match cursor_id with
    | Some _ ->
      Hh_logger.warn
        ( "A cursor ID was passed to `debug`, "
        ^^ "but loading from a previous cursor is not yet implemented." );
      Previous_cursor_from_saved_state saved_state_result
    | None -> Previous_cursor_from_saved_state saved_state_result
  in

  let path = Relative_path.create_detect_prefix (Path.to_string path) in
  let input_files = Relative_path.Set.singleton path in
  let incremental_state = make_incremental_state ~env in
  let cursor =
    advance_cursor
      ~env
      ~setup_result
      ~incremental_state
      ~previous_cursor
      ~input_files
  in
  let file_deltas = cursor#get_file_deltas in
  let new_naming_table =
    Naming_table.update_from_deltas old_naming_table file_deltas
  in

  let cursor_id = incremental_state#add_cursor cursor in

  let json =
    Debug_fanout.go
      ~ctx
      ~workers
      ~old_naming_table
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
  let open Command.Param in
  let open Command.Let_syntax in
  Command.basic
    ~summary:"Produces debugging information about the fanout of a certain file"
    (let%map env = parse_env ()
     and path = anon ("FILE" %: string)
     and cursor_id =
       flag
         "--cursor"
         (optional string)
         ~doc:"CURSOR The cursor that the previous request returned."
     in
     let path = Path.make path in
     (fun () -> Lwt_main.run (mode_debug ~env ~path ~cursor_id)))

let mode_query
    ~(env : env) ~(dep_hash : Typing_deps.Dep.t) ~(include_extends : bool) :
    unit Lwt.t =
  let setup_result = set_up_global_environment env in
  let%lwt (_saved_state_result : saved_state_result) =
    load_saved_state ~env ~setup_result
  in
  let json =
    Query_fanout.go ~dep_hash ~include_extends |> Query_fanout.result_to_json
  in
  let json = Hh_json.JSON_Object [("result", json)] in
  Hh_json.json_to_multiline_output Out_channel.stdout json;
  Lwt.return_unit

let query_subcommand =
  let open Command.Param in
  let open Command.Let_syntax in
  Command.basic
    ~summary:"Get the edges for which the given input node is a dependency"
    (let%map env = parse_env ()
     and include_extends =
       flag
         "--include-extends"
         no_arg
         ~doc:
           "Traverse the extends dependencies for this node and include them in the output as well"
     and dep_hash = anon ("HASH" %: dep_hash_arg) in
     (fun () -> Lwt_main.run (mode_query ~env ~dep_hash ~include_extends)))

let mode_query_path
    ~(env : env) ~(source : Typing_deps.Dep.t) ~(dest : Typing_deps.Dep.t) :
    unit Lwt.t =
  let setup_result = set_up_global_environment env in
  let%lwt (_saved_state_result : saved_state_result) =
    load_saved_state ~env ~setup_result
  in
  let json = Query_path.go ~source ~dest |> Query_path.result_to_json in
  let json = Hh_json.JSON_Object [("result", json)] in
  Hh_json.json_to_multiline_output Out_channel.stdout json;
  Lwt.return_unit

let query_path_subcommand =
  let open Command.Param in
  let open Command.Let_syntax in
  Command.basic
    ~summary:
      "Find a path of dependencies edges leading from one node to another"
    ~readme:(fun () ->
      String.strip
        {|
Produces a list of nodes in the dependency graph connected by typing- or
extends-dependency edges. This is a list of n nodes, where the leading pairs
are connected by extends-dependency edges, and the last pair is connected by
a typing-dependency edge.
|})
    (let%map env = parse_env ()
     and source = anon ("SOURCE-HASH" %: dep_hash_arg)
     and dest = anon ("DEST-HASH" %: dep_hash_arg) in
     (fun () -> Lwt_main.run (mode_query_path ~env ~source ~dest)))

let () =
  EventLogger.init EventLogger.Event_logger_fake 0.0;
  Hh_daemon.check_entry_point ();
  Command.run
  @@ Command.group
       ~summary:"Provides access to Hack's dependency graph"
       [
         ("clean", clean_subcommand);
         ("calculate", calculate_subcommand);
         ("calculate-errors", calculate_errors_subcommand);
         ("debug", debug_subcommand);
         ("query", query_subcommand);
         ("query-path", query_path_subcommand);
       ]
