open Integration_test_base_types
open ServerEnv
module Test = Integration_test_base

let foo_contents =
  "<?hh
        function g(): string {
            return 'a';
        }
"

let foo_changes =
  "<?hh
        function g(): int {
            return 'a';
        }
"

let test () =
  let env = Test.setup_server () in
  let (env, loop_output) =
    Test.(
      run_loop_once
        env
        { default_loop_input with disk_changes = [("foo.php", foo_contents)] })
  in
  if not loop_output.did_read_disk_changes then
    Test.fail "Expected the server to process disk updates";
  (match Errors.get_error_list env.errorl with
  | [] -> ()
  | _ -> Test.fail "Expected no errors");
  let (env, loop_output) =
    Test.(
      run_loop_once
        env
        { default_loop_input with disk_changes = [("foo.php", foo_changes)] })
  in
  if not loop_output.did_read_disk_changes then
    Test.fail "Expected the server to process disk updates";

  let expected_error =
    "File \"/foo.php\", line 3, characters 20-22:\n"
    ^ "Invalid return type (Typing[4110])\n"
    ^ "File \"/foo.php\", line 2, characters 23-25:\n"
    ^ "Expected int\n"
    ^ "File \"/foo.php\", line 3, characters 20-22:\n"
    ^ "But got string\n"
  in
  Test.assertSingleError expected_error (Errors.get_error_list env.errorl)
