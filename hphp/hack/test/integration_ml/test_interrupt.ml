module Test = Integration_test_base

let foo_name = "foo.php"
let bar_name = Printf.sprintf "bar%d.php"

let foo_contents = "<?hh //strict
function foo() : string {
  // UNSAFE_EXPR
}
"

let bar_contents = Printf.sprintf "<?hh //strict

function bar%d() : int {
  return foo();
}
"

let expected_errors = {|
File "/bar2.php", line 4, characters 10-14:
Invalid return type (Typing[4110])
File "/bar2.php", line 3, characters 19-21:
This is an int
File "/foo.php", line 2, characters 18-23:
It is incompatible with a string
|}


let () =
  let env = Test.setup_server () in
  (* There are errors in both bar files *)
  let env = Test.setup_disk env [
    foo_name, foo_contents;
    bar_name 1, bar_contents 1;
    bar_name 2, bar_contents 2;
  ] in

  (* Prepare rechecking of all files *)
  let workers = None in
  let options = TypecheckerOptions.default in
  let fast = Naming_table.to_fast env.ServerEnv.naming_table in

  (* Pretend that this rechecking will be cancelled before we get to bar1 *)
  let bar1_path =
    Relative_path.(create Root (Test.prepend_root (bar_name 1))) in
  Typing_check_service.TestMocking.set_is_cancelled (bar1_path);

  (* Run the recheck *)
  let interrupt = MultiThreadedCall.no_interrupt () in
  let errors, (), cancelled = Typing_check_service.go_with_interrupt
    workers options Relative_path.Set.empty fast ~interrupt ~memory_cap:None in

  (* Assert that we got the errors in bar2 only... *)
  Test.assert_errors errors expected_errors;
  (* ...while bar1 is among cancelled jobs*)
  begin match cancelled with
    | [(x, _)] when x = bar1_path -> ()
    | _ -> assert false
  end;
  ()
