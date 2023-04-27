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
module WEWClient = WatchmanEventWatcherClient
module WEWConfig = WatchmanEventWatcherConfig

module Tools = struct
  include Informant_test_tools

  (** Test the given transition to an hg_rev and assert the expected report. *)
  let test_transition
      informant transition hg_rev server_status expected_report assert_msg =
    set_next_watchman_state_transition transition hg_rev;
    let report = Informant.report informant server_status in
    Report_asserter.assert_equals expected_report report assert_msg
end

(** Create XDB table entries for rev 5 and 200. Start Informant. *)
let basic_setup_rev_5_and_200_and_start_informant temp_dir =
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
let test_informant_restarts_significant_move temp_dir =
  let informant = basic_setup_rev_5_and_200_and_start_informant temp_dir in

  (**** Following tests all have a State_enter followed by a State_leave
   * and then a Changed_merge_base. *)

  (* Move base revisions insignificant distance away. *)
  Tools.test_transition
    informant
    Tools.State_enter
    Tools.hg_rev_5
    Informant.Server_alive
    Informant.Move_along
    "state enter insignificant distance";
  Tools.test_transition
    informant
    Tools.State_leave
    Tools.hg_rev_5
    Informant.Server_alive
    Informant.Move_along
    "state leave insignificant distance";

  (* Move significant distance. *)
  Tools.test_transition
    informant
    Tools.State_enter
    Tools.hg_rev_200
    Informant.Server_alive
    Informant.Move_along
    "state enter significant distance";
  Tools.test_transition
    informant
    Tools.State_leave
    Tools.hg_rev_200
    Informant.Server_alive
    Informant.Move_along
    "state leave significant distance";
  Tools.test_transition
    informant
    Tools.Changed_merge_base
    Tools.hg_rev_200
    Informant.Server_alive
    Informant.Restart_server
    "Move forward significant distance";

  (* Informant now sitting at revision 200. Moving to 230 no restart. *)
  Tools.test_transition
    informant
    Tools.State_enter
    Tools.hg_rev_230
    Informant.Server_alive
    Informant.Move_along
    "state enter insignificant distance";
  Tools.test_transition
    informant
    Tools.State_leave
    Tools.hg_rev_230
    Informant.Server_alive
    Informant.Move_along
    "state leave insignificant distance";
  Tools.test_transition
    informant
    Tools.Changed_merge_base
    Tools.hg_rev_230
    Informant.Server_alive
    Informant.Move_along
    "Changed merge base insignificant distance";

  (* Moving back to 200 no restart. *)
  Tools.test_transition
    informant
    Tools.State_enter
    Tools.hg_rev_200
    Informant.Server_alive
    Informant.Move_along
    "state enter insignificant distance";
  Tools.test_transition
    informant
    Tools.State_leave
    Tools.hg_rev_200
    Informant.Server_alive
    Informant.Move_along
    "state leave insignificant distance";
  Tools.test_transition
    informant
    Tools.Changed_merge_base
    Tools.hg_rev_200
    Informant.Server_alive
    Informant.Move_along
    "Changed_merge_base insignificant distance";

  (* Moving back to SVN rev 5 (hg_rev_5) restarts. *)
  Tools.test_transition
    informant
    Tools.State_enter
    Tools.hg_rev_5
    Informant.Server_alive
    Informant.Move_along
    "state enter significant distance";
  Tools.test_transition
    informant
    Tools.State_leave
    Tools.hg_rev_5
    Informant.Server_alive
    Informant.Move_along
    "state leave significant distance";
  Tools.test_transition
    informant
    Tools.Changed_merge_base
    Tools.hg_rev_5
    Informant.Server_alive
    Informant.Restart_server
    "Move back significant distance";
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
      ( "test_informant_restarts_significant_move",
        fun () ->
          Tempfile.with_tempdir test_informant_restarts_significant_move );
    ]
