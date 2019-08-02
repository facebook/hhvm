open Integration_test_base_types
module Test = Integration_test_base

let file_a = "A.php"

let file_b = "B.php"

let content_a_0 =
  {|<?hh // strict
interface Rx {
}

class A {
}

<<__Rx>>
function f(A $a): void {
}
|}

let content_a_1 =
  {|<?hh // strict
interface Rx {
}

class A {
}

<<__Rx, __AtMostRxAsArgs>>
function f(<<__OnlyRxIfImpl(Rx::class)>> A $a): void {
}
|}

let content_b =
  {|<?hh // strict

<<__Rx>>
function g(): void {
  f(new A());
}
|}

let errors =
  {|
File "/B.php", line 5, characters 3-12:
Cannot invoke conditionally reactive function in reactive context, because at least one reactivity condition is not met. (Typing[4237])
File "/B.php", line 5, characters 9-9:
Argument type must be a subtype of \Rx, now \A.
File "/A.php", line 9, characters 10-10:
This is the function declaration
|}

let test () =
  let env = Test.setup_server () in
  let env = Test.setup_disk env [(file_a, content_a_0); (file_b, content_b)] in
  Test.assert_no_errors env;

  let (env, loop_output) =
    Test.(
      run_loop_once
        env
        { default_loop_input with disk_changes = [(file_a, content_a_1)] })
  in
  if not loop_output.did_read_disk_changes then
    Test.fail "Expected the server to process disk updates";

  Test.assert_env_errors env errors
