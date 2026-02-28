(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

val render : ?add_bold:bool -> ?color:Tty.raw_color -> string -> string

val md_codify : string -> string

val md_highlight : string -> string

val md_bold : string -> string

val md_italicize : string -> string
