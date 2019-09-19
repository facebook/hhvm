(*
 * Copyright (c) 2017, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

type t = {
  type_constant_name: string;
  type_constant_initializer: Typed_value.t option; (* TODO: constraint? *)
}

let make type_constant_name type_constant_initializer =
  { type_constant_name; type_constant_initializer }

let name c = c.type_constant_name

let initializer_t c = c.type_constant_initializer
