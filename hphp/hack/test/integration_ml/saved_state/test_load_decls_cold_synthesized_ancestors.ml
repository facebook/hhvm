open Core_kernel
open Asserter.Int_asserter

module Test = Integration_test_base

let cfoo_contents = "<?hh // strict
abstract class CFoo {
  public abstract function foo(): int;
}"

let ibar_contents = "<?hh // strict
interface IBar {
  public function bar(): int;
}"

let hot_contents = "<?hh // strict
trait Hot {
  require extends CFoo;
  require implements IBar;
}"

let cold_contents = Printf.sprintf "<?hh // strict
trait Cold {
  use Hot;
  public function baz(): %s {
    return $this->foo() + $this->bar();
  }
}"

let init_disk_state =
  [ "hack/hh_hot_classes.json", {|{"classes":["\\Hot"]}|}
  ; "cfoo.php", cfoo_contents
  ; "ibar.php", ibar_contents
  ; "hot.php", hot_contents
  ; "cold.php", cold_contents "int"
  ]

let () = Tempfile.with_real_tempdir @@ fun temp_dir ->
  let temp_dir = Path.to_string temp_dir in
  Test.save_state init_disk_state temp_dir ~store_decls_in_saved_state:true;
  Test.in_daemon @@ fun () ->
    let env =
      Test.load_state temp_dir
        ~disk_state:(init_disk_state @ ["cold.php", cold_contents "num"])
        ~master_changes:["cold.php"]
        ~use_precheked_files:true
        ~load_decls_from_saved_state:true
    in
    assert (Decl_heap.Classes.mem "\\CFoo");
    assert (Decl_heap.Classes.mem "\\IBar");
    assert (Decl_heap.Classes.mem "\\Hot");
    assert (not @@ Decl_heap.Classes.mem "\\Cold");
    Test.assert_needs_no_recheck env "cfoo.php";
    Test.assert_needs_no_recheck env "ibar.php";
    Test.assert_needs_no_recheck env "hot.php";
    Test.assert_needs_recheck env "cold.php";
    let env, total_rechecked_count = Test.start_initial_full_check env in
    assert_equals 1 total_rechecked_count "Only cold.php should be rechecked";
    Test.assert_needs_no_recheck env "cfoo.php";
    Test.assert_needs_no_recheck env "ibar.php";
    Test.assert_needs_no_recheck env "hot.php";
    Test.assert_needs_no_recheck env "cold.php";
    Test.assert_no_errors env
