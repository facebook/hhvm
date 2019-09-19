(*
 * Copyright (c) 2019, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

let converter =
  let convert_pos p : Pos.t = p in
  Ast_to_aast.converter convert_pos Nast.NamedWithUnsafeBlocks () ()

let convert = converter#on_program

let on_class = converter#on_class

let on_fun = converter#on_fun

let on_typedef = converter#on_typedef

let on_constant = converter#on_constant
