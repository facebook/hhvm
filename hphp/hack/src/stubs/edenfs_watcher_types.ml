(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

type changes =
  | Lost
  | FileChanges of string list  (** List is not guaranteed to be deduplicated *)
  | CommitTransition of {
      from_commit: string;
      to_commit: string;
      file_changes: string list;
          (** List is not guaranteed to be deduplicated *)
    }
[@@deriving show]

type edenfs_watcher_error = EdenfsWatcherError of string [@@deriving show]
