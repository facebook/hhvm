(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

type t

module Ident : sig
  type t [@@deriving ord, eq, hash, show]

  module Map : WrappedMap_sig.S with type key = t

  module Set : Set.S with type elt = t

  val is_immutable : t -> bool

  val make_immutable : t -> t

  val dodgy_from_int : int -> t
end

val init : unit -> t

val provide : t -> Ident.t
