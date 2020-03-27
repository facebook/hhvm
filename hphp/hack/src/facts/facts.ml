(*
 * Copyright (c) 2018, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Core_kernel
module J = Hh_json

module InvStringKey = struct
  type t = string

  let compare (x : t) (y : t) = String.compare y x
end

module InvSMap = WrappedMap.Make (InvStringKey)
module InvSSet = Caml.Set.Make (InvStringKey)

type type_kind =
  | TKClass
  | TKRecord
  | TKInterface
  | TKEnum
  | TKTrait
  | TKTypeAlias
  | TKUnknown
  | TKMixed

let type_kind_to_string = function
  | TKClass -> "class"
  | TKRecord -> "record"
  | TKInterface -> "interface"
  | TKEnum -> "enum"
  | TKTrait -> "trait"
  | TKTypeAlias -> "typeAlias"
  | TKUnknown -> "unknown"
  | TKMixed -> "mixed"

let type_kind_from_string s =
  if s = "class" then
    TKClass
  else if s = "record" then
    TKRecord
  else if s = "interface" then
    TKInterface
  else if s = "enum" then
    TKEnum
  else if s = "trait" then
    TKTrait
  else if s = "typeAlias" then
    TKTypeAlias
  else if s = "unknown" then
    TKUnknown
  else if s = "mixed" then
    TKMixed
  else
    raise (Failure (Printf.sprintf "No Facts.type_kind matches string: %s" s))

type type_facts = {
  base_types: InvSSet.t;
  kind: type_kind;
  flags: int;
  require_extends: InvSSet.t;
  require_implements: InvSSet.t;
  attributes: string list InvSMap.t;
}

let empty_type_facts =
  {
    base_types = InvSSet.empty;
    kind = TKUnknown;
    flags = 0;
    require_extends = InvSSet.empty;
    require_implements = InvSSet.empty;
    attributes = InvSMap.empty;
  }

type facts = {
  types: type_facts InvSMap.t;
  functions: string list;
  constants: string list;
  type_aliases: string list;
}

let empty =
  { types = InvSMap.empty; functions = []; constants = []; type_aliases = [] }

(* Facts to JSON *)

let hex_number_to_json s =
  let number = "0x" ^ s |> Int64.of_string |> Int64.to_string in
  J.JSON_Number number

let add_set_member ~include_empty name values members =
  if InvSSet.is_empty values && not include_empty then
    members
  else
    let elements =
      InvSSet.fold (fun el acc -> J.JSON_String el :: acc) values []
    in
    (name, J.JSON_Array elements) :: members

let list_to_json_array l =
  let elements =
    List.fold_left l ~init:[] ~f:(fun acc el -> J.JSON_String el :: acc)
  in
  J.JSON_Array elements

let map_to_json_object m =
  let foo =
    InvSMap.fold (fun name v acc -> (name, list_to_json_array v) :: acc) m []
  in
  J.JSON_Object foo

let add_map_member name values members =
  if InvSMap.is_empty values then
    members
  else
    let elements = map_to_json_object values in
    (name, elements) :: members

let type_facts_to_json name tf =
  let members =
    add_set_member
      ~include_empty:(tf.kind = TKInterface || tf.kind = TKTrait)
      "requireExtends"
      tf.require_extends
      []
    |> add_set_member
         ~include_empty:(tf.kind = TKTrait)
         "requireImplements"
         tf.require_implements
    |> add_map_member "attributes" tf.attributes
    |> add_set_member ~include_empty:true "baseTypes" tf.base_types
  in
  let members =
    ("name", J.JSON_String name)
    :: ("kindOf", J.JSON_String (type_kind_to_string tf.kind))
    :: ("flags", J.JSON_Number (string_of_int tf.flags))
    :: members
  in
  J.JSON_Object members

let facts_to_json ~md5 ~sha1 facts =
  let md5sum0 = ("md5sum0", hex_number_to_json (String.sub md5 0 16)) in
  let md5sum1 = ("md5sum1", hex_number_to_json (String.sub md5 16 16)) in
  let sha1sum = ("sha1sum", J.JSON_String sha1) in
  let type_facts_json =
    let elements =
      InvSMap.fold
        (fun name v acc -> type_facts_to_json name v :: acc)
        facts.types
        []
    in
    ("types", J.JSON_Array elements)
  in
  let functions_json = ("functions", list_to_json_array facts.functions) in
  let constants_json = ("constants", list_to_json_array facts.constants) in
  let type_aliases_json =
    ("typeAliases", list_to_json_array facts.type_aliases)
  in
  J.JSON_Object
    [
      md5sum0;
      md5sum1;
      sha1sum;
      type_facts_json;
      functions_json;
      constants_json;
      type_aliases_json;
    ]

(* Facts from JSON *)

let facts_from_json : Hh_json.json -> facts option =
  Hh_json.(
    let list_from_jstr_array = List.rev_map ~f:get_string_exn in
    let type_facts_from_jobj entry : string * type_facts =
      let set_from_jstr_array =
        List.fold ~init:InvSSet.empty ~f:(fun acc j ->
            InvSSet.add (get_string_exn j) acc)
      in
      let name_ref = ref "" in
      let ret =
        List.fold_left
          ~init:empty_type_facts
          ~f:(fun acc (k, v) ->
            match v with
            | JSON_String name when k = "name" ->
              name_ref := name;
              acc
            | JSON_String kind when k = "kindOf" ->
              { acc with kind = type_kind_from_string kind }
            | JSON_Number flags when k = "flags" ->
              { acc with flags = int_of_string flags }
            | JSON_Array xs when k = "baseTypes" ->
              { acc with base_types = set_from_jstr_array xs }
            | JSON_Array xs when k = "requireExtends" ->
              { acc with require_extends = set_from_jstr_array xs }
            | JSON_Array xs when k = "requireImplements" ->
              { acc with require_implements = set_from_jstr_array xs }
            | JSON_Object key_values when k = "attributes" ->
              {
                acc with
                attributes =
                  List.fold_left
                    ~init:InvSMap.empty
                    ~f:(fun acc -> function
                      | (k, JSON_Array attrs_json) ->
                        InvSMap.add k (list_from_jstr_array attrs_json) acc
                      | _ -> acc)
                    key_values;
              }
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
             | JSON_Array xs when k = "constants" ->
               { acc with constants = list_from_jstr_array xs }
             | JSON_Array xs when k = "functions" ->
               { acc with functions = list_from_jstr_array xs }
             | JSON_Array xs when k = "typeAliases" ->
               { acc with type_aliases = list_from_jstr_array xs }
             | JSON_Array types when k = "types" ->
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
