open Core_kernel
open Integration_test_base_types

module Test = Integration_test_base

let enum_name = "my_enum.php"

let enum_contents = {|<?hh // partial
enum MyEnum : string {
  FIELD_1 = "field 1";
}
|}

let enum_contents_with_one_more_field =  {|<?hh // strict
enum MyEnum : string {
  FIELD_1 = "field 1"
  FIELD_2 = "field 2"
}
|}

let enum_user_name = Printf.sprintf "my_enum_user_%d.php"
let enum_user_contents = Printf.sprintf {|<?hh // strict
function enum_user_%d(): void {
  MyEnum::FIELD_1;
}
|}

let enum_users = List.init 10 (fun n -> enum_user_name n, enum_user_contents n)

let enum_switch_name = "enum_switch.php"

let enum_switch_contents = {|<?hh // strict
function enum_switch(MyEnum $x) : void {
  /* HH_FIXME[4019] non-exhaustive check */
  switch($x) {
  }
}
|}

let init_disk_contents = [
  enum_name, enum_contents;
  enum_switch_name, enum_switch_contents;
] @ enum_users

let save_state saved_state_dir =
  Test.save_state init_disk_contents saved_state_dir

let load_state saved_state_dir =
  Test.load_state
    saved_state_dir
    ~disk_state:init_disk_contents
    ~master_changes:[]
    ~local_changes:[]
    ~use_precheked_files:true

let test_ok saved_state_dir  () =
  let env = load_state saved_state_dir in
  (* "touch" (without changing) my_enum.php to ensure that it's declaration is
   * loaded in shared memory. Note that this will also recheck all the enum
   * users, but this is not what we test for here. *)
  let env, _ = Test.(run_loop_once env { default_loop_input with
    disk_changes = [enum_name, enum_contents];
  }) in
  (* Add a new member to enum and observe that thanks to previous version and
   * Typing_Deps.Dep.AllMembers logic we don't have to recheck all of the
   * users *)
  let env, loop_output = Test.(run_loop_once env { default_loop_input with
    disk_changes = [enum_name, enum_contents_with_one_more_field];
  }) in
  Asserter.Int_asserter.assert_equals 2 loop_output.total_rechecked_count
    "Adding a new member to an enum should not recheck all of it's users";
  ignore env;
  ()

(* Same test as test_ok, except we don't "warm up" declarations, so there
 * will be no previous version to compare against. *)
let test_fail saved_state_dir  () =
  let env = load_state saved_state_dir in
  let env, loop_output = Test.(run_loop_once env { default_loop_input with
    disk_changes = [enum_name, enum_contents_with_one_more_field];
  }) in
  Asserter.Int_asserter.assert_equals 12 loop_output.total_rechecked_count @@
    "Adding a new member to an enum should not recheck all of it's users" ^
    "(but it does, sometimes)";
  ignore env;
  ()

let () = Tempfile.with_real_tempdir @@ fun temp_dir ->
  let temp_dir = Path.to_string temp_dir in
  save_state temp_dir;

  Test.in_daemon @@ test_ok temp_dir;
  Test.in_daemon @@ test_fail temp_dir;
  ()
