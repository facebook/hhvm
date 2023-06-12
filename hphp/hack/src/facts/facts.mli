(*
 * Copyright (c) 2018, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

module InvStringKey : Map.OrderedType with type t = string

module InvSMap : WrappedMap.S with type key = InvStringKey.t

module InvSSet : Caml.Set.S with type elt = InvStringKey.t

type type_kind =
  | TKClass
  | TKInterface
  | TKEnum
  | TKTrait
  | TKTypeAlias
  | TKUnknown
  | TKMixed

val is_tk_interface : type_kind -> bool

val is_tk_trait : type_kind -> bool

val is_tk_unknown : type_kind -> bool

type type_facts = {
  base_types: InvSSet.t;
  kind: type_kind;
  flags: int;
  require_extends: InvSSet.t;
  require_implements: InvSSet.t;
  require_class: InvSSet.t;
  attributes: string list InvSMap.t;
}

type module_facts = unit

type facts = {
  types: type_facts InvSMap.t;
  functions: string list;
  constants: string list;
  modules: module_facts InvSMap.t;
}

val empty : facts

val facts_from_json : Hh_json.json -> facts option
