open Core_kernel

module Test = Integration_test_base

(* The FIXME in Foo is necessary only because of the order of trait inclusion in
   TBothTraits. Normally, the 4110 error would be emitted at the incorrect
   override in Child, but TBothTraits inherits the version of f inherited by
   TReqExtFoo, which is the one defined in Foo. This causes us to emit the error
   at Foo's definition rather than Child's. *)

let foo = Printf.sprintf "<?hh // strict
class Foo {%s
  /* HH_FIXME[4110] */
  public function f(int $x): void {}
}"

let treq_ext_foo = "<?hh // strict
trait TReqExtFoo {
  require extends Foo;
}"

let child = "<?hh // strict
class Child extends Foo {
  /* HH_FIXME[4110] */
  public function f(string $x): void {}
}"

let treq_ext_child = "<?hh // strict
trait TReqExtChild {
  require extends Child;
}"

let tboth_traits = "<?hh // strict
trait TBothTraits {
  use TReqExtChild;
  use TReqExtFoo;
}"

let init_disk_state =
  [ "hack/hh_hot_classes.json", {|{"classes":["\\Foo"]}|}
  ; "foo.php", foo "\n  public function x(): void {}\n"
  ; "treq_ext_foo.php", treq_ext_foo
  ; "child.php", child
  ; "treq_ext_child.php", treq_ext_child
  ; "tboth_traits.php", tboth_traits
  ]

(* This test ensures that we remove any HH_FIXMEs we loaded from the saved state
   from shared memory before re-parsing any changed files. If we fail to do so,
   here is what happens. We load an HH_FIXMEs map from the saved state for
   foo.php, which indicates that Foo has a fixme for line 5 suppressing the
   error emitted at f. We then parse the decl AST of foo.php in ServerLazyInit.
   This adds a DECL_HH_FIXMEs map based on the newly parsed file, which
   indicates that Foo has a fixme for line 4 suppressing the error emitted at f
   (now that the line above defining x has been deleted), but does not modify
   the existing HH_FIXMEs map in shared memory. When there is a map in both the
   HH_FIXMEs heap and the DECL_HH_FIXMEs heap for the same file, we use the one
   in HH_FIXMEs. Thus we read stale data and fail to suppress the error. *)

let () = Tempfile.with_real_tempdir @@ fun temp_dir ->
  let temp_dir = Path.to_string temp_dir in
  Test.save_state init_disk_state temp_dir ~store_decls_in_saved_state:true;
  Test.in_daemon @@ fun () ->
    let env =
      Test.load_state temp_dir
        ~disk_state:(init_disk_state @ ["foo.php", foo ""])
        ~local_changes:["foo.php"]
        ~load_decls_from_saved_state:true
    in

    Test.assert_needs_recheck env "foo.php";
    Test.assert_needs_recheck env "treq_ext_foo.php";
    Test.assert_needs_recheck env "child.php";
    Test.assert_needs_recheck env "treq_ext_child.php";
    Test.assert_needs_recheck env "tboth_traits.php";

    let env, _ = Test.full_check env in

    Test.assert_needs_no_recheck env "foo.php";
    Test.assert_needs_no_recheck env "treq_ext_foo.php";
    Test.assert_needs_no_recheck env "child.php";
    Test.assert_needs_no_recheck env "treq_ext_child.php";
    Test.assert_needs_no_recheck env "tboth_traits.php";

    Test.assert_no_errors env
