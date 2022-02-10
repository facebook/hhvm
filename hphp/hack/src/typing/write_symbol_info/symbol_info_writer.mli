(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

val go :
  MultiWorker.worker list option ->
  Provider_context.t ->
  string ->
  string ->
  string ->
  Relative_path.t list ->
  unit
