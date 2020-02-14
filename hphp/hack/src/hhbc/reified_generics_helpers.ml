(*
 * Copyright (c) 2017, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)
open Core_kernel
open Instruction_sequence
module A = Ast_defs
module T = Aast
module SN = Naming_special_names

type type_constraint =
  | DefinitelyReified (* There is a reified generic *)
  | MaybeReified (* There is no function or class reified generics,
                  * but there may be an inferred one *)
  | NotReified
  | NoConstraint

let get_erased_tparams env =
  Ast_scope.Scope.get_tparams (Emit_env.get_scope env)
  |> List.filter_map ~f:(fun tp ->
         if tp.T.tp_reified <> Aast.Reified then
           Some (snd tp.T.tp_name)
         else
           None)

let rec has_reified_type_constraint env h =
  let is_all_erased hl =
    let erased_tparams = get_erased_tparams env in
    List.for_all hl ~f:(function
        | (_, Aast.Happly ((_, id), [])) ->
          List.mem ~equal:String.equal erased_tparams id
        | _ -> false)
  in
  let combine v1 v2 =
    match (v1, v2) with
    | (DefinitelyReified, _)
    | (_, DefinitelyReified) ->
      DefinitelyReified
    | (MaybeReified, _)
    | (_, MaybeReified) ->
      MaybeReified
    | _ -> NotReified
  in
  match snd h with
  | Aast.Happly ((_, id), hl) ->
    if
      None <> Emit_expression.is_reified_tparam ~is_fun:true env id
      || None <> Emit_expression.is_reified_tparam ~is_fun:false env id
    then
      DefinitelyReified
    else if List.is_empty hl || is_all_erased hl then
      NotReified
    else
      List.fold_right hl ~init:MaybeReified ~f:(fun h v ->
          combine v @@ has_reified_type_constraint env h)
  | Aast.Hsoft h
  | Aast.Hlike h
  | Aast.Hoption h ->
    has_reified_type_constraint env h
  | Aast.Hprim _
  | Aast.Hmixed
  | Aast.Hnonnull
  | Aast.Harray _
  | Aast.Hdarray _
  | Aast.Hvarray _
  | Aast.Hvarray_or_darray _
  | Aast.Hthis
  | Aast.Hnothing
  | Aast.Hdynamic
  | Aast.Htuple _
  | Aast.Hunion _
  | Aast.Hintersection _
  | Aast.Hshape _
  | Aast.Hfun _
  | Aast.Haccess _
  | Aast.Hpu_access _ ->
    NotReified
  (* Not found in the original AST *)
  | Aast.Herr
  | Aast.Hany ->
    failwith "Should be a naming error"
  | Aast.Habstr _ -> failwith "TODO Unimplemented: Not in the original AST"

let rec remove_awaitable ((pos, h_) as h) =
  match h_ with
  | Aast.Happly ((_, id), [h]) when String.Caseless.(id = SN.Classes.cAwaitable)
    ->
    h
  (* For @Awaitable<T>, the soft type hint is moved to the inner type, i.e @T *)
  | Aast.Hsoft h -> (pos, Aast.Hsoft (remove_awaitable h))
  (* For ~Awaitable<T>, the like-type hint is moved to the inner type, i.e ~T *)
  | Aast.Hlike h -> (pos, Aast.Hlike (remove_awaitable h))
  (* For ?Awaitable<T>, the optional is dropped *)
  | Aast.Hoption h -> remove_awaitable h
  | Aast.Htuple _
  | Aast.Hunion _
  | Aast.Hintersection _
  | Aast.Hshape _
  | Aast.Hfun _
  | Aast.Haccess _
  | Aast.Happly _ ->
    h
  | Aast.Herr
  | Aast.Hany
  | Aast.Hmixed
  | Aast.Hnonnull
  | Aast.Habstr _
  | Aast.Harray _
  | Aast.Hdarray _
  | Aast.Hvarray _
  | Aast.Hvarray_or_darray _
  | Aast.Hprim _
  | Aast.Hthis
  | Aast.Hnothing
  | Aast.Hdynamic ->
    failwith "TODO Unimplemented Did not exist on legacy AST"
  | Aast.Hpu_access _ ->
    failwith "TODO(T36532263) awaitable in pocket universe type hint"

let convert_awaitable env h =
  if Ast_scope.Scope.is_in_async (Emit_env.get_scope env) then
    remove_awaitable h
  else
    h

let simplify_verify_type env pos check hint verify_instr =
  let get_ts hint =
    fst @@ Emit_expression.emit_reified_arg env pos ~isas:false hint
  in
  match hint with
  | (_, Aast.Hoption hint) ->
    let done_label = Label.next_regular () in
    gather
      [
        check;
        instr_jmpnz done_label;
        get_ts hint;
        verify_instr;
        instr_label done_label;
      ]
  | _ -> gather [get_ts hint; verify_instr]

let remove_erased_generics env h =
  let erased_tparams = get_erased_tparams env in
  let modify id =
    if List.mem ~equal:String.equal erased_tparams id then
      "_"
    else
      id
  in
  let rec aux (pos, h) =
    ( pos,
      match h with
      | Aast.Happly ((pos, id), hl) ->
        Aast.Happly ((pos, modify id), List.map ~f:aux hl)
      | Aast.Hsoft h -> Aast.Hsoft (aux h)
      | Aast.Hlike h -> Aast.Hlike (aux h)
      | Aast.Hoption h -> Aast.Hoption (aux h)
      | Aast.Htuple hl -> Aast.Htuple (List.map ~f:aux hl)
      | Aast.Hunion hl -> Aast.Hunion (List.map ~f:aux hl)
      | Aast.Hintersection hl -> Aast.Hintersection (List.map ~f:aux hl)
      | Aast.Hshape si ->
        let modify_sfi sfi =
          { sfi with Aast.sfi_hint = aux sfi.Aast.sfi_hint }
        in
        let fields = List.map ~f:modify_sfi si.Aast.nsi_field_map in
        Aast.Hshape
          {
            Aast.nsi_allows_unknown_fields = si.Aast.nsi_allows_unknown_fields;
            Aast.nsi_field_map = fields;
          }
      | Aast.Hfun _
      | Aast.Haccess _ ->
        h
      | Aast.Hpu_access (h, (pos, id), pu_loc) ->
        Aast.Hpu_access (aux h, (pos, modify id), pu_loc)
      | Aast.Herr
      | Aast.Hany
      | Aast.Hmixed
      | Aast.Hnonnull
      | Aast.Habstr _
      | Aast.Harray _
      | Aast.Hdarray _
      | Aast.Hvarray _
      | Aast.Hvarray_or_darray _
      | Aast.Hprim _
      | Aast.Hthis
      | Aast.Hnothing
      | Aast.Hdynamic ->
        failwith "TODO Unimplemented Did not exist on legacy AST" )
  in
  aux h
