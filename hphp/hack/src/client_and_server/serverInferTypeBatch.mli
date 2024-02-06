(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

type pos = Relative_path.t * int * int * (int * int) option

val get_tast_map :
  Provider_context.t ->
  Relative_path.t list ->
  Provider_context.t * Tast.program Tast_with_dynamic.t Relative_path.Map.t

val go :
  MultiWorker.worker list option ->
  (string * int * int * (int * int) option) list ->
  ServerEnv.env ->
  string list
