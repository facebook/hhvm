(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

val go :
  ctx:Provider_context.t ->
  genv:ServerEnv.genv ->
  env:ServerEnv.env ->
  max_distance:int ->
  string list ->
  ServerCommandTypes.Find_my_tests.result
