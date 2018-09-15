(**
 * Copyright (c) 2017, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
*)
open Core_kernel

module ULS = Unique_list_string
module SN = Naming_special_names

type bare_this_usage =
  (* $this appear as expression *)
  | Bare_this
  (* bare $this appear as possible targetfor assignment
  - $this as function argument
  - $this as target of & operator
  - $this as reference in catch clause *)
  | Bare_this_as_ref

type decl_vars_state = {
  (* set of locals used inside the functions *)
  dvs_locals: ULS.t;
  (* does function use anything that might access local dynamically:
  - special function
  - '$' operator *)
  dvs_use_dynamic_var_access: bool;
  (* does function uses bare form of $this *)
  dvs_bare_this: bare_this_usage option
}

(* List of functions from SimpleFunctionCall::InitFunctionTypeMap()
   with kinds: Extract, Assert, Compact, GetDefinedVars.
   When function contains calls to functions from the list its is considered
   to be having dynamic vars *)
let dynamic_functions = SSet.of_list @@ [
  "extract";
  "parse_str";
  "compact";
  "assert";
  "get_defined_vars"
]

let with_dynamic_var_access s =
  if s.dvs_use_dynamic_var_access then s
  else { s with dvs_use_dynamic_var_access = true }

let with_local name s =
  { s with dvs_locals = ULS.add s.dvs_locals name }

let with_this barethis s =
  let new_bare_this =
    match s.dvs_bare_this, barethis with
    | _, Bare_this_as_ref -> Some Bare_this_as_ref
    | None, Bare_this     -> Some Bare_this
    | u, _ -> u in
  if s.dvs_bare_this = new_bare_this then s
  else
  { s with
      dvs_bare_this = new_bare_this;
      dvs_locals = ULS.add s.dvs_locals SN.SpecialIdents.this }

let dvs_empty = {
  dvs_locals = ULS.empty;
  dvs_use_dynamic_var_access = false;
  dvs_bare_this = None; }

(* Add a local to the accumulated list. Don't add if it's $GLOBALS or
 * the pipe variable $$. If it's $this, add it, and if this variable appears
 * "bare" (because bareparam=true), remember for needs_local_this *)
let add_local ~barethis s (_, name) =
  if name = SN.Superglobals.globals || name = SN.SpecialIdents.dollardollar
  then s
  else if name = SN.SpecialIdents.this
  then with_this barethis s
  else with_local name s

(* Add locals for an expression for which $this counts as "bare_ref" *)
let add_bare_expr this acc expr =
  match expr with
  | (_, Ast.Lvar(_, "$this")) ->
    with_this Bare_this_as_ref acc
  | _ ->
    this#on_expr acc expr

let add_bare_exprs this acc exprs =
  List.fold_left exprs ~f:(add_bare_expr this) ~init:acc

let on_class_get this acc recv prop ~is_call_target =
  let acc = this#on_expr acc recv in
  (* Distinguish between cases
  - A::$b() - $b is a local variable
  - A::$b = 1 - $b is a static field name *)
  match snd prop with
  | Ast.Lvar pid ->
    if is_call_target then add_local ~barethis:Bare_this acc pid
    else acc
  | _ -> this#on_expr acc prop

class declvar_visitor explicit_use_set_opt is_in_static_method is_closure_body
  = object(this)
  inherit [decl_vars_state] Ast_visitor.ast_visitor as super

  method! on_global_var acc exprs =
    let rec add_local_from_expr acc e =
      match snd e with
      | (Ast.Id id | Ast.Lvar id) -> add_local ~barethis:Bare_this acc id
      | Ast.Dollar e -> add_local_from_expr acc e
      | _ -> this#on_expr acc e in
    List.fold_left exprs ~init:acc ~f:add_local_from_expr

  method! on_obj_get acc e prop =
    let acc = match snd e with
    | Ast.Lvar (_, ("$this" as id)) ->
      if is_in_static_method && not is_closure_body then acc
      else with_local id acc
    | _ -> this#on_expr acc e in
    match snd prop with
    (* Only add if it is a variable *)
    | Ast.Lvar id -> add_local ~barethis:Bare_this acc id
    | _ -> this#on_expr acc prop

  method! on_foreach acc e pos iterator block =
    let acc =
      match snd e with
      | Ast.Lvar(_, "$this") when Iterator.is_mutable_iterator iterator ->
        with_this Bare_this_as_ref acc
      | _ ->
        acc
    in
    super#on_foreach acc e pos iterator block

  method! on_unop acc unop expr =
    match unop with
    | Ast.Uref -> add_bare_expr this acc expr
    | _ -> super#on_unop acc unop expr

  method! on_binop acc binop e1 e2 =
    match binop, e2 with
    | (Ast.Eq _, (_, Ast.Await _))
    | (Ast.Eq _, (_, Ast.Yield _))
    | (Ast.Eq _, (_, Ast.Yield_from _)) ->
      let acc = this#on_expr acc e2 in this#on_expr acc e1
    | _ -> super#on_binop acc binop e1 e2

  method! on_lvar acc id =
    add_local ~barethis:Bare_this acc id

  method! on_dollar acc e =
    match e with
    | _, Ast.Lvar id -> with_dynamic_var_access (add_local ~barethis:Bare_this acc id)
    | _ -> this#on_expr acc e

  method! on_class_get acc id prop =
    on_class_get this acc id prop ~is_call_target:false

  method! on_efun acc fn use_list =
  (* at this point AST is already rewritten so use lists on EFun nodes
    contain list of captured variables. However if use list was initially absent
    it is not correct to traverse such nodes to collect locals because it will impact
    the order of locals in generated .declvars section:
    // .declvars $a, $c, $b
    $a = () => { $b = 1 };
    $c = 1;
    $b = 2;
    // .declvars $a, $b, $c
    $a = function () use ($b) => { $b = 1 };
    $c = 1;
    $b = 2;

    'explicit_use_set' is used to in order to avoid synthesized use list *)
    let fn_name = snd fn.Ast.f_name in
    let has_use_list =
      Option.value_map explicit_use_set_opt
        ~default:false ~f:(fun s -> SSet.mem fn_name s) in
    if has_use_list
    then List.fold_left use_list ~init:acc
      ~f:(fun acc (x, _isref) -> add_local ~barethis:Bare_this acc x)
    else acc
  method! on_class_const acc e _ = this#on_expr acc e
  method! on_call acc e _ el1 el2 =
    let acc =
      match e with
      | (_, Ast.Id(p, "HH\\set_frame_metadata"))
      | (_, Ast.Id(p, "\\HH\\set_frame_metadata")) ->
        add_local ~barethis:Bare_this acc (p,"$86metadata")
      | _ -> acc in
    let acc =
      match e with
      | (_, Ast.Id (_, s))
        when SSet.mem (String.lowercase s) dynamic_functions ->
        with_dynamic_var_access acc
      | _ -> acc in
    let barethis =
      match e with
      | (_, Ast.Id(_, ("isset" | "echo" | "empty"))) -> Bare_this
      | _ -> Bare_this_as_ref in
    let on_arg acc e =
      match e with
      (* Only add $this to locals if it's bare *)
      | (_, Ast.Lvar(_, "$this")) ->
        with_this barethis acc
      | (_, Ast.(Unop (Uref, (_, Lvar id)))) ->
        add_local ~barethis:Bare_this acc id
      | _ ->
        this#on_expr acc e
    in
    let acc =
      match snd e with
      | Ast.Class_get (id, prop) ->
        on_class_get this acc id prop ~is_call_target:true
      | _ -> this#on_expr acc e
    in
    let acc = List.fold_left el1 ~f:on_arg ~init:acc in
    let acc = List.fold_left el2 ~f:on_arg ~init:acc in
    acc

  method! on_new acc expr _hl exprs1 exprs2 =
    let acc = this#on_expr acc expr in
    let acc = add_bare_exprs this acc exprs1 in
    let acc = add_bare_exprs this acc exprs2 in
    acc

  method! on_catch acc (_, x, b) =
    this#on_block (add_local ~barethis:Bare_this_as_ref acc x) b
  method! on_class_ acc _ = acc
  method! on_fun_ acc _ = acc
end

let uls_from_ast ~is_closure_body ~has_this
  ~params ~is_toplevel ~is_in_static_method
  ~get_param_name ~get_param_default_value
  ~explicit_use_set_opt b =
  let visitor =
    new declvar_visitor explicit_use_set_opt is_in_static_method is_closure_body in
  let state =
    (* pull variables used in default values *)
    let acc = List.fold_left params ~init:dvs_empty ~f:(
      fun acc p -> Option.fold (get_param_default_value p) ~init:acc ~f:visitor#on_expr)
    in
    visitor#on_program acc b in
  let needs_local_this =
    state.dvs_bare_this = Some Bare_this_as_ref ||
    (state.dvs_bare_this = Some Bare_this && state.dvs_use_dynamic_var_access) ||
    is_in_static_method
  in
  let param_names =
    List.fold_left
      params
        ~init:ULS.empty
        ~f:(fun l p -> ULS.add l @@ get_param_name p)
  in
  let decl_vars = ULS.diff state.dvs_locals param_names in
  let decl_vars =
    if needs_local_this || is_closure_body || not has_this || is_toplevel
    then decl_vars
    else ULS.remove "$this" decl_vars in
  needs_local_this && has_this, decl_vars

(* See decl_vars.mli for details *)
let from_ast
  ~is_closure_body ~has_this ~params ~is_toplevel
  ~is_in_static_method ~explicit_use_set b =
  let needs_local_this, decl_vars =
    uls_from_ast
      ~is_closure_body
      ~has_this
      ~params
      ~is_toplevel
      ~is_in_static_method
      ~get_param_name:Hhas_param.name
      ~get_param_default_value:(fun p -> Option.map (Hhas_param.default_value p) ~f:snd)
      ~explicit_use_set_opt:(Some explicit_use_set)
      b in
  needs_local_this, ULS.items decl_vars

let vars_from_ast
  ~is_closure_body ~has_this ~params ~is_toplevel ~is_in_static_method b =
  let _, decl_vars =
    uls_from_ast
      ~is_closure_body
      ~has_this
      ~params
      ~is_toplevel
      ~is_in_static_method
      ~get_param_name:(fun p -> snd p.Ast.param_id)
      ~get_param_default_value:(fun p -> p.Ast.param_expr)
      ~explicit_use_set_opt:None
      b in
  ULS.items_set decl_vars
