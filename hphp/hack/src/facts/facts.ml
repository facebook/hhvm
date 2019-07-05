(**
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
  let compare (x: t) (y: t) = Pervasives.compare y x
end

module InvSMap = MyMap.Make(InvStringKey)
module InvSSet = Caml.Set.Make(InvStringKey)

type type_kind =
  | TKClass
  | TKInterface
  | TKEnum
  | TKTrait
  | TKUnknown
  | TKMixed

let type_kind_to_string = function
  | TKClass -> "class"
  | TKInterface -> "interface"
  | TKEnum -> "enum"
  | TKTrait -> "trait"
  | TKUnknown -> "unknown"
  | TKMixed -> "mixed"

type type_facts = {
  base_types: InvSSet.t;
  kind: type_kind;
  flags: int;
  require_extends: InvSSet.t;
  require_implements: InvSSet.t;
}

type facts = {
  types: type_facts InvSMap.t;
  functions: string list;
  constants: string list;
  type_aliases: string list;
}

let empty = {
  types = InvSMap.empty;
  functions = [];
  constants = [];
  type_aliases = [];
}

(* Facts to JSON *)

let hex_number_to_json s =
  let number =
    "0x" ^ s
    |> Int64.of_string
    |> Int64.to_string in
  J.JSON_Number number

let add_member ~include_empty name values members =
  if InvSSet.is_empty values && not include_empty
  then members
  else
    let elements = InvSSet.fold (fun el acc -> J.JSON_String el :: acc ) values [] in
    (name, J.JSON_Array elements) :: members

let list_to_json_array l =
  let elements =
    List.fold_left l ~init:[] ~f:(fun acc el -> J.JSON_String el :: acc) in
  J.JSON_Array elements

let type_facts_to_json name tf =
  let members =
    add_member ~include_empty:(tf.kind = TKInterface || tf.kind = TKTrait)
      "requireExtends" tf.require_extends []
    |> add_member ~include_empty:(tf.kind = TKTrait)
      "requireImplements" tf.require_implements
    |> add_member ~include_empty:true "baseTypes" tf.base_types in
  let members =
    ("name", J.JSON_String name)::
    ("kindOf", J.JSON_String (type_kind_to_string tf.kind))::
    ("flags", J.JSON_Number (string_of_int tf.flags)) ::
    members in
  J.JSON_Object members

let facts_to_json ~md5 ~sha1 facts =
  let md5sum0 =
    "md5sum0", hex_number_to_json (String.sub md5 0 16) in
  let md5sum1 =
    "md5sum1", hex_number_to_json (String.sub md5 16 16) in
  let sha1sum = ("sha1sum", J.JSON_String sha1) in
  let type_facts_json =
    let elements =
      InvSMap.fold (fun name v acc -> (type_facts_to_json name v) :: acc)
        facts.types [] in
    "types", J.JSON_Array elements in
  let functions_json =
    "functions", list_to_json_array facts.functions in
  let constants_json =
    "constants", list_to_json_array facts.constants in
  let type_aliases_json =
    "typeAliases", list_to_json_array facts.type_aliases in
  J.JSON_Object [
    md5sum0;
    md5sum1;
    sha1sum;
    type_facts_json;
    functions_json;
    constants_json;
    type_aliases_json; ]
