module Test = Integration_test_base

let a_file_name = "A.php"

let b_file_name = "B.php"

let c_file_name = "C.php"

let a_contents = "<?hh // strict

abstract class A {
  public X $x;
}
"

let b_contents = "<?hh // strict

type X = int;
"

let b_contents_after = "<?hh // strict

type X = ?int;
"

let c_contents = "<?hh // strict

class C extends A {}
"

let errors =
  {|
File "/C.php", line 3, characters 7-7:
Class `C` has properties that cannot be null and aren't always set in `__construct`. (NastCheck[3015])
  File "/A.php", line 4, characters 12-13:
  `$this->x` is not initialized.
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
  Test.assert_env_errors env errors;

  let (env, _) = Test.change_files env [(b_file_name, b_contents_after)] in
  Test.assert_no_errors env;
  ()
