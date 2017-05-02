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
  type_constant_name         : Litstr.id;
  type_constant_initializer  : Typed_value.t;
  (* TODO: constraint? *)
}

let make type_constant_name type_constant_initializer =
  { type_constant_name; type_constant_initializer}

let name c = c.type_constant_name
let initializer_t c = c.type_constant_initializer
