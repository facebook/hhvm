(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Shape_analysis_types

val init : Tast.saved_env -> env

val add_constraint : env -> constraint_ -> env

val get_local : Local_id.t -> env -> entity

val set_local : Local_id.t -> entity -> env -> env

val merge : env -> env -> env
