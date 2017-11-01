let hg_rev_1 = "abc"
let hg_rev_5 = "def"
let hg_rev_200 = "ghi"
let hg_rev_230 = "jkl"
let svn_1 = 1
let svn_5 = 5
let svn_200 = 200 (** is a significant distance from the above. *)
let svn_230 = 230

type state_transition =
  | State_leave
  | State_enter
  | Changed_merge_base

let set_hg_to_svn_map ?delay_rev_200 () =
  Hg.Mocking.closest_svn_ancestor_bind_value hg_rev_1
    @@ Future.of_value svn_1;
  Hg.Mocking.closest_svn_ancestor_bind_value hg_rev_5
    @@ Future.of_value svn_5;
  Hg.Mocking.closest_svn_ancestor_bind_value hg_rev_200
    begin match delay_rev_200 with
      | None -> Future.of_value svn_200
      | Some i -> Future.delayed_value ~delays:i svn_200
    end;
  Hg.Mocking.closest_svn_ancestor_bind_value hg_rev_230
    @@ Future.of_value svn_230

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

(** Create an entry in XDB table for a saved state.
 *
 * stat_svn_rev: The revision on which this saved state was created.
 * for_svn_rev: The svn_rev lookup in "Xdb.find_nearest" that will locate
 *              this result.
 *)
let set_xdb ~state_svn_rev ~for_svn_rev ~everstore_handle ~tiny  =
  let hh_version = Build_id.build_revision in
  let hg_hash = match state_svn_rev with
    | 1 -> hg_rev_1
    | 5 -> hg_rev_5
    | 200 -> hg_rev_200
    | 230 -> hg_rev_230
    | _ ->
      Printf.eprintf "Error: Invalid svn_rev number\n";
      assert false
  in
  let hhconfig_hash, _config = Config_file.parse "/tmp/.hhconfig" in
  match hhconfig_hash with
  | None ->
    Printf.eprintf "Error: Failed to get hash of config file. Cannot continue.\n";
    assert false
  | Some hhconfig_hash ->
    let result = {
      Xdb.svn_rev = state_svn_rev;
      hg_hash;
      everstore_handle;
      hh_version;
      hhconfig_hash;
    } in
    let result = Future.of_value [result] in
    Xdb.Mocking.find_nearest_returns ~db:Xdb.hack_db_name
      ~db_table:Xdb.mini_saved_states_table
      ~svn_rev:for_svn_rev ~hh_version ~hhconfig_hash ~tiny result
