(**
 * Tests documenting various invariants about the order of things coming out
 * of Errors module. Some of them are __probably__ not necessary for
 * correctness, but if you break them you will still change a bunch of tests
 * outputs and will have to spend time wondering whether the changes are
 * significant or not.
 **)

open Hh_prelude

let error_list_to_string_buffer buf x =
  List.iter x ~f:(fun error ->
      Printf.bprintf
        buf
        "%s\n"
        (error |> User_error.to_absolute |> Errors.to_string))

let error_list_to_string errors =
  let buf = Buffer.create 1024 in
  error_list_to_string_buffer buf errors;
  Buffer.contents buf

let create_path x = Relative_path.(create Root ("/" ^ x))

let error_in file =
  Errors.add_parsing_error
  @@ Parsing_error.Parsing_error
       { pos = Pos.make_from (create_path file); msg = ""; quickfixes = [] }

let expect_error_in =
  Printf.sprintf "File \"/%s\", line 0, characters 0-0:\n (Parsing[1002])\n\n"

let test_do () =
  let (errors, ()) =
    Errors.do_ (fun () ->
        error_in "A";
        error_in "B";
        ())
  in
  let expected = expect_error_in "A" ^ expect_error_in "B" in
  Asserter.String_asserter.assert_equals
    expected
    (Errors.get_error_list errors |> error_list_to_string)
    "Errors should be returned from do_ in the order they were added";
  Asserter.String_asserter.assert_equals
    expected
    (Errors.get_sorted_error_list errors |> error_list_to_string)
    "get_sorted_error_list should sort errors by filename";
  true

let expected_unsorted =
  {|File "/FileWithErrors.php", line 1, characters 4-7:
This value is not a valid key type for this container (Typing[4324])
  File "/C2", line 0, characters 0-0:
  This container is C2_Type
  File "/K2", line 0, characters 0-0:
  K2_Type cannot be used as a key for C2_Type

File "/FileWithErrors.php", line 1, characters 4-7:
This value is not a valid key type for this container (Typing[4324])
  File "/C1", line 0, characters 0-0:
  This container is C1_Type
  File "/K1", line 0, characters 0-0:
  K1_Type cannot be used as a key for C1_Type

File "/FileWithErrors.php", line 0, characters 0-0:
 (Parsing[1002])

File "/FileWithErrors.php", line 1, characters 4-7:
This value is not a valid key type for this container (Typing[4324])
  File "/C2", line 0, characters 0-0:
  This container is C2_Type
  File "/K2", line 0, characters 0-0:
  K2_Type cannot be used as a key for C2_Type

File "/FileWithErrors.php", line 1, characters 4-7:
This value is not a valid key type for this container (Typing[4324])
  File "/C1", line 0, characters 0-0:
  This container is C1_Type
  File "/K1", line 0, characters 0-0:
  K1_Type cannot be used as a key for C1_Type

|}

let expected_sorted =
  {|File "/FileWithErrors.php", line 0, characters 0-0:
 (Parsing[1002])

File "/FileWithErrors.php", line 1, characters 4-7:
This value is not a valid key type for this container (Typing[4324])
  File "/C1", line 0, characters 0-0:
  This container is C1_Type
  File "/K1", line 0, characters 0-0:
  K1_Type cannot be used as a key for C1_Type

File "/FileWithErrors.php", line 1, characters 4-7:
This value is not a valid key type for this container (Typing[4324])
  File "/C2", line 0, characters 0-0:
  This container is C2_Type
  File "/K2", line 0, characters 0-0:
  K2_Type cannot be used as a key for C2_Type

|}

let test_get_sorted_error_list () =
  let (errors, ()) =
    Errors.do_ (fun () ->
        error_in "B";
        error_in "A";
        ())
  in
  let expected = expect_error_in "B" ^ expect_error_in "A" in
  Asserter.String_asserter.assert_equals
    expected
    (Errors.get_error_list errors |> error_list_to_string)
    "Errors should be returned from do_ in the order they were added";

  let expected = expect_error_in "A" ^ expect_error_in "B" in
  Asserter.String_asserter.assert_equals
    expected
    (Errors.get_sorted_error_list errors |> error_list_to_string)
    "get_sorted_error_list should sort errors by filename";

  let file_with_errors = create_path "FileWithErrors.php" in
  let err_pos =
    Pos.make_from_lnum_bol_offset
      ~pos_file:file_with_errors
      ~pos_start:(1, 5, 8)
      ~pos_end:(2, 10, 12)
  in
  Printf.printf "%s" (Pos.print_verbose_relative err_pos);
  let container_pos1 =
    Pos.make_from (create_path "C1") |> Pos_or_decl.of_raw_pos
  in
  let container_pos2 =
    Pos.make_from (create_path "C2") |> Pos_or_decl.of_raw_pos
  in
  let key_pos1 = Pos.make_from (create_path "K1") |> Pos_or_decl.of_raw_pos in
  let key_pos2 = Pos.make_from (create_path "K2") |> Pos_or_decl.of_raw_pos in
  let (errors, ()) =
    Errors.do_with_context file_with_errors Errors.Typing (fun () ->
        Errors.add_typing_error
          Typing_error.(
            primary
            @@ Primary.Invalid_arraykey
                 {
                   pos = err_pos;
                   ctxt = `read;
                   container_pos = container_pos2;
                   container_ty_name = lazy "C2_Type";
                   key_pos = key_pos2;
                   key_ty_name = lazy "K2_Type";
                 });
        Errors.add_typing_error
          Typing_error.(
            primary
            @@ Primary.Invalid_arraykey
                 {
                   pos = err_pos;
                   ctxt = `read;
                   container_pos = container_pos1;
                   container_ty_name = lazy "C1_Type";
                   key_pos = key_pos1;
                   key_ty_name = lazy "K1_Type";
                 });
        error_in "FileWithErrors.php";
        Errors.add_typing_error
          Typing_error.(
            primary
            @@ Primary.Invalid_arraykey
                 {
                   pos = err_pos;
                   ctxt = `read;
                   container_pos = container_pos2;
                   container_ty_name = lazy "C2_Type";
                   key_pos = key_pos2;
                   key_ty_name = lazy "K2_Type";
                 });
        Errors.add_typing_error
          Typing_error.(
            primary
            @@ Primary.Invalid_arraykey
                 {
                   pos = err_pos;
                   ctxt = `read;
                   container_pos = container_pos1;
                   container_ty_name = lazy "C1_Type";
                   key_pos = key_pos1;
                   key_ty_name = lazy "K1_Type";
                 });
        ())
  in
  Asserter.String_asserter.assert_equals
    expected_unsorted
    (Errors.get_error_list errors |> error_list_to_string)
    "Errors should be returned in the order they were added";

  Asserter.String_asserter.assert_equals
    expected_sorted
    (Errors.get_sorted_error_list errors |> error_list_to_string)
    "get_sorted_error_list should sort errors by position, code, and warrant";

  true

let test_try () =
  let error_ref = ref None in
  let () =
    Errors.try_
      (fun () ->
        error_in "A";
        error_in "B";
        ())
      (fun error ->
        error_ref := Some error;
        ())
  in
  let expected = expect_error_in "A" in
  match !error_ref with
  | None -> failwith "Expected error handler to run"
  | Some e ->
    Asserter.String_asserter.assert_equals
      expected
      (error_list_to_string [e])
      "Errors.try_ should call the handler with first encountered error";
    true

let test_merge () =
  let (errors1, ()) =
    Errors.do_ (fun () ->
        error_in "A";
        error_in "B";
        ())
  in
  let (errors2, ()) =
    Errors.do_ (fun () ->
        error_in "C";
        error_in "D";
        ())
  in
  let errors = Errors.merge errors1 errors2 in
  let expected =
    expect_error_in "B"
    ^ expect_error_in "A"
    ^ expect_error_in "C"
    ^ expect_error_in "D"
  in
  Asserter.String_asserter.assert_equals
    expected
    (Errors.get_error_list errors |> error_list_to_string)
    "Errors.merge behaves like List.rev_append";

  let errors = Errors.merge errors1 Errors.empty in
  let expected = expect_error_in "B" ^ expect_error_in "A" in
  Asserter.String_asserter.assert_equals
    expected
    (Errors.get_error_list errors |> error_list_to_string)
    "Errors.merge behaves like List.rev_append";

  let errors = Errors.merge Errors.empty errors2 in
  let expected = expect_error_in "C" ^ expect_error_in "D" in
  Asserter.String_asserter.assert_equals
    expected
    (Errors.get_error_list errors |> error_list_to_string)
    "Errors.merge behaves like List.rev_append";
  true

let test_from_error_list () =
  let (errors, ()) =
    Errors.do_ (fun () ->
        error_in "A";
        error_in "B";
        ())
  in
  let errors = Errors.get_error_list errors in
  let expected = error_list_to_string errors in
  let errors = Errors.(errors |> from_error_list |> get_error_list) in
  Asserter.String_asserter.assert_equals
    expected
    (errors |> error_list_to_string)
    "get_error_list(from_error_list(x)) == x";
  Asserter.Bool_asserter.assert_equals
    true
    Errors.([] |> from_error_list |> is_empty)
    "is_empty(from_error_list([])) == true";
  true

let test_phases () =
  let a_path = create_path "A" in
  let (errors, ()) =
    Errors.do_ (fun () ->
        Errors.run_in_context a_path Errors.Typing (fun () ->
            Errors.add_parsing_error
            @@ Parsing_error.Parsing_error
                 { pos = Pos.make_from a_path; msg = ""; quickfixes = [] });
        Errors.run_in_context a_path Errors.Typing (fun () ->
            Errors.add_typing_error
              Typing_error.(
                primary
                @@ Primary.Generic_unify
                     { pos = Pos.make_from a_path; msg = "" }));
        ())
  in
  let expected =
    "File \"/A\", line 0, characters 0-0:\n (Parsing[1002])\n\n"
    ^ "File \"/A\", line 0, characters 0-0:\n (Typing[4116])\n\n"
  in
  Asserter.String_asserter.assert_equals
    expected
    (Errors.get_error_list errors |> error_list_to_string)
    "Errors from earlier phase should come first";
  true

let test_incremental_update () =
  let a_path = create_path "A" in
  let b_path = create_path "B" in
  let (foo_error_a, ()) =
    Errors.do_with_context a_path Errors.Typing (fun () ->
        error_in "foo1";
        error_in "foo2";
        ())
  in
  let (bar_error_a, ()) =
    Errors.do_with_context a_path Errors.Typing (fun () ->
        error_in "bar1";
        error_in "bar2";
        ())
  in
  let (baz_error_b, ()) =
    Errors.do_with_context b_path Errors.Typing (fun () ->
        error_in "baz1";
        error_in "baz2";
        ())
  in
  let errors =
    Errors.incremental_update
      ~old:foo_error_a
      ~new_:bar_error_a
      ~rechecked:(Relative_path.Set.singleton a_path)
      Errors.Typing
  in
  let expected =
    "File \"/bar2\", line 0, characters 0-0:\n (Parsing[1002])\n\n"
    ^ "File \"/bar1\", line 0, characters 0-0:\n (Parsing[1002])\n\n"
  in
  Asserter.String_asserter.assert_equals
    expected
    (Errors.get_error_list errors |> error_list_to_string)
    "Incremental update should overwrite foo error with bar.";

  let errors =
    Errors.incremental_update
      ~old:foo_error_a
      ~new_:baz_error_b
      ~rechecked:(Relative_path.Set.singleton b_path)
      Errors.Typing
  in
  let expected =
    "File \"/foo1\", line 0, characters 0-0:\n (Parsing[1002])\n\n"
    ^ "File \"/foo2\", line 0, characters 0-0:\n (Parsing[1002])\n\n"
    ^ "File \"/baz2\", line 0, characters 0-0:\n (Parsing[1002])\n\n"
    ^ "File \"/baz1\", line 0, characters 0-0:\n (Parsing[1002])\n\n"
  in
  Asserter.String_asserter.assert_equals
    expected
    (Errors.get_error_list errors |> error_list_to_string)
    "Incremental update should add baz error and leave foo error unchanged";

  let errors =
    Errors.incremental_update
      ~old:foo_error_a
      ~new_:Errors.empty
      ~rechecked:(Relative_path.Set.singleton a_path)
      Errors.Typing
  in
  Asserter.Bool_asserter.assert_equals
    true
    (Errors.is_empty errors)
    "Incremental update should clear errors if a rechecked file has no errors";
  true

let test_merge_into_current () =
  let (errors1, ()) =
    Errors.do_ (fun () ->
        error_in "A";
        error_in "B";
        error_in "C";
        error_in "D";
        error_in "E";
        error_in "F";
        ())
  in
  let expected =
    expect_error_in "A"
    ^ expect_error_in "B"
    ^ expect_error_in "C"
    ^ expect_error_in "D"
    ^ expect_error_in "E"
    ^ expect_error_in "F"
  in
  let error_message =
    "merge_into_current should behave as if the code that "
    ^ "generated errors was inlined at the callsite."
  in
  Asserter.String_asserter.assert_equals
    expected
    (Errors.get_error_list errors1 |> error_list_to_string)
    error_message;

  let (sub_errors, ()) =
    Errors.do_ (fun () ->
        error_in "C";
        error_in "D";
        ())
  in
  let (errors2, ()) =
    Errors.do_ (fun () ->
        Errors.merge_into_current sub_errors;
        ())
  in
  let expected2 = expect_error_in "C" ^ expect_error_in "D" in
  Asserter.String_asserter.assert_equals
    expected2
    (Errors.get_error_list errors2 |> error_list_to_string)
    error_message;

  let (errors, ()) =
    Errors.do_ (fun () ->
        error_in "A";
        error_in "B";
        Errors.merge_into_current sub_errors;
        error_in "E";
        error_in "F";
        ())
  in
  Asserter.String_asserter.assert_equals
    expected
    (Errors.get_error_list errors |> error_list_to_string)
    error_message;
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
      let path = string_of_int n ^ ".php" in
      let (errors, ()) =
        Errors.(do_with_context (create_path path) Typing) (fun () ->
            error_in path)
      in
      (* note argument order: small first, big second *)
      aux (Errors.merge errors acc) (n - 1)
  in
  let errors = aux Errors.empty n in
  List.length (Errors.get_error_list errors) = n

let tests =
  [
    ("test", test_do);
    ("test_get_sorted_error_list", test_get_sorted_error_list);
    ("test_try", test_try);
    ("test_merge", test_merge);
    ("test_from_error_list", test_from_error_list);
    ("test_phases", test_phases);
    (* TODO T44055462 please amend test to maintain new invariants of error API
       "test_incremental_update", test_incremental_update; *)
    ("test_merge_into_current", test_merge_into_current);
    ("test_performance", test_performance);
  ]

let () =
  Relative_path.(set_path_prefix Root (Path.make "/"));
  Unit_test.run_all tests
