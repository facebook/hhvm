module Test = Integration_test_base

let foo_contents = Printf.sprintf {|<?hh // strict

class %s {}
|}

let baz_contents = {|<?hh
class C {}
|}

let () = Tempfile.with_real_tempdir @@ fun temp_dir ->
  let temp_dir = Path.to_string temp_dir in

  let expected_error =
    "File \"/foo.php\", line 3, characters 7-7:\n" ^
    "Name already bound: C (Naming[2012])\n" ^
    "File \"/baz.php\", line 2, characters 7-7:\n" ^
    "Previous definition is here\n" in

  let foo_contents_with_error = foo_contents "C" in

  Test.save_state_with_errors [
    "foo.php", foo_contents_with_error;
    "baz.php", baz_contents;
  ] temp_dir expected_error;

  let env = Test.load_state
    temp_dir
    ~disk_state:[
      "foo.php", foo_contents_with_error;
      "baz.php", baz_contents;
    ]
    ~master_changes:[]
    ~local_changes:[]
    ~use_precheked_files:false
  in

  (* Trigger next full recheck to get global errors *)
  let env, _ = Test.full_check env in

  (* Make sure there's only the error we expect *)
  Test.assert_env_errors env expected_error;

  let env = Test.assert_errors_in_phase env 1 Errors.Naming in

  (* Fix the error *)
  let foo_contents_no_errors = (foo_contents "D") in
  let env = Test.connect_persistent_client env in
  let env, _ = Test.edit_file env "foo.php" foo_contents_no_errors in
  let env, _ = Test.full_check env in

  Test.assert_no_errors env;

  ignore env
