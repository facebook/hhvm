(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

module type S = sig
  type t

  module Error_code : Error_category.S

  val error_code : t -> Error_code.t

  val claim : t -> Pos.t * string

  val reasons : t -> (Pos_or_decl.t * string) list

  val quickfixes : t -> Quickfix.t list
end
