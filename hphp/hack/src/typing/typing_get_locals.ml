(*
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Core_kernel
open Ast_defs
open Utils
module FuncTerm = Typing_func_terminality
module NS = Namespaces

(* Module calculating the locals for a statement
 * This is useful when someone uses $x on both sides
 * of an If statement, for example:
 * if(true) {
 *   $x = 0;
 * } else {
 *   $x = 1;
 * }
 *)

let smap_union
    ((nsenv : Namespace_env.env), (m1 : Pos.t SMap.t)) (m2 : Pos.t SMap.t) =
  let m_combined = SMap.fold SMap.add m1 m2 in
  (nsenv, m_combined)

let rec lvalue ((nsenv, m) as acc) (p, e) =
  match e with
  | Aast.List lv -> List.fold_left ~init:acc ~f:lvalue lv
  | Aast.Lvar (_, lid) -> (nsenv, SMap.add (Local_id.to_string lid) p m)
  | Aast.Unop (Uref, (p, Aast.Lvar (_, lid))) ->
    (nsenv, SMap.add (Local_id.to_string lid) p m)
  | _ -> acc

(* TODO It really sucks that this and Nast_terminality.Terminal are very
 * slightly different (notably, this version is somewhat buggier). Fixing that
 * exposes a lot of errors in www unfortunately -- we should bite the bullet on
 * fixing switch all the way when we do that, most likely though -- see tasks
 * #3140431 and #2813555. *)
let rec terminal nsenv ~in_try stl = List.iter stl (terminal_ nsenv ~in_try)

and terminal_ nsenv ~in_try st =
  match snd st with
  | Aast.Throw _ when not in_try -> raise Exit
  | Aast.Throw _ -> ()
  | Aast.Continue
  | Aast.TempContinue _
  | Aast.Expr
      ( _,
        ( Aast.Call (_, (_, Aast.Id (_, "assert")), _, [(_, Aast.False)], [])
        | Aast.Call
            (_, (_, Aast.Id (_, "invariant")), _, (_, Aast.False) :: _ :: _, [])
          ) )
  | Aast.Return _ ->
    raise Exit
  | Aast.Expr (_, Aast.Call (_, (_, Aast.Id fun_id), _, _, _)) ->
    let (_, fun_name) = NS.elaborate_id nsenv NS.ElaborateFun fun_id in
    FuncTerm.(raise_exit_if_terminal (get_fun fun_name))
  | Aast.Expr
      ( _,
        Aast.Call
          ( _,
            ( _,
              Aast.Class_const
                ((_, Aast.CIexpr (_, Aast.Id cls_id)), (_, meth_name)) ),
            _,
            _,
            _ ) ) ->
    let (_, cls_name) = NS.elaborate_id nsenv NS.ElaborateClass cls_id in
    FuncTerm.(raise_exit_if_terminal (get_static_meth cls_name meth_name))
  | Aast.If (_, b1, b2) ->
    (try
       terminal nsenv ~in_try b1;
       ()
     with Exit -> terminal nsenv ~in_try b2)
  | Aast.Switch (_, cl) -> terminal_cl nsenv ~in_try cl
  | Aast.Block b -> terminal nsenv ~in_try b
  | Aast.Using u -> terminal nsenv ~in_try u.Aast.us_block
  | Aast.Try (b, catch_l, _fb) ->
    (* return is not allowed in finally, so we can ignore fb *)
    terminal nsenv ~in_try:true b;
    List.iter catch_l (terminal_catch nsenv ~in_try)
  | Aast.Break
  (* TODO this is terminal sometimes too, except switch, see above. *)
  
  | Aast.TempBreak _
  | Aast.Expr _
  | Aast.Markup _
  | Aast.Let _
  | Aast.Do _
  | Aast.While _
  | Aast.For _
  | Aast.Foreach _
  | Aast.Def_inline _
  | Aast.Noop
  | Aast.Fallthrough
  | Aast.GotoLabel _
  | Aast.Goto _
  | Aast.Awaitall _ ->
    ()

and terminal_catch nsenv ~in_try (_, _, b) = terminal nsenv ~in_try b

and terminal_cl nsenv ~in_try = function
  | [] -> raise Exit
  | Aast.Case (_, b) :: rl ->
    (try
       terminal nsenv ~in_try b;
       if blockHasBreak b then
         ()
       else
         raise Exit
     with Exit -> terminal_cl nsenv ~in_try rl)
  | Aast.Default (_, b) :: rl ->
    begin
      try terminal nsenv ~in_try b with Exit -> terminal_cl nsenv ~in_try rl
    end

and blockHasBreak = function
  | [] -> false
  | (_, Aast.Break) :: _ -> true
  | x :: xs ->
    let x' =
      match snd x with
      | Aast.If (_, [], []) -> false
      | Aast.If (_, b, [])
      | Aast.If (_, [], b) ->
        blockHasBreak b
      | Aast.If (_, b1, b2) -> blockHasBreak b1 && blockHasBreak b2
      | _ -> false
    in
    x' || blockHasBreak xs

let is_terminal nsenv stl =
  try
    terminal nsenv ~in_try:false stl;
    false
  with Exit -> true

let rec expr acc (_, e) =
  let expr_expr acc e1 e2 =
    let acc = expr acc e1 in
    let acc = expr acc e2 in
    acc
  in
  let field acc f =
    match f with
    | Aast.AFvalue e -> expr acc e
    | Aast.AFkvalue (k, v) -> expr_expr acc k v
  in
  let exprs acc es = List.fold_left es ~init:acc ~f:expr in
  match e with
  | Aast.Binop (Eq None, lv, rv) ->
    let acc = expr acc rv in
    lvalue acc lv
  | Aast.Array fields
  | Aast.Collection (_, _, fields) ->
    List.fold_left fields ~init:acc ~f:field
  | Aast.Varray (_, es)
  | Aast.List es
  | Aast.Expr_list es
  | Aast.String2 es ->
    exprs acc es
  | Aast.PrefixedString (_, e) -> expr acc e
  | Aast.Darray (_, exprexprs) ->
    List.fold_left exprexprs ~init:acc ~f:(fun acc (e1, e2) ->
        expr_expr acc e1 e2)
  | Aast.Shape fields ->
    List.fold_left fields ~init:acc ~f:(fun acc (_, e) -> expr acc e)
  | Aast.Clone e
  | Aast.Await e
  | Aast.Is (e, _)
  | Aast.As (e, _, _)
  | Aast.BracedExpr e
  | Aast.ParenthesizedExpr e
  | Aast.Cast (_, e)
  | Aast.Unop (_, e)
  | Aast.Class_const ((_, Aast.CIexpr e), _)
  | Aast.Callconv (_, e)
  | Aast.Import (_, e)
  | Aast.Yield_from e
  | Aast.Suspend e ->
    expr acc e
  | Aast.Obj_get (e1, e2, _)
  | Aast.Binop (_, e1, e2)
  | Aast.Pipe (_, e1, e2)
  | Aast.Class_get ((_, Aast.CIexpr e1), Aast.CGexpr e2) ->
    expr_expr acc e1 e2
  | Aast.Class_get ((_, Aast.CIexpr e1), _) -> expr acc e1
  | Aast.Class_const _
  | Aast.Class_get _ ->
    failwith "Unexpected Expr: Typing_get_locals expected CIexpr"
  | Aast.Array_get (e1, oe2) ->
    let acc = expr acc e1 in
    let acc = Option.value_map oe2 ~default:acc ~f:(expr acc) in
    acc
  | Aast.New ((_, Aast.CIexpr e1), _, es2, es3, _)
  | Aast.Call (_, e1, _, es2, es3) ->
    let acc = expr acc e1 in
    let acc = exprs acc es2 in
    let acc = exprs acc es3 in
    acc
  | Aast.New _ ->
    failwith "Unexpected Expr: Typing_get_locals expected CIexpr in New"
  | Aast.Record ((_, Aast.CIexpr e1), _, exprexprs) ->
    let acc = expr acc e1 in
    List.fold_left exprexprs ~init:acc ~f:(fun acc (e1, e2) ->
        expr_expr acc e1 e2)
  | Aast.Record _ ->
    failwith "Unexpected Expr: Typing_get_locals expected CIexpr in Record"
  | Aast.Yield f -> field acc f
  | Aast.Eif (e1, oe2, e3) ->
    let acc = expr acc e1 in
    let (_, acc2) = Option.value_map oe2 ~default:acc ~f:(expr acc) in
    let (_, acc3) = expr acc e3 in
    smap_union acc (smap_inter acc2 acc3)
  | Aast.Xml (_, attribs, es) ->
    let attrib acc a =
      match a with
      | Aast.Xhp_simple (_, e)
      | Aast.Xhp_spread e ->
        expr acc e
    in
    let acc = List.fold_left attribs ~init:acc ~f:attrib in
    let acc = exprs acc es in
    acc
  | Aast.Null
  | Aast.True
  | Aast.False
  | Aast.Omitted
  | Aast.Id _
  | Aast.Yield_break
  | Aast.Int _
  | Aast.Float _
  | Aast.String _
  | Aast.Efun _
  | Aast.Lfun _
  | Aast.Lvar _
  | Aast.PU_atom _
  | Aast.PU_identifier _ ->
    acc
  (* These are not in the original AST *)
  | Aast.This
  | Aast.Any
  | Aast.ValCollection _
  | Aast.KeyValCollection _
  | Aast.ImmutableVar _
  | Aast.Dollardollar _
  | Aast.Lplaceholder _
  | Aast.Fun_id _
  | Aast.Method_id _
  | Aast.Method_caller _
  | Aast.Smethod_id _
  | Aast.Special_func _
  | Aast.Pair _
  | Aast.Assert _
  | Aast.Typename _ ->
    failwith "Unexpected Expr: Typing_get_locals expr not found on legacy AST"

let rec stmt (acc : Namespace_env.env * Pos.t SMap.t) st =
  let nsenv = fst acc in
  match snd st with
  | Aast.Expr e -> expr acc e
  | Aast.Fallthrough
  | Aast.Markup _
  | Aast.Break
  | Aast.TempBreak _
  | Aast.Continue
  | Aast.TempContinue _
  | Aast.Throw _ ->
    acc
  | Aast.Do (b, e) ->
    let acc = block acc b in
    let acc = expr acc e in
    acc
  | Aast.While (e, _b) -> expr acc e
  | Aast.For (e1, e2, _e3, _b) ->
    let acc = expr acc e1 in
    let acc = expr acc e2 in
    acc
  | Aast.Foreach (e, as_e, _b) ->
    let acc = expr acc e in
    begin
      match as_e with
      | Aast.As_v v
      | Aast.Await_as_v (_, v) ->
        expr acc v
      | Aast.As_kv (k, v)
      | Aast.Await_as_kv (_, k, v) ->
        let acc = expr acc k in
        let acc = expr acc v in
        acc
    end
  | Aast.Return _
  | Aast.Goto _
  | Aast.GotoLabel _
  | Aast.Def_inline _
  | Aast.Noop ->
    acc
  | Aast.Awaitall (el, b) ->
    let acc =
      List.fold_left ~init:acc ~f:(fun acc (_, e2) -> expr acc e2) el
    in
    let acc = block acc b in
    acc
  | Aast.Let (_x, _h, e) ->
    (* We would like to exclude scoped locals here, but gather the locals in
     * expression *)
    expr acc e
  | Aast.Using u -> block acc u.Aast.us_block
  | Aast.Block b -> block acc b
  | Aast.If (e, b1, b2) ->
    let acc = expr acc e in
    let term1 = is_terminal nsenv b1 in
    let term2 = is_terminal nsenv b2 in
    if term1 && term2 then
      acc
    else if term1 then
      let (_, m2) = block (nsenv, SMap.empty) b2 in
      smap_union acc m2
    else if term2 then
      let (_, m1) = block (nsenv, SMap.empty) b1 in
      smap_union acc m1
    else
      let (_, m1) = block (nsenv, SMap.empty) b1 in
      let (_, m2) = block (nsenv, SMap.empty) b2 in
      let (m : Pos.t SMap.t) = smap_inter m1 m2 in
      smap_union acc m
  | Aast.Switch (e, cl) ->
    let acc = expr acc e in
    let cl =
      List.filter cl ~f:(function
          | Aast.Case (_, b)
          | Aast.Default (_, b)
          -> not (is_terminal nsenv b))
    in
    let cl = casel nsenv cl in
    let c = smap_inter_list cl in
    smap_union acc c
  | Aast.Try (b, cl, _fb) ->
    let (_, c) = block (nsenv, SMap.empty) b in
    let cl = List.filter cl ~f:(fun (_, _, b) -> not (is_terminal nsenv b)) in
    let lcl = List.map cl (catch nsenv) in
    let c = smap_inter_list (c :: lcl) in
    smap_union acc c

and block acc l = List.fold_left ~init:acc ~f:(fun acc st -> stmt acc st) l

and casel nsenv cl =
  match cl with
  | [] -> []
  | Aast.Case (_, []) :: rl -> casel nsenv rl
  | Aast.Default (_, b) :: rl
  | Aast.Case (_, b) :: rl ->
    let (_, b) = block (nsenv, SMap.empty) b in
    b :: casel nsenv rl

and catch nsenv (_, _, b) = snd (block (nsenv, SMap.empty) b)
