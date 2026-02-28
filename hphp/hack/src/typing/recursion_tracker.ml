(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Hh_prelude

module type Operation = sig
  type t [@@deriving eq]
end

module Make (Op : Operation) = struct
  type t = Op.t list

  let empty = []

  let add_op_and_check_infinite_recursion (t : t) (op : Op.t) =
    match List.find t ~f:(Op.equal op) with
    | None -> Ok (op :: t)
    | Some t' -> Error t'
end
