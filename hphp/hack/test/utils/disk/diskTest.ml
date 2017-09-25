open Test_disk_utils

let with_temp_dir f = fun () ->
  Tempfile.with_tempdir f

let verify_contents_equal ~dir ~file ~expected =
  let file = Path.concat dir file in
  let contents = Sys_utils.cat (Path.to_string file) in
  Asserter.String_asserter.assert_equals expected contents "verifying disk contents";
  true

let test_write_and_read dir =
  write_file ~dir ~file:"a.txt" ~contents:"hello";
  verify_contents_equal ~dir ~file:"a.txt" ~expected:"hello"

(** We can append a "." to the end of a directory and it should exist. *)
let test_is_directory_with_dot dir =
  let dir = Path.to_string dir in
  Asserter.Bool_asserter.assert_equals true
    (Disk.is_directory dir)
    "directory should not exist at start";
  Asserter.Bool_asserter.assert_equals true
    (Disk.is_directory (dir ^ "/."))
    "Appending a dot represents same directory, should exist";
  true

let test_mkdir_p dir =
  let dir = Path.concat dir "some/path/to/leaf_dir" in
  Asserter.Bool_asserter.assert_equals false
    (Disk.is_directory (Path.to_string dir))
    "directory should not exist at start";
  let () = Sys_utils.mkdir_p (Path.to_string dir) in
  Asserter.Bool_asserter.assert_equals true
    (Disk.is_directory (Path.to_string dir))
    "directory should exist at the end";
  true

(** Writing a file requires all its parent directories to exist first. *)
let test_write_needs_directory_tree dir =
  let dir = Path.concat dir "some/parent/dirs" in
  let basename = "sample.txt" in
  Asserter.Bool_asserter.assert_equals false
    (Disk.is_directory (Path.to_string dir))
    "Directory should not exist at start";
  try begin
    write_file ~dir ~file:basename ~contents:"hello";
    Printf.eprintf "Error: Expected exception didn't throw\n";
    false
  end with
  | Disk_sig.Types.No_such_file_or_directory _ ->
    Sys_utils.mkdir_p (Path.to_string dir);
    write_file ~dir ~file:basename ~contents:"hello";
    Asserter.Bool_asserter.assert_equals true
      (Disk.is_directory (Path.to_string dir))
      "Directory should exist after writing file";
    verify_contents_equal ~dir ~file:basename ~expected:"hello"

let test_rename_basic dir =
  write_file ~dir ~file:"a.txt" ~contents:"hello";
  Disk.rename (Path.to_string (Path.concat dir "a.txt"))
    (Path.to_string (Path.concat dir "b.txt"));
  ignore @@ verify_contents_equal ~dir ~file:"b.txt" ~expected:"hello";
  let old_exists = Disk.file_exists (Path.to_string (Path.concat dir "a.txt")) in
  Asserter.Bool_asserter.assert_equals false old_exists
    "Old file should no longer exist";
  true

(** The target is a directory, but the parent directories leading
 * to the target path don't exist yet. *)
let test_rename_parents_dont_exist dir =
  write_file ~dir ~file:"a.txt" ~contents:"hello";
  let path = Path.concat dir "a.txt" in
  let target = Path.concat dir "some/path/doesnt/exist/b.txt" in
  Disk.rename (Path.to_string path) (Path.to_string target)

let verify_dir dir files =
  let is_dir = Disk.is_directory (Path.to_string dir) in
  Asserter.Bool_asserter.assert_equals true is_dir
    (Printf.sprintf "%s should be a directory" (Path.to_string dir));
  List.iter (
    fun (file, expected) ->
      ignore @@ verify_contents_equal ~dir ~file ~expected
    )
    files

let test_rename_dir_but_target_not_empty dir =
  let target = Path.concat dir "some/path/exists" in
  let old = Path.concat dir "old" in
  setup_dir old [("a.txt", "hello"); ("b.txt", "world")];
  setup_dir target [("oops.txt", "dont overwrite me");];
  Disk.rename (Path.to_string old) (Path.to_string target);
  true

(** The target is a directory, and is empty, and the parent directories
 * leading to the target directory exist. *)
let test_rename_target_is_dir dir =
  let target = Path.concat dir "some/path/exists/to/here" in
  setup_dir target [];
  let src = Path.concat dir "testing" in
  let files = [("abc.txt", "hello"); ("foo.txt", "world")] in
  setup_dir src files;
  (** write_file ~dir ~file:"a.txt" ~contents:"hello"; *)
  Disk.rename (Path.to_string src) (Path.to_string target);
  verify_dir target files;
  true

let tests = [
  ("test_write_and_read", with_temp_dir test_write_and_read);
  ("test_is_directory_with_dot", with_temp_dir test_is_directory_with_dot);
  ("test_mkdir_p", with_temp_dir test_mkdir_p);
  ("test_write_needs_directory_tree", with_temp_dir test_write_needs_directory_tree);
  ("test_rename_basic", with_temp_dir test_rename_basic);
  ("test_rename_parents_dont_exist", with_temp_dir (fun tmp_dir ->
    let expected_err = Printf.sprintf "%s/%s"
      (Path.to_string tmp_dir) "some/path/doesnt/exist" in
    let ex = Disk_sig.Types.No_such_file_or_directory expected_err in
    Unit_test.expect_throws ex test_rename_parents_dont_exist tmp_dir));
  ("test_rename_dir_but_target_not_empty", (with_temp_dir (fun tmp_dir ->
    let ex = (Disk_sig.Types.Rename_target_dir_not_empty (Path.to_string
        (Path.concat tmp_dir "some/path/exists"))) in
    Unit_test.expect_throws ex
      test_rename_dir_but_target_not_empty tmp_dir)));
  ("test_rename_target_is_dir", with_temp_dir test_rename_target_is_dir);
]

let () =
  Unit_test.run_all tests
