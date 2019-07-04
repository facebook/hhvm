(**
 * Copyright (c) 2018, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

type type_kind =
  | TKClass
  | TKInterface
  | TKEnum
  | TKTrait
  | TKUnknown
  | TKMixed

type type_facts = {
  base_types: SSet.t;
  kind: type_kind;
  flags: int;
  require_extends: SSet.t;
  require_implements: SSet.t;
}

type facts = {
  types: type_facts SMap.t;
  functions: string list;
  constants: string list;
  type_aliases: string list;
}

val empty: facts

val facts_to_json: md5:string -> sha1:string -> facts -> Hh_json.json
