(*
  This checks that we handle duplicate parent classes, i.e. when Bar
  extends Foo and there are two declarations of Foo. We want to make sure
  that when the duplicate gets removed, we recover correctly by
  redeclaring Bar with the remaining parent class.
*)
open Integration_test_base_types
open ServerEnv
module Test = Integration_test_base

let foo_contents =
  "<?hh // partial
    class Foo {
        public static $x;
    }
"

let qux_contents =
  "<?hh // partial
function h(): string {
    return 'a';
}

class Foo {}

function setUpClass() {
    new Foo();
    h();
}
"

let bar_contents =
  "<?hh // partial
      class Bar extends Foo {}

      function main(Bar $a) {
          return $a::$y;
      }
"

let test () =
  let env = Test.setup_server () in
  let (env, loop_output) =
    Test.(
      run_loop_once
        env
        {
          default_loop_input with
          disk_changes =
            [
              ("foo.php", foo_contents);
              ("qux.php", qux_contents);
              ("bar.php", bar_contents);
            ];
        })
  in
  if not loop_output.did_read_disk_changes then
    Test.fail "Expected the server to process disk updates";
  let bar_error =
    "File \"/bar.php\", line 5, characters 22-23:\n"
    ^ "No class variable '$y' in Bar (Typing[4090])\n"
    ^ "File \"/bar.php\", line 2, characters 13-15:\n"
    ^ "Declaration of Bar is here\n"
  in
  let qux_error =
    "File \"/qux.php\", line 6, characters 7-9:\n"
    ^ "Name already bound: Foo (Naming[2012])\n"
    ^ "File \"/foo.php\", line 2, characters 11-13:\n"
    ^ "Previous definition is here\n"
  in
  (* We should get exactly these two errors *)
  match Errors.get_error_list env.errorl with
  | [x; y] ->
    Test.assertSingleError qux_error [y];
    Test.assertSingleError bar_error [x]
  | _ -> Test.fail "Expected exactly two errors"
