open ServerEnv

module Test = Integration_test_base

(* Same as test_prechecked_incremental, except local changes happen AFTER
 * initial rechecking is finished *)
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
      (* No change to B yet! *)
      "B.php", x_contents "B" "int";
      (* changes foo() return type to B *)
      "foo.php", foo_contents "B";
      "test.php", test_contents;
      ]
    ~master_changes:["foo.php"]
    ~local_changes:[]
    ~use_precheked_files:true
  in

  (* Only foo needs rechecking, and since it's prechecked, none of it's fan-outs
   * are included *)
  Test.assert_needs_recheck env "foo.php";
  Test.assert_needs_no_recheck env "A.php";
  Test.assert_needs_no_recheck env "B.php";
  Test.assert_needs_no_recheck env "test.php";

  let env, _ = Test.full_check env in
  begin match env.prechecked_files with
  | Prechecked_files_ready _ -> ()
  | _ -> assert false end;

  let env = Test.connect_persistent_client env in
  let env = Test.open_file env "B.php" ~contents:(x_contents "B" "string") in
  let env = Test.wait env in
  let env, _ = Test.(run_loop_once env default_loop_input) in

  Test.assert_needs_recheck env "test.php";
  let env, _ = Test.full_check env in
  Test.assert_env_errors env test_errors;

  ignore env
