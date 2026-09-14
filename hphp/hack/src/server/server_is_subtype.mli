(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

val check :
  MultiWorker.worker list option ->
  string ->
  Server_env.env ->
  (string, string) result
