(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Shape_analysis_types

val parse_mode : string -> (command * mode) option

val mk : command:command -> mode:mode -> verbosity:int -> options
