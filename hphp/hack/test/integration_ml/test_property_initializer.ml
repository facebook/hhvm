module Test = Integration_test_base

let a_file_name = "A.php"

let b_file_name = "B.php"

let a_contents = "<?hh // strict

abstract class A {
  protected ?int $x;
}
"

let a_contents_require_init =
  "<?hh // strict

abstract class A {
  protected int $x;
}
"

let b_contents = "<?hh // strict

class B extends A {}
"

let errors =
  {|
File "/B.php", line 3, characters 7-7:
Class `B` has properties that cannot be null and aren't always set in `__construct`. (NastCheck[3015])
  File "/A.php", line 4, characters 17-18:
  `$this->x` is not initialized.
|}

let test () =
  let env = Test.setup_server () in
  let env =
    Test.setup_disk env [(a_file_name, a_contents); (b_file_name, b_contents)]
  in
  Test.assert_no_errors env;

  let (env, _) =
    Test.change_files env [(a_file_name, a_contents_require_init)]
  in
  Test.assert_env_errors env errors;
  ()
