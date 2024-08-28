(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

module type S = sig
  val handler : Tast_visitor.handler
end

module Warning = struct
  module type S = sig
    val error_codes : Error_codes.Warning.t list

    val handler : as_lint:bool -> Tast_visitor.handler
  end
end
