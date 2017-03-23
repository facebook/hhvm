(**
 * Copyright (c) 2017, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 *
*)

type t = {
  constant_name         : Litstr.id;
  constant_value        : Hhbc_ast.instruct_lit_const;
}

(* Interestingly, HHAS does not represent the declared types of constants,
unlike formal parameters and return types. We might consider fixing this. *)

(* Also interestingly, abstract constants are not emitted at all. *)

let make constant_name constant_value =
  { constant_name; constant_value }

let name hhas_constant = hhas_constant.constant_name
let value hhas_constant = hhas_constant.constant_value
