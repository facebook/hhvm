(**
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Core_kernel
open Ast
open Utils

module Aast = Ast_to_nast.Aast
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

(* TODO It really sucks that this and Nast_terminality.Terminal are very
 * slightly different (notably, this version is somewhat buggier). Fixing that
 * exposes a lot of errors in www unfortunately -- we should bite the bullet on
 * fixing switch all the way when we do that, most likely though -- see tasks
 * #3140431 and #2813555. *)
let rec terminal nsenv ~in_try stl =
  List.iter stl (terminal_ nsenv ~in_try)

and terminal_ nsenv ~in_try (_, st_) =
  match st_ with
  | Throw _ when not in_try -> raise Exit
  | Throw _ -> ()
  | Continue _
  | Expr (_, ( Call ((_, Id (_, "assert")), _, [_, False], [])
             | Call ((_, Id (_, "invariant")), _, (_, False) :: _ :: _, [])))
  | Return _ -> raise Exit
  | Expr (_, Call ((_, Id fun_id), _, _, _)) ->
    let _, fun_name = NS.elaborate_id nsenv NS.ElaborateFun fun_id in
    FuncTerm.(raise_exit_if_terminal (get_fun fun_name))
  | Expr (_, Call ((_, Class_const ((_, Id cls_id), (_, meth_name))), _, _, _)) ->
    let _, cls_name = NS.elaborate_id nsenv NS.ElaborateClass cls_id in
    FuncTerm.(raise_exit_if_terminal (get_static_meth cls_name meth_name))
  | If (_, b1, b2) ->
    (try terminal nsenv ~in_try b1; () with Exit ->
      terminal nsenv ~in_try b2)
  | Switch (_, cl) ->
    terminal_cl nsenv ~in_try cl
  | Block b -> terminal nsenv ~in_try b
  | Using u -> terminal nsenv ~in_try u.us_block
  | Try (b, catch_l, _fb) ->
    (* return is not allowed in finally, so we can ignore fb *)
    (terminal nsenv ~in_try:true b;
     List.iter catch_l (terminal_catch nsenv ~in_try))
  | Markup _
  | Let _
  | Do _
  | While _
  | Declare _
  | For _
  | Foreach _
  | Def_inline _
  | Noop
  | Expr _
  | Unsafe
  | Fallthrough
  | Break _ (* TODO this is terminal sometimes too, except switch, see above. *)
  | GotoLabel _
  | Goto _
  | Awaitall _
    -> ()

and terminal_catch nsenv ~in_try (_, _, b) =
  terminal nsenv ~in_try b

and terminal_cl nsenv ~in_try = function
  | [] -> raise Exit
  | Case (_, b) :: rl ->
    (try
      terminal nsenv ~in_try b;
      if blockHasBreak b
      then ()
      else raise Exit
    with Exit -> terminal_cl nsenv ~in_try rl)
  | Default b :: rl ->
    begin try terminal nsenv ~in_try b with
      | Exit ->
        terminal_cl nsenv ~in_try rl
    end

and blockHasBreak = function
  | [] -> false
  | (_, Break _) :: _ -> true
  | x :: xs ->
    let x' =
      match x with
      | _, If (_, [], []) -> false
      | _, If (_, b, []) | _, If (_, [], b) -> blockHasBreak b
      | _, If (_, b1, b2) -> blockHasBreak b1 && blockHasBreak b2
      | _ -> false
    in
    x' || blockHasBreak xs

let is_terminal nsenv stl =
  try terminal nsenv ~in_try:false stl; false
  with Exit -> true

let smap_union ((nsenv:Namespace_env.env), (m1:Pos.t SMap.t))
    (m2:Pos.t SMap.t) =
  let m_combined = SMap.fold SMap.add m1 m2 in
  nsenv, m_combined

let rec lvalue (acc:(Namespace_env.env * Pos.t SMap.t)) = function
  | (p, Lvar (_, x)) ->
    let nsenv, m = acc in
    nsenv, SMap.add x p m
  | _, List lv -> List.fold_left lv ~init:acc ~f:(lvalue)
  (* Ref forms a local inside a foreach *)
  | (_, Unop (Uref, (p, Lvar (_, x)))) ->
    let nsenv, m = acc in
    nsenv, SMap.add x p m
  | _ -> acc

let rec stmt (acc:(Namespace_env.env * Pos.t SMap.t)) (_, st_) =
  let nsenv = fst acc in
  match st_ with
  | Expr e -> expr acc e
  | Unsafe
  | Fallthrough
  | Markup _
  | Break _ | Continue _ | Throw _ -> acc
  | Do (b, e) ->
    let acc = block acc b in
    let acc = expr acc e in
    acc
  | While (e, _b) -> expr acc e
  | For (e1, e2, _e3, _b) ->
    let acc = expr acc e1 in
    let acc = expr acc e2 in
    acc
  | Foreach (e, _await, as_e, _b) ->
    let acc = expr acc e in
    begin match as_e with
      | As_v v -> expr acc v
      | As_kv (k, v) ->
        let acc = expr acc k in
        let acc = expr acc v in
        acc
    end (* match *)
  | Declare _ | Return _ | GotoLabel _ | Goto _ | Def_inline _ | Noop -> acc
  | Awaitall (el, b) ->
    let acc = List.fold_left el ~init:acc ~f:(fun acc (_, e2) ->
      expr acc e2
    ) in
    let acc = block acc b in
    acc
  | Let (_x, _h, e) ->
    (* We would like to exclude scoped locals here, but gather the locals in
     * expression *)
    expr acc e
  | Using u -> block acc u.us_block
  | Block b -> block acc b
  | If (e, b1, b2) ->
    let acc = expr acc e in
    let term1 = is_terminal nsenv b1 in
    let term2 = is_terminal nsenv b2 in
    if term1 && term2
    then acc
    else if term1
    then
      let _, m2 = block (nsenv, SMap.empty) b2 in
      smap_union acc m2
    else if term2
    then
      let _, m1 = block (nsenv, SMap.empty) b1 in
      smap_union acc m1
    else begin
      let _, m1 = block (nsenv, SMap.empty) b1 in
      let _, m2 = block (nsenv, SMap.empty) b2 in
      let (m:Pos.t SMap.t) = (smap_inter m1 m2) in
      smap_union acc m
    end
  | Switch (e, cl) ->
    let acc = expr acc e in
    let cl = List.filter cl begin function
      | Case (_, b)
      | Default b -> not (is_terminal nsenv b)
    end in
    let cl = casel nsenv cl in
    let c = smap_inter_list cl in
    smap_union acc c
  | Try (b, cl, _fb) ->
    let _, c = block (nsenv, SMap.empty) b in
    let cl = List.filter cl begin fun (_, _, b) ->
      not (is_terminal nsenv b)
    end in
    let lcl = List.map cl (catch nsenv) in
    let c = smap_inter_list (c :: lcl) in
    smap_union acc c

and block acc l = List.fold_left l ~init:acc ~f:(stmt)

and casel nsenv = function
  | [] -> []
  | Case (_, []) :: rl -> casel nsenv rl
  | Default b :: rl
  | Case (_, b) :: rl ->
      let _, b = block (nsenv, SMap.empty) b in
      b :: casel nsenv rl

and catch nsenv (_, _, b) =
  snd (block (nsenv, SMap.empty) b)

and expr acc (_, e) =
  let expr_expr acc e1 e2 =
    let acc = expr acc e1 in
    let acc = expr acc e2 in
    acc
  in
  let field acc f =
    match f with
    | AFvalue e -> expr acc e
    | AFkvalue (k, v) -> expr_expr acc k v
  in
  let exprs acc es =
    List.fold_left es ~init:acc ~f:(expr)
  in
  match e with
  | Binop (Eq None, lv, rv) ->
    let acc = expr acc rv in
    lvalue acc lv
  | Array fields
  | Collection (_, _, fields) ->
    List.fold_left fields ~init:acc ~f:field
  | Varray (_, es)
  | List es
  | Expr_list es
  | String2 es ->
    exprs acc es
  | PrefixedString (_, e) -> expr acc e
  | Darray (_, exprexprs) ->
    List.fold_left exprexprs ~init:acc ~f:(fun acc -> fun (e1, e2) -> expr_expr acc e1 e2)
  | Shape fields ->
    List.fold_left fields ~init:acc ~f:(fun acc -> fun (_, e) -> expr acc e)
  | Clone e
  | Await e
  | Is (e, _)
  | As (e, _, _)
  | BracedExpr e
  | ParenthesizedExpr e
  | Cast (_, e)
  | Unop (_, e)
  | Class_const (e, _)
  | Callconv (_, e)
  | Import (_, e)
  | Yield_from e
  | Suspend e
  | PU_identifier (e, _, _) -> expr acc e
  | Obj_get (e1, e2, _)
  | Binop (_, e1, e2)
  | Pipe (e1, e2)
  | InstanceOf (e1, e2)
  | Class_get (e1, e2) -> expr_expr acc e1 e2
  | Array_get (e1, oe2) ->
    let acc = expr acc e1 in
    let acc = Option.value_map oe2 ~default:acc ~f:(expr acc) in
    acc
  | New (e1, _, es2, es3)
  | Call (e1, _, es2, es3) ->
    let acc = expr acc e1 in
    let acc = exprs acc es2 in
    let acc = exprs acc es3 in
    acc
  | Record (e1, exprexprs) ->
    let acc = expr acc e1 in
    List.fold_left exprexprs ~init:acc ~f:(fun acc -> fun (e1, e2) -> expr_expr acc e1 e2)
  | Yield f ->
    field acc f
  | Eif (e1, oe2, e3) ->
    let acc = expr acc e1 in
    let _, acc2 = Option.value_map oe2 ~default:acc ~f:(expr acc) in
    let _, acc3 = expr acc e3 in
    smap_union acc (smap_inter acc2 acc3)
  | Xml (_, attribs, es) ->
    let attrib acc a =
      match a with
      | Xhp_simple (_, e)
      | Xhp_spread e -> expr acc e
    in
    let acc = List.fold_left attribs ~init:acc ~f:attrib in
    let acc = exprs acc es in
    acc
  | Null
  | True
  | False
  | Omitted
  | Id _
  | Yield_break
  | Int _
  | Float _
  | String _
  | Efun _
  | Lfun _
  | Lvar _
  | PU_atom _
  | Unsafeexpr _ -> acc

let rec aast_lvalue (nsenv, m as acc) (p, e) =
  match e with
  | Aast.List lv -> List.fold_left ~init:acc ~f:(aast_lvalue) lv
  | Aast.Lvar (_, lid) -> nsenv, SMap.add (Local_id.to_string lid) p m
  | Aast.Unop (Uref, (p, Aast.Lvar (_, lid))) -> nsenv, SMap.add (Local_id.to_string lid) p m
  | _ -> acc

(* TODO: See TODO from `terminal` above *)
let rec aast_terminal nsenv ~in_try stl =
  List.iter stl (aast_terminal_ nsenv ~in_try)

and aast_terminal_ nsenv ~in_try st =
  match snd st with
  | Aast.Throw _ when not in_try -> raise Exit
  | Aast.Throw _ -> ()
  | Aast.Continue
  | Aast.Expr (_, ( Aast.Call (_, (_, Aast.Id (_, "assert")), _, [_, Aast.False], [])
             | Aast.Call (_, (_, Aast.Id (_, "invariant")), _, (_, Aast.False) :: _ :: _, [])))
  | Aast.Return _ -> raise Exit
  | Aast.Expr (_, Aast.Call (_, (_, Aast.Id fun_id), _, _, _)) ->
    let _, fun_name = NS.elaborate_id nsenv NS.ElaborateFun fun_id in
    FuncTerm.(raise_exit_if_terminal (get_fun fun_name))
  | Aast.Expr (_, Aast.Call (_, (_, Aast.Class_const
    ((_, Aast.CIexpr (_, Aast.Id cls_id)), (_, meth_name))), _, _, _)) ->
      let _, cls_name = NS.elaborate_id nsenv NS.ElaborateClass cls_id in
      FuncTerm.(raise_exit_if_terminal
        (get_static_meth cls_name meth_name))
  | Aast.If (_, b1, b2) ->
    (try aast_terminal nsenv ~in_try b1; () with Exit ->
      aast_terminal nsenv ~in_try b2)
  | Aast.Switch (_, cl) ->
    aast_terminal_cl nsenv ~in_try cl
  | Aast.Block b -> aast_terminal nsenv ~in_try b
  | Aast.Using u -> aast_terminal nsenv ~in_try u.Aast.us_block
  | Aast.Try (b, catch_l, _fb) ->
    (* return is not allowed in finally, so we can ignore fb *)
    (aast_terminal nsenv ~in_try:true b;
     List.iter catch_l (aast_terminal_catch nsenv ~in_try))
  | Aast.Break (* TODO this is terminal sometimes too, except switch, see above. *)
  | Aast.Expr _
  | Aast.Markup _
  | Aast.Let _
  | Aast.Do _
  | Aast.While _
  | Aast.Declare _
  | Aast.For _
  | Aast.Foreach _
  | Aast.Def_inline _
  | Aast.Noop
  | Aast.Fallthrough
  | Aast.GotoLabel _
  | Aast.Goto _
  | Aast.Awaitall _
  | Aast.Unsafe_block _
    -> ()

and aast_terminal_catch nsenv ~in_try (_, _, b) =
  aast_terminal nsenv ~in_try b

and aast_terminal_cl nsenv ~in_try = function
  | [] -> raise Exit
  | Aast.Case (_, b) :: rl ->
    (try
      aast_terminal nsenv ~in_try b;
      if aast_blockHasBreak b
      then ()
      else raise Exit
    with Exit -> aast_terminal_cl nsenv ~in_try rl)
  | Aast.Default b :: rl ->
    begin try aast_terminal nsenv ~in_try b with
      | Exit ->
        aast_terminal_cl nsenv ~in_try rl
    end

and aast_blockHasBreak = function
  | [] -> false
  | (_, Aast.Break) :: _ -> true
  | x :: xs ->
    let x' =
      match snd x with
      | Aast.If (_, [], []) -> false
      | Aast.If (_, b, [])
      | Aast.If (_, [], b) -> aast_blockHasBreak b
      | Aast.If (_, b1, b2) -> aast_blockHasBreak b1 && aast_blockHasBreak b2
      | _ -> false
    in
    x' || aast_blockHasBreak xs

let aast_is_terminal nsenv stl =
  try aast_terminal nsenv ~in_try:false stl; false
  with Exit -> true

let rec aast_expr acc (_, e) =
  let expr_expr acc e1 e2 =
    let acc = aast_expr acc e1 in
    let acc = aast_expr acc e2 in
    acc
  in
  let field acc f =
    match f with
    | Aast.AFvalue e -> aast_expr acc e
    | Aast.AFkvalue (k, v) -> expr_expr acc k v
  in
  let exprs acc es =
    List.fold_left es ~init:acc ~f:(aast_expr)
  in
  match e with
  | Aast.Binop (Eq None, lv, rv) ->
    let acc = aast_expr acc rv in
    aast_lvalue acc lv
  | Aast.Array fields
  | Aast.Collection (_, _, fields) ->
    List.fold_left fields ~init:acc ~f:field
  | Aast.Varray (_, es)
  | Aast.List es
  | Aast.Expr_list es
  | Aast.String2 es ->
    exprs acc es
  | Aast.PrefixedString (_, e) -> aast_expr acc e
  | Aast.Darray (_, exprexprs) ->
    List.fold_left exprexprs ~init:acc ~f:(fun acc -> fun (e1, e2) -> expr_expr acc e1 e2)
  | Aast.Shape fields ->
    List.fold_left fields ~init:acc ~f:(fun acc -> fun (_, e) -> aast_expr acc e)
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
  | Aast.Suspend e -> aast_expr acc e
  | Aast.Obj_get (e1, e2, _)
  | Aast.Binop (_, e1, e2)
  | Aast.Pipe (_, e1, e2)
  | Aast.InstanceOf (e1, (_, Aast.CIexpr e2))
  | Aast.Class_get ((_, Aast.CIexpr e1), Aast.CGexpr e2) -> expr_expr acc e1 e2
  | Aast.Class_get ((_, Aast.CIexpr e1), _) -> aast_expr acc e1
  | Aast.Class_const _
  | Aast.InstanceOf _
  | Aast.Class_get _ ->
    failwith "Unexpected Expr: Typing_get_locals expected CIexpr"
  | Aast.Array_get (e1, oe2) ->
    let acc = aast_expr acc e1 in
    let acc = Option.value_map oe2 ~default:acc ~f:(aast_expr acc) in
    acc
  | Aast.New ((_, Aast.CIexpr e1), _, es2, es3, _)
  | Aast.Call (_, e1, _, es2, es3) ->
    let acc = aast_expr acc e1 in
    let acc = exprs acc es2 in
    let acc = exprs acc es3 in
    acc
  | Aast.New _ ->
    failwith "Unexpected Expr: Typing_get_locals expected CIexpr in New"
  | Aast.Record ((_, Aast.CIexpr e1), exprexprs) ->
    let acc = aast_expr acc e1 in
    List.fold_left exprexprs ~init:acc ~f:(fun acc -> fun (e1, e2) -> expr_expr acc e1 e2)
  | Aast.Record _ ->
    failwith "Unexpected Expr: Typing_get_locals expected CIexpr in Record"
  | Aast.Yield f ->
    field acc f
  | Aast.Eif (e1, oe2, e3) ->
    let acc = aast_expr acc e1 in
    let _, acc2 = Option.value_map oe2 ~default:acc ~f:(aast_expr acc) in
    let _, acc3 = aast_expr acc e3 in
    smap_union acc (smap_inter acc2 acc3)
  | Aast.Xml (_, attribs, es) ->
    let attrib acc a =
      match a with
      | Aast.Xhp_simple (_, e)
      | Aast.Xhp_spread e -> aast_expr acc e
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
  | Aast.PU_identifier _
  | Aast.Unsafe_expr _ -> acc
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

let rec aast_stmt (acc:(Namespace_env.env * Pos.t SMap.t)) st =
  let nsenv = fst acc in
  match snd st with
  | Aast.Expr e -> aast_expr acc e
  | Aast.Fallthrough
  | Aast.Markup _
  | Aast.Break
  | Aast.Continue
  | Aast.Throw _ -> acc
  | Aast.Do (b, e) ->
    let acc = aast_block acc b in
    let acc = aast_expr acc e in
    acc
  | Aast.While (e, _b) -> aast_expr acc e
  | Aast.For (e1, e2, _e3, _b) ->
    let acc = aast_expr acc e1 in
    let acc = aast_expr acc e2 in
    acc
  | Aast.Foreach (e, as_e, _b) ->
    let acc = aast_expr acc e in
    begin
      match as_e with
      | Aast.As_v v
      | Aast.Await_as_v (_, v) -> aast_expr acc v
      | Aast.As_kv (k, v)
      | Aast.Await_as_kv (_, k, v) ->
        let acc = aast_expr acc k in
        let acc = aast_expr acc v in
        acc
    end
  | Aast.Declare _
  | Aast.Return _
  | Aast.Goto _
  | Aast.GotoLabel _
  | Aast.Def_inline _
  | Aast.Noop -> acc
  | Aast.Awaitall (el, b) ->
    let acc = List.fold_left ~init:acc ~f:(fun acc (_, e2) -> aast_expr acc e2) el in
    let acc = aast_block acc b in
    acc
  | Aast.Let (_x, _h, e) ->
    (* We would like to exclude scoped locals here, but gather the locals in
     * expression *)
    aast_expr acc e
  | Aast.Using u -> aast_block acc u.Aast.us_block
  | Aast.Block b -> aast_block acc b
  | Aast.If (e, b1, b2) ->
    let acc = aast_expr acc e in
    let term1 = aast_is_terminal nsenv b1 in
    let term2 = aast_is_terminal nsenv b2 in
    if term1 && term2
    then acc
    else if term1
    then
      let _, m2 = aast_block (nsenv, SMap.empty) b2 in
      smap_union acc m2
    else if term2
    then
      let _, m1 = aast_block (nsenv, SMap.empty) b1 in
      smap_union acc m1
    else
      let _, m1 = aast_block (nsenv, SMap.empty) b1 in
      let _, m2 = aast_block (nsenv, SMap.empty) b2 in
      let (m:Pos.t SMap.t) = (smap_inter m1 m2) in
      smap_union acc m
  | Aast.Switch (e, cl) ->
    let acc = aast_expr acc e in
    let cl = List.filter cl ~f:(function
      | Aast.Case (_, b)
      | Aast.Default b -> not (aast_is_terminal nsenv b)) in
    let cl = aast_casel nsenv cl in
    let c = smap_inter_list cl in
    smap_union acc c
  | Aast.Try (b, cl, _fb) ->
    let _, c = aast_block (nsenv, SMap.empty) b in
    let cl = List.filter cl ~f:(fun (_, _, b) -> not (aast_is_terminal nsenv b)) in
    let lcl = List.map cl (aast_catch nsenv) in
    let c = smap_inter_list (c :: lcl) in
    smap_union acc c
  | Aast.Unsafe_block b -> aast_block acc b

  and aast_block acc l =
    List.fold_left
      ~init:acc
      ~f:(fun acc st -> aast_stmt acc st)
      l

  and aast_casel nsenv cl =
    match cl with
    | [] -> []
    | Aast.Case (_, []) :: rl -> aast_casel nsenv rl
    | Aast.Default b :: rl
    | Aast.Case (_, b) :: rl ->
        let _, b = aast_block (nsenv, SMap.empty) b in
        b :: aast_casel nsenv rl

  and aast_catch nsenv (_, _, b) =
    snd (aast_block (nsenv, SMap.empty) b)
