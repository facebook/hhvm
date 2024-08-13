(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

val handle :
  'res.
  ServerEnv.genv ->
  ServerEnv.env ->
  is_stale:bool ->
  'res ServerCommandTypes.t ->
  ServerEnv.env * 'res
