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
  (* TODO: initializer *)
}

(* Interestingly, HHAS does not represent the declared types of constants,
unlike formal parameters and return types. We might consider fixing this. *)

(* Also interestingly, abstract constants are not emitted at all. *)

let make constant_name =
  { constant_name }

let name hhas_constant = hhas_constant.constant_name
