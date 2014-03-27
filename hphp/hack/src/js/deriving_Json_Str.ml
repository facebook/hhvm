(* Js_of_ocaml
 * http://www.ocsigen.org
 * Copyright Grgoire Henry 2010.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, with linking exception;
 * either version 2.1 of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 *)

let write buffer s =
  Buffer.add_char buffer '\"';
  for i = 0 to String.length s - 1 do
    match s.[i] with
    | '\"' -> Buffer.add_string buffer "\\\""
    | '\\' -> Buffer.add_string buffer "\\\\"
    | '\b' -> Buffer.add_string buffer "\\b"
    | '\x0C' -> Buffer.add_string buffer "\\f"
    | '\n' -> Buffer.add_string buffer "\\n"
    | '\r' -> Buffer.add_string buffer "\\r"
    | '\t' -> Buffer.add_string buffer "\\t"
    | c when c <= '\x1F' -> (* Other control characters are escaped. *)
      Printf.bprintf buffer "\\u%04X" (int_of_char c)
        | c when c < '\x80' ->
      Buffer.add_char buffer s.[i]
        | c (* >= '\x80' *) -> (* Bytes greater than 127 are embeded in a UTF-8 sequence. *)
      Buffer.add_char buffer (Char.chr (0xC2 lor (Char.code s.[i] lsr 6)));
      Buffer.add_char buffer (Char.chr (0x80 lor (Char.code s.[i] land 0x3F)))
      done;
      Buffer.add_char buffer '\"'
