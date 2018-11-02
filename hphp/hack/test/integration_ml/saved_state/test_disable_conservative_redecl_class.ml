open Asserter.Int_asserter
open Integration_test_base_types

module Test = Integration_test_base

(*

We expect these dependencies (among others) to be stored in the saved state:

  Class Foo -> Class Bar
  Method Foo::get -> Class Bar
  Class Bar -> Class Qux
  Method Bar::get -> Class Qux

So when Foo is changed (and we have no old decl to compare against because we
have loaded from a saved state), and we have disabled the conservative redecl
behavior, we only redeclare classes which extend Foo (and there are none). We
must recheck both Foo and Bar, but we don't need to recheck Qux.

Without disable_conservative_redecl, we would redeclare all of Foo's typing
dependencies, including Bar. Since we redeclared Bar (with no old decl to
compare against), we would then also redeclare and recheck Qux.

*)

let foo_name = "foo.php"
let foo_contents ty = Printf.sprintf "<?hh // strict
class Foo {
  public function get(): %s { return 0; }
}
" ty

let bar_name = "bar.php"
let bar_contents = "<?hh // strict
class Bar {
  public function get(): num { return (new Foo())->get(); }
}
"

let qux_name = "qux.php"
let qux_contents = "<?hh // strict
class Qux {
  public function test(Bar $bar): num {
    return $bar->get();
  }
}
"

let init_disk_state = [
  foo_name, foo_contents "num";
  bar_name, bar_contents;
  qux_name, qux_contents;
]

let test_cases = [
  false, 3; (* Without disable flag, recheck Foo, Bar, Qux *)
  true, 2; (* With conservative redecl disabled, recheck only Foo and Bar *)
]

let run_test saved_state_dir test_case () =
  let disable_conservative_redecl, expected_rechecked = test_case in
  let env = Test.load_state saved_state_dir
    ~disk_state:init_disk_state
    ~disable_conservative_redecl
  in
  let env, loop_output = Test.change_files env [foo_name, foo_contents "int"] in
  Test.assert_no_errors env;

  assert_equals 1 loop_output.rechecked_count @@
    "Expected only Foo to be changed";
  assert_equals expected_rechecked loop_output.total_rechecked_count @@
    "Wrong number of total files rechecked"

let () =
  Tempfile.with_real_tempdir @@ fun temp_dir ->
    let saved_state_dir = Path.to_string temp_dir in
    Test.save_state init_disk_state saved_state_dir;
    test_cases |> List.iter @@ fun test_case ->
      Test.in_daemon @@ run_test saved_state_dir test_case
