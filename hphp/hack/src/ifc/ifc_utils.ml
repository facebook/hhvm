(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Hh_prelude
module A = Aast

let identity x = x

let rec funpow n ~f ~init =
  if n <= 0 then
    init
  else
    funpow (n - 1) ~f ~init:(f init)

let combine_opts keep_some combine x y =
  match (x, y) with
  | (Some z, None)
  | (None, Some z) ->
    if keep_some then
      Some z
    else
      None
  | (Some x, Some y) -> Some (combine x y)
  | (None, None) -> None

let rec fold3 ~f ~init xs ys zs =
  List.Or_unequal_lengths.(
    match (xs, ys, zs) with
    | ([], [], []) -> Ok init
    | (x :: xs, y :: ys, z :: zs) ->
      begin
        match fold3 ~f ~init xs ys zs with
        | Ok acc -> Ok (f acc x y z)
        | err -> err
      end
    | _ -> Unequal_lengths)

let stmt_name = function
  | A.Fallthrough -> "Fallthrough"
  | A.Expr _ -> "Expr"
  | A.Break -> "Break"
  | A.Continue -> "Continue"
  | A.Throw _ -> "Throw"
  | A.Return _ -> "Return"
  | A.Yield_break -> "Yield_break"
  | A.Awaitall _ -> "Awaitall"
  | A.If _ -> "If"
  | A.Do _ -> "Do"
  | A.While _ -> "While"
  | A.Using _ -> "Using"
  | A.For _ -> "For"
  | A.Switch _ -> "Switch"
  | A.Foreach _ -> "Foreach"
  | A.Try _ -> "Try"
  | A.Noop -> "Noop"
  | A.Block _ -> "Block"
  | A.Markup _ -> "Markup"
  | A.AssertEnv _ -> "AssertEnv"

let expr_name = function
  | A.Darray _ -> "Darray"
  | A.Varray _ -> "Varray"
  | A.Shape _ -> "Shape"
  | A.ValCollection _ -> "ValCollection"
  | A.KeyValCollection _ -> "KeyValCollection"
  | A.Null -> "Null"
  | A.This -> "This"
  | A.True -> "True"
  | A.False -> "False"
  | A.Omitted -> "Omitted"
  | A.Id _ -> "Id"
  | A.Lvar _ -> "Lvar"
  | A.Dollardollar _ -> "Dollardollar"
  | A.Clone _ -> "Clone"
  | A.Obj_get _ -> "Obj_get"
  | A.Array_get _ -> "Array_get"
  | A.Class_get _ -> "Class_get"
  | A.Class_const _ -> "Class_const"
  | A.Call _ -> "Call"
  | A.FunctionPointer _ -> "FunctionPointer"
  | A.Int _ -> "Int"
  | A.Float _ -> "Float"
  | A.String _ -> "String"
  | A.String2 _ -> "String2"
  | A.PrefixedString _ -> "PrefixedString"
  | A.Yield _ -> "Yield"
  | A.Await _ -> "Await"
  | A.Tuple _ -> "Tuple"
  | A.List _ -> "List"
  | A.Cast _ -> "Cast"
  | A.Unop _ -> "Unop"
  | A.Binop _ -> "Binop"
  | A.Pipe _ -> "Pipe"
  | A.Eif _ -> "Eif"
  | A.Is _ -> "Is"
  | A.As _ -> "As"
  | A.New _ -> "New"
  | A.Record _ -> "Record"
  | A.Efun _ -> "Efun"
  | A.Lfun _ -> "Lfun"
  | A.Xml _ -> "Xml"
  | A.Callconv _ -> "Callconv"
  | A.Import _ -> "Import"
  | A.Collection _ -> "Collection"
  | A.ExpressionTree _ -> "ExpressionTree"
  | A.Lplaceholder _ -> "Lplaceholder"
  | A.Fun_id _ -> "Fun_id"
  | A.Method_id _ -> "Method_id"
  | A.Method_caller _ -> "Method_caller"
  | A.Smethod_id _ -> "Smethod_id"
  | A.Pair _ -> "Pair"
  | A.ET_Splice _ -> "ET_splice"
  | A.EnumClassLabel _ -> "EnumClassLabel"
  | A.Any -> "Any"
  | A.ReadonlyExpr _ -> "Readonly"
  | A.Hole _ -> "Hole"
