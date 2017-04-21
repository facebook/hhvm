(* Duplicate the same file twice *)
open Integration_test_base_types
open ServerEnv

module Test = Integration_test_base

let foo_contents = "<?hh //strict
    class Foo {
        public function g(): string {
            return 'a';
        }
    }
"



let () =
  let env = Test.setup_server () in
  let env, loop_output = Test.(run_loop_once env { default_loop_input with
    disk_changes = [
      "foo.php", foo_contents;
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
      "foo.php", foo_contents;
      "bar.php", foo_contents;
    ]
  }) in
  if not loop_output.did_read_disk_changes then
    Test.fail "Expected the server to process disk updates";

  let expected_result = "File \"/foo.php\", line 2, characters 11-13:\n" ^
                        "Name already bound: Foo (Naming[2012])\n" ^
                        "File \"/bar.php\", line 2, characters 11-13:\n" ^
                        "Previous definition is here\n" in
  Test.assertSingleError expected_result (Errors.get_error_list env.errorl);
