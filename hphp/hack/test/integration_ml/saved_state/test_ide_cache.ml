module Test = Integration_test_base
open Integration_test_base_types

let foo_contents = {|<?hh //strict
function foo(): int {
  /* HH_FIXME[4110] */
  return "4";
}
|}

let bar_name = "bar.php"
let bar_contents = {|<?hh //strict
// parse error
function foo {
|}

(* Computing this will consume HH_FIXMEs in the file. Computing it twice will check if
 * those FIXME's are properly cached along with AST. *)
let status_single_request = ServerCommandTypes.(STATUS_SINGLE (FileContent foo_contents))

(* Computing this will generate errors. Computing it twice will check if
 * those errors are properly cached along with AST. *)
let parse_error_by_contents =
  ServerCommandTypes.(STATUS_SINGLE (FileContent bar_contents))
(* This will parse the same file contents, but under different name. We embed filenames in
 * AST positions and errors, so need to be careful not to cache them together *)
let parse_error_by_filename =
  ServerCommandTypes.(STATUS_SINGLE (FileName (Test.prepend_root bar_name)))

let check_no_errors = function
| None -> Test.fail "Expected STATUS_SINGLE response"
| Some [] -> ()
| Some _ -> Test.fail "Expected no errors"

let expected_errors_by_contents = {|
File "", line 3, characters 14-14:
A left parenthesis ('(') is expected here. (Parsing[1002])

File "", line 3, characters 14-14:
Encountered unexpected text '{', was expecting a type hint. (Parsing[1002])
|}

let expected_errors_by_filename = {|
File "/bar.php", line 3, characters 14-14:
A left parenthesis ('(') is expected here. (Parsing[1002])

File "/bar.php", line 3, characters 14-14:
Encountered unexpected text '{', was expecting a type hint. (Parsing[1002])
|}

let check_errors expected_errors = function
| None -> Test.fail "Expected STATUS_SINGLE response"
| Some [] -> Test.fail "Expected errors"
| Some errors -> Test.assertEqual expected_errors (Test.errors_to_string errors)

let () = Tempfile.with_real_tempdir @@ fun temp_dir ->
  let saved_state_dir = Path.to_string temp_dir in
  Test.save_state [] saved_state_dir;
  Ide_parser_cache.enable ();
  let env = Test.load_state
    saved_state_dir
    ~disk_state:[
      bar_name, bar_contents;
    ]
    ~master_changes:[]
    ~local_changes:[bar_name]
  in

  let env, loop_output = Test.(run_loop_once env { default_loop_input with
    new_client = Some (RequestResponse status_single_request)
  }) in
  check_no_errors loop_output.new_client_response;

  let env, loop_output = Test.(run_loop_once env { default_loop_input with
    new_client = Some (RequestResponse status_single_request)
  }) in
  check_no_errors loop_output.new_client_response;

  let env, loop_output = Test.(run_loop_once env { default_loop_input with
    new_client = Some (RequestResponse parse_error_by_contents)
  }) in
  check_errors expected_errors_by_contents loop_output.new_client_response;

  let env, loop_output = Test.(run_loop_once env { default_loop_input with
    new_client = Some (RequestResponse parse_error_by_contents)
  }) in
  check_errors expected_errors_by_contents loop_output.new_client_response;

  let env, loop_output = Test.(run_loop_once env { default_loop_input with
    new_client = Some (RequestResponse parse_error_by_filename)
  }) in
  check_errors expected_errors_by_filename loop_output.new_client_response;

  ignore env;
  ()
