module Report_comparator : Asserter.Comparator
  with type t = Informant_sig.report = struct
    open Informant_sig
    type t = Informant_sig.report
    let to_string v = match v with
      | Move_along ->
        "Move_along"
      | Restart_server ->
        "Restart_server"

    let is_equal exp actual =
      exp = actual
end;;


module Report_asserter = Asserter.Make_asserter (Report_comparator);;

module WEWClient = WatchmanEventWatcherClient
module WEWConfig = WatchmanEventWatcherConfig

module Tools = struct
  let hg_rev_1 = "abc"
  let hg_rev_2 = "def"
  let hg_rev_3 = "ghi"
  let hg_rev_4 = "jkl"
  let svn_1 = "1"
  let svn_2 = "5"
  let svn_3 = "200" (** is a significant distance from the above. *)
  let svn_4 = "230"

  type state_transition =
    | State_leave
    | State_enter
    | Changed_merge_base

  let set_hg_to_svn_map () =
    Hg.Mocking.closest_svn_ancestor_bind_value hg_rev_1
      @@ Future.of_value svn_1;
    Hg.Mocking.closest_svn_ancestor_bind_value hg_rev_2
      @@ Future.of_value svn_2;
    Hg.Mocking.closest_svn_ancestor_bind_value hg_rev_3
      @@ Future.of_value svn_3;
    Hg.Mocking.closest_svn_ancestor_bind_value hg_rev_4
      @@ Future.of_value svn_4

  let set_next_watchman_state_transition move hg_rev =
    let open Hh_json in
    let json = JSON_Object
      [("rev", JSON_String hg_rev)] in
    let move = match move with
    | State_leave ->
      Watchman.State_leave ("hg.update", (Some json))
    | State_enter ->
      Watchman.State_enter ("hg.update", (Some json))
    | Changed_merge_base ->
      Watchman.Changed_merge_base (hg_rev, SSet.empty)
    in
    Watchman.Mocking.get_changes_returns
      (Watchman.Watchman_pushed move)

  (** Test the given transition to an hg_rev and assert the expected report. *)
  let test_transition informant transition hg_rev server_status
    expected_report assert_msg =
      set_next_watchman_state_transition
        transition hg_rev;
      let report = HhMonitorInformant.report
        informant server_status in
      Report_asserter.assert_equals expected_report report
        assert_msg
end;;


(** When base revision has changed significantly, informant asks
 * for server restart. *)
let test_informant_restarts_significant_move temp_dir =
  Tools.set_hg_to_svn_map ();
  Watchman.Mocking.init_returns @@ Some "test_mock_basic";
  Hg.Mocking.current_working_copy_base_rev_returns
    (Future.of_value Tools.svn_1);
  Watchman.Mocking.get_changes_returns
    (Watchman.Watchman_pushed (Watchman.Files_changed SSet.empty));
  let informant = HhMonitorInformant.init {
    HhMonitorInformant.root = temp_dir;
    state_prefetcher = State_prefetcher.dummy;
    allow_subscriptions = true;
    min_distance_restart = 100;
    use_dummy = false;
  } in
  let report = HhMonitorInformant.report
    informant Informant_sig.Server_alive in
  Report_asserter.assert_equals Informant_sig.Move_along report
    "no distance moved" ;

  (**** Following tests all have a State_enter followed by a State_leave
   * and then a Changed_merge_base. *)

  (** Move base revisions insignificant distance away. *)
  Tools.test_transition
    informant Tools.State_enter Tools.hg_rev_2
    Informant_sig.Server_alive Informant_sig.Move_along
    "state enter insignificant distance";
  Tools.test_transition
    informant Tools.State_leave Tools.hg_rev_2
    Informant_sig.Server_alive Informant_sig.Move_along
    "state leave insignificant distance";

  (** Move significant distance. *)
  Tools.test_transition
    informant Tools.State_enter Tools.hg_rev_3
    Informant_sig.Server_alive Informant_sig.Move_along
    "state enter significant distance";
  Tools.test_transition
    informant Tools.State_leave Tools.hg_rev_3
    Informant_sig.Server_alive Informant_sig.Move_along
    "state leave significant distance";
  Tools.test_transition
    informant Tools.Changed_merge_base Tools.hg_rev_3
    Informant_sig.Server_alive Informant_sig.Restart_server
    "Changed merge base significant distance";

  (** Informant now sitting at revision 200. Moving to 230 no restart. *)
  Tools.test_transition
    informant Tools.State_enter Tools.hg_rev_4
    Informant_sig.Server_alive Informant_sig.Move_along
    "state enter insignificant distance";
  Tools.test_transition
    informant Tools.State_leave Tools.hg_rev_4
    Informant_sig.Server_alive Informant_sig.Move_along
    "state leave insignificant distance";
  Tools.test_transition
    informant Tools.Changed_merge_base Tools.hg_rev_4
    Informant_sig.Server_alive Informant_sig.Move_along
    "Changed merge base insignificant distance";

  (** Moving back to 200 no restart. *)
  Tools.test_transition
    informant Tools.State_enter Tools.hg_rev_3
    Informant_sig.Server_alive Informant_sig.Move_along
    "state enter insignificant distance";
  Tools.test_transition
    informant Tools.State_leave Tools.hg_rev_3
    Informant_sig.Server_alive Informant_sig.Move_along
    "state leave insignificant distance";
  Tools.test_transition
    informant Tools.Changed_merge_base Tools.hg_rev_3
    Informant_sig.Server_alive Informant_sig.Move_along
    "Changed_merge_base insignificant distance";

  (** Moving back to SVN rev 5 (hg_rev_2) restarts. *)
  Tools.test_transition
    informant Tools.State_enter Tools.hg_rev_2
    Informant_sig.Server_alive Informant_sig.Move_along
    "state enter significant distance";
  Tools.test_transition
    informant Tools.State_leave Tools.hg_rev_2
    Informant_sig.Server_alive Informant_sig.Move_along
    "state leave significant distance";
  Tools.test_transition
    informant Tools.Changed_merge_base Tools.hg_rev_2
    Informant_sig.Server_alive Informant_sig.Restart_server
    "Changed merge base significant distance";
  true

(** We emulate the repo being in a mid-update state when the informant
 * starts. i.e., the .hg/updatestate file is present. *)
let test_repo_starts_midupdate temp_dir =
  Tools.set_hg_to_svn_map ();
  (** Start by having the mock response Mid_update. *)
  WEWClient.Mocking.get_status_returns (Some WEWConfig.Responses.Mid_update);
  Watchman.Mocking.init_returns @@ Some "test_mock_basic";
  Hg.Mocking.current_working_copy_base_rev_returns
    (Future.of_value Tools.svn_1);
  Watchman.Mocking.get_changes_returns
    (Watchman.Watchman_pushed (Watchman.Files_changed SSet.empty));
  let informant = HhMonitorInformant.init {
    HhMonitorInformant.root = temp_dir;
    min_distance_restart = 100;
    state_prefetcher = State_prefetcher.dummy;
    allow_subscriptions = true;
    use_dummy = false;
  } in
  let should_start_first_server =
    HhMonitorInformant.should_start_first_server informant in
  Asserter.Bool_asserter.assert_equals false
    should_start_first_server "Shouldn't start when repo is in flux";
  (** Set mock response to settled, then informant report should restart. *)
  WEWClient.Mocking.get_status_returns (Some WEWConfig.Responses.Settled);
  let report = HhMonitorInformant.report
    informant Informant_sig.Server_not_yet_started in
  Report_asserter.assert_equals
    Informant_sig.Restart_server report
    "Should report restart server since repo is settled";
  true

(** When Watchman Event Watcher in unknown state, then we should start
 * first server. *)
let test_watcher_in_unknown_state temp_dir =
  Tools.set_hg_to_svn_map ();
  (** WEW status returns Unknown. *)
  WEWClient.Mocking.get_status_returns (Some WEWConfig.Responses.Unknown);
  Watchman.Mocking.init_returns @@ Some "test_mock_basic";
  Hg.Mocking.current_working_copy_base_rev_returns
    (Future.of_value Tools.svn_1);
  Watchman.Mocking.get_changes_returns
    (Watchman.Watchman_pushed (Watchman.Files_changed SSet.empty));
  let informant = HhMonitorInformant.init {
    HhMonitorInformant.root = temp_dir;
    min_distance_restart = 100;
    state_prefetcher = State_prefetcher.dummy;
    allow_subscriptions = true;
    use_dummy = false;
  } in
  let should_start_first_server =
    HhMonitorInformant.should_start_first_server informant in
  Asserter.Bool_asserter.assert_equals true
    should_start_first_server "Should start when repo is in unknown state";
    true

let tests =
  [
    "test_informant_restarts_significant_move", (fun () ->
      Tempfile.with_tempdir test_informant_restarts_significant_move);
    "test_repo_starts_midupdate", (fun () ->
      Tempfile.with_tempdir test_repo_starts_midupdate);
    "test_watcher_in_unknown_state", (fun () ->
      Tempfile.with_tempdir test_watcher_in_unknown_state);
  ]

let () =
  EventLogger.init EventLogger.Event_logger_fake 0.0;
  Unit_test.run_all tests
