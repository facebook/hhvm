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

let is_dynamic = function
  | Aast.(
      ( CIparent | CIself | CIstatic | CI _
      | CIexpr (_, _, (Lvar _ | This | Dollardollar _)) )) ->
    false
  | _ -> true

let on_expr (env, (ex, pos, expr_), err_acc) =
  let res =
    let open Aast in
    match expr_ with
    | Aast.(New ((_, ci_pos, ci), targs, exprs, expr_opt, ann))
      when is_dynamic ci ->
      let err = Err.naming @@ Naming_error.Dynamic_new_in_strict_mode ci_pos in
      let class_id = ((), pos, Aast.CI (ci_pos, SN.Classes.cUnknown)) in
      let expr_ = Aast.New (class_id, targs, exprs, expr_opt, ann) in
      Ok (expr_, Some err)
    (* TODO[mjt] can we decompose this case further? This is considering
       both the class_id and the class_get_expr *)
    | Class_get ((_, _, ci), CGstring _, _prop_or_meth)
      when not @@ is_dynamic ci ->
      Ok (expr_, None)
    | Class_get ((_, _, ci), CGexpr (_, cg_expr_pos, _), Ast_defs.Is_method)
      when not @@ is_dynamic ci ->
      let err = Err.naming @@ Naming_error.Dynamic_method_access cg_expr_pos in
      Ok (expr_, Some err)
    | Class_get
        ( (_, _, CIexpr (_, ci_pos, _)),
          (CGstring _ | CGexpr (_, _, (Lvar _ | This))),
          _prop_or_meth ) ->
      let err =
        Err.naming @@ Naming_error.Dynamic_class_name_in_strict_mode ci_pos
      in
      Error (Some err)
    | Class_get
        ((_, _, CIexpr (_, ci_pos, _)), CGexpr (_, cg_pos, _), _prop_or_meth) ->
      let err1 =
        Err.naming @@ Naming_error.Dynamic_class_name_in_strict_mode ci_pos
      in
      (* TODO[mjt] this seems like the wrong error? Shouldn't this be
         `Dynamic_method_access` as in the case above? *)
      let err2 =
        Err.naming @@ Naming_error.Dynamic_class_name_in_strict_mode cg_pos
      in
      Error (Some (Err.Free_monoid.plus err1 err2))
    | Class_get (_, Aast.CGexpr (_, cg_pos, _), _prop_or_meth) ->
      let err =
        Err.naming @@ Naming_error.Dynamic_class_name_in_strict_mode cg_pos
      in
      Error (Some err)
    | Aast.(FunctionPointer (FP_class_const ((_, _, ci), _), _))
      when is_dynamic ci ->
      (* TODO[mjt] report error in strict mode *)
      Error None
    | Aast.Class_const ((_, _, ci), _) when is_dynamic ci ->
      (* TODO[mjt] report error in strict mode *)
      Error None
    | _ -> Ok (expr_, None)
  in
  match res with
  | Error err_opt ->
    let err =
      Option.value_map
        err_opt
        ~default:err_acc
        ~f:(Err.Free_monoid.plus err_acc)
    in
    Naming_phase_pass.Cont.finish (env, (ex, pos, Err.invalid_expr_ pos), err)
  | Ok (expr_, err_opt) ->
    let err =
      Option.value_map
        err_opt
        ~default:err_acc
        ~f:(Err.Free_monoid.plus err_acc)
    in
    Naming_phase_pass.Cont.next (env, (ex, pos, expr_), err)

let pass =
  Naming_phase_pass.(bottom_up { identity with on_expr = Some on_expr })
