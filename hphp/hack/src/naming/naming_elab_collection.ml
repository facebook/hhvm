(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)
open Hh_prelude
module Err = Naming_phase_error
module SN = Naming_special_names

let afield_value cname afield =
  match afield with
  | Aast.AFvalue e -> (e, None)
  | Aast.AFkvalue (((_, pos, _) as e), _) ->
    (e, Some (Err.naming @@ Naming_error.Unexpected_arrow { pos; cname }))

let afield_key_value cname afield =
  match afield with
  | Aast.AFkvalue (ek, ev) -> ((ek, ev), None)
  | Aast.AFvalue ((annot, pos, _) as ek) ->
    let ev =
      ( annot,
        pos,
        Aast.Lvar (pos, Local_id.make_unscoped "__internal_placeholder") )
    in
    ((ek, ev), Some (Err.naming @@ Naming_error.Missing_arrow { pos; cname }))

let on_expr :
      'a 'b.
      _ * ('a * Pos.t * ('a, 'b) Aast_defs.expr_) * Err.t list ->
      ( _ * ('a * Pos.t * ('a, 'b) Aast_defs.expr_) * Err.t list,
        _ * ('a, 'b) Aast.expr * Err.t list )
      result =
 fun (env, ((annot, pos, expr_) as expr), err_acc) ->
  let res =
    match expr_ with
    | Aast.Collection ((pos, cname), c_targ_opt, afields)
      when Nast.is_vc_kind cname ->
      let (targ_opt, targ_err_opt) =
        match c_targ_opt with
        | Some (Aast.CollectionTV tv) -> (Some tv, None)
        | Some (Aast.CollectionTKV _) ->
          (None, Some (Err.naming @@ Naming_error.Too_many_arguments pos))
        | _ -> (None, None)
      in
      let (exprs, fields_err_opts) =
        List.unzip @@ List.map ~f:(afield_value cname) afields
      in
      let vc_kind = Nast.get_vc_kind cname in

      let err = List.filter_map ~f:Fn.id @@ (targ_err_opt :: fields_err_opts) in
      Ok (Aast.ValCollection ((pos, vc_kind), targ_opt, exprs), err)
    | Aast.Collection ((pos, cname), c_targ_opt, afields)
      when Nast.is_kvc_kind cname ->
      let (targs_opt, targ_err_opt) =
        match c_targ_opt with
        | Some (Aast.CollectionTKV (tk, tv)) -> (Some (tk, tv), None)
        | Some (Aast.CollectionTV _) ->
          (None, Some (Err.naming @@ Naming_error.Too_few_arguments pos))
        | _ -> (None, None)
      in
      let (fields, fields_err_opts) =
        List.unzip @@ List.map ~f:(afield_key_value cname) afields
      in
      let kvc_kind = Nast.get_kvc_kind cname in
      let err = List.filter_map ~f:Fn.id @@ (targ_err_opt :: fields_err_opts) in
      Ok (Aast.KeyValCollection ((pos, kvc_kind), targs_opt, fields), err)
    | Aast.Collection ((pos, cname), _, [])
      when String.equal SN.Collections.cPair cname ->
      Error (pos, [Err.naming @@ Naming_error.Too_few_arguments pos])
    | Aast.Collection ((pos, cname), c_targ_opt, [fst; snd])
      when String.equal SN.Collections.cPair cname ->
      let (targs_opt, targ_err_opt) =
        match c_targ_opt with
        | Some (Aast.CollectionTKV (tk, tv)) -> (Some (tk, tv), None)
        | Some (Aast.CollectionTV _) ->
          (None, Some (Err.naming @@ Naming_error.Too_few_arguments pos))
        | _ -> (None, None)
      in
      let (fst, fst_err_opt) = afield_value SN.Collections.cPair fst
      and (snd, snd_err_opt) = afield_value SN.Collections.cPair snd in
      let err =
        List.filter_map ~f:Fn.id [targ_err_opt; fst_err_opt; snd_err_opt]
      in
      Ok (Aast.Pair (targs_opt, fst, snd), err)
    | Aast.Collection ((pos, cname), _, _)
      when String.equal SN.Collections.cPair cname ->
      Error (pos, [Err.naming @@ Naming_error.Too_many_arguments pos])
    | Aast.Collection ((pos, cname), _, _) ->
      Error
        (pos, [Err.naming @@ Naming_error.Expected_collection { pos; cname }])
    | _ -> Ok (expr_, [])
  in
  match res with
  | Ok (expr_, errs) -> Ok (env, (annot, pos, expr_), errs @ err_acc)
  | Error (_pos, errs) -> Error (env, Err.invalid_expr expr, errs @ err_acc)

let pass =
  Naming_phase_pass.(
    top_down Ast_transform.{ identity with on_expr = Some on_expr })
