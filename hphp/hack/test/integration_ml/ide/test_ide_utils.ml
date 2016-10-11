open Integration_test_base_types
open ServerEnv

module Test = Integration_test_base

let errors_to_string errors =
  List.fold_left begin fun str error ->
    str ^ Errors.(error |> to_absolute |> to_string)
  end "" @@ errors

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

(* We revive removed class elements early because they may be referenced by
 * subclasses
 *)
let test_early_revive () =
  let expect_mem () =
    if not @@ Decl_heap.Methods.mem ("\\Base", "meth") then
      Test.fail "Expected Base::meth type to be in Decl_heap.Methods"
  in

  let env = Test.setup_server () in
  let tcopt = env.tcopt in

  let env, _ = Test.(run_loop_once env { default_loop_input with
    disk_changes = init_disk_changes;
  }) in
  let errors = Errors.get_sorted_error_list env.errorl in
  if errors <> [] then
    Test.fail ("Expected no errors. Got:\n" ^ errors_to_string errors);

  expect_mem();
  SharedMem.invalidate_caches();
  ServerIdeUtils.declare_and_check content ~f:(fun _ _ -> expect_mem()) tcopt;
  SharedMem.invalidate_caches();
  expect_mem();

  let env, _ = Test.(run_loop_once env { default_loop_input with
    disk_changes = next_disk_changes;
  }) in
  let errors = Errors.get_sorted_error_list env.errorl in
  if errors <> [] then
    Test.fail ("Expected no errors. Got:\n" ^ errors_to_string errors)

let kitchen_sink_content ="<?hh // strict
class Bar {
  const int C = 1;
  const type T = int;
  public static int $y = 0;
  public function __construct(
   private int $x
  ) {/* HH_FIXME[0] */}

  public function meth(): void {}
  public static function smeth(): void {}
}

function foo(): void {}
const int FOO = 1;
type Foo = int;
"

let kitchen_sink_content2 ="<?hh // strict
class Bar {
  const string T = '1';
  const type C = string;
  public static string $x = '0';
  public function __construct(
   private string $y
  ) {}

  public function smeth(): void {}
  public static function meth(): void {}
}

function Foo(): void {}
const string FOO = '1';
type Foo = string;
"

(* ServerIdeUtils.declare_and_check should leave the state of the world
 * exactly the same before and after being called. The easiest way to
 * verify this is seeing if the number of used slots in the shared hashtable
 * changed.
 *)
let test_cleanup () =
  let expect_same used after =
    if used <> after then
      Test.fail (
        Printf.sprintf
          "Number of hash slots used changed from %d to %d"
          used after
      )
  in
  let f _ _ = () in
  let env = Test.setup_server () in
  let tcopt = env.tcopt in

  let env, _ = Test.(run_loop_once env { default_loop_input with
    disk_changes = ["base.php", kitchen_sink_content];
  }) in
  let {SharedMem.used_slots; _ } = SharedMem.hash_stats () in
  ServerIdeUtils.declare_and_check kitchen_sink_content2 ~f tcopt;
  let {SharedMem.used_slots = after_used_slots; _ } =
    SharedMem.hash_stats () in
  expect_same used_slots after_used_slots;

  let env, _ = Test.(run_loop_once env { default_loop_input with
    disk_changes = ["base.php", kitchen_sink_content2];
  }) in
  let {SharedMem.used_slots; _ } = SharedMem.hash_stats () in
  ServerIdeUtils.declare_and_check kitchen_sink_content ~f tcopt;
  let {SharedMem.used_slots = after_used_slots; _ } =
    SharedMem.hash_stats () in
  expect_same used_slots after_used_slots;

  let env, _ = Test.(run_loop_once env { default_loop_input with
    disk_changes = ["base.php", ""];
  }) in
  let {SharedMem.used_slots; _ } = SharedMem.hash_stats () in
  ServerIdeUtils.declare_and_check kitchen_sink_content ~f tcopt;
  let {SharedMem.used_slots = after_used_slots; _ } =
    SharedMem.hash_stats () in
  expect_same used_slots after_used_slots;

  let _env, _ = Test.(run_loop_once env { default_loop_input with
    disk_changes = ["base.php", ""];
  }) in
  let {SharedMem.used_slots; _ } = SharedMem.hash_stats () in
  ServerIdeUtils.declare_and_check kitchen_sink_content2 ~f tcopt;
  let {SharedMem.used_slots = after_used_slots; _ } =
    SharedMem.hash_stats () in
  expect_same used_slots after_used_slots

let () =
  test_cleanup();
  test_early_revive()
