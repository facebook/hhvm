module Test = Integration_test_base

let x_contents = Printf.sprintf {|<?hh
class %s {
    public function bar() : %s {
      // UNSAFE_BLOCK
    }
}
|}

let foo_contents = Printf.sprintf {|<?hh
function foo(): %s {
  // UNSAFE_BLOCK
}
|}

let test_contents = {|<?hh
function test() : int {
  return foo()->bar();
}
|}

let test_errors = {|
File "/test.php", line 3, characters 10-21:
Invalid return type (Typing[4110])
File "/test.php", line 2, characters 19-21:
This is an int
File "/B.php", line 3, characters 29-34:
It is incompatible with a string
|}

let () = Tempfile.with_real_tempdir @@ fun temp_dir ->
  let temp_dir = Path.to_string temp_dir in

  (* A and B are identical classes with a function named "bar" returning int *)
  Test.save_state [
    "A.php", x_contents "A" "int";
    "B.php", x_contents "B" "int";
    "foo.php", foo_contents "A";
    "test.php", test_contents;
  ] temp_dir;

  let env = Test.load_state
    temp_dir
    ~disk_state:[
      "A.php", x_contents "A" "int";
      (* changes B::foo return type to string *)
      "B.php", x_contents "B" "string";
      (* changes foo() return type to B *)
      "foo.php", foo_contents "B";
      "test.php", test_contents;
      ]
    ~master_changes:["foo.php"]
    ~local_changes:["B.php"]
    ~use_precheked_files:true
  in

  (* foo and B are dirty, so they need recheck *)
  Test.assert_needs_recheck env "foo.php";
  Test.assert_needs_recheck env "B.php";
  Test.assert_needs_no_recheck env "test.php";

  let env, _ = Test.full_check env in
  Test.assert_no_errors env;

  (* test depends (through foo) on B, which was a local dirty change, so needs
   * to be rechecked *)
  Test.assert_needs_no_recheck env "foo.php";
  Test.assert_needs_no_recheck env "B.php";
  Test.assert_needs_recheck env "test.php"; (* important part *)

  let env, _ = Test.(run_loop_once env default_loop_input) in
  Test.assert_env_errors env test_errors;
  ignore env
