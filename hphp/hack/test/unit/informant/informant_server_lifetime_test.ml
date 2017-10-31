module Target_mini_state_comparator = struct
  type t = ServerMonitorUtils.target_mini_state
  let to_string { ServerMonitorUtils.mini_state_everstore_handle; target_svn_rev; } =
    Printf.sprintf "(Target mini state. everstore handle: %s. svn_rev: %d)"
      mini_state_everstore_handle target_svn_rev

  let is_equal x y = x = y
end;;


module Target_mini_state_opt_comparator =
  Asserter.Make_option_comparator (Target_mini_state_comparator);;


module Start_server_args_comparator = struct

  (** We only care about this arg and drop the rest. *)
  type t = ServerMonitorUtils.target_mini_state option

  let to_string state =
    let state_string = Target_mini_state_opt_comparator.to_string state in
    Printf.sprintf "(Start server call args: %s)"
      state_string

  let is_equal x y =
    Target_mini_state_opt_comparator.is_equal x y

end;;


module Start_server_args_opt_comparator =
  Asserter.Make_option_comparator (Start_server_args_comparator);;


module Start_server_args_opt_asserter = Asserter.Make_asserter
  (Start_server_args_opt_comparator);;


module type Mock_server_config_sig = sig
  include ServerMonitorUtils.Server_config with type server_start_options = unit
  val get_last_start_server_call : unit -> Start_server_args_comparator.t option
end;;


module Tools = struct
  include Informant_test_tools

  let monitor_config temp_dir = {
    ServerMonitorUtils.socket_file =
      Path.to_string (Path.concat temp_dir "test_server.monitor_socket");
    lock_file =
      Path.to_string (Path.concat temp_dir "test_server.monitor_lock");
    server_log_file =
      Path.to_string (Path.concat temp_dir "test_server.log");
    monitor_log_file =
      Path.to_string (Path.concat temp_dir "test_server.monitor_log");
    load_script_log_file =
      Path.to_string (Path.concat temp_dir "test_load_script.log");
  }

  let simple_informant_options temp_dir = {
    HhMonitorInformant.root = temp_dir;
    allow_subscriptions = true;
    state_prefetcher = State_prefetcher.dummy;
    use_dummy = false;
    min_distance_restart = 100;
    use_xdb = true;
  }
end;;


(** To test what actions a Monitor takes, we make a Mock for the Server_config
 * and inspect what gets called on it. We make it as a first class module that gets
 * passed into the unit test so it is fresh for each test. *)
let make_test test =
  let mock_server_config = (module struct

    type server_start_options = unit

    let null_fd = Unix.openfile Sys_utils.null_path [Unix.O_RDWR] 0o666

    let fake_process_data = {
      ServerProcess.pid = 0;
      name = "MockServerConfigProcessData";
      start_t = 0.0;
      in_fd = null_fd;
      out_fd = null_fd;
      last_request_handoff = ref 0.0;
    }

    let last_start_server_call = ref None

    let start_server ?(target_mini_state:_) ~informant_managed:_ ~prior_exit_status:_ start_options =
      last_start_server_call := (Some target_mini_state);
      fake_process_data

    let get_last_start_server_call () = !last_start_server_call

    let on_server_exit : ServerMonitorUtils.monitor_config -> unit = fun _ -> ()

    let kill_server _ = ()

    let wait_for_server_exit _ _ = ()

    let wait_pid _ = 0, (Unix.WEXITED 0)

  end : Mock_server_config_sig) in
  Xdb.Mocking.reset_find_nearest ();
  Tools.set_hg_to_svn_map ();
  fun () ->
    Tempfile.with_tempdir begin fun temp_dir ->
      try test mock_server_config temp_dir with
      | e ->
        raise e
    end

let test_no_event mock_server_config temp_dir =
  let module Mock_server_config = (val mock_server_config : Mock_server_config_sig) in
  let module Test_monitor = ServerMonitor.Make_monitor (Mock_server_config) (HhMonitorInformant) in
  let last_call = Mock_server_config.get_last_start_server_call () in
  let expected = None in
  Start_server_args_opt_asserter.assert_equals expected last_call
    "Before starting monitor, start_server should not have been called.";
  let monitor = Test_monitor.start_monitor
    ~waiting_client:None
    ~max_purgatory_clients:10
    ()
    (Tools.simple_informant_options temp_dir)
    (Tools.monitor_config temp_dir)
  in
  ignore monitor;
  let last_call = Mock_server_config.get_last_start_server_call () in
  let expected = Some None in
  Start_server_args_opt_asserter.assert_equals expected last_call
    "Start server should have been called, but with None for target_saved_state";
  true

let test_restart_server_with_target_saved_state mock_server_config temp_dir =
  let module Mock_server_config = (val mock_server_config : Mock_server_config_sig) in
  let module Test_monitor = ServerMonitor.Make_monitor (Mock_server_config) (HhMonitorInformant) in
  Watchman.Mocking.init_returns @@ Some "Fake name for watchman instance";
  let last_call = Mock_server_config.get_last_start_server_call () in
  let expected = None in
  Start_server_args_opt_asserter.assert_equals expected last_call
    "Before starting monitor, start_server should not have been called.";
  Hg.Mocking.current_working_copy_base_rev_returns
    (Future.of_value Tools.svn_1);
  let monitor = Test_monitor.start_monitor
    ~waiting_client:None
    ~max_purgatory_clients:10
    ()
    (Tools.simple_informant_options temp_dir)
    (Tools.monitor_config temp_dir)
  in
  let monitor = Test_monitor.check_and_run_loop_once monitor in
  let last_call = Mock_server_config.get_last_start_server_call () in
  let expected = Some None in
  Start_server_args_opt_asserter.assert_equals expected last_call
    "First call of start server should have no target saved state";
  Tools.set_xdb ~state_svn_rev:200
    ~for_svn_rev:200 ~everstore_handle:"dummy_handle_for_svn_200" ~tiny:false;
  Tools.set_next_watchman_state_transition Tools.Changed_merge_base Tools.hg_rev_200;
  let monitor = Test_monitor.check_and_run_loop_once monitor in
  ignore monitor;
  let last_call = Mock_server_config.get_last_start_server_call () in
  let state_target = {
    ServerMonitorUtils.mini_state_everstore_handle = "dummy_handle_for_svn_200";
    target_svn_rev = 200;
  } in
  let expected = Some (Some state_target) in
  Start_server_args_opt_asserter.assert_equals expected last_call
    "Should be starting fresh server with target saved state after significant Changed_merge_base";
  true


let setup_global_test_state () =
  EventLogger.init EventLogger.Event_logger_fake 0.0;
  Relative_path.(set_path_prefix Root (Path.make "/tmp"));
  Relative_path.(set_path_prefix Root (Path.make Sys_utils.temp_dir_name));
  let hhconfig_path = Relative_path.(create Root "/tmp/.hhconfig") in
  Disk.write_file (Relative_path.to_absolute hhconfig_path) "assume_php = false"

let tests =
  [
    "test_no_event",
      make_test test_no_event;
    "test_restart_server_with_target_saved_state",
      make_test test_restart_server_with_target_saved_state;
  ]

let () =
  setup_global_test_state ();
  Unit_test.run_all tests
