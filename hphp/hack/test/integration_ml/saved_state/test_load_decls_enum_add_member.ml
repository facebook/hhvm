open Core_kernel
open Asserter.Int_asserter
open Integration_test_base_types

module Test = Integration_test_base

let make_enum_contents = Printf.sprintf "<?hh // strict
enum MyEnum : int {
  A = 1;
  %s
}"
let enum_contents = make_enum_contents ""
let enum_contents_with_added_variant = make_enum_contents "B = 2;"

let enum_user_name = Printf.sprintf "my_enum_user_%d.php"
let enum_user_contents = Printf.sprintf "<?hh // strict
function enum_user_%d(): void {
  MyEnum::A;
}"
let enum_users =
  List.init 300 (fun n -> enum_user_name n, enum_user_contents n)

let enum_switch_name = Printf.sprintf "enum_switch_%d.php"
let enum_switch_contents = Printf.sprintf "<?hh // strict
function enum_switch_%d(MyEnum $x) : void {
  /* HH_FIXME[4019] non-exhaustive check */
  switch($x) {
  }
}"
let enum_switches =
  List.init 20 (fun n -> enum_switch_name n, enum_switch_contents n)

let init_disk_state =
  [ "hack/hh_hot_classes.json", {|{"classes":[ "\\MyEnum" ]}|}
  ; "my_enum.php", enum_contents ]
  @ enum_users
  @ enum_switches

let added_member = ["my_enum.php", enum_contents_with_added_variant]

let save_state saved_state_dir =
  Test.save_state init_disk_state saved_state_dir
    ~load_hhi_files:true
    ~store_decls_in_saved_state:true

let load_state
    ?(master_changes=[])
    ?(local_changes=[])
    ?(load_decls=true)
    saved_state_dir =
  Test.load_state saved_state_dir
    ~disk_state:(init_disk_state @ master_changes @ local_changes)
    ~master_changes:(List.map master_changes fst)
    ~local_changes:(List.map local_changes fst)
    ~load_hhi_files:true
    ~use_precheked_files:true
    ~load_decls_from_saved_state:load_decls


let test_change_after_init_no_loaded_decls saved_state_dir () =
  let env = load_state saved_state_dir ~load_decls:false in
  let _, loop_output = Test.change_files env added_member in
  assert_equals 321 loop_output.total_rechecked_count @@
    "When we don't store an enum declaration in the saved state, "^
    "adding a member causes us to recheck all of its users"


let test_change_after_init saved_state_dir () =
  let env = load_state saved_state_dir in
  let _, loop_output = Test.change_files env added_member in
  assert_equals 21 loop_output.total_rechecked_count @@
    "Adding a new member to an enum after init should cause us " ^
    "to recheck only its AllMembers dependents"


let test_master_change saved_state_dir () =
  let env = load_state saved_state_dir ~master_changes:added_member in

  (* File that changed *)
  Test.assert_needs_recheck env "my_enum.php";
  (* Dependents of master change *)
  Test.assert_needs_no_recheck env @@ enum_user_name 1;
  Test.assert_needs_no_recheck env @@ enum_switch_name 1;

  let env, total_rechecked_count = Test.start_initial_full_check env in
  assert_equals 1 total_rechecked_count @@
    "Adding a new member to an enum in a master change should not cause us " ^
    "to recheck any of its dependents";

  Test.assert_needs_no_recheck env "my_enum.php";
  Test.assert_needs_no_recheck env @@ enum_user_name 1;
  Test.assert_needs_no_recheck env @@ enum_switch_name 1;

  let env, loop_output = Test.(run_loop_once env default_loop_input) in
  assert_equals 0 loop_output.total_rechecked_count @@
    "Master change and dependents should have been checked already in " ^
    "prechecked files handling";
  Test.assert_no_errors env


let test_local_change saved_state_dir () =
  let env = load_state saved_state_dir ~local_changes:added_member in

  (* File that changed *)
  Test.assert_needs_recheck env "my_enum.php";
  (* AllMembers dependents of local change *)
  Test.assert_needs_recheck env @@ enum_switch_name 1;
  (* Other dependents of local change *)
  Test.assert_needs_no_recheck env @@ enum_user_name 1;

  let env, total_rechecked_count = Test.start_initial_full_check env in
  assert_equals 21 total_rechecked_count @@
    "Adding a new member to an enum in a local change should cause us " ^
    "to recheck only its AllMembers dependents";

  Test.assert_needs_no_recheck env "my_enum.php";
  Test.assert_needs_no_recheck env @@ enum_switch_name 1;
  Test.assert_needs_no_recheck env @@ enum_user_name 1;

  let env, loop_output = Test.(run_loop_once env default_loop_input) in
  assert_equals 0 loop_output.total_rechecked_count @@
    "Local change and dependents should have been checked already in " ^
    "prechecked files handling";
  Test.assert_no_errors env


let test_master_change_with_locally_changed_dependent saved_state_dir () =
  let env = load_state saved_state_dir
    ~master_changes:added_member
    ~local_changes:[enum_switch_name 2, "<?hh // strict
      function enum_switch_2(MyEnum $x) : void { switch($x) {} }"] in

  (* Files that changed *)
  Test.assert_needs_recheck env "my_enum.php";
  Test.assert_needs_recheck env @@ enum_switch_name 2;
  (* Dependents of master change *)
  Test.assert_needs_no_recheck env @@ enum_user_name 1;
  Test.assert_needs_no_recheck env @@ enum_switch_name 1;

  let env, total_rechecked_count = Test.start_initial_full_check env in
  assert_equals 2 total_rechecked_count @@
    "Adding a new member to an enum in a master change should not cause us " ^
    "to recheck any of its dependents which are unchanged locally";

  Test.assert_needs_no_recheck env "my_enum.php";
  Test.assert_needs_no_recheck env @@ enum_user_name 1;
  Test.assert_needs_no_recheck env @@ enum_switch_name 1;
  Test.assert_needs_no_recheck env @@ enum_switch_name 2;

  let env, loop_output = Test.(run_loop_once env default_loop_input) in
  assert_equals 0 loop_output.total_rechecked_count
    "All changes should have been checked already in prechecked files handling";
  Test.assert_env_errors env @@
    "File \"/enum_switch_2.php\", line 2, characters 57-58:\n" ^
    "Switch statement nonexhaustive; the following cases are missing: B, A (Typing[4019])\n" ^
    "File \"/my_enum.php\", line 2, characters 6-11:\n" ^
    "Enum declared here"


let () = Tempfile.with_real_tempdir @@ fun temp_dir ->
  let temp_dir = Path.to_string temp_dir in
  save_state temp_dir;

  Test.in_daemon @@ test_change_after_init_no_loaded_decls temp_dir;
  Test.in_daemon @@ test_change_after_init temp_dir;

  Test.in_daemon @@ test_master_change temp_dir;
  Test.in_daemon @@ test_local_change temp_dir;
  Test.in_daemon @@ test_master_change_with_locally_changed_dependent temp_dir;
  ()
