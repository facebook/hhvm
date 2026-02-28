(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

(** A type variable id *)
type t = Ident_provider.Ident.t [@@deriving eq, hash, ord, show]

type provider = Ident_provider.t

let make provider = Ident_provider.provide provider

let make_provider () = Ident_provider.init ()

module Map = Ident_provider.Ident.Map
module Set = Ident_provider.Ident.Set
