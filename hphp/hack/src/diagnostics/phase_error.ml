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

  val to_user_diagnostic : t -> (Pos.t, Pos_or_decl.t) User_diagnostic.t
end
