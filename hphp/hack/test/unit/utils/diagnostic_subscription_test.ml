open Hh_core
open Reordered_argument_collections

module DS = Diagnostic_subscription

let errors_to_string buf x =
  List.iter x ~f: begin fun error ->
    Printf.bprintf buf "%s\n" (Errors.to_string error)
  end

let diagnostics_to_string x =
  let buf = Buffer.create 1024 in
  SMap.iter x ~f:begin fun path errors ->
    Printf.bprintf buf "%s:\n" path;
    errors_to_string buf errors;
  end;
  Buffer.contents buf

let error_list_to_string_buffer buf x =
  List.iter x ~f: begin fun error ->
    Printf.bprintf buf "%s\n"  Errors.(error |> to_absolute |> to_string)
  end

let error_list_to_string errors =
  let buf = Buffer.create 1024 in
  error_list_to_string_buffer buf errors;
  Buffer.contents buf

let create_path x = Relative_path.(create Root ("/" ^ x))

let error_in path message = Errors.parsing_error ((Pos.make_from path), message)

let test_update () =
  let a_path = create_path "A" in
  let b_path = create_path "B" in

  let foo_error_a, (), _ =
  Errors.do_with_context a_path Errors.Parsing begin fun () ->
    error_in a_path "foo";
    ()
  end in
  let bar_error_a, (), _ =
    Errors.do_with_context a_path Errors.Parsing begin fun () ->
    error_in a_path "bar";
    ()
  end in
  let baz_error_b, (), _ =
    Errors.do_with_context b_path Errors.Parsing begin fun () ->
    error_in b_path "baz";
    ()
  end in

  let ds = DS.of_id ~id:1 ~init:Errors.empty in

  let priority_files = Relative_path.Set.empty in
  let rechecked_a = Relative_path.Map.singleton a_path FileInfo.empty_names in
  let rechecked_b = Relative_path.Map.singleton b_path FileInfo.empty_names in

  let ds, diagnostics =
    DS.update ds priority_files rechecked_a foo_error_a |> DS.pop_errors in
  let expected =
    "/A:\nFile \"/A\", line 0, characters 0--1:\nfoo (Parsing[1002])\n\n" in
  Asserter.String_asserter.assert_equals expected
    (diagnostics_to_string diagnostics)
    "foo error in A should be pushed";

  let ds, diagnostics =
    DS.update ds priority_files rechecked_a foo_error_a |> DS.pop_errors in
  Asserter.Bool_asserter.assert_equals true
    (SMap.is_empty diagnostics)
    "Unchanged diagnostics in A should be not pushed again";

  let expected =
    "/A:\nFile \"/A\", line 0, characters 0--1:\nbar (Parsing[1002])\n\n" in
  let ds, diagnostics =
    DS.update ds priority_files rechecked_a bar_error_a |> DS.pop_errors in
  Asserter.String_asserter.assert_equals expected
    (diagnostics_to_string diagnostics)
    "foo error in A should be replaced with bar";

  let expected =
    "/B:\nFile \"/B\", line 0, characters 0--1:\nbaz (Parsing[1002])\n\n" in
  let ds, diagnostics =
    DS.update ds priority_files rechecked_b baz_error_b |> DS.pop_errors in
  Asserter.String_asserter.assert_equals expected
    (diagnostics_to_string diagnostics)
    "baz error in B should be added";

  let ds, diagnostics =
    DS.update ds priority_files rechecked_a Errors.empty |> DS.pop_errors in
  let expected = "/A:\n" in
  Asserter.String_asserter.assert_equals expected
    (diagnostics_to_string diagnostics)
    "A diagnostics should be cleared";

  ignore ds;
  true

let test_error_sources () =
  let a_path = create_path "A" in
  let b_path = create_path "B" in

  let errors, (), _ =
    Errors.do_ begin fun () ->
      Errors.run_in_context a_path Errors.Typing begin fun () ->
        error_in a_path "error from a";
      end;
      Errors.run_in_context b_path Errors.Typing begin fun () ->
        error_in a_path "error from b";
      end;
    end
  in

  let ds = DS.of_id ~id:1 ~init:Errors.empty in

  let priority_files = Relative_path.Set.empty in
  let rechecked = Relative_path.Map.(
    (singleton a_path FileInfo.empty_names) |> union
    (singleton b_path FileInfo.empty_names)
  ) in

  let ds = DS.update ds priority_files rechecked errors  in

  Asserter.Bool_asserter.assert_equals true
    (Relative_path.Set.mem (DS.files_with_errors_in_ide ds) a_path)
    "files_with_errors_in_ide should contain A";

  Asserter.Bool_asserter.assert_equals false (* TODO: this should be
    true and can lead to errors missing in ide *)
    (Relative_path.Set.mem (DS.files_with_errors_in_ide ds) b_path)
    "files_with_errors_in_ide should contain B";
  true

let tests = [
  "test_update", test_update;
  "test_error_sources", test_error_sources;
]

let () =
  Relative_path.(set_path_prefix Root (Path.make "/"));
  Unit_test.run_all tests
