module Test = Integration_test_base

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

let () = Tempfile.with_real_tempdir @@ fun temp_dir ->
  let temp_dir = Path.to_string temp_dir in

  Test.save_state [
    "A.php", a_contents "int";
    "test.php", test_contents;
  ] temp_dir;

  let env = Test.load_state
    temp_dir
    ~disk_state:[
      "A.php", a_contents "string";
      "test.php", test_contents;
      ]
    ~master_changes:[]
    ~local_changes:["A.php"]
    ~use_precheked_files:false
  in

  Test.assert_needs_recheck env "test.php";
  ignore env
