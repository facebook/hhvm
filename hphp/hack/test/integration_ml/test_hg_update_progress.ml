(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *)

open Hh_prelude
module Test = Integration_test_base

let root = "/"

(** Run an iteration with no new client and expect hg waiting/working progress. *)
let assert_hg_update_progress env =
  let (env, _) = Test.run_loop_once env Test.default_loop_input in
  (* Inspect the status written by the loop. Server_progress.read would apply
     process-liveness checks that classify this isolated test runner as stopped. *)
  let progress =
    Server_files.server_progress_file (Path.make root)
    |> Sys_utils.protected_read_exn
    |> Yojson.Safe.from_string
  in
  let message = Yojson.Safe.Util.(progress |> member "message" |> to_string) in
  if not (String.equal message "hg-transaction") then
    Test.fail
      (Printf.sprintf
         "Expected progress message 'hg-transaction', got '%s'"
         message);
  let disposition = Yojson.Safe.Util.member "disposition" progress in
  if not (Yojson.Safe.equal disposition (`List [`String "DWorking"])) then
    Test.fail
      (Printf.sprintf
         "Expected working disposition, got %s"
         (Yojson.Safe.to_string disposition));
  env

(** Construct a mock client to populate the already-admitted command slot. *)
let make_client () =
  Test_client_provider.mock_new_client_type Server_command_types.Non_persistent;
  match
    Client_provider.sleep_and_check
      (Client_provider.provider_for_test ())
      ~idle_gc_slice:0
      `Any
  with
  | Client_provider.Select_new { Client_provider.client; _ } -> client
  | _ -> Test.fail "Expected the mocked client to be selected"

(** While hg.update blocks admission, progress must report hg-transaction and
    DWorking rather than ready. Cover both an empty client poll and an
    already-admitted client waiting for a full check. *)
let test () =
  let env = Test.setup_server () in
  (* Enable real progress-file writes in an isolated directory and assert hg.update
     through the tracker's Eden entry point, without starting an Eden process. *)
  let tmp = Tempfile.mkdtemp ~skip_mocking:true in
  Server_files.set_tmp_FOR_TESTING_ONLY tmp;
  Server_progress.set_root (Path.make root);
  Server_revision_tracker.Edenfs_watcher.on_state_enter Hg_states.update;
  Utils.try_finally
    ~f:(fun () ->
      (* Without a pending command, the loop polls the force-dormant pipe.
         An empty poll must not make the deferred server appear ready. *)
      let env = assert_hg_update_progress env in
      (* With an already-admitted command, polling is skipped entirely.
         This path must retain the same hg waiting/working status. *)
      let env =
        {
          env with
          Server_env.nonpersistent_client_pending_command_needs_full_check =
            Some ((fun env -> env), "test", make_client ());
        }
      in
      let (_env : Server_env.env) = assert_hg_update_progress env in
      ())
    ~finally:(fun () ->
      Server_revision_tracker.Edenfs_watcher.on_state_leave
        (Path.make root)
        Hg_states.update;
      Server_progress.try_delete ();
      Sys_utils.rm_dir_tree ~skip_mocking:true (Path.to_string tmp))
