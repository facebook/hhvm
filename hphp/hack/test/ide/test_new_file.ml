open Integration_test_base_types
open ServerEnv

module Test = Integration_test_base

let foo_contents = "<?hh

{
"

let () =
  let env = Test.setup_server () in
  let env, loop_output = Test.run_loop_once env {
    disk_changes = [
      "foo.php", foo_contents
    ]
  } in

  if not loop_output.did_read_disk_changes then
    Test.fail "Expected the server to process disk updates";

  match Errors.get_error_list env.errorl with
  | [x] ->
      let error_string = Errors.(to_string (to_absolute x)) in
      let expected_error =
        "File \"/foo.php\", line 4, characters 1-0:\n" ^
        "Expected } (Parsing[1002])\n"
        in
      Test.assertEqual expected_error error_string
  | _ -> Test.fail "Expected to have errors"
