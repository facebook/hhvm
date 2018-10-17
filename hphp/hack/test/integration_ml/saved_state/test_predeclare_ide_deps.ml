module Test = Integration_test_base
open Integration_test_base_types

let foo_fun = {|<?hh // strict
function foo(): void {}
|}

let bar_class = {|<?hh // strict
class Bar {}
|}

let baz_typedef = {|<?hh // strict
type Baz = int;
|}

let qux_const = {|<?hh // strict
const int QUX = 4;
|}

let test_contents = {|<?hh // strict
function test(Baz $x): void {
  Bar::class;
  foo();
  QUX + $x;
}
|}

let disk_state = [
  "foo.php", foo_fun;
  "bar.php", bar_class;
  "baz.php", baz_typedef;
  "qux.php", qux_const;
]

let () = Tempfile.with_real_tempdir @@ fun temp_dir ->
  let saved_state_dir = Path.to_string temp_dir in
  Test.save_state disk_state saved_state_dir;
  let env = Test.load_state
    saved_state_dir
    ~disk_state
    ~master_changes:[]
    ~local_changes:[]
    ~predeclare_ide_deps:true
  in

  (* After loading from saved state, nothing should be in memory *)
  assert (not @@ Decl_heap.Funs.mem "\\foo");
  assert (not @@ Decl_heap.Classes.mem "\\Bar");
  assert (not @@ Decl_heap.Typedefs.mem "\\Baz");
  assert (not @@ Decl_heap.GConsts.mem "\\QUX");

  (* Executing any command that will lead to running type inference on test_contents *)
  let env = Test.connect_persistent_client env in
  let env, _ = Test.(run_loop_once env { default_loop_input with
    persistent_client_request = Some (
      Request ServerCommandTypes.(IDENTIFY_FUNCTION ((FileContent test_contents), 5, 5))
    )
  }) in

  (* Ensure that any results we'll get are from shared memory *)
  SharedMem.invalidate_caches ();

  let foo_path = Relative_path.from_root "foo.php" in

  (* Check that we didn't pollute parser heaps with anything *)
  assert (not @@ File_heap.FileHeap.mem foo_path);
  assert (not @@ Parser_heap.ParserHeap.mem foo_path);

  (* Check that we did populate declaration heaps *)
  assert (Decl_heap.Funs.mem "\\foo");
  assert (Decl_heap.Classes.mem "\\Bar");
  assert (Decl_heap.Typedefs.mem "\\Baz");
  assert (Decl_heap.GConsts.mem "\\QUX");
  ignore env;
  ()
