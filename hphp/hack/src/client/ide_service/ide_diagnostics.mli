(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

(** Convert a list of [Diagnostics.finalized_diagnostic] to [ClientIdeMessage.diagnostic] *)
val convert :
  ctx:Provider_context.t ->
  entry:Provider_context.entry ->
  (Diagnostics.finalized_diagnostic * int) list ->
  ClientIdeMessage.diagnostic list
