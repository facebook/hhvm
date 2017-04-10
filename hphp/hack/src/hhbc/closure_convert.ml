(**
 * Copyright (c) 2017, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 *
*)

open Core
open Ast

module ULS = Unique_list_string

let strip_dollar id =
  String.sub id 1 (String.length id - 1)

type env = {
  (* Prefix used to name closure classes in form
   *   Closure$ (top-level statements)
   *   Closure$fun (top-level function called fun)
   *   Closure$cls::meth (method)
   *)
  prefix : string;
  (* Are we in a static or top-level function or statement?
   *)
  outer_is_static : bool;
  (* Number of closures created in the current function *)
  per_function_count : int;
  (* Free variables computed so far *)
  captured_vars : ULS.t;
  (* Variables that are *defined* in current scope (e.g. lambda parameters,
   * locals) and so should not be added to capture set
   *)
  defined_vars : SSet.t;
  (* Total number of closures created *)
  total_count : int;
  (* Closure class number, used in CreateCl instruction *)
  class_num : int;
  (* The closure classes themselves *)
  closures : class_ list;
}

let initial_env initial_class_num =
{
  prefix = "Closure$";
  outer_is_static = false;
  per_function_count = 0;
  captured_vars = ULS.empty;
  defined_vars = SSet.empty;
  total_count = 0;
  class_num = initial_class_num;
  closures = [];
}

(* Add a variable to the captured variables *)
let add_var env var =
  (* If it's bound as a parameter or definite assignment, don't add it *)
  if SSet.mem var env.defined_vars
  then env
  else
  (* Don't bother if it's $this, as this is captured implicitly *)
  if var = Naming_special_names.SpecialIdents.this
  then env
  else { env with captured_vars = ULS.add env.captured_vars var }

(* Clear the variables, upon entering a lambda *)
let enter_lambda env fd =
  { env with
    captured_vars = ULS.empty;
    defined_vars =
      SSet.of_list (List.map fd.f_params (fun param -> snd param.param_id));
   }

let add_defined_var env var =
  { env with defined_vars = SSet.add var env.defined_vars }

(* Set the prefix name, upon entering a top-level function *)
let set_function env fd =
  { env with
    prefix = "Closure$" ^ Utils.strip_ns (snd fd.f_name);
    outer_is_static = true;
    per_function_count = 0;
  }

(* Set the prefix name, upon entering a top-level function *)
let set_method env cd md =
  { env with
    prefix = "Closure$" ^
      Utils.strip_ns (snd cd.c_name) ^
      "::" ^
      Utils.strip_ns (snd md.m_name);
    outer_is_static = List.mem md.m_kind Static;
    per_function_count = 0;
  }

(* Retrieve the closure classes in the order that they were generated *)
let get_closure_classes env =
  List.rev env.closures

let make_closure_name total_count env =
  let name = if env.per_function_count = 1
               then env.prefix
               else env.prefix ^ "#" ^ string_of_int env.per_function_count in
  name ^ ";" ^ string_of_int total_count

let make_closure ~explicit_use p total_count class_num env lambda_vars fd body =
  let md = {
    m_kind = [Public] @ (if env.outer_is_static then [Static] else []);
    m_tparams = fd.f_tparams;
    m_constrs = [];
    m_name = (fst fd.f_name, "__invoke");
    m_params = fd.f_params;
    m_body = body;
    m_user_attributes = fd.f_user_attributes;
    m_ret = fd.f_ret;
    m_ret_by_ref = fd.f_ret_by_ref;
    m_fun_kind = fd.f_fun_kind;
    m_span = fd.f_span;
  } in
  let cvl =
    List.map lambda_vars (fun name -> (p, (p, strip_dollar name), None)) in
  let cd = {
    c_mode = fd.f_mode;
    c_user_attributes = [];
    c_final = false;
    c_kind = Cnormal;
    c_is_xhp = false;
    c_name = (p, make_closure_name total_count env);
    c_tparams = [];
    c_extends = [(p, Happly((p, "Closure"), []))];
    c_implements = [];
    c_body = [ClassVars ([Private], None, cvl); Method md];
    c_namespace = Namespace_env.empty_with_default_popt;
    c_enum = None;
    c_span = p;
  } in
  (* Horrid hack: use empty body for implicit closed vars, [Noop] otherwise *)
  let inline_fundef =
    { fd with f_body = if explicit_use then [Noop] else [];
              f_name = (p, string_of_int class_num) } in
  inline_fundef, cd

let rec convert_expr env (p, expr_ as expr) =
  match expr_ with
  | Array afl ->
    let env, afl = List.map_env env afl convert_afield in
    env, (p, Array afl)
  | Shape pairs ->
    let env, pairs = List.map_env env pairs (fun env (n, e) ->
      let env, e = convert_expr env e in
      env, (n, e)) in
    env, (p, Shape pairs)
  | Collection (id, afl) ->
    let env, afl = List.map_env env afl convert_afield in
    env, (p, Collection (id, afl))
  | Lvar id ->
    let env = add_var env (snd id) in
    env, (p, Lvar id)
  | Clone e ->
    let env, e = convert_expr env e in
    env, (p, Clone e)
  | Obj_get (e1, e2, flavor) ->
    let env, e1 = convert_expr env e1 in
    let env, e2 = convert_expr env e2 in
    env, (p, Obj_get (e1, e2, flavor))
  | Array_get(e1, Some e2) ->
    let env, e1 = convert_expr env e1 in
    let env, e2 = convert_expr env e2 in
    env, (p, Array_get (e1, Some e2))
  | Array_get (e, None) ->
    let env, e = convert_expr env e in
    env, (p, Array_get (e, None))
  | Call (e, el1, el2) ->
    let env, e = convert_expr env e in
    let env, el1 = convert_exprs env el1 in
    let env, el2 = convert_exprs env el2 in
    env, (p, Call(e, el1, el2))
  | String2 el ->
    let env, el = convert_exprs env el in
    env, (p, String2 el)
  | Yield af ->
    let env, af = convert_afield env af in
    env, (p, Yield af)
  | Await e ->
    let env, e = convert_expr env e in
    env, (p, Await e)
  | List el ->
    let env, el = convert_exprs env el in
    env, (p, List el)
  | Expr_list el ->
    let env, el = convert_exprs env el in
    env, (p, Expr_list el)
  | Cast (h, e) ->
    let env, e = convert_expr env e in
    env, (p, Cast (h, e))
  | Unop (op, e) ->
    let env, e = convert_expr env e in
    env, (p, Unop (op, e))
  | Binop (op, e1, e2) ->
    let env, e1 = convert_expr env e1 in
    let env, e2 = convert_expr env e2 in
    env, (p, Binop (op, e1, e2))
  | Pipe (e1, e2) ->
    let env, e1 = convert_expr env e1 in
    let env, e2 = convert_expr env e2 in
    env, (p, Pipe (e1, e2))
  | Eif (e1, opt_e2, e3) ->
    let env, e1 = convert_expr env e1 in
    let env, opt_e2 = convert_opt_expr env opt_e2 in
    let env, e3 = convert_expr env e3 in
    env, (p, Eif(e1, opt_e2, e3))
  | NullCoalesce (e1, e2) ->
    let env, e1 = convert_expr env e1 in
    let env, e2 = convert_expr env e2 in
    env, (p, NullCoalesce (e1, e2))
  | InstanceOf (e1, e2) ->
    let env, e1 = convert_expr env e1 in
    let env, e2 = convert_expr env e2 in
    env, (p, InstanceOf (e1, e2))
  | New (e, el1, el2) ->
    let env, e = convert_expr env e in
    let env, el1 = convert_exprs env el1 in
    let env, el2 = convert_exprs env el2 in
    env, (p, New(e, el1, el2))
  | Efun (fd, use_vars) ->
    convert_lambda env p fd (Some use_vars)
  | Lfun fd ->
    convert_lambda env p fd None
  | Xml(id, pairs, el) ->
    let env, pairs = List.map_env env pairs
      (fun env (id, e) ->
        let env, e = convert_expr env e in
        env, (id, e)) in
    let env, el = convert_exprs env el in
    env, (p, Xml(id, pairs, el))
  | Unsafeexpr e ->
    let env, e = convert_expr env e in
    env, (p, Unsafeexpr e)
  | Import(flavor, e) ->
    let env, e = convert_expr env e in
    env, (p, Import(flavor, e))
  | _ ->
    env, expr

(* Closure-convert a lambda expression, with use_vars_opt = Some vars
 * if there is an explicit `use` clause.
 *)
and convert_lambda env p fd use_vars_opt =
  (* Remember the current capture and defined set across the lambda *)
  let captured_vars = env.captured_vars in
  let defined_vars = env.defined_vars in
  let total_count = env.total_count in
  let class_num = env.class_num in
  let closures_until_now = env.closures in
  let env = { env with
    total_count = total_count + 1;
    closures = [];
    class_num = env.class_num + 1 } in
  let env = enter_lambda env fd in
  let env, block = convert_block env fd.f_body in
  let env = { env with per_function_count = env.per_function_count + 1 } in
  let lambda_vars = ULS.items env.captured_vars in
  (* For lambdas without  explicit `use` variables, we ignore the computed
   * capture set and instead use the explicit set *)
  let lambda_vars, use_vars =
    match use_vars_opt with
    | None ->
      lambda_vars, List.map lambda_vars (fun var -> (p, var), false)
    | Some use_vars ->
      List.map use_vars (fun ((_, var), _ref) -> var), use_vars in
  let inline_fundef, cd =
    make_closure ~explicit_use:(Option.is_some use_vars_opt)
      p total_count class_num env lambda_vars fd block in
  (* Restore capture and defined set *)
  let env = { env with captured_vars = captured_vars;
                       defined_vars = defined_vars } in
  (* Add lambda captured vars to current captured vars *)
  let env = List.fold_left lambda_vars ~init:env ~f:add_var in
  let env = { env with closures = env.closures @ cd :: closures_until_now } in
  env, (p, Efun (inline_fundef, use_vars))

and convert_exprs env el =
  List.map_env env el convert_expr

and convert_opt_expr env oe =
  match oe with
  | None -> env, None
  | Some e ->
    let env, e = convert_expr env e in
    env, Some e

and convert_stmt env stmt =
  match stmt with
  | Expr e ->
    let env, e = convert_expr env e in
    env, Expr e
  | Block b ->
    let env, b = convert_block env b in
    env, Block b
  | Throw e ->
    let env, e = convert_expr env e in
    env, Throw e
  | Return (p, opt_e) ->
    let env, opt_e = convert_opt_expr env opt_e in
    env, Return (p, opt_e)
  | Static_var el ->
    let env, el = convert_exprs env el in
    env, Static_var el
  | If (e, b1, b2) ->
    let env, e = convert_expr env e in
    let env, b1 = convert_block env b1 in
    let env, b2 = convert_block env b2 in
    env, If(e, b1, b2)
  | Do (b, e) ->
    let env, b = convert_block env b in
    let env, e = convert_expr env e in
    env, Do (b, e)
  | While (e, b) ->
    let env, e = convert_expr env e in
    let env, b = convert_block env b in
    env, While (e, b)
  | For (e1, e2, e3, b) ->
    let env, e1 = convert_expr env e1 in
    let env, e2 = convert_expr env e2 in
    let env, e3 = convert_expr env e3 in
    let env, b = convert_block env b in
    env, For(e1, e2, e3, b)
  | Switch (e, cl) ->
    let env, e = convert_expr env e in
    let env, cl = List.map_env env cl convert_case in
    env, Switch (e, cl)
  | Foreach (e, p, ae, b) ->
    let env, e = convert_expr env e in
    let env, ae = convert_as_expr env ae in
    let env, b = convert_block env b in
    env, Foreach (e, p, ae, b)
  | Try (b1, cl, b2) ->
    let env, b1 = convert_block env b1 in
    let env, cl = List.map_env env cl convert_catch in
    let env, b2 = convert_block env b2 in
    env, Try (b1, cl, b2)
  | _ ->
    env, stmt

and convert_block env stmts =
  List.map_env env stmts convert_sequential_stmt

(* Special case for definitely assigned locals in straight-line code *)
and convert_sequential_stmt env stmt =
  match stmt with
  | Expr (p1, Binop (Ast.Eq None, e1, e2)) ->
    let env, e2 = convert_expr env e2 in
    let env, e1 = convert_lvalue_expr env e1 in
    env, Expr (p1, Binop (Ast.Eq None, e1, e2))

  | _ ->
    convert_stmt env stmt

and convert_catch env (id1, id2, b) =
  let env = add_defined_var env (snd id2) in
  let env, b = convert_block env b in
  env, (id1, id2, b)

and convert_case env case =
  match case with
  | Default b ->
    let env, b = convert_block env b in
    env, Default b
  | Case (e, b) ->
    let env, e = convert_expr env e in
    let env, b = convert_block env b in
    env, Case (e, b)

and convert_lvalue_expr env (p, expr_ as expr) =
  match expr_ with
  | Lvar id ->
    let env = add_defined_var env (snd id) in
    env, expr
  | List exprs ->
    let env, exprs = List.map_env env exprs convert_lvalue_expr in
    env, (p, List exprs)
  | _ ->
    convert_expr env expr

(* Everything here is an l-value *)
and convert_as_expr env aexpr =
  match aexpr with
  | As_v e ->
    let env, e = convert_lvalue_expr env e in
    env, As_v e
  | As_kv (e1, e2) ->
    let env, e1 = convert_lvalue_expr env e1 in
    let env, e2 = convert_lvalue_expr env e2 in
    env, As_kv (e1, e2)

and convert_afield env afield =
  match afield with
  | AFvalue e ->
    let env, e = convert_expr env e in
    env, AFvalue e
  | AFkvalue (e1, e2) ->
    let env, e1 = convert_expr env e1 in
    let env, e2 = convert_expr env e2 in
    env, AFkvalue (e1, e2)

let convert_fun env fd =
  let env = set_function env fd in
  let env, block = convert_block env fd.f_body in
  env, { fd with f_body = block }

let rec convert_class env cd =
  let env, c_body = List.map_env env cd.c_body (convert_class_elt cd) in
  env, { cd with c_body = c_body }

and convert_class_elt cd env ce =
  match ce with
  | Method md ->
    let env = set_method env cd md in
    let env, block = convert_block env md.m_body in
    env, Method { md with m_body = block }

  (* TODO Const, other elements containing expressions *)
  | _ ->
    env, ce

let rec convert_def env d =
  match d with
  | Fun fd ->
    let env, fd = convert_fun env fd in
    env, Fun fd
  | Class cd ->
    let env, cd = convert_class env cd in
    env, Class cd
  | Stmt st ->
    let env, st = convert_stmt env st in
    env, Stmt st
  | Typedef td ->
    env, Typedef td
  | Constant c ->
    env, Constant c
  | Namespace(id, dl) ->
    let env, dl = convert_prog env dl in
    env, Namespace(id, dl)
  | NamespaceUse x ->
    env, NamespaceUse x

and convert_prog env dl =
  List.map_env env dl convert_def
