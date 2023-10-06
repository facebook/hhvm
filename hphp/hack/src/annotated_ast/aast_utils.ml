(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

let rec can_be_captured =
  let open Aast_defs in
  function
  | Darray _
  | Varray _
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
