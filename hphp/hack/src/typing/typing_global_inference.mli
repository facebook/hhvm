(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Typing_env_types

val init : unit -> unit

val save_subgraphs : global_tvenv list -> unit

val merge_subgraph_in_env : tyvar_info_ IMap.t -> env -> env
