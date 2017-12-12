open Hh_core
(**
 * Tests documenting various invariants about the order of things coming out
 * of Errors module. Some of them are __probably__ not necessary for
 * correctness, but if you break them you will still change a bunch of tests
 * outputs and will have to spend time wondering whether the changes are
 * significant or not.
 **)

let error_list_to_string_buffer buf x =
  List.iter x ~f: begin fun error ->
    Printf.bprintf buf "%s\n"  Errors.(error |> to_absolute |> to_string)
  end

let error_list_to_string errors =
  let buf = Buffer.create 1024 in
  error_list_to_string_buffer buf errors;
  Buffer.contents buf

let create_path x = Relative_path.(create Root ("/" ^ x))

let error_in file =
  Errors.parsing_error (Pos.make_from (create_path file), "")

let expect_error_in =
  Printf.sprintf
    "File \"/%s\", line 0, characters 0--1:\n (Parsing[1002])\n\n"

let test_do () =
  let errors, (), _ = Errors.do_ begin fun () ->
    error_in "A";
    error_in "B";
    ()
  end in
  let expected = (expect_error_in "A") ^ (expect_error_in "B") in
  Asserter.String_asserter.assert_equals expected
    (Errors.get_error_list errors |> error_list_to_string )
    "Errors should be returned from do_ in the order they were added";
  Asserter.String_asserter.assert_equals expected
    (Errors.get_sorted_error_list errors |> error_list_to_string )
    "get_sorted_error_list should sort errors by filename";
  true

let test_get_sorted_error_list () =
  let errors, (), _ = Errors.do_ begin fun () ->
    error_in "B";
    error_in "A";
    ()
  end in
  let expected = (expect_error_in "B") ^ (expect_error_in "A") in
  Asserter.String_asserter.assert_equals expected
    (Errors.get_error_list errors |> error_list_to_string )
    "Errors should be returned from do_ in the order they were added";

  let expected = (expect_error_in "A") ^ (expect_error_in "B") in
  Asserter.String_asserter.assert_equals expected
    (Errors.get_sorted_error_list errors |> error_list_to_string )
    "get_sorted_error_list should sort errors by filename";
  true

let test_try () =
  let error_ref = ref None in
  let () = Errors.try_
    begin fun () ->
      error_in "A";
      error_in "B";
      ()
    end
    begin fun error ->
      error_ref := Some error;
      ()
    end
  in
  let expected = expect_error_in "A" in
  match !error_ref with
  | None -> failwith "Expected error handler to run"
  | Some e ->
    Asserter.String_asserter.assert_equals expected
      (error_list_to_string [e])
      "Errors.try_ should call the handler with first encountered error";
  true

let test_merge () =
  let errors1, (), _ = Errors.do_ begin fun () ->
    error_in "A";
    error_in "B";
    ()
  end in
  let errors2, (), _ = Errors.do_ begin fun () ->
    error_in "C";
    error_in "D";
    ()
  end in

  let errors = Errors.merge errors1 errors2 in
  let expected = (expect_error_in "B") ^ (expect_error_in "A")
    ^ (expect_error_in "C")^ (expect_error_in "D") in
  Asserter.String_asserter.assert_equals expected
    (Errors.get_error_list errors |> error_list_to_string )
    "Errors.merge behaves like List.rev_append";
  true

let test_from_error_list () =
  let errors, (), _ = Errors.do_ begin fun () ->
    error_in "A";
    error_in "B";
    ()
  end in
  let errors = Errors.get_error_list errors in
  let expected = error_list_to_string errors in
  let errors = Errors.(errors |> from_error_list |> get_error_list) in
  Asserter.String_asserter.assert_equals expected
    (errors |> error_list_to_string )
    "get_error_list(from_error_list(x)) == x";
  Asserter.Bool_asserter.assert_equals true
    Errors.([] |> from_error_list |> is_empty)
    "is_empty(from_error_list([])) == true";
  true

let test_phases () =
  let a_path = create_path "A" in
  let errors, (), _ = Errors.do_ begin fun () ->
    Errors.run_in_context a_path Errors.Parsing begin fun () ->
      Errors.parsing_error (Pos.make_from a_path, "");
    end;
    Errors.run_in_context a_path Errors.Typing begin fun () ->
      Errors.typing_error (Pos.make_from a_path) "";
    end;
    ()
  end in
  let expected =
    "File \"/A\", line 0, characters 0--1:\n (Parsing[1002])\n\n" ^
    "File \"/A\", line 0, characters 0--1:\n (Typing[4116])\n\n"
  in
  Asserter.String_asserter.assert_equals expected
    (Errors.get_error_list errors |> error_list_to_string )
    "Errors from earlier phase should come first";
  true

let tests = [
  "test", test_do;
  "test_get_sorted_error_list", test_get_sorted_error_list;
  "test_try", test_try;
  "test_merge", test_merge;
  "test_from_error_list", test_from_error_list;
  "test_phases", test_phases;
]

let () =
  Relative_path.(set_path_prefix Root (Path.make "/"));
  Unit_test.run_all tests
