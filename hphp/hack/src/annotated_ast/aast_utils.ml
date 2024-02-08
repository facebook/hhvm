(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

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
  List.find_opt
    (fun (field_name, _e) ->
      match field_name with
      | SFlit_str (_, n) -> String.equal name n
      | _ -> false)
    fields

let get_return_from_fun e =
  match e with
  | (_, _, Lfun ({ f_body = { fb_ast = [(_, Return (Some e))]; _ }; _ }, _))
  | ( _,
      _,
      Efun
        { ef_fun = { f_body = { fb_ast = [(_, Return (Some e))]; _ }; _ }; _ }
    ) ->
    Some e
  | _ -> None

let get_virtual_expr_from_et et =
  let get_body_helper e =
    match e with
    | (_, _, Call { args = _ :: (Pnormal, (_, _, Shape fields)) :: _; _ }) ->
      (match find_shape_field "type" fields with
      | Some (_, e) -> get_return_from_fun e
      | None -> None)
    | _ -> None
  in
  match et.et_runtime_expr with
  | (_, _, Call { func = e; _ }) ->
    (match get_return_from_fun e with
    | Some e -> get_body_helper e
    | None -> get_body_helper et.et_runtime_expr)
  | _ -> get_body_helper et.et_runtime_expr

let get_virtual_expr expr =
  match expr with
  | (_, _, ExpressionTree et) ->
    (match get_virtual_expr_from_et et with
    | Some expr -> expr
    | None -> expr)
  | _ -> expr
