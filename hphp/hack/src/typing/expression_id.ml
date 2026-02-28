(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

type t = Ident_provider.Ident.t [@@deriving eq, hash, ord, show]

type provider = Ident_provider.t

let make provider = Ident_provider.provide provider

module Map = Ident_provider.Ident.Map

let make_provider () = Ident_provider.init ()

let make_immutable t = Ident_provider.Ident.make_immutable t

let is_immutable t = Ident_provider.Ident.is_immutable t

let display (id : t) = Printf.sprintf "<expr#%s>" (Ident_provider.Ident.show id)

let debug x = show x

let dodgy_from_int x = Ident_provider.Ident.dodgy_from_int x
