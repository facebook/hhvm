(*
When a file generates errors in the decl phase, we make the very
conservative assumption that we know nothing about its dependencies.
So we redeclare it every time a file changes, even if the changed file
is wholly unrelated to the contents of the erroneous file.

I can't actually think of any example where this is necessary, but
until we have a good argument for why it isn't, it's probably
reasonable to ensure this behavior is not accidentally broken.
*)
open Integration_test_base_types
open ServerEnv

module Test = Integration_test_base

let foo_contents = "<?hh
class Foo {
  <<__Override>>
  public function f() {}
}
"

let baz_contents = "<?hh
class Baz extends Foo {

}
"

let qux_contents = "<?hh
class Qux {

}
"

let () =
  let env = Test.setup_server () in
  let env, loop_output = Test.(run_loop_once env { default_loop_input with
    disk_changes = [
      "foo.php", foo_contents;
      "baz.php", baz_contents;
    ];
  }) in

  if not loop_output.did_read_disk_changes then
    Test.fail "Expected the server to process disk updates";
  let expected_error =
    "File \"/foo.php\", line 4, characters 19-19:\n" ^
    "Foo::f() is marked as override; no non-private parent definition found " ^
    "or overridden parent is defined in non-<?hh code (Typing[4087])\n" in
  Test.assertSingleError expected_error (Errors.get_error_list env.errorl);

  (* Now let's edit a wholly unrelated file *)
  let env, loop_output = Test.(run_loop_once env { default_loop_input with
    disk_changes = [
      "qux.php", qux_contents;
    ];
  }) in
  if not loop_output.did_read_disk_changes then
    Test.fail "Expected the server to process disk updates";
  let path = Relative_path.create Relative_path.Root "/foo.php" in
  let failed_decl = Errors.(get_failed_files env.errorl Decl) in
  if Relative_path.Set.mem failed_decl path then ()
    else Test.fail "Foo should still have failed declaration"
