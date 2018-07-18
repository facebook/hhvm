(**
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Ast
open Hh_core
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

(* TODO It really sucks that this and Nast_terminality.Terminal are very
 * slightly different (notably, this version is somewhat buggier). Fixing that
 * exposes a lot of errors in www unfortunately -- we should bite the bullet on
 * fixing switch all the way when we do that, most likely though -- see tasks
 * #3140431 and #2813555. *)
let rec terminal tcopt nsenv ~in_try stl =
  List.iter stl (terminal_ tcopt nsenv ~in_try)

and terminal_ tcopt nsenv ~in_try (_, st_) =
  match st_ with
  | Throw _ when not in_try -> raise Exit
  | Throw _ -> ()
  | Continue _
  | Expr (_, ( Call ((_, Id (_, "assert")), _, [_, False], [])
             | Call ((_, Id (_, "invariant")), _, (_, False) :: _ :: _, [])))
  | Return _ -> raise Exit
  | Expr (_, Call ((_, Id fun_id), _, _, _)) ->
    let _, fun_name = NS.elaborate_id nsenv NS.ElaborateFun fun_id in
    FuncTerm.(raise_exit_if_terminal (get_fun tcopt fun_name))
  | Expr (_, Call ((_, Class_const ((_, Id cls_id), (_, meth_name))), _, _, _)) ->
    let _, cls_name = NS.elaborate_id nsenv NS.ElaborateClass cls_id in
    FuncTerm.(raise_exit_if_terminal
      (get_static_meth tcopt cls_name meth_name))
  | If (_, b1, b2) ->
    (try terminal tcopt nsenv ~in_try b1; () with Exit ->
      terminal tcopt nsenv ~in_try b2)
  | Switch (_, cl) ->
    terminal_cl tcopt nsenv ~in_try cl
  | Block b -> terminal tcopt nsenv ~in_try b
  | Using u -> terminal tcopt nsenv ~in_try u.us_block
  | Try (b, catch_l, _fb) ->
    (* return is not allowed in finally, so we can ignore fb *)
    (terminal tcopt nsenv ~in_try:true b;
     List.iter catch_l (terminal_catch tcopt nsenv ~in_try))
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
  | Static_var _
  | Global_var _
    -> ()

and terminal_catch tcopt nsenv ~in_try (_, _, b) =
  terminal tcopt nsenv ~in_try b

and terminal_cl tcopt nsenv ~in_try = function
  | [] -> raise Exit
  | Case (_, b) :: rl ->
    (try
      terminal tcopt nsenv ~in_try b;
      if blockHasBreak b
      then ()
      else raise Exit
    with Exit -> terminal_cl tcopt nsenv ~in_try rl)
  | Default b :: rl ->
    begin try terminal tcopt nsenv ~in_try b with
      | Exit ->
        terminal_cl tcopt nsenv ~in_try rl
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

let is_terminal tcopt nsenv stl =
  try terminal tcopt nsenv ~in_try:false stl; false
  with Exit -> true

let smap_union ((nsenv:Namespace_env.env), (m1:Pos.t SMap.t))
    (m2:Pos.t SMap.t) =
  let m_combined = SMap.fold SMap.add m1 m2 in
  nsenv, m_combined

let rec lvalue tcopt (acc:(Namespace_env.env * Pos.t SMap.t)) = function
  | (p, Lvar (_, x)) ->
    let nsenv, m = acc in
    nsenv, SMap.add x p m
  | _, List lv -> List.fold_left lv ~init:acc ~f:(lvalue tcopt)
  (* Ref forms a local inside a foreach *)
  | (_, Unop (Uref, (p, Lvar (_, x)))) ->
    let nsenv, m = acc in
    nsenv, SMap.add x p m
  | _ -> acc

let rec stmt tcopt (acc:(Namespace_env.env * Pos.t SMap.t)) (_, st_) =
  let nsenv = fst acc in
  match st_ with
  | Expr e -> expr tcopt acc e
  | Unsafe
  | Fallthrough
  | Markup _
  | Break _ | Continue _ | Throw _ -> acc
  | Do (b, e) ->
    let acc = block tcopt acc b in
    let acc = expr tcopt acc e in
    acc
  | While (e, _b) -> expr tcopt acc e
  | For (e1, e2, _e3, _b) ->
    let acc = expr tcopt acc e1 in
    let acc = expr tcopt acc e2 in
    acc
  | Foreach (e, _await, as_e, _b) ->
    let acc = expr tcopt acc e in
    begin match as_e with
      | As_v v -> expr tcopt acc v
      | As_kv (k, v) ->
        let acc = expr tcopt acc k in
        let acc = expr tcopt acc v in
        acc
    end (* match *)
  | Declare _
  | Return _ | GotoLabel _ | Goto _ | Static_var _
  | Global_var _ | Def_inline _ | Noop -> acc
  | Let (_x, _h, e) ->
    (* We would like to exclude scoped locals here, but gather the locals in
     * expression *)
    expr tcopt acc e
  | Using u -> block tcopt acc u.us_block
  | Block b -> block tcopt acc b
  | If (e, b1, b2) ->
    let acc = expr tcopt acc e in
    let term1 = is_terminal tcopt nsenv b1 in
    let term2 = is_terminal tcopt nsenv b2 in
    if term1 && term2
    then acc
    else if term1
    then
      let _, m2 = block tcopt (nsenv, SMap.empty) b2 in
      smap_union acc m2
    else if term2
    then
      let _, m1 = block tcopt (nsenv, SMap.empty) b1 in
      smap_union acc m1
    else begin
      let _, m1 = block tcopt (nsenv, SMap.empty) b1 in
      let _, m2 = block tcopt (nsenv, SMap.empty) b2 in
      let (m:Pos.t SMap.t) = (smap_inter m1 m2) in
      smap_union acc m
    end
  | Switch (e, cl) ->
    let acc = expr tcopt acc e in
    let cl = List.filter cl begin function
      | Case (_, b)
      | Default b -> not (is_terminal tcopt nsenv b)
    end in
    let cl = casel tcopt nsenv cl in
    let c = smap_inter_list cl in
    smap_union acc c
  | Try (b, cl, _fb) ->
    let _, c = block tcopt (nsenv, SMap.empty) b in
    let cl = List.filter cl begin fun (_, _, b) ->
      not (is_terminal tcopt nsenv b)
    end in
    let lcl = List.map cl (catch tcopt nsenv) in
    let c = smap_inter_list (c :: lcl) in
    smap_union acc c

and block tcopt acc l = List.fold_left l ~init:acc ~f:(stmt tcopt)

and casel tcopt nsenv = function
  | [] -> []
  | Case (_, []) :: rl -> casel tcopt nsenv rl
  | Default b :: rl
  | Case (_, b) :: rl ->
      let _, b = block tcopt (nsenv, SMap.empty) b in
      b :: casel tcopt nsenv rl

and catch tcopt nsenv (_, _, b) =
  snd (block tcopt (nsenv, SMap.empty) b)

and expr tcopt acc (_, e) =
  let expr_expr acc e1 e2 =
    let acc = expr tcopt acc e1 in
    let acc = expr tcopt acc e2 in
    acc
  in
  let field acc f =
    match f with
    | AFvalue e -> expr tcopt acc e
    | AFkvalue (k, v) -> expr_expr acc k v
  in
  let exprs acc es =
    List.fold_left es ~init:acc ~f:(expr tcopt)
  in
  match e with
  | Binop (Eq None, lv, rv) ->
    let acc = expr tcopt acc rv in
    lvalue tcopt acc lv
  | Array fields
  | Collection (_, fields) ->
    List.fold_left fields ~init:acc ~f:field
  | Varray es
  | List es
  | Expr_list es
  | Execution_operator es
  | String2 es ->
    exprs acc es
  | PrefixedString (_, e) -> expr tcopt acc e
  | Darray exprexprs ->
    List.fold_left exprexprs ~init:acc ~f:(fun acc -> fun (e1, e2) -> expr_expr acc e1 e2)
  | Shape fields ->
    List.fold_left fields ~init:acc ~f:(fun acc -> fun (_, e) -> expr tcopt acc e)
  | Dollar e
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
  | Suspend e -> expr tcopt acc e
  | Obj_get (e1, e2, _)
  | Binop (_, e1, e2)
  | Pipe (e1, e2)
  | InstanceOf (e1, e2)
  | Class_get (e1, e2) -> expr_expr acc e1 e2
  | Array_get (e1, oe2) ->
    let acc = expr tcopt acc e1 in
    let acc = Option.value_map oe2 ~default:acc ~f:(expr tcopt acc) in
    acc
  | New (e1, _, es2, es3)
  | Call (e1, _, es2, es3) ->
    let acc = expr tcopt acc e1 in
    let acc = exprs acc es2 in
    let acc = exprs acc es3 in
    acc
  | Yield f ->
    field acc f
  | Eif (e1, oe2, e3) ->
    let acc = expr tcopt acc e1 in
    let _, acc2 = Option.value_map oe2 ~default:acc ~f:(expr tcopt acc) in
    let _, acc3 = expr tcopt acc e3 in
    smap_union acc (smap_inter acc2 acc3)
  | NewAnonClass (es1, es2, _) ->
    let acc = exprs acc es1 in
    let acc = exprs acc es2 in
    acc
  | Xml (_, attribs, es) ->
    let attrib acc a =
      match a with
      | Xhp_simple (_, e)
      | Xhp_spread e -> expr tcopt acc e
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
  | Unsafeexpr _ -> acc
