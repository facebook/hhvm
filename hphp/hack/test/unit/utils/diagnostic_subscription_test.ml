open Hh_prelude
open Reordered_argument_collections

let errors_to_string buf x =
  List.iter x ~f:(fun error ->
      Printf.bprintf buf "%s\n" (Errors.to_string error))

let diagnostics_to_string x =
  let buf = Buffer.create 1024 in
  SMap.iter x ~f:(fun path errors ->
      Printf.bprintf buf "%s:\n" path;
      errors_to_string buf errors);
  Buffer.contents buf

let error_list_to_string_buffer buf x =
  List.iter x ~f:(fun error ->
      Printf.bprintf buf "%s\n" Errors.(error |> to_absolute |> to_string))

let error_list_to_string errors =
  let buf = Buffer.create 1024 in
  error_list_to_string_buffer buf errors;
  Buffer.contents buf

let create_path x = Relative_path.(create Root ("/" ^ x))

let error_in path message = Errors.parsing_error (Pos.make_from path, message)

let test_update () =
  let a_path = create_path "A" in
  let b_path = create_path "B" in
  let (foo_error_a, ()) =
    Errors.do_with_context a_path Errors.Parsing (fun () ->
        error_in a_path "foo";
        ())
  in
  let (bar_error_a, ()) =
    Errors.do_with_context a_path Errors.Parsing (fun () ->
        error_in a_path "bar";
        ())
  in
  let bar_error_a =
    Errors.incremental_update_set
      ~old:foo_error_a
      ~new_:bar_error_a
      ~rechecked:(Relative_path.Set.singleton a_path)
      Errors.Parsing
  in
  let (baz_error_b, ()) =
    Errors.do_with_context b_path Errors.Parsing (fun () ->
        error_in b_path "baz";
        ())
  in
  let baz_error_b =
    Errors.incremental_update_set
      ~old:bar_error_a
      ~new_:baz_error_b
      ~rechecked:(Relative_path.Set.singleton b_path)
      Errors.Parsing
  in
  let bar_error_cleared_a =
    Errors.incremental_update_set
      ~old:baz_error_b
      ~new_:Errors.empty
      ~rechecked:(Relative_path.Set.singleton a_path)
      Errors.Parsing
  in
  let ds = Diagnostic_subscription.of_id 1 ~initial_errors:Errors.empty in
  let priority_files = Relative_path.Set.empty in
  let (ds, diagnostics, _is_truncated) =
    Diagnostic_subscription.update
      ds
      ~priority_files
      ~global_errors:foo_error_a
      ~full_check_done:true
    |> Diagnostic_subscription.pop_errors ~global_errors:foo_error_a
  in
  let expected =
    "/A:\nFile \"/A\", line 0, characters 0-0:\nfoo (Parsing[1002])\n\n"
  in
  Asserter.String_asserter.assert_equals
    expected
    (diagnostics_to_string diagnostics)
    "foo error in A should be pushed";

  let (ds, diagnostics, _is_truncated) =
    Diagnostic_subscription.update
      ds
      ~priority_files
      ~global_errors:foo_error_a
      ~full_check_done:false
    |> Diagnostic_subscription.pop_errors ~global_errors:foo_error_a
  in
  Asserter.Bool_asserter.assert_equals
    true
    (SMap.is_empty diagnostics)
    "Unchanged diagnostics in A should be not pushed again";

  let expected =
    "/A:\nFile \"/A\", line 0, characters 0-0:\nbar (Parsing[1002])\n\n"
  in
  let (ds, diagnostics, _is_truncated) =
    Diagnostic_subscription.update
      ds
      ~priority_files
      ~global_errors:bar_error_a
      ~full_check_done:false
    |> Diagnostic_subscription.pop_errors ~global_errors:bar_error_a
  in
  Asserter.String_asserter.assert_equals
    expected
    (diagnostics_to_string diagnostics)
    "foo error in A should be replaced with bar";

  let priority_files = Relative_path.Set.singleton b_path in
  let expected =
    "/B:\nFile \"/B\", line 0, characters 0-0:\nbaz (Parsing[1002])\n\n"
  in
  let (ds, diagnostics, _is_truncated) =
    Diagnostic_subscription.update
      ds
      ~priority_files
      ~global_errors:baz_error_b
      ~full_check_done:false
    |> Diagnostic_subscription.pop_errors ~global_errors:baz_error_b
  in
  Asserter.String_asserter.assert_equals
    expected
    (diagnostics_to_string diagnostics)
    "baz error in B should be added";

  let (ds, diagnostics, _is_truncated) =
    Diagnostic_subscription.update
      ds
      ~priority_files
      ~global_errors:bar_error_cleared_a
      ~full_check_done:false
    |> Diagnostic_subscription.pop_errors ~global_errors:bar_error_cleared_a
  in
  let expected = "/A:\n" in
  Asserter.String_asserter.assert_equals
    expected
    (diagnostics_to_string diagnostics)
    "A diagnostics should be cleared";

  ignore ds;
  true

let test_error_sources () =
  let a_path = create_path "A" in
  let b_path = create_path "B" in
  let (errors, ()) =
    Errors.do_ (fun () ->
        Errors.run_in_context a_path Errors.Typing (fun () ->
            error_in a_path "error from a");
        Errors.run_in_context b_path Errors.Typing (fun () ->
            error_in a_path "error from b"))
  in
  let ds = Diagnostic_subscription.of_id 1 ~initial_errors:Errors.empty in
  let priority_files = Relative_path.Set.empty in
  let ds =
    Diagnostic_subscription.update
      ds
      ~priority_files
      ~global_errors:errors
      ~full_check_done:true
  in
  Asserter.Bool_asserter.assert_equals
    true
    (Relative_path.Set.mem (Diagnostic_subscription.diagnosed_files ds) a_path)
    "diagnosed_files should contain A";

  Asserter.Bool_asserter.assert_equals
    true
    (Relative_path.Set.mem (Diagnostic_subscription.diagnosed_files ds) b_path)
    "diagnosed_files should contain B";
  true

let tests =
  [("test_update", test_update); ("test_error_sources", test_error_sources)]

let () =
  EventLogger.init_fake ();
  Relative_path.(set_path_prefix Root (Path.make "/"));
  Unit_test.run_all tests
