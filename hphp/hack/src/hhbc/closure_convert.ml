(**
 * Copyright (c) 2017, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Hh_core
open Ast
open Ast_scope

module ULS = Unique_list_string
module SN = Naming_special_names
module SU = Hhbc_string_utils

let constant_folding () =
  Hhbc_options.constant_folding !Hhbc_options.compiler_options

let hacksperimental () =
  Hhbc_options.hacksperimental !Hhbc_options.compiler_options

type convert_result = {
  ast_defs: (bool * Ast.def) list;
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
  (* How many existing functions are there? *)
  defined_function_count : int;
  (* if we are immediately in using statement *)
  in_using: bool;
  (* how many nested anonymous classes we are in *)
  anonclass_depth: int;
}

type per_function_state = {
  inline_hhas_blocks: string list;
  has_finally: bool;
  has_goto: bool;
  labels: bool SMap.t
}

let empty_per_function_state = {
  inline_hhas_blocks = [];
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
  (* Number of anonymous classes created in the current function *)
  anon_cls_cnt_per_fun : int;
  (* Free variables computed so far *)
  captured_vars : ULS.t;
  captured_this : bool;
  (* Closure classes and hoisted inline classes *)
  hoisted_classes : class_ list;
  (* Hoisted inline functions *)
  hoisted_functions : fun_ list;
  (* Functions with inout_wrappers *)
  inout_wrappers : fun_ list;
  (* The current namespace environment *)
  namespace: Namespace_env.env;
  (* Static variables in closures have special properties with mangled names
   * defined for them *)
  static_vars : ULS.t;
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
  (* map  functions -> list of inline hhas blocks *)
  functions_with_hhas_blocks: (string list) SMap.t;
  (* most recent definition of lexical-scoped `let` variables *)
  let_vars: int SMap.t;
}

let set_has_finally st =
  if st.current_function_state.has_finally then st
  else { st with current_function_state =
       { st.current_function_state with has_finally = true } }

let set_inline_hhas st hhas =
  { st with current_function_state =
  { st.current_function_state with inline_hhas_blocks =
    hhas :: st.current_function_state.inline_hhas_blocks }}

let set_label st l v =
  { st with current_function_state =
  { st.current_function_state with labels =
    SMap.add l v st.current_function_state.labels } }

let set_has_goto st =
  if st.current_function_state.has_goto then st
  else { st with current_function_state =
       { st.current_function_state with has_goto = true } }

let initial_state =
{
  closure_cnt_per_fun = 0;
  anon_cls_cnt_per_fun = 0;
  captured_vars = ULS.empty;
  captured_this = false;
  hoisted_classes = [];
  hoisted_functions = [];
  inout_wrappers = [];
  namespace = Namespace_env.empty_with_default_popt;
  static_vars = ULS.empty;
  explicit_use_set = SSet.empty;
  closure_namespaces = SMap.empty;
  closure_enclosing_classes = SMap.empty;
  current_function_state = empty_per_function_state;
  functions_with_finally = SSet.empty;
  function_to_labels_map = SMap.empty;
  seen_strict_types = None;
  functions_with_hhas_blocks = SMap.empty;
  let_vars = SMap.empty;
}

let total_class_count env st =
  List.length st.hoisted_classes + env.defined_class_count + env.anonclass_depth

let set_in_using env =
  if env.in_using then env else { env with in_using = true }

let reset_in_using env =
  if env.in_using then { env with in_using = false } else env

let is_in_lambda scope =
  match scope with
  | ScopeItem.Lambda | ScopeItem.LongLambda _ -> true
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

let env_with_lambda env fd =
  env_with_function_like env ScopeItem.Lambda ~is_closure_body:true fd

let env_with_longlambda env is_static fd =
  env_with_function_like env (ScopeItem.LongLambda is_static) ~is_closure_body:true fd

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
    let n = make_class_name cd in
    if Hhbc_string_utils.Classes.is_anonymous_class_name n then n ^ "::" else n
  | _ :: scope ->
    make_scope_name ns scope

let env_with_function env fd =
  env_with_function_like env (ScopeItem.Function fd) ~is_closure_body:false fd

let env_toplevel class_count function_count defs =
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
  ; defined_function_count = function_count
  ; in_using = false
  ; anonclass_depth = 0
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
    static_vars = ULS.empty;
   }

let add_static_var st var =
  { st with static_vars = ULS.add st.static_vars var }

let set_namespace st ns =
  { st with namespace = ns }

let reset_function_counts st =
  { st with
    closure_cnt_per_fun = 0;
    anon_cls_cnt_per_fun = 0 }

let record_function_state key { inline_hhas_blocks; has_finally; has_goto; labels } st =
  if List.is_empty inline_hhas_blocks &&
     not has_finally &&
     not has_goto &&
     SMap.is_empty labels
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
  let functions_with_hhas_blocks =
    if not @@ List.is_empty inline_hhas_blocks
    then SMap.add key (List.rev inline_hhas_blocks) st.functions_with_hhas_blocks
    else st.functions_with_hhas_blocks in
  { st with
    functions_with_finally; function_to_labels_map; functions_with_hhas_blocks }

let add_function ~has_inout_params env st fd =
  let n = env.defined_function_count
        + List.length st.hoisted_functions
        + List.length st.inout_wrappers in
  let inout_wrappers =
    if has_inout_params then fd :: st.inout_wrappers else st.inout_wrappers in
  let hoisted_functions = fd :: st.hoisted_functions in
  { st with hoisted_functions; inout_wrappers},
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

let make_closure_name env st =
  let per_fun_idx = st.closure_cnt_per_fun in
  SU.Closures.mangle_closure
    (make_scope_name st.namespace env.scope) per_fun_idx

let make_anonymous_class_name env st =
  let per_fun_idx = st.anon_cls_cnt_per_fun + 1 in
  SU.Classes.mangle_anonymous_class
    (make_scope_name st.namespace env.scope) per_fun_idx

let make_closure ~class_num
  p env st lambda_vars tparams is_static fd body =
  let md = {
    m_kind = [Public] @ (if is_static then [Static] else []);
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
    c_name = (p, make_closure_name env st);
    c_tparams = tparams;
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

let inline_class_name_if_possible env ~trait ~fallback_to_empty_string p pe =
  let get_class_call =
    p, Call ((pe, Id (pe, "get_class")), [], [], [])
  in
  let name c = p, String (SU.Xhp.mangle @@ strip_id c.c_name) in
  let empty_str = p, String ("") in
  match Scope.get_class env.scope with
  | Some c when trait ->
    if c.c_kind = Ctrait then name c else empty_str
  | Some c ->
    if c.c_kind = Ctrait then get_class_call else name c
  | None ->
    if fallback_to_empty_string then p, String ("")
    else get_class_call

(* Translate special identifiers __CLASS__, __METHOD__ and __FUNCTION__ into
 * literal strings. It's necessary to do this before closure conversion
 * because the enclosing class will be changed. *)
let convert_id (env:env) p (pid, str as id) =
  let str = String.uppercase_ascii str in
  let return newstr = (p, String newstr) in
  match str with
  | "__CLASS__" | "__TRAIT__"->
    inline_class_name_if_possible
      ~trait:(str = "__TRAIT__")
      ~fallback_to_empty_string:true
      env p pid
  | "__METHOD__" ->
    begin match env.scope with
    | ScopeItem.Method _ :: ScopeItem.Class { c_name = (_, n); _ } :: _
      when SU.Classes.is_anonymous_class_name n ->
      (* HHVM does not inline method name in anonymous classes *)
      p, Id (pid, (snd id))
    | _ ->
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
      else Core_list.drop_while env.scope ~f:(function
        | ScopeItem.Lambda | ScopeItem.LongLambda _ -> true
        | _ -> false) in
    begin match scope with
      | ScopeItem.Function fd :: _ -> return (prefix ^ strip_id fd.f_name)
      | ScopeItem.Method md :: _ -> return (prefix ^ strip_id md.m_name)
      | (ScopeItem.Lambda | ScopeItem.LongLambda _) :: _ ->
        return (prefix ^ "{closure}")
      (* PHP weirdness: __METHOD__ inside a class outside a method
       * returns class name *)
      | ScopeItem.Class cd :: _ -> return @@ strip_id cd.c_name
      | _ -> return ""
    end
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
    p, Int (string_of_int line)
  | _ ->
    (p, Id id)

let check_if_in_async_context { scope; pos; _ } =
  let check_valid_fun_kind (_, name) =
    function | FAsync | FAsyncGenerator -> ()
             | _ -> Emit_fatal.raise_fatal_parse pos @@
    "Function '"
    ^ (SU.strip_global_ns name)
    ^ "' contains 'await' but is not declared as async."
  in
  match scope with
  | [] -> Emit_fatal.raise_fatal_parse pos
            "'await' can only be used inside a function"
  | ScopeItem.Lambda :: _
  | ScopeItem.LongLambda _ :: _ ->
   (* TODO: In a lambda, we dont see whether there is a
    * async keyword in front or not >.> so assume this is fine, for now. *)
    ()
  | ScopeItem.Class _ :: _ -> () (* Syntax error, wont get here *)
  | ScopeItem.Function fd :: _ ->
    check_valid_fun_kind fd.f_name fd.f_fun_kind
  | ScopeItem.Method md :: _ ->
    check_valid_fun_kind md.m_name md.m_fun_kind

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
  | Call ((_, Id (pe, "get_class")), _, [], [])
    when st.namespace.Namespace_env.ns_name = None ->
    st, inline_class_name_if_possible
      ~trait:false
      ~fallback_to_empty_string:false
      env p pe
  | Call ((_, (Class_const ((_, Id (_, cid)), _)
             | Class_get ((_, Id (_, cid)), _))) as e,
    el1, el2, el3)
    when SU.is_parent cid ->
    let st = add_var env st "$this" in
    let st, e = convert_expr env st e in
    let st, el2 = convert_exprs env st el2 in
    let st, el3 = convert_exprs env st el3 in
    st, (p, Call(e, el1, el2, el3))
  | Call ((_, Id (_, id)), _, es, _)
    when String.lowercase_ascii id = "tuple" &&
      Emit_env.is_hh_syntax_enabled () ->
    convert_expr env st (p, Varray es)
  | Call (e, el1, el2, el3) ->
    let st =
      begin match snd e, el2 with
      | Id (_, s), [_, String arg] when
        String.lowercase_ascii @@ SU.strip_global_ns s = "hh\\asm"->
        set_inline_hhas st arg
      | _ -> st
      end in
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
  | NewAnonClass (args, varargs, cls) ->
    let cls = { cls with
      c_name = (fst cls.c_name, make_anonymous_class_name env st) } in
    let class_idx = total_class_count env st in
    let cls_condensed = { cls with
      c_name = (fst cls.c_name, string_of_int class_idx);
      c_body = [] } in
    let add_ns_to_hint = function
      | p, Happly (id, args) ->
        let classname, _ = Hhbc_id.Class.elaborate_id st.namespace id in
        p, Happly ((p, Hhbc_id.Class.to_raw_string classname), args)
      | other -> other in
    let elaborate_fun_param param = { param with
      param_hint = Option.map param.param_hint add_ns_to_hint } in
    let elaborate_method meth = { meth with
      m_params = List.map meth.m_params elaborate_fun_param;
      m_ret = Option.map meth.m_ret add_ns_to_hint } in
    let elaborate_class_elt = function
      | Method meth -> Method (elaborate_method meth)
      | other -> other in
    let cls = { cls with
      c_extends = List.map cls.c_extends add_ns_to_hint;
      c_implements = List.map cls.c_implements add_ns_to_hint;
      c_body = List.map cls.c_body elaborate_class_elt } in
    let prev_anonclass_depth = env.anonclass_depth in
    let env = { env with anonclass_depth = prev_anonclass_depth + 1 } in
    let num_hoisted_classes = List.length st.hoisted_classes in
    let st, cls = convert_class env st cls in
    let env = { env with anonclass_depth = prev_anonclass_depth } in
    let re_ordered_hoisted_classes =
      let new_classes, old_classes =
        List.split_n st.hoisted_classes
          (List.length st.hoisted_classes - num_hoisted_classes) in
      new_classes @ (cls :: old_classes)
    in
    let st = { st with
      anon_cls_cnt_per_fun = st.anon_cls_cnt_per_fun + 1;
      hoisted_classes = re_ordered_hoisted_classes } in
    let env = env_with_class env cls in
    let st, args = convert_exprs env st args in
    let st, varargs = convert_exprs env st varargs in
    st, (p, NewAnonClass (args, varargs, cls_condensed))
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
  | Dollar e ->
    let st, e = convert_expr env st e in
    st, (p, Dollar e)
  | Import(flavor, e) ->
    let st, e = convert_expr env st e in
    st, (p, Import(flavor, e))
  | Id (_, id) as ast_id when String_utils.string_starts_with id "$" ->
    let st = add_var env st id in
    st, (p, ast_id)
  | Id (_, var as id) ->
    (match get_let_var st var with
      | Some idx ->
        let lvar_name = transform_let_var_name var idx in
        let st = add_var env st lvar_name in
        st, (p, Lvar (p, lvar_name))
      | None -> st, convert_id env p id)
  | Class_get (cid, n) ->
    let st, e = convert_expr env st cid in
    let st, n = convert_expr env st n in
    st, (p, Class_get (e, n))
  | Class_const (cid, n) ->
    let st, e = convert_expr env st cid in
    st, (p, Class_const (e, n))
  | _ ->
    st, expr

and convert_prop_expr env st (_, expr_ as expr) =
  match expr_ with
  | Id (_, id) when not (String_utils.string_starts_with id "$") ->
    st, expr
  | _ ->
    convert_expr env st expr

(* Closure-convert a lambda expression, with use_vars_opt = Some vars
 * if there is an explicit `use` clause.
 *)
and convert_lambda env st p fd use_vars_opt =
  (* Remember the current capture and defined set across the lambda *)
  let captured_vars = st.captured_vars in
  let captured_this = st.captured_this in
  let static_vars = st.static_vars in
  let old_function_state = st.current_function_state in
  let st = enter_lambda st in
  let old_env = env in
  Option.iter use_vars_opt
    ~f:(List.iter ~f:(fun ((p, id), _) ->
      if id = SN.SpecialIdents.this
      then Emit_fatal.raise_fatal_parse p "Cannot use $this as lexical variable"));
  let env = append_let_vars env st.let_vars in
  let env = if Option.is_some use_vars_opt
            then env_with_longlambda env false fd
            else env_with_lambda env fd in
  let st, block, function_state = convert_function_like_body env st fd.f_body in
  let st = { st with closure_cnt_per_fun = st.closure_cnt_per_fun + 1 } in
  (* HHVM lists lambda vars in descending order - do the same *)
  let lambda_vars = List.sort ~cmp:(fun a b -> compare b a) @@ ULS.items st.captured_vars in
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
  let class_num = total_class_count env st in

  let rec is_scope_static scope =
    match scope with
    | ScopeItem.LongLambda is_static :: scope ->
      is_static || is_scope_static scope
    | ScopeItem.Function _ :: _ -> false
    | ScopeItem.Method md :: _ ->
      List.mem md.m_kind Static
    | ScopeItem.Lambda :: scope ->
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
      p env st lambda_vars tparams is_static fd block in
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
  let st = { st with captured_vars = captured_vars;
                     captured_this;
                     static_vars = static_vars;
                     explicit_use_set;
                     closure_enclosing_classes;
                     closure_namespaces = SMap.add
                       closure_class_name st.namespace st.closure_namespaces;
                     current_function_state = old_function_state;
           } in
  let st =
    record_function_state
      (Emit_env.get_unique_id_for_method cd md) function_state st in
  let env = old_env in
  (* Add lambda captured vars to current captured vars *)
  let st = List.fold_left lambda_vars ~init:st ~f:(add_var env) in
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
  | Static_var el ->
    let visit_static_var st e =
      begin match snd e with
      | Lvar (_, name)
      | Binop (Eq None, (_, Lvar (_, name)), _) -> add_static_var st name
      | _ -> failwith "Static var - impossible"
      end in
    let st = List.fold_left el ~init:st ~f:visit_static_var in
    let st, el = convert_exprs env st el in
    st, (p, Static_var el)
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
        let st, stub_cd = add_class env st cd in
        st, (p, Def_inline (Class stub_cd))
      | Fun fd :: _defs ->
        let st, fd = convert_fun env st fd in
        let has_inout_params =
          let wrapper, _ =
            Emit_inout_helpers.extract_function_inout_or_ref_param_locations fd in
          Option.is_some wrapper in
        let st, stub_fd =
          add_function ~has_inout_params:has_inout_params env st fd in
        st, (p, Def_inline (Fun stub_fd))
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
      (Emit_env.get_unique_id_for_function fd) function_state st in
  let st, f_params = convert_params env st fd.f_params in
  let st, f_user_attributes =
    convert_user_attributes env st fd.f_user_attributes in
  st, { fd with f_body; f_params; f_user_attributes }

and convert_class env st cd =
  let env = env_with_class env cd in
  let st = reset_function_counts st in
  let st, c_body = List.map_env st cd.c_body (convert_class_elt env) in
  let st, c_user_attributes =
    convert_user_attributes env st cd.c_user_attributes in
  st, { cd with c_body = c_body; c_user_attributes }

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
        (Emit_env.get_unique_id_for_method cls md) function_state st in
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

and convert_defs env class_count typedef_count st dl =
  match dl with
  | [] -> st, []
  | Fun fd :: dl ->
    let let_vars_copy = st.let_vars in
    let st = { st with let_vars = SMap.empty } in
    let st, fd = convert_fun env st fd in
    let st, dl = convert_defs env class_count typedef_count st dl in
    { st with let_vars = let_vars_copy }, (true, Fun fd) :: dl
    (* Convert a top-level class definition into a true class definition and
     * a stub class that just corresponds to the DefCls instruction *)
  | Class cd :: dl ->
    let let_vars_copy = st.let_vars in
    let st = { st with let_vars = SMap.empty } in
    let st, cd = convert_class env st cd in
    let stub_class = make_defcls cd class_count in
    let st, dl = convert_defs env (class_count + 1) typedef_count st dl in
    { st with let_vars = let_vars_copy },
      (true, Class cd) :: (true, Stmt (Pos.none, Def_inline (Class stub_class))) :: dl
  | Stmt stmt :: dl ->
    let st, stmt = convert_stmt env st stmt in
    let st, dl = convert_defs env class_count typedef_count st dl in
    st, (true, Stmt stmt) :: dl
  | Typedef td :: dl ->
    let st, dl = convert_defs env class_count (typedef_count + 1) st dl in
    let st, t_user_attributes =
      convert_user_attributes env st td.t_user_attributes in
    let td = { td with t_user_attributes } in
    let stub_td = { td with t_id =
      (fst td.t_id, string_of_int (typedef_count)) } in
    st, (true, Typedef td) :: (true, Stmt (Pos.none, Def_inline (Typedef stub_td))) :: dl
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
    List.partition_tf all_defs ~f:(function _, Fun _ -> true | _ -> false) in
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
  (* First compute the number of explicit classes in order to generate correct
   * integer identifiers for the generated classes. .main counts as a top-level
   * function and we place hoisted functions just after that *)
  let env = env_toplevel (count_classes defs) 1 defs in
  let st, original_defs = convert_defs env 0 0 initial_state defs in
  let main_state = st.current_function_state in
  let st =
    record_function_state (Emit_env.get_unique_id_for_main ()) main_state st in
  (* Reorder the functions so that they appear first. This matches the
   * behaviour of HHVM. *)
  let original_defs = hoist_toplevel_functions original_defs in
  let fun_defs = List.rev_map st.hoisted_functions (fun fd -> false, Fun fd) in
  let class_defs = List.rev_map st.hoisted_classes (fun cd -> false, Class cd) in
  let ast_defs = fun_defs @ original_defs @ class_defs in
  let global_state =
    Emit_env.(
      { global_explicit_use_set = st.explicit_use_set
      ; global_closure_namespaces = st.closure_namespaces
      ; global_closure_enclosing_classes = st.closure_enclosing_classes
      ; global_functions_with_finally = st.functions_with_finally
      ; global_function_to_labels_map = st.function_to_labels_map
      ; global_functions_with_hhas_blocks = st.functions_with_hhas_blocks }) in
  {
    ast_defs;
    global_state;
    strict_types = st.seen_strict_types;
  }
