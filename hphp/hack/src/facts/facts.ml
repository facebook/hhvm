(*
 * Copyright (c) 2018, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Hh_prelude

module InvStringKey = struct
  type t = string

  let compare (x : t) (y : t) = String.compare y x
end

module InvSMap = WrappedMap.Make (InvStringKey)
module InvSSet = Caml.Set.Make (InvStringKey)

type type_kind =
  | TKClass
  | TKInterface
  | TKEnum
  | TKTrait
  | TKTypeAlias
  | TKUnknown
  | TKMixed

let is_tk_unknown = function
  | TKUnknown -> true
  | TKClass
  | TKInterface
  | TKEnum
  | TKTrait
  | TKTypeAlias
  | TKMixed ->
    false

let is_tk_interface = function
  | TKInterface -> true
  | TKClass
  | TKEnum
  | TKTrait
  | TKTypeAlias
  | TKUnknown
  | TKMixed ->
    false

let is_tk_trait = function
  | TKTrait -> true
  | TKClass
  | TKInterface
  | TKEnum
  | TKTypeAlias
  | TKUnknown
  | TKMixed ->
    false

let type_kind_from_string s =
  if String.equal s "class" then
    TKClass
  else if String.equal s "interface" then
    TKInterface
  else if String.equal s "enum" then
    TKEnum
  else if String.equal s "trait" then
    TKTrait
  else if String.equal s "typeAlias" then
    TKTypeAlias
  else if String.equal s "unknown" then
    TKUnknown
  else if String.equal s "mixed" then
    TKMixed
  else
    raise (Failure (Printf.sprintf "No Facts.type_kind matches string: %s" s))

type type_facts = {
  kind: type_kind;
  flags: int;
}

type module_facts = unit

let empty_type_facts = { kind = TKUnknown; flags = 0 }

type facts = {
  types: type_facts InvSMap.t;
  functions: string list;
  constants: string list;
}

let empty = { types = InvSMap.empty; functions = []; constants = [] }

(* Facts from JSON *)

let facts_from_json : Hh_json.json -> facts option =
  Hh_json.(
    let list_from_jstr_array = List.rev_map ~f:get_string_exn in
    let type_facts_from_jobj entry : string * type_facts =
      let name_ref = ref "" in
      let ret =
        List.fold_left
          ~init:empty_type_facts
          ~f:(fun acc (k, v) ->
            match v with
            | JSON_String name when String.equal k "name" ->
              name_ref := name;
              acc
            | JSON_String kind when String.equal k "kindOf" ->
              { acc with kind = type_kind_from_string kind }
            | JSON_Number flags when String.equal k "flags" ->
              { acc with flags = int_of_string flags }
            | _ -> acc)
          entry
      in
      (!name_ref, ret)
    in
    function
    | JSON_Object key_values ->
      Some
        (List.fold
           ~init:empty
           ~f:(fun acc (k, v) ->
             match v with
             | JSON_Array xs when String.equal k "constants" ->
               { acc with constants = list_from_jstr_array xs }
             | JSON_Array xs when String.equal k "functions" ->
               { acc with functions = list_from_jstr_array xs }
             | JSON_Array types when String.equal k "types" ->
               {
                 acc with
                 types =
                   List.fold_left
                     ~init:InvSMap.empty
                     ~f:(fun acc v ->
                       match v with
                       | JSON_Object entry ->
                         let (k, v) = type_facts_from_jobj entry in
                         InvSMap.add k v acc
                       | _ -> acc)
                     types;
               }
             | _ -> acc)
           key_values)
    | _ -> None)
