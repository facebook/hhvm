open Integration_test_base_types
module Test = Integration_test_base

let a_file_name = "A.php"

let b_file_name = "B.php"

let c_file_name = "C.php"

let a_contents = "<?hh
  new module foo {}
"

let b_contents = "<?hh
new module foo {}
"

let c_contents = "<?hh
module foo;
class Foo {}
"

let errors =
  {|
ERROR: File "/B.php", line 2, characters 12-14:
Name already bound: `foo` (Naming[2012])
  File "/A.php", line 2, characters 14-16:
  Previous definition is here
|}

let test () =
  let env = Test.setup_server () in
  let env =
    Test.setup_disk
      env
      [
        (a_file_name, a_contents);
        (b_file_name, b_contents);
        (c_file_name, c_contents);
      ]
  in
  Test.assert_env_diagnostics env errors;

  let (env, _) =
    Test.(
      run_loop_once
        env
        { default_loop_input with disk_changes = [(b_file_name, "")] })
  in
  Test.assert_no_diagnostics env;
  ignore env;
  ()
