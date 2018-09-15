open Integration_test_base_types
module Test = Integration_test_base

let foo_name = "foo.php"
let bar_name = Printf.sprintf "bar%d.php"

let foo_contents = Printf.sprintf "<?hh //strict
function foo() : %s {
  // UNSAFE_EXPR
}
"

let bar_contents = Printf.sprintf "<?hh //strict

function bar%d() : int {
  return foo();
}
"

let expected_errors = {|
File "/bar1.php", line 4, characters 10-14:
Invalid return type (Typing[4110])
File "/bar1.php", line 3, characters 19-21:
This is an int
File "/foo.php", line 2, characters 18-23:
It is incompatible with a string

File "/bar2.php", line 4, characters 10-14:
Invalid return type (Typing[4110])
File "/bar2.php", line 3, characters 19-21:
This is an int
File "/foo.php", line 2, characters 18-23:
It is incompatible with a string
|}

let () =
  let env = Test.setup_server () in
  (* There are initially no errors *)
  let env = Test.setup_disk env [
    foo_name , foo_contents "int";
    bar_name 1, bar_contents 1;
    bar_name 2, bar_contents 2;
  ] in
  Test.assert_no_errors env;

  (* This change will recheck all files and reveal errors in bar1 and bar2 *)
  let loop_input = Test.({default_loop_input with
    disk_changes = [
      foo_name, foo_contents "string"
    ]
  }) in

  (* ... but we'll pretend that this recheck will be cancelled before getting
   * to bar1 *)
  let bar1_path =
    Relative_path.(create Root (Test.prepend_root (bar_name 1))) in
  Typing_check_service.TestMocking.set_is_cancelled (bar1_path);

  let env, _ = Test.run_loop_once env loop_input in
  (* We'll get all the errors anyway due to server looping until check is
   * fully finished *)
  Test.assert_env_errors env expected_errors;
