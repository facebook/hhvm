open Core_kernel
open Asserter.Int_asserter

module Test = Integration_test_base

let ifoo_contents = "<?hh // strict
class Thing<T> {
  public function __construct(private T $x) {}
}
interface IFoo {
  /* HH_FIXME[4101] */
  public function foo(): Thing;
}"

let my_foo_contents = Printf.sprintf "<?hh // strict
class MyFoo implements IFoo {%s
  public function foo(): Thing<int> { return new Thing(3); }
}"

let init_disk_state =
  (* We only really need IFoo to be hot to reproduce the problem, but any
     cold dependency of MyFoo in the same file as IFoo will cause the issue to
     disappear, since we will need to re-parse the file in order to declare the
     cold dependency. *)
  [ "hack/hh_hot_classes.json", {|{"classes":["\\IFoo", "\\Thing"]}|}
  ; "ifoo.php", ifoo_contents
  ; "my_foo.php", my_foo_contents ""
  ]

(* When we recheck MyFoo, no declaration will have been loaded from the saved
   state, so we will redeclare it. The error 4101 suppression comment in the hot
   interface IFoo normally suppresses the compatibility error we would emit due
   to MyFoo's version of foo returning a Thing with more type arguments, but
   if we never parse ifoo.php and don't store HH_FIXMEs in the saved state, this
   suppression comment won't be accounted for in the HH_FIXMES or DECL_HH_FIXMES
   heaps. Storing HH_FIXMES and DECL_HH_FIXMES in the saved state ensures that
   this situation cannot arise. *)

let () = Tempfile.with_real_tempdir @@ fun temp_dir ->
  let temp_dir = Path.to_string temp_dir in
  Test.save_state init_disk_state temp_dir ~store_decls_in_saved_state:true;
  Test.in_daemon @@ fun () ->
    let env =
      Test.load_state temp_dir
        ~disk_state:(init_disk_state @ ["my_foo.php", my_foo_contents "\n"])
        ~local_changes:["my_foo.php"]
        ~use_precheked_files:true
        ~load_decls_from_saved_state:true
    in
    Test.assert_needs_recheck env "my_foo.php";
    Test.assert_needs_no_recheck env "ifoo.php";
    let env, total_rechecked_count = Test.start_initial_full_check env in
    assert_equals 1 total_rechecked_count "Only my_foo should be rechecked";
    Test.assert_needs_no_recheck env "ifoo.php";
    Test.assert_needs_no_recheck env "my_foo.php";
    Test.assert_no_errors env
