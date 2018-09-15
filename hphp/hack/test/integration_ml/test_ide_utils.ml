module Test = Integration_test_base

let foo_contents = "<?hh //strict
class Foo {
  public function f() : void {}
}
"

let bar_contents = "<?hh //strict
function bar(Foo $foo): void {}
"

let baz_contents = "<?hh //strict
function baz(Foo $foo) : void {
  $foo->f();
}
"

let () =
  let env = Test.setup_server () in
  let env = Test.setup_disk env [
    "foo.php", foo_contents;
    "bar.php", bar_contents;
  ] in

  let classes = [ "\\Foo"; "\\Bar" ] in

  (* Remove things from shared memory to simulate lazy saved state init. *)
  let defs = { FileInfo.empty_names with
    FileInfo.n_classes = SSet.of_list classes
  } in
  let elems = Decl_class_elements.get_for_classes ~old:false classes in
  Decl_redecl_service.remove_defs ~collect_garbage:false defs elems;
  SharedMem.invalidate_caches();

  let tcopt = TypecheckerOptions.default in
  (* bar uses Foo but not its members. Hence they would remain deferred (see
   * usage of "lazy" in Decl_class.to_class_type). We need to ensure that if
   * we remove what they refer to (which declare_and_check does at the end,
   * by popping LocalChanges) we remove Foo too ...*)
  ServerIdeUtils.declare_and_check bar_contents ~f:(fun _ _ _ -> ()) tcopt;
  (* ... but we used to fail to do this fully - Foo could escape through
   * Typing_heap.Classes.Cache, which was not invalidated.
   * During analysis of baz below, we would retrieve Foo and try to use its
   * members, which are missing, crashing the server *)
  ServerIdeUtils.declare_and_check baz_contents ~f:(fun _ _ _ -> ()) tcopt;
  ignore env
