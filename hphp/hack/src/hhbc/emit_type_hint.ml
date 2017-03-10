(**
 * Copyright (c) 2017, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 *
*)

open Core

module A = Ast
module TC = Hhas_type_constraint

(* TODO *)
let fmt_name s = s

let fmt_name_or_prim x =
  match x with
  | "void" -> "HH\\void"
  | "int" -> "HH\\int"
  | "bool" -> "HH\\bool"
  | "float" -> "HH\\float"
  | "string" -> "HH\\string"
  | "num" -> "HH\\num"
  | "resource" -> "HH\\resource"
  | "arraykey" -> "HH\\arraykey"
  | "noreturn" -> "HH\\noreturn"
  | "mixed" -> "HH\\mixed"
  | "this" -> "HH\\this"
  | _ -> fmt_name x

(* Produce the "userType" bit of the annotation *)
let rec fmt_hint (_, h) =
  match h with
  | A.Happly ((_, s), []) -> fmt_name_or_prim s
  | A.Happly ((_, s), args) ->
    fmt_name_or_prim s ^ "<" ^ String.concat ", " (List.map args fmt_hint) ^ ">"

  | A.Hfun (args, _, ret) ->
    "(function (" ^ String.concat ", " (List.map args fmt_hint) ^ "): " ^
      fmt_hint ret ^ ")"

  | A.Htuple hs ->
    "(" ^ String.concat ", " (List.map hs fmt_hint) ^ ")"

  | A.Haccess (h1, h2, accesses) ->
    String.concat "::" (List.map (h1::h2::accesses) snd)

  | A.Hoption t -> "?" ^ fmt_hint t

  | A.Hshape smap ->
    let fmt_field = function
      | A.SFlit (_, s) -> "'" ^ s ^ "'"
      | A.SFclass_const ((_, s1), (_, s2)) -> fmt_name s1 ^ "::" ^ s2
    in
    let format_shape_field ({ A.sf_name; A.sf_hint; _ }) =
      fmt_field sf_name ^ "=>" ^ fmt_hint sf_hint in
    let shape_fields =
      List.map ~f:format_shape_field smap in
    "HH\\shape(" ^ String.concat ", " shape_fields ^ ")"

let rec hint_to_type_constraint tparams (_, h) =
match h with
| A.Happly ((_, ("mixed" | "this" | "void")), []) ->
  TC.make None []

| A.Hfun (_, _, _) ->
  TC.make None []

| A.Haccess _ ->
  let tc_name = Some "" in
  let tc_flags = [TC.HHType; TC.ExtendedHint; TC.TypeConstant] in
  TC.make tc_name tc_flags

(* Need to differentiate between type params and classes *)
| A.Happly ((_, s), _) ->
  if List.mem tparams s then
    let tc_name = Some "" in
    let tc_flags = [TC.HHType; TC.ExtendedHint; TC.TypeVar] in
    TC.make tc_name tc_flags
  else
    let tc_name = Some (fmt_name_or_prim s) in
    let tc_flags = [TC.HHType] in
    TC.make tc_name tc_flags

(* Shapes and tuples are just arrays *)
| A.Hshape _ |  A.Htuple _ ->
  let tc_name = Some "array" in
  let tc_flags = [TC.HHType] in
  TC.make tc_name tc_flags

| A.Hoption t ->
  let tc = hint_to_type_constraint tparams t in
  let tc_name = TC.name tc in
  let tc_flags = TC.flags tc in
  let tc_flags = List.dedup
    ([TC.Nullable; TC.HHType; TC.ExtendedHint] @ tc_flags) in
  TC.make tc_name tc_flags

let hint_to_type_info ~always_extended tparams h =
  let tc = hint_to_type_constraint tparams h in
  let tc_name = TC.name tc in
  let tc_flags = TC.flags tc in
  let tc_flags =
    if always_extended && tc_name != None
    then List.dedup (TC.ExtendedHint :: tc_flags)
    else tc_flags in
  let type_info_user_type = Some (fmt_hint h) in
  let type_info_type_constraint = TC.make tc_name tc_flags in
  Hhas_type_info.make type_info_user_type type_info_type_constraint

let hints_to_type_infos ~always_extended tparams hints =
  let mapper hint = hint_to_type_info always_extended tparams hint in
  List.map hints mapper

let hint_to_class h =
  match h with
  | (_, A.Happly ((_, s), _)) -> s
  | _ -> "__type_is_not_class__"
