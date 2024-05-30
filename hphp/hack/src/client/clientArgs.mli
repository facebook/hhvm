(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

val parse_args : from_default:string -> ClientCommand.command

val root : ClientCommand.command -> Path.t option

val from : ClientCommand.command -> string

val config : ClientCommand.command -> (string * string) list option
