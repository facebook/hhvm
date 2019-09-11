(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Config_file
open Config_file.Getters

let test_parse_version_valid_components () =
  let version = parse_version (Some "^3.37.1") in
  match version with
  | Version_components _version -> true
  | _ -> false

let test_parse_version_some_string () =
  let version = parse_version (Some "something") in
  match version with
  | Opaque_version (Some version) ->
    Asserter.String_asserter.assert_equals
      "something"
      version
      "Opaque version should match";
    true
  | _ -> false

let test_parse_version_none () =
  let version = parse_version None in
  match version with
  | Opaque_version None -> true
  | _ -> false

let test_compare_versions () =
  let v1 = Version_components { major = 3; minor = 37; build = 1 } in
  let v2 = Version_components { major = 3; minor = 37; build = 1 } in
  let result = compare_versions v1 v2 in
  Asserter.Int_asserter.assert_equals 0 result "Equal versions";

  let v2 = Version_components { major = 2; minor = 37; build = 1 } in
  let result = compare_versions v1 v2 in
  Asserter.Int_asserter.assert_equals 1 result "Major version: v2 less than v1";

  let v2 = Version_components { major = 4; minor = 37; build = 1 } in
  let result = compare_versions v1 v2 in
  Asserter.Int_asserter.assert_equals
    (-1)
    result
    "Major version: v2 greater than v1";

  let v2 = Version_components { major = 3; minor = 3; build = 1 } in
  let result = compare_versions v1 v2 in
  Asserter.Int_asserter.assert_equals 1 result "Minor version: v2 less than v1";

  let v2 = Version_components { major = 3; minor = 300; build = 1 } in
  let result = compare_versions v1 v2 in
  Asserter.Int_asserter.assert_equals
    (-1)
    result
    "Minor version: v2 greater than v1";

  let v2 = Version_components { major = 3; minor = 37; build = 0 } in
  let result = compare_versions v1 v2 in
  Asserter.Int_asserter.assert_equals 1 result "Build version: v2 less than v1";

  let v2 = Version_components { major = 3; minor = 37; build = 2222 } in
  let result = compare_versions v1 v2 in
  Asserter.Int_asserter.assert_equals
    (-1)
    result
    "Build version: v2 greater than v1";

  true

let test_bool_if_min_version () =
  let current_version =
    Version_components { major = 3; minor = 37; build = 1 }
  in
  let config = SMap.empty in
  let config = SMap.add "feature_true" "true" config in
  let config = SMap.add "feature_false" "false" config in
  let config = SMap.add "feature_version_eq" "^3.37.1" config in
  let config = SMap.add "feature_version_less" "^3.36.1" config in
  let config = SMap.add "feature_version_greater" "^3.40.1" config in
  let actual =
    bool_if_min_version "feature_true" ~default:false ~current_version config
  in
  Asserter.Bool_asserter.assert_equals true actual "Plain 'true' boolean value";

  let actual =
    bool_if_min_version "feature_false" ~default:true ~current_version config
  in
  Asserter.Bool_asserter.assert_equals
    false
    actual
    "Plain 'false' boolean value";

  let actual =
    bool_if_min_version
      "feature_version_eq"
      ~default:false
      ~current_version
      config
  in
  Asserter.Bool_asserter.assert_equals
    true
    actual
    "Semantic version value equal to current version";

  let actual =
    bool_if_min_version
      "feature_version_less"
      ~default:false
      ~current_version
      config
  in
  Asserter.Bool_asserter.assert_equals
    true
    actual
    "Semantic version value less than current version";

  let actual =
    bool_if_min_version
      "feature_version_greater"
      ~default:true
      ~current_version
      config
  in
  Asserter.Bool_asserter.assert_equals
    false
    actual
    "Semantic version value greater than current version";

  true

let tests =
  [
    ("test_parse_version_none", test_parse_version_none);
    ("test_parse_version_some_string", test_parse_version_some_string);
    ("test_parse_version_valid_components", test_parse_version_valid_components);
    ("test_compare_versions", test_compare_versions);
    ("test_bool_if_min_version", test_bool_if_min_version);
  ]

let () = Unit_test.run_all tests
