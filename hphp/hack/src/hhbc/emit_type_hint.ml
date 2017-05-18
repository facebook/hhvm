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
open Hhbc_string_utils

module A = Ast
module TC = Hhas_type_constraint

let fmt_name_or_prim ~tparams ~namespace x =
  if List.mem tparams (snd x)
  then snd x
  else
    let fq_id, _ = Hhbc_id.Class.elaborate_id namespace x in
    Hhbc_id.Class.to_unmangled_string fq_id

(* Produce the "userType" bit of the annotation *)
let rec fmt_hint ~tparams ~namespace (_, h) =
  match h with
  | A.Happly (id, []) ->
    fmt_name_or_prim ~tparams ~namespace id

  | A.Happly (id, args) ->
    fmt_name_or_prim ~tparams ~namespace id
    ^ "<" ^ fmt_hints ~tparams ~namespace args ^ ">"

  | A.Hfun (args, _, ret) ->
    "(function (" ^ fmt_hints ~tparams ~namespace args
    ^ "): " ^ fmt_hint ~tparams ~namespace ret ^ ")"

  | A.Htuple hs ->
    "(" ^ fmt_hints ~tparams ~namespace hs ^ ")"

  | A.Haccess (h1, h2, accesses) ->
    fmt_name_or_prim ~tparams ~namespace h1 ^ "::" ^
      String.concat "::" (List.map (h2::accesses) snd)

  | A.Hoption t -> "?" ^ fmt_hint ~tparams ~namespace t

  | A.Hsoft h -> "@" ^ fmt_hint ~tparams ~namespace h

  | A.Hshape { A.si_allows_unknown_fields; si_shape_field_list } ->
    let fmt_field = function
      | A.SFlit (_, s) -> "'" ^ s ^ "'"
      | A.SFclass_const (cid, (_, s2)) ->
        fmt_name_or_prim ~tparams ~namespace cid ^ "::" ^ s2
    in
    let format_shape_field ({ A.sf_name; A.sf_hint; _ }) =
      fmt_field sf_name ^ "=>" ^ fmt_hint ~tparams ~namespace sf_hint in
    let shape_fields =
      List.map ~f:format_shape_field si_shape_field_list in
    let shape_suffix = if si_allows_unknown_fields then ["..."] else [] in
    let formatted_shape_entries = shape_fields @ shape_suffix in
    prefix_namespace "HH" "shape(" ^
      String.concat ", " formatted_shape_entries ^ ")"

and fmt_hints ~tparams ~namespace hints =
  String.concat ", " (List.map hints (fmt_hint ~tparams ~namespace))

let rec hint_to_type_constraint ~tparams ~skipawaitable ~namespace (_, h) =
match h with
| A.Happly ((_, ("mixed" | "this" | "void")), []) ->
  TC.make None []

| A.Hfun (_, _, _) ->
  TC.make None []

| A.Haccess _ ->
  let tc_name = Some "" in
  let tc_flags = [TC.HHType; TC.ExtendedHint; TC.TypeConstant] in
  TC.make tc_name tc_flags

  (* Elide the Awaitable class for async return types only *)
| A.Happly ((_, "Awaitable"), [(_, A.Happly((_, "void"), []))])
  when skipawaitable ->
  TC.make None []

| A.Happly ((_, "Awaitable"), [h])
| A.Hoption (_, A.Happly ((_, "Awaitable"), [h]))
  when skipawaitable ->
  hint_to_type_constraint ~tparams ~skipawaitable:false ~namespace h

(* Need to differentiate between type params and classes *)
| A.Happly (id, _) ->
  if List.mem tparams (snd id) then
    let tc_name = Some "" in
    let tc_flags = [TC.HHType; TC.ExtendedHint; TC.TypeVar] in
    TC.make tc_name tc_flags
  else
    let tc_name =
      let fq_id, _ = Hhbc_id.Class.elaborate_id namespace id in
      Hhbc_id.Class.to_raw_string fq_id in
    let tc_flags = [TC.HHType] in
    TC.make (Some tc_name) tc_flags

(* Shapes and tuples are just arrays *)
| A.Hshape _ |  A.Htuple _ ->
  let tc_name = Some "array" in
  let tc_flags = [TC.HHType] in
  TC.make tc_name tc_flags

| A.Hoption t ->
  let tc = hint_to_type_constraint ~tparams ~skipawaitable ~namespace t in
  let tc_name = TC.name tc in
  let tc_flags = TC.flags tc in
  let tc_flags = List.dedup
    ([TC.Nullable; TC.HHType; TC.ExtendedHint] @ tc_flags) in
  TC.make tc_name tc_flags

| A.Hsoft t ->
  let tc = hint_to_type_constraint ~tparams ~skipawaitable ~namespace t in
  let tc_name = TC.name tc in
  let tc_flags = TC.flags tc in
  let tc_flags = List.dedup
    ([TC.Soft; TC.HHType; TC.ExtendedHint] @ tc_flags) in
  TC.make tc_name tc_flags

let hint_to_type_info ~skipawaitable ~always_extended ~tparams ~namespace h =
  let tc = hint_to_type_constraint ~tparams ~skipawaitable ~namespace h in
  let tc_name = TC.name tc in
  let tc_flags = TC.flags tc in
  let tc_flags =
    if always_extended && tc_name <> None
    then List.dedup (TC.ExtendedHint :: tc_flags)
    else tc_flags in
  let type_info_user_type = Some (fmt_hint ~tparams ~namespace h) in
  let type_info_type_constraint = TC.make tc_name tc_flags in
  Hhas_type_info.make type_info_user_type type_info_type_constraint

let hint_to_class ~namespace h =
  match h with
  | (_, A.Happly (id, _)) ->
    let fq_id, _ = Hhbc_id.Class.elaborate_id namespace id in
    fq_id
  | _ -> Hhbc_id.Class.from_raw_string "__type_is_not_class__"
