(*
 * Copyright (c) 2017, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Core_kernel
open Hhbc_string_utils
module A = Ast_defs
module SN = Naming_special_names
module TC = Hhas_type_constraint

type type_hint_kind =
  | Property
  | Return
  | Param
  | TypeDef
  | UpperBound

(* Produce the "userType" bit of the annotation *)
let rec fmt_name_or_prim ~tparams (_, name) =
  if List.mem ~equal:( = ) tparams name || is_self name || is_parent name then
    name
  else
    let needs_unmangling = Xhp.is_xhp (strip_ns name) in
    let id = Hhbc_id.Class.from_ast_name name in
    if needs_unmangling then
      Hhbc_id.Class.to_unmangled_string id
    else
      Hhbc_id.Class.to_raw_string id

and prim_to_string prim =
  match prim with
  | Aast.Tnull -> SN.Typehints.null
  | Aast.Tvoid -> SN.Typehints.void
  | Aast.Tint -> SN.Typehints.int
  | Aast.Tbool -> SN.Typehints.bool
  | Aast.Tfloat -> SN.Typehints.float
  | Aast.Tstring -> SN.Typehints.string
  | Aast.Tresource -> SN.Typehints.resource
  | Aast.Tnum -> SN.Typehints.num
  | Aast.Tarraykey -> SN.Typehints.arraykey
  | Aast.Tnoreturn -> SN.Typehints.noreturn
  | Aast.Tatom s -> ":@" ^ s

and fmt_hint ~tparams ?(strip_tparams = false) (pos, h) =
  let fmt_name_or_prim = fmt_name_or_prim ~tparams in
  match h with
  | Aast.Happly (id, []) -> fmt_name_or_prim id
  | Aast.Happly (id, args) ->
    let name = fmt_name_or_prim id in
    if strip_tparams then
      name
    else
      name ^ "<" ^ fmt_hints ~tparams args ^ ">"
  | Aast.Hfun Aast.{ hf_is_coroutine = true; _ } ->
    failwith "Codegen for coroutine functions is not supported"
  | Aast.Hfun
      Aast.
        {
          hf_reactive_kind = _;
          hf_is_coroutine = false;
          hf_param_tys = args;
          hf_param_kinds = _;
          hf_param_mutability = _;
          hf_variadic_ty = _;
          hf_return_ty = ret;
          hf_is_mutable_return = _;
        } ->
    (* TODO(mqian): Implement for inout parameters *)
    "(function ("
    ^ fmt_hints ~tparams args
    ^ "): "
    ^ fmt_hint ~tparams ret
    ^ ")"
  | Aast.Htuple hs -> "(" ^ fmt_hints ~tparams hs ^ ")"
  | Aast.Haccess ((_, Aast.Happly (id, _)), accesses) ->
    fmt_name_or_prim id ^ "::" ^ String.concat ~sep:"::" (List.map accesses snd)
  | Aast.Haccess _ -> failwith "ast_to_nast error. Should be Haccess(Happly())"
  (* Follow HHVM order: soft -> option *)
  (* Can we fix this eventually? *)
  | Aast.Hoption (_, Aast.Hsoft t) -> "@?" ^ fmt_hint ~tparams t
  | Aast.Hoption t -> "?" ^ fmt_hint ~tparams t
  | Aast.Hlike t -> "~" ^ fmt_hint ~tparams t
  | Aast.Hsoft h -> "@" ^ fmt_hint ~tparams h
  (* No guarantee that this is in the correct order when using map instead of list
   * TODO: Check whether shape fields need to retain order *)
  | Aast.Hshape { Aast.nsi_field_map; _ } ->
    let fmt_field_name name =
      match name with
      | A.SFlit_int (_, s_i) -> s_i
      | A.SFlit_str (_, s) -> "'" ^ s ^ "'"
      | A.SFclass_const (cid, (_, s2)) -> fmt_name_or_prim cid ^ "::" ^ s2
    in
    let format { Aast.sfi_hint; Aast.sfi_optional; Aast.sfi_name } =
      let prefix =
        if sfi_optional then
          "?"
        else
          ""
      in
      prefix ^ fmt_field_name sfi_name ^ "=>" ^ fmt_hint ~tparams sfi_hint
    in
    let shape_fields = List.map ~f:format nsi_field_map in
    prefix_namespace "HH" "shape(" ^ String.concat ~sep:", " shape_fields ^ ")"
  | Aast.Hprim p -> fmt_name_or_prim (pos, prim_to_string p)
  (* Didn't exist in the AST *)
  | Aast.Herr
  | Aast.Hany ->
    failwith "I'm convinced that this should be an error caught in naming"
  | Aast.Hmixed -> fmt_name_or_prim (pos, SN.Typehints.mixed)
  | Aast.Hnonnull -> fmt_name_or_prim (pos, SN.Typehints.nonnull)
  | Aast.Habstr s -> fmt_name_or_prim (pos, s)
  | Aast.Harray _ -> fmt_name_or_prim (pos, SN.Typehints.array)
  | Aast.Hdarray _ -> fmt_name_or_prim (pos, SN.Typehints.darray)
  | Aast.Hvarray _ -> fmt_name_or_prim (pos, SN.Typehints.varray)
  | Aast.Hvarray_or_darray _ ->
    fmt_name_or_prim (pos, SN.Typehints.varray_or_darray)
  | Aast.Hthis -> fmt_name_or_prim (pos, SN.Typehints.this)
  | Aast.Hdynamic -> fmt_name_or_prim (pos, SN.Typehints.dynamic)
  | Aast.Hnothing -> fmt_name_or_prim (pos, SN.Typehints.nothing)
  | Aast.Hpu_access (h, sid, _) ->
    "(" ^ fmt_hint ~tparams h ^ ":@" ^ snd sid ^ ")"
  | Aast.Hunion _ -> fmt_name_or_prim (pos, SN.Typehints.mixed)
  | Aast.Hintersection _ -> fmt_name_or_prim (pos, SN.Typehints.mixed)

and fmt_hints ~tparams hints =
  String.concat ~sep:", " (List.map hints (fmt_hint ~tparams))

(* Differs from above in that this assumes that naming has occurred *)
let can_be_nullable (_, h) =
  match h with
  | Aast.Hfun _
  | Aast.Hoption (_, Aast.Hfun _)
  | Aast.Happly ((_, "\\HH\\dynamic"), _)
  | Aast.Hoption (_, Aast.Happly ((_, "\\HH\\dynamic"), _))
  | Aast.Happly ((_, "\\HH\\nonnull"), _)
  | Aast.Hoption (_, Aast.Happly ((_, "\\HH\\nonnull"), _))
  | Aast.Happly ((_, "\\HH\\mixed"), _)
  | Aast.Hoption (_, Aast.Happly ((_, "\\HH\\mixed"), _))
  | Aast.Hdynamic
  | Aast.Hnonnull
  | Aast.Hmixed
  | Aast.Hoption (_, Aast.Hdynamic)
  | Aast.Hoption (_, Aast.Hnonnull)
  | Aast.Hoption (_, Aast.Hmixed) ->
    false
  | Aast.Haccess _ -> false
  (* HHVM does not emit nullable for type consts that are set to null by default
   * function(Class::Type $a = null) unless it is explicitly marked as nullable
   *)
  | Aast.Herr
  | Aast.Hany ->
    failwith "I'm convinced that this should be an error caught in naming"
  (* Naming converted the following from Happly's so assuming it should be true *)
  | Aast.Habstr _
  | Aast.Harray _
  | Aast.Hdarray _
  | Aast.Hvarray _
  | Aast.Hvarray_or_darray _
  | Aast.Hthis ->
    true
  | _ -> true

let rec hint_to_type_constraint ~kind ~tparams ~skipawaitable (p, h) =
  let happly_helper (pos, name) =
    if List.mem ~equal:( = ) tparams name then
      let tc_name =
        if kind = Param || kind = Return then
          name
        else
          ""
      in
      let tc_flags = [TC.ExtendedHint; TC.TypeVar] in
      TC.make (Some tc_name) tc_flags
    else if kind = TypeDef && (is_self name || is_parent name) then
      Emit_fatal.raise_fatal_runtime
        pos
        (Printf.sprintf "Cannot access %s when no class scope is active" name)
    else
      let tc_name =
        if is_self name || is_parent name then
          name
        else
          Hhbc_id.Class.(from_ast_name name |> to_raw_string)
      in
      TC.make (Some tc_name) []
  in
  let is_awaitable s = s = SN.Classes.cAwaitable in
  match h with
  (* The dynamic and nonnull types are treated by the runtime as mixed *)
  | Aast.Happly ((_, "\\HH\\dynamic"), [])
  | Aast.Happly ((_, "\\HH\\mixed"), [])
  | Aast.Hdynamic
  | Aast.Hlike _
  | Aast.Hfun _
  | Aast.Hunion _
  | Aast.Hintersection _
  | Aast.Hmixed ->
    TC.make None []
  | Aast.Hprim Aast.Tvoid when kind <> TypeDef -> TC.make None []
  | Aast.Happly ((_, s), [])
    when String.lowercase s = "\\hh\\void" && kind <> TypeDef ->
    TC.make None []
  | Aast.Haccess _ ->
    let tc_name = Some "" in
    let tc_flags = [TC.ExtendedHint; TC.TypeConstant] in
    TC.make tc_name tc_flags
  (* Elide the Awaitable class for async return types only *)
  | Aast.Happly ((_, s), [(_, Aast.Hprim Aast.Tvoid)])
  | Aast.Happly ((_, s), [(_, Aast.Happly ((_, "\\HH\\void"), []))])
    when skipawaitable && is_awaitable s ->
    TC.make None []
  | Aast.Happly ((_, s), [h])
  | Aast.Hoption (_, Aast.Happly ((_, s), [h]))
    when skipawaitable && is_awaitable s ->
    hint_to_type_constraint ~kind ~tparams ~skipawaitable:false h
  | Aast.Hoption (_, Aast.Hsoft (_, Aast.Happly ((_, s), [h])))
    when skipawaitable && is_awaitable s ->
    make_tc_with_flags_if_non_empty_flags
      ~kind
      ~tparams
      ~skipawaitable
      h
      [TC.Soft; TC.ExtendedHint]
  | Aast.Happly ((_, s), [])
  | Aast.Hoption (_, Aast.Happly ((_, s), []))
    when skipawaitable && is_awaitable s ->
    TC.make None []
  (* Need to differentiate between type params and classes *)
  | Aast.Happly ((pos, name), _) -> happly_helper (pos, name)
  (* Shapes and tuples are just arrays *)
  | Aast.Hshape _ ->
    let tc_name = Some "HH\\darray" in
    let tc_flags = [TC.ExtendedHint] in
    TC.make tc_name tc_flags
  | Aast.Htuple _ ->
    let tc_name = Some "HH\\varray" in
    let tc_flags = [TC.ExtendedHint] in
    TC.make tc_name tc_flags
  | Aast.Hoption t ->
    make_tc_with_flags_if_non_empty_flags
      ~kind
      ~tparams
      ~skipawaitable
      t
      [TC.Nullable; TC.DisplayNullable; TC.ExtendedHint]
  | Aast.Hsoft t ->
    make_tc_with_flags_if_non_empty_flags
      ~kind
      ~tparams
      ~skipawaitable
      t
      [TC.Soft; TC.ExtendedHint]
  | Aast.Herr
  | Aast.Hany ->
    failwith "I'm convinced that this should be an error caught in naming"
  (* Naming converted the following from Happly's so use the Happly logic here*)
  | Aast.Hnonnull -> happly_helper (p, SN.Typehints.nonnull)
  | Aast.Harray _ -> happly_helper (p, SN.Typehints.array)
  | Aast.Hdarray _ -> happly_helper (p, SN.Typehints.darray)
  | Aast.Hvarray _ -> happly_helper (p, SN.Typehints.varray)
  | Aast.Hvarray_or_darray _ -> happly_helper (p, SN.Typehints.varray_or_darray)
  | Aast.Hprim prim -> happly_helper (p, prim_to_string prim)
  | Aast.Hthis -> happly_helper (p, SN.Typehints.this)
  | Aast.Hnothing -> happly_helper (p, SN.Typehints.nothing)
  | Aast.Habstr s -> happly_helper (p, s)
  | Aast.Hpu_access _ -> TC.make None []

and make_tc_with_flags_if_non_empty_flags ~kind ~tparams ~skipawaitable t flags
    =
  let tc = hint_to_type_constraint ~kind ~tparams ~skipawaitable t in
  let tc_name = TC.name tc in
  let tc_flags = TC.flags tc in
  match (tc_name, tc_flags) with
  | (None, []) -> tc
  | _ ->
    let tc_flags = List.stable_dedup (flags @ tc_flags) in
    TC.make tc_name tc_flags

let add_nullable ~nullable flags =
  if nullable then
    List.stable_dedup (TC.Nullable :: TC.DisplayNullable :: flags)
  else
    flags

let try_add_nullable ~nullable h flags =
  add_nullable ~nullable:(nullable && can_be_nullable h) flags

let make_type_info ~tparams h tc_name tc_flags =
  let type_info_user_type = Some (fmt_hint ~tparams h) in
  let type_info_type_constraint = TC.make tc_name tc_flags in
  Hhas_type_info.make type_info_user_type type_info_type_constraint

let param_hint_to_type_info ~kind ~skipawaitable ~nullable ~tparams h =
  let is_simple_hint =
    match snd h with
    | Aast.Hsoft _
    | Aast.Hoption _
    | Aast.Haccess _
    | Aast.Hfun _
    | Aast.Happly (_, _ :: _)
    | Aast.Happly ((_, "\\HH\\dynamic"), [])
    | Aast.Happly ((_, "\\HH\\nonnull"), [])
    | Aast.Happly ((_, "\\HH\\mixed"), [])
    | Aast.Hdynamic
    | Aast.Hnonnull
    | Aast.Hmixed ->
      false
    (* I think Happly where id is in tparams is translated into Habstr *)
    | Aast.Habstr s when List.mem ~equal:( = ) tparams s -> false
    | Aast.Happly ((_, id), _) when List.mem ~equal:( = ) tparams id -> false
    | Aast.Herr
    | Aast.Hany ->
      failwith "Expected error on Tany in naming: param_hint_to_type_info"
    (* The following are based on Happly conversions in naming *)
    | Aast.Harray (Some _, Some _) -> false
    | Aast.Harray _ -> true
    | Aast.Hdarray _ -> false
    | Aast.Hvarray _ -> true
    | Aast.Hvarray_or_darray _ -> true
    | Aast.Hprim _ -> true
    | Aast.Hthis -> true
    | _ -> true
  in
  let tc = hint_to_type_constraint ~kind ~tparams ~skipawaitable h in
  let tc_name = TC.name tc in
  if is_simple_hint then
    let tc_flags = try_add_nullable ~nullable h [] in
    make_type_info ~tparams h tc_name tc_flags
  else
    let tc_flags = TC.flags tc in
    let tc_flags = try_add_nullable ~nullable h tc_flags in
    make_type_info ~tparams h tc_name tc_flags

let hint_to_type_info ~kind ~skipawaitable ~nullable ~tparams h =
  match kind with
  | Param -> param_hint_to_type_info ~kind ~skipawaitable ~nullable ~tparams h
  | _ ->
    let tc = hint_to_type_constraint ~kind ~tparams ~skipawaitable h in
    let tc_name = TC.name tc in
    let tc_flags = TC.flags tc in
    let tc_flags =
      if (kind = Return || kind = Property) && tc_name <> None then
        List.stable_dedup (TC.ExtendedHint :: tc_flags)
      else
        tc_flags
    in
    let tc_flags =
      if kind = TypeDef then
        add_nullable ~nullable tc_flags
      else
        try_add_nullable ~nullable h tc_flags
    in
    let tc_flags =
      if kind = UpperBound then
        List.stable_dedup (TC.UpperBound :: tc_flags)
      else
        tc_flags
    in
    make_type_info ~tparams h tc_name tc_flags

let hint_to_class (h : Aast.hint) =
  match h with
  | (_, Aast.Happly ((_, name), _)) -> Hhbc_id.Class.from_ast_name name
  | _ -> Hhbc_id.Class.from_raw_string "__type_is_not_class__"

let emit_type_constraint_for_native_function tparams ret ti =
  let user_type = Hhas_type_info.user_type ti in
  let (name, flags) =
    match (user_type, ret) with
    | (_, None)
    | (None, _) ->
      (Some "HH\\void", [TC.ExtendedHint])
    | (Some t, _) when t = "HH\\mixed" || t = "callable" -> (None, [])
    | (Some t, Some ret) ->
      let strip_nullable n = String_utils.lstrip n "?" in
      let strip_soft n = String_utils.lstrip n "@" in
      let vanilla_name n = Hhbc_string_utils.strip_type_list n in
      let name =
        (* Strip twice since we don't know which one is coming first *)
        Some (vanilla_name @@ strip_nullable @@ strip_soft @@ strip_nullable t)
      in
      let flags = [TC.ExtendedHint] in
      let rec get_flags (_, t) flags =
        match t with
        | Aast.Hoption x ->
          TC.Nullable :: TC.DisplayNullable :: get_flags x flags
        | Aast.Hsoft x -> TC.Soft :: get_flags x flags
        | Aast.Haccess _ -> TC.TypeConstant :: flags
        | Aast.Happly ((_, name), _) when List.mem ~equal:( = ) tparams name ->
          TC.TypeVar :: flags
        | _ -> flags
      in
      let flags = get_flags ret flags in
      (name, flags)
  in
  let tc = Hhas_type_constraint.make name flags in
  Hhas_type_info.make user_type tc
