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
  ServerEnv.env ->
  (string, string) result
