(**
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Asserter
open Cli_args

let spec_input =
  {|
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

let expected_spec_json =
  {|{
  "gen_with_errors":true,
  "files_to_check":[
    "/some/path/prefix1",
    {"from_prefix_incl":"/from/path/prefix1","to_prefix_excl":"/to/path/prefix1"},
    {"from_prefix_incl":"/from/path/prefix2","to_prefix_excl":"/to/path/prefix2"},
    {"from_prefix_incl":"/from/path/only"},
    {"to_prefix_excl":"/to/path/only"}
  ],
  "filename":"/some/dir/some_filename"
}|}

let compare_paths (expected : Relative_path.t) (actual : Relative_path.t) : unit
    =
  String_asserter.assert_equals
    (Relative_path.suffix expected)
    (Relative_path.suffix actual)
    "Paths must be equal"

let compare_optional_paths
    (expected : Relative_path.t option) (actual : Relative_path.t option) : unit
    =
  match (expected, actual) with
  | (None, None) -> ()
  | (Some expected, Some actual) -> compare_paths expected actual
  | (_, _) -> assert false

let compare_files_to_check_spec
    (expected : files_to_check_spec) (actual : files_to_check_spec) : unit =
  match (expected, actual) with
  | (Range expected, Range actual) ->
    compare_optional_paths expected.from_prefix_incl actual.from_prefix_incl;

    compare_optional_paths expected.to_prefix_excl actual.to_prefix_excl
  | (Prefix expected, Prefix actual) -> compare_paths expected actual
  | (_, _) -> assert false

let verify_parsed_spec parsed_spec =
  Int_asserter.assert_equals
    7
    (List.length parsed_spec.files_to_check)
    "There should be 7 file specs";

  let (from_path : Relative_path.t option) =
    Some (Relative_path.from_root "/from/path/prefix1")
  in
  let (to_path : Relative_path.t option) =
    Some (Relative_path.from_root "/to/path/prefix1")
  in
  let (range1 : files_to_check_range) =
    { from_prefix_incl = from_path; to_prefix_excl = to_path }
  in
  let (from_path : Relative_path.t option) =
    Some (Relative_path.from_root "/from/path/prefix2")
  in
  let (to_path : Relative_path.t option) =
    Some (Relative_path.from_root "/to/path/prefix2")
  in
  let (range2 : files_to_check_range) =
    { from_prefix_incl = from_path; to_prefix_excl = to_path }
  in
  let (range3 : files_to_check_range) =
    { from_prefix_incl = None; to_prefix_excl = None }
  in
  let (range4 : files_to_check_range) =
    {
      from_prefix_incl = Some (Relative_path.from_root "/from/path/only");
      to_prefix_excl = None;
    }
  in
  let (range5 : files_to_check_range) =
    {
      from_prefix_incl = None;
      to_prefix_excl = Some (Relative_path.from_root "/to/path/only");
    }
  in
  let expected =
    [
      Prefix (Relative_path.from_root "/some/path/prefix1");
      Range range1;
      Range range2;
      Prefix (Relative_path.from_root "/some/path/prefix2");
      Range range3;
      Range range4;
      Range range5;
    ]
  in
  List.iter2 compare_files_to_check_spec expected parsed_spec.files_to_check

let test_save_state_spec () : bool =
  let spec = Some spec_input in
  let parsed_spec = get_save_state_spec spec in
  match parsed_spec with
  | Ok (Some parsed_spec) ->
    verify_parsed_spec parsed_spec;
    true
  | Ok None
  | Error _ ->
    false

let test_save_state_spec_json () : bool =
  let spec_json =
    get_save_state_spec_json
      {
        files_to_check =
          [
            Prefix (Relative_path.from_root "/some/path/prefix1");
            Range
              {
                from_prefix_incl =
                  Some (Relative_path.from_root "/from/path/prefix1");
                to_prefix_excl =
                  Some (Relative_path.from_root "/to/path/prefix1");
              };
            Range
              {
                from_prefix_incl =
                  Some (Relative_path.from_root "/from/path/prefix2");
                to_prefix_excl =
                  Some (Relative_path.from_root "/to/path/prefix2");
              };
            Range
              {
                from_prefix_incl =
                  Some (Relative_path.from_root "/from/path/only");
                to_prefix_excl = None;
              };
            Range
              {
                from_prefix_incl = None;
                to_prefix_excl = Some (Relative_path.from_root "/to/path/only");
              };
          ];
        filename = "/some/dir/some_filename";
        gen_with_errors = true;
      }
  in
  String_asserter.assert_equals
    expected_spec_json
    spec_json
    "The output should match the expected";
  true

let tests =
  [
    ("Test save state spec", test_save_state_spec);
    ("Test save state spec JSON", test_save_state_spec_json);
  ]

let () = Unit_test.run_all tests
