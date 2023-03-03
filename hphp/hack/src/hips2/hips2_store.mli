(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)
open Hh_prelude

val persist : db_dir:string -> worker_id:int -> 'a -> unit

val un_persist : db_dir:string -> 'a Sequence.t
