(*
 * Copyright (c) 2018, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

type saved_legacy_decls [@@deriving show]

val collect_legacy_decls : Provider_context.t -> SSet.t -> saved_legacy_decls

val restore_legacy_decls : saved_legacy_decls -> int

type saved_shallow_decls [@@deriving show]

val collect_shallow_decls :
  Provider_context.t ->
  MultiWorker.worker list option ->
  SSet.t ->
  saved_shallow_decls

val restore_shallow_decls : saved_shallow_decls -> int
