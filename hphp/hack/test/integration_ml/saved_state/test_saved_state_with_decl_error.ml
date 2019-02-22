module Test = Integration_test_base

let foo_contents = Printf.sprintf {|<?hh // partial
class Foo {
  %s
  public function f() {}
}
|}

let baz_contents = {|<?hh // partial
class Baz extends Foo {

}
|}

let () = Tempfile.with_real_tempdir @@ fun temp_dir ->
  let temp_dir = Path.to_string temp_dir in

  let expected_error =
    "File \"/foo.php\", line 4, characters 19-19:\n" ^
    "Foo::f() is marked as override; no non-private parent definition found " ^
    "or overridden parent is defined in non-<?hh code (Typing[4087])\n" in

  let foo_contents_with_error = foo_contents "<<__Override>>" in

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

  Test.assert_needs_recheck env "foo.php";

  (* Trigger next full recheck to get global errors *)
  let env, _ = Test.full_check env in

  (* Make sure there's only the error we expect *)
  Test.assert_env_errors env expected_error;

  let env = Test.assert_errors_in_phase env 1 Errors.Typing in

  (* Fix the error *)
  let foo_contents_no_errors = (foo_contents "") in
  let env = Test.connect_persistent_client env in
  let env, _ = Test.edit_file env "foo.php" foo_contents_no_errors in
  let env, _ = Test.full_check env in

  Test.assert_no_errors env;

  ignore env
