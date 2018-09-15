(**
 * Copyright (c) 2017, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
*)

(* string_of_instruction already appends a newline, so remove it *)
let droplast s = String.sub s 0 (String.length s - 1)

let string_of_instruction i = droplast (Hhbc_hhas.string_of_instruction i)

let string_of_instruction_type i =
  List.hd @@ String.split_on_char ' ' @@ string_of_instruction i
