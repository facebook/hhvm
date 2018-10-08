open Integration_test_base_types

module Test = Integration_test_base

let bar_contents = Printf.sprintf {|<?hh
class %s {
  public function bar() : int {
    // UNSAFE_BLOCK
  }
}

|}

let foo_contents = Printf.sprintf {|<?hh
function foo() : %s {
  // UNSAFE_BLOCK
}
|}

let test_contents = Printf.sprintf {|<?hh
function test(): int {
  return foo()->bar();
}
|}

let save_state temp_dir =
  (* A and B are identical classes with a function bar(). foo() returns A,
   * and test contains an invocation to A::bar ()) *)
  Test.save_state [
    "A.php", bar_contents "A";
    "B.php", bar_contents "B";
    "foo.php", foo_contents "A";
    "test.php", test_contents;
  ] temp_dir

let test saved_state_dir test_request assert_response () =
  (* Change to foo changes A::bar() invocation in test to actually be B::bar() *)
  let env = Test.load_state
    saved_state_dir
    ~disk_state:[
      "A.php", bar_contents "A";
      "B.php", bar_contents "B";
      "foo.php", foo_contents "B";
      "test.php", test_contents;
    ]
    ~master_changes:["foo.php"]
    ~local_changes:[]
    ~use_precheked_files:true
  in

  let env, _ = Test.full_check env in
  let env = Test.connect_persistent_client env in
  (* Initially, "test.php" is prechecked, so no need to check it *)
  Test.assert_needs_no_recheck env "test.php";

  (* But the above below request will infer that test.php is a possible place where
   * B::bar can occur. *)
  let env, loop_output = Test.(run_loop_once env { default_loop_input with
    persistent_client_request = Some (Request test_request)
  }) in

  (* Which puts test.php into needs_recheck and asks client to retry find refs later *)
  Test.assert_needs_recheck env "test.php";
  Test.assert_needs_retry loop_output;

  (* After full check, retried request succeeds and returns expected results *)
  let env, _ = Test.full_check env in
  let env, loop_output = Test.(run_loop_once env { default_loop_input with
    persistent_client_request = Some (Request test_request)
  }) in
  assert_response loop_output;
  ignore env;
  ()

let () = Tempfile.with_real_tempdir @@ fun temp_dir ->
  let temp_dir = Path.to_string temp_dir in
  save_state temp_dir;

  (* Run each test in separate process to ensure clean slate start. *)
  Test.in_daemon @@ test temp_dir
    ServerCommandTypes.(FIND_REFS  Find_refs.(Member("\\B", Method "bar")))
    (fun x -> Test.assert_find_refs x [{|B::bar: File "/test.php", line 3, characters 17-19:|}]);

  Test.in_daemon @@ test temp_dir
    ServerCommandTypes.((IDE_FIND_REFS (LabelledFileName "/B.php", 3, 22, true)))
    (fun x -> Test.assert_ide_find_refs x "B::bar" [
      {|File "/B.php", line 3, characters 19-21:|};
      {|File "/test.php", line 3, characters 17-19:|}
    ]);

  Test.in_daemon @@ test temp_dir
    ServerCommandTypes.(REFACTOR ServerRefactorTypes.( MethodRename {
      class_name = "B";
      old_name = "bar";
      new_name = "baz";
      filename = None;
      definition = None;
    }))
    (fun x -> Test.assert_refactor x
      ({|[{"filename":"/test.php","patches":[{"char_start":44,"char_end":47,"line":3,"col_start":17,"col_end":19,"patch_type":"replace","replacement":"baz"}]},|} ^
        {|{"filename":"/B.php","patches":[{"char_start":33,"char_end":36,"line":3,"col_start":19,"col_end":21,"patch_type":"replace","replacement":"baz"}]}]|})
    );

  Test.in_daemon @@ test temp_dir
    ServerCommandTypes.(IDE_REFACTOR Ide_refactor_type.({
      filename = "/B.php";
      line = 3;
      char = 22;
      new_name = "baz";
    }))
    (fun x -> Test.assert_ide_refactor x
      ({|[{"filename":"/test.php","patches":[{"char_start":44,"char_end":47,"line":3,"col_start":17,"col_end":19,"patch_type":"replace","replacement":"baz"}]},|} ^
        {|{"filename":"/B.php","patches":[{"char_start":33,"char_end":36,"line":3,"col_start":19,"col_end":21,"patch_type":"replace","replacement":"baz"},|} ^
        {|{"char_start":15,"char_end":15,"line":3,"col_start":1,"col_end":1,"patch_type":"insert","replacement":|} ^
        {|"\n  <<__Deprecated(\"Use `baz` instead\")>>\n  public function bar() : int {\n    return $this->baz();\n  }\n"}]}]|})
    );
  ()
