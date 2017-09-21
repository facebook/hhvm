let with_temp_dir f = fun () ->
  Tempfile.with_tempdir f

let write_file ~dir ~file ~contents =
  let file = Path.concat dir file in
  Sys_utils.write_file ~file:(Path.to_string file) contents

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

let tests = [
  ("test_write_and_read", with_temp_dir test_write_and_read);
  ("test_is_directory_with_dot", with_temp_dir test_is_directory_with_dot);
  ("test_mkdir_p", with_temp_dir test_mkdir_p);
  ("test_write_needs_directory_tree", with_temp_dir test_write_needs_directory_tree);
]

let () =
  Unit_test.run_all tests
