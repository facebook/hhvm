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

type type_hint_kind =
| Property
| Return
| Param
| TypeDef

let fmt_name_or_prim ~tparams ~namespace x =
  let name = snd x in
  if List.mem tparams name || name = "self"
  then name
  else
    let fq_id, _ = Hhbc_id.Class.elaborate_id namespace x in
    Hhbc_id.Class.to_unmangled_string fq_id

(* Produce the "userType" bit of the annotation *)
let rec fmt_hint ~tparams ~namespace ?(strip_tparams=false) (_, h) =
  match h with
  | A.Happly (id, []) ->
    fmt_name_or_prim ~tparams ~namespace id

  | A.Happly (id, args) ->
    let name = fmt_name_or_prim ~tparams ~namespace id in
    if strip_tparams then name
    else name ^ "<" ^ fmt_hints ~tparams ~namespace args ^ ">"

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

let can_be_nullable h =
  match snd h with
  | A.Hfun (_, _, _)
  | A.Hoption (_, A.Hfun (_, _, _))
  | A.Happly ((_, "mixed"), _)
  | A.Hoption (_, A.Happly ((_, "mixed"), _)) -> false
  | _ -> true

let rec hint_to_type_constraint ~tparams ~skipawaitable ~namespace (_, h) =
match h with
| A.Happly ((_, ("mixed" | "void")), []) ->
  TC.make None []

| A.Hfun (_, _, _) ->
  TC.make None []

| A.Haccess _ ->
  let tc_name = Some "" in
  let tc_flags = [TC.HHType; TC.ExtendedHint; TC.TypeConstant] in
  TC.make tc_name tc_flags

  (* Elide the Awaitable class for async return types only *)
| A.Happly ((_, "WaitHandle"), [(_, A.Happly((_, "void"), []))])
| A.Happly ((_, "Awaitable"), [(_, A.Happly((_, "void"), []))])
  when skipawaitable ->
  TC.make None []

| A.Happly ((_, "WaitHandle"), [h])
| A.Hoption (_, A.Happly ((_, "WaitHandle"), [h]))
| A.Happly ((_, "Awaitable"), [h])
| A.Hoption (_, A.Happly ((_, "Awaitable"), [h]))
  when skipawaitable ->
  hint_to_type_constraint ~tparams ~skipawaitable:false ~namespace h

| A.Happly ((_, "WaitHandle"), [])
| A.Hoption (_, A.Happly ((_, "WaitHandle"), []))
| A.Happly ((_, "Awaitable"), [])
| A.Hoption (_, A.Happly ((_, "Awaitable"), []))
  when skipawaitable ->
  TC.make None []

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
  let tc_flags = [TC.HHType; TC.ExtendedHint] in
  TC.make tc_name tc_flags

| A.Hoption t ->
  make_tc_with_flags_if_non_empty_flags ~tparams ~skipawaitable ~namespace
    t [TC.Nullable; TC.HHType; TC.ExtendedHint]

| A.Hsoft t ->
  make_tc_with_flags_if_non_empty_flags ~tparams ~skipawaitable ~namespace
    t [TC.Soft; TC.HHType; TC.ExtendedHint]

and make_tc_with_flags_if_non_empty_flags
  ~tparams ~skipawaitable ~namespace t flags =
  let tc = hint_to_type_constraint ~tparams ~skipawaitable ~namespace t in
  let tc_name = TC.name tc in
  let tc_flags = TC.flags tc in
  match tc_name, tc_flags with
  | None, [] -> tc
  | _ ->
  let tc_flags = List.dedup (flags @ tc_flags) in
  TC.make tc_name tc_flags

let try_add_nullable ~nullable h flags =
  if nullable && can_be_nullable h then List.dedup (TC.Nullable :: flags)
  else flags

let make_type_info ~tparams ~namespace h tc_name tc_flags =
  let type_info_user_type = Some (fmt_hint ~tparams ~namespace h) in
  let type_info_type_constraint = TC.make tc_name tc_flags in
  Hhas_type_info.make type_info_user_type type_info_type_constraint

let param_hint_to_type_info ~skipawaitable ~nullable
  ~tparams ~namespace h =
  let is_simple_hint =
    match snd h with
    | A.Hsoft _ | A.Hoption _ | A.Haccess _
    | A.Hfun (_, _, _)
    | A.Happly (_, _::_)
    | A.Happly ((_, "mixed"), []) -> false
    | A.Happly ((_, id), _) when List.mem tparams id -> false
    | _ -> true
  in
  let tc = hint_to_type_constraint ~tparams ~skipawaitable ~namespace h in
  let tc_name = TC.name tc in
  if is_simple_hint
  then
    let is_hh_type =
      Emit_env.is_hh_file ()
      || Hhbc_options.enable_hiphop_syntax !Hhbc_options.compiler_options
    in
    let tc_flags = if is_hh_type then [TC.HHType] else [] in
    let tc_flags = try_add_nullable ~nullable h tc_flags in
    make_type_info ~tparams ~namespace h tc_name tc_flags
  else
    let tc_flags = TC.flags tc in
    let tc_flags = try_add_nullable ~nullable h tc_flags in
    make_type_info ~tparams ~namespace h tc_name tc_flags

let hint_to_type_info ~kind ~skipawaitable ~nullable ~tparams ~namespace h =
  match kind with
  | Param ->
    param_hint_to_type_info ~skipawaitable ~nullable ~tparams ~namespace h
  | _ ->
  let tc =
    if kind = Property then TC.make None []
    else hint_to_type_constraint ~tparams ~skipawaitable ~namespace h
  in
  let tc_name = TC.name tc in
  let tc_flags = TC.flags tc in
  let tc_flags =
    if kind = Return && tc_name <> None
    then List.dedup (TC.ExtendedHint :: tc_flags)
    else tc_flags in
  let tc_flags = try_add_nullable ~nullable h tc_flags in
  make_type_info ~tparams ~namespace h tc_name tc_flags

let hint_to_class ~namespace h =
  match h with
  | (_, A.Happly (id, _)) ->
    let fq_id, _ = Hhbc_id.Class.elaborate_id namespace id in
    fq_id
  | _ -> Hhbc_id.Class.from_raw_string "__type_is_not_class__"
