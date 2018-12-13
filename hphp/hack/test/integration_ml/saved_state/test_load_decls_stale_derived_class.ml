open Core_kernel
open Asserter.Int_asserter
open Integration_test_base_types

module Test = Integration_test_base

(* Dependencies:

  Base -> Child, base_bar_user, base_foo_user
  Child -> child_bar_user, child_foo_user
  Cstr Base -> Child
  Extends Base -> Child
  Base::bar -> base_bar_user, child_bar_user
  Base::foo -> base_foo_user, child_foo_user
  Child::bar -> child_bar_user
  Child::foo -> child_foo_user
*)

let base_contents = Printf.sprintf "<?hh // strict
class Base {
  %s function foo(): int { return 0; }
  public function bar(): int { return 0; }
}"

let base_foo_user_contents = "<?hh // strict
function base_foo_user(Base $b): int {
  return $b->foo();
}"

let base_bar_user_contents = "<?hh // strict
function base_bar_user(Base $b): int {
  return $b->bar();
}"

let child_contents = "<?hh // strict
class Child extends Base {}"

let child_foo_user_contents = Printf.sprintf "<?hh // strict
function child_foo_user(Child $c): int {
  return $c->foo()%s;
}"

let child_bar_user_contents = "<?hh // strict
function child_bar_user(Child $c): int {
  return $c->bar();
}"

let init_disk_state =
  [ "base.php", base_contents "public"
  ; "base_foo_user.php", base_foo_user_contents
  ; "base_bar_user.php", base_bar_user_contents
  ; "child.php", child_contents
  ; "child_foo_user.php", child_foo_user_contents ""
  ; "child_bar_user.php", child_bar_user_contents
  ]

let hot_classes_file hot_classes =
  "hack/hh_hot_classes.json",
  hot_classes
  |> List.map ~f:(Printf.sprintf {|"\\%s"|})
  |> String.concat ~sep:", "
  |> Printf.sprintf {|{"classes":[ %s ]}|}

let hot_classes : string list ref = ref []

let make_foo_private = ["base.php", base_contents "private"]
let child_foo_user_plus_one =
  ["child_foo_user.php", child_foo_user_contents " + 1"]

let base_foo_user_error = {|
File "/base_foo_user.php", line 3, characters 14-16:
You cannot access this member (Typing[4112])
File "/base.php", line 3, characters 20-22:
This member is private
|}

let child_foo_user_error = {|
File "/child_foo_user.php", line 3, characters 14-16:
Could not find method foo in an object of type Child (Typing[4053])
File "/child_foo_user.php", line 2, characters 25-29:
This is why I think it is an object of type Child
File "/child.php", line 2, characters 7-11:
Declaration of Child is here
|}

let save_state saved_state_dir =
  let disk_state = hot_classes_file !hot_classes :: init_disk_state in
  Test.save_state disk_state saved_state_dir
    ~store_decls_in_saved_state:true

let load_state
    ?(master_changes=[])
    ?(local_changes=[])
    saved_state_dir =
  let disk_state = hot_classes_file !hot_classes :: init_disk_state in
  Test.load_state saved_state_dir
    ~disk_state:(disk_state @ master_changes @ local_changes)
    ~master_changes:(List.map master_changes fst)
    ~local_changes:(List.map local_changes fst)
    ~use_precheked_files:true
    ~disable_conservative_redecl:true
    ~load_decls_from_saved_state:true


let test_change_after_init saved_state_dir () =
  assert (!hot_classes = ["Base"]);
  let env = load_state saved_state_dir in
  Test.assert_no_errors env;
  let env, loop_output = Test.change_files env make_foo_private in
  Test.assert_env_errors env @@ base_foo_user_error ^ child_foo_user_error;
  (* Because we don't have a loaded declaration for Child, we must check all its
     dependents instead of only the dependents of Child::foo *)
  assert_equals 5 loop_output.total_rechecked_count
    "Only dependents of Base::foo and Child should be rechecked"


let test_change_after_init_with_hot_child saved_state_dir () =
  assert (!hot_classes = ["Base"; "Child"]);
  let env = load_state saved_state_dir in
  Test.assert_no_errors env;
  let env, loop_output = Test.change_files env make_foo_private in
  Test.assert_env_errors env @@ base_foo_user_error ^ child_foo_user_error;
  (* Loading both Base and Child means we don't need to recheck base_bar_user or
     child_bar_user *)
  assert_equals 4 loop_output.total_rechecked_count
    "Only dependents of Base::foo and Child::foo should be rechecked"


let test_local_change saved_state_dir () =
  assert (!hot_classes = ["Base"]);
  let env = load_state saved_state_dir ~local_changes:make_foo_private in

  (* File that changed *)
  Test.assert_needs_recheck env "base.php";
  (* Dependent of local change *)
  Test.assert_needs_recheck env "base_foo_user.php";
  (* Redeclared due to local change *)
  Test.assert_needs_recheck env "child.php";
  (* Coarse-granularity dependents of redeclared Child *)
  Test.assert_needs_recheck env "child_foo_user.php";
  Test.assert_needs_recheck env "child_bar_user.php";
  (* Dependent of local change which we determined needed no recheck because we
     loaded the old Base declaration *)
  Test.assert_needs_no_recheck env "base_bar_user.php";

  let env, total_rechecked_count = Test.start_initial_full_check env in
  assert_equals 5 total_rechecked_count
    "All files except base_bar_user should be rechecked";

  Test.assert_needs_no_recheck env "base.php";
  Test.assert_needs_no_recheck env "base_foo_user.php";
  Test.assert_needs_no_recheck env "base_bar_user.php";
  Test.assert_needs_no_recheck env "child.php";
  Test.assert_needs_no_recheck env "child_foo_user.php";
  Test.assert_needs_no_recheck env "child_bar_user.php";

  let env, loop_output = Test.(run_loop_once env default_loop_input) in
  assert_equals 0 loop_output.total_rechecked_count
    "All changes should have been checked already";
  Test.assert_env_errors env @@ base_foo_user_error ^ child_foo_user_error


let test_local_change_with_hot_child saved_state_dir () =
  assert (!hot_classes = ["Base"; "Child"]);
  let env = load_state saved_state_dir ~local_changes:make_foo_private in

  (* File that changed *)
  Test.assert_needs_recheck env "base.php";
  (* Dependent of local change *)
  Test.assert_needs_recheck env "base_foo_user.php";
  (* Redeclared due to local change *)
  Test.assert_needs_recheck env "child.php";
  (* Fine-granularity dependents of redeclared Child *)
  Test.assert_needs_recheck env "child_foo_user.php";
  (* Dependent of local change which we determined needed no recheck because we
     loaded the old Base declaration *)
  Test.assert_needs_no_recheck env "base_bar_user.php";
  (* If we were cleverer, we could probably avoid rechecking this. The reason we
     recheck it is because we recheck all deps of any class which was loaded
     from saved state and then redeclared. We could avoid doing this by
     performing two-phase redecl in ServerLazyInit (as we do in
     ServerTypeCheck), but it's probably better not to invest the effort at this
     time (since we are about to begin implementing per-file-decl, which should
     eliminate the need for two-phase redecl in both cases). *)
  Test.assert_needs_recheck env "child_bar_user.php";

  let env, total_rechecked_count = Test.start_initial_full_check env in
  assert_equals 5 total_rechecked_count
    "All files except base_bar_user should be rechecked";

  Test.assert_needs_no_recheck env "base.php";
  Test.assert_needs_no_recheck env "base_foo_user.php";
  Test.assert_needs_no_recheck env "base_bar_user.php";
  Test.assert_needs_no_recheck env "child.php";
  Test.assert_needs_no_recheck env "child_foo_user.php";
  Test.assert_needs_no_recheck env "child_bar_user.php";

  let env, loop_output = Test.(run_loop_once env default_loop_input) in
  assert_equals 0 loop_output.total_rechecked_count
    "All changes should have been checked already";
  Test.assert_env_errors env @@ base_foo_user_error ^ child_foo_user_error


(* If we fail to invalidate Base's declaration when there is a prechecked change
   to it, then Child will inherit its stale declaration. *)
let test_master_change saved_state_dir () =
  assert (!hot_classes = ["Base"]);
  let env = load_state saved_state_dir ~master_changes:make_foo_private in

  (* File that changed *)
  Test.assert_needs_recheck env "base.php";
  (* Dependents of master change *)
  Test.assert_needs_no_recheck env "base_foo_user.php";
  Test.assert_needs_no_recheck env "base_bar_user.php";
  Test.assert_needs_no_recheck env "child.php";
  Test.assert_needs_no_recheck env "child_foo_user.php";
  Test.assert_needs_no_recheck env "child_bar_user.php";

  let env, total_rechecked_count = Test.start_initial_full_check env in
  assert_equals 1 total_rechecked_count "Only base.php should be rechecked";

  Test.assert_needs_no_recheck env "base.php";
  Test.assert_needs_no_recheck env "base_foo_user.php";
  Test.assert_needs_no_recheck env "base_bar_user.php";
  Test.assert_needs_no_recheck env "child.php";
  Test.assert_needs_no_recheck env "child_foo_user.php";
  Test.assert_needs_no_recheck env "child_bar_user.php";

  let env, loop_output = Test.(run_loop_once env default_loop_input) in
  assert_equals 0 loop_output.total_rechecked_count
    "All changes should have been checked already";
  Test.assert_no_errors env;

  (* Modify child_foo_user so that we will recheck it. We should now see the
     visibility error in child_foo_user, but not the error in base_foo_user.
     If we failed to invalidate Base's declaration, the symptom of Child
     inheriting its stale declaration would be that we would not see the
     visibility error in child_foo_user. *)
  let env, loop_output = Test.change_files env child_foo_user_plus_one in
  assert_equals 1 loop_output.total_rechecked_count
    "Only child_foo_user.php should be rechecked";
  Test.assert_env_errors env child_foo_user_error;

  (* Make foo public again so that we will redeclare Base. If we have an
     oldified declaration of Base still sitting around, this will cause an
     assertion failure when we try to oldify the current declaration. *)
  let env, loop_output = Test.change_files env ["base.php", base_contents "public"] in
  assert_equals 6 loop_output.total_rechecked_count @@
    "Prechecked files intersection with master deps causes us to recheck all " ^
    "files rather than just the dependents of Base::foo and Child::foo";
  Test.assert_no_errors env


let test_master_change_with_hot_child saved_state_dir () =
  assert (!hot_classes = ["Base"; "Child"]);
  let env = load_state saved_state_dir ~master_changes:make_foo_private in

  (* File that changed *)
  Test.assert_needs_recheck env "base.php";
  (* Dependents of master change *)
  Test.assert_needs_no_recheck env "base_foo_user.php";
  Test.assert_needs_no_recheck env "base_bar_user.php";
  Test.assert_needs_no_recheck env "child.php";
  Test.assert_needs_no_recheck env "child_foo_user.php";
  Test.assert_needs_no_recheck env "child_bar_user.php";

  let env, total_rechecked_count = Test.start_initial_full_check env in
  assert_equals 1 total_rechecked_count "Only base.php should be rechecked";

  Test.assert_needs_no_recheck env "base.php";
  Test.assert_needs_no_recheck env "base_foo_user.php";
  Test.assert_needs_no_recheck env "base_bar_user.php";
  Test.assert_needs_no_recheck env "child.php";
  Test.assert_needs_no_recheck env "child_foo_user.php";
  Test.assert_needs_no_recheck env "child_bar_user.php";

  let env, loop_output = Test.(run_loop_once env default_loop_input) in
  assert_equals 0 loop_output.total_rechecked_count
    "All changes should have been checked already";
  Test.assert_no_errors env;

  (* Modify child_foo_user so that we will recheck it. We should now see the
     visibility error in child_foo_user, but not the error in base_foo_user.
     If we failed to invalidate the declarations of Base or Child, the symptom
     of using Child's stale declaration or Child inheriting Base's stale
     declaration would be that we would not see the visibility error in
     child_foo_user. We only recheck one file here because Child is redeclared
     lazily as we recheck child_foo_user. *)
  let env, loop_output = Test.change_files env child_foo_user_plus_one in
  assert_equals 1 loop_output.total_rechecked_count
    "Only child_foo_user.php should be rechecked";
  Test.assert_env_errors env child_foo_user_error


(* If a new class has been added to the hot_classes list, we should store it in
   an incremental saved state even though it won't be present in the decl heap
   ahead of time. *)
let save_state_incremental existing_state_dir incremental_state_dir () =
  assert (!hot_classes = ["Base"]);
  hot_classes := ["Base"; "Child"];
  let env = load_state existing_state_dir in

  (* Base was loaded from the saved state... *)
  assert (Decl_heap.Classes.mem "\\Base");
  assert (Decl_heap.Methods.mem ("\\Base", "foo"));
  (* ...but Child was not. It will be declared when we save a new state. *)
  assert (not @@ Decl_heap.Classes.mem "\\Child");
  assert (not @@ Decl_heap.Methods.mem ("\\Child", "foo"));

  let env, total_rechecked_count = Test.start_initial_full_check env in
  assert_equals 0 total_rechecked_count "No files should be rechecked";

  let env, loop_output = Test.(run_loop_once env default_loop_input) in
  assert_equals 0 loop_output.total_rechecked_count
    "No files should be rechecked";

  let _edges_added = Test.save_state_incremental
    env
    incremental_state_dir
    ~store_decls_in_saved_state:true in ();
  (* Saving the state declared Child as a side effect. *)
  assert (Decl_heap.Classes.mem "\\Child")


(* If we failed to save the declaration of Child in save_state_incremental, then
   we will recheck the wrong number of files here. *)
let test_incremental_state saved_state_dir () =
  let env = load_state saved_state_dir in

  (* Both Base and Child were loaded from the saved state... *)
  assert (Decl_heap.Classes.mem "\\Base");
  assert (Decl_heap.Methods.mem ("\\Base", "foo"));
  assert (Decl_heap.Classes.mem "\\Child");
  (* ...and Child::foo shares Base::foo's type in the Methods heap. *)
  assert (not @@ Decl_heap.Methods.mem ("\\Child", "foo"));

  let env, loop_output = Test.change_files env make_foo_private in
  Test.assert_env_errors env @@ base_foo_user_error ^ child_foo_user_error;
  assert_equals 4 loop_output.total_rechecked_count
    "Only dependents of Base::foo and Child::foo should be rechecked"


(* On CI machines, we shouldn't end up in the situation that this test case
   exercises. Since we don't use prechecked files on CI machines, we should
   never be missing the declaration of a hot class (in this case, Child) when we
   attempt to save the state. This can happen when responding to the SAVE_STATE
   ServerRpc command, though, so we need to ensure that Decl_export will declare
   the class if necessary. *)
let save_state_incremental_hot_child existing_state_dir incremental_state_dir () =
  assert (!hot_classes = ["Base"; "Child"]);
  (* Loading with Base modified in a master change will cause us to oldify Base,
     and when we do so, we entirely remove the declaration of Child. When saving
     the state, we should notice that Child is missing and redeclare it rather
     than silently continuing without it. *)
  let env = load_state existing_state_dir
    ~master_changes:["base.php", base_contents "final public"] in

  (* Base's declaration was loaded from the saved state. Because Base was in a
     dirty file, its declaration was oldified. Because it wasn't modified in a
     local change and we are using prechecked files, it was not redeclared by
     redo_type_decl. Then its oldified declaration was deleted. *)
  assert (not @@ Decl_heap.Classes.mem "\\Base");
  assert (not @@ Decl_heap.Methods.mem ("\\Base", "foo"));
  (* Child was loaded from the saved state, but deleted when we oldified Base *)
  assert (not @@ Decl_heap.Classes.mem "\\Child");
  (* Child::foo never existed in the Methods heap *)
  assert (not @@ Decl_heap.Methods.mem ("\\Child", "foo"));

  let env, total_rechecked_count = Test.start_initial_full_check env in
  assert_equals 1 total_rechecked_count "Only base.php should be rechecked";

  let env, loop_output = Test.(run_loop_once env default_loop_input) in
  assert_equals 0 loop_output.total_rechecked_count
    "All changes should have been checked already";

  let _edges_added = Test.save_state_incremental
    env
    incremental_state_dir
    ~store_decls_in_saved_state:true in ()


let hot_base_tests () = Tempfile.with_real_tempdir @@ fun temp_dir ->
  hot_classes := ["Base"];
  let temp_dir = Path.to_string temp_dir in
  save_state temp_dir;

  Test.in_daemon @@ test_change_after_init temp_dir;
  Test.in_daemon @@ test_local_change temp_dir;
  Test.in_daemon @@ test_master_change temp_dir;

  Tempfile.with_real_tempdir @@ fun incremental_state_dir ->
    let incremental_state_dir = Path.to_string incremental_state_dir in
    Test.in_daemon @@ save_state_incremental temp_dir incremental_state_dir;
    Test.in_daemon @@ test_incremental_state incremental_state_dir;
    ()

let hot_base_and_child_tests () = Tempfile.with_real_tempdir @@ fun temp_dir ->
  hot_classes := ["Base"; "Child"];
  let temp_dir = Path.to_string temp_dir in
  save_state temp_dir;

  Test.in_daemon @@ test_change_after_init_with_hot_child temp_dir;
  Test.in_daemon @@ test_local_change_with_hot_child temp_dir;
  Test.in_daemon @@ test_master_change_with_hot_child temp_dir;

  Tempfile.with_real_tempdir @@ fun incremental_state_dir ->
    let incremental_state_dir = Path.to_string incremental_state_dir in
    Test.in_daemon @@ save_state_incremental_hot_child temp_dir incremental_state_dir;
    Test.in_daemon @@ test_incremental_state incremental_state_dir;
    ()

let () =
  hot_base_tests ();
  hot_base_and_child_tests ();
  ()
