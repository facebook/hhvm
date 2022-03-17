(*
 * Copyright (c) 2019, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

exception Malformed of string

val write_message : out_channel -> string -> unit

val read_headers : Buffered_line_reader.t -> string list

(* The rest of these methods are exposed solely for unit tests. *)

val parse_headers_to_lowercase_map : string list -> string SMap.t

val parse_charset : string -> string option

val read_message_utf8 : Buffered_line_reader.t -> string
