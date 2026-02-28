(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

module type Operation = sig
  type t [@@deriving eq]
end

module Make (Op : Operation) : sig
  type t

  val empty : t

  val add_op_and_check_infinite_recursion : t -> Op.t -> (t, Op.t) result
end
