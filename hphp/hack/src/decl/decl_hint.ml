(*
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

(*****************************************************************************)
(* Converts a type hint into a type  *)
(*****************************************************************************)
open Core_kernel
open Aast
open Typing_defs
module Partial = Partial_provider

(* Unpacking a hint for typing *)
let rec hint env (p, h) =
  let h = hint_ p env h in
  (Typing_reason.Rhint p, h)

and shape_field_info_to_shape_field_type env { sfi_optional; sfi_hint; _ } =
  { sft_optional = sfi_optional; sft_ty = hint env sfi_hint }

and aast_tparam_to_decl_tparam env t =
  {
    tp_variance = t.Aast.tp_variance;
    tp_name = t.Aast.tp_name;
    tp_constraints =
      List.map ~f:(Tuple.T2.map_snd ~f:(hint env)) t.Aast.tp_constraints;
    tp_reified = t.Aast.tp_reified;
    tp_user_attributes = t.Aast.tp_user_attributes;
  }

and hint_ p env = function
  | Hany -> Typing_defs.make_tany ()
  | Herr -> Terr
  | Hmixed -> Tmixed
  | Hnonnull -> Tnonnull
  | Hthis -> Tthis
  | Hdynamic -> Tdynamic
  | Hnothing -> Tnothing
  | Harray (h1, h2) ->
    if Partial.should_check_error env.Decl_env.mode 4045 && h1 = None then
      Errors.generic_array_strict p;
    let h1 = Option.map h1 (hint env) in
    let h2 = Option.map h2 (hint env) in
    Tarray (h1, h2)
  | Hdarray (h1, h2) -> Tdarray (hint env h1, hint env h2)
  | Hvarray h -> Tvarray (hint env h)
  | Hvarray_or_darray h -> Tvarray_or_darray (hint env h)
  | Hprim p -> Tprim p
  | Habstr x -> Tgeneric x
  | Hoption (_, Hprim Tnull) ->
    Errors.option_null p;
    Terr
  | Hoption (_, Hprim Tvoid) ->
    Errors.option_return_only_typehint p `void;
    Terr
  | Hoption (_, Hprim Tnoreturn) ->
    Errors.option_return_only_typehint p `noreturn;
    Terr
  | Hoption (_, Hmixed) ->
    Errors.option_mixed p;
    Terr
  | Hoption h ->
    let h = hint env h in
    Toption h
  | Hlike h -> Tlike (hint env h)
  | Hfun
      {
        reactive_kind = reactivity;
        is_coroutine;
        param_tys = hl;
        param_kinds = kl;
        param_mutability = muts;
        variadic_ty = vh;
        return_ty = h;
        is_mutable_return = mut_ret;
      } ->
    let make_param ((p, _) as x) k mut =
      let fp_mutability =
        match mut with
        | Some PMutable -> Some Param_borrowed_mutable
        | Some POwnedMutable -> Some Param_owned_mutable
        | Some PMaybeMutable -> Some Param_maybe_mutable
        | _ -> None
      in
      {
        fp_pos = p;
        fp_name = None;
        fp_type = possibly_enforced_hint env x;
        fp_kind = get_param_mode ~is_ref:false k;
        fp_accept_disposable = false;
        fp_mutability;
        fp_rx_annotation = None;
      }
    in
    let paraml = List.map3_exn hl kl muts ~f:make_param in
    let ret = possibly_enforced_hint env h in
    let arity_min = List.length paraml in
    let arity =
      match vh with
      | Some t -> Fvariadic (arity_min, make_param t None None)
      | None -> Fstandard (arity_min, arity_min)
    in
    let reactivity =
      match reactivity with
      | FReactive -> Reactive None
      | FShallow -> Shallow None
      | FLocal -> Local None
      | FNonreactive -> Nonreactive
    in
    Tfun
      {
        ft_pos = p;
        ft_deprecated = None;
        ft_is_coroutine = is_coroutine;
        ft_arity = arity;
        ft_tparams = ([], FTKtparams);
        ft_where_constraints = [];
        ft_params = paraml;
        ft_ret = ret;
        ft_fun_kind = Ast_defs.FSync;
        ft_reactive = reactivity;
        ft_return_disposable = false;
        ft_mutability = None;
        ft_returns_mutable = mut_ret;
        ft_decl_errors = None;
        ft_returns_void_to_rx = false;
      }
  | Happly ((p, "\\Tuple"), _)
  | Happly ((p, "\\tuple"), _) ->
    Errors.tuple_syntax p;
    Terr
  | Happly (((_p, c) as id), argl) ->
    Decl_env.add_wclass env c;
    let argl = List.map argl (hint env) in
    Tapply (id, argl)
  | Haccess (root_ty, ids) ->
    let root_ty = hint env root_ty in
    Taccess (root_ty, ids)
  | Htuple hl ->
    let tyl = List.map hl (hint env) in
    Ttuple tyl
  | Hshape { nsi_allows_unknown_fields; nsi_field_map } ->
    let shape_kind =
      if nsi_allows_unknown_fields then
        Open_shape
      else
        Closed_shape
    in
    let fdm =
      List.fold_left
        ~f:(fun acc i ->
          ShapeMap.add
            i.sfi_name
            (shape_field_info_to_shape_field_type env i)
            acc)
        ~init:ShapeMap.empty
        nsi_field_map
    in
    Tshape (shape_kind, fdm)
  | Hsoft (p, h_) -> hint_ p env h_
  | Hpu_access (base, sid) -> Tpu_access (hint env base, sid)

and possibly_enforced_hint env h =
  (* Initially we assume that a type is not enforced at runtime.
   * We refine this during localization
   *)
  { et_enforced = false; et_type = hint env h }
