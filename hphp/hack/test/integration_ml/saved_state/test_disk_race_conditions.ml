open Core_kernel
module Test = Integration_test_base
open Integration_test_base_types

(* This "tests" an incorrect behavior of hh_server to expose/document a bug.
 * If we every fix hh_server's susceptibility to race conditions with disk,
 * it might be deleted / modified. *)

let foo_name = "foo.php"

let foo_contents = Printf.sprintf {|<?hh
function foo(): %s {
  // UNSAFE_EXPR
}
|}

let uses_foo_name = "uses_foo.php"

let uses_foo_contents = {|<?hh
function uses_foo(): mixed {
  return foo();
}
|}

let test_contents = {|<?hh
function test(): int {
  return foo();
}
|}

let () = Tempfile.with_real_tempdir @@ fun temp_dir ->
  let temp_dir = Path.to_string temp_dir in

  let disk_state = [
    foo_name, foo_contents "int";
    uses_foo_name, uses_foo_contents;
    "test.php", test_contents;
  ] in

  (* No changes between saving and loading state *)
  Test.save_state disk_state temp_dir;
  let env = Test.load_state
    temp_dir
    ~disk_state
    ~master_changes:[]
    ~local_changes:[]
    ~use_precheked_files:false
  in

  (* When using saved state, no declarations are in memory initially. *)
  assert (Option.is_none @@ Decl_heap.Funs.get "\\foo");

  let uses_foo_relative_path = Relative_path.(create Root (Test.prepend_root uses_foo_name)) in
  (* Change the file on disk without letting server know (yet).
   * We want to simulate a situation where we are in the middle of an operation
   * that will end up declaring "foo", but foo.php changes on disk before it
   * happens. *)
  TestDisk.set (Test.prepend_root foo_name) (foo_contents "string");
  (* We could just directly call Typing_lazy_heap.get_fun, but let's go thorugh
   * an actual server operation that will just happen to do it. *)
  let env = ServerEnv.{ env with
    needs_recheck = Relative_path.Set.add env.needs_recheck uses_foo_relative_path
  } in
  let env, _ = Test.full_check env in
  (* Verify that typechecking of uses_foo.php computed (as a side effect)
   * declaration of foo and stored it in shared memory. *)
  begin match Decl_heap.Funs.get "\\foo" with
    | Some f ->
      let f_ret = Typing_print.suggest f.Typing_defs.ft_ret in
      (* Notice that this declaration is "wrong" - it's based on file contents
       * that we didn't put through the right channels that would track if and how
       * things changed in that file.
       * This should still be "int" at this point! *)
      assert (f_ret = "string")
    | None -> assert false
  end;
  (* Only now server will notice the change to foo.php. During this operation,
   * it will compare new return type of foo ("string"), with what it thinks
   * is the old one (also "string", see above), and conclude that foo remains
   * unchanged. But it did actually change from int to string, and we should
   * have rechecked test because of that! *)
  let env, _ = Test.(run_loop_once env { default_loop_input with
    disk_changes = [foo_name, foo_contents "string"];
  }) in
  (* This should report errors in test.php! It does so if you comment out TestDisk.set line. *)
  Test.assert_no_errors env;
  ()
