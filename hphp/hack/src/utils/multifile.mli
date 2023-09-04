(*
 * Copyright (c) 2015-present, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

(** This allows one to fake having multiple files in one file. This
  is used only in unit test files.
  Indeed, there are some features that require mutliple files to be tested.
  For example, newtype has a different meaning depending on the file. *)

type path = string

type content = string

type repo = content Relative_path.Map.t

val file_to_file_list : Relative_path.t -> (Relative_path.t * content) list

(** Takes the path of a "multifile".
  For example if a multifile a.php has the following lines
  ```
  //// x.php
  <?hh
  type F = int

  //// y.php
  <?hh
  type G = float

  ```

  [file_to_files] will return a map
    [
      "a.php--x.php" -> "<?hh
  type F = int
  ";
      "a.php--y.php" -> "<?hh
  type G = float
  ";
    ]
  *)
val file_to_files : Relative_path.t -> repo

(** Multifile pathnames are like (DummyRoot, "/home/ljw/a.php--chess.php"),
or just (DummyRoot, "/home/ljw/chess.php") if there weren't any multifiles within the file.
This function strips both to "chess.php", a more appropriate rendering for test output. *)
val short_suffix : Relative_path.t -> string

(** Given a path of the form `sample/file/name.php--another/file/name.php`,
  read in the portion of multifile `sample/file/name.php` corresponding
  to `another/file/name.php`. *)
val read_file_from_multifile : path -> content list

val print_files_as_multifile : repo -> unit

module States : sig
  type change =
    | Modified of content
    | Deleted

  type repo_change = change Relative_path.Map.t

  type t = {
    base: repo;
    changes: repo_change list;
  }
  [@@deriving show]

  val apply_repo_change : repo -> repo_change -> repo

  val parse : path -> t
end
