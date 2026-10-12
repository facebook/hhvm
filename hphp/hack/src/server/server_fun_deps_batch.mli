(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

val go :
  Multi_worker.worker list option ->
  (string * int * int) list ->
  Server_env.env ->
  string list

module Results :
  Stdlib.Set.S with type elt = Relative_path.t Symbol_occurrence.t

val handlers :
  ( Results.t,
    Relative_path.t Symbol_definition.t option list,
    Nast.program )
  Server_rx_api_shared.handlers
