(*  Delete a file that still has dangling references *)
open Integration_test_base_types
open ServerEnv
module Test = Integration_test_base

let foo_contents =
  "<?hh
    class Foo {
        public static function g(): int {
            return 1;
        }
    }
"

let bar_contents =
  "<?hh
        function h(): int {
            return Foo::g();
        }
"

let test () =
  let env = Test.setup_server () in
  let (env, loop_output) =
    Test.(
      run_loop_once
        env
        {
          default_loop_input with
          disk_changes = [("foo.php", foo_contents); ("bar.php", bar_contents)];
        })
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
          disk_changes = [("foo.php", ""); ("bar.php", bar_contents)];
        })
  in
  if not loop_output.did_read_disk_changes then
    Test.fail "Expected the server to process disk updates";
  let expected_error =
    "ERROR: File \"/bar.php\", line 3, characters 20-22:\n"
    ^ "Unbound name: `Foo` (an object type) (Naming[2049])\n"
  in
  Test.assertSingleDiagnostic
    expected_error
    (Diagnostics.get_diagnostic_list env.diagnostics)
