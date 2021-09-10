open Hh_prelude

module Report_comparator :
  Asserter.Comparator with type t = Informant_sig.report = struct
  open Informant_sig

  type t = Informant_sig.report

  let to_string v =
    match v with
    | Move_along -> "Move_along"
    | Restart_server -> "Restart_server"

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
        ignore_hh_version = false;
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
  Tools.test_transition
    informant
    Tools.Changed_merge_base
    Tools.hg_rev_200
    Informant_sig.Server_alive
    Informant_sig.Restart_server
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
  Tools.test_transition
    informant
    Tools.Changed_merge_base
    Tools.hg_rev_5
    Informant_sig.Server_alive
    Informant_sig.Restart_server
    "Move back significant distance";
  true

let run_test test = Tempfile.with_tempdir test

let tests =
  [
    ( "test_informant_restarts_significant_move",
      (fun () -> run_test test_informant_restarts_significant_move) );
  ]
