(**
 * Copyright (c) 2016, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 *
 *)

open Core
open Integration_test_base_types
open Reordered_argument_collections
open ServerCommandTypes

let root = "/"
let server_config = ServerEnvBuild.default_genv.ServerEnv.config
let global_opts = GlobalOptions.make
  ~tco_assume_php: false
  ~tco_unsafe_xhp: false
  ~tco_user_attrs: None
  ~tco_experimental_features: GlobalOptions.tco_experimental_all
  ~po_auto_namespace_map: []

let server_config = ServerConfig.set_tc_options server_config global_opts
let server_config = ServerConfig.set_parser_options
  server_config global_opts

let genv = ref { ServerEnvBuild.default_genv with
  ServerEnv.config = server_config
}

let setup_server ?custom_config ()  =
  Printexc.record_backtrace true;
  EventLogger.init EventLogger.Event_logger_fake 0.0;
  Relative_path.set_path_prefix Relative_path.Root (Path.make root);
  HackSearchService.attach_hooks ();
  let _ = SharedMem.init GlobalConfig.default_sharedmem_config in
  match custom_config with
  | Some config -> ServerEnvBuild.make_env config
  | None -> ServerEnvBuild.make_env !genv.ServerEnv.config


let default_loop_input = {
  disk_changes = [];
  new_client = None;
  persistent_client_request = None;
}

let run_loop_once : type a b. ServerEnv.env -> (a, b) loop_inputs ->
    (ServerEnv.env * (a, b) loop_outputs) = fun env inputs ->
  TestClientProvider.clear();
  Option.iter inputs.new_client (function
  | RequestResponse x ->
    TestClientProvider.mock_new_client_type Non_persistent;
    TestClientProvider.mock_client_request x
  | ConnectPersistent ->
    TestClientProvider.mock_new_client_type Persistent);

  Option.iter inputs.persistent_client_request
    TestClientProvider.mock_persistent_client_request;

  let client_provider = ClientProvider.provider_for_test () in

  let disk_changes =
    List.map inputs.disk_changes (fun (x, y) -> root ^ x, y) in

  List.iter disk_changes
    (fun (path, contents) -> TestDisk.set path contents);

  let did_read_disk_changes_ref = ref false in

  let notifier () =
    if not !did_read_disk_changes_ref then begin
      did_read_disk_changes_ref := true;
      SSet.of_list (List.map disk_changes fst)
    end else SSet.empty
  in

  let genv = { !genv with
    ServerEnv.notifier_async =
      (fun () ->
        ServerNotifierTypes.Notifier_synchronous_changes (notifier ()));
    ServerEnv.notifier = notifier;
  } in

  let env = ServerMain.serve_one_iteration genv env client_provider in
  env, {
    did_read_disk_changes = !did_read_disk_changes_ref;
    rechecked_count =
      env.ServerEnv.recent_recheck_loop_stats.ServerEnv.rechecked_count;
    new_client_response =
      TestClientProvider.get_client_response Non_persistent;
    persistent_client_response =
      TestClientProvider.get_client_response Persistent;
    push_message = TestClientProvider.get_push_message ();
  }

let prepend_root x = root ^ x

let fail x =
  print_endline x;
  Printexc.(get_callstack 100 |> print_raw_backtrace stderr);
  exit 1

let assertEqual expected got =
  let expected = String.trim expected in
  let got = String.trim got in
  if expected <> got then fail
    (Printf.sprintf "Expected:\n%s\nGot:\n%s\n" expected got)

let setup_disk env disk_changes =
  let env, loop_output = run_loop_once env { default_loop_input with
    disk_changes
  } in
  if not loop_output.did_read_disk_changes then
    fail "Expected the server to process disk updates";
  env

let connect_persistent_client env =
  let env, _ = run_loop_once env { default_loop_input with
    new_client = Some ConnectPersistent
  } in
  match env.ServerEnv.persistent_client with
  | Some _ -> env
  | None -> fail "Expected persistent client to be connected"

let get_errors env = Errors.get_error_list env.ServerEnv.errorl

let assert_no_errors env =
  if get_errors env <> [] then fail "Expected to have no errors"

let assertSingleError expected err_list =
  match err_list with
  | [x] ->
      let error_string = Errors.(to_string (to_absolute x)) in
      assertEqual expected error_string
  | _ -> fail "Expected to have exactly one error"

let subscribe_diagnostic ?(id=4) env =
  let env, _ = run_loop_once env { default_loop_input with
    persistent_client_request = Some (
      SUBSCRIBE_DIAGNOSTIC id
    )
  } in
  if not @@ Option.is_some env.ServerEnv.diag_subscribe then
    fail "Expected to subscribe to push diagnostics";
  env

let open_file env ?contents file_name =
  let file_name = root ^ file_name in
  let contents = match contents with
    | Some contents -> contents
    | _ -> TestDisk.get file_name
  in
  let env, loop_output = run_loop_once env { default_loop_input with
    persistent_client_request = Some (OPEN_FILE (file_name, contents))
  } in
  (match loop_output.persistent_client_response with
  | Some () -> ()
  | None -> fail "Expected OPEN_FILE to be processeded");
  env

let edit_file env name contents =
  let env, loop_output = run_loop_once env { default_loop_input with
    persistent_client_request = Some (EDIT_FILE
      (root ^ name, [File_content.{range = None; text = contents;}])
    )
  } in
  (match loop_output.persistent_client_response with
  | Some () -> ()
  | None -> fail "Expected EDIT_FILE to be processeded");
  env, loop_output

let close_file env name =
  let env, loop_output = run_loop_once env { default_loop_input with
    persistent_client_request = Some (CLOSE_FILE (root ^ name))
  } in
  (match loop_output.persistent_client_response with
  | Some () -> ()
  | None -> fail "Expected CLOSE_FILE to be processeded");
  env, loop_output

let wait env =
  (* We simulate waiting one second since last command by manipulating
   * last_command_time. Will not work on timers that compare against other
   * counters. *)
  ServerEnv.{ env with last_command_time = env.last_command_time -. 1.0 }

let autocomplete env contents =
  run_loop_once env { default_loop_input with
    persistent_client_request = Some (AUTOCOMPLETE contents)
  }

let ide_autocomplete env (path, line, column) =
  run_loop_once env { default_loop_input with
    persistent_client_request = Some (IDE_AUTOCOMPLETE
      (root ^ path, File_content.{line; column})
    )
  }

let status env =
  run_loop_once env { default_loop_input with
    new_client = Some (RequestResponse ServerCommandTypes.STATUS)
  }

let assert_no_diagnostics loop_output =
  match loop_output.push_message with
  | Some (DIAGNOSTIC _) ->
    fail "Did not expect to receive push diagnostics."
  | None -> ()

let assert_has_diagnostics loop_output =
  match loop_output.push_message with
  | Some (DIAGNOSTIC _) -> ()
  | None -> fail "Expected to receive push diagnostics."

let errors_to_string buf x =
  List.iter x ~f: begin fun error ->
    Printf.bprintf buf "%s\n" (Errors.to_string error)
  end

let diagnostics_to_string x =
  let buf = Buffer.create 1024 in
  SMap.iter x ~f:begin fun path errors ->
    Printf.bprintf buf "%s:\n" path;
    errors_to_string buf errors;
  end;
  Buffer.contents buf

let assert_diagnostics loop_output expected =
  let diagnostics = match loop_output.push_message with
    | None -> fail "Expected push diagnostics"
    | Some (DIAGNOSTIC (_, m)) -> m
  in

  let diagnostics_as_string = diagnostics_to_string diagnostics in
  assertEqual expected diagnostics_as_string

let list_to_string l =
  let buf = Buffer.create 1024 in
  List.iter l ~f:(Printf.bprintf buf "%s ");
  Buffer.contents buf

let assert_autocomplete loop_output expected =
  let results = match loop_output.persistent_client_response with
    | Some res -> res
    | _ -> fail "Expected autocomplete response"
  in
  let results = List.map results ~f:(fun x -> x.AutocompleteService.res_name) in
  let results_as_string = list_to_string results in
  let expected_as_string = list_to_string expected in
  assertEqual expected_as_string results_as_string
