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
    "File \"/%s\", line 0, characters 0-0:\n (Parsing[1002])\n\n"

let test_do () =
  let errors, () = Errors.do_ begin fun () ->
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
  let errors, () = Errors.do_ begin fun () ->
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
  let errors1, () = Errors.do_ begin fun () ->
    error_in "A";
    error_in "B";
    ()
  end in
  let errors2, () = Errors.do_ begin fun () ->
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

  let errors = Errors.merge errors1 Errors.empty in
  let expected = (expect_error_in "B") ^ (expect_error_in "A") in
  Asserter.String_asserter.assert_equals expected
    (Errors.get_error_list errors |> error_list_to_string )
    "Errors.merge behaves like List.rev_append";

  let errors = Errors.merge Errors.empty errors2 in
  let expected = (expect_error_in "C") ^ (expect_error_in "D") in
  Asserter.String_asserter.assert_equals expected
    (Errors.get_error_list errors |> error_list_to_string )
    "Errors.merge behaves like List.rev_append";
  true

let test_from_error_list () =
  let errors, () = Errors.do_ begin fun () ->
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
  let errors, () = Errors.do_ begin fun () ->
    Errors.run_in_context a_path Errors.Parsing begin fun () ->
      Errors.parsing_error (Pos.make_from a_path, "");
    end;
    Errors.run_in_context a_path Errors.Typing begin fun () ->
      Errors.typing_error (Pos.make_from a_path) "";
    end;
    ()
  end in
  let expected =
    "File \"/A\", line 0, characters 0-0:\n (Parsing[1002])\n\n" ^
    "File \"/A\", line 0, characters 0-0:\n (Typing[4116])\n\n"
  in
  Asserter.String_asserter.assert_equals expected
    (Errors.get_error_list errors |> error_list_to_string )
    "Errors from earlier phase should come first";
  true

let test_incremental_update () =
  let a_path = create_path "A" in
  let b_path = create_path "B" in

  let foo_error_a, () =
  Errors.do_with_context a_path Errors.Parsing begin fun () ->
    error_in "foo";
    ()
  end in
  let bar_error_a, () =
    Errors.do_with_context a_path Errors.Parsing begin fun () ->
    error_in "bar";
    ()
  end in
  let baz_error_b, () =
    Errors.do_with_context b_path Errors.Parsing begin fun () ->
    error_in "baz";
    ()
  end in

  let errors = Errors.incremental_update_set
    ~old:foo_error_a
    ~new_:bar_error_a
    ~rechecked:(Relative_path.Set.singleton a_path)
    Errors.Parsing
  in
  let expected =
    "File \"/bar\", line 0, characters 0-0:\n (Parsing[1002])\n\n" in
  Asserter.String_asserter.assert_equals expected
    (Errors.get_error_list errors |> error_list_to_string )
    "Incremental update should overwrite foo error with bar.";

  let errors = Errors.incremental_update_set
    ~old:foo_error_a
    ~new_:baz_error_b
    ~rechecked:(Relative_path.Set.singleton b_path)
    Errors.Parsing
  in
  let expected =
    "File \"/foo\", line 0, characters 0-0:\n (Parsing[1002])\n\n" ^
    "File \"/baz\", line 0, characters 0-0:\n (Parsing[1002])\n\n"
  in
  Asserter.String_asserter.assert_equals expected
    (Errors.get_error_list errors |> error_list_to_string )
    "Incremental update should add baz error and leave foo error unchanged";

  let errors = Errors.incremental_update_set
    ~old:foo_error_a
    ~new_:Errors.empty
    ~rechecked:(Relative_path.Set.singleton a_path)
    Errors.Parsing
  in
  Asserter.Bool_asserter.assert_equals true
    (Errors.is_empty errors)
    "Incremental update should clear errors if a rechecked file has no errors";
  true

let test_merge_into_current () =
  let errors1, () = Errors.do_ begin fun () ->
    error_in "A";
    error_in "B";
    error_in "C";
    error_in "D";
    error_in "E";
    error_in "F";
    ()
  end in

  let expected =
    (expect_error_in "A") ^ (expect_error_in "B") ^ (expect_error_in "C") ^
    (expect_error_in "D") ^ (expect_error_in "E") ^ (expect_error_in "F")
  in

  let error_message = "merge_into_current should behave as if the code that " ^
    "generated errors was inlined at the callsite." in

  Asserter.String_asserter.assert_equals expected
    (Errors.get_error_list errors1 |> error_list_to_string) error_message;

  let sub_errors, () = Errors.do_ begin fun () ->
    error_in "C";
    error_in "D";
    ()
  end in

  let errors2, () =  Errors.do_ begin fun () ->
    Errors.merge_into_current sub_errors;
    ()
  end in

  let expected2 = (expect_error_in "C") ^ (expect_error_in "D") in

  Asserter.String_asserter.assert_equals expected2
    (Errors.get_error_list errors2 |> error_list_to_string) error_message;

  let errors, () = Errors.do_ begin fun () ->
    error_in "A";
    error_in "B";
    Errors.merge_into_current sub_errors;
    error_in "E";
    error_in "F";
    ()
  end in

  Asserter.String_asserter.assert_equals expected
    (Errors.get_error_list errors |> error_list_to_string) error_message;
  true

(* Errors.merge is called on very critical paths in Parsing_service,
 * Decl_redecl_service, and Typing_check_service to merge partial results from
 * workers. If it's too slow, it delays scheduling of more jobs, and hurts
 * parallelism rate. All those callsites pass the second argument as the
 * accumulator, so the runtime needs to be proportional to the size of first
 * argument. *)
let test_performance () =
  let n = 1000000 in
  let rec aux acc = function
    | 0 -> acc
    | n ->
      let path = (string_of_int n) ^ ".php" in
      let errors, () =
        Errors.(do_with_context (create_path path) Typing) begin fun () ->
          error_in path
        end
      in
      (* note argument order: small first, big second *)
      aux (Errors.merge errors acc) (n-1)
  in
  let errors = aux Errors.empty n in
  List.length (Errors.get_error_list errors) == n

let tests = [
  "test", test_do;
  "test_get_sorted_error_list", test_get_sorted_error_list;
  "test_try", test_try;
  "test_merge", test_merge;
  "test_from_error_list", test_from_error_list;
  "test_phases", test_phases;
  "test_incremental_update", test_incremental_update;
  "test_merge_into_current", test_merge_into_current;
  "test_performance", test_performance;
]

let () =
  Relative_path.(set_path_prefix Root (Path.make "/"));
  Unit_test.run_all tests
