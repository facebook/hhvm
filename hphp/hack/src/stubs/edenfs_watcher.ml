(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Hh_prelude

type clock = string [@@deriving show, eq]

type init_settings = {
  root: Path.t;
  watch_spec: FilesToIgnore.watch_spec;
  debug_logging: bool;
}

type instance_handle_ffi

type instance = { handle: instance_handle_ffi }

type edenfs_watcher_error = Edenfs_watcher_types.edenfs_watcher_error
[@@deriving show]

type changes = Edenfs_watcher_types.changes [@@deriving show]

let init (_settings : init_settings) = failwith "not implemented"

let get_changes_sync (_instance : instance) :
    (changes list * clock, edenfs_watcher_error) result =
  failwith "not implemented"

let get_changes_async (_instance : instance) :
    (changes list * clock, edenfs_watcher_error) result =
  failwith "not implemented"

let get_notification_fd (_instance : instance) :
    (Caml_unix.file_descr, edenfs_watcher_error) result =
  failwith "not implemented"

let get_all_files (_instance : instance) :
    (string list * clock, edenfs_watcher_error) result =
  failwith "not implemented"
