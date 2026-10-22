(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Hh_prelude
module Dir_move = Client_isolation_move.Dir_move

let files = ["x.php"; "sub/y.php"; "data.json"; ".hidden"]

let directories = ["sub"; "sub/empty"]

(** A directory with a tree under [source] in it, and the absolute paths of the
    source and of a target two levels down that does not exist. *)
let with_tree f =
  let root = Stdlib.Filename.temp_file "hh_isolation_dir_move" "" in
  Stdlib.Sys.remove root;
  let source = Filename.concat root "projects/a" in
  Disk.mkdir_p (Filename.concat source "sub/empty");
  List.iter files ~f:(fun file ->
      Disk.write_file
        ~file:(Filename.concat source file)
        ~contents:("contents of " ^ file));
  let target = Filename.concat root "packages/deep/a" in
  Exn.protect
    ~f:(fun () -> f ~root ~source ~target)
    ~finally:(fun () -> Disk.rm_dir_tree root)

let exists = Disk.file_exists

let tree_is_under dir =
  List.for_all (directories @ files) ~f:(fun path ->
      exists (Filename.concat dir path))
  && List.for_all files ~f:(fun file ->
         String.equal
           (Disk.cat (Filename.concat dir file))
           ("contents of " ^ file))

let test_everything_moves_and_the_missing_parents_are_made () =
  with_tree @@ fun ~root:_ ~source ~target ->
  match Dir_move.move ~source ~target with
  | Ok _ -> tree_is_under target && not (exists source)
  | Error _ -> false

let test_undo_puts_everything_back_and_removes_what_was_made () =
  with_tree @@ fun ~root ~source ~target ->
  match Dir_move.move ~source ~target with
  | Error _ -> false
  | Ok moved ->
    Dir_move.undo moved;
    tree_is_under source
    && (not (exists target))
    && not (exists (Filename.concat root "packages"))

let test_undo_keeps_a_directory_that_gained_something () =
  with_tree @@ fun ~root:_ ~source ~target ->
  match Dir_move.move ~source ~target with
  | Error _ -> false
  | Ok moved ->
    let added = Filename.concat (Filename.dirname target) "added.txt" in
    Disk.write_file ~file:added ~contents:"x";
    Dir_move.undo moved;
    tree_is_under source && (not (exists target)) && exists added

let test_undo_twice_changes_nothing () =
  with_tree @@ fun ~root ~source ~target ->
  match Dir_move.move ~source ~target with
  | Error _ -> false
  | Ok moved ->
    Dir_move.undo moved;
    Dir_move.undo moved;
    tree_is_under source
    && (not (exists target))
    && not (exists (Filename.concat root "packages"))

let test_undo_does_not_overwrite_what_is_at_the_old_place () =
  with_tree @@ fun ~root:_ ~source ~target ->
  match Dir_move.move ~source ~target with
  | Error _ -> false
  | Ok moved ->
    let other = Filename.concat source "x.php" in
    Disk.mkdir_p source;
    Disk.write_file ~file:other ~contents:"other";
    (match Dir_move.undo moved with
    | () -> false
    | exception Failure message ->
      (* The directory stays where it was moved to, and what is at the old
         place is as it was. *)
      String.is_substring message ~substring:source
      && String.equal (Disk.cat other) "other"
      && tree_is_under target)

let test_a_move_that_fails_after_making_the_parents_is_put_back () =
  with_tree @@ fun ~root ~source ~target ->
  (* A directory cannot be renamed into another one without write permission on
     it, so the rename fails after the parents of the target were made. *)
  let locked = source in
  Unix.chmod locked 0o555;
  Exn.protect
    ~finally:(fun () -> Unix.chmod locked 0o755)
    ~f:(fun () ->
      match Dir_move.move ~source ~target with
      | Ok _ -> false
      | Error _ ->
        tree_is_under source
        && (not (exists target))
        && not (exists (Filename.concat root "packages")))

let tests =
  [
    ( "test_everything_moves_and_the_missing_parents_are_made",
      test_everything_moves_and_the_missing_parents_are_made );
    ( "test_undo_puts_everything_back_and_removes_what_was_made",
      test_undo_puts_everything_back_and_removes_what_was_made );
    ( "test_undo_keeps_a_directory_that_gained_something",
      test_undo_keeps_a_directory_that_gained_something );
    ("test_undo_twice_changes_nothing", test_undo_twice_changes_nothing);
    ( "test_undo_does_not_overwrite_what_is_at_the_old_place",
      test_undo_does_not_overwrite_what_is_at_the_old_place );
    ( "test_a_move_that_fails_after_making_the_parents_is_put_back",
      test_a_move_that_fails_after_making_the_parents_is_put_back );
  ]

let () = Unit_test.run_all tests
