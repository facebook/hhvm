(*
  This checks that we handle duplicate parent classes, i.e. when Bar
  extends Foo and there are two declarations of Foo. We want to make sure
  that when the duplicate gets removed, we recover correctly by
  redeclaring Bar with the remaining parent class.
*)
open Integration_test_base_types
open ServerEnv
open Hh_prelude
module Test = Integration_test_base

let foo_contents = "<?hh
    class Foo {
        public static ?int $x;
    }
"

let qux_contents =
  "<?hh
function h(): int {
    return 1;
}

class Foo {}

function setUpClass(): void {
    new Foo();
    h();
}
"

let bar_contents =
  "<?hh
      class Bar extends Foo {}

      function main(Bar $a): void {
          $a::$y;
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
    "ERROR: File \"/bar.php\", line 5, characters 15-16:\n"
    ^ "No class variable `$y` in `Bar` (Typing[4090])\n"
    ^ "  File \"/foo.php\", line 3, characters 23-26:\n"
    ^ "  Did you mean `$x` instead?\n"
    ^ "  File \"/bar.php\", line 2, characters 13-15:\n"
    ^ "  Declaration of `Bar` is here\n"
  in
  let qux_error =
    "ERROR: File \"/qux.php\", line 6, characters 7-9:\n"
    ^ "Name already bound: `Foo` (Naming[2012])\n"
    ^ "  File \"/foo.php\", line 2, characters 11-13:\n"
    ^ "  Previous definition is here\n"
  in
  (* We should get exactly these two errors *)
  match Diagnostics.get_diagnostic_list env.diagnostics with
  | [x; y] ->
    Test.assertSingleDiagnostic qux_error [y];
    Test.assertSingleDiagnostic bar_error [x]
  | errs ->
    Test.fail
    @@ "Expected exactly two errors, but got:\n"
    ^ String.concat ~sep:"\n" (Test.diagnostic_strings errs)
