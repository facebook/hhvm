(**
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *
*)

open Asserter

let spec_input = {|
{
  "files_to_check": [
    "/some/path/prefix1",
    {
      "from_prefix_incl": "/from/path/prefix1",
      "to_prefix_excl": "/to/path/prefix1"
    },
    {
      "from_prefix_incl": "/from/path/prefix2",
      "to_prefix_excl": "/to/path/prefix2"
    },
    "/some/path/prefix2",
    {},
    {
      "from_prefix_incl": "/from/path/only"
    },
    {
      "to_prefix_excl": "/to/path/only"
    }
  ],
  "filename": "/some/dir/some_filename",
  "gen_with_errors": true
}
|}

let compare_paths (expected: Relative_path.t) (actual: Relative_path.t): unit =
  String_asserter.assert_equals
    (Relative_path.suffix expected)
    (Relative_path.suffix actual)
    "Paths must be equal"

let compare_optional_paths
    (expected: Relative_path.t option)
    (actual: Relative_path.t option): unit =
  match expected, actual with
  | None, None -> ()
  | Some expected, Some actual ->
    compare_paths expected actual
  | _, _ -> assert false

let compare_files_to_check_spec
    (expected: ServerArgs.files_to_check_spec)
    (actual: ServerArgs.files_to_check_spec): unit =
  match expected, actual with
  | ServerArgs.Range expected, ServerArgs.Range actual ->
    compare_optional_paths
      expected.ServerArgs.from_prefix_incl
      actual.ServerArgs.from_prefix_incl;

    compare_optional_paths
      expected.ServerArgs.to_prefix_excl
      actual.ServerArgs.to_prefix_excl
  | ServerArgs.Prefix expected, ServerArgs.Prefix actual ->
    compare_paths expected actual
  | _, _ -> assert false

let verify_parsed_spec parsed_spec =
  Int_asserter.assert_equals
    7
    (List.length parsed_spec.ServerArgs.files_to_check)
    "There should be 7 file specs";

  let (from_path: Relative_path.t option) = Some (Relative_path.from_root "/from/path/prefix1") in
  let (to_path: Relative_path.t option) = Some (Relative_path.from_root "/to/path/prefix1") in
  let (range1: ServerArgs.files_to_check_range) = {
    ServerArgs.from_prefix_incl = from_path;
    ServerArgs.to_prefix_excl = to_path;
  } in

  let (from_path: Relative_path.t option) = Some (Relative_path.from_root "/from/path/prefix2") in
  let (to_path: Relative_path.t option) = Some (Relative_path.from_root "/to/path/prefix2") in
  let (range2: ServerArgs.files_to_check_range) = {
    ServerArgs.from_prefix_incl = from_path;
    ServerArgs.to_prefix_excl = to_path;
  } in

  let (range3: ServerArgs.files_to_check_range) = {
    ServerArgs.from_prefix_incl = None;
    ServerArgs.to_prefix_excl = None;
  } in

  let (range4: ServerArgs.files_to_check_range) = {
    ServerArgs.from_prefix_incl = Some (Relative_path.from_root "/from/path/only");
    ServerArgs.to_prefix_excl = None;
  } in

  let (range5: ServerArgs.files_to_check_range) = {
    ServerArgs.from_prefix_incl = None;
    ServerArgs.to_prefix_excl = Some (Relative_path.from_root "/to/path/only");
  } in

  let expected = [
    ServerArgs.Prefix (Relative_path.from_root "/some/path/prefix1");
    ServerArgs.Range range1;
    ServerArgs.Range range2;
    ServerArgs.Prefix (Relative_path.from_root "/some/path/prefix2");
    ServerArgs.Range range3;
    ServerArgs.Range range4;
    ServerArgs.Range range5;
  ] in

  List.iter2
    compare_files_to_check_spec
    expected
    parsed_spec.ServerArgs.files_to_check

let test_save_with_spec () : bool =
  let spec = Some spec_input in
  let parsed_spec = ServerArgs.verify_save_with_spec spec in

  match parsed_spec with
  | Some parsed_spec ->
    verify_parsed_spec parsed_spec;
    true
  | None -> false

let tests = [
  ("Test --save-with-spec", test_save_with_spec);
]

let () =
  Unit_test.run_all tests
