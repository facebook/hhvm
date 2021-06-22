open Hh_prelude
module Int_asserter = Asserter.Int_asserter

module type Mock_server_config_sig = sig
  include ServerMonitorUtils.Server_config with type server_start_options = unit

  val get_start_server_count : unit -> int

  val get_kill_server_count : unit -> int
end

module Tools = struct
  include Informant_test_tools

  let monitor_config temp_dir =
    {
      ServerMonitorUtils.socket_file =
        Path.to_string (Path.concat temp_dir "test_server.monitor_socket");
      lock_file =
        Path.to_string (Path.concat temp_dir "test_server.monitor_lock");
      server_log_file = Path.to_string (Path.concat temp_dir "test_server.log");
      monitor_log_file =
        Path.to_string (Path.concat temp_dir "test_server.monitor_log");
    }

  let simple_informant_options temp_dir =
    {
      HhMonitorInformant.root = temp_dir;
      allow_subscriptions = true;
      use_dummy = false;
      watchman_debug_logging = false;
      min_distance_restart = 100;
      ignore_hh_version = false;
      is_saved_state_precomputed = false;
    }
end

(** To test what actions a Monitor takes, we make a Mock for the Server_config
 * and inspect what gets called on it. We make it as a first class module that gets
 * passed into the unit test so it is fresh for each test. *)
let make_test test =
  let mock_server_config =
    ( module struct
      type server_start_options = unit

      let null_fd = Unix.openfile Sys_utils.null_path [Unix.O_RDWR] 0o666

      let fake_process_data =
        {
          ServerProcess.pid = 0;
          server_specific_files =
            {
              ServerCommandTypes.server_finale_file = "";
              server_progress_file = "";
            };
          start_t = 0.0;
          in_fd = null_fd;
          out_fds = [];
          last_request_handoff = ref 0.0;
        }

      let start_server_count = ref 0

      let start_server ~informant_managed:_ ~prior_exit_status:_ _start_options
          =
        start_server_count := !start_server_count + 1;
        fake_process_data

      let get_start_server_count () = !start_server_count

      let kill_server_count = ref 0

      let kill_server _ = kill_server_count := !kill_server_count + 1

      let get_kill_server_count () = !kill_server_count

      let wait_for_server_exit _ _ = ()

      let wait_pid _ = (0, Unix.WEXITED 0)

      let is_saved_state_precomputed _ = false
    end : Mock_server_config_sig )
  in
  Tools.set_hg_to_global_rev_map ();
  fun () ->
    Tempfile.with_tempdir (fun temp_dir ->
        (try test mock_server_config temp_dir with e -> raise e))

let test_no_event mock_server_config temp_dir =
  let module Mock_server_config = ( val mock_server_config
                                      : Mock_server_config_sig )
  in
  let module Test_monitor =
    ServerMonitor.Make_monitor (Mock_server_config) (HhMonitorInformant)
  in
  Int_asserter.assert_equals
    0
    (Mock_server_config.get_start_server_count ())
    "Before starting monitor, start_server should not have been called.";
  let monitor =
    Test_monitor.start_monitor
      ~current_version:(Config_file.Opaque_version None)
      ~waiting_client:None
      ~max_purgatory_clients:10
      ~monitor_fd_close_delay:(-1)
      ~monitor_backpressure:true
      ()
      (Tools.simple_informant_options temp_dir)
      (Tools.monitor_config temp_dir)
  in
  ignore monitor;
  Int_asserter.assert_equals
    1
    (Mock_server_config.get_start_server_count ())
    "Start server should have been called";
  true

let test_restart_server_with_target_saved_state mock_server_config temp_dir =
  let module Mock_server_config = ( val mock_server_config
                                      : Mock_server_config_sig )
  in
  let module Test_monitor =
    ServerMonitor.Make_monitor (Mock_server_config) (HhMonitorInformant)
  in
  Watchman.Mocking.init_returns @@ Some "Fake name for watchman instance";
  Int_asserter.assert_equals
    0
    (Mock_server_config.get_start_server_count ())
    "Before starting monitor, start_server should not have been called.";
  Hg.Mocking.current_working_copy_base_rev_returns
    (Future.of_value Tools.global_rev_1);
  let monitor =
    Test_monitor.start_monitor
      ~current_version:(Config_file.Opaque_version None)
      ~waiting_client:None
      ~max_purgatory_clients:10
      ~monitor_fd_close_delay:(-1)
      ~monitor_backpressure:true
      ()
      (Tools.simple_informant_options temp_dir)
      (Tools.monitor_config temp_dir)
  in
  let monitor = Test_monitor.check_and_run_loop_once monitor in
  Int_asserter.assert_equals
    1
    (Mock_server_config.get_start_server_count ())
    "First call of start server";
  Tools.set_xdb
    ~state_global_rev:200
    ~for_global_rev:200
    ~everstore_handle:"dummy_handle_for_global_200";
  Tools.set_next_watchman_state_transition
    Tools.Changed_merge_base
    Tools.hg_rev_200;
  let monitor = Test_monitor.check_and_run_loop_once monitor in
  ignore monitor;
  Int_asserter.assert_equals
    2
    (Mock_server_config.get_start_server_count ())
    "Should be starting fresh server";
  true

let test_server_restart_suppressed_on_hhconfig_version_change
    mock_server_config temp_dir =
  (* The first part of this is mostly copy-pasta.
   * Can't reuse code because of First Class Modules :( *)
  (* ---- Start of copy-pasta from test_restart_server_with_target_saved_state -------- *)
  (* ------------------------ This sets up an Informant-directed restart -------------- *)
  let module Mock_server_config = ( val mock_server_config
                                      : Mock_server_config_sig )
  in
  let module Test_monitor =
    ServerMonitor.Make_monitor (Mock_server_config) (HhMonitorInformant)
  in
  Watchman.Mocking.init_returns @@ Some "Fake name for watchman instance";
  Int_asserter.assert_equals
    0
    (Mock_server_config.get_start_server_count ())
    "Before starting monitor, start_server should not have been called.";
  Hg.Mocking.current_working_copy_base_rev_returns
    (Future.of_value Tools.global_rev_1);
  let monitor =
    Test_monitor.start_monitor
    (* ------ Except we want to specify a version ------- *)
      ~current_version:(Config_file.Opaque_version (Some "aaa"))
      ~waiting_client:None
      ~max_purgatory_clients:10
      ~monitor_fd_close_delay:(-1)
      ~monitor_backpressure:true
      ()
      (Tools.simple_informant_options temp_dir)
      (Tools.monitor_config temp_dir)
  in
  let monitor = Test_monitor.check_and_run_loop_once monitor in
  Int_asserter.assert_equals
    1
    (Mock_server_config.get_start_server_count ())
    "First call of start server";

  (* ----------------------------- End of copy-pasta ---------------------------- *)
  let start_server_count = Mock_server_config.get_start_server_count () in
  Int_asserter.assert_equals
    1
    start_server_count
    "Start server called once to start the first server.";

  (* Next we set up next check_and_run_loop to trigger an Informant-directed restart *)
  Tools.set_xdb
    ~state_global_rev:200
    ~for_global_rev:200
    ~everstore_handle:"dummy_handle_for_global_200";
  Tools.set_next_watchman_state_transition
    Tools.Changed_merge_base
    Tools.hg_rev_200;

  (* ...except we want version to mismatch when we look it up *)
  Sys_utils.write_file
    ~file:
      (Relative_path.to_absolute
         (Relative_path.create Relative_path.Root "/tmp/.hhconfig"))
    "version = def";
  let monitor = Test_monitor.check_and_run_loop_once monitor in
  ignore monitor;
  let kill_server_count = Mock_server_config.get_kill_server_count () in
  let start_server_count = Mock_server_config.get_start_server_count () in
  Int_asserter.assert_equals
    1
    start_server_count
    "Informant tried to do a restart with the better saved state, but it should not go through.";
  Int_asserter.assert_equals
    1
    kill_server_count
    "Kill should still be called, even though the next server instance isn't started.";

  (* Pump the monitor's run loop again and ensure server stil hasn't tbeen started. *)
  let monitor = Test_monitor.check_and_run_loop_once monitor in
  ignore monitor;
  let start_server_count = Mock_server_config.get_start_server_count () in
  Int_asserter.assert_equals
    1
    start_server_count
    "Start server still shouldn't be called";

  (* Testing for a server actually being restarted on the next client connection
   * happens in the integration tests. *)
  true

let setup_global_test_state () =
  EventLogger.init_fake ();
  Relative_path.(set_path_prefix Root (Path.make "/tmp"));
  Relative_path.(set_path_prefix Root (Path.make Sys_utils.temp_dir_name));
  let hhconfig_path = Relative_path.(create Root "/tmp/.hhconfig") in
  Disk.write_file
    ~file:(Relative_path.to_absolute hhconfig_path)
    ~contents:"assume_php = false"

let tests =
  [
    ("test_no_event", make_test test_no_event);
    ( "test_restart_server_with_target_saved_state",
      make_test test_restart_server_with_target_saved_state );
    ( "test_server_restart_suppressed_on_hhconfig_version_change",
      make_test test_server_restart_suppressed_on_hhconfig_version_change );
  ]

let () =
  setup_global_test_state ();

  let tests = List.concat [tests; Informant_test.tests] in
  Unit_test.run_all tests
