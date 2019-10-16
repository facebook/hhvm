open Core_kernel
module Test = Integration_test_base

let base =
  "<?hh // partial
abstract class Base {
  private $foo;
  final public function getInt() {
    return '0';
  }
}
"

let base_with_typed_prop =
  "<?hh // partial
abstract class Base {
  private int $foo;
  final public function getInt() {
    return '0';
  }
}
"

let getint_returns_string =
  "<?hh // partial
abstract class Base {
  private $foo;
  final public function getInt(): string {
    return '0';
  }
}
"

let child = "<?hh // strict
class Child extends Base {}
"

let f = "<?hh // strict
function f(Child $c): int {
  return $c->getInt();
}
"

let init_disk_state = [("base.php", base); ("child.php", child); ("f.php", f)]

let expected_init_error =
  {|
File "/base.php", line 2, characters 16-19:
Lacking __construct, class Base does not initialize its private member(s): foo  (NastCheck[3030])

File "/child.php", line 2, characters 7-11:
Class Child does not initialize all of its members; foo is not always initialized.
Make sure you systematically set $this->foo when the method __construct is called.
Alternatively, you can define the member as optional (?...)
 (NastCheck[3015])
|}

let expected_type_error =
  {|
File "/f.php", line 3, characters 10-21:
Invalid return type (Typing[4110])
File "/f.php", line 2, characters 23-25:
Expected int
File "/base.php", line 4, characters 35-40:
But got string
|}

let test () =
  Tempfile.with_real_tempdir
  @@ fun temp_dir ->
  let temp_dir = Path.to_string temp_dir in
  Test.save_state init_disk_state temp_dir ~store_decls_in_saved_state:true;
  Test.in_daemon
  @@ fun () ->
  let env =
    Test.load_state
      temp_dir
      ~disk_state:init_disk_state
      ~disable_conservative_redecl:true
      ~load_decls_from_saved_state:true
      ~use_precheked_files:true
  in
  Test.assert_needs_no_recheck env "base.php";
  Test.assert_needs_no_recheck env "child.php";
  Test.assert_needs_no_recheck env "f.php";
  let (env, _) = Test.full_check env in
  Test.assert_no_errors env;
  let (env, _) = Test.change_files env [("base.php", base_with_typed_prop)] in
  Test.assert_env_errors env expected_init_error;
  let (env, _) = Test.change_files env [("base.php", getint_returns_string)] in
  Test.assert_env_errors env expected_type_error;
  ()
