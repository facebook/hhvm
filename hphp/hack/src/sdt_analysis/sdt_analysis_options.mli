(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Sdt_analysis_types

val parse_command : string -> Options.command option

val mk : command:Options.command -> verbosity:int -> Options.t
