open Integration_test_base_types
open ServerEnv

module Test = Integration_test_base

let init_disk_changes = [
"base.php",
"<?hh // strict
abstract class Base {
  public static function meth(): void {}
}";

"parent.php",
"<?hh // strict
abstract class ParentClass extends Base {
}";

"achild.php",
"<?hh // strict

class AChild extends ParentClass {
  public static function test(): void {
    $achild = new self();
    $achild->__meth();
  }

  private function __meth(): void {}
}
";
]

let next_disk_changes = [

"base.php",
"<?hh // strict
abstract class Base {
}
";

"achild.php",
"<?hh // strict

class AChild extends ParentClass {
  public static function test(): void {
    $achild = new self();
    $achild->meth();
  }

  private function meth(): void {}
}
";

]

let errors_to_string errors =
  List.fold_left begin fun str error ->
    str ^ Errors.(error |> to_absolute |> to_string)
  end "" @@ errors

let () =
  let env = Test.setup_server () in

  let env, _ = Test.(run_loop_once env { default_loop_input with
    disk_changes = init_disk_changes;
  }) in
  let errors = Errors.get_sorted_error_list env.errorl in
  if errors <> [] then
    Test.fail ("Expected no errors. Got:\n" ^ errors_to_string errors);

  let env, _ = Test.(run_loop_once env { default_loop_input with
    disk_changes = next_disk_changes;
  }) in
  let errors = Errors.get_sorted_error_list env.errorl in
  if errors <> [] then
    Test.fail ("Expected no errors. Got:\n" ^ errors_to_string errors)
