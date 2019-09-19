(*
 * Copyright (c) 2018, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

module InvStringKey : Map.OrderedType with type t = string

module InvSMap : MyMap.S with type key = InvStringKey.t

module InvSSet : Caml.Set.S with type elt = InvStringKey.t

type type_kind =
  | TKClass
  | TKInterface
  | TKEnum
  | TKTrait
  | TKTypeAlias
  | TKUnknown
  | TKMixed

type type_facts = {
  base_types: InvSSet.t;
  kind: type_kind;
  flags: int;
  require_extends: InvSSet.t;
  require_implements: InvSSet.t;
  attributes: string list InvSMap.t;
}

type facts = {
  types: type_facts InvSMap.t;
  functions: string list;
  constants: string list;
  type_aliases: string list;
}

val empty : facts

val facts_to_json : md5:string -> sha1:string -> facts -> Hh_json.json

val facts_from_json : Hh_json.json -> facts option
