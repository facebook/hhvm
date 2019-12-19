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

let tests =
  [
    ("test_delayed_future", test_delayed_future);
    ("test_continue_delayed_future_with", test_continue_delayed_future_with);
    ("test_of_error", test_of_error);
    ( "test_exception_in_future_continuation",
      test_exception_in_future_continuation );
  ]

let () =
  Daemon.check_entry_point ();
  Unit_test.run_all tests
