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

module Env = struct
  let in_mode
      Naming_phase_env.{ elab_const_expr = Elab_const_expr.{ in_mode; _ }; _ } =
    in_mode

  let in_enum_class
      Naming_phase_env.
        { elab_const_expr = Elab_const_expr.{ in_enum_class; _ }; _ } =
    in_enum_class

  let set_mode t ~in_mode =
    Naming_phase_env.
      {
        t with
        elab_const_expr = Elab_const_expr.{ t.elab_const_expr with in_mode };
      }

  let set_in_enum_class t ~in_enum_class =
    Naming_phase_env.
      {
        t with
        elab_const_expr =
          Elab_const_expr.{ t.elab_const_expr with in_enum_class };
      }
end

let and_ err1 err2 = Option.merge ~f:Err.Free_monoid.plus err1 err2

let rec const_expr_err in_mode (_, pos, expr_) =
  match expr_ with
  | Aast.(Id _ | Null | True | False | Int _ | Float _ | String _) -> None
  | Aast.(Class_const ((_, _, (CIparent | CIself | CI _)), _)) -> None
  | Aast.(Class_const ((_, _, Aast.CIexpr (_, _, (This | Id _))), _)) -> None
  | Aast.(Smethod_id _ | Fun_id _) -> None
  | Aast.(FunctionPointer ((FP_id _ | FP_class_const _), _)) -> None
  | Aast.Upcast (e, _) -> const_expr_err in_mode e
  | Aast.(As (e, (_, Hlike _), _)) -> const_expr_err in_mode e
  | Aast.(As (e, (_, Happly ((p, cn), [_])), _)) ->
    if String.equal cn SN.FB.cIncorrectType then
      const_expr_err in_mode e
    else
      Some (Err.naming @@ Naming_error.Illegal_constant p)
  | Aast.Unop
      ((Ast_defs.Uplus | Ast_defs.Uminus | Ast_defs.Utild | Ast_defs.Unot), e)
    ->
    const_expr_err in_mode e
  | Aast.Binop (op, e1, e2) ->
    (* Only assignment is invalid *)
    begin
      match op with
      | Ast_defs.Eq _ -> Some (Err.naming @@ Naming_error.Illegal_constant pos)
      | _ -> and_ (const_expr_err in_mode e1) (const_expr_err in_mode e2)
    end
  | Aast.Eif (e1, e2_opt, e3) ->
    and_
      (const_expr_err in_mode e1)
      (and_
         (Option.bind ~f:(const_expr_err in_mode) e2_opt)
         (const_expr_err in_mode e3))
  | Aast.Darray (_, kvs)
  | Aast.(KeyValCollection (Dict, _, kvs)) ->
    List.fold_left kvs ~init:None ~f:(fun acc (ek, ev) ->
        and_ acc @@ and_ (const_expr_err in_mode ek) (const_expr_err in_mode ev))
  | Aast.Varray (_, exprs)
  | Aast.(ValCollection ((Vec | Keyset), _, exprs))
  | Aast.Tuple exprs ->
    List.fold_left exprs ~init:None ~f:(fun acc e ->
        and_ acc @@ const_expr_err in_mode e)
  | Aast.Shape flds ->
    List.fold_left flds ~init:None ~f:(fun acc (_, e) ->
        and_ acc @@ const_expr_err in_mode e)
  | Aast.Call ((_, _, Aast.Id (_, cn)), _, fn_params, _)
    when String.equal cn SN.StdlibFunctions.array_mark_legacy
         || String.equal cn SN.PseudoFunctions.unsafe_cast
         || String.equal cn SN.PseudoFunctions.unsafe_nonnull_cast ->
    List.fold_left fn_params ~init:None ~f:(fun acc (_, e) ->
        and_ acc @@ const_expr_err in_mode e)
  | Aast.Omitted when FileInfo.is_hhi in_mode ->
    (* Only allowed in HHI positions where we don't care about the value *)
    None
  | _ -> Some (Err.naming @@ Naming_error.Illegal_constant pos)

let const_expr in_mode in_enum_class ((_, pos, _) as expr) =
  if in_enum_class then
    (expr, None)
  else
    match const_expr_err in_mode expr with
    | Some err -> (((), pos, Err.invalid_expr_ pos), Some err)
    | None -> (expr, None)

let on_class_ (env, c, err) =
  let in_enum_class =
    match c.Aast.c_kind with
    | Ast_defs.Cenum_class _ -> true
    | Ast_defs.(Cclass _ | Cinterface | Cenum | Ctrait) -> false
  in
  let env =
    Env.set_in_enum_class ~in_enum_class
    @@ Env.set_mode ~in_mode:c.Aast.c_mode env
  in
  Naming_phase_pass.Cont.next (env, c, err)

let on_gconst (env, cst, err) =
  let env = Env.set_mode env ~in_mode:cst.Aast.cst_mode in
  Naming_phase_pass.Cont.next (env, cst, err)

let on_typedef (env, t, err) =
  Naming_phase_pass.Cont.next (Env.set_mode env ~in_mode:t.Aast.t_mode, t, err)

let on_fun_def (env, fd, err) =
  Naming_phase_pass.Cont.next
    (Env.set_mode env ~in_mode:fd.Aast.fd_mode, fd, err)

let on_module_def (env, md, err) =
  Naming_phase_pass.Cont.next
    (Env.set_mode env ~in_mode:md.Aast.md_mode, md, err)

let on_class_const_kind (env, kind, err_acc) =
  let in_mode = Env.in_mode env and in_enum_class = Env.in_enum_class env in
  let (kind, err_opt) =
    match kind with
    | Aast.CCConcrete expr ->
      let (expr, err) = const_expr in_mode in_enum_class expr in
      (Aast.CCConcrete expr, err)
    | Aast.CCAbstract _ -> (kind, None)
  in
  let err =
    Option.value_map ~default:err_acc ~f:(Err.Free_monoid.plus err_acc) err_opt
  in
  Naming_phase_pass.Cont.next (env, kind, err)

let on_gconst_cst_value (env, cst_value, err_acc) =
  let in_mode = Env.in_mode env and in_enum_class = Env.in_enum_class env in
  let (cst_value, err_opt) = const_expr in_mode in_enum_class cst_value in
  let err =
    Option.value_map ~default:err_acc ~f:(Err.Free_monoid.plus err_acc) err_opt
  in
  Naming_phase_pass.Cont.next (env, cst_value, err)

let top_down_pass =
  Naming_phase_pass.(
    top_down
      {
        identity with
        on_class_ = Some on_class_;
        on_gconst = Some on_gconst;
        on_typedef = Some on_typedef;
        on_fun_def = Some on_fun_def;
        on_module_def = Some on_module_def;
      })

let bottom_up_pass =
  Naming_phase_pass.(
    bottom_up
      {
        identity with
        on_class_const_kind = Some on_class_const_kind;
        on_gconst_cst_value = Some on_gconst_cst_value;
      })
