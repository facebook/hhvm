(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Common
open Ast_defs
open Aast_defs

let rec can_be_captured = function
  | Shape _
  | ValCollection _
  | KeyValCollection _
  | Null
  | This
  | True
  | False
  | Omitted
  | Id _
  | Lvar _
  | Dollardollar _
  | Array_get _
  | Obj_get _
  | Class_get _
  | Class_const _
  | Nameof _
  | Call _
  | FunctionPointer _
  | Int _
  | Float _
  | String _
  | String2 _
  | PrefixedString _
  | Tuple _
  | List _
  | Xml _
  | Import _
  | Lplaceholder _
  | Method_caller _
  | EnumClassLabel _
  | Invalid None ->
    false
  | Yield _
  | Clone _
  | Await _
  | ReadonlyExpr _
  | Cast _
  | Unop _
  | Binop _
  | Assign _
  | Pipe _
  | Eif _
  | Is _
  | As _
  | Upcast _
  | New _
  | Efun _
  | Lfun _
  | Collection _
  | ExpressionTree _
  | Pair _
  | ET_Splice _
  | Package _ ->
    true
  | Invalid (Some (_, _, exp))
  | Hole ((_, _, exp), _, _, _) ->
    can_be_captured exp

let find_shape_field name fields =
  List.find
    ~f:(fun (field_name, _e) ->
      match field_name with
      | SFlit_str (_, n) -> String.equal name n
      | _ -> false)
    fields

let get_return_from_fun e =
  match e with
  | (_, _, Lfun ({ f_body = { fb_ast; _ }; _ }, _))
  | (_, _, Efun { ef_fun = { f_body = { fb_ast; _ }; _ }; _ }) ->
    List.find_map fb_ast ~f:(fun s ->
        match s with
        | (_, Return (Some e)) -> Some e
        | _ -> None)
  | _ -> None

let get_virtual_expr_from_et et =
  let get_body_helper e =
    match e with
    | (_, _, Call { args = _ :: Anormal (_, _, Shape fields) :: _; _ }) ->
      (match find_shape_field "type" fields with
      | Some (_, e) -> get_return_from_fun e
      | None -> None)
    | _ -> None
  in
  match et.et_runtime_expr with
  | (_, _, Await (_, _, Call { func = e; _ }))
  | (_, _, Call { func = e; _ }) ->
    (match get_return_from_fun e with
    | Some e -> get_body_helper e
    | None -> get_body_helper et.et_runtime_expr)
  | _ -> get_body_helper et.et_runtime_expr

let is_assign (s : ('a, 'b) stmt) =
  match s with
  | (_, Expr (_, _, Assign (_, None, _))) -> true
  | _ -> false

let get_splices_from_fun e =
  match e with
  | (_, _, Lfun ({ f_body = { fb_ast; _ }; _ }, _))
  | (_, _, Efun { ef_fun = { f_body = { fb_ast; _ }; _ }; _ }) ->
    List.filter fb_ast ~f:is_assign
  | _ -> []

let get_splices_from_et et =
  match et.et_runtime_expr with
  | (_, _, Await (_, _, Call { func = e; _ }))
  | (_, _, Call { func = e; _ }) ->
    get_splices_from_fun e
  | _ -> []

let get_virtual_expr expr =
  match expr with
  | (_, _, ExpressionTree et) ->
    (match get_virtual_expr_from_et et with
    | Some expr -> expr
    | None -> expr)
  | _ -> expr

let rec get_fun_expr expr =
  match expr with
  | (_, _, Lfun (f, _))
  | (_, _, Efun { ef_fun = f; _ }) ->
    Some f
  | (_, _, Hole (e, _, _, _)) -> get_fun_expr e
  | _ -> None

let rec is_const_expr (_, _, expr_) =
  match expr_ with
  | Null
  | True
  | False
  | This
  | Omitted
  | Id _
  | Lvar _
  | Dollardollar _
  | Class_const _
  | FunctionPointer _
  | Int _
  | Float _
  | String _
  | List _
  | Efun _
  | Lfun _
  | EnumClassLabel _
  | Lplaceholder _
  | Method_caller _
  | Package _
  | Nameof _ ->
    true
  | Call _
  | New _
  | Await _
  | Assign _
  | ExpressionTree _
  | ET_Splice _
  | Xml _ ->
    false
  | Clone expr
  | PrefixedString (_, expr)
  | ReadonlyExpr expr
  | Cast (_, expr)
  | Unop (_, expr)
  | Is (expr, _)
  | As { expr; hint = _; is_nullable = _; enforce_deep = _ }
  | Upcast (expr, _)
  | Hole (expr, _, _, _)
  | Import (_, expr) ->
    is_const_expr expr
  | Obj_get (e1, e2, _, _)
  | Pipe (_, e1, e2)
  | Binop { bop = _; lhs = e1; rhs = e2 }
  | Pair (_, e1, e2) ->
    is_const_expr e1 && is_const_expr e2
  | ValCollection (_, _, exprs)
  | String2 exprs
  | Tuple exprs ->
    List.for_all exprs ~f:is_const_expr
  | Shape shapes -> List.for_all ~f:(fun (_, e) -> is_const_expr e) shapes
  | KeyValCollection (_, _, exprs) ->
    List.for_all ~f:(fun (e1, e2) -> is_const_expr e1 && is_const_expr e2) exprs
  | Invalid e -> Option.fold ~none:true ~some:is_const_expr e
  | Array_get (e1, e2) ->
    is_const_expr e1 && Option.fold ~none:true ~some:is_const_expr e2
  | Class_get (_, cge, _) ->
    (match cge with
    | CGstring _ -> true
    | CGexpr e -> is_const_expr e)
  | Yield afield -> is_const_afield afield
  | Eif (e1, e2, e3) ->
    is_const_expr e1
    && Option.fold ~none:true ~some:is_const_expr e2
    && is_const_expr e3
  | Collection (_, _, afields) -> List.for_all afields ~f:is_const_afield

and is_const_afield afield =
  match afield with
  | AFvalue expr -> is_const_expr expr
  | AFkvalue (e1, e2) -> is_const_expr e1 && is_const_expr e2

let is_param_variadic param =
  match param.param_info with
  | Param_variadic -> true
  | Param_required -> false
  | Param_optional _ -> false

let is_param_splat param = Option.is_some param.param_splat

let is_param_optional param =
  match param.param_info with
  | Param_variadic -> false
  | Param_required -> false
  | Param_optional _ -> true

let get_param_default param =
  match param.param_info with
  | Param_variadic -> None
  | Param_required -> None
  | Param_optional e -> e

let get_expr_pos (_, p, _) = p

let get_argument_pos a =
  match a with
  | Anormal e -> get_expr_pos e
  | Ainout (_, e) -> get_expr_pos e

let arg_to_expr arg =
  match arg with
  | Anormal e -> e
  | Ainout (_, e) -> e

let expr_to_arg pk e =
  match pk with
  | Ast_defs.Pnormal -> Anormal e
  | Ast_defs.Pinout p -> Ainout (p, e)

let get_package_name = function
  | PackageConfigAssignment pkg
  | PackageOverride (_, pkg) ->
    pkg
