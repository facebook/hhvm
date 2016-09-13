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
open ServerCommandTypes

let root = "/"
let stats = ServerMain.empty_stats ()
let genv = ref ServerEnvBuild.default_genv

let setup_server () =
  Printexc.record_backtrace true;
  EventLogger.init (Daemon.devnull ()) 0.0;
  Relative_path.set_path_prefix Relative_path.Root (Path.make root);
  HackSearchService.attach_hooks ();
  let _ = SharedMem.init GlobalConfig.default_sharedmem_config in
  ServerEnvBuild.make_env !genv.ServerEnv.config

let default_loop_input = {
  disk_changes = [];
  new_client = None;
  persistent_client_request = None;
}

let run_loop_once env inputs =
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

  let genv = { !genv with
    ServerEnv.notifier = begin fun () ->
      if not !did_read_disk_changes_ref then begin
        did_read_disk_changes_ref := true;
        SSet.of_list (List.map disk_changes fst)
      end else SSet.empty
    end
  } in

  let env = ServerMain.serve_one_iteration genv env client_provider stats in
  env, {
    did_read_disk_changes = !did_read_disk_changes_ref;
    rechecked_count = ServerMain.get_rechecked_count stats;
    new_client_response =
      TestClientProvider.get_client_response Non_persistent;
    persistent_client_response =
      TestClientProvider.get_client_response Persistent;
    push_message = TestClientProvider.get_push_message ();
  }

let prepend_root x = root ^ x

let fail x =
  print_endline x;
  exit 1

let assertEqual expected got =
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

let assertSingleError expected err_list =
  match err_list with
  | [x] ->
      let error_string = Errors.(to_string (to_absolute x)) in
      assertEqual expected error_string
  | _ -> fail "Expected to have exactly one error"
