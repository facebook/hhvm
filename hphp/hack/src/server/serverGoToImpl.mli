(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open ServerCommandTypes.Find_refs

val go :
  action:action ->
  genv:ServerEnv.genv ->
  env:ServerEnv.env ->
  ServerEnv.env * server_result_or_retry
