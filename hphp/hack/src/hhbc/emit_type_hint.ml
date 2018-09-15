(**
 * Copyright (c) 2017, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
*)

open Core_kernel
open Hhbc_string_utils

module A = Ast
module HOpt = Hhbc_options
module SN = Naming_special_names
module TC = Hhas_type_constraint

type type_hint_kind =
| Property
| Return
| Param
| TypeDef

let fmt_name_or_prim ~tparams ~namespace x =
  let name = snd x in
  if List.mem ~equal:(=) tparams name || is_self name || is_parent name
  then name
  else
    let needs_unmangling = Xhp.is_xhp (strip_ns name) in
    let fq_id, _ = Hhbc_id.Class.elaborate_id namespace x in
    if needs_unmangling then Hhbc_id.Class.to_unmangled_string fq_id
    else Hhbc_id.Class.to_raw_string fq_id

(* Produce the "userType" bit of the annotation *)
let rec fmt_hint ~tparams ~namespace ?(strip_tparams=false) (_, h) =
  match h with
  | A.Happly (id, []) ->
    fmt_name_or_prim ~tparams ~namespace id

  | A.Happly (id, args) ->
    let name = fmt_name_or_prim ~tparams ~namespace id in
    if strip_tparams then name
    else name ^ "<" ^ fmt_hints ~tparams ~namespace args ^ ">"

  | A.Hfun (true, _, _, _, _) ->
    failwith "Codegen for coroutine functions is not supported"

  | A.Hfun (false, args, _kinds, _, ret) ->
    (* TODO(mqian): Implement for inout parameters *)
    "(function (" ^ fmt_hints ~tparams ~namespace args
    ^ "): " ^ fmt_hint ~tparams ~namespace ret ^ ")"

  | A.Htuple hs ->
    "(" ^ fmt_hints ~tparams ~namespace hs ^ ")"

  | A.Haccess (h1, h2, accesses) ->
    fmt_name_or_prim ~tparams ~namespace h1 ^ "::" ^
      String.concat ~sep:"::" (List.map (h2::accesses) snd)

  (* Follow HHVM order: soft -> option *)
  | A.Hoption (_, A.Hsoft t) -> "@?" ^ fmt_hint ~tparams ~namespace t

  | A.Hoption t -> "?" ^ fmt_hint ~tparams ~namespace t

  | A.Hsoft h -> "@" ^ fmt_hint ~tparams ~namespace h

  | A.Hshape { A.si_shape_field_list; _ } ->
    let fmt_field = function
      | A.SFlit_int (_, s_i) -> s_i
      | A.SFlit_str (_, s) -> "'" ^ s ^ "'"
      | A.SFclass_const (cid, (_, s2)) ->
        fmt_name_or_prim ~tparams ~namespace cid ^ "::" ^ s2
    in
    let format_shape_field ({ A.sf_name; A.sf_hint; A.sf_optional }) =
      let prefix = if sf_optional then "?" else "" in
      prefix ^ fmt_field sf_name ^ "=>" ^ fmt_hint ~tparams ~namespace sf_hint in
    let shape_fields =
      List.map ~f:format_shape_field si_shape_field_list in
    prefix_namespace "HH" "shape(" ^
      String.concat ~sep:", " shape_fields ^ ")"

and fmt_hints ~tparams ~namespace hints =
  String.concat ~sep:", " (List.map hints (fmt_hint ~tparams ~namespace))

let can_be_nullable h =
  not (Emit_env.is_hh_syntax_enabled ())
  ||
  match snd h with
  | A.Hfun (_, _, _, _, _)
  | A.Hoption (_, A.Hfun (_, _, _, _, _))
  | A.Happly ((_, "dynamic"), _)
  | A.Happly ((_, "nonnull"), _)
  | A.Happly ((_, "mixed"), _)
  | A.Hoption (_, A.Happly ((_, "dynamic"), _))
  | A.Hoption (_, A.Happly ((_, "nonnull"), _))
  | A.Hoption (_, A.Happly ((_, "mixed"), _))
  (* HHVM does not emit nullable for type consts that are set to null by default
   * function(Class::Type $a = null) unless it is explicitly marked as nullable
   *)
  | A.Haccess (_, _, _) -> false
  | _ -> true

let rec hint_to_type_constraint
  ~kind ~tparams ~skipawaitable ~namespace (_, h) =
  match h with
  (* The dynamic and nonnull types are treated by the runtime as mixed *)
  | A.Happly ((_, "dynamic"), [])
  | A.Happly ((_, "mixed"), []) ->
    if Emit_env.is_hh_syntax_enabled ()
    then TC.make None []
    else TC.make (Some "mixed") []

  | A.Happly ((_, s), []) when String.lowercase s = "void" && kind <> TypeDef ->
    if Emit_env.is_hh_syntax_enabled ()
    || Hhbc_options.php7_scalar_types !Hhbc_options.compiler_options
    then TC.make None []
    else TC.make (Some "void") [TC.HHType; TC.ExtendedHint]

  | A.Hfun _ ->
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
    hint_to_type_constraint ~kind ~tparams ~skipawaitable:false ~namespace h

  | A.Hoption (_, A.Hsoft (_, A.Happly ((_, "Awaitable"), [h])))
    when skipawaitable ->
    make_tc_with_flags_if_non_empty_flags ~kind ~tparams ~skipawaitable ~namespace
      h [TC.Soft; TC.HHType; TC.ExtendedHint]

  | A.Happly ((_, "Awaitable"), [])
  | A.Hoption (_, A.Happly ((_, "Awaitable"), []))
    when skipawaitable ->
    TC.make None []

  (* Need to differentiate between type params and classes *)
  | A.Happly ((pos,name) as id, _) ->
    if List.mem ~equal:(=) tparams name then
      let tc_name = Some "" in
      let tc_flags = [TC.HHType; TC.ExtendedHint; TC.TypeVar] in
      TC.make tc_name tc_flags
    else
      if kind = TypeDef && (is_self name || is_parent name)
      then Emit_fatal.raise_fatal_runtime pos
        (Printf.sprintf "Cannot access %s when no class scope is active" name)
      else
      let tc_name =
        if is_self name || is_parent name
        then name
        else
          let fq_id, _ = Hhbc_id.Class.elaborate_id namespace id in
          Hhbc_id.Class.to_raw_string fq_id
      in
      let tc_flags = [TC.HHType] in
      TC.make (Some tc_name) tc_flags

  (* Shapes and tuples are just arrays *)
  | A.Hshape _ ->
    let tc_name = Some "HH\\darray" in
    let tc_flags = [TC.HHType; TC.ExtendedHint] in
    TC.make tc_name tc_flags

  | A.Htuple _ ->
    let tc_name = Some "HH\\varray" in
    let tc_flags = [TC.HHType; TC.ExtendedHint] in
    TC.make tc_name tc_flags

  | A.Hoption t ->
    make_tc_with_flags_if_non_empty_flags ~kind ~tparams ~skipawaitable ~namespace
      t [TC.Nullable; TC.HHType; TC.ExtendedHint]

  | A.Hsoft t ->
    make_tc_with_flags_if_non_empty_flags ~kind ~tparams ~skipawaitable ~namespace
    t [TC.Soft; TC.HHType; TC.ExtendedHint]

and make_tc_with_flags_if_non_empty_flags
  ~kind ~tparams ~skipawaitable ~namespace t flags =
  let tc = hint_to_type_constraint ~kind ~tparams ~skipawaitable ~namespace t in
  let tc_name = TC.name tc in
  let tc_flags = TC.flags tc in
  match tc_name, tc_flags with
  | None, [] -> tc
  | _ ->
  let tc_flags = List.stable_dedup (flags @ tc_flags) in
  TC.make tc_name tc_flags

let add_nullable ~nullable flags =
  if nullable then List.stable_dedup (TC.Nullable :: flags) else flags

let try_add_nullable ~nullable h flags =
  add_nullable ~nullable:(nullable && can_be_nullable h) flags

let make_type_info ~tparams ~namespace h tc_name tc_flags =
  let type_info_user_type = Some (fmt_hint ~tparams ~namespace h) in
  let type_info_type_constraint = TC.make tc_name tc_flags in
  Hhas_type_info.make type_info_user_type type_info_type_constraint

let param_hint_to_type_info
  ~kind ~skipawaitable ~nullable ~tparams ~namespace h =
  let is_simple_hint =
    match snd h with
    | A.Hsoft _ | A.Hoption _ | A.Haccess _
    | A.Hfun _
    | A.Happly (_, _::_)
    | A.Happly ((_, "dynamic"), [])
    | A.Happly ((_, "nonnull"), [])
    | A.Happly ((_, "mixed"), []) -> false
    | A.Happly ((_, id), _) when List.mem ~equal:(=) tparams id -> false
    | _ -> true
  in
  let tc = hint_to_type_constraint ~kind ~tparams ~skipawaitable ~namespace h in
  let tc_name = TC.name tc in
  if is_simple_hint
  then
    let is_hh_type = Emit_env.is_hh_syntax_enabled () in
    let tc_flags = if is_hh_type then [TC.HHType] else [] in
    let tc_flags = try_add_nullable ~nullable h tc_flags in
    make_type_info ~tparams ~namespace h tc_name tc_flags
  else
    let tc_flags = TC.flags tc in
    let tc_flags = try_add_nullable ~nullable h tc_flags in
    make_type_info ~tparams ~namespace h tc_name tc_flags

let fail_if_contains_reserved_id hint namespace =
  (* Based on Parser::onTypeAnnotation,
  for type hints like foo\bar, check that bar isn't a reserved scalar type
  like int or string *)
  let must_check = HOpt.php7_scalar_types !HOpt.compiler_options in
  if must_check then
    let is_reserved id =
      let fully_qualified_id =
        Hhbc_id.Class.elaborate_id namespace id
        |> fst |> Hhbc_id.Class.to_unmangled_string in
      SN.Typehints.is_namespace_with_reserved_hh_name fully_qualified_id in
    let fail_if_reserved pos_id =
      let pos, id = pos_id in
      if is_reserved pos_id then Emit_fatal.raise_fatal_parse pos
        (Printf.sprintf "Cannot use '%s' as class name as it is reserved" id) in
    let checking_visitor = object(self) inherit [_] Ast.iter as super
      method! on_hint () hint =
        match snd hint with
        | A.Happly (id, hints) ->
          fail_if_reserved id;
          List.iter hints ~f:(self#on_hint ())
        | A.Haccess (id1, id2, ids) ->
          fail_if_reserved id1;
          fail_if_reserved id2;
          List.iter ids ~f:fail_if_reserved
        | _ -> super#on_hint () hint
    end in
    checking_visitor#on_hint () hint

let hint_to_type_info ~kind ~skipawaitable ~nullable ~tparams ~namespace h =
  fail_if_contains_reserved_id h namespace;
  match kind with
  | Param ->
    param_hint_to_type_info ~kind ~skipawaitable ~nullable ~tparams ~namespace h
  | _ ->
    let tc = hint_to_type_constraint ~kind ~tparams ~skipawaitable ~namespace h in
    let tc_name = TC.name tc in
    let tc_flags = TC.flags tc in
    let tc_flags =
      if (kind = Return || kind = Property) && tc_name <> None
      then List.stable_dedup (TC.ExtendedHint :: tc_flags)
      else tc_flags in
    let tc_flags =
      if kind = TypeDef then add_nullable ~nullable tc_flags
      else try_add_nullable ~nullable h tc_flags in
    make_type_info ~tparams ~namespace h tc_name tc_flags

let hint_to_class ~namespace h =
  fail_if_contains_reserved_id h namespace;
  match h with
  | (_, A.Happly (id, _)) ->
    let fq_id, _ = Hhbc_id.Class.elaborate_id namespace id in
    fq_id
  | _ -> Hhbc_id.Class.from_raw_string "__type_is_not_class__"

let emit_type_constraint_for_native_function tparams ret ti =
  let user_type = Hhas_type_info.user_type ti in
  let name, flags = match user_type, ret with
    | _ , None
    | None, _ -> Some "HH\\void", [TC.HHType; TC.ExtendedHint]
    | Some t, _ when t = "HH\\mixed" || t = "callable" -> None, []
    | Some t, Some ret ->
      let strip_nullable n = String_utils.lstrip n "?" in
      let strip_soft n = String_utils.lstrip n "@" in
      let vanilla_name n = Hhbc_string_utils.strip_type_list n in
      let name =
        (* Strip twice since we don't know which one is coming first *)
        Some (vanilla_name @@ strip_nullable @@ strip_soft @@ strip_nullable t)
      in
      let flags = [TC.HHType; TC.ExtendedHint] in
      let rec get_flags t flags = match snd t with
        | A.Hoption x -> TC.Nullable :: (get_flags x flags)
        | A.Hsoft x -> TC.Soft :: (get_flags x flags)
        | A.Haccess _ -> TC.TypeConstant :: flags
        | A.Happly ((_, name), _) when List.mem ~equal:(=) tparams name ->
          TC.TypeVar :: flags
        | _ -> flags
      in
      let flags = get_flags ret flags in
      name, flags
  in
  let tc = Hhas_type_constraint.make name flags in
  Hhas_type_info.make user_type tc
