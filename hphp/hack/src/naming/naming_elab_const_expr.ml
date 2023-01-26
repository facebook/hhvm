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

let rec const_expr_err in_mode acc (_, pos, expr_) =
  match expr_ with
  | Aast.(Id _ | Null | True | False | Int _ | Float _ | String _) -> acc
  | Aast.(Class_const ((_, _, (CIparent | CIself | CI _)), _)) -> acc
  | Aast.(Class_const ((_, _, Aast.CIexpr (_, _, (This | Id _))), _)) -> acc
  | Aast.(Smethod_id _ | Fun_id _) -> acc
  | Aast.(FunctionPointer ((FP_id _ | FP_class_const _), _)) -> acc
  | Aast.Upcast (e, _) -> const_expr_err in_mode acc e
  | Aast.(As (e, (_, Hlike _), _)) -> const_expr_err in_mode acc e
  | Aast.(As (e, (_, Happly ((p, cn), [_])), _)) ->
    if String.equal cn SN.FB.cIncorrectType then
      const_expr_err in_mode acc e
    else
      (Err.naming @@ Naming_error.Illegal_constant p) :: acc
  | Aast.Unop
      ((Ast_defs.Uplus | Ast_defs.Uminus | Ast_defs.Utild | Ast_defs.Unot), e)
    ->
    const_expr_err in_mode acc e
  | Aast.Binop (op, e1, e2) -> begin
    (* Only assignment is invalid *)
    match op with
    | Ast_defs.Eq _ -> (Err.naming @@ Naming_error.Illegal_constant pos) :: acc
    | _ ->
      let acc = const_expr_err in_mode acc e1 in
      const_expr_err in_mode acc e2
  end
  | Aast.Eif (e1, e2_opt, e3) ->
    let acc = const_expr_err in_mode acc e1 in
    let acc = const_expr_err in_mode acc e3 in
    Option.value_map ~default:acc ~f:(const_expr_err in_mode acc) e2_opt
  | Aast.Darray (_, kvs)
  | Aast.(KeyValCollection ((_, Dict), _, kvs)) ->
    List.fold_left kvs ~init:acc ~f:(fun acc (ek, ev) ->
        let acc = const_expr_err in_mode acc ek in
        const_expr_err in_mode acc ev)
  | Aast.Varray (_, exprs)
  | Aast.(ValCollection ((_, (Vec | Keyset)), _, exprs))
  | Aast.Tuple exprs ->
    List.fold_left exprs ~init:acc ~f:(fun acc e ->
        const_expr_err in_mode acc e)
  | Aast.Shape flds ->
    List.fold_left flds ~init:acc ~f:(fun acc (_, e) ->
        const_expr_err in_mode acc e)
  | Aast.Call ((_, _, Aast.Id (_, cn)), _, fn_params, _)
    when String.equal cn SN.StdlibFunctions.array_mark_legacy
         || String.equal cn SN.PseudoFunctions.unsafe_cast
         || String.equal cn SN.PseudoFunctions.unsafe_nonnull_cast ->
    List.fold_left fn_params ~init:acc ~f:(fun acc (_, e) ->
        const_expr_err in_mode acc e)
  | Aast.Omitted when FileInfo.is_hhi in_mode ->
    (* Only allowed in HHI positions where we don't care about the value *)
    acc
  | _ -> (Err.naming @@ Naming_error.Illegal_constant pos) :: acc

let const_expr in_mode in_enum_class expr =
  if in_enum_class then
    Ok expr
  else
    match const_expr_err in_mode [] expr with
    | [] -> Ok expr
    | errs -> Error (Err.invalid_expr expr, errs)

let on_class_ :
      'a 'b.
      Naming_phase_env.t * ('a, 'b) Aast_defs.class_ ->
      (Naming_phase_env.t * ('a, 'b) Aast_defs.class_, _) result =
 fun (env, c) ->
  let in_enum_class =
    match c.Aast.c_kind with
    | Ast_defs.Cenum_class _ -> true
    | Ast_defs.(Cclass _ | Cinterface | Cenum | Ctrait) -> false
  in
  let env =
    Env.set_in_enum_class ~in_enum_class
    @@ Env.set_mode ~in_mode:c.Aast.c_mode env
  in
  Ok (env, c)

let on_gconst :
      'a 'b.
      Naming_phase_env.t * ('a, 'b) Aast_defs.gconst ->
      (Naming_phase_env.t * ('a, 'b) Aast_defs.gconst, _) result =
 fun (env, cst) ->
  let env = Env.set_mode env ~in_mode:cst.Aast.cst_mode in
  Ok (env, cst)

let on_typedef :
      'a 'b.
      Naming_phase_env.t * ('a, 'b) Aast_defs.typedef ->
      (Naming_phase_env.t * ('a, 'b) Aast_defs.typedef, _) result =
 (fun (env, t) -> Ok (Env.set_mode env ~in_mode:t.Aast.t_mode, t))

let on_fun_def :
      'a 'b.
      Naming_phase_env.t * ('a, 'b) Aast_defs.fun_def ->
      (Naming_phase_env.t * ('a, 'b) Aast_defs.fun_def, _) result =
 (fun (env, fd) -> Ok (Env.set_mode env ~in_mode:fd.Aast.fd_mode, fd))

let on_module_def :
      'a 'b.
      Naming_phase_env.t * ('a, 'b) Aast_defs.module_def ->
      (Naming_phase_env.t * ('a, 'b) Aast_defs.module_def, _) result =
 (fun (env, md) -> Ok (Env.set_mode env ~in_mode:md.Aast.md_mode, md))

let on_class_const_kind on_error =
  let handler
        : 'a 'b.
          Naming_phase_env.t * ('a, 'b) Aast_defs.class_const_kind ->
          ( Naming_phase_env.t * ('a, 'b) Aast_defs.class_const_kind,
            Naming_phase_env.t * ('a, 'b) Aast_defs.class_const_kind )
          result =
   fun (env, kind) ->
    let in_mode = Env.in_mode env and in_enum_class = Env.in_enum_class env in
    match kind with
    | Aast.CCConcrete expr -> begin
      match const_expr in_mode in_enum_class expr with
      | Ok expr -> Ok (env, Aast.CCConcrete expr)
      | Error (expr, errs) ->
        List.iter ~f:on_error errs;
        Error (env, Aast.CCConcrete expr)
    end
    | Aast.CCAbstract _ -> Ok (env, kind)
  in
  handler

let on_gconst_cst_value on_error =
  let handler
        : 'a 'b.
          Naming_phase_env.t * ('a, 'b) Aast.expr ->
          ( Naming_phase_env.t * ('a, 'b) Aast.expr,
            Naming_phase_env.t * ('a, 'b) Aast.expr )
          result =
   fun (env, cst_value) ->
    let in_mode = Env.in_mode env and in_enum_class = Env.in_enum_class env in
    match const_expr in_mode in_enum_class cst_value with
    | Ok expr -> Ok (env, expr)
    | Error (expr, errs) ->
      List.iter ~f:on_error errs;
      Error (env, expr)
  in
  handler

let top_down_pass =
  Naming_phase_pass.(
    top_down
      Ast_transform.
        {
          identity with
          on_class_ = Some on_class_;
          on_gconst = Some on_gconst;
          on_typedef = Some on_typedef;
          on_fun_def = Some on_fun_def;
          on_module_def = Some on_module_def;
        })

let bottom_up_pass on_error =
  Naming_phase_pass.(
    bottom_up
      Ast_transform.
        {
          identity with
          on_class_const_kind = Some (on_class_const_kind on_error);
          on_gconst_cst_value = Some (on_gconst_cst_value on_error);
        })
