(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

(** Convert a list of [Errors.finalized_error] to [ClientIdeMessage.diagnostic] *)
val convert :
  ctx:Provider_context.t ->
  entry:Provider_context.entry ->
  (Errors.finalized_error * int) list ->
  ClientIdeMessage.diagnostic list
