module Test = Integration_test_base

let foo_contents = Printf.sprintf {|<?hh // partial

function woot() {}
%s
|}

let baz_contents = {|<?hh // partial
class Baz {
  public function f() { woot(); }
}
|}

let test_parsing_error (expected_error: string) (bad_contents: string) () : unit =
  Tempfile.with_real_tempdir @@ fun temp_dir ->
    let temp_dir = Path.to_string temp_dir in

    let foo_contents_with_error = foo_contents bad_contents in

    let disk_state = [
      "foo.php", foo_contents_with_error;
      "baz.php", baz_contents;
    ] in

    Test.save_state_with_errors disk_state temp_dir expected_error;

    let env = Test.load_state
      temp_dir
      ~disk_state:disk_state
      ~master_changes:[]
      ~local_changes:[]
      ~use_precheked_files:false
    in

    (* Trigger next full recheck to get global errors *)
    let env, _ = Test.full_check env in

    (* Make sure there's only the error we expect *)
    Test.assert_env_errors env expected_error;

    let env = Test.assert_errors_in_phase env 1 Errors.Parsing in

    (* Fix the error *)
    let foo_contents_no_errors = (foo_contents "") in
    let env = Test.connect_persistent_client env in
    let env, _ = Test.edit_file env "foo.php" foo_contents_no_errors in
    let env, _ = Test.full_check env in

    Test.assert_no_errors env;

    ignore env

let () =
  let expected_error =
    "File \"/foo.php\", line 4, characters 11-11:\n" ^
    "A semicolon (';') is expected here. (Parsing[1002])\n" in

  let bad_contents = "asdl;jaflj" in

  Test.in_daemon @@ test_parsing_error expected_error bad_contents;

  let expected_error =
    "File \"/foo.php\", line 4, characters 1-3:\n" ^
    "An expression is expected here. (Parsing[1002])\n" in

  let bad_contents = "?>" in

  Test.in_daemon @@ test_parsing_error expected_error bad_contents;
