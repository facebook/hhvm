let hg_rev_1 = Hg.Rev.of_string "abc"

let hg_rev_5 = Hg.Rev.of_string "def"

let hg_rev_200 = Hg.Rev.of_string "ghi"

(* hg_rev_200 above with one more local commit on top of it. It
 * base revision is the same as hg_rev_200's (i.e. global_200). *)
let hg_rev_200_plus_local = Hg.Rev.of_string "ghi_plus_local"

let hg_rev_230 = Hg.Rev.of_string "jkl"

let global_rev_1 = 1

let global_rev_5 = 5

let global_rev_200 = 200 (* is a significant distance from the above. *)

let global_rev_230 = 230

type state_transition =
  | State_leave
  | State_enter
  | Changed_merge_base
  | Changed_merge_base_plus_files of SSet.t

let set_hg_to_global_rev_map ?delay_rev_200 () =
  Hg.Mocking.closest_global_ancestor_bind_value hg_rev_1
  @@ Future.of_value global_rev_1;
  Hg.Mocking.closest_global_ancestor_bind_value hg_rev_5
  @@ Future.of_value global_rev_5;
  Hg.Mocking.closest_global_ancestor_bind_value
    hg_rev_200
    (match delay_rev_200 with
    | None -> Future.of_value global_rev_200
    | Some i -> Future.delayed_value ~delays:i global_rev_200);

  (* This local commit has the same base revision as hg_rev_200. *)
  Hg.Mocking.closest_global_ancestor_bind_value hg_rev_200_plus_local
  @@ Future.of_value global_rev_200;
  Hg.Mocking.closest_global_ancestor_bind_value hg_rev_230
  @@ Future.of_value global_rev_230

let set_next_watchman_state_transition move (hg_rev : Hg.Rev.t) =
  Hh_json.(
    let json = JSON_Object [("rev", JSON_String (Hg.Rev.to_string hg_rev))] in
    let move =
      match move with
      | State_leave -> Watchman.State_leave ("hg.update", Some json)
      | State_enter -> Watchman.State_enter ("hg.update", Some json)
      | Changed_merge_base ->
        Watchman.Changed_merge_base (hg_rev, SSet.empty, "dummy_clock")
      | Changed_merge_base_plus_files files ->
        Watchman.Changed_merge_base (hg_rev, files, "dummy_clock")
    in
    Watchman.Mocking.get_changes_returns (Watchman.Watchman_pushed move))
