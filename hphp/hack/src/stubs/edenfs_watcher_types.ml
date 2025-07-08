(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

type settings = {
  root: Path.t;
  watch_spec: FilesToIgnore.watch_spec;
  debug_logging: bool;
}

type changes =
  | FileChanges of string list  (** List is not guaranteed to be deduplicated *)
  | CommitTransition of {
      from_commit: string;
      to_commit: string;
      file_changes: string list;
          (** List is not guaranteed to be deduplicated *)
    }
[@@deriving show]

type edenfs_watcher_error =
  | EdenfsWatcherError of string
  | LostChanges of string
      (** There may have been some changes that Eden has lost track of.
        (e.g., due to Eden restarting).
        The string is just a description of what happened. *)
  | NonEdenWWW  (** The root directory is not inside an EdenFS mount. *)
[@@deriving show]
