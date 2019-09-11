open Core_kernel
open Asserter.Int_asserter
open Integration_test_base_types
module Test = Integration_test_base

let make_enum_contents =
  Printf.sprintf "<?hh // strict
enum MyEnum : int {
  A = 1;
  %s
}"

let enum_contents = make_enum_contents ""

let enum_contents_with_added_variant = make_enum_contents "B = 2;"

let enum_user_name = Printf.sprintf "my_enum_user_%d.php"

let enum_user_contents =
  Printf.sprintf
    "<?hh // strict
function enum_user_%d(): void {
  MyEnum::A;
}"

let enum_users =
  List.init 300 ~f:(fun n -> (enum_user_name n, enum_user_contents n))

let enum_switch_name = Printf.sprintf "enum_switch_%d.php"

let enum_switch_contents =
  Printf.sprintf
    "<?hh // strict
function enum_switch_%d(MyEnum $x) : void {
  /* HH_FIXME[4019] non-exhaustive check */
  switch($x) {
  }
}"

let enum_switches =
  List.init 20 ~f:(fun n -> (enum_switch_name n, enum_switch_contents n))

let init_disk_state =
  [
    ("hack/hh_hot_classes.json", {|{"classes":[ "\\MyEnum" ]}|});
    ("my_enum.php", enum_contents);
  ]
  @ enum_users
  @ enum_switches

let added_member = [("my_enum.php", enum_contents_with_added_variant)]

let save_state saved_state_dir =
  Test.save_state
    init_disk_state
    saved_state_dir
    ~load_hhi_files:true
    ~store_decls_in_saved_state:true

let load_state
    ?(master_changes = [])
    ?(local_changes = [])
    ?(load_decls = true)
    saved_state_dir =
  Test.load_state
    saved_state_dir
    ~disk_state:(init_disk_state @ master_changes @ local_changes)
    ~master_changes:(List.map master_changes ~f:fst)
    ~local_changes:(List.map local_changes ~f:fst)
    ~load_hhi_files:true
    ~use_precheked_files:true
    ~load_decls_from_saved_state:load_decls

let test_recheck_stats_after_simple_change saved_state_dir () =
  let env = load_state saved_state_dir in
  let (env, loop_output) = Test.change_files env added_member in
  assert_equals 21 loop_output.total_rechecked_count
  @@ "Adding a new member to an enum after init should cause us "
  ^ "to recheck only its AllMembers dependents";

  let () =
    match loop_output.last_actual_total_rechecked_count with
    | None -> assert false
    | Some count ->
      assert_equals 21 count
      @@ "The last actual recheck loop stats should be correct"
  in
  let (env, loop_output) = Test.(run_loop_once env default_loop_input) in
  assert_equals 0 loop_output.total_rechecked_count
  @@ "We should have already checked the changes";

  let () =
    match loop_output.last_actual_total_rechecked_count with
    | None -> assert false
    | Some count ->
      assert_equals 21 count
      @@ "The last actual recheck loop stats shouldn't be updated by no-op rechecks"
  in
  Test.assert_no_errors env

let test () =
  Tempfile.with_real_tempdir
  @@ fun temp_dir ->
  let temp_dir = Path.to_string temp_dir in
  save_state temp_dir;

  Test.in_daemon @@ test_recheck_stats_after_simple_change temp_dir;
  ()
