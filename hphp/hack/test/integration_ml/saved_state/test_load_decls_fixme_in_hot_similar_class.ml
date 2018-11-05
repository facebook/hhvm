open Core_kernel
open Asserter.Int_asserter

module Test = Integration_test_base

let ifoo_contents = "<?hh // strict
interface IFoo {
  public function foo(): int;
}"

let hot_contents = Printf.sprintf "<?hh // strict
class Hot implements IFoo {%s
  /* HH_FIXME[4041] */
  public function foo(int $x): int { return $x; }
}"

let init_disk_state =
  [ "hack/hh_hot_classes.json", {|{"classes":["\\Hot"]}|}
  ; "ifoo.php", ifoo_contents
  ; "hot.php", hot_contents ""
  ]

(* If we fail to oldify declarations in similar files, the position of foo in
   the loaded declaration for Hot will have a different line number than the
   line the HH_FIXME suppresses. We need to make sure the loaded declaration is
   oldified even though the decl AST hash has not changed. *)

let () = Tempfile.with_real_tempdir @@ fun temp_dir ->
  let temp_dir = Path.to_string temp_dir in
  Test.save_state init_disk_state temp_dir ~store_decls_in_saved_state:true;
  Test.in_daemon @@ fun () ->
    let env =
      Test.load_state temp_dir
        ~disk_state:(init_disk_state @ ["hot.php", hot_contents "\n"])
        ~master_changes:["hot.php"]
        ~use_precheked_files:true
        ~load_decls_from_saved_state:true
    in
    (* We should have noticed that Hot was declared in a similar file and
       therefore oldified the declaration of Hot before redo_type_decl. Then we
       remove all oldified declarations, so it should not be present at all. *)
    assert (not @@ Decl_heap.Classes.mem "\\Hot");
    assert (not @@ Decl_heap.Classes.mem_old "\\Hot");
    Test.assert_needs_recheck env "hot.php";
    Test.assert_needs_no_recheck env "ifoo.php";
    let env, total_rechecked_count = Test.start_initial_full_check env in
    assert_equals 1 total_rechecked_count "Only hot.php should be rechecked";
    Test.assert_needs_no_recheck env "hot.php";
    Test.assert_needs_no_recheck env "ifoo.php";
    Test.assert_no_errors env
