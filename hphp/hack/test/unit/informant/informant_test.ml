open Hh_prelude

module Report_comparator :
  Asserter.Comparator with type t = Informant_sig.report = struct
  open Informant_sig

  type t = Informant_sig.report

  let to_string v =
    match v with
    | Move_along -> "Move_along"
    | Restart_server state ->
      let str =
        match state with
        | None -> "None"
        | Some
            {
              ServerMonitorUtils.saved_state_everstore_handle;
              target_global_rev;
              watchman_mergebase;
              _;
            } ->
          Printf.sprintf
            "(Some %s %d %s)"
            saved_state_everstore_handle
            target_global_rev
            (Option.value_map
               watchman_mergebase
               ~default:"None"
               ~f:ServerMonitorUtils.watchman_mergebase_to_string)
      in
      Printf.sprintf "Restart_server %s" str

  let is_equal exp actual = String.equal (to_string exp) (to_string actual)
end

module Report_asserter = Asserter.Make_asserter (Report_comparator)
module WEWClient = WatchmanEventWatcherClient
module WEWConfig = WatchmanEventWatcherConfig

module Tools = struct
  include Informant_test_tools

  (** Test the given transition to an hg_rev and assert the expected report. *)
  let test_transition
      informant transition hg_rev server_status expected_report assert_msg =
    set_next_watchman_state_transition transition hg_rev;
    let report = HhMonitorInformant.report informant server_status in
    Report_asserter.assert_equals expected_report report assert_msg
end

(** Create XDB table entries for rev 5 and 200. Start Informant. *)
let basic_setup_rev_5_and_200_and_start_informant temp_dir =
  Tools.set_hg_to_global_rev_map ();

  (* In XDB table, add an entry for global rev 200. *)
  Tools.set_xdb
    ~state_global_rev:200
    ~for_global_rev:200
    ~everstore_handle:"dummy_handle_for_global_200";
  Tools.set_xdb
    ~state_global_rev:5
    ~for_global_rev:5
    ~everstore_handle:"dummy_handle_for_global_5";
  Watchman.Mocking.init_returns @@ Some "test_mock_basic";
  Hg.Mocking.current_working_copy_base_rev_returns
    (Future.of_value Tools.global_rev_1);
  Watchman.Mocking.get_changes_returns
    (Watchman.Watchman_pushed (Watchman.Files_changed SSet.empty));
  let informant =
    HhMonitorInformant.init
      {
        HhMonitorInformant.root = temp_dir;
        allow_subscriptions = true;
        min_distance_restart = 100;
        use_dummy = false;
        watchman_debug_logging = false;
        use_xdb = true;
        saved_state_cache_limit = 20;
        ignore_hh_version = false;
        ignore_hhconfig = false;
        is_saved_state_precomputed = false;
      }
  in
  let report = HhMonitorInformant.report informant Informant_sig.Server_alive in
  Report_asserter.assert_equals
    Informant_sig.Move_along
    report
    "no distance moved";
  informant

(** When base revision has changed significantly, informant asks
 * for server restart. Also, ensures that there are entries in XDB
 * table for those revisions. *)
let test_informant_restarts_significant_move temp_dir =
  let informant = basic_setup_rev_5_and_200_and_start_informant temp_dir in

  (**** Following tests all have a State_enter followed by a State_leave
   * and then a Changed_merge_base. *)

  (* Move base revisions insignificant distance away. *)
  Tools.test_transition
    informant
    Tools.State_enter
    Tools.hg_rev_5
    Informant_sig.Server_alive
    Informant_sig.Move_along
    "state enter insignificant distance";
  Tools.test_transition
    informant
    Tools.State_leave
    Tools.hg_rev_5
    Informant_sig.Server_alive
    Informant_sig.Move_along
    "state leave insignificant distance";

  (* Move significant distance. *)
  Tools.test_transition
    informant
    Tools.State_enter
    Tools.hg_rev_200
    Informant_sig.Server_alive
    Informant_sig.Move_along
    "state enter significant distance";
  Tools.test_transition
    informant
    Tools.State_leave
    Tools.hg_rev_200
    Informant_sig.Server_alive
    Informant_sig.Move_along
    "state leave significant distance";
  let expected_mergebase =
    {
      ServerMonitorUtils.mergebase_global_rev = 200;
      files_changed = SSet.empty;
      watchman_clock = "dummy_clock";
    }
  in
  let expected_state_target =
    {
      ServerMonitorUtils.saved_state_everstore_handle =
        "dummy_handle_for_global_200";
      target_global_rev = 200;
      watchman_mergebase = Some expected_mergebase;
    }
  in
  Tools.test_transition
    informant
    Tools.Changed_merge_base
    Tools.hg_rev_200
    Informant_sig.Server_alive
    (Informant_sig.Restart_server (Some expected_state_target))
    "Move forward significant distance";

  (* Informant now sitting at revision 200. Moving to 230 no restart. *)
  Tools.test_transition
    informant
    Tools.State_enter
    Tools.hg_rev_230
    Informant_sig.Server_alive
    Informant_sig.Move_along
    "state enter insignificant distance";
  Tools.test_transition
    informant
    Tools.State_leave
    Tools.hg_rev_230
    Informant_sig.Server_alive
    Informant_sig.Move_along
    "state leave insignificant distance";
  Tools.test_transition
    informant
    Tools.Changed_merge_base
    Tools.hg_rev_230
    Informant_sig.Server_alive
    Informant_sig.Move_along
    "Changed merge base insignificant distance";

  (* Moving back to 200 no restart. *)
  Tools.test_transition
    informant
    Tools.State_enter
    Tools.hg_rev_200
    Informant_sig.Server_alive
    Informant_sig.Move_along
    "state enter insignificant distance";
  Tools.test_transition
    informant
    Tools.State_leave
    Tools.hg_rev_200
    Informant_sig.Server_alive
    Informant_sig.Move_along
    "state leave insignificant distance";
  Tools.test_transition
    informant
    Tools.Changed_merge_base
    Tools.hg_rev_200
    Informant_sig.Server_alive
    Informant_sig.Move_along
    "Changed_merge_base insignificant distance";

  (* Moving back to SVN rev 5 (hg_rev_5) restarts. *)
  Tools.test_transition
    informant
    Tools.State_enter
    Tools.hg_rev_5
    Informant_sig.Server_alive
    Informant_sig.Move_along
    "state enter significant distance";
  Tools.test_transition
    informant
    Tools.State_leave
    Tools.hg_rev_5
    Informant_sig.Server_alive
    Informant_sig.Move_along
    "state leave significant distance";
  let expected_mergebase =
    {
      ServerMonitorUtils.mergebase_global_rev = 5;
      files_changed = SSet.empty;
      watchman_clock = "dummy_clock";
    }
  in
  let expected_state_target =
    {
      ServerMonitorUtils.saved_state_everstore_handle =
        "dummy_handle_for_global_5";
      target_global_rev = 5;
      watchman_mergebase = Some expected_mergebase;
    }
  in
  Tools.test_transition
    informant
    Tools.Changed_merge_base
    Tools.hg_rev_5
    Informant_sig.Server_alive
    (Informant_sig.Restart_server (Some expected_state_target))
    "Move back significant distance";
  true

(**
 * Like the above test, but tests a lot less.
 *
 * Informant starts, and we jump to a local commit that is hg_rev_200 plus
 * some files changed. This triggers an informant-directed restart which
 * also has rev 200 as the mergebase, plus the files changed list
 * threaded through.
 *)
let test_informant_restarts_significant_move_with_local_changes temp_dir =
  let informant = basic_setup_rev_5_and_200_and_start_informant temp_dir in
  (**** Following tests all have a State_enter followed by a State_leave
   * and then a Changed_merge_base. *)
  let expected_mergebase =
    {
      ServerMonitorUtils.mergebase_global_rev = 200;
      files_changed = SSet.singleton "local_file.php";
      watchman_clock = "dummy_clock";
    }
  in
  let expected_state_target =
    {
      ServerMonitorUtils.saved_state_everstore_handle =
        "dummy_handle_for_global_200";
      target_global_rev = 200;
      watchman_mergebase = Some expected_mergebase;
    }
  in
  Tools.test_transition
    informant
    (Tools.Changed_merge_base_plus_files (SSet.singleton "local_file.php"))
    Tools.hg_rev_200_plus_local
    Informant_sig.Server_alive
    (Informant_sig.Restart_server (Some expected_state_target))
    "Move forward significant distance";
  true

(** This test is similar to the above (but shorter) except the
 * query for the SVN revision for the Changed_merge_base revision is
 * delayed. *)
let test_informant_restarts_significant_move_delayed temp_dir =
  (* We delay it by 8. It seems an odd number, but the state
   * transition queue can get pumped multiple times at each
   * Informant.report call. This occurs because Revision_tracker.report
   * calls itself recursively if something arrived on the Watchman
   * subscription. *)
  Tools.set_hg_to_global_rev_map ~delay_rev_200:8 ();
  Watchman.Mocking.init_returns @@ Some "test_mock_basic";
  Hg.Mocking.current_working_copy_base_rev_returns
    (Future.of_value Tools.global_rev_1);
  Watchman.Mocking.get_changes_returns
    (Watchman.Watchman_pushed (Watchman.Files_changed SSet.empty));
  let informant =
    HhMonitorInformant.init
      {
        HhMonitorInformant.root = temp_dir;
        allow_subscriptions = true;
        min_distance_restart = 100;
        use_dummy = false;
        watchman_debug_logging = false;
        use_xdb = false;
        saved_state_cache_limit = 20;
        ignore_hh_version = false;
        ignore_hhconfig = false;
        is_saved_state_precomputed = false;
      }
  in
  let report = HhMonitorInformant.report informant Informant_sig.Server_alive in
  Report_asserter.assert_equals
    Informant_sig.Move_along
    report
    "no distance moved";

  (* Start with a significant Changed Merge Base to a revision that's mapped
   * SVN revision is delayed by a few cycles. *)
  Tools.test_transition
    informant
    Tools.Changed_merge_base
    Tools.hg_rev_200
    Informant_sig.Server_alive
    Informant_sig.Move_along
    "Changed_merge_base significant distance, but delayed should not restart";

  (* Informant now sitting at revision 200, but informant doesn't know it yet
   * because of the delayed computation. we nudge it twice. *)
  Tools.test_transition
    informant
    Tools.State_enter
    Tools.hg_rev_230
    Informant_sig.Server_alive
    Informant_sig.Move_along
    "Nudge delayed future with State_enter";
  Tools.test_transition
    informant
    Tools.State_leave
    Tools.hg_rev_230
    Informant_sig.Server_alive
    Informant_sig.Move_along
    "Nudge delayed future with State_leave";

  (* Now we come back to 200, but this last nudge of the delayed value produces
   * the actual SVN rev for 200. So the Restart_server from the
   * Changed_merge_base above shows up now. *)
  Tools.test_transition
    informant
    Tools.State_enter
    Tools.hg_rev_200
    Informant_sig.Server_alive
    (Informant_sig.Restart_server None)
    "Trigger last delayed value for prior Changed_merge_base global rev mapping";
  true

(** This test is similar to the above (but shorter) except the
 * we are going to global_230 which has no entry in the XDB table,
 * and thus we get no restart report.
 *)
let test_informant_no_saved_state_no_restart temp_dir =
  Tools.set_hg_to_global_rev_map ();
  Watchman.Mocking.init_returns @@ Some "test_mock_basic";
  Hg.Mocking.current_working_copy_base_rev_returns
    (Future.of_value Tools.global_rev_1);
  Watchman.Mocking.get_changes_returns
    (Watchman.Watchman_pushed (Watchman.Files_changed SSet.empty));
  let informant =
    HhMonitorInformant.init
      {
        HhMonitorInformant.root = temp_dir;
        allow_subscriptions = true;
        min_distance_restart = 100;
        use_dummy = false;
        watchman_debug_logging = false;
        use_xdb = true;
        saved_state_cache_limit = 20;
        ignore_hh_version = false;
        ignore_hhconfig = false;
        is_saved_state_precomputed = false;
      }
  in
  let report = HhMonitorInformant.report informant Informant_sig.Server_alive in
  Report_asserter.assert_equals
    Informant_sig.Move_along
    report
    "no distance moved";

  (* There's no saved state for rev 4, so we don't restart. *)
  Tools.test_transition
    informant
    Tools.Changed_merge_base
    Tools.hg_rev_230
    Informant_sig.Server_alive
    Informant_sig.Move_along
    "Significantly changed merge base, but no saved state for this rev";
  true

(** The found saved state in the XDB table is too far, so we should
 * just prefer letting incremental run its course.
 *
 * We start Informant at rev 5, transition to rev 200, but only find
 * a saved state for rev 1. So we prefer incremental (Move_along) instead
 * of loading a new saved state (Restart_server). *)
let test_informant_xdb_saved_state_too_far temp_dir =
  Tools.set_hg_to_global_rev_map ();
  Watchman.Mocking.init_returns @@ Some "test_mock_basic";
  Hg.Mocking.current_working_copy_base_rev_returns
    (Future.of_value Tools.global_rev_5);
  Watchman.Mocking.get_changes_returns
    (Watchman.Watchman_pushed (Watchman.Files_changed SSet.empty));
  let informant =
    HhMonitorInformant.init
      {
        HhMonitorInformant.root = temp_dir;
        allow_subscriptions = true;
        min_distance_restart = 100;
        use_dummy = false;
        watchman_debug_logging = false;
        use_xdb = true;
        saved_state_cache_limit = 20;
        ignore_hh_version = false;
        ignore_hhconfig = false;
        is_saved_state_precomputed = false;
      }
  in
  let report = HhMonitorInformant.report informant Informant_sig.Server_alive in
  Report_asserter.assert_equals
    Informant_sig.Move_along
    report
    "no distance moved";

  (* At rev 200, we will find a saved state made for rev 1. *)
  Tools.set_xdb
    ~state_global_rev:1
    ~for_global_rev:200
    ~everstore_handle:"Fake everstore handle for global rev 1";
  Tools.test_transition
    informant
    Tools.Changed_merge_base
    Tools.hg_rev_200
    Informant_sig.Server_alive
    Informant_sig.Move_along
    "Significantly changed merge base, saved state for this rev is too distant";
  true

(** We emulate the repo being in a mid-update state when the informant
 * starts. i.e., the .hg/updatestate file is present. *)
let test_repo_starts_midupdate temp_dir =
  Tools.set_hg_to_global_rev_map ();

  (* Start by having the mock response Mid_update. *)
  WEWClient.Mocking.get_status_returns (Some WEWConfig.Responses.Mid_update);
  Watchman.Mocking.init_returns @@ Some "test_mock_basic";
  Hg.Mocking.current_working_copy_base_rev_returns
    (Future.of_value Tools.global_rev_1);
  Watchman.Mocking.get_changes_returns
    (Watchman.Watchman_pushed (Watchman.Files_changed SSet.empty));
  let informant =
    HhMonitorInformant.init
      {
        HhMonitorInformant.root = temp_dir;
        min_distance_restart = 100;
        allow_subscriptions = true;
        use_dummy = false;
        watchman_debug_logging = false;
        use_xdb = false;
        saved_state_cache_limit = 20;
        ignore_hh_version = false;
        ignore_hhconfig = false;
        is_saved_state_precomputed = false;
      }
  in
  let should_start_first_server =
    HhMonitorInformant.should_start_first_server informant
  in
  Asserter.Bool_asserter.assert_equals
    false
    should_start_first_server
    "Shouldn't start when repo is in flux";

  (* Set mock response to settled, then informant report should restart. *)
  WEWClient.Mocking.get_status_returns (Some WEWConfig.Responses.Settled);
  let report =
    HhMonitorInformant.report informant Informant_sig.Server_not_yet_started
  in
  Report_asserter.assert_equals
    (Informant_sig.Restart_server None)
    report
    "Should report restart server since repo is settled";
  true

(** When Watchman Event Watcher in unknown state, then we should start
 * first server. *)
let test_watcher_in_unknown_state temp_dir =
  Tools.set_hg_to_global_rev_map ();

  (* WEW status returns Unknown. *)
  WEWClient.Mocking.get_status_returns (Some WEWConfig.Responses.Unknown);
  Watchman.Mocking.init_returns @@ Some "test_mock_basic";
  Hg.Mocking.current_working_copy_base_rev_returns
    (Future.of_value Tools.global_rev_1);
  Watchman.Mocking.get_changes_returns
    (Watchman.Watchman_pushed (Watchman.Files_changed SSet.empty));
  let informant =
    HhMonitorInformant.init
      {
        HhMonitorInformant.root = temp_dir;
        min_distance_restart = 100;
        allow_subscriptions = true;
        use_dummy = false;
        watchman_debug_logging = false;
        use_xdb = false;
        saved_state_cache_limit = 20;
        ignore_hh_version = false;
        ignore_hhconfig = false;
        is_saved_state_precomputed = false;
      }
  in
  let should_start_first_server =
    HhMonitorInformant.should_start_first_server informant
  in
  Asserter.Bool_asserter.assert_equals
    true
    should_start_first_server
    "Should start when repo is in unknown state";
  true

let run_test test =
  Xdb.Mocking.reset_find_nearest ();
  Tempfile.with_tempdir test

let tests =
  [
    ( "test_informant_restarts_significant_move",
      (fun () -> run_test test_informant_restarts_significant_move) );
    (*"test_informant_restarts_significant_move_with_local_changes", (fun () ->
      run_test test_informant_restarts_significant_move_with_local_changes);
    "test_informant_restarts_significant_move_delayed", (fun () ->
      run_test test_informant_restarts_significant_move_delayed);
    "test_informant_no_saved_state_no_restart", (fun () ->
      run_test test_informant_no_saved_state_no_restart);
    "test_informant_xdb_saved_state_too_far", (fun () ->
      run_test test_informant_xdb_saved_state_too_far);
    "test_repo_starts_midupdate", (fun () ->
      run_test test_repo_starts_midupdate);
    "test_watcher_in_unknown_state", (fun () ->
      run_test test_watcher_in_unknown_state); *)
  ]
