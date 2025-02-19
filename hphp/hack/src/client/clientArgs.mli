(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

val parse_args : from_default:string -> ClientCommand.command

val root : ClientCommand.heavy_command -> Path.t

val from : ClientCommand.heavy_command -> string

val is_interactive : ClientCommand.heavy_command -> bool

val config : ClientCommand.heavy_command -> (string * string) list option

val dump_config : ClientCommand.heavy_command -> bool
