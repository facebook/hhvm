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
Class `C` does not initialize all of its members; `x` is not always initialized.
Make sure you systematically set `$this->x` when the method `__construct` is called.
Alternatively, you can define the member as nullable with `?YourTypeHere`
 (NastCheck[3015])
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
