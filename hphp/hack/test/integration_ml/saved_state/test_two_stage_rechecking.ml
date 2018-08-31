module Test = Integration_test_base

(* Same test as test_saved_state, but with precheked_files flag enabled, which
 * causes two-stage rechecking (see ServerEnv.Initial_typechecking) *)
let a_contents = Printf.sprintf {|<?hh
class A {
  public function foo(): %s {
    // UNSAFE_EXPR
  }
}
|}

let test_contents = {|<?hh
function test(A $a): int { return $a->foo(); }
|}

let test_errors = {|
File "/test.php", line 2, characters 35-43:
Invalid return type (Typing[4110])
File "/test.php", line 2, characters 22-24:
This is an int
File "/A.php", line 3, characters 26-31:
It is incompatible with a string
|}

let () = Tempfile.with_real_tempdir @@ fun temp_dir ->
  let temp_dir = Path.to_string temp_dir in

  Test.save_state [
    "A.php", a_contents "int";
    "test.php", test_contents;
  ] temp_dir;

  let env = Test.load_state
    ~saved_state_dir:temp_dir
    ~disk_state:[
      "A.php", a_contents "string";
      "test.php", test_contents;
      ]
    ~master_changes:[]
    ~local_changes:["A.php"]
    ~use_precheked_files:true
  in

  Test.assert_needs_recheck env "A.php";
  Test.assert_needs_no_recheck env "test.php";

  (match env.ServerEnv.prechecked_files with
  | ServerEnv.Initial_typechecking _ -> ()
  | _ -> assert false);

  (* Prevent batching of two stages together so that we can inspect the state
   * between then *)
  ServerMain.force_break_recheck_loop_for_test true;
  let env, _ = Test.full_check env in

  (match env.ServerEnv.prechecked_files with
  | ServerEnv.Prechecked_files_ready -> ()
  | _ -> assert false);

  Test.assert_no_errors env;

  Test.assert_needs_recheck env "A.php"; (* TODO: avoid double checking of A *)
  Test.assert_needs_recheck env "test.php";
  (* Run second stage of rechecking *)
  let env, _ = Test.(run_loop_once env default_loop_input) in

  Test.assert_needs_no_recheck env "A.php";
  Test.assert_needs_no_recheck env "test.php";

  Test.assert_env_errors env test_errors;
  ignore env
