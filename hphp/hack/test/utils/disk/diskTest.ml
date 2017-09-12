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

let tests = [
  ("test_write_and_read", with_temp_dir test_write_and_read);
]

let () =
  Unit_test.run_all tests
