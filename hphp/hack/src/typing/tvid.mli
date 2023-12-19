(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

(** A type variable ID *)
type t [@@deriving eq, hash, ord, show]

type provider

val make : provider -> t

val make_provider : unit -> provider

module Map : WrappedMap_sig.S with type key = t

module Set : Set.S with type elt = t
