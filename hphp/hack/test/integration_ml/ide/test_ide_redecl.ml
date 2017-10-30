(*  Delete a file that still has dangling references *)
open Integration_test_base_types

module Test = Integration_test_base

let foo_contents = "<?hh // strict
class Food {




  public function f(): void {}


}

"

let foo2_contents =  "<?hh // strict


class Food {




  public function f(): void {}


}
"

let bar_contents = "<?hh // strict
class Bar extends Food {


  public function test() : void {
    $this->f();
  }
}

"
let bar2_contents = "<?hh // strict


  class Bar extends Foo  {

    function test() : string {
      return $this->g();
    }
  }
"

let diagnostic_subscription_id = 223
let assert_no_push_message loop_outputs =
  match loop_outputs.push_message with
  | Some _ -> Test.fail "Unexpected push message"
  | None -> ()


let () =

  let env = Test.setup_server () in
  let env = Test.connect_persistent_client env in
  (* Initially there is a single typing error in bar.php *)
  let env = Test.setup_disk env
  [
    "foo.php", foo_contents;
    "bar.php", bar_contents;
  ] in
  (* Edit foo, which causes bar.php to be added to "lazy_decl_later" *)
  let env = Test.subscribe_diagnostic ~id:diagnostic_subscription_id env in
  let env, _ = Test.(run_loop_once env default_loop_input) in
  let env = Test.open_file env "foo.php" ~contents:foo_contents in
  let env = Test.wait env in
  let env, loop_outputs = Test.(run_loop_once env default_loop_input) in
  assert_no_push_message loop_outputs;
  let env, _ = Test.edit_file env "foo.php" foo2_contents in
  let env = Test.wait env in
  let env, _ = Test.(run_loop_once env default_loop_input) in

  (* Edit bar once *)
  let env = Test.open_file env "bar.php" ~contents:bar_contents in
  let env = Test.wait env in
  let env, _ = Test.(run_loop_once env default_loop_input) in
  let env, _ = Test.edit_file env "bar.php" bar2_contents in
  let env = Test.wait env in
  let env, _ = Test.(run_loop_once env default_loop_input) in

  (* Edit bar a second time, causing it to be oldified twice *)
  let env = Test.open_file env "bar.php" ~contents:bar_contents in
  let env = Test.wait env in
  let env, _ = Test.(run_loop_once env default_loop_input) in
  let env, _ = Test.edit_file env "bar.php" bar_contents in
  let env = Test.wait env in
  let _, _ = Test.(run_loop_once env default_loop_input) in


  ()
