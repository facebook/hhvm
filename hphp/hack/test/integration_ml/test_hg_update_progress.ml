(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *)

open Hh_prelude
module Test = Integration_test_base

let root = "/"

(** Construct a mock client to populate the already-admitted command slot. *)
let make_client () =
  Test_client_provider.mock_new_client_type Server_command_types.Non_persistent;
  match
    Client_provider.sleep_and_check
      (Client_provider.provider_for_test ())
      ~idle_gc_slice:0
      `Any
  with
  | Client_provider.Select_new { Client_provider.client; _ } ->
    Test_client_provider.clear ();
    client
  | _ -> Test.fail "Expected the mocked client to be selected"

(** Even with an admitted client waiting for a full check, an asserted hg.update
    must report waiting/working progress rather than ready. *)
let test () =
  let env = Test.setup_server () in
  (* Configure an asserted hg.update without file changes or a real Eden process. *)
  let default = Server_local_config_load.default in
  let local_config =
    {
      default with
      Server_local_config.hg_aware = true;
      block_client_connections_while_deferring = true;
      edenfs_file_watcher =
        {
          default.Server_local_config.edenfs_file_watcher with
          Server_local_config.EdenfsFileWatcher.enabled = true;
          state_tracking = true;
          tracked_states = [Hg_states.update; Hg_states.transaction];
        };
    }
  in
  Edenfs_watcher.Mocking.get_changes_async_returns [];
  Edenfs_watcher.Mocking.get_asserted_states_returns (Ok [Hg_states.update]);
  let (notifier, _indexer) =
    Server_notifier.init
      (Server_args.default_options ~root)
      local_config
      ~num_workers:0
  in
  let genv =
    { Server_env_build.default_genv with Server_env.local_config; notifier }
  in
  (* Populate the already-admitted client slot, leaving no new client to select. *)
  let env =
    {
      env with
      Server_env.nonpersistent_client_pending_command_needs_full_check =
        Some ((fun env -> env), "test", make_client ());
    }
  in
  (* Enable real progress-file writes in an isolated directory. *)
  let tmp = Tempfile.mkdtemp ~skip_mocking:true in
  Server_files.set_tmp_FOR_TESTING_ONLY tmp;
  Server_progress.set_root (Path.make root);
  Utils.try_finally
    ~f:(fun () ->
      (* With an already-admitted command, polling is skipped entirely.
         This path must still report hg waiting/working status. *)
      let (_env : Server_env.env) =
        Server_main.serve_one_iteration
          genv
          env
          (Client_provider.provider_for_test ())
      in
      (* Inspect the status written by the loop. Server_progress.read would apply
         process-liveness checks that classify this isolated test runner as stopped. *)
      let progress =
        Server_files.server_progress_file (Path.make root)
        |> Sys_utils.protected_read_exn
        |> Yojson.Safe.from_string
      in
      Test.assertEqual
        "waiting for hg.update"
        Yojson.Safe.Util.(progress |> member "message" |> to_string);
      Test.assertEqual
        "[\"DWorking\"]"
        (Yojson.Safe.Util.member "disposition" progress |> Yojson.Safe.to_string))
    ~finally:(fun () ->
      Server_progress.try_delete ();
      Sys_utils.rm_dir_tree ~skip_mocking:true (Path.to_string tmp))
