(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

let assert_root ~start ~expected =
  match
    Wwwroot.interpret_command_line_root_parameter [Path.to_string start]
  with
  | Error error ->
    Printf.eprintf "Failed to find www root: %s\n" error;
    false
  | Ok actual ->
    Asserter.String_asserter.assert_equals
      (Path.to_string expected)
      (Path.to_string actual)
      "Resolved www root should match";
    true

let assert_no_root ~start =
  match
    Wwwroot.interpret_command_line_root_parameter [Path.to_string start]
  with
  | Error _ -> true
  | Ok actual ->
    Printf.eprintf
      "Unexpectedly resolved www root: %s\n"
      (Path.to_string actual);
    false

let create_fbsource_layout temp_dir =
  let fbsource = Path.concat temp_dir "fbsource" in
  let www = Path.concat fbsource "www" in
  Real_disk.mkdir_p (Path.to_string www);
  Real_disk.write_file
    ~file:(Path.concat www ".hhconfig" |> Path.to_string)
    ~contents:"";
  (fbsource, www)

let test_finds_www_child_from_fbsource_root () =
  Tempfile.with_real_tempdir (fun temp_dir ->
      let (fbsource, www) = create_fbsource_layout temp_dir in
      assert_root ~start:fbsource ~expected:www)

let test_does_not_find_www_child_from_fbsource_subdirectory () =
  Tempfile.with_real_tempdir (fun temp_dir ->
      let (fbsource, _www) = create_fbsource_layout temp_dir in
      let fbcode = Path.concat fbsource "fbcode" in
      Real_disk.mkdir_p (Path.to_string fbcode);
      assert_no_root ~start:fbcode)

let test_does_not_find_www_child_without_hhconfig () =
  Tempfile.with_real_tempdir (fun temp_dir ->
      let fbsource = Path.concat temp_dir "fbsource" in
      Real_disk.mkdir_p (Path.concat fbsource "www" |> Path.to_string);
      assert_no_root ~start:fbsource)

let test_prefers_ancestor_over_www_child () =
  Tempfile.with_real_tempdir (fun temp_dir ->
      let outer_www = Path.concat temp_dir "www" in
      let current_dir = Path.concat outer_www "foo/bar" in
      let inner_www = Path.concat current_dir "www" in
      Real_disk.mkdir_p (Path.to_string inner_www);
      Real_disk.write_file
        ~file:(Path.concat outer_www ".hhconfig" |> Path.to_string)
        ~contents:"";
      Real_disk.write_file
        ~file:(Path.concat inner_www ".hhconfig" |> Path.to_string)
        ~contents:"";
      assert_root ~start:current_dir ~expected:outer_www)

let tests =
  [
    ( "finds_www_child_from_fbsource_root",
      test_finds_www_child_from_fbsource_root );
    ( "does_not_find_www_child_from_fbsource_subdirectory",
      test_does_not_find_www_child_from_fbsource_subdirectory );
    ( "does_not_find_www_child_without_hhconfig",
      test_does_not_find_www_child_without_hhconfig );
    ("prefers_ancestor_over_www_child", test_prefers_ancestor_over_www_child);
  ]

let () = Unit_test.run_all tests
