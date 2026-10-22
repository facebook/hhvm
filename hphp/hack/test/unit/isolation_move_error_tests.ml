(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Hh_prelude
module Error = Client_isolation_move.Move_error

let source = Relative_path.from_root ~suffix:"projects/a"

let target = Relative_path.from_root ~suffix:"packages/deep/a"

let with_root f =
  let dir = Stdlib.Filename.temp_file "hh_isolation_error" "" in
  Stdlib.Sys.remove dir;
  Disk.mkdir_p dir;
  Relative_path.set_path_prefix Relative_path.Root (Path.make dir);
  Exn.protect ~f ~finally:(fun () -> Disk.rm_dir_tree dir)

let diagnostic ?(severity = User_diagnostic.Err) ~code ~file ~line message =
  let pos =
    Pos.make_from_lnum_bol_offset
      ~pos_file:(Relative_path.from_root ~suffix:file)
      ~pos_start:(line, 0, 0)
      ~pos_end:(line, 0, 1)
    |> Pos.to_absolute
  in
  User_diagnostic.make_absolute severity code [(pos, message)]

let status error_list =
  {
    Server_command_types.Server_status.liveness =
      Server_command_types.Live_status;
    error_list;
    dropped_count = 0;
    last_recheck_stats = None;
    file_watcher_clock = None;
  }

let error ~code ~file ~line message =
  { Error.code; file = Relative_path.from_root ~suffix:file; line; message }

let introduced ~baseline ~after =
  Error.introduced ~source ~target ~baseline ~after

let test_the_errors_of_a_status_leave_out_the_warnings () =
  with_root @@ fun () ->
  let warning = User_diagnostic.Warning { is_trusted = true } in
  let errors =
    Error.of_status
      (status
         [
           diagnostic ~code:4110 ~file:"lib/o.php" ~line:3 "bad";
           diagnostic
             ~severity:warning
             ~code:12005
             ~file:"lib/p.php"
             ~line:9
             "w";
         ])
  in
  List.equal
    (fun a b ->
      Int.equal a.Error.code b.Error.code
      && Relative_path.equal a.Error.file b.Error.file
      && Int.equal a.Error.line b.Error.line
      && String.equal a.Error.message b.Error.message)
    errors
    [error ~code:4110 ~file:"lib/o.php" ~line:3 "bad"]

let test_an_error_the_files_already_had_is_not_introduced () =
  let before = error ~code:4110 ~file:"projects/a/x.php" ~line:3 "bad" in
  let after = error ~code:4110 ~file:"packages/deep/a/x.php" ~line:3 "bad" in
  List.is_empty (introduced ~baseline:[before] ~after:[after])

let test_an_error_that_names_the_directory_is_not_introduced () =
  let named path = Printf.sprintf "bad %s/x.php" path in
  let before =
    error ~code:4110 ~file:"projects/a/x.php" ~line:3 (named "projects/a")
  in
  let after =
    error
      ~code:4110
      ~file:"packages/deep/a/x.php"
      ~line:3
      (named "packages/deep/a")
  in
  List.is_empty (introduced ~baseline:[before] ~after:[after])

let test_an_introduced_error_is_named_as_it_was_before_the_move () =
  let after =
    error ~code:2049 ~file:"packages/deep/a/x.php" ~line:5 "unbound"
  in
  match introduced ~baseline:[] ~after:[after] with
  | [found] ->
    String.equal (Relative_path.suffix found.Error.file) "projects/a/x.php"
    && Int.equal found.Error.code 2049
    && Int.equal found.Error.line 5
  | _ -> false

let test_an_error_outside_the_directory_keeps_its_file () =
  let after = error ~code:4472 ~file:"lib/o.php" ~line:30 "not available" in
  match introduced ~baseline:[] ~after:[after] with
  | [found] -> String.equal (Relative_path.suffix found.Error.file) "lib/o.php"
  | _ -> false

let test_a_sibling_whose_name_starts_with_the_target_is_not_rewritten () =
  let message =
    "see packages/deep/a_bar/x.php, my_packages/deep/a/z.php and packages/deep/a/y.php"
  in
  let before =
    error
      ~code:4110
      ~file:"lib/o.php"
      ~line:3
      "see packages/deep/a_bar/x.php, my_packages/deep/a/z.php and projects/a/y.php"
  in
  let after = error ~code:4110 ~file:"lib/o.php" ~line:3 message in
  List.is_empty (introduced ~baseline:[before] ~after:[after])

let test_the_same_errors_are_the_same_in_any_order () =
  let a = error ~code:1 ~file:"a.php" ~line:1 "a" in
  let b = error ~code:2 ~file:"b.php" ~line:2 "b" in
  Error.same [a; b] [b; a]
  && (not (Error.same [a; b] [a]))
  && (not (Error.same [a] [a; b]))
  && not (Error.same [a] [error ~code:1 ~file:"a.php" ~line:2 "a"])

let tests =
  [
    ( "test_the_errors_of_a_status_leave_out_the_warnings",
      test_the_errors_of_a_status_leave_out_the_warnings );
    ( "test_an_error_the_files_already_had_is_not_introduced",
      test_an_error_the_files_already_had_is_not_introduced );
    ( "test_an_error_that_names_the_directory_is_not_introduced",
      test_an_error_that_names_the_directory_is_not_introduced );
    ( "test_an_introduced_error_is_named_as_it_was_before_the_move",
      test_an_introduced_error_is_named_as_it_was_before_the_move );
    ( "test_an_error_outside_the_directory_keeps_its_file",
      test_an_error_outside_the_directory_keeps_its_file );
    ( "test_a_sibling_whose_name_starts_with_the_target_is_not_rewritten",
      test_a_sibling_whose_name_starts_with_the_target_is_not_rewritten );
    ( "test_the_same_errors_are_the_same_in_any_order",
      test_the_same_errors_are_the_same_in_any_order );
  ]

let () = Unit_test.run_all tests
