open Integration_test_base_types
module Test = Integration_test_base

let f1 = "f1.php"

let f2 = "f2.php"

let test = "test.php"

let test_contents = "<?hh // strict

function test(C $c) : void {}
"

let contents = Printf.sprintf "<?hh // strict

class %s {}
"

let c_contents = contents "C"

let d_contents = contents "D"

let errors =
  "
File \"/test.php\", line 3, characters 15-15:
Unbound name: C (Naming[2049])
"

let test () =
  let env = Test.setup_server () in
  let env =
    Test.setup_disk
      env
      [(test, test_contents); (f1, c_contents); (f2, d_contents)]
  in
  Test.assert_no_errors env;

  (* Rename C to (duplicate of) D *)
  let (env, _) =
    Test.(
      run_loop_once
        env
        { default_loop_input with disk_changes = [(f1, d_contents)] })
  in
  (* Remove the duplicate *)
  let (env, _) =
    Test.(
      run_loop_once env { default_loop_input with disk_changes = [(f1, "")] })
  in
  Test.assert_env_errors env errors
