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
      ('a, 'b) Aast_defs.class_ ->
      ctx:Naming_phase_env.t ->
      Naming_phase_env.t * (('a, 'b) Aast_defs.class_, _) result =
 fun c ~ctx ->
  let in_enum_class =
    match c.Aast.c_kind with
    | Ast_defs.Cenum_class _ -> true
    | Ast_defs.(Cclass _ | Cinterface | Cenum | Ctrait) -> false
  in
  let ctx =
    Env.set_in_enum_class ~in_enum_class
    @@ Env.set_mode ~in_mode:c.Aast.c_mode ctx
  in
  (ctx, Ok c)

let on_gconst :
      'a 'b.
      ('a, 'b) Aast_defs.gconst ->
      ctx:Naming_phase_env.t ->
      Naming_phase_env.t * (('a, 'b) Aast_defs.gconst, _) result =
 fun cst ~ctx ->
  let ctx = Env.set_mode ctx ~in_mode:cst.Aast.cst_mode in
  (ctx, Ok cst)

let on_typedef :
      'a 'b.
      ('a, 'b) Aast_defs.typedef ->
      ctx:Naming_phase_env.t ->
      Naming_phase_env.t * (('a, 'b) Aast_defs.typedef, _) result =
 (fun t ~ctx -> (Env.set_mode ctx ~in_mode:t.Aast.t_mode, Ok t))

let on_fun_def :
      'a 'b.
      ('a, 'b) Aast_defs.fun_def ->
      ctx:Naming_phase_env.t ->
      Naming_phase_env.t * (('a, 'b) Aast_defs.fun_def, _) result =
 (fun fd ~ctx -> (Env.set_mode ctx ~in_mode:fd.Aast.fd_mode, Ok fd))

let on_module_def :
      'a 'b.
      ('a, 'b) Aast_defs.module_def ->
      ctx:Naming_phase_env.t ->
      Naming_phase_env.t * (('a, 'b) Aast_defs.module_def, _) result =
 (fun md ~ctx -> (Env.set_mode ctx ~in_mode:md.Aast.md_mode, Ok md))

let on_class_const_kind on_error =
  let handler
        : 'a 'b.
          ('a, 'b) Aast_defs.class_const_kind ->
          ctx:Naming_phase_env.t ->
          Naming_phase_env.t
          * ( ('a, 'b) Aast_defs.class_const_kind,
              ('a, 'b) Aast_defs.class_const_kind )
            result =
   fun kind ~ctx ->
    let in_mode = Env.in_mode ctx and in_enum_class = Env.in_enum_class ctx in
    match kind with
    | Aast.CCConcrete expr -> begin
      match const_expr in_mode in_enum_class expr with
      | Ok expr -> (ctx, Ok (Aast.CCConcrete expr))
      | Error (expr, errs) ->
        List.iter ~f:on_error errs;
        (ctx, Error (Aast.CCConcrete expr))
    end
    | Aast.CCAbstract _ -> (ctx, Ok kind)
  in
  handler

let on_gconst_cst_value on_error cst_value ~ctx =
  let in_mode = Env.in_mode ctx and in_enum_class = Env.in_enum_class ctx in
  match const_expr in_mode in_enum_class cst_value with
  | Ok expr -> (ctx, Ok expr)
  | Error (expr, errs) ->
    List.iter ~f:on_error errs;
    (ctx, Error expr)

let top_down_pass =
  let id = Aast.Pass.identity () in
  Naming_phase_pass.top_down
    Aast.Pass.
      {
        id with
        on_ty_class_ = Some on_class_;
        on_ty_gconst = Some on_gconst;
        on_ty_typedef = Some on_typedef;
        on_ty_fun_def = Some on_fun_def;
        on_ty_module_def = Some on_module_def;
      }

let bottom_up_pass on_error =
  let id = Aast.Pass.identity () in
  Naming_phase_pass.bottom_up
    Aast.Pass.
      {
        id with
        on_ty_class_const_kind =
          Some (fun elem ~ctx -> on_class_const_kind on_error elem ~ctx);
        on_fld_gconst_cst_value =
          Some (fun elem ~ctx -> on_gconst_cst_value on_error elem ~ctx);
      }
