module Test = Integration_test_base
open Integration_test_base_types


let foo_name = "foo.php"
let foo_contents = Printf.sprintf {|<?hh //strict
function foo(): %s {
  // UNSAFE_BLOCK
}
|}

let bar_contents = {|<?hh //strict
function bar(): int {
  return foo();
}
|}

let typing_error_by_content =
  ServerCommandTypes.(STATUS_SINGLE (FileContent bar_contents))

let check_no_errors = function
| None -> Test.fail "Expected STATUS_SINGLE response"
| Some [] -> ()
| Some _ -> Test.fail "Expected no errors"

let check_errors expected_errors = function
| None -> Test.fail "Expected STATUS_SINGLE response"
| Some [] -> Test.fail "Expected errors"
| Some errors -> Test.assertEqual expected_errors (Test.errors_to_string errors)

let expected_typing_error = {|
File "", line 3, characters 10-14:
Invalid return type (Typing[4110])
File "", line 2, characters 17-19:
This is an int
File "/foo.php", line 2, characters 17-22:
It is incompatible with a string
|}

let () = Tempfile.with_real_tempdir @@ fun temp_dir ->
  let saved_state_dir = Path.to_string temp_dir in
  Test.save_state [] saved_state_dir;
  Ide_tast_cache.enable ();
  let env = Test.load_state
    saved_state_dir
    ~disk_state:[
      foo_name, foo_contents "string";
    ]
    ~master_changes:[]
    ~local_changes:[foo_name]
  in

  let env, loop_output = Test.(run_loop_once env { default_loop_input with
    new_client = Some (RequestResponse typing_error_by_content)
  }) in
  check_errors expected_typing_error loop_output.new_client_response;

  let env, loop_output = Test.(run_loop_once env { default_loop_input with
    new_client = Some (RequestResponse typing_error_by_content)
  }) in
  check_errors expected_typing_error loop_output.new_client_response;

  let env, _ = Test.(run_loop_once env { default_loop_input with
    disk_changes = [foo_name, foo_contents "int"]
  }) in

  (* Check that we invalidate caches ASTs *)
  let env, loop_output = Test.(run_loop_once env { default_loop_input with
    new_client = Some (RequestResponse typing_error_by_content)
  }) in
  check_no_errors loop_output.new_client_response;

  ignore env;
  ()
