open Integration_test_base_types
open ServerEnv

module Test = Integration_test_base

let init_disk_changes = [
"base.php",
"<?hh // strict
abstract class Base {
  public function meth(): void {}
  public function test(Child $c): void {
    $c->meth();
  }
}";

"child.php",
"<?hh // strict

class Child extends Base {}
";
]

let content = "<?hh // strict
abstract class Base {
  public function methFoo(): void {}
  public function test(Child $c): void {
    $c->meth();
  }
}
"

let next_disk_changes = [
"base.php",
"<?hh // strict
abstract class Base {
  public function methFoo(): void {}
  public function test(Child $c): void {
    $c->methFoo();
  }
}";
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

  SharedMem.invalidate_caches();
  ServerIdeUtils.declare_and_check content ~f:(fun _ _ -> ());

  let env, _ = Test.(run_loop_once env { default_loop_input with
    disk_changes = next_disk_changes;
  }) in
  let errors = Errors.get_sorted_error_list env.errorl in
  if errors <> [] then
    Test.fail ("Expected no errors. Got:\n" ^ errors_to_string errors)
