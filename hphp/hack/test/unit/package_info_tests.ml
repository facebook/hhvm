(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)
open Hh_prelude

let package ~name ~include_paths =
  {
    Package.name = (Pos.none, name);
    includes = [];
    soft_includes = [];
    include_paths = List.map include_paths ~f:(fun p -> (Pos.none, p));
    enable_strict_isolation = false;
    allow_deployed_packages_checking = false;
    is_implicit = false;
  }

let package_name_for info ~path =
  Package_info.get_package_for_file info ~path
  |> Option.map ~f:Package.get_package_name

(* A directory-level package, and then a file inside it claimed individually —
   which is what synthesizing a candidate package out of a cluster looks like. *)
let owned_dir = package ~name:"Owned" ~include_paths:["flib/owned/"]

let candidate =
  package ~name:"Candidate" ~include_paths:["flib/owned/Candidate.php"]

let test_added_package_is_found () =
  let info = Package_info.add_package Package_info.empty candidate in
  Option.is_some (Package_info.get_package info "Candidate")

(* Why the entries are prepended: a directory package already claims everything
   under it, so an appended candidate would never be reached for its own
   files. *)
let test_added_package_wins_for_its_own_files () =
  let info =
    Package_info.add_package
      (Package_info.add_package Package_info.empty owned_dir)
      candidate
  in
  Option.equal
    String.equal
    (Some "Candidate")
    (package_name_for info ~path:"flib/owned/Candidate.php")

(* Adding must not disturb what the packages already there resolve. A file that
   the candidate does not name keeps the package that owned it. *)
let test_other_files_are_untouched () =
  let before = Package_info.add_package Package_info.empty owned_dir in
  let after = Package_info.add_package before candidate in
  Option.equal
    String.equal
    (package_name_for before ~path:"flib/owned/Other.php")
    (package_name_for after ~path:"flib/owned/Other.php")

let test_existing_packages_are_kept () =
  let info =
    Package_info.add_package
      (Package_info.add_package Package_info.empty owned_dir)
      candidate
  in
  Package_info.package_exists info "Owned"

(* Replacing a configured package would silently drop it from resolution for as
   long as the caller holds the result, so a name already in use is refused
   rather than shadowed. A caller adding a package needs one the configuration
   will not use. *)
let test_a_name_already_configured_is_refused () =
  let info = Package_info.add_package Package_info.empty owned_dir in
  let same_name_again =
    package ~name:"Owned" ~include_paths:["flib/elsewhere/"]
  in
  match Package_info.add_package info same_name_again with
  | exception Failure _ ->
    (* And the one that was there is untouched. *)
    Option.equal
      String.equal
      (Some "Owned")
      (package_name_for info ~path:"flib/owned/Other.php")
  | _ -> false

let () =
  Unit_test.run_all
    [
      ("added package is found by name", test_added_package_is_found);
      ( "added package wins for its own files",
        test_added_package_wins_for_its_own_files );
      ("other files resolve as before", test_other_files_are_untouched);
      ("existing packages are kept", test_existing_packages_are_kept);
      ( "a name already configured is refused",
        test_a_name_already_configured_is_refused );
    ]
