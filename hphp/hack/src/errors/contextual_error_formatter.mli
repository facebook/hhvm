(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

val to_string :
  ?claim_color:Tty.raw_color -> Pos.absolute Errors.error_ -> string
