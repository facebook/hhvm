(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

val go :
  MultiWorker.worker list option ->
  (string * int * int) list ->
  ServerEnv.env ->
  string list

module Results : Stdlib.Set.S with type elt = Relative_path.t SymbolOccurrence.t

val handlers :
  ( Results.t,
    Relative_path.t SymbolDefinition.t option list,
    Nast.program )
  ServerRxApiShared.handlers
