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

let constant_folding () =
  Hhbc_options.constant_folding !Hhbc_options.compiler_options

type env = {
  (* What is the current context? *)
  scope : Scope.t;
  (* How many existing classes are there? *)
  defined_class_count : int;
  (* How many existing functions are there? *)
  defined_function_count : int;
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
  (* Closure classes and hoisted inline classes *)
  hoisted_classes : class_ list;
  (* Hoisted inline functions *)
  hoisted_functions : fun_ list;
  (* The current namespace environment *)
  namespace: Namespace_env.env;
  (* Static variables in closures have special properties with mangled names
   * defined for them *)
  static_vars : ULS.t;
}

let initial_state =
{
  per_function_count = 0;
  captured_vars = ULS.empty;
  captured_this = false;
  defined_vars = SSet.empty;
  total_count = 0;
  hoisted_classes = [];
  hoisted_functions = [];
  namespace = Namespace_env.empty_with_default_popt;
  static_vars = ULS.empty;
}

(* Add a variable to the captured variables *)
let add_var st var =
  (* If it's bound as a parameter or definite assignment, don't add it *)
  (* Also don't add the pipe variable *)
  if SSet.mem var st.defined_vars
  || var = Naming_special_names.SpecialIdents.dollardollar
  then st
  else
  (* Don't bother if it's $this, as this is captured implicitly *)
  if var = Naming_special_names.SpecialIdents.this
  then { st with captured_this = true; }
  else { st with captured_vars = ULS.add st.captured_vars var }

let env_with_lambda env =
  { env with scope = ScopeItem.Lambda :: env.scope }

let env_with_longlambda env is_static =
  { env with scope = ScopeItem.LongLambda is_static :: env.scope }

let strip_id id = SU.strip_global_ns (snd id)

let rec make_scope_name scope =
  match scope with
  | [] -> ""
  | ScopeItem.Function fd :: _ -> strip_id fd.f_name
  | ScopeItem.Method md :: scope ->
    make_scope_name scope ^ "::" ^ strip_id md.m_name
  | ScopeItem.Class cd :: _ ->
    SU.Xhp.mangle_id (strip_id cd.c_name)
  | _ :: scope -> make_scope_name scope

let env_with_function env fd =
  { env with scope = ScopeItem.Function fd :: env.scope; }

let env_toplevel class_count function_count =
  { scope = Scope.toplevel;
    defined_class_count = class_count;
    defined_function_count = function_count; }

let env_with_method env md =
  { env with scope = ScopeItem.Method md :: env.scope; }

let env_with_class env cd =
  { env with scope = [ScopeItem.Class cd]; }

(* Clear the variables, upon entering a lambda *)
let enter_lambda st fd =
  { st with
    captured_vars = ULS.empty;
    captured_this = false;
    defined_vars =
      SSet.of_list (List.map fd.f_params (fun param -> snd param.param_id));
    static_vars = ULS.empty;
   }

let add_defined_var st var =
  { st with defined_vars = SSet.add var st.defined_vars }

let add_static_var st var =
  let st = add_defined_var st var in
  { st with static_vars = ULS.add st.static_vars var }

let set_namespace st ns =
  { st with namespace = ns }

let reset_function_count st =
  { st with per_function_count = 0; }

let add_function env st fd =
  let n = env.defined_function_count + List.length st.hoisted_functions in
  { st with hoisted_functions = fd :: st.hoisted_functions },
  { fd with f_body = []; f_name = (fst fd.f_name, string_of_int n) }

(* Make a stub class purely for the purpose of emitting the DefCls instruction
 *)
let make_defcls cd n =
{ cd with
  c_body = [];
  c_name = (fst cd.c_name, string_of_int n) }

let add_class env st cd =
  let n = env.defined_class_count + List.length st.hoisted_classes in
  { st with hoisted_classes = cd :: st.hoisted_classes },
  make_defcls cd n

let make_closure_name total_count env st =
  SU.Closures.mangle_closure
    (make_scope_name env.scope) st.per_function_count total_count

let make_closure ~explicit_use ~class_num
  p total_count env st lambda_vars tparams fd body =
  let rec is_scope_static scope =
    match scope with
    | ScopeItem.LongLambda is_static :: scope ->
      is_static || is_scope_static scope
    | ScopeItem.Function _ :: _ -> false
    | ScopeItem.Method md :: _ ->
      List.mem md.m_kind Static
    | ScopeItem.Lambda :: scope ->
      not st.captured_this || is_scope_static scope
    | _ -> false in
  let md = {
    m_kind = [Public] @ (if fd.f_static || is_scope_static env.scope then [Static] else []);
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
    m_doc_comment = fd.f_doc_comment;
  } in
  let cvl =
    List.map lambda_vars
    (fun name -> (p, (p, Hhbc_string_utils.Locals.strip_dollar name), None)) in
  let cvl = cvl @ (List.map (ULS.items st.static_vars)
    (fun name -> (p, (p,
     "86static_" ^ (Hhbc_string_utils.Locals.strip_dollar name)), None))) in
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
    c_doc_comment = None;
  } in
  (* Horrid hack: use empty body for implicit closed vars, [Noop] otherwise *)
  let inline_fundef =
    { fd with f_body = if explicit_use then [Noop] else [];
              f_name = (p, string_of_int class_num) } in
  inline_fundef, cd

let inline_class_name_if_possible env ~trait ~fallback_to_empty_string p pe =
  let get_class_call =
    p, Call ((pe, Id (pe, "get_class")), [], [], [])
  in
  let name c = p, String (pe, strip_id c.c_name) in
  let empty_str = p, String (pe, "") in
  match Scope.get_class env.scope with
  | Some c when trait ->
    if c.c_kind = Ctrait then name c else empty_str
  | Some c ->
    if c.c_kind = Ctrait then get_class_call else name c
  | None ->
    if fallback_to_empty_string then p, String (pe, "")
    else get_class_call

(* Translate special identifiers __CLASS__, __METHOD__ and __FUNCTION__ into
 * literal strings. It's necessary to do this before closure conversion
 * because the enclosing class will be changed. *)
let convert_id (env:env) p (pid, str as id) =
  let str = String.uppercase_ascii str in
  let return newstr = (p, String (pid, newstr)) in
  match str with
  | "__CLASS__" | "__TRAIT__"->
    inline_class_name_if_possible
      ~trait:(str = "__TRAIT__")
      ~fallback_to_empty_string:true
      env p pid
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
  | "__LINE__" ->
    (* If the expression goes on multi lines, we return the last line *)
    let _, line, _, _ = Pos.info_pos_extended pid in
    p, Int (pid, string_of_int line)
  | _ ->
    (p, Id id)

let rec convert_expr env st (p, expr_ as expr) =
  match expr_ with
  | Varray es ->
    let st, es = List.map_env st es (convert_expr env) in
    st, (p, Varray es)
  | Darray es ->
    let convert_pair st (e1, e2) = begin
      let st, e1 = convert_expr env st e1 in
      let st, e2 = convert_expr env st e2 in
      st, (e1, e2)
    end in
    let st, es =  List.map_env st es convert_pair in
    st, (p, Darray es)
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
  | Call ((_, Id (pe, "get_class")), _, [], []) ->
    st, inline_class_name_if_possible
      ~trait:false
      ~fallback_to_empty_string:false
      env p pe
  | Call ((_, (Class_const ((_, cid), _) | Class_get ((_, cid), _))) as e, el1, el2, el3)
  when cid = "parent" ->
    let st = add_var st "$this" in
    let st, e = convert_expr env st e in
    let st, el2 = convert_exprs env st el2 in
    let st, el3 = convert_exprs env st el3 in
    st, (p, Call(e, el1, el2, el3))
  | Call (e, el1, el2, el3) ->
    let st, e = convert_expr env st e in
    let st, el2 = convert_exprs env st el2 in
    let st, el3 = convert_exprs env st el3 in
    st, (p, Call(e, el1, el2, el3))
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
  | BracedExpr e ->
    let st, e = convert_expr env st e in
    st, (p, BracedExpr e)
  | Import(flavor, e) ->
    let st, e = convert_expr env st e in
    st, (p, Import(flavor, e))
  | Id (_, id) as ast_id when String_utils.string_starts_with id "$" ->
    let st = add_var st id in
    st, (p, ast_id)
  | Id id ->
    st, convert_id env p id
  | Class_get (cid, _)
  | Class_const (cid, _) ->
    let st = begin match (Ast_class_expr.id_to_expr cid) with
    | _, Lvar (_, id) -> add_var st id
    | _ -> st
    end in
    st, expr
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
  let static_vars = st.static_vars in
  let st = { st with total_count = total_count + 1; } in
  let st = enter_lambda st fd in
  let env = if Option.is_some use_vars_opt
            then env_with_longlambda env false
            else env_with_lambda env in
  let st, block = convert_block env st fd.f_body in
  let st = { st with per_function_count = st.per_function_count + 1 } in
  let lambda_vars = ULS.items st.captured_vars in
  (* For lambdas without explicit `use` variables, we ignore the computed
   * capture set and instead use the explicit set *)
  let lambda_vars, use_vars =
    match use_vars_opt with
    | None ->
      lambda_vars, List.map lambda_vars (fun var -> (p, var), false)
    | Some use_vars ->
      (* Remove duplicates (not efficient, but unlikely to be large),
       * remove variables that are actually just parameters *)
      let use_vars =
         (List.fold_right use_vars ~init:[]
          ~f:(fun ((_, name), _ as use_var) use_vars ->
            if List.exists use_vars (fun ((_, name'), _) -> name = name')
            || List.exists fd.f_params (fun p -> name = snd p.param_id)
            then use_vars else use_var :: use_vars))
      in
        List.map use_vars (fun ((_, var), _ref) -> var), use_vars in
  let tparams = Scope.get_tparams env.scope in
  let class_num = List.length st.hoisted_classes + env.defined_class_count in
  let inline_fundef, cd =
      make_closure
      ~explicit_use:(Option.is_some use_vars_opt)
      ~class_num
      p total_count env st lambda_vars tparams fd block in
  (* Restore capture and defined set *)
  let st = { st with captured_vars = captured_vars;
                     captured_this = captured_this || st.captured_this;
                     defined_vars = defined_vars;
                     static_vars = static_vars; } in
  (* Add lambda captured vars to current captured vars *)
  let st = List.fold_left lambda_vars ~init:st ~f:add_var in
  let st = { st with hoisted_classes = cd :: st.hoisted_classes } in
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
    let visit_static_var st e =
      begin match snd e with
      | Lvar (_, name)
      | Binop (Eq None, (_, Lvar (_, name)), _) -> add_static_var st name
      | _ -> failwith "Static var - impossible"
      end in
    let st = List.fold_left el ~init:st ~f:visit_static_var in
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
  | Def_inline (Class cd) ->
    let st, cd = convert_class env st cd in
    let st, stub_cd = add_class env st cd in
    st, Def_inline (Class stub_cd)
  | Def_inline (Fun fd) ->
    let st, fd = convert_fun env st fd in
    let st, stub_fd = add_function env st fd in
    st, Def_inline (Fun stub_fd)
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

and convert_params env st param_list =
  let convert_param env st param = match param.param_expr with
    | None -> st, param
    | Some e ->
      let st, e = convert_expr env st e in
      st, { param with param_expr = Some e }
  in
  List.map_env st param_list (convert_param env)

and convert_fun env st fd =
  let env = env_with_function env fd in
  let st = reset_function_count st in
  let st, block = convert_block env st fd.f_body in
  let st, params = convert_params env st fd.f_params in
  st, { fd with f_body = block; f_params = params }

and convert_class env st cd =
  let env = env_with_class env cd in
  let st = reset_function_count st in
  let st, c_body = List.map_env st cd.c_body (convert_class_elt env) in
  st, { cd with c_body = c_body }

and convert_class_var env st (pos, id, expr_opt) =
  match expr_opt with
  | None ->
    st, (pos, id, expr_opt)
  | Some expr ->
    let st, expr = convert_expr env st expr in
    st, (pos, id, Some expr)

and convert_class_const env st (id, expr) =
  let st, expr = convert_expr env st expr in
  st, (id, expr)

and convert_class_elt env st ce =
  match ce with
  | Method md ->
    let env = env_with_method env md in
    let st = reset_function_count st in
    let st, block = convert_block env st md.m_body in
    let st, params = convert_params env st md.m_params in
    st, Method { md with m_body = block; m_params = params }

  | ClassVars (kinds, hint, cvl) ->
    let st, cvl = List.map_env st cvl (convert_class_var env) in
    st, ClassVars (kinds, hint, cvl)

  | Const (ho, iel) ->
    let st, iel = List.map_env st iel (convert_class_const env) in
    st, Const (ho, iel)

  | _ ->
    st, ce

and convert_gconst env st gconst =
  let st, expr = convert_expr env st gconst.Ast.cst_value in
  st, { gconst with Ast.cst_value = expr }

and convert_defs env class_count typedef_count st dl =
  match dl with
  | [] -> st, []
  | Fun fd :: dl ->
    let st, fd = convert_fun env st fd in
    let st, dl = convert_defs env class_count typedef_count st dl in
    st, (true, Fun fd) :: dl
    (* Convert a top-level class definition into a true class definition and
     * a stub class that just corresponds to the DefCls instruction *)
  | Class cd :: dl ->
    let st, cd = convert_class env st cd in
    let stub_class = make_defcls cd class_count in
    let st, dl = convert_defs env (class_count + 1) typedef_count st dl in
    st, (true, Class cd) :: (true, Stmt (Def_inline (Class stub_class))) :: dl
  | Stmt stmt :: dl ->
    let st, stmt = convert_stmt env st stmt in
    let st, dl = convert_defs env class_count typedef_count st dl in
    st, (true, Stmt stmt) :: dl
  | Typedef td :: dl ->
    let st, dl = convert_defs env class_count (typedef_count + 1) st dl in
    let stub_td = { td with t_id =
      (fst td.t_id, string_of_int (typedef_count)) } in
    st, (true, Typedef td) :: (true, Stmt (Def_inline (Typedef stub_td))) :: dl
  | Constant c :: dl ->
    let st, c = convert_gconst env st c in
    let st, dl = convert_defs env class_count typedef_count st dl in
    st, (true, Constant c) :: dl
  | Namespace(_id, dl) :: dl' ->
    convert_defs env class_count typedef_count st (dl @ dl')
  | NamespaceUse x :: dl ->
    let st, dl = convert_defs env class_count typedef_count st dl in
    st, (true, NamespaceUse x) :: dl
  | SetNamespaceEnv ns :: dl ->
    let st = set_namespace st ns in
    let st, dl = convert_defs env class_count typedef_count st dl in
    st, (true, SetNamespaceEnv ns) :: dl

let count_classes defs =
  List.count defs ~f:(function Class _ -> true | _ -> false)

let hoist_toplevel_functions all_defs =
  let funs, nonfuns =
    List.partition_tf all_defs ~f:(function Fun _ -> true | _ -> false) in
  funs @ nonfuns

(* For all the definitions in a file unit, convert lambdas into classes with
 * invoke methods, and hoist inline class and function definitions to top
 * level.
 * The closure classes and hoisted definitions are placed after the existing
 * definitions.
 *)
let convert_toplevel_prog defs =
  let defs =
    if constant_folding ()
    then Ast_constant_folder.fold_program defs else defs in
  (* Reorder the functions so that they appear first. This matches the
   * behaviour of HHVM *)
  let defs = hoist_toplevel_functions defs in
  (* First compute the number of explicit classes in order to generate correct
   * integer identifiers for the generated classes. .main counts as a top-level
   * function and we place hoisted functions just after that *)
  let env = env_toplevel (count_classes defs) 1 in
  let st, p = convert_defs env 0 0 initial_state defs in
  let fun_defs = List.rev_map st.hoisted_functions (fun fd -> false, Fun fd) in
  let class_defs = List.rev_map st.hoisted_classes (fun cd -> false, Class cd) in
  fun_defs @ p @ class_defs
