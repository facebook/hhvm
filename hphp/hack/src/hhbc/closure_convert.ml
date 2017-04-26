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
open Ast_scope

module ULS = Unique_list_string
module SN = Naming_special_names
module SU = Hhbc_string_utils

type env = {
  (* What is the current context? *)
  scope : Scope.t;
}

type state = {
  (* Number of closures created in the current function *)
  per_function_count : int;
  (* Free variables computed so far *)
  captured_vars : ULS.t;
  captured_this : bool;
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

let initial_state initial_class_num =
{
  per_function_count = 0;
  captured_vars = ULS.empty;
  captured_this = false;
  defined_vars = SSet.empty;
  total_count = 0;
  class_num = initial_class_num;
  closures = [];
}

(* Add a variable to the captured variables *)
let add_var st var =
  (* If it's bound as a parameter or definite assignment, don't add it *)
  if SSet.mem var st.defined_vars
  then st
  else
  (* Don't bother if it's $this, as this is captured implicitly *)
  if var = Naming_special_names.SpecialIdents.this
  then { st with captured_this = true; }
  else { st with captured_vars = ULS.add st.captured_vars var }

let env_with_lambda env =
  { scope = ScopeItem.Lambda :: env.scope }

let env_with_longlambda env is_static =
  { scope = ScopeItem.LongLambda is_static :: env.scope }

let strip_id id = Utils.strip_ns (snd id)

let rec make_scope_name scope =
  match scope with
  | [] -> ""
  | ScopeItem.Function fd :: _ -> strip_id fd.f_name
  | ScopeItem.Method md :: scope ->
    make_scope_name scope ^ "::" ^ strip_id md.m_name
  | ScopeItem.Class cd :: _ -> strip_id cd.c_name
  | _ :: scope -> make_scope_name scope

let env_with_function env fd =
  { scope = ScopeItem.Function fd :: env.scope; }

let env_toplevel =
  { scope = Scope.toplevel; }

let env_with_method env md =
  { scope = ScopeItem.Method md :: env.scope; }

let env_with_class cd =
  { scope = [ScopeItem.Class cd]; }

(* Clear the variables, upon entering a lambda *)
let enter_lambda st fd =
  { st with
    captured_vars = ULS.empty;
    captured_this = false;
    defined_vars =
      SSet.of_list (List.map fd.f_params (fun param -> snd param.param_id));
   }

let add_defined_var st var =
  { st with defined_vars = SSet.add var st.defined_vars }

let reset_function_count st =
  { st with per_function_count = 0; }

(* Retrieve the closure classes in the order that they were generated *)
let get_closure_classes st =
  List.rev st.closures

let make_closure_name total_count env st =
  SU.Closures.mangle_closure
    (make_scope_name env.scope) st.per_function_count total_count

let make_closure ~explicit_use
  p total_count class_num env st lambda_vars tparams fd body =
  let rec is_scope_static scope =
    match scope with
    | ScopeItem.LongLambda is_static :: _ -> is_static
    | ScopeItem.Function _ :: _ -> true
    | ScopeItem.Method md :: _ -> List.mem md.m_kind Static
    | ScopeItem.Lambda :: scope -> is_scope_static scope || not st.captured_this
    | [] -> true
    | _ -> false in
  let md = {
    m_kind = [Public] @ (if is_scope_static env.scope then [Static] else []);
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
    List.map lambda_vars
    (fun name -> (p, (p, Hhbc_string_utils.Locals.strip_dollar name), None)) in
  let cd = {
    c_mode = fd.f_mode;
    c_user_attributes = [];
    c_final = false;
    c_kind = Cnormal;
    c_is_xhp = false;
    c_name = (p, make_closure_name total_count env st);
    c_tparams = tparams;
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

(* Translate special identifiers __CLASS__, __METHOD__ and __FUNCTION__ into
 * literal strings. It's necessary to do this before closure conversion
 * because the enclosing class will be changed. *)
let convert_id (env:env) p (pid, str as id) =
  let return newstr = (p, String (pid, newstr)) in
  let get_class_name () =
    match Scope.get_class env.scope with
    | None -> ""
    | Some cd -> strip_id cd.c_name in
  match str with
  | "__CLASS__" ->
    return (get_class_name ())
  | "__METHOD__" ->
    let prefix =
      match Scope.get_class env.scope with
      | None -> ""
      | Some cd -> strip_id cd.c_name ^ "::" in
    begin match env.scope with
      | ScopeItem.Function fd :: _ -> return (prefix ^ strip_id fd.f_name)
      | ScopeItem.Method md :: _ -> return (prefix ^ strip_id md.m_name)
      | (ScopeItem.Lambda | ScopeItem.LongLambda _) :: _ ->
        return (prefix ^ "{closure}")
      | _ -> return ""
    end
  | "__FUNCTION__" ->
    begin match env.scope with
    | ScopeItem.Function fd :: _ -> return (strip_id fd.f_name)
    | ScopeItem.Method md :: _ -> return (strip_id md.m_name)
    | (ScopeItem.Lambda | ScopeItem.LongLambda _) :: _ -> return "{closure}"
    | _ -> return ""
    end
  | _ ->
    (p, Id id)

let rec convert_expr env st (p, expr_ as expr) =
  match expr_ with
  | Array afl ->
    let st, afl = List.map_env st afl (convert_afield env) in
    st, (p, Array afl)
  | Shape pairs ->
    let st, pairs = List.map_env st pairs (fun st (n, e) ->
      let st, e = convert_expr env st e in
      st, (n, e)) in
    st, (p, Shape pairs)
  | Collection (id, afl) ->
    let st, afl = List.map_env st afl (convert_afield env) in
    st, (p, Collection (id, afl))
  | Lvar id ->
    let st = add_var st (snd id) in
    st, (p, Lvar id)
  | Clone e ->
    let st, e = convert_expr env st e in
    st, (p, Clone e)
  | Obj_get (e1, e2, flavor) ->
    let st, e1 = convert_expr env st e1 in
    let st, e2 = convert_expr env st e2 in
    st, (p, Obj_get (e1, e2, flavor))
  | Array_get(e1, opt_e2) ->
    let st, e1 = convert_expr env st e1 in
    let st, opt_e2 = convert_opt_expr env st opt_e2 in
    st, (p, Array_get (e1, opt_e2))
  | Call (e, el1, el2) ->
    let st, e = convert_expr env st e in
    let st, el1 = convert_exprs env st el1 in
    let st, el2 = convert_exprs env st el2 in
    st, (p, Call(e, el1, el2))
  | String2 el ->
    let st, el = convert_exprs env st el in
    st, (p, String2 el)
  | Yield af ->
    let st, af = convert_afield env st af in
    st, (p, Yield af)
  | Await e ->
    let st, e = convert_expr env st e in
    st, (p, Await e)
  | List el ->
    let st, el = convert_exprs env st el in
    st, (p, List el)
  | Expr_list el ->
    let st, el = convert_exprs env st el in
    st, (p, Expr_list el)
  | Cast (h, e) ->
    let st, e = convert_expr env st e in
    st, (p, Cast (h, e))
  | Unop (op, e) ->
    let st, e = convert_expr env st e in
    st, (p, Unop (op, e))
  | Binop (op, e1, e2) ->
    let st, e1 = convert_expr env st e1 in
    let st, e2 = convert_expr env st e2 in
    st, (p, Binop (op, e1, e2))
  | Pipe (e1, e2) ->
    let st, e1 = convert_expr env st e1 in
    let st, e2 = convert_expr env st e2 in
    st, (p, Pipe (e1, e2))
  | Eif (e1, opt_e2, e3) ->
    let st, e1 = convert_expr env st e1 in
    let st, opt_e2 = convert_opt_expr env st opt_e2 in
    let st, e3 = convert_expr env st e3 in
    st, (p, Eif(e1, opt_e2, e3))
  | NullCoalesce (e1, e2) ->
    let st, e1 = convert_expr env st e1 in
    let st, e2 = convert_expr env st e2 in
    st, (p, NullCoalesce (e1, e2))
  | InstanceOf (e1, e2) ->
    let st, e1 = convert_expr env st e1 in
    let st, e2 = convert_expr env st e2 in
    st, (p, InstanceOf (e1, e2))
  | New (e, el1, el2) ->
    let st, e = convert_expr env st e in
    let st, el1 = convert_exprs env st el1 in
    let st, el2 = convert_exprs env st el2 in
    st, (p, New(e, el1, el2))
  | Efun (fd, use_vars) ->
    convert_lambda env st p fd (Some use_vars)
  | Lfun fd ->
    convert_lambda env st p fd None
  | Xml(id, pairs, el) ->
    let st, pairs = List.map_env st pairs
      (fun st (id, e) ->
        let st, e = convert_expr env st e in
        st, (id, e)) in
    let st, el = convert_exprs env st el in
    st, (p, Xml(id, pairs, el))
  | Unsafeexpr e ->
    let st, e = convert_expr env st e in
    st, (p, Unsafeexpr e)
  | Import(flavor, e) ->
    let st, e = convert_expr env st e in
    st, (p, Import(flavor, e))
  | Id id ->
    st, convert_id env p id
  | _ ->
    st, expr

(* Closure-convert a lambda expression, with use_vars_opt = Some vars
 * if there is an explicit `use` clause.
 *)
and convert_lambda env st p fd use_vars_opt =
  (* Remember the current capture and defined set across the lambda *)
  let captured_vars = st.captured_vars in
  let captured_this = st.captured_this in
  let defined_vars = st.defined_vars in
  let total_count = st.total_count in
  let class_num = st.class_num in
  let closures_until_now = st.closures in
  let st = { st with
    total_count = total_count + 1;
    closures = [];
    class_num = st.class_num + 1 } in
  let st = enter_lambda st fd in
  let env = if Option.is_some use_vars_opt
            then env_with_longlambda env false
            else env_with_lambda env in
  let st, block = convert_block env st fd.f_body in
  let st = { st with per_function_count = st.per_function_count + 1 } in
  let lambda_vars = ULS.items st.captured_vars in
  (* For lambdas without  explicit `use` variables, we ignore the computed
   * capture set and instead use the explicit set *)
  let lambda_vars, use_vars =
    match use_vars_opt with
    | None ->
      lambda_vars, List.map lambda_vars (fun var -> (p, var), false)
    | Some use_vars ->
      List.map use_vars (fun ((_, var), _ref) -> var), use_vars in
  let tparams = Scope.get_tparams env.scope in
  let inline_fundef, cd =
    make_closure ~explicit_use:(Option.is_some use_vars_opt)
      p total_count class_num env st lambda_vars tparams fd block in
  (* Restore capture and defined set *)
  let st = { st with captured_vars = captured_vars;
                     captured_this = captured_this;
                     defined_vars = defined_vars; } in
  (* Add lambda captured vars to current captured vars *)
  let st = List.fold_left lambda_vars ~init:st ~f:add_var in
  let st = { st with closures = st.closures @ cd :: closures_until_now } in
  st, (p, Efun (inline_fundef, use_vars))

and convert_exprs env st el =
  List.map_env st el (convert_expr env)

and convert_opt_expr env st oe =
  match oe with
  | None -> st, None
  | Some e ->
    let st, e = convert_expr env st e in
    st, Some e

and convert_stmt env st stmt =
  match stmt with
  | Expr e ->
    let st, e = convert_expr env st e in
    st, Expr e
  | Block b ->
    let st, b = convert_block env st b in
    st, Block b
  | Throw e ->
    let st, e = convert_expr env st e in
    st, Throw e
  | Return (p, opt_e) ->
    let st, opt_e = convert_opt_expr env st opt_e in
    st, Return (p, opt_e)
  | Static_var el ->
    let st, el = convert_exprs env st el in
    st, Static_var el
  | If (e, b1, b2) ->
    let st, e = convert_expr env st e in
    let st, b1 = convert_block env st b1 in
    let st, b2 = convert_block env st b2 in
    st, If(e, b1, b2)
  | Do (b, e) ->
    let st, b = convert_block env st b in
    let st, e = convert_expr env st e in
    st, Do (b, e)
  | While (e, b) ->
    let st, e = convert_expr env st e in
    let st, b = convert_block env st b in
    st, While (e, b)
  | For (e1, e2, e3, b) ->
    let st, e1 = convert_expr env st e1 in
    let st, e2 = convert_expr env st e2 in
    let st, e3 = convert_expr env st e3 in
    let st, b = convert_block env st b in
    st, For(e1, e2, e3, b)
  | Switch (e, cl) ->
    let st, e = convert_expr env st e in
    let st, cl = List.map_env st cl (convert_case env) in
    st, Switch (e, cl)
  | Foreach (e, p, ae, b) ->
    let st, e = convert_expr env st e in
    let st, ae = convert_as_expr env st ae in
    let st, b = convert_block env st b in
    st, Foreach (e, p, ae, b)
  | Try (b1, cl, b2) ->
    let st, b1 = convert_block env st b1 in
    let st, cl = List.map_env st cl (convert_catch env) in
    let st, b2 = convert_block env st b2 in
    st, Try (b1, cl, b2)
  | _ ->
    st, stmt

and convert_block env st stmts =
  List.map_env st stmts (convert_sequential_stmt env)

(* Special case for definitely assigned locals in straight-line code *)
and convert_sequential_stmt env st stmt =
  match stmt with
  | Expr (p1, Binop (Ast.Eq None, e1, e2)) ->
    let st, e2 = convert_expr env st e2 in
    let st, e1 = convert_lvalue_expr env st e1 in
    st, Expr (p1, Binop (Ast.Eq None, e1, e2))

  | _ ->
    convert_stmt env st stmt

and convert_catch env st (id1, id2, b) =
  let st = add_defined_var st (snd id2) in
  let st, b = convert_block env st b in
  st, (id1, id2, b)

and convert_case env st case =
  match case with
  | Default b ->
    let st, b = convert_block env st b in
    st, Default b
  | Case (e, b) ->
    let st, e = convert_expr env st e in
    let st, b = convert_block env st b in
    st, Case (e, b)

and convert_lvalue_expr env st (p, expr_ as expr) =
  match expr_ with
  | Lvar id ->
    let st = add_defined_var st (snd id) in
    st, expr
  | List exprs ->
    let st, exprs = List.map_env st exprs (convert_lvalue_expr env) in
    st, (p, List exprs)
  | _ ->
    convert_expr env st expr

(* Everything here is an l-value *)
and convert_as_expr env st aexpr =
  match aexpr with
  | As_v e ->
    let st, e = convert_lvalue_expr env st e in
    st, As_v e
  | As_kv (e1, e2) ->
    let st, e1 = convert_lvalue_expr env st e1 in
    let st, e2 = convert_lvalue_expr env st e2 in
    st, As_kv (e1, e2)

and convert_afield env st afield =
  match afield with
  | AFvalue e ->
    let st, e = convert_expr env st e in
    st, AFvalue e
  | AFkvalue (e1, e2) ->
    let st, e1 = convert_expr env st e1 in
    let st, e2 = convert_expr env st e2 in
    st, AFkvalue (e1, e2)

let convert_fun st fd =
  let fd = Ast_constant_folder.fold_function fd in
  let env = env_with_function env_toplevel fd in
  let st = reset_function_count st in
  let st, block = convert_block env st fd.f_body in
  st, { fd with f_body = block }

let rec convert_class st cd =
  let env = env_with_class cd in
  let st = reset_function_count st in
  let st, c_body = List.map_env st cd.c_body (convert_class_elt env) in
  st, { cd with c_body = c_body }

and convert_class_elt env st ce =
  match ce with
  | Method md ->
    let md = Ast_constant_folder.fold_method md in
    let env = env_with_method env md in
    let st = reset_function_count st in
    let st, block = convert_block env st md.m_body in
    st, Method { md with m_body = block }

  (* TODO Const, other elements containing expressions *)
  | _ ->
    st, ce

let convert_toplevel st stmt =
  let stmt = Ast_constant_folder.fold_stmt stmt in
  convert_stmt env_toplevel st stmt

and convert_gconst st gconst =
  let gconst = Ast_constant_folder.fold_gconst gconst in
  let st, expr = convert_expr env_toplevel st gconst.Ast.cst_value in
  st, { gconst with Ast.cst_value = expr }

(* Convert *only* top level definitions, not classes and functions *)
let rec convert_toplevel_def st d =
  match d with
  | Fun fd ->
    st, Fun fd
  | Class cd ->
    st, Class cd
  | Stmt stmt ->
    let st, stmt = convert_toplevel st stmt in
    st, Stmt stmt
  | Typedef td ->
    st, Typedef td
  | Constant c ->
    let st, c = convert_gconst st c in
    st, Constant c
  | Namespace(id, dl) ->
    let st, dl = convert_toplevel_prog st dl in
    st, Namespace(id, dl)
  | NamespaceUse x ->
    st, NamespaceUse x

and convert_toplevel_prog st dl =
  List.map_env st dl convert_toplevel_def
