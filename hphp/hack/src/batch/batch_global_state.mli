(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

type batch_state

val save : trace:bool -> batch_state

val worker_id_str : worker_id:int -> string

val restore : batch_state -> worker_id:int -> unit
