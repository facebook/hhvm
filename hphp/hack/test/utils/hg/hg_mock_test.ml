let base_rev = 789

let hg_rev = Hg.Rev.of_string "abcdefg123"

let fake_repo_path = "some/fake/dir"

let test_mock_basic () =
  Hg.Mocking.current_working_copy_base_rev_returns (Future.of_value base_rev);
  let result =
    Hg.current_working_copy_base_rev fake_repo_path
    |> Future.get_exn ~timeout:30
  in
  Asserter.Int_asserter.assert_equals
    base_rev
    result
    "current_working_copy_base_rev_returns";
  Hg.Mocking.closest_global_ancestor_bind_value
    hg_rev
    (Future.of_value base_rev);
  let result =
    Hg.get_closest_global_ancestor hg_rev fake_repo_path
    |> Future.get_exn ~timeout:30
  in
  Asserter.Int_asserter.assert_equals
    base_rev
    result
    "Hg.Mocking.closest_global_ancestor_bind_value";
  true

let tests = [("test_mock_basic", test_mock_basic)]

let () = Unit_test.run_all tests
