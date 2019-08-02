open Integration_test_base_types
module Test = Integration_test_base

let a_file_name = "A.php"

let b_file_name = "B.php"

let a_contents =
  "<?hh // strict

final class A {
  const mixed C = dict[
    'k' => B::class,
  ];
}
"

let b_contents = "<?hh // strict

class B {}
"

let errors =
  {|
File "/A.php", line 5, characters 12-12:
Unbound name: B (an object type) (Naming[2049])
|}

let test () =
  let env = Test.setup_server () in
  let env =
    Test.setup_disk env [(a_file_name, a_contents); (b_file_name, "")]
  in
  Test.assert_env_errors env errors;

  let (env, _) =
    Test.(
      run_loop_once
        env
        { default_loop_input with disk_changes = [(b_file_name, b_contents)] })
  in
  Test.assert_no_errors env;
  ignore env;
  ()
