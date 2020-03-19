open Asserter

let test_delayed_future () =
  let future = Future.delayed_value ~delays:3 "Delayed value" in
  Bool_asserter.assert_equals false (Future.is_ready future) "First delay";
  Bool_asserter.assert_equals false (Future.is_ready future) "Second delay";
  Bool_asserter.assert_equals false (Future.is_ready future) "Third delay";
  Bool_asserter.assert_equals
    true
    (Future.is_ready future)
    "After third delay should be ready";
  String_asserter.assert_equals
    "Delayed value"
    (Future.get_exn future)
    "retrieve delayed value";
  true

let test_continue_delayed_future_with () =
  let future = Future.delayed_value ~delays:1 "Delayed value" in
  let future = Future.continue_with future @@ fun result -> result in
  Bool_asserter.assert_equals false (Future.is_ready future) "Delay!";
  Bool_asserter.assert_equals
    true
    (Future.is_ready future)
    "After the delay, the continuation should execute";
  String_asserter.assert_equals
    "Delayed value"
    (Future.get_exn future)
    "The delayed value must match the expected";
  true

let test_exception_in_future_continuation () =
  let make_future (i : int) : int Future.t =
    if i mod 2 = 0 then
      Future.of_value 5
    else
      Future.continue_with (Future.of_value ()) @@ fun _ ->
      failwith "Does it fire?"
  in
  let () =
    match Future.get (make_future 4) with
    | Ok result -> Asserter.Int_asserter.assert_equals 5 result "must be 5"
    | Error error -> failwith (Future.error_to_string error)
  in
  let () =
    match Future.get (make_future 3) with
    | Ok _result -> failwith "Expected a failure"
    | Error error ->
      Asserter.String_asserter.assert_equals
        "Continuation_raised((Failure \"Does it fire?\"))"
        (Future.error_to_string error)
        "expecting the correct error"
  in
  true

let test_of_error () =
  let make_future (i : int) : int Future.t =
    if i mod 2 = 0 then
      Future.of_value 5
    else
      Future.of_error "Does it fail?"
  in
  let () =
    match Future.get (make_future 4) with
    | Ok result -> Asserter.Int_asserter.assert_equals 5 result "must be 5"
    | Error error -> failwith (Future.error_to_string error)
  in
  let () =
    match Future.get (make_future 3) with
    | Ok _result -> failwith "Expected a failure"
    | Error error ->
      Asserter.String_asserter.assert_equals
        "Continuation_raised((Failure \"Does it fail?\"))"
        (Future.error_to_string error)
        "expecting the correct error"
  in
  true

(* This test verifies that all continuations and result handlers are called
    exactly once when calling `Future`'s `is_ready` and `get` functions. *)
let test_bound_future_idempotency () =
  let initial_future_id = ref 0 in
  let initial_future () =
    Printf.printf "initial_future future\n";
    initial_future_id := !initial_future_id + 1;
    Future.of_value "initial_future"
  in

  let next_future_id = ref 0 in
  let next_future previous_result =
    Printf.printf "next future: %s\n" previous_result;
    next_future_id := !next_future_id + 1;
    Future.of_value "next_future"
  in

  let penultimate_future_id = ref 0 in
  let penultimate_future previous_result =
    Printf.printf "penultimate future: %s\n" previous_result;
    penultimate_future_id := !penultimate_future_id + 1;
    Future.of_value "penultimate_future"
  in

  let ultimate_future_id = ref 0 in
  let ultimate_future =
    Future.continue_with_future (initial_future ()) @@ fun result ->
    Printf.printf "Continue with next future; result: %s\n" result;
    let penultimate_future =
      Future.continue_with_future (next_future result) @@ fun result ->
      Printf.printf "Continue with penultimate future; result: %s\n" result;
      penultimate_future result
    in
    Future.continue_with penultimate_future @@ fun result ->
    ultimate_future_id := !ultimate_future_id + 1;
    Printf.printf "continue_with: %s\n" result;
    "ultimate_future"
  in

  let is_future_ready = Future.is_ready ultimate_future in
  Bool_asserter.assert_equals
    true
    is_future_ready
    "The future should be ready immediately";

  let get_and_assert_value future ~expected =
    match Future.get future with
    | Ok actual ->
      String_asserter.assert_equals
        expected
        actual
        "The actual value must match the expected"
    | Error error ->
      failwith (Printf.sprintf "Error! %s\n" (Future.error_to_string error))
  in

  get_and_assert_value ultimate_future ~expected:"ultimate_future";

  (* Now, try getting the value out of the future a second time *)
  get_and_assert_value ultimate_future ~expected:"ultimate_future";

  let rec verify_ids ids =
    match ids with
    | [] -> ()
    | (id, name) :: rest ->
      Int_asserter.assert_equals
        1
        id
        (Printf.sprintf "'%s' should only be called once!" name);

      verify_ids rest
  in

  verify_ids
    [
      (!initial_future_id, "initial_future");
      (!next_future_id, "next_future");
      (!penultimate_future_id, "penultimate_future");
      (!ultimate_future_id, "ultimate_future");
    ];

  true

let tests =
  [
    ("test_delayed_future", test_delayed_future);
    ("test_continue_delayed_future_with", test_continue_delayed_future_with);
    ("test_of_error", test_of_error);
    ( "test_exception_in_future_continuation",
      test_exception_in_future_continuation );
    ("test_bound_future_idempotency", test_bound_future_idempotency);
  ]

let () =
  Daemon.check_entry_point ();
  Unit_test.run_all tests
