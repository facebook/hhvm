(*
 * Copyright (c) 2016, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *
 *)

open Hh_prelude
open Integration_test_base_types
open Reordered_argument_collections
open ServerCommandTypes
open SearchServiceRunner
open Coverage_level
open Coverage_level_defs
open Int.Replace_polymorphic_compare

exception Integration_test_failure

let root = "/"

let hhi = "/hhi"

let tmp = "/tmp"

let () = Hh_logger.Level.set_min_level Hh_logger.Level.Off

let server_config = ServerEnvBuild.default_genv.ServerEnv.config

let global_opts =
  GlobalOptions.make
    ~po_disable_xhp_element_mangling:false
    ~po_deregister_php_stdlib:true
    ()

let server_config = ServerConfig.set_tc_options server_config global_opts

let server_config = ServerConfig.set_parser_options server_config global_opts

let genv =
  ref { ServerEnvBuild.default_genv with ServerEnv.config = server_config }

let did_init = ref false

let real_hhi_files () = Hhi.get_raw_hhi_contents () |> Array.to_list

(* Init part common to fresh and saved state init *)
let test_init_common ?(hhi_files = []) () =
  if !did_init then
    failwith
      ( "Initializing the server twice in same process. There is no guarantee of global state "
      ^ "cleanup between them. Split your test in multiple separate tests, or run individual test "
      ^ "cases in separate processes using Test.in_daemon." )
  else
    did_init := true;

  Unix.putenv "HH_TEST_MODE" "1";
  Printexc.record_backtrace true;
  EventLogger.init_fake ();
  Relative_path.set_path_prefix Relative_path.Root (Path.make root);
  Relative_path.set_path_prefix Relative_path.Hhi (Path.make hhi);
  Relative_path.set_path_prefix Relative_path.Tmp (Path.make tmp);

  let handle = SharedMem.init ~num_workers:0 SharedMem.default_config in
  ignore (handle : SharedMem.handle);

  ServerMain.force_break_recheck_loop_for_test true;

  List.iter hhi_files ~f:(fun (fn, contents) ->
      TestDisk.set (Filename.concat hhi fn) contents);
  ()

(* Hhi files are loaded during server setup. If given a list of string + contents, we add them
to the test disk and add them to disk_needs_parsing. After one server run loop, they will be loaded.
This isn't exactly the same as how initialization does it, but the purpose is not to test the hhi
files, but to test incremental mode behavior with Hhi files present.
*)
let setup_server ?custom_config ?(hhi_files = []) () =
  test_init_common () ~hhi_files;

  let init_id = Random_id.short_string () in
  let deps_mode = Typing_deps_mode.SQLiteMode in
  let result =
    match custom_config with
    | Some config -> ServerEnvBuild.make_env ~init_id ~deps_mode config
    | None -> ServerEnvBuild.make_env ~init_id ~deps_mode !genv.ServerEnv.config
  in
  let hhi_file_list =
    List.map hhi_files ~f:(fun (fn, _) ->
        Relative_path.create Relative_path.Hhi (Filename.concat hhi fn))
  in
  let hhi_set = Relative_path.Set.of_list hhi_file_list in

  (* Initialize symbol index *)
  let sienv =
    SymbolIndex.initialize
      ~globalrev:None
      ~gleanopt:result.ServerEnv.gleanopt
      ~namespace_map:[]
      ~provider_name:"LocalIndex"
      ~quiet:true
      ~ignore_hh_version:false
      ~savedstate_file_opt:None
      ~workers:None
  in
  (* Return environment *)
  {
    result with
    ServerEnv.disk_needs_parsing = hhi_set;
    ServerEnv.local_symbol_table = sienv;
  }

let default_loop_input =
  { disk_changes = []; new_client = None; persistent_client_request = None }

let run_loop_once :
    type a b.
    ServerEnv.env -> (a, b) loop_inputs -> ServerEnv.env * (a, b) loop_outputs =
 fun env inputs ->
  TestClientProvider.clear ();
  Option.iter inputs.new_client ~f:(function
      | RequestResponse x ->
        TestClientProvider.mock_new_client_type Non_persistent;
        TestClientProvider.mock_client_request x
      | ConnectPersistent -> TestClientProvider.mock_new_client_type Persistent);

  Option.iter inputs.persistent_client_request ~f:(function
      | Request x -> TestClientProvider.mock_persistent_client_request x
      | UncleanDisconect x ->
        TestClientProvider.mock_persistent_client_request x;
        TestClientProvider.mock_unclean_disconnect ());

  let client_provider = ClientProvider.provider_for_test () in
  let disk_changes =
    List.map inputs.disk_changes ~f:(fun (x, y) -> (root ^ x, y))
  in
  List.iter disk_changes ~f:(fun (path, contents) -> TestDisk.set path contents);

  let did_read_disk_changes_ref = ref false in
  let notifier () =
    if not !did_read_disk_changes_ref then (
      did_read_disk_changes_ref := true;
      SSet.of_list (List.map disk_changes ~f:fst)
    ) else
      SSet.empty
  in
  let genv =
    {
      !genv with
      ServerEnv.notifier_async =
        (fun () ->
          ServerNotifierTypes.Notifier_synchronous_changes (notifier ()));
      ServerEnv.notifier;
    }
  in
  (* Always pick up disk changes in tests immediately *)
  let env = ServerEnv.{ env with last_notifier_check_time = 0.0 } in
  let env = ServerMain.serve_one_iteration genv env client_provider in
  let ctx = Provider_utils.ctx_from_server_env env in
  let env =
    {
      env with
      ServerEnv.local_symbol_table =
        SearchServiceRunner.run_completely ctx env.ServerEnv.local_symbol_table;
    }
  in
  ( env,
    {
      did_read_disk_changes = !did_read_disk_changes_ref;
      rechecked_count =
        env.ServerEnv.last_recheck_loop_stats.ServerEnv.rechecked_count;
      total_rechecked_count =
        env.ServerEnv.last_recheck_loop_stats.ServerEnv.total_rechecked_count;
      last_actual_total_rechecked_count =
        (match env.ServerEnv.last_recheck_loop_stats_for_actual_work with
        | None -> None
        | Some stats -> Some stats.ServerEnv.total_rechecked_count);
      new_client_response =
        TestClientProvider.get_client_response Non_persistent;
      persistent_client_response =
        TestClientProvider.get_client_response Persistent;
      push_messages = TestClientProvider.get_push_messages ();
    } )

let prepend_root x = root ^ x

let fail x =
  print_endline x;
  Caml.Printexc.(get_callstack 100 |> print_raw_backtrace stderr);
  raise Integration_test_failure

(******************************************************************************(
 * Utility functions to help format/throw errors for informative errors
)******************************************************************************)
let indent_string_with (indent : string) (error : string) : string =
  indent ^ String.concat ~sep:("\n" ^ indent) Str.(split (regexp "\n") error)

let indent_strings_with (indent : string) (errors : string list) : string =
  String.concat ~sep:"" @@ List.map ~f:(indent_string_with indent) errors

let fail_on_none (error : string) optional_thing =
  match optional_thing with
  | None -> fail error
  | Some _ -> ()

let assert_responded (error : string) loop_output =
  fail_on_none error loop_output.persistent_client_response

let assertEqual expected got =
  let expected = String.strip expected in
  let got = String.strip got in
  if String.( <> ) expected got then
    fail (Printf.sprintf "Expected:\n%s\nGot:\n%s\n" expected got)

let change_files env disk_changes =
  let (env, loop_output) =
    run_loop_once env { default_loop_input with disk_changes }
  in
  if not loop_output.did_read_disk_changes then
    fail "Expected the server to process disk updates";
  (env, loop_output)

let setup_disk env disk_changes = fst @@ change_files env disk_changes

let connect_persistent_client env =
  let (env, _) =
    run_loop_once
      env
      { default_loop_input with new_client = Some ConnectPersistent }
  in
  fail_on_none
    "Expected persistent client to be connected"
    env.ServerEnv.persistent_client;
  env

let assert_errors_in_phase
    (env : ServerEnv.env) (expected_count : int) (phase : Errors.phase) :
    ServerEnv.env =
  let all_phases =
    [Errors.Parsing; Errors.Decl; Errors.Naming; Errors.Typing; Errors.Init]
  in
  let errors_in_phases =
    List.map
      ~f:(fun (phase : Errors.phase) ->
        (phase, Errors.get_failed_files env.ServerEnv.errorl phase))
      all_phases
  in
  let (decl_error_files, other_error_files) =
    SaveStateService.partition_error_files_tf errors_in_phases [phase]
  in
  let count_of_errors = Relative_path.Set.cardinal decl_error_files in
  if count_of_errors <> expected_count then
    fail
      (Printf.sprintf
         "Expected %d errors in phase %s but got %d"
         expected_count
         (Errors.phase_to_string phase)
         count_of_errors);

  if Relative_path.Set.cardinal other_error_files <> 0 then
    fail (Printf.sprintf "Expected %d" expected_count);
  env

let assertSingleError expected err_list =
  let error_strings =
    List.map ~f:(fun x -> Errors.(to_string (to_absolute x))) err_list
  in
  match error_strings with
  | [x] -> assertEqual expected x
  | _ ->
    let err_count = List.length err_list in
    let fmt_expected = indent_string_with " < " expected in
    let fmt_actual = indent_strings_with " > " error_strings in
    let msg =
      Printf.sprintf
        "Expected to have exactly one error, namely:

%s

... but got %d errors...

%s
"
        fmt_expected
        err_count
        fmt_actual
    in
    fail msg

let subscribe_diagnostic ?(id = 4) ?error_limit env =
  let (env, _) =
    run_loop_once
      env
      {
        default_loop_input with
        persistent_client_request =
          Some (Request (SUBSCRIBE_DIAGNOSTIC { id; error_limit }));
      }
  in
  fail_on_none
    "Expected to subscribe to push diagnostics"
    env.ServerEnv.diag_subscribe;
  env

let open_file env ?contents file_name =
  let file_name = root ^ file_name in
  let contents =
    match contents with
    | Some contents -> contents
    | _ -> TestDisk.get file_name
  in
  let (env, loop_output) =
    run_loop_once
      env
      {
        default_loop_input with
        persistent_client_request =
          Some (Request (OPEN_FILE (file_name, contents)));
      }
  in
  assert_responded "Expected OPEN_FILE to be processeded" loop_output;
  env

let edit_file env name contents =
  let (env, loop_output) =
    run_loop_once
      env
      {
        default_loop_input with
        persistent_client_request =
          Some
            (Request
               (EDIT_FILE
                  ( root ^ name,
                    [{ Ide_api_types.range = None; text = contents }] )));
      }
  in
  assert_responded "Expected EDIT_FILE to be processed" loop_output;
  (env, loop_output)

let save_file env name contents =
  let (env, loop_output) =
    run_loop_once
      env
      { default_loop_input with disk_changes = [(name, contents)] }
  in
  if not loop_output.did_read_disk_changes then
    fail "Expected the server to process disk updates";
  (env, loop_output)

let close_file ?(ignore_response = false) env name =
  let (env, loop_output) =
    run_loop_once
      env
      {
        default_loop_input with
        persistent_client_request = Some (Request (CLOSE_FILE (root ^ name)));
      }
  in
  if not ignore_response then
    assert_responded "Expected CLOSE_FILE to be processed" loop_output;
  (env, loop_output)

let wait env =
  (* We simulate waiting one second since last command by manipulating
   * last_command_time. Will not work on timers that compare against other
   * counters. *)
  ServerEnv.{ env with last_command_time = env.last_command_time -. 1.0 }

let coverage_levels env filename =
  let file_input = ServerCommandTypes.FileName filename in
  run_loop_once
    env
    {
      default_loop_input with
      persistent_client_request =
        Some (Request (COVERAGE_LEVELS (filename, file_input)));
    }

let coverage_counts env contents =
  run_loop_once
    env
    {
      default_loop_input with
      persistent_client_request = Some (Request (COVERAGE_COUNTS contents));
    }

let autocomplete env contents =
  run_loop_once
    env
    {
      default_loop_input with
      persistent_client_request =
        Some (Request (COMMANDLINE_AUTOCOMPLETE contents));
    }

let ide_autocomplete env (path, line, column) =
  let is_manually_invoked = false in
  run_loop_once
    env
    {
      default_loop_input with
      persistent_client_request =
        Some
          (Request
             (IDE_AUTOCOMPLETE
                ( root ^ path,
                  Ide_api_types.{ line; column },
                  is_manually_invoked )));
    }

let status ?(ignore_ide = false) ?(max_errors = None) ?(remote = false) env =
  run_loop_once
    env
    {
      default_loop_input with
      new_client =
        Some
          (RequestResponse
             (ServerCommandTypes.STATUS { ignore_ide; max_errors; remote }));
    }

let full_check_status env =
  (* the empty string isn't just a test placeholder - when a file is deleted
     or renamed, the content is set to the empty string internally *)
  run_loop_once
    env
    {
      default_loop_input with
      disk_changes = [("__dummy_file_to_trigger_full_check.php", "")];
    }

let start_initial_full_check env =
  (match env.ServerEnv.prechecked_files with
  | ServerEnv.Initial_typechecking _ -> ()
  | _ -> assert false);

  let (env, loop_output) = full_check_status env in
  (match env.ServerEnv.prechecked_files with
  | ServerEnv.Prechecked_files_ready _ -> ()
  | _ -> assert false);

  let { total_rechecked_count; _ } = loop_output in
  (* full_check_status adds a dummy file to trigger a recheck, so we subtract one here
     and return the number of rechecked non-dummy files. *)
  assert (total_rechecked_count >= 1);
  (env, total_rechecked_count - 1)

let assert_no_diagnostics loop_output =
  match loop_output.push_messages with
  | DIAGNOSTIC _ :: _ -> fail "Did not expect to receive push diagnostics."
  | NEW_CLIENT_CONNECTED :: _ -> fail "Unexpected push message"
  | _ -> ()

let assert_has_diagnostics loop_output =
  match loop_output.push_messages with
  | DIAGNOSTIC _ :: _ -> ()
  | BUSY_STATUS s :: _ ->
    let msg =
      match s with
      | Needs_local_typecheck -> "Needs_local_typecheck"
      | Doing_local_typecheck -> "Doing_local_typecheck"
      | Done_local_typecheck -> "Done_local_typecheck"
      | Doing_global_typecheck _ -> "Doing_global_typecheck"
      | Done_global_typecheck _ -> "Done_global_typecheck"
    in
    let msg =
      Printf.sprintf "Expected DIAGNOSTIC, but got BUSY_STATUS %s." msg
    in
    fail msg
  | NEW_CLIENT_CONNECTED :: _ ->
    fail "Expected DIAGNOSTIC, but got NEW_CLIENT_CONNECTED."
  | FATAL_EXCEPTION e :: _
  | NONFATAL_EXCEPTION e :: _ ->
    let msg =
      Printf.sprintf
        "Expected DIAGNOSTIC, but got NON/FATAL_EXCEPTION:\n%s"
        e.Marshal_tools.message
    in
    fail msg
  | [] -> fail "Expected to receive push diagnostics."

let errors_to_string buf x =
  List.iter x ~f:(fun error ->
      Printf.bprintf buf "%s\n" (Errors.to_string error))

let print_telemetries env =
  Printf.eprintf "\n==Telementries==\n";
  List.iter
    ServerEnv.(env.last_recheck_loop_stats.per_batch_telemetry)
    ~f:(fun t -> Printf.eprintf "%s\n" (Telemetry.to_string ~pretty:true t))

let assert_errors errors expected =
  let buf = Buffer.create 1024 in
  Errors.get_error_list errors
  |> List.map ~f:Errors.to_absolute
  |> errors_to_string buf;
  assertEqual expected (Buffer.contents buf)

let assert_env_errors env expected = assert_errors env.ServerEnv.errorl expected

let assert_no_errors env = assert_env_errors env ""

let saved_state_filename = "test_saved_state"

let saved_naming_filename = "test_saved_naming"

let in_daemon f =
  let handle =
    Daemon.fork
      ~channel_mode:`socket
      Unix.(stdout, stderr)
      (fun () _channels -> f ())
      ()
  in
  match Unix.waitpid [] handle.Daemon.pid with
  | (_, Unix.WEXITED 0) -> ()
  | _ -> assert false

let save_state
    ?(load_hhi_files = false)
    ?(store_decls_in_saved_state =
      ServerLocalConfig.(default.store_decls_in_saved_state))
    ?(enable_naming_table_fallback = false)
    ?custom_config
    disk_changes
    temp_dir =
  in_daemon @@ fun () ->
  let hhi_files =
    if load_hhi_files then
      real_hhi_files ()
    else
      []
  in
  let env = setup_server () ?custom_config ~hhi_files in
  let env = setup_disk env disk_changes in
  assert_no_errors env;
  genv :=
    {
      !genv with
      ServerEnv.local_config =
        {
          !genv.ServerEnv.local_config with
          ServerLocalConfig.store_decls_in_saved_state;
          ServerLocalConfig.enable_naming_table_fallback;
        };
      ServerEnv.config =
        Option.value custom_config ~default:!genv.ServerEnv.config;
    };
  let _edges_added =
    ServerInit.save_state !genv env (temp_dir ^ "/" ^ saved_state_filename)
  in
  if enable_naming_table_fallback then (
    let _rows_added =
      SaveStateService.go_naming
        env.ServerEnv.naming_table
        (temp_dir ^ "/" ^ saved_naming_filename)
      |> Result.ok_or_failwith
    in
    ();
    ()
  )

let save_state_incremental
    env
    ?(store_decls_in_saved_state =
      ServerLocalConfig.(default.store_decls_in_saved_state))
    temp_dir =
  assert_no_errors env;
  genv :=
    {
      !genv with
      ServerEnv.local_config =
        {
          !genv.ServerEnv.local_config with
          ServerLocalConfig.store_decls_in_saved_state;
        };
    };
  ServerInit.save_state !genv env (temp_dir ^ "/" ^ saved_state_filename)

let save_state_with_errors disk_changes temp_dir expected_error : unit =
  in_daemon @@ fun () ->
  let env = setup_server () in
  let env = setup_disk env disk_changes in
  assert_env_errors env expected_error;

  let genv =
    {
      !genv with
      ServerEnv.options =
        ServerArgs.set_gen_saved_ignore_type_errors !genv.ServerEnv.options true;
    }
  in
  let _edges_added =
    ServerInit.save_state genv env (temp_dir ^ "/" ^ saved_state_filename)
  in
  ()

let load_state
    ?(master_changes = [])
    ?(local_changes = [])
    ?(load_hhi_files = false)
    ?(use_precheked_files = ServerLocalConfig.(default.prechecked_files))
    ?(load_decls_from_saved_state =
      ServerLocalConfig.(default.load_decls_from_saved_state))
    ?(enable_naming_table_fallback = false)
    ?custom_config
    ~disk_state
    saved_state_dir =
  (* In production, saved state is only used in conjunction with lazy init
   * right now, and I'm not convinced if it even works in any other
   * configuration. *)
  genv :=
    {
      !genv with
      ServerEnv.local_config =
        {
          !genv.ServerEnv.local_config with
          ServerLocalConfig.lazy_parse = true;
          lazy_init = true;
          prechecked_files = use_precheked_files;
          predeclare_ide = true;
          load_decls_from_saved_state;
          naming_sqlite_path =
            ( if enable_naming_table_fallback then
              Some (saved_state_dir ^ "/" ^ saved_naming_filename)
            else
              None );
          enable_naming_table_fallback;
        };
      ServerEnv.config =
        Option.value custom_config ~default:!genv.ServerEnv.config;
    };
  let hhi_files =
    if load_hhi_files then
      real_hhi_files ()
    else
      []
  in
  test_init_common () ~hhi_files;

  let disk_changes = List.map disk_state ~f:(fun (x, y) -> (root ^ x, y)) in
  List.iter disk_changes ~f:(fun (path, contents) -> TestDisk.set path contents);

  let prechecked_changes =
    List.map master_changes ~f:(fun x ->
        Relative_path.create_detect_prefix (root ^ x))
  in
  let changes =
    List.map local_changes ~f:(fun x ->
        Relative_path.create_detect_prefix (root ^ x))
  in
  let naming_table_path = saved_state_dir ^ "/" ^ saved_state_filename in
  let deptable_fn = saved_state_dir ^ "/" ^ saved_state_filename ^ ".sql" in
  (* TODO(hverr): Figure out 64-bit *)
  let deptable_is_64bit = false in
  let load_state_approach =
    ServerInit.Precomputed
      {
        ServerArgs.naming_table_path;
        (* in Precomputed scenario, base revision should only be used in logging,
         * which is irrelevant in tests *)
        corresponding_base_revision = "-1";
        deptable_fn;
        deptable_is_64bit;
        naming_changes = [];
        prechecked_changes;
        changes;
      }
  in
  let init_id = Random_id.short_string () in
  (* TODO(hverr): Figure out 64-bit *)
  let env =
    ServerEnvBuild.make_env
      ~init_id
      ~deps_mode:Typing_deps_mode.SQLiteMode
      !genv.ServerEnv.config
  in
  match
    ServerInit.init
      ~init_approach:(ServerInit.Saved_state_init load_state_approach)
      !genv
      env
  with
  | (env, ServerInit.Load_state_succeeded _) -> env
  | (_env, ServerInit.Load_state_declined s) ->
    Printf.eprintf "> DECLINED %s\n" s;
    assert false
  | (_env, ServerInit.Load_state_failed (s, _stack)) ->
    Printf.eprintf "> FAILED %s\n" s;
    assert false

let diagnostics_to_string x =
  let buf = Buffer.create 1024 in
  SMap.iter x ~f:(fun path errors ->
      Printf.bprintf buf "%s:\n" path;
      errors_to_string buf errors);
  Buffer.contents buf

let errors_to_string x =
  let buf = Buffer.create 1024 in
  errors_to_string buf x;
  Buffer.contents buf

let get_diagnostics loop_output =
  let diags =
    List.filter_map loop_output.push_messages ~f:(function
        | DIAGNOSTIC { errors; is_truncated = _ } -> Some errors
        | _ -> None)
  in
  match diags with
  | m :: _ -> m
  | [] -> fail "Expected at least one diagnostic message"

let assert_diagnostics loop_output expected =
  let diagnostics = get_diagnostics loop_output in
  let diagnostics_as_string = diagnostics_to_string diagnostics in
  assertEqual expected diagnostics_as_string

let assert_diagnostics_in loop_output filename expected =
  let diagnostics = get_diagnostics loop_output in
  let diagnostics =
    SMap.filter diagnostics ~f:(fun path _ ->
        String.equal path (prepend_root filename))
  in
  let diagnostics_as_string = diagnostics_to_string diagnostics in
  assertEqual expected diagnostics_as_string

let list_to_string l =
  let buf = Buffer.create 1024 in
  List.iter l ~f:(Printf.bprintf buf "%s ");
  Buffer.contents buf

let coverage_levels_to_str_helper (pos, cl) =
  let cl_str = string_of_level cl in
  let interval = Pos.string pos in
  interval ^ " " ^ cl_str

let assert_coverage_levels loop_output expected =
  let (results, counts) =
    match loop_output.persistent_client_response with
    | Some res -> res
    | _ -> fail "Expected coverage levels response"
  in
  let strings_of_stats =
    [
      "checked: " ^ string_of_int counts.checked;
      "partial: " ^ string_of_int counts.partial;
      "unchecked: " ^ string_of_int counts.unchecked;
    ]
  in
  let results_as_string =
    List.map results ~f:coverage_levels_to_str_helper
    |> List.sort ~compare:String.compare
    |> List.append strings_of_stats
    |> list_to_string
  in
  let expected_as_string = list_to_string expected in
  assertEqual expected_as_string results_as_string

let assert_autocomplete loop_output expected =
  let results =
    match loop_output.persistent_client_response with
    | Some res -> res
    | _ -> fail "Expected autocomplete response"
  in
  let results =
    results |> List.map ~f:(fun x -> x.AutocompleteTypes.res_name)
  in
  (* The autocomplete results out of hack are unsorted *)
  let results_as_string =
    results |> List.sort ~compare:String.compare |> list_to_string
  in
  let expected_as_string =
    expected |> List.sort ~compare:String.compare |> list_to_string
  in
  assertEqual expected_as_string results_as_string

let assert_autocomplete_does_not_contain loop_output not_expected =
  let results =
    match loop_output.persistent_client_response with
    | Some res -> res
    | _ -> fail "Expected autocomplete response"
  in
  let results =
    List.map results ~f:(fun x -> x.AutocompleteTypes.res_name) |> SSet.of_list
  in
  let not_expected = SSet.of_list not_expected in
  let occured = SSet.inter results not_expected in
  if SSet.is_empty occured |> not then
    Printf.sprintf
      "unexpected symbol(s) %s occurs in autocomplete list"
      (SSet.show occured)
    |> fail
  else
    ()

let assert_ide_autocomplete loop_output expected =
  let results =
    match loop_output.persistent_client_response with
    | Some res -> res
    | _ -> fail "Expected autocomplete response"
  in
  let results =
    List.map results.AutocompleteTypes.completions ~f:(fun x ->
        x.AutocompleteTypes.res_name)
  in
  let results_as_string = list_to_string results in
  let expected_as_string = list_to_string expected in
  assertEqual expected_as_string results_as_string

let smap_to_str_list (f : 'a -> string) (m : 'a SMap.t) =
  (m |> SMap.ordered_keys |> List.map) ~f:(fun s ->
      s ^ "< " ^ (SMap.find m s |> f) ^ ">")

let level_stats_to_str (ls : level_stats) =
  let lvls =
    [Ide_api_types.Checked; Ide_api_types.Partial; Ide_api_types.Unchecked]
  in
  let str_list =
    List.map lvls ~f:(fun lvl ->
        string_of_level lvl ^ "=" ^ string_of_int (CLMap.find lvl ls).count)
  in
  list_to_string str_list

let rec trie_to_string (base : string) (f : 'a -> string) (t : 'a trie) =
  match t with
  | Leaf a -> base ^ "( " ^ f a ^ ")"
  | Node (_, smap_of_tr) ->
    (*TODO: figure out why the first of the pair is duplicated*)
    List.map (SMap.ordered_keys smap_of_tr) ~f:(fun k ->
        trie_to_string (base ^ "/" ^ k) f (SMap.find smap_of_tr k))
    |> list_to_string

let assert_coverage_counts loop_output expected =
  let resOpt =
    match loop_output.persistent_client_response with
    | Some res -> res
    | None -> fail "Expected coverage count response"
  in
  let results =
    match resOpt with
    | Some res -> res
    | None -> fail "Expected some coverage count response"
  in
  let results_as_string =
    trie_to_string
      ""
      (fun x -> x |> smap_to_str_list level_stats_to_str |> list_to_string)
      results
  in
  let expected_as_string = list_to_string expected in
  assertEqual expected_as_string results_as_string

let assert_status loop_output expected =
  let { Server_status.error_list; _ } =
    match loop_output.new_client_response with
    | Some res -> res
    | _ -> fail "Expected status response"
  in
  let results_as_string = errors_to_string error_list in
  assertEqual expected results_as_string

let assert_error_count loop_output ~expected_count =
  let { Server_status.error_list; _ } =
    match loop_output.new_client_response with
    | Some res -> res
    | _ -> fail "Expected status response"
  in
  let actual_count = List.length error_list in
  Asserter.Int_asserter.assert_equals
    expected_count
    actual_count
    (Printf.sprintf
       "Expected %d errors, but got %d"
       expected_count
       actual_count)

let assert_needs_retry loop_output =
  Done_or_retry.(
    match loop_output.persistent_client_response with
    | Some Retry -> ()
    | Some (Done _) -> fail "Expected needing to retry"
    | None -> fail "Expected response")

let assert_response loop_output =
  Done_or_retry.(
    match loop_output.persistent_client_response with
    | Some (Done res) -> res
    | Some Retry -> fail "Expecteded not needing to retry"
    | None -> fail "Expected response")

let assert_find_refs loop_output expected =
  let results = assert_response loop_output in
  let results =
    List.map results ~f:(fun (name, pos) -> name ^ ": " ^ Pos.string pos)
  in
  let results_as_string = list_to_string results in
  let expected_as_string = list_to_string expected in
  assertEqual expected_as_string results_as_string

let assert_ide_find_refs loop_output expected_name expected =
  let results = assert_response loop_output in
  let results_as_string =
    match results with
    | None -> "None"
    | Some (name, results) ->
      assertEqual expected_name name;
      List.map results ~f:Pos.string |> list_to_string
  in
  let expected_as_string = list_to_string expected in
  assertEqual expected_as_string results_as_string

let assert_refactor loop_output expected =
  let results = assert_response loop_output in
  (* We don't have any (better than JSON) human-readable format for refactor results,
   * and I'm too lazy to write it. Tests will have to compare JSON outputs for now. *)
  let results_as_string = ClientRefactor.patches_to_json_string results in
  assertEqual expected results_as_string

let assert_ide_refactor loop_output expected =
  let results = assert_response loop_output in
  let results = Result.ok_or_failwith results in
  let results_as_string = ClientRefactor.patches_to_json_string results in
  assertEqual expected results_as_string

let assert_needs_recheck env x =
  if
    not
      Relative_path.(
        Set.mem env.ServerEnv.needs_recheck (create_detect_prefix (root ^ x)))
  then
    let () = Printf.eprintf "Expected %s to need recheck\n" x in
    assert false

let assert_needs_no_recheck env x =
  if
    Relative_path.(
      Set.mem env.ServerEnv.needs_recheck (create_detect_prefix (root ^ x)))
  then
    let () = Printf.eprintf "Expected %s not to need recheck\n" x in
    assert false
