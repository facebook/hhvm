(* Duplicate the same file twice *)
open Integration_test_base_types
open ServerEnv
module Test = Integration_test_base

let foo_contents =
  "<?hh //strict
    class Foo {
        public function g(): int {
            return 1;
        }
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
  (match Diagnostics.get_diagnostic_list env.diagnostics with
  | [] -> ()
  | _ -> Test.fail "Expected no errors");
  let (env, loop_output) =
    Test.(
      run_loop_once
        env
        {
          default_loop_input with
          disk_changes = [("foo.php", foo_contents); ("bar.php", foo_contents)];
        })
  in
  if not loop_output.did_read_disk_changes then
    Test.fail "Expected the server to process disk updates";

  let expected_result =
    "ERROR: File \"/foo.php\", line 2, characters 11-13:\n"
    ^ "Name already bound: `Foo` (Naming[2012])\n"
    ^ "  File \"/bar.php\", line 2, characters 11-13:\n"
    ^ "  Previous definition is here\n"
  in
  Test.assertSingleDiagnostic
    expected_result
    (Diagnostics.get_diagnostic_list env.diagnostics)
