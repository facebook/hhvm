let hg_rev_1 = "abc"

let hg_rev_5 = "def"

let hg_rev_200 = "ghi"

(* hg_rev_200 above with one more local commit on top of it. It
 * base revision is the same as hg_rev_200's (i.e. global_200). *)
let hg_rev_200_plus_local = "ghi_plus_local"

let hg_rev_230 = "jkl"

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

let set_next_watchman_state_transition move hg_rev =
  Hh_json.(
    let json = JSON_Object [("rev", JSON_String hg_rev)] in
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

(** Create an entry in XDB table for a saved state.
 *
 * stat_global_rev: The revision on which this saved state was created.
 * for_global_rev: The global_rev lookup in "Xdb.find_nearest" that will locate
 *              this result.
 *)
let set_xdb ~state_global_rev ~for_global_rev ~everstore_handle =
  let hh_version = Build_id.build_revision in
  let hg_hash =
    match state_global_rev with
    | 1 -> hg_rev_1
    | 5 -> hg_rev_5
    | 200 -> hg_rev_200
    | 230 -> hg_rev_230
    | _ ->
      Printf.eprintf "Error: Invalid global_rev number\n";
      assert false
  in
  let (hhconfig_hash, _config) =
    Config_file.parse_hhconfig ~silent:true "/tmp/.hhconfig"
  in
  let result =
    {
      Xdb.global_rev = state_global_rev;
      hg_hash;
      everstore_handle;
      hh_version;
      hhconfig_hash;
    }
  in
  let result = Future.of_value [result] in
  Xdb.Mocking.find_nearest_returns
    ~db:Xdb.hack_db_name
    ~db_table:Xdb.saved_states_table
    ~global_rev:for_global_rev
    ~hh_version:(Some hh_version)
    result
