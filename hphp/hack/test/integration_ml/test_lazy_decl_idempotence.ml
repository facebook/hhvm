module Test = Integration_test_base

let foo_name = "\\Foo"

let bar_name = "\\Bar"

let foo_file_name = "foo.php"

let bar_file_name = "bar.php"

let foo_contents =
  {|<?hh //strict

const string GLOBAL_X = "X";

class Foo {
  // this is an error (lack of typehint) revealed during declaration only
  const X = GLOBAL_X;
}
|}

let bar_contents =
  {|<?hh //strict
class Bar {
  public function test(Foo $x) : void {}
}
|}

let expected_errors =
  {|
File "/foo.php", line 7, characters 9-9:
Please add a type hint (Naming[2035])
|}

let test () =
  let env = Test.setup_server () in
  let env =
    Test.setup_disk
      env
      [(foo_file_name, foo_contents); (bar_file_name, bar_contents)]
  in
  let ctx = Provider_utils.ctx_from_server_env env in
  let classes = [foo_name; bar_name] in
  let foo_path = Relative_path.from_root foo_file_name in
  let bar_path = Relative_path.from_root bar_file_name in
  (* Remove things from shared memory (that were put there by Test.setup_disk)
   * to simulate lazy saved state init. *)
  let defs =
    { FileInfo.empty_names with FileInfo.n_classes = SSet.of_list classes }
  in
  let elems = Decl_class_elements.get_for_classes ~old:false classes in
  Decl_redecl_service.remove_defs ctx ~collect_garbage:false defs elems;
  SharedMem.invalidate_caches ();

  (* Local caches need to be invalidated whenever things are removed from shared
   * memory (to avoid getting cached old versions of declarations) *)
  let empty = Relative_path.Set.empty in
  let memory_cap = None in
  let check_info =
    {
      Typing_check_service.init_id = "";
      recheck_id = Some "";
      profile_log = false;
      profile_type_check_twice = false;
      profile_type_check_duration_threshold = 0.;
    }
  in
  let delegate_state = Typing_service_delegate.create () in
  let (errors, delegate_state, telemetry) =
    Typing_check_service.go
      ctx
      None
      delegate_state
      (Telemetry.create ())
      empty
      [bar_path]
      ~memory_cap
      ~check_info
  in
  Test.assert_errors errors "";
  let (errors, delegate_state, telemetry) =
    Typing_check_service.go
      ctx
      None
      delegate_state
      telemetry
      empty
      [bar_path]
      ~memory_cap
      ~check_info
  in
  Test.assert_errors errors "";

  let (errors, delegate_state, telemetry) =
    Typing_check_service.go
      ctx
      None
      delegate_state
      telemetry
      empty
      [foo_path]
      ~memory_cap
      ~check_info
  in
  Test.assert_errors errors expected_errors;
  let (errors, _delegate_state, _telemetry) =
    Typing_check_service.go
      ctx
      None
      delegate_state
      telemetry
      empty
      [foo_path]
      ~memory_cap
      ~check_info
  in
  Test.assert_errors errors expected_errors;

  ignore env;
  ()
