(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Hh_prelude

let suffix_of s =
  Option.map (Relative_path.from_root_opt ~suffix:s) ~f:Relative_path.suffix

let test_from_root_opt_keeps_a_relative_path () =
  Option.equal String.equal (suffix_of "a/b") (Some "a/b")

let test_from_root_opt_drops_slashes_at_the_end () =
  Option.equal String.equal (suffix_of "a/b//") (Some "a/b")

let test_from_root_opt_refuses_what_is_not_a_relative_path () =
  List.for_all [""; "/"; "/a"; "/a/b"; "../a"; "a/../b"; "a/.."] ~f:(fun s ->
      Option.is_none (suffix_of s))

let test_from_root_opt_allows_dots_inside_a_name () =
  Option.equal String.equal (suffix_of "a/..b/c.d") (Some "a/..b/c.d")

let within ~dir path =
  Relative_path.is_within
    ~dir:(Relative_path.from_root ~suffix:dir)
    (Relative_path.from_root ~suffix:path)

let test_is_within_holds_for_the_directory_and_what_is_under_it () =
  within ~dir:"a" "a" && within ~dir:"a" "a/b" && within ~dir:"a" "a/b/c"

let test_is_within_does_not_hold_for_a_longer_name () =
  not (within ~dir:"a" "ab")

let test_is_within_does_not_hold_for_a_parent_or_sibling () =
  (not (within ~dir:"a/b" "a")) && not (within ~dir:"a/b" "a/c")

let test_is_within_does_not_hold_across_prefixes () =
  not
    (Relative_path.is_within
       ~dir:(Relative_path.from_root ~suffix:"a")
       (Relative_path.create Relative_path.Dummy "a/b"))

let tests =
  [
    ( "test_from_root_opt_keeps_a_relative_path",
      test_from_root_opt_keeps_a_relative_path );
    ( "test_from_root_opt_drops_slashes_at_the_end",
      test_from_root_opt_drops_slashes_at_the_end );
    ( "test_from_root_opt_refuses_what_is_not_a_relative_path",
      test_from_root_opt_refuses_what_is_not_a_relative_path );
    ( "test_from_root_opt_allows_dots_inside_a_name",
      test_from_root_opt_allows_dots_inside_a_name );
    ( "test_is_within_holds_for_the_directory_and_what_is_under_it",
      test_is_within_holds_for_the_directory_and_what_is_under_it );
    ( "test_is_within_does_not_hold_for_a_longer_name",
      test_is_within_does_not_hold_for_a_longer_name );
    ( "test_is_within_does_not_hold_for_a_parent_or_sibling",
      test_is_within_does_not_hold_for_a_parent_or_sibling );
    ( "test_is_within_does_not_hold_across_prefixes",
      test_is_within_does_not_hold_across_prefixes );
  ]

let () = Unit_test.run_all tests
