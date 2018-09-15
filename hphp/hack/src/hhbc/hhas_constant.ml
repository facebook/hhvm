(**
 * Copyright (c) 2017, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
*)

type t = {
  constant_name         : string;
  constant_value        : Typed_value.t option;
  constant_initializer_instrs : Instruction_sequence.t option;
}

(* Interestingly, HHAS does not represent the declared types of constants,
unlike formal parameters and return types. We might consider fixing this. *)

(* Also interestingly, abstract constants are not emitted at all. *)

let make constant_name constant_value constant_initializer_instrs =
  { constant_name; constant_value; constant_initializer_instrs }

let name hhas_constant = hhas_constant.constant_name
let value hhas_constant = hhas_constant.constant_value
let initializer_instrs hhas_constant = hhas_constant.constant_initializer_instrs
