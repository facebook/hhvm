open Hh_prelude

module Report_comparator : Asserter.Comparator with type t = Informant.report =
struct
  open Informant

  type t = Informant.report

  let to_string v =
    match v with
    | Move_along -> "Move_along"
    | Restart_server -> "Restart_server"

  let is_equal exp actual = String.equal (to_string exp) (to_string actual)
end

module Report_asserter = Asserter.Make_asserter (Report_comparator)

module Tools = struct
  include Informant_test_tools

  (** Test the given transition to an hg_rev and assert the expected report. *)
  let test_transition_watchman
      informant transition hg_rev server_status expected_report assert_msg =
    set_next_watchman_state_transition transition hg_rev;
    let report = Informant.report informant server_status in
    Report_asserter.assert_equals expected_report report assert_msg

  let test_transition_eden
      informant transition hg_rev server_status expected_report assert_msg =
    set_next_eden_state_transitions [transition] hg_rev;
    let report = Informant.report informant server_status in
    Report_asserter.assert_equals expected_report report assert_msg
end

(** Create XDB table entries for rev 5 and 200. Start Informant. *)
let basic_setup_rev_5_and_200_and_start_informant_watchman temp_dir =
  Tools.set_hg_to_global_rev_map ();

  Watchman.Mocking.init_returns @@ Some "test_mock_basic";
  Hg.Mocking.current_working_copy_base_rev_returns
    (Future.of_value Tools.global_rev_1);
  Watchman.Mocking.get_changes_returns
    (Watchman.Watchman_pushed (Watchman.Files_changed SSet.empty));
  let informant =
    Informant.init
      {
        Informant.root = temp_dir;
        allow_subscriptions = true;
        min_distance_restart = 100;
        use_dummy = false;
        watchman_debug_logging = false;
        use_eden = false;
        ignore_hh_version = false;
        is_saved_state_precomputed = false;
      }
  in
  let report = Informant.report informant Informant.Server_alive in
  Report_asserter.assert_equals Informant.Move_along report "no distance moved";
  informant

let basic_setup_rev_5_and_200_and_start_informant_eden ?delay_rev_5 temp_dir =
  Tools.set_hg_to_global_rev_map ?delay_rev_5 ();

  Hg.Mocking.current_working_copy_base_rev_returns
    (Future.of_value Tools.global_rev_1);
  Edenfs_watcher.Mocking.get_changes_async_returns [];
  let informant =
    Informant.init
      {
        Informant.root = temp_dir;
        allow_subscriptions = true;
        min_distance_restart = 100;
        use_dummy = false;
        watchman_debug_logging = false;
        use_eden = true;
        ignore_hh_version = false;
        is_saved_state_precomputed = false;
      }
  in
  let report = Informant.report informant Informant.Server_alive in
  Report_asserter.assert_equals Informant.Move_along report "no distance moved";
  informant

(** When base revision has changed significantly, informant asks
 * for server restart. Also, ensures that there are entries in XDB
 * table for those revisions. *)
let test_watchman_informant_restarts_significant_move temp_dir =
  let informant =
    basic_setup_rev_5_and_200_and_start_informant_watchman temp_dir
  in
  let test_transition = Tools.test_transition_watchman in

  (**** Following tests all have a State_enter followed by a State_leave
   * and then possibly a Changed_merge_base. *)

  (* Move base revisions insignificant distance away. *)
  test_transition
    informant
    Tools.State_enter
    Tools.hg_rev_5
    Informant.Server_alive
    Informant.Move_along
    "state enter insignificant distance";
  test_transition
    informant
    Tools.State_leave
    Tools.hg_rev_5
    Informant.Server_alive
    Informant.Move_along
    "state leave insignificant distance";

  (* Move significant distance. *)
  test_transition
    informant
    Tools.State_enter
    Tools.hg_rev_200
    Informant.Server_alive
    Informant.Move_along
    "state enter significant distance";
  test_transition
    informant
    Tools.State_leave
    Tools.hg_rev_200
    Informant.Server_alive
    Informant.Move_along
    "state leave significant distance";
  test_transition
    informant
    Tools.Changed_merge_base
    Tools.hg_rev_200
    Informant.Server_alive
    Informant.Restart_server
    "Move forward significant distance";

  (* Informant now sitting at revision 200. Moving to 230 no restart. *)
  test_transition
    informant
    Tools.State_enter
    Tools.hg_rev_230
    Informant.Server_alive
    Informant.Move_along
    "state enter insignificant distance";
  test_transition
    informant
    Tools.State_leave
    Tools.hg_rev_230
    Informant.Server_alive
    Informant.Move_along
    "state leave insignificant distance";
  test_transition
    informant
    Tools.Changed_merge_base
    Tools.hg_rev_230
    Informant.Server_alive
    Informant.Move_along
    "Changed merge base insignificant distance";

  (* Moving back to 200 no restart. *)
  test_transition
    informant
    Tools.State_enter
    Tools.hg_rev_200
    Informant.Server_alive
    Informant.Move_along
    "state enter insignificant distance";
  test_transition
    informant
    Tools.State_leave
    Tools.hg_rev_200
    Informant.Server_alive
    Informant.Move_along
    "state leave insignificant distance";
  test_transition
    informant
    Tools.Changed_merge_base
    Tools.hg_rev_200
    Informant.Server_alive
    Informant.Move_along
    "Changed_merge_base insignificant distance";

  (* Moving to a local commit on top of 200 (same globalrev/merge base).
     No Changed_merge_base! *)
  test_transition
    informant
    Tools.State_enter
    Tools.hg_rev_200_plus_local
    Informant.Server_alive
    Informant.Move_along
    "state enter insignificant distance";
  test_transition
    informant
    Tools.State_leave
    Tools.hg_rev_200_plus_local
    Informant.Server_alive
    Informant.Move_along
    "state leave insignificant distance";

  (* Moving back to SVN rev 5 (hg_rev_5) restarts. *)
  test_transition
    informant
    Tools.State_enter
    Tools.hg_rev_5
    Informant.Server_alive
    Informant.Move_along
    "state enter significant distance";
  test_transition
    informant
    Tools.State_leave
    Tools.hg_rev_5
    Informant.Server_alive
    Informant.Move_along
    "state leave significant distance";
  test_transition
    informant
    Tools.Changed_merge_base
    Tools.hg_rev_5
    Informant.Server_alive
    Informant.Restart_server
    "Move back significant distance";
  true

(** Like test_watchman_informant_restarts_significant_move, but using Eden.
    Therefore, we get Changed_commit instead of Changed_merge_base  *)
let test_eden_informant_restarts_significant_move temp_dir =
  let informant = basic_setup_rev_5_and_200_and_start_informant_eden temp_dir in
  let test_transition = Tools.test_transition_eden in

  (**** Following tests all have a State_enter followed by a State_leave
   * and then possibly a Changed_commit. *)

  (* Move base revisions insignificant distance away. *)
  test_transition
    informant
    Tools.State_enter
    Tools.hg_rev_5
    Informant.Server_alive
    Informant.Move_along
    "state enter (not translated to ServerInformant.repo_transition)";
  test_transition
    informant
    Tools.State_leave
    Tools.hg_rev_5
    Informant.Server_alive
    Informant.Move_along
    "state leave (not translated to ServerInformant.repo_transition)";

  (* Move significant distance. *)
  test_transition
    informant
    Tools.State_enter
    Tools.hg_rev_200
    Informant.Server_alive
    Informant.Move_along
    "state enter (not translated to ServerInformant.repo_transition)";
  test_transition
    informant
    Tools.State_leave
    Tools.hg_rev_200
    Informant.Server_alive
    Informant.Move_along
    "state leave (not translated to ServerInformant.repo_transition)";
  test_transition
    informant
    Tools.Commit_transition
    Tools.hg_rev_200
    Informant.Server_alive
    Informant.Restart_server
    "Move forward significant distance";

  (* Informant now sitting at revision 200. Moving to 230 no restart. *)
  test_transition
    informant
    Tools.State_enter
    Tools.hg_rev_230
    Informant.Server_alive
    Informant.Move_along
    "state enter (not translated to ServerInformant.repo_transition)";
  test_transition
    informant
    Tools.State_leave
    Tools.hg_rev_230
    Informant.Server_alive
    Informant.Move_along
    "state leave (not translated to ServerInformant.repo_transition)";
  test_transition
    informant
    Tools.Commit_transition
    Tools.hg_rev_230
    Informant.Server_alive
    Informant.Move_along
    "Changed commit insignificant distance";

  (* Moving back to 200 no restart. *)
  test_transition
    informant
    Tools.State_enter
    Tools.hg_rev_200
    Informant.Server_alive
    Informant.Move_along
    "state enter (not translated to ServerInformant.repo_transition)";
  test_transition
    informant
    Tools.State_leave
    Tools.hg_rev_200
    Informant.Server_alive
    Informant.Move_along
    "state leave (not translated to ServerInformant.repo_transition)";
  test_transition
    informant
    Tools.Commit_transition
    Tools.hg_rev_200
    Informant.Server_alive
    Informant.Move_along
    "Changed_commit insignificant distance";

  (* Moving to a local commit on top of 200 (same globalrev/merge base).
     Watchman produces no Changed_merge_base here! *)
  test_transition
    informant
    Tools.State_enter
    Tools.hg_rev_200_plus_local
    Informant.Server_alive
    Informant.Move_along
    "state enter (not translated to ServerInformant.repo_transition)";
  test_transition
    informant
    Tools.State_leave
    Tools.hg_rev_200_plus_local
    Informant.Server_alive
    Informant.Move_along
    "state leave (not translated to ServerInformant.repo_transition)";
  test_transition
    informant
    Tools.Commit_transition
    Tools.hg_rev_200
    Informant.Server_alive
    Informant.Move_along
    "Changed_commit insignificant distance";

  (* Moving back to rev 5 (hg_rev_5) restarts. *)
  test_transition
    informant
    Tools.State_enter
    Tools.hg_rev_5
    Informant.Server_alive
    Informant.Move_along
    "state enter (not translated to ServerInformant.repo_transition)";
  test_transition
    informant
    Tools.State_leave
    Tools.hg_rev_5
    Informant.Server_alive
    Informant.Move_along
    "state leave (not translated to ServerInformant.repo_transition)";
  test_transition
    informant
    Tools.Commit_transition
    Tools.hg_rev_5
    Informant.Server_alive
    Informant.Restart_server
    "Move back significant distance";
  true

(** Test that when Eden delivers a batch of [StateEnter; StateLeave; CommitTransition],
    the pending queue processes them in FIFO order *)
let test_eden_batch_ordering temp_dir =
  let informant = basic_setup_rev_5_and_200_and_start_informant_eden temp_dir in
  Tools.set_next_eden_state_transitions
    [Tools.State_enter; Tools.State_leave; Tools.Commit_transition]
    Tools.hg_rev_200;
  let report = Informant.report informant Informant.Server_alive in
  Report_asserter.assert_equals
    Informant.Restart_server
    report
    "Batch [StateEnter; StateLeave; CommitTransition] should restart";
  true

(** Test that preprocessing short-circuits when a newer change's
    future resolves before an older queued one, and that the early decision
    clears the queue of pending changes. *)
let test_eden_preprocess_early_decision temp_dir =
  (* delay_rev_5:10 is well above the number of is_ready calls that happen
     during the first report, ensuring that rev_5's future stays unresolved
     and the change gets queued. *)
  let informant =
    basic_setup_rev_5_and_200_and_start_informant_eden ~delay_rev_5:10 temp_dir
  in

  (* Send a batch for rev_5.  Its future is delayed, so it can't be resolved yet. *)
  Tools.set_next_eden_state_transitions
    [Tools.State_enter; Tools.State_leave; Tools.Commit_transition]
    Tools.hg_rev_5;
  let report = Informant.report informant Informant.Server_alive in
  Report_asserter.assert_equals
    Informant.Move_along
    report
    "rev_5 future not ready, queued";

  (* Send a batch for rev_200.  Its future resolves immediately and
     |200 - 1| > 100, so preprocess makes an early restart decision,
     clearing the queue (discarding the pending rev_5 entry). *)
  Tools.set_next_eden_state_transitions
    [Tools.State_enter; Tools.State_leave; Tools.Commit_transition]
    Tools.hg_rev_200;
  let report = Informant.report informant Informant.Server_alive in
  Report_asserter.assert_equals
    Informant.Restart_server
    report
    "rev_200 future ready, preprocess early restart";

  (* Verify queue was cleared: pump enough empty reports for rev_5's delayed
     future to have resolved if it were still in the queue.  Since
     |5 - 200| > 100, a surviving entry would trigger Restart_server. *)
  Edenfs_watcher.Mocking.get_changes_async_returns [];
  for _ = 1 to 10 do
    let report = Informant.report informant Informant.Server_alive in
    Report_asserter.assert_equals
      Informant.Move_along
      report
      "queue should be empty after early decision"
  done;

  (* Verify the early decision updated base to globalrev 200: rev_230 should
     be insignificant (|230 - 200| < 100).  If base were still 1,
     this would trigger a restart (|230 - 1| > 100). *)
  Tools.set_next_eden_state_transitions
    [Tools.State_enter; Tools.State_leave; Tools.Commit_transition]
    Tools.hg_rev_230;
  let report = Informant.report informant Informant.Server_alive in
  Report_asserter.assert_equals
    Informant.Move_along
    report
    "rev_230 insignificant from new base 200";
  true

let () =
  EventLogger.init_fake ();
  Relative_path.(set_path_prefix Root (Path.make "/tmp"));
  Relative_path.(set_path_prefix Root (Path.make Sys_utils.temp_dir_name));
  let hhconfig_path = Relative_path.(create Root "/tmp/.hhconfig") in
  Disk.write_file
    ~file:(Relative_path.to_absolute hhconfig_path)
    ~contents:"assume_php = false";

  Unit_test.run_all
    [
      ( "test_watchman_informant_restarts_significant_move",
        fun () ->
          Tempfile.with_tempdir
            test_watchman_informant_restarts_significant_move );
      ( "test_eden_informant_restarts_significant_move",
        fun () ->
          Tempfile.with_tempdir test_eden_informant_restarts_significant_move );
      ( "test_eden_batch_ordering",
        (fun () -> Tempfile.with_tempdir test_eden_batch_ordering) );
      ( "test_eden_preprocess_early_decision",
        (fun () -> Tempfile.with_tempdir test_eden_preprocess_early_decision) );
    ]
