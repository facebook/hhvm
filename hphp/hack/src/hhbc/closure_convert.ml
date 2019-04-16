(**
 * Copyright (c) 2017, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Ast
open Ast_scope
open Core_kernel
open Common

module ULS = Unique_list_string
module SN = Naming_special_names
module SU = Hhbc_string_utils

let constant_folding () =
  Hhbc_options.constant_folding !Hhbc_options.compiler_options

let hacksperimental () =
  Hhbc_options.hacksperimental !Hhbc_options.compiler_options

type hoist_kind =
  | TopLevel (* Def that is already at top-level *)
  | Hoisted (* Def that was hoisted to top-level *)

type convert_result = {
  ast_defs: (hoist_kind * Ast.def) list;
  global_state: Emit_env.global_state;
  strict_types: bool option;
}

type variables = {
  (* all variables declared/used in the scope*)
  all_vars: SSet.t;
  (* names of parameters if scope correspond to a function *)
  parameter_names: SSet.t
}

type env = {
  (* Span of function/method body *)
  pos: Pos.t;
  (* What is the current context? *)
  scope : Scope.t;
  variable_scopes: variables list;
  (* How many existing classes are there? *)
  defined_class_count : int;
  (* How many existing records are there? *)
  defined_record_count : int;
  (* How many existing functions are there? *)
  defined_function_count : int;
  (* if we are immediately in using statement *)
  in_using: bool;
}

type per_function_state = {
  has_finally: bool;
  has_goto: bool;
  labels: bool SMap.t
}

let empty_per_function_state = {
  has_finally = false;
  has_goto = false;
  labels = SMap.empty;
}

(* let to_empty_state_if_no_goto s =
  (* if function does not have any goto statements inside - it is ok
     to ignore any labels that might appear in it *)
  if s.has_goto then s
  else empty_goto_state *)

type state = {
  (* Number of closures created in the current function *)
  closure_cnt_per_fun : int;
  (* Free variables computed so far *)
  captured_vars : ULS.t;
  captured_this : bool;
  captured_generics : ULS.t;
  (* Closure classes and hoisted inline classes *)
  hoisted_classes : class_ list;
  (* Hoisted inline functions *)
  hoisted_functions : fun_ list;
  (* Hoisted meth_caller functions *)
  named_hoisted_functions : fun_ SMap.t;
  (* Functions with inout_wrappers *)
  inout_wrappers : fun_ list;
  (* The current namespace environment *)
  namespace: Namespace_env.env;
  (* Set of closure names that used to have explicit 'use' language construct
    in original anonymous function *)
  explicit_use_set: SSet.t;
  (* Closures get converted into methods on a class. We need to keep
     track of the original namsepace of the closure in order to
     properly qualify things when that method's body is emitted. *)
  closure_namespaces : Namespace_env.env SMap.t;
  (* original enclosing class for closure *)
  closure_enclosing_classes: Ast.class_ SMap.t;
  (* information about current function *)
  current_function_state: per_function_state;

  (*** accumulated information about program ***)

  (* set of functions that has try-finally block *)
  functions_with_finally: SSet.t;
  (* maps name of function that has at least one goto statement
     to labels in function (bool value denotes whether label appear in using) *)
  function_to_labels_map: (bool SMap.t) SMap.t;
  seen_strict_types: bool option;
  (* most recent definition of lexical-scoped `let` variables *)
  let_vars: int SMap.t;
  (* maps unique name of lambda to Rx level of the declaring scope *)
  lambda_rx_of_scope: Rx.t SMap.t;
}

let set_has_finally st =
  if st.current_function_state.has_finally then st
  else { st with current_function_state =
       { st.current_function_state with has_finally = true } }

let set_label st l v =
  { st with current_function_state =
  { st.current_function_state with labels =
    SMap.add l v st.current_function_state.labels } }

let set_has_goto st =
  if st.current_function_state.has_goto then st
  else { st with current_function_state =
       { st.current_function_state with has_goto = true } }

let initial_state popt =
{
  closure_cnt_per_fun = 0;
  captured_vars = ULS.empty;
  captured_this = false;
  captured_generics = ULS.empty;
  hoisted_classes = [];
  hoisted_functions = [];
  named_hoisted_functions = SMap.empty;
  inout_wrappers = [];
  namespace = Namespace_env.empty popt;
  explicit_use_set = SSet.empty;
  closure_namespaces = SMap.empty;
  closure_enclosing_classes = SMap.empty;
  current_function_state = empty_per_function_state;
  functions_with_finally = SSet.empty;
  function_to_labels_map = SMap.empty;
  seen_strict_types = None;
  let_vars = SMap.empty;
  lambda_rx_of_scope = SMap.empty;
}

let total_class_count env st =
  List.length st.hoisted_classes + env.defined_class_count

let set_in_using env =
  if env.in_using then env else { env with in_using = true }

let reset_in_using env =
  if env.in_using then { env with in_using = false } else env

let is_in_lambda scope =
  match scope with
  | ScopeItem.Lambda _ | ScopeItem.LongLambda _ -> true
  | _ -> false

let should_capture_var env var =
  let rec aux scope vars =
    match scope, vars with
    | [], [{ all_vars; _ }] ->
      SSet.mem var all_vars
    | x :: xs, { all_vars; parameter_names; }::vs ->
      SSet.mem var all_vars ||
      SSet.mem var parameter_names ||
      (is_in_lambda x && aux xs vs)
    | _ -> false in
  match env.scope, env.variable_scopes with
  | _ :: xs, { parameter_names; _ } :: vs ->
    (* variable used in lambda should be captured if is
    - not contained in lambda parameter list
    AND
    - it exists in one of enclosing scopes *)
    not (SSet.mem var parameter_names) && aux xs vs
  | _ ->
    false

let get_let_var st x =
  let let_vars = st.let_vars in
  SMap.get x let_vars

let next_let_var_id st x =
  match get_let_var st x with
    | Some id -> id + 1
    | None -> 0

let update_let_var_id st x =
  let id = next_let_var_id st x in
  id, { st with let_vars = SMap.add x id st.let_vars }

(* We prefix the let variable with "$LET_VAR", and add "$%d" suffix for
 * distinguishing shadowings
 * This by construction does not clash with other variables, note that dollar
 * is not allowed as part of variable name by parser, but HHVM is fine with it.
 *)
let transform_let_var_name name id =
  Printf.sprintf "$LET_VAR_%s$%d" name id

let append_let_vars env let_vars =
  match env.variable_scopes with
  | [] -> env
  | outermost :: rest ->
    let all_vars = outermost.all_vars in
    let rec app var idx all_vars =
      if idx < 0
        then all_vars
        else
          let var_name = transform_let_var_name var idx in
          if SSet.mem var_name all_vars
            then all_vars
            else app var (idx - 1) (SSet.add var_name all_vars)
      in
    let all_vars = SMap.fold app let_vars all_vars in
    { env with variable_scopes = { outermost with all_vars = all_vars } :: rest }

(* Add a variable to the captured variables *)
let add_var env st var =
  (* Don't bother if it's $this, as this is captured implicitly *)
  if var = Naming_special_names.SpecialIdents.this
  then { st with captured_this = true; }
  (* If it's bound as a parameter or definite assignment, don't add it *)
  (* Also don't add the pipe variable and superglobals *)
  else if not (should_capture_var env var)
  || var = Naming_special_names.SpecialIdents.dollardollar
  || Naming_special_names.Superglobals.is_superglobal var
  then st
  else { st with captured_vars = ULS.add st.captured_vars var }

let add_generic env st var =
  let is_reified_tparam is_fun =
    let tparams =
      if is_fun then Ast_scope.Scope.get_fun_tparams env.scope
      else Ast_scope.Scope.get_class_tparams env.scope
    in
    List.find_mapi tparams ~f:(fun i { tp_name = (_, id); tp_reified = b; _ } ->
      if b && id = var then Some i else None)
  in
  match is_reified_tparam true with
  | Some i ->
    let var = SU.Reified.reified_generic_captured_name true i in
    { st with captured_generics = ULS.add st.captured_generics var }
  | None ->
    match is_reified_tparam false with
    | Some i ->
      let var = SU.Reified.reified_generic_captured_name false i in
      { st with captured_generics = ULS.add st.captured_generics var }
    | None -> st

let get_vars scope ~is_closure_body params body =
  let has_this = Scope.has_this scope in
  let is_toplevel = Scope.is_toplevel scope in
  let is_in_static_method = Scope.is_in_static_method scope in
  Decl_vars.vars_from_ast
    ~is_closure_body ~has_this ~params ~is_toplevel ~is_in_static_method body

let wrap_block b = [Ast.Stmt (Pos.none, Ast.Block b)]

let get_parameter_names params =
  List.fold_left
    ~init:SSet.empty
    ~f:(fun s p -> SSet.add (snd p.Ast.param_id) s)
    params

let env_with_function_like_ env e ~is_closure_body params pos body =
  let scope = e :: env.scope in
  let all_vars =
    (get_vars scope
      ~is_closure_body
      params
      (wrap_block body)) in
  let parameter_names = get_parameter_names params in
  { env with
      scope;
      pos;
      variable_scopes = { all_vars; parameter_names } :: env.variable_scopes }

let env_with_function_like env e ~is_closure_body fd =
  env_with_function_like_ env e ~is_closure_body fd.Ast.f_params
    fd.Ast.f_span fd.Ast.f_body

let fun_is_async = function FAsync | FAsyncGenerator -> true | _ -> false

let env_with_lambda env fd =
  let is_async = fun_is_async fd.Ast.f_fun_kind in
  let rx_level = Rx.rx_level_from_ast fd.Ast.f_user_attributes in
  env_with_function_like env (ScopeItem.Lambda (is_async, rx_level))
    ~is_closure_body:true fd

let env_with_longlambda env is_static fd =
  let is_async = fun_is_async fd.Ast.f_fun_kind in
  let rx_level = Rx.rx_level_from_ast fd.Ast.f_user_attributes in
  env_with_function_like env
    (ScopeItem.LongLambda (is_static, is_async, rx_level))
    ~is_closure_body:true fd

let strip_id id = SU.strip_global_ns (snd id)
let make_class_name cd = SU.Xhp.mangle_id (strip_id cd.c_name)

let rec make_scope_name ns scope =
  match scope with
  | [] ->
    begin match ns.Namespace_env.ns_name with
    | None -> ""
    | Some n -> n ^ "\\"
    end
  | ScopeItem.Function fd :: scope ->
    let fname = strip_id fd.f_name in
    begin match Scope.get_class scope with
    | None -> fname
    | Some cd -> make_class_name cd ^ "::" ^ fname
    end
  | ScopeItem.Method md :: scope ->
    let scope_name = make_scope_name ns scope in
    let scope_name =
      scope_name ^
      (if String_utils.string_ends_with scope_name "::" then "" else "::") in
    scope_name ^ strip_id md.m_name
  | ScopeItem.Class cd :: _ ->
    make_class_name cd
  | _ :: scope ->
    make_scope_name ns scope

let env_with_function env fd =
  env_with_function_like env (ScopeItem.Function fd) ~is_closure_body:false fd

let env_toplevel class_count record_count function_count defs =
  let scope = Scope.toplevel in
  let all_vars =
    get_vars scope
      ~is_closure_body:false
      []
      defs in
  { scope = scope
  ; pos = Pos.none
  ; variable_scopes = [{ all_vars; parameter_names = SSet.empty }]
  ; defined_class_count = class_count
  ; defined_record_count = record_count
  ; defined_function_count = function_count
  ; in_using = false
  }

let env_with_method env md =
  env_with_function_like_
    env
    (ScopeItem.Method md)
    ~is_closure_body:false
    md.Ast.m_params
    md.Ast.m_span
    md.Ast.m_body

let env_with_class env cd =
  { env with
      scope = [ScopeItem.Class cd];
      variable_scopes = env.variable_scopes; }

(* Clear the variables, upon entering a lambda *)
let enter_lambda st =
  { st with
    captured_vars = ULS.empty;
    captured_this = false;
    captured_generics = ULS.empty;
   }

let set_namespace st ns =
  { st with namespace = ns }

let reset_function_counts st =
  { st with closure_cnt_per_fun = 0 }

let record_function_state key {has_finally; has_goto; labels } rx_of_scope st =
  if not has_finally &&
     not has_goto &&
     SMap.is_empty labels &&
     rx_of_scope = Rx.NonRx
  then st
  else
  let functions_with_finally =
    if has_finally
    then SSet.add key st.functions_with_finally
    else st.functions_with_finally in
  let function_to_labels_map =
    if not @@ SMap.is_empty labels
    then SMap.add key labels st.function_to_labels_map
    else st.function_to_labels_map in
  let lambda_rx_of_scope =
    if rx_of_scope <> Rx.NonRx
    then SMap.add key rx_of_scope st.lambda_rx_of_scope
    else st.lambda_rx_of_scope in
  { st with functions_with_finally; function_to_labels_map; lambda_rx_of_scope }

(* Make a stub class purely for the purpose of emitting the DefCls instruction
 *)
let make_defcls cd n =
{ cd with
  c_body = [];
  c_name = (fst cd.c_name, string_of_int n) }

(* Make a stub record purely for the purpose of emitting the DefRecord instruction
 *)
let make_defrecord cd n =
{ cd with
  c_body = [];
  c_kind = Crecord;
  c_name = (fst cd.c_name, string_of_int n) }

let add_class env st cd =
  let n = env.defined_class_count + List.length st.hoisted_classes in
  { st with hoisted_classes = cd :: st.hoisted_classes },
  make_defcls cd n

let add_record env st cd =
  st, make_defrecord cd env.defined_record_count

let make_closure_name env st name =
  let per_fun_idx = st.closure_cnt_per_fun in
  SU.Closures.mangle_closure
    (make_scope_name st.namespace env.scope) per_fun_idx name

(* Get the user-provided closure name from the set of user attributes. This is
 * only allowed in systemlib and in transpiled javascript, otherwise we ignore the
 * attribute
 *
 * Returns the filtered list of attributes and the closure nam
 *)
let get_closure_name attrs =
  let is_closure_name attr = (snd attr.ua_name) = "__ClosureName" in
  if Emit_env.is_systemlib () || Emit_env.is_js ()
   then match List.find attrs is_closure_name with
     | Some { ua_params = [(_, String s)]; _ } ->
       (List.filter attrs (Fn.compose not is_closure_name)),(Some s)
     | _ -> attrs, None
   else attrs, None

let make_closure ~class_num
  p env st lambda_vars fun_tparams class_tparams is_static fd body =
  let user_attrs, name = get_closure_name fd.f_user_attributes in
  let md = {
    m_kind = [Public] @ (if is_static then [Static] else []);
    m_tparams = fun_tparams;
    m_constrs = [];
    m_name = (fst fd.f_name, "__invoke");
    m_params = fd.f_params;
    m_body = body;
    m_user_attributes = user_attrs;
    m_ret = fd.f_ret;
    m_fun_kind = fd.f_fun_kind;
    m_span = fd.f_span;
    m_doc_comment = fd.f_doc_comment;
    m_external = false;
  } in
  let cvl =
    List.map lambda_vars
    (fun name -> (p, (p, Hhbc_string_utils.Locals.strip_dollar name), None)) in
  let cd = {
    c_mode = fd.f_mode;
    c_user_attributes = [];
    c_file_attributes = [];
    c_final = false;
    c_kind = Cnormal;
    c_is_xhp = false;
    c_name = (p, make_closure_name env st name);
    c_tparams = class_tparams;
    c_extends = [(p, Happly((p, "Closure"), []))];
    c_implements = [];
    c_body = [
      ClassVars
      { cv_kinds = [Private]
      ; cv_hint = None
      ; cv_is_promoted_variadic = false
      ; cv_names = cvl
      ; cv_doc_comment = None
      ; cv_user_attributes = []};
      Method md
    ];
    c_namespace = Namespace_env.empty_with_default_popt;
    c_enum = None;
    c_span = p;
    c_doc_comment = None;
  } in
  let inline_fundef =
    { fd with f_body = body;
              f_static = is_static;
              f_name = (p, string_of_int class_num) } in
  inline_fundef, cd, md

(* Translate special identifiers __CLASS__, __METHOD__ and __FUNCTION__ into
 * literal strings. It's necessary to do this before closure conversion
 * because the enclosing class will be changed. *)
let convert_id (env:env) p (pid, str as id) =
  let str = String.uppercase str in
  let return newstr = (p, String newstr) in
  let name c = p, String (SU.Xhp.mangle @@ strip_id c.c_name) in
  match str with
  | "__TRAIT__" ->
    begin match Scope.get_class env.scope with
    | Some c when c.c_kind = Ctrait -> name c
    | _ -> return ""
    end
  | "__CLASS__" ->
    begin match Scope.get_class env.scope with
    | Some c when c.c_kind <> Ctrait -> name c
    | Some _ -> p, Id (pid, (snd id))
    | None -> return ""
    end
  | "__METHOD__" ->
    let prefix, is_trait =
      match Scope.get_class env.scope with
      | None -> "", false
      | Some cd ->
        (SU.Xhp.mangle @@ strip_id cd.c_name) ^ "::",
        cd.Ast.c_kind = Ast.Ctrait in
    let scope =
      if not is_trait then env.scope
      (* for lambdas nested in trait methods HHVM replaces __METHOD__
         with enclosing method name - do the same and bubble up from lambdas *)
      else List.drop_while env.scope ~f:(function
        | ScopeItem.Lambda _ | ScopeItem.LongLambda _ -> true
        | _ -> false) in
    begin match scope with
      | ScopeItem.Function fd :: _ -> return (prefix ^ strip_id fd.f_name)
      | ScopeItem.Method md :: _ -> return (prefix ^ strip_id md.m_name)
      | (ScopeItem.Lambda _ | ScopeItem.LongLambda _) :: _ ->
        return (prefix ^ "{closure}")
      (* PHP weirdness: __METHOD__ inside a class outside a method
       * returns class name *)
      | ScopeItem.Class cd :: _ -> return @@ strip_id cd.c_name
      | _ -> return ""
    end
  | "__FUNCTION__" ->
    begin match env.scope with
    | ScopeItem.Function fd :: _ -> return (strip_id fd.f_name)
    | ScopeItem.Method md :: _ -> return (strip_id md.m_name)
    | (ScopeItem.Lambda _ | ScopeItem.LongLambda _) :: _ -> return "{closure}"
    | _ -> return ""
    end
  | "__LINE__" ->
    (* If the expression goes on multi lines, we return the last line *)
    let _, line, _, _ = Pos.info_pos_extended pid in
    p, Int (string_of_int line)
  | _ ->
    (p, Id id)

let check_if_in_async_context { scope; pos; _ } =
  let check_valid_fun_kind (_, name) fk =
    if not (fun_is_async fk) then Emit_fatal.raise_fatal_parse pos @@
    "Function '"
    ^ (SU.strip_global_ns name)
    ^ "' contains 'await' but is not declared as async."
  in
  match scope with
  | [] -> Emit_fatal.raise_fatal_parse pos
            "'await' can only be used inside a function"
  | ScopeItem.Lambda (is_async, _) :: _
  | ScopeItem.LongLambda (_, is_async, _) :: _ ->
    if not is_async then
      Emit_fatal.raise_fatal_parse pos "Await may only appear in an async function"
  | ScopeItem.Class _ :: _ -> () (* Syntax error, wont get here *)
  | ScopeItem.Function fd :: _ ->
    check_valid_fun_kind fd.f_name fd.f_fun_kind
  | ScopeItem.Method md :: _ ->
    check_valid_fun_kind md.m_name md.m_fun_kind

(* meth_caller helpers *)
let rec get_scope_fmode scope =
  match scope with
  | [] -> FileInfo.Mstrict
  | ScopeItem.Class cd :: _ -> cd.c_mode
  | ScopeItem.Function fd :: _ -> fd.f_mode
  | _ :: scope -> get_scope_fmode scope

let make_fn_param pid is_variadic =
  {
    param_hint = None;
    param_is_reference = false;
    param_is_variadic = is_variadic;
    param_id = pid;
    param_expr = None;
    param_modifier = None;
    param_callconv = None;
    param_user_attributes = [];
  }

let convert_meth_caller_to_func_ptr env st p pc cls pf func =
  let mangle_name = SU.mangle_meth_caller cls func in
  let fun_handle =
    p, Call ((p, Id (p, "\\hh\\fun")), [], [(p, String mangle_name)], []) in
  if SMap.has_key mangle_name st.named_hoisted_functions then
    st, fun_handle
  else
    (* Build a new meth_caller function *)
    (* invariant(is_a($o, <cls>), 'object must be an instance of <cls>'); *)
    let obj_var = p, "$o" in
    let obj_lvar = p, Lvar obj_var in
    let assert_invariant = p, Call ((p, Id (p, "invariant")), [],
      [(p, Call((p, Id (p, "\\is_a")), [], [obj_lvar; (pc, String cls)], []));
       (p, String ("object must be an instance of (" ^ cls ^ ")"))], []) in

    (* return $o-><func>(...$args); *)
    let args_var = p, "$args" in
    let meth_caller_handle = p, Call
      ((p, Obj_get (obj_lvar, (p, Id (pf, func)), OG_nullthrows)), [], [],
      [(p, Lvar args_var)]) in
    let fd =  {
      f_mode = (get_scope_fmode env.scope);
      f_namespace = Namespace_env.empty_with_default_popt;
      f_name = (p, mangle_name);
      f_params =
        [(make_fn_param obj_var false); (make_fn_param args_var true)];
      f_body =
        [(p, Expr(assert_invariant)); (p, Return (Some meth_caller_handle))];
      f_tparams = [];
      f_constrs = [];
      f_ret = None;
      f_user_attributes = [{ua_name = (p, "builtin"); ua_params = []}];
      f_file_attributes = [];
      f_fun_kind = FSync;
      f_span = p;
      f_doc_comment = None;
      f_static = false;
      f_external = false;
    } in
    let named_hoisted_functions =
      SMap.add mangle_name fd st.named_hoisted_functions in
    { st with named_hoisted_functions }, fun_handle

let rec convert_expr env st (p, expr_ as expr) =
  match expr_ with
  | Null | True | False | Omitted | Yield_break
  | Int _ | Float _ | String _ -> st, expr
  | Varray (targ, es) ->
    let st, es = List.map_env st es (convert_expr env) in
    let st, targ = begin match targ with
      | None -> st, None
      | Some targ ->
        let st, targ = convert_hint env st targ in
        st, Some targ
    end in
    st, (p, Varray (targ, es))
  | Darray (tarp, es) ->
    let convert_pair st (e1, e2) = begin
      let st, e1 = convert_expr env st e1 in
      let st, e2 = convert_expr env st e2 in
      st, (e1, e2)
    end in
    let convert_tarp st (t1, t2) = begin
      let st, t1 = convert_hint env st t1 in
      let st, t2 = convert_hint env st t2 in
      st, (t1, t2)
    end in
    let st, es =  List.map_env st es convert_pair in
    begin match tarp with
    | Some typepair ->
      let st, tp = convert_tarp st typepair in
      st, (p, Darray (Some tp, es))
    | None -> st, (p, Darray (None, es))
    end
  | Array afl ->
    let st, afl = List.map_env st afl (convert_afield env) in
    st, (p, Array afl)
  | Shape pairs ->
    let st, pairs = List.map_env st pairs (fun st (n, e) ->
      let st, e = convert_expr env st e in
      st, (n, e)) in
    st, (p, Shape pairs)
  | Collection (id, targs, afl) ->
    let st, afl = List.map_env st afl (convert_afield env) in
    let st, ta = begin match targs with
    | Some CollectionTV tv ->
      let st, ta = convert_hint env st tv in
      st, Some (CollectionTV ta)
    | Some CollectionTKV (tk, tv) ->
      let st, tk = convert_hint env st tk in
      let st, tv = convert_hint env st tv in
      st, Some (CollectionTKV (tk, tv))
    | None -> st, None
    end in
    st, (p, Collection (id, ta, afl))
  | Lvar id ->
    let st = add_var env st (snd id) in
    st, (p, Lvar id)
  | Clone e ->
    let st, e = convert_expr env st e in
    st, (p, Clone e)
  | Obj_get (e1, e2, flavor) ->
    let st, e1 = convert_expr env st e1 in
    let st, e2 = convert_prop_expr env st e2 in
    st, (p, Obj_get (e1, e2, flavor))
  | Array_get(e1, opt_e2) ->
    let st, e1 = convert_expr env st e1 in
    let st, opt_e2 = convert_opt_expr env st opt_e2 in
    st, (p, Array_get (e1, opt_e2))
  | Call ((_, Id (_, meth_caller)), _, [(pc, cls); (pf, func)], [])
    when let name = String.lowercase @@ SU.strip_global_ns meth_caller in
      (name = "hh\\meth_caller" || name = "meth_caller") &&
      (Hhbc_options.emit_meth_caller_func_pointers !Hhbc_options.compiler_options) ->
    let cls = match cls with
      | Class_const (cid, (_, cs)) when SU.is_class cs ->
        let _, (_, ex) = convert_expr env st cid in
        let get_mangle_cls_name = match ex with
          | Id (pc, id) when not (SU.is_self id) &&
                             not (SU.is_parent id) &&
                             not (SU.is_static id) ->
              let fq_id =
                Hhbc_id.Class.elaborate_id st.namespace (pc, id) in
              Hhbc_id.Class.to_raw_string fq_id
          | _ -> Emit_fatal.raise_fatal_parse pc "Invalid class"
        in
        get_mangle_cls_name
      | String name -> name
      | _ -> Emit_fatal.raise_fatal_parse pc "Class must be a Class or string type"
      in
    let func = match func with
      | String name -> name
      | _ -> Emit_fatal.raise_fatal_parse pf "Func must be a string type"
      in
    convert_meth_caller_to_func_ptr env st p pc cls pf func
  | Call ((_, (Class_const ((_, Id (_, cid)), _)
             | Class_get ((_, Id (_, cid)), _))) as e,
    targs, el2, el3)
    when SU.is_parent cid ->
    let st = add_var env st "$this" in
    let st, e = convert_expr env st e in
    let st, targs = convert_hints env st targs in
    let st, el2 = convert_exprs env st el2 in
    let st, el3 = convert_exprs env st el3 in
    st, (p, Call(e, targs, el2, el3))
  | Call ((_, Id (_, id)), _, es, _)
    when String.lowercase id = "tuple" &&
      Emit_env.is_hh_syntax_enabled () ->
    convert_expr env st (p, Varray (None, es))
  | Call (e, targs, el2, el3) ->
    let st, e = convert_expr env st e in
    let st, targs = convert_hints env st targs in
    let st, el2 = convert_exprs env st el2 in
    let st, el3 = convert_exprs env st el3 in
    st, (p, Call(e, targs, el2, el3))
  | String2 el ->
    let st, el = convert_exprs env st el in
    st, (p, String2 el)
  | Yield af ->
    let st, af = convert_afield env st af in
    st, (p, Yield af)
  | Await e ->
    check_if_in_async_context env;
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
  | InstanceOf (e1, e2) ->
    let st, e1 = convert_expr env st e1 in
    let st, e2 = convert_expr env st e2 in
    st, (p, InstanceOf (e1, e2))
  | Is (e, h) ->
    let st, e = convert_expr env st e in
    let st, h = convert_hint env st h in
    st, (p, Is (e, h))
  | As (e, h, b) ->
    let st, e = convert_expr env st e in
    let st, h = convert_hint env st h in
    st, (p, As (e, h, b))
  | New (e, targs, el1, el2) ->
    let st, e = convert_expr env st e in
    let st, targs = convert_hints env st targs in
    let st, el1 = convert_exprs env st el1 in
    let st, el2 = convert_exprs env st el2 in
    st, (p, New (e, targs, el1, el2))
  | Record (e, es) ->
    let st, e = convert_expr env st e in
    let convert_pair st (e1, e2) = begin
      let st, e1 = convert_expr env st e1 in
      let st, e2 = convert_expr env st e2 in
      st, (e1, e2)
    end in
    let st, es = List.map_env st es convert_pair in
    st, (p, Record (e, es))
  | Efun (fd, use_vars) ->
    convert_lambda env st p fd (Some use_vars)
  | Lfun fd ->
    convert_lambda env st p fd None
  | Xml(id, pairs, el) ->
    let st, pairs = List.map_env st pairs (convert_xhp_attr env) in
    let st, el = convert_exprs env st el in
    st, (p, Xml(id, pairs, el))
  | Unsafeexpr e ->
    (* remove unsafe expressions from the AST, they are not used during
     * codegen
     *)
    convert_expr env st e
  | BracedExpr e ->
    let st, e = convert_expr env st e in
    (* For strings and lvars we should elide the braces *)
    begin match e with
    | _, Lvar _ | _, String _ -> st, e
    | _ -> st, (p, BracedExpr e)
    end
  | Import(flavor, e) ->
    let st, e = convert_expr env st e in
    st, (p, Import(flavor, e))
  | Id (_, id) as ast_id when String_utils.string_starts_with id "$" ->
    let st = add_var env st id in
    let st = add_generic env st id in
    st, (p, ast_id)
  | Id (_, var as id) ->
    (match get_let_var st var with
      | Some idx ->
        let lvar_name = transform_let_var_name var idx in
        let st = add_var env st lvar_name in
        st, (p, Lvar (p, lvar_name))
      | None ->
        let st = add_generic env st var in
        st, convert_id env p id)
  | Class_get (cid, n) ->
    let st, e = convert_expr env st cid in
    let st, n = convert_expr env st n in
    st, (p, Class_get (e, n))
  | Class_const (cid, n) ->
    let st, e = convert_expr env st cid in
    st, (p, Class_const (e, n))
  | PrefixedString (s, e) ->
    let st, e = convert_expr env st e in
    st, (p, PrefixedString (s, e))
  | Yield_from e ->
    let st, e = convert_expr env st e in
    st, (p, Yield_from e)
  | Suspend e ->
    let st, e = convert_expr env st e in
    st, (p, Suspend e)
  | ParenthesizedExpr e ->
    let st, e = convert_expr env st e in
    st, (p, ParenthesizedExpr e)
  | Callconv (k, e) ->
    let st, e = convert_expr env st e in
    st, (p, Callconv (k, e))
  | PU_atom _
  | PU_identifier _ ->
    failwith "TODO(T35357243): Pocket Universes syntax must be erased by now"

and convert_prop_expr env st (_, expr_ as expr) =
  match expr_ with
  | Id (_, id) when not (String_utils.string_starts_with id "$") ->
    st, expr
  | _ ->
    convert_expr env st expr

and convert_snd_expr env st (a, b_exp) =
  let s, b_exp = convert_expr env st b_exp in
  s, (a, b_exp)

and convert_hint env st (p, h as hint) =
  match h with
  | Happly ((_, id as ast_id), hl) ->
    let st = add_generic env st id in
    let st, hl = convert_hints env st hl in
    st, (p, Happly (ast_id, hl))
  | Hoption h ->
    let st, h = convert_hint env st h in
    st, (p, Hoption h)
  | Hlike h ->
    let st, h = convert_hint env st h in
    st, (p, Hlike h)
  | Hsoft h ->
    let st, h = convert_hint env st h in
    st, (p, Hsoft h)
  | Htuple hl ->
    let st, hl = convert_hints env st hl in
    st, (p, Htuple hl)
  | Hshape { si_allows_unknown_fields; si_shape_field_list } ->
    let st, si_shape_field_list =
      List.fold_right ~init:(st, []) si_shape_field_list
        ~f:(fun field (st, acc) ->
              let st, h = convert_hint env st field.sf_hint in
              st, { field with sf_hint = h } :: acc
           )
    in
    let info = { si_allows_unknown_fields; si_shape_field_list } in
    st, (p, Hshape info)
  | Haccess _
  | Hfun _ -> st, hint

(* Closure-convert a lambda expression, with use_vars_opt = Some vars
 * if there is an explicit `use` clause.
 *)
and convert_lambda env st p fd use_vars_opt =
  (* Remember the current capture and defined set across the lambda *)
  let captured_vars = st.captured_vars in
  let captured_this = st.captured_this in
  let captured_generics = st.captured_generics in
  let old_function_state = st.current_function_state in
  let st = enter_lambda st in
  let old_env = env in
  Option.iter use_vars_opt
    ~f:(List.iter ~f:(fun ((p, id)) ->
      if id = SN.SpecialIdents.this
      then Emit_fatal.raise_fatal_parse p "Cannot use $this as lexical variable"));
  let env = append_let_vars env st.let_vars in
  let rx_of_scope = Scope.rx_of_scope env.scope in
  let env = if Option.is_some use_vars_opt
            then env_with_longlambda env false fd
            else env_with_lambda env fd in
  let st, block, function_state = convert_function_like_body env st fd.f_body in
  let st = { st with closure_cnt_per_fun = st.closure_cnt_per_fun + 1 } in
  let current_generics = ULS.items st.captured_generics in
  (* HHVM lists lambda vars in descending order - do the same *)
  let lambda_vars =
    List.sort ~compare:(fun a b -> compare b a) @@
    (ULS.items st.captured_vars @ current_generics) in
  (* For lambdas with explicit `use` variables, we ignore the computed
   * capture set and instead use the explicit set *)
  let lambda_vars, use_vars =
    match use_vars_opt with
    | None ->
      lambda_vars, List.map lambda_vars (fun var -> (p, var))
    | Some use_vars ->
      (* Remove duplicates (not efficient, but unlikely to be large),
       * remove variables that are actually just parameters *)
      let use_vars =
         (List.fold_right use_vars ~init:[]
          ~f:(fun ((_, name) as use_var) use_vars ->
            if List.exists use_vars (fun ((_, name')) -> name = name')
            || List.exists fd.f_params (fun p -> name = snd p.param_id)
            then use_vars else use_var :: use_vars))
      in
      (* We still need to append the generics *)
      List.map use_vars (fun ((_, var)) -> var) @ current_generics,
      use_vars @ List.map current_generics (fun var -> (p, var))
  in
  let fun_tparams = Scope.get_fun_tparams env.scope in
  let class_tparams = Scope.get_class_tparams env.scope in
  let class_num = total_class_count env st in

  let rec is_scope_static scope =
    match scope with
    | ScopeItem.LongLambda (is_static, _, _) :: scope ->
      is_static || is_scope_static scope
    | ScopeItem.Function _ :: _ -> false
    | ScopeItem.Method md :: _ ->
      List.mem ~equal:(=) md.m_kind Static
    | ScopeItem.Lambda _ :: scope ->
      is_scope_static scope
    | _ -> false in

  let is_static =
    let is_static =
      (* long lambdas are static if they are annotated as such *)
      if Option.is_some use_vars_opt
      then fd.f_static
      (* short lambdas can be made static if they don't capture this in
         any form (including any nested non-static lambdas )*)
      else not st.captured_this in
    (* check if something can be promoted to static based on enclosing scope *)
    if is_static then is_static
    else is_scope_static env.scope in

  let inline_fundef, cd, md =
      make_closure
      ~class_num
      p env st lambda_vars fun_tparams class_tparams is_static fd block in
  let explicit_use_set =
    if Option.is_some use_vars_opt
    then SSet.add (snd inline_fundef.f_name) st.explicit_use_set
    else st.explicit_use_set in

  let closure_class_name = snd cd.c_name in

  let closure_enclosing_classes =
    match Scope.get_class env.scope with
    | Some cd -> SMap.add closure_class_name cd st.closure_enclosing_classes
    | None -> st.closure_enclosing_classes in

  (* adjust captured $this information *)
  let captured_this =
    (* we already know that $this is captured *)
    captured_this ||
    (* lambda that was just processed was converted into non-static one *)
    (not is_static) in
  (* Restore capture and defined set *)
  let st = { st with captured_vars;
                     captured_this;
                     captured_generics;
                     explicit_use_set;
                     closure_enclosing_classes;
                     closure_namespaces = SMap.add
                       closure_class_name st.namespace st.closure_namespaces;
                     current_function_state = old_function_state;
           } in
  let st =
    record_function_state
      (Emit_env.get_unique_id_for_method cd md) function_state rx_of_scope st in
  let env = old_env in
  (* Add lambda captured vars to current captured vars *)
  let st = List.fold_left lambda_vars ~init:st ~f:(add_var env) in
  let st = List.fold_left current_generics ~init:st ~f:(fun st var ->
    { st with captured_generics = ULS.add st.captured_generics var}) in
  let st = { st with hoisted_classes = cd :: st.hoisted_classes } in
  st, (p, Efun (inline_fundef, use_vars))

and convert_hints env st hl =
  List.map_env st hl (convert_hint env)

and convert_exprs env st el =
  List.map_env st el (convert_expr env)

and convert_opt_expr env st oe =
  match oe with
  | None -> st, None
  | Some e ->
    let st, e = convert_expr env st e in
    st, Some e

and convert_stmt env st (p, stmt_ as stmt) : _ * stmt =
  match stmt_ with
  | Expr e ->
    let st, e = convert_expr env st e in
    st, (p, Expr e)
  | Block b ->
    let st, b = convert_block env st b in
    st, (p, Block b)
  | Throw e ->
    let st, e = convert_expr env st e in
    st, (p, Throw e)
  | Return opt_e ->
    let st, opt_e = convert_opt_expr env st opt_e in
    st, (p, Return opt_e)
  | Awaitall (el, b) ->
    let st, el = List.map_env st el (convert_snd_expr env) in
    let st, b = convert_block env st b in
    st, (p, Awaitall (el, b))
  | If (e, b1, b2) ->
    let st, e = convert_expr env st e in
    let st, b1 = convert_block env st b1 in
    let st, b2 = convert_block env st b2 in
    st, (p, If(e, b1, b2))
  | Do (b, e) ->
    let let_vars_copy = st.let_vars in
    let st, b = convert_block ~scope:false (reset_in_using env) st b in
    let st, e = convert_expr env st e in
    { st with let_vars = let_vars_copy }, (p, Do (b, e))
  | While (e, b) ->
    let st, e = convert_expr env st e in
    let st, b = convert_block (reset_in_using env) st b in
    st, (p, While (e, b))
  | For (e1, e2, e3, b) ->
    let st, e1 = convert_expr env st e1 in
    let st, e2 = convert_expr env st e2 in
    let let_vars_copy = st.let_vars in
    let st, b = convert_block ~scope:false (reset_in_using env) st b in
    let st, e3 = convert_expr env st e3 in
    { st with let_vars = let_vars_copy }, (p, For(e1, e2, e3, b))
  | Switch (e, cl) ->
    let st, e = convert_expr env st e in
    let st, cl = List.map_env st cl (convert_case (reset_in_using env)) in
    st, (p, Switch (e, cl))
  | Foreach (e, p_opt, ae, b) ->
    if p_opt <> None then check_if_in_async_context env;
    let st, e = convert_expr env st e in
    let let_vars_copy = st.let_vars in
    let st, ae = convert_as_expr env st ae in
    let st, b = convert_block env st b in
    { st with let_vars = let_vars_copy } , (p, Foreach (e, p_opt, ae, b))
  | Try (b1, cl, b2) ->
    let st, b1 = convert_block env st b1 in
    let st, cl = List.map_env st cl (convert_catch env) in
    let st, b2 = convert_block (reset_in_using env) st b2 in
    let st = if List.is_empty b2 then st else set_has_finally st in
    st, (p, Try (b1, cl, b2))
  | Using s ->
    if s.us_has_await then check_if_in_async_context env;
    let st, us_expr = convert_expr env st s.us_expr in
    let st, us_block = convert_block (set_in_using env) st s.us_block in
    let st = set_has_finally st in
    st, (p, Using { s with us_expr; us_block })
  | Def_inline d ->
    (* propagate namespace information to nested classes or functions *)
    let _, defs = Namespaces.elaborate_def st.namespace d in
    let rec process_inline_defs st defs =
      match defs with
      | SetNamespaceEnv ns :: defs ->
        let st = set_namespace st ns in
        process_inline_defs st defs
      | Class cd :: _defs ->
        let st, cd = convert_class env st cd in
        let st, stub_cd =
          if cd.c_kind = Crecord then add_record env st cd
          else add_class env st cd in
        st, (p, Def_inline (Class stub_cd))
      | _ ->
        failwith "expected single class or function declaration" in
    process_inline_defs st defs
  | GotoLabel (_, l) ->
    (* record known label in function *)
    let st = set_label st l env.in_using in
    st, stmt
  | Goto _ ->
    (* record the fact that function has goto *)
    let st = set_has_goto st in
    st, stmt
  | Declare (_, (_, Binop (Eq None, (_, Id (_, "strict_types")), (_, Int v))), _) ->
    let st = { st with seen_strict_types = Some (v = "1") } in
    st, stmt
  | Declare (true, _, b) ->
    let st, _ = convert_block env st b in
    st, stmt
  | Let ((_, var), _hint, e) ->
    let st, e = convert_expr env st e in
    let id, st = update_let_var_id st var in
    let var_name = transform_let_var_name var id in
    (* We convert let statement to a simple assignment expression for simplicity *)
    st, (p, Expr (p, Binop (Eq None, (p, Lvar (p, var_name)), e)))
  | _ ->
    st, stmt

and convert_block ?(scope=true) env st stmts =
  if scope
  then
    let let_vars_copy = st.let_vars in
    let st, stmts = List.map_env st stmts (convert_stmt env) in
    { st with let_vars = let_vars_copy }, stmts
  else
    List.map_env st stmts (convert_stmt env)

and convert_function_like_body env old_st block =
  (* reset has_finally/goto_state values on the state *)
  let st =
    if old_st.current_function_state = empty_per_function_state
    then old_st
    else { old_st with current_function_state = empty_per_function_state }
  in
  let st, r = convert_block env st block in
  let function_state = st.current_function_state in
  (* restore old has_finally/goto_state values *)
  let st = { st with current_function_state = old_st.current_function_state } in
  st, r, function_state

and convert_catch env st (ty, (p, catch_var), b) =
  let let_vars_copy = st.let_vars in
  (* hacksperimental feature:
     variables with name not beginning with dollar are treated as immutable *)
  let st, catch_var = if catch_var.[0] = '$' || not (hacksperimental ())
    then
      st, catch_var
    else
      let id, st = update_let_var_id st catch_var in
      let var_name = transform_let_var_name catch_var id in
      st, var_name
  in
  let st, b = convert_block env st b in
  { st with let_vars = let_vars_copy }, (ty, (p, catch_var), b)

and convert_case env st case =
  match case with
  | Default b ->
    let st, b = convert_block env st b in
    st, Default b
  | Case (e, b) ->
    let st, e = convert_expr env st e in
    let st, b = convert_block env st b in
    st, Case (e, b)

and convert_as_expr env st aexpr =
  let convert_expr env st e = match e with
    | p1, Id (p2, var) when hacksperimental () ->
      let id, st = update_let_var_id st var in
      let var_name = transform_let_var_name var id in
      st, (p1, Lvar (p2, var_name))
    | _ -> convert_expr env st e
  in
  match aexpr with
  | As_v e ->
    let st, e = convert_expr env st e in
    st, As_v e
  | As_kv (e1, e2) ->
    let st, e1 = convert_expr env st e1 in
    let st, e2 = convert_expr env st e2 in
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

and convert_xhp_attr env st = function
  | Xhp_simple (id, e) ->
      let st, e = convert_expr env st e in
      st, Xhp_simple (id, e)
  | Xhp_spread e ->
      let st, e = convert_expr env st e in
      st, Xhp_spread e

and convert_params env st param_list =
  let convert_param env st param =
    let st, param_user_attributes =
      convert_user_attributes env st param.param_user_attributes in
    let st, param_expr =
      match param.param_expr with
      | None -> st, None
      | Some e ->
        let st, e = convert_expr env st e in
        st, Some e
    in
    st, { param with param_expr; param_user_attributes }
  in
  List.map_env st param_list (convert_param env)

and convert_user_attributes env st ual =
  List.map_env st ual
    (fun st ua ->
      let st, ua_params = convert_exprs env st ua.ua_params in
      st, { ua with ua_params })

and convert_fun env st fd =
  let env = env_with_function env fd in
  let st = reset_function_counts st in
  let st, f_body, function_state =
    convert_function_like_body env st fd.f_body in
  let st =
    record_function_state
      (Emit_env.get_unique_id_for_function fd) function_state Rx.NonRx st in
  let st, f_params = convert_params env st fd.f_params in
  let st, f_user_attributes =
    convert_user_attributes env st fd.f_user_attributes in
  st, { fd with f_body; f_params; f_user_attributes }

and add_reified_property cd body =
  if not @@ List.exists cd.c_tparams ~f:(function t -> t.tp_reified) then body
  else
    let p = Pos.none in
    (*
     * varray/vec that holds a list of type structures
     * this prop will be initilized during runtime
     *)
    let hint = Some (p, Happly ((p, "varray"), [])) in
    ClassVars
      { cv_kinds = [Private]
      ; cv_hint = hint
      ; cv_is_promoted_variadic = false
      ; cv_names =
        [p, (p, SU.Reified.reified_prop_name), None]
      ; cv_doc_comment = None
      ; cv_user_attributes = []
      } :: body

and convert_class env st cd =
  let env = env_with_class env cd in
  let st = reset_function_counts st in
  let st, c_body = List.map_env st cd.c_body (convert_class_elt env) in
  let c_body = add_reified_property cd c_body in
  let st, c_user_attributes =
    convert_user_attributes env st cd.c_user_attributes in
  st, { cd with c_body; c_user_attributes }

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
    let cls =
      match env.scope with
      | ScopeItem.Class c :: _-> c
      | _ -> failwith "unexpected scope shape - method is not inside the class" in
    let env = env_with_method env md in
    let st = reset_function_counts st in
    let st, m_body, function_state =
      convert_function_like_body env st md.m_body in
    let st =
      record_function_state
        (Emit_env.get_unique_id_for_method cls md) function_state Rx.NonRx st in
    let st, m_params = convert_params env st md.m_params in
    let st, m_user_attributes =
      convert_user_attributes env st md.m_user_attributes in
    st, Method { md with m_body; m_params; m_user_attributes }

  | ClassVars cv ->
    let st, cv_names = List.map_env st cv.cv_names (convert_class_var env) in
    let st, cv_user_attributes =
      convert_user_attributes env st cv.cv_user_attributes in
    st, ClassVars { cv with cv_names; cv_user_attributes }

  | Const (ho, iel) ->
    let st, iel = List.map_env st iel (convert_class_const env) in
    st, Const (ho, iel)

  | XhpAttr (h, c, v, es) ->
    let st, c = convert_class_var env st c in
    let st, es =
      match es with
      | None -> st, es
      | Some (p, opt, es) ->
        let st, es = convert_exprs env st es in
        st, Some (p, opt, es) in
    st, XhpAttr (h, c, v, es)
  | _ ->
    st, ce

and convert_gconst env st gconst =
  let st, expr = convert_expr env st gconst.Ast.cst_value in
  st, { gconst with Ast.cst_value = expr }

and convert_defs env class_count record_count typedef_count st dl =
  match dl with
  | [] -> st, []
  | Fun fd :: dl ->
    let let_vars_copy = st.let_vars in
    let st = { st with let_vars = SMap.empty } in
    let st, fd = convert_fun env st fd in
    let st, dl = convert_defs env class_count record_count typedef_count st dl in
    { st with let_vars = let_vars_copy }, (TopLevel, Fun fd) :: dl
    (* Convert a top-level class definition into a true class definition and
     * a stub class that just corresponds to the DefCls instruction *)
  | Class cd :: dl ->
    let let_vars_copy = st.let_vars in
    let st = { st with let_vars = SMap.empty } in
    let st, cd = convert_class env st cd in
    let stub_class =
      if cd.c_kind = Crecord then
        make_defrecord cd record_count
      else
        make_defcls cd class_count in
    let st, dl =
      if cd.c_kind = Crecord then
        convert_defs env class_count (record_count + 1) typedef_count st dl
      else
        convert_defs env (class_count + 1) record_count typedef_count st dl
    in
    { st with let_vars = let_vars_copy },
      (TopLevel, Class cd) :: (TopLevel, Stmt (Pos.none, Def_inline (Class stub_class))) :: dl
  | Stmt stmt :: dl ->
    let st, stmt = convert_stmt env st stmt in
    let st, dl = convert_defs env class_count record_count typedef_count st dl in
    st, (TopLevel, Stmt stmt) :: dl
  | Typedef td :: dl ->
    let st, dl = convert_defs env class_count record_count (typedef_count + 1) st dl in
    let st, t_user_attributes =
      convert_user_attributes env st td.t_user_attributes in
    let td = { td with t_user_attributes } in
    let stub_td = { td with t_id =
      (fst td.t_id, string_of_int (typedef_count)) } in
    st, (TopLevel, Typedef td) :: (TopLevel, Stmt (Pos.none, Def_inline (Typedef stub_td))) :: dl
  | Constant c :: dl ->
    let st, c = convert_gconst env st c in
    let st, dl = convert_defs env class_count record_count typedef_count st dl in
    st, (TopLevel, Constant c) :: dl
  | Namespace(_id, dl) :: dl' ->
    convert_defs env class_count record_count typedef_count st (dl @ dl')
  | NamespaceUse x :: dl ->
    let st, dl = convert_defs env class_count record_count typedef_count st dl in
    st, (TopLevel, NamespaceUse x) :: dl
  | SetNamespaceEnv ns :: dl ->
    let st = set_namespace st ns in
    let st, dl = convert_defs env class_count record_count typedef_count st dl in
    st, (TopLevel, SetNamespaceEnv ns) :: dl
  | FileAttributes fa :: dl ->
    let st, dl = convert_defs env class_count record_count typedef_count st dl in
    let st, fa_user_attributes =
      convert_user_attributes env st fa.fa_user_attributes in
    let fa = { fa with fa_user_attributes } in
    st, (TopLevel, FileAttributes fa) :: dl

let count_classes defs =
  List.count defs ~f:(function
    | Class {c_kind; _} -> c_kind <> Crecord
    | _ -> false)

let count_records defs =
  List.count defs ~f:(function
    | Class {c_kind; _} -> c_kind = Crecord
    | _ -> false)

let hoist_toplevel_functions all_defs =
  let funs, nonfuns =
    List.partition_tf all_defs ~f:(function _, Fun _ -> true | _ -> false) in
  funs @ nonfuns

(* For all the definitions in a file unit, convert lambdas into classes with
 * invoke methods, and hoist inline class and function definitions to top
 * level.
 * The closure classes and hoisted definitions are placed after the existing
 * definitions.
 *)
let convert_toplevel_prog ~popt defs =
  let defs =
    if constant_folding ()
    then Ast_constant_folder.fold_program defs else defs in
  (* First compute the number of explicit classes in order to generate correct
   * integer identifiers for the generated classes. .main counts as a top-level
   * function and we place hoisted functions just after that *)
  let env = env_toplevel (count_classes defs) (count_records defs) 1 defs in
  let st = initial_state popt in
  let st, original_defs = convert_defs env 0 0 0 st defs in
  let main_state = st.current_function_state in
  let st =
    record_function_state
      (Emit_env.get_unique_id_for_main ()) main_state Rx.NonRx st in
  (* Reorder the functions so that they appear first. This matches the
   * behaviour of HHVM. *)
  let original_defs = hoist_toplevel_functions original_defs in
  let fun_defs = List.rev_map st.hoisted_functions (fun fd -> Hoisted, Fun fd) in
  let named_hoisted_functions = SMap.values st.named_hoisted_functions in
  let named_fun_defs = List.rev_map named_hoisted_functions (fun fd -> TopLevel, Fun fd) in
  let class_defs = List.rev_map st.hoisted_classes (fun cd -> Hoisted, Class cd) in
  let ast_defs = fun_defs @ named_fun_defs @ original_defs @ class_defs in
  let global_state =
    Emit_env.(
      { global_explicit_use_set = st.explicit_use_set
      ; global_closure_namespaces = st.closure_namespaces
      ; global_closure_enclosing_classes = st.closure_enclosing_classes
      ; global_functions_with_finally = st.functions_with_finally
      ; global_function_to_labels_map = st.function_to_labels_map
      ; global_lambda_rx_of_scope = st.lambda_rx_of_scope }) in
  {
    ast_defs;
    global_state;
    strict_types = st.seen_strict_types;
  }
