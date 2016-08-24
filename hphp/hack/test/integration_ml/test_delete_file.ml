(*  Delete a file that still has dangling references *)
open Integration_test_base_types
open ServerEnv

module Test = Integration_test_base

let foo_contents = "<?hh //strict
    class Foo {
        public static function g(): string {
            return 'a';
        }
    }
"

let bar_contents = "<?hh //strict
        function h(): string {
            return Foo::g();
        }
"

let () =
  let env = Test.setup_server () in
  let env, loop_output = Test.(run_loop_once env { default_loop_input with
    disk_changes = [
      "foo.php", foo_contents;
      "bar.php", bar_contents;
    ]
  }) in
  if not loop_output.did_read_disk_changes then
    Test.fail "Expected the server to process disk updates";
  begin
    match Errors.get_error_list env.errorl with
      | [] -> ()
      | _ -> Test.fail "Expected no errors"
  end;
  let env, loop_output = Test.(run_loop_once env { default_loop_input with
    disk_changes = [
      "foo.php", "";
      "bar.php", bar_contents;
    ]
  }) in
  if not loop_output.did_read_disk_changes then
    Test.fail "Expected the server to process disk updates";
  let expected_error = "File \"/bar.php\", line 3, characters 20-22:\n" ^
                       "Unbound name: Foo (an object type) (Naming[2049])\n" in
  Test.assertSingleError expected_error (Errors.get_error_list env.errorl)
