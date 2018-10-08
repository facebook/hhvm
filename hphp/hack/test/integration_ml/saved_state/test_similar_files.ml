open Core_kernel

module Test = Integration_test_base

(* test.php depends on foo.php. foo.php will change, but remain similar. *)
let foo_name = "foo.php"
let foo_contents = {|<?hh // strict

/**
 * foo docblock
 */
function foo() : void {
  1 + 1 == 2;
}

/**
 * Bar docblock
 */
class Bar {
  /**
   * Bar::$x docblock
   */
  public int $x = 1;
  /**
   * Bar::baz docblock
   */
  public function baz(): void {}
}
|}

let foo_similar_contents = {|<?hh // strict

/**
 * slightly different foo docblock
 */
function foo() : void {
  // slightly different foo contents
  1 + 1 == 3;
}

/**
 * slightly different Bar docblock
 */
class Bar {
  /**
   * Slightly different Bar::$x docblock
   */
  public int $x = 1;
  /**
   * Slightly different Bar::baz docblock
   */
  public function baz(): void {}
}
|}

let test_name = "test.php"
let test_contests = {|<?hh // strict
function test(Bar $bar): void {
  foo();
  $bar->baz();
}
|}

(* y that depends on x. x will change in a significant way *)
let x_name = "x.php"
let x_contents = Printf.sprintf {|<?hh // strict
function x(): %s {
  // UNSAFE BLOCK
}
|}
let y_name = "y.php"
let y_contents = {|<?hh // strict
function y(): int {
  return x();
}
|}

let () = Tempfile.with_real_tempdir @@ fun temp_dir ->
  let saved_state_dir = Path.to_string temp_dir in

  Test.save_state [
    foo_name, foo_contents;
    test_name, test_contests;
    x_name, x_contents "int";
    y_name, y_contents;
  ] saved_state_dir;

  let env = Test.load_state
    saved_state_dir
    ~disk_state:[
      foo_name, foo_similar_contents;
      test_name, test_contests;
      x_name, x_contents "string";
      y_name, y_contents;
    ]
    ~master_changes:[]
    ~local_changes:[foo_name; x_name]
    ~use_precheked_files:false
  in
  (* Checking that we fan-out to y as a sanity check *)
  Test.assert_needs_recheck env y_name;
  (* Main test: even though test depends on foo and foo have changed, it's still
   * similar, so we don't need to recheck test *)
  Test.assert_needs_no_recheck env test_name;
  ignore env;
