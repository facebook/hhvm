(*
 * Copyright (c) 2019, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

val is_lit_printable : char -> bool

val escape_char : char -> string

val escape : ?f:(char -> string) -> string -> string
