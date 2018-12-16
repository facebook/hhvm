(**
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)
open Ast
module Aast = Nast
module SN = Naming_special_names

type class_body =
{
  c_uses           : Aast.hint list                       ;
  c_method_redeclarations : Aast.method_redeclaration list;
  c_xhp_attr_uses  : Aast.hint list                       ;
  c_xhp_category   : pstring list                         ;
  c_req_extends    : Aast.hint list                       ;
  c_req_implements : Aast.hint list                       ;
  c_consts         : Aast.class_const list                ;
  c_typeconsts     : Aast.class_typeconst list            ;
  c_static_vars    : Aast.static_var list                 ;
  c_vars           : Aast.class_var list                  ;
  c_constructor    : Aast.constructor option              ;
  c_static_methods : Aast.static_method list              ;
  c_methods        : Aast.method_ list                    ;
  c_attributes     : Aast.class_attr list                 ;
  c_xhp_children   : (pos * Aast.xhp_child) list          ;
  c_xhp_attrs      : Aast.xhp_attr list                   ;
}

let make_empty_class_body = {
  c_uses = [];
  c_method_redeclarations = [];
  c_xhp_attr_uses = [];
  c_xhp_category = [];
  c_req_extends = [];
  c_req_implements = [];
  c_consts = [];
  c_typeconsts = [];
  c_static_vars = [];
  c_vars = [];
  c_constructor = None;
  c_static_methods = [];
  c_methods = [];
  c_attributes = [];
  c_xhp_children = [];
  c_xhp_attrs = [];
}

let on_list f l = List.map f l

let optional f = function
  | None -> None
  | Some x -> Some (f x)

let rec on_variadic_hint h =
  match h with
  | Hvariadic h -> Aast.Hvariadic (optional on_hint h)
  | Hnon_variadic -> Aast.Hnon_variadic

and on_shape_info info =
  Aast.{
    nsi_allows_unknown_fields = info.si_allows_unknown_fields;
    nsi_field_map = ShapeMap.empty; (* TODO T37786581: Fill this out properly *)
  }

and on_haccess (pos, root_id) id ids =
  let root_ty = Aast.Happly ((pos, root_id), []) in
  Aast.Haccess ((pos, root_ty), id :: ids)

and on_hint (p, h) : Aast.hint =
  match h with
  | Hoption h -> (p, Aast.Hoption (on_hint h))
  | Hfun (is_coroutine, hl, param_kinds, variadic, h) ->
    (p, Aast.Hfun
      (Aast.FNonreactive,
       is_coroutine,
       (on_list on_hint hl),
       param_kinds,
       [],
       (on_variadic_hint variadic),
       (on_hint h),
       true
      )
    )
  | Htuple (hl) -> (p, Aast.Htuple (on_list on_hint hl))
  | Happly (x, hl) -> (p, Aast.Happly (x, on_list on_hint hl))
  | Hshape s -> (p, Aast.Hshape (on_shape_info s))
  | Haccess (root, id, ids) -> (p, on_haccess root id ids)
  | Hsoft _ -> (p, Aast.Hany)  (* TODO: T37786581 *)
  | Hreified _ -> (p, Aast.Hany) (* TODO: T37786581 *)

and on_class_elt body elt : class_body =
  match elt with
  | Attributes attrs ->
    let attrs = body.c_attributes @ (on_list on_class_attr attrs) in
    { body with c_attributes = attrs; }
  | Const (hopt, el) ->
    let hopt = optional on_hint hopt in
    let consts = List.map (fun (id, e) ->
      (hopt, id, Some (on_expr e))
    ) el in
    let consts = body.c_consts @ consts in
    { body with c_consts = consts; }
  | AbsConst (hopt, id) ->
    let consts = body.c_consts @ [(optional on_hint hopt, id, None)] in
    { body with c_consts = consts; }
  | ClassUse h  ->
    let hints = body.c_uses @ [on_hint h] in
    { body with c_uses = hints; }
  | ClassUseAlias (_, (p, _), _, _) ->
    Errors.unsupported_feature p "Trait use aliasing"; body
  | ClassUsePrecedence (_, (p, _), _) ->
    Errors.unsupported_feature p "The insteadof keyword"; body
  | MethodTraitResolution res ->
    let redecls = body.c_method_redeclarations @ [on_method_trait_resolution res] in
    { body with c_method_redeclarations = redecls; }
  | XhpAttrUse h ->
    let hints = body.c_xhp_attr_uses @ [on_hint h] in
    { body with c_xhp_attr_uses = hints }
  | ClassTraitRequire (MustExtend, h) ->
    let hints = body.c_req_extends @ [on_hint h] in
    { body with c_req_extends = hints; }
  | ClassTraitRequire (MustImplement, h) ->
    let hints = body.c_req_implements @ [on_hint h] in
    { body with c_req_implements = hints; }
  | ClassVars cv when List.mem Static cv.cv_kinds ->
    let attrs = on_list on_user_attribute cv.cv_user_attributes in
    let vars = body.c_static_vars @
      (on_list (on_class_var false (optional on_hint cv.cv_hint) attrs) cv.cv_names) in
    { body with c_static_vars = vars; }
  | ClassVars { cv_names; cv_user_attributes; cv_hint; _ } ->
    let attrs = on_list on_user_attribute cv_user_attributes in
    let vars =
      body.c_vars @ (on_list (on_class_var false (optional on_hint cv_hint) attrs) cv_names) in
    { body with c_vars = vars; }
  | XhpAttr (hopt, var, is_required, maybe_enum)  ->
    (* TODO: T37984688 Updating naming.ml to use c_xhp_attrs *)
    let hopt = optional on_hint hopt in
    let attrs = body.c_xhp_attrs @
      [(hopt,
        on_class_var true hopt [] var,
        is_required,
        optional on_xhp_attr maybe_enum)] in
    { body with c_xhp_attrs = attrs; }
  | XhpCategory (_, cs) ->
    { body with c_xhp_category = cs; }
  | XhpChild (p, c) ->
    let children = body.c_xhp_children @ [(p, on_xhp_child c)] in
    { body with c_xhp_children = children; }
  | Method m when snd m.m_name = SN.Members.__construct ->
    { body with c_constructor = Some (on_method m) }
  | Method m when List.mem Static m.m_kind ->
    let statics = body.c_static_methods @ [on_method m] in
    { body with c_static_methods = statics; }
  | Method m ->
    let methods = body.c_methods @ [on_method m] in
    { body with c_methods = methods; }
  | TypeConst tc ->
    let typeconsts = body.c_typeconsts @ [on_class_typeconst tc] in
    { body with c_typeconsts = typeconsts; }

and on_as_expr e : Aast.as_expr =
  match e with
  | As_v e -> Aast.As_v (on_expr e)
  | As_kv (e1, e2) -> Aast.As_kv (on_expr e1, on_expr e2)

and on_afield f : Aast.afield =
  match f with
  | AFvalue e -> Aast.AFvalue (on_expr e)
  | AFkvalue (e1, e2) -> Aast.AFkvalue (on_expr e1, on_expr e2)

and on_darray_element (e1, e2) =
  (on_expr e1, on_expr e2)


and on_xhp_attribute a : Aast.xhp_attribute =
  match a with
  | Xhp_simple (id, e) -> Aast.Xhp_simple (id, on_expr e)
  | Xhp_spread e -> Aast.Xhp_spread (on_expr e)

and on_targ (h, r) : Aast.targ = (on_hint h, r)

and on_expr (p, e) : Aast.expr =
  let node = match e with
  | Array al -> Aast.Array (on_list on_afield al)
  | Varray el -> Aast.Varray (on_list on_expr el)
  | Darray d -> Aast.Darray (on_list on_darray_element d)
  | Shape _ -> Aast.Any (* TODO: T37786581 *)
  | Collection _ -> Aast.Any (* TODO: T37786581 *)
  | Null -> Aast.Null
  | True -> Aast.True
  | False -> Aast.False
  | Omitted -> Aast.Omitted
  | Id id -> Aast.Id id
  | Lvar id ->
    let lid = Local_id.make_unscoped (snd id) in
    Aast.Lvar (p, lid)
  | Dollar e -> Aast.Dollar (on_expr e)
  | Clone e -> Aast.Clone (on_expr e)
  | Obj_get (e1, e2, f) -> Aast.Obj_get (on_expr e1, on_expr e2, f)
  | Array_get (e, opt_e) -> Aast.Array_get (on_expr e, optional on_expr opt_e)
  | Class_get _ -> Aast.Any (* TODO: T37786581 *)
  | Class_const _ -> Aast.Any (* TODO: T37786581 *)
  | Call (e, tl, el, uel) ->
    Aast.Call (Aast.Cnormal, on_expr e, on_list on_targ tl, on_list on_expr el, on_list on_expr uel)
  | Int s -> Aast.Int s
  | Float s -> Aast.Float s
  | String s -> Aast.String s
  | String2 el -> Aast.String2 (on_list on_expr el)
  | PrefixedString (s, e) -> Aast.PrefixedString (s, on_expr e)
  | Yield f -> Aast.Yield (on_afield f)
  | Yield_break -> Aast.Yield_break
  | Yield_from e -> Aast.Yield_from (on_expr e)
  | Await e -> Aast.Await (on_expr e)
  | Suspend e -> Aast.Suspend (on_expr e)
  | List el -> Aast.List (on_list on_expr el)
  | Expr_list el -> Aast.Expr_list (on_list on_expr el)
  | Cast (h, e) -> Aast.Cast (on_hint h, on_expr e)
  | Unop (op, e) -> Aast.Unop (op, on_expr e)
  | Binop (op, e1, e2) -> Aast.Binop (op, on_expr e1, on_expr e2)
  | Pipe (e1, e2) ->
    let id = Local_id.make_scoped SN.SpecialIdents.dollardollar in
    Aast.Pipe ((p, id), on_expr e1, on_expr e2)
  | Eif (e1, opt_e, e2) -> Aast.Eif (on_expr e1, optional on_expr opt_e, on_expr e2)
  | InstanceOf _ -> Aast.Any
  | Is (e, h) -> Aast.Is (on_expr e, on_hint h)
  | As (e, h, b) -> Aast.As (on_expr e, on_hint h, b)
  | BracedExpr _ -> Aast.Any (* TODO: T37786581 *)
  | ParenthesizedExpr _ -> Aast.Any (* TODO: T37786581 *)
  | New _ -> Aast.Any (* TODO: T37786581 *)
  | NewAnonClass _ -> Aast.Any (* TODO: T37786581 *)
  | Efun _ -> Aast.Any (* TODO: T37786581 *)
  | Lfun _ -> Aast.Any (* TODO: T37786581 *)
  | Xml (id, xhpl, el) -> Aast.Xml (id, on_list on_xhp_attribute xhpl, on_list on_expr el)
  | Unsafeexpr e -> Aast.Unsafe_expr (on_expr e)
  | Import _ -> Aast.Any (* TODO: T37786581 *)
  | Callconv (k, e) -> Aast.Callconv (k, on_expr e)
  | Execution_operator el -> Aast.Execution_operator (on_list on_expr el)
  in
  (p, node)

and on_case c : Aast.case =
  match c with
  | Default b -> Aast.Default (on_block b)
  | Case (e, b) -> Aast.Case (on_expr e, on_block b)

and on_catch (id1, id2, b) : Aast.catch =
  let lid = Local_id.make_unscoped (snd id2) in
  (id1, ((fst id2), lid), on_block b)

and on_stmt (p, st) :  Aast.stmt =
  match st with
  | Let (id, h, e) ->
    let lid = Local_id.make_unscoped (snd id) in
    Aast.Let ((p, lid), optional on_hint h, on_expr e)
  | Block sl                  -> Aast.Block (on_block sl)
  | Unsafe                    -> failwith "Unsafe statements should be removed in on_block"
  | Fallthrough               -> Aast.Fallthrough
  | Noop                      -> Aast.Noop
  | Markup (s, e)             -> Aast.Markup (s, optional on_expr e)
  | Break (Some _)            -> failwith "Breaks with labels are not allowed in Hack"
  | Break None                -> Aast.Break p
  | Continue (Some _)         -> failwith "Continues with labels are not allowed in Hack"
  | Continue None             -> Aast.Continue p
  | Throw e                   -> Aast.Throw (false, on_expr e)
  | Return e                  -> Aast.Return (p, optional on_expr e)
  | GotoLabel label           -> Aast.GotoLabel label
  | Goto label                -> Aast.Goto label
  | Static_var el             -> Aast.Static_var (on_list on_expr el)
  | Global_var el             -> Aast.Global_var (on_list on_expr el)
  | Awaitall _                -> Aast.Noop (* TODO: T37786581 *)
  | If (e, b1, b2)            -> Aast.If (on_expr e, on_block b1, on_block b2)
  | Do (b, e)                 -> Aast.Do (on_block b, on_expr e)
  | While (e, b)              -> Aast.While (on_expr e, on_block b)
  | Declare (is_blk, e, b)    -> Aast.Declare (is_blk, on_expr e, on_block b)
  | Using s ->
    Aast.Using Aast.{
      us_expr = on_expr s.us_expr;
      us_block = on_block s.us_block;
      us_has_await = s.us_has_await;
      us_is_block_scoped = s.us_is_block_scoped;
   }
  | For (st1, e, st2, b)      -> Aast.For (on_expr st1, on_expr e, on_expr st2, on_block b)
  | Switch (e, cl)            -> Aast.Switch (on_expr e, on_list on_case cl)
  | Foreach (e, _, ae, b)     -> Aast.Foreach (on_expr e, on_as_expr ae, on_block b)
  | Try (b, cl, fb)           -> Aast.Try (on_block b, on_list on_catch cl, on_block fb)
  | Def_inline d              -> Aast.Def_inline (on_def d)
  | Expr e                    -> Aast.Expr (on_expr e)

and on_block stmt_list : Aast.stmt list =
  match stmt_list with
  | [] -> []
  | (_, Unsafe) :: rest -> [Aast.Unsafe_block (on_block rest)]
  | x :: rest -> (on_stmt x) :: (on_block rest)

and on_tparam_constraint (kind, hint) : (constraint_kind * Aast.hint) =
  (kind, on_hint hint)

and on_tparam (variance, id, constraint_list, reified) : Aast.tparam =
  let constraint_list = on_list on_tparam_constraint constraint_list in
  (variance, id, constraint_list, reified)

and on_fun_param param : Aast.fun_param =
  let p, name = param.param_id in
  { Aast.param_annotation = p;
    param_hint = optional on_hint param.param_hint;
    param_is_reference = param.param_is_reference;
    param_is_variadic = param.param_is_variadic;
    param_pos = p;
    param_name = name;
    param_expr = optional on_expr param.param_expr;
    param_callconv = param.param_callconv;
    param_user_attributes = on_list on_user_attribute param.param_user_attributes;
  }

and on_user_attribute attribute : Aast.user_attribute =
  let ua_params = on_list on_expr attribute.ua_params in
  Aast.{ ua_name = attribute.ua_name; ua_params; }

and on_fun f : Aast.fun_ =
  let body = on_block f.f_body in
  let body = Aast.NamedBody {
        Aast.fnb_nast = body;
        fnb_unsafe = true;
  }
  in
  let named_fun = {
    Aast.f_annotation = ();
    f_span = f.f_span;
    f_mode = f.f_mode;
    f_ret = (optional on_hint f.f_ret);
    f_name = f.f_name;
    f_tparams = on_list on_tparam f.f_tparams;
    f_where_constraints = []; (* TODO: T37786581 *)
    f_params = on_list on_fun_param f.f_params;
    f_body = body;
    f_fun_kind = f.f_fun_kind;
    f_variadic = Aast.FVnonVariadic; (* TODO: T37786581 *)
    f_user_attributes = on_list on_user_attribute f.f_user_attributes;
    f_ret_by_ref = f.f_ret_by_ref;
    f_external = f.f_external;
    (* TODO: T37786581l; missing f_doc_comment and perhaps other fields *)
  } in
  named_fun

and on_enum (e : enum_) : Aast.enum_ =
  Aast.{
    e_base = on_hint e.e_base;
    e_constraint = optional on_hint e.e_constraint
  }

and on_class_attr attr : Aast.class_attr =
  match attr with
  | CA_name id -> Aast.CA_name id
  | CA_field f -> Aast.CA_field Aast.{
    ca_type = on_ca_type f.ca_type;
    ca_id = f.ca_id;
    ca_value = optional on_expr f.ca_value;
    ca_required = f.ca_required;
  }

and on_ca_type ty : Aast.ca_type =
  match ty with
  | CA_hint h -> Aast.CA_hint (on_hint h)
  | CA_enum sl -> Aast.CA_enum (sl)

and on_class_typeconst (tc: Ast.typeconst) : Aast.class_typeconst =
  Aast.{
    c_tconst_name = tc.tconst_name;
    c_tconst_constraint = optional on_hint tc.tconst_constraint;
    c_tconst_type = optional on_hint tc.tconst_type;
  }

and on_class_var is_xhp h attrs (_, id, eopt) : Aast.class_var =
  Aast.{
    cv_final = false; (* TODO: T37786581 *)
    cv_is_xhp = is_xhp;
    cv_visibility = Public; (* TODO: T37786581 *)
    cv_type = h;
    cv_id = id;
    cv_expr = optional on_expr eopt;
    cv_user_attributes = attrs;
  }

and on_xhp_attr (p, b, el) = (p, b, on_list on_expr el)

and on_constr (h1, k, h2) = (on_hint h1, k, on_hint h2)

and on_method_trait_resolution res : Aast.method_redeclaration =
  let acc = false, false, false, Aast.Public in
  let final, abs, static, vis = List.fold_left kind acc res.mt_kind in
  Aast.{
    mt_final           = final;
    mt_abstract        = abs;
    mt_static          = static;
    mt_visibility      = vis;
    mt_name            = res.mt_name;
    mt_tparams         = on_list on_tparam res.mt_tparams;
    mt_where_constraints = on_list on_constr res.mt_constrs;
    mt_variadic        = Aast.FVnonVariadic; (* TODO: T37786581 Set correct variadicity *)
    mt_params          = on_list on_fun_param res.mt_params;
    mt_fun_kind        = res.mt_fun_kind;
    mt_ret             = optional on_hint res.mt_ret;
    mt_ret_by_ref      = res.mt_ret_by_ref;
    mt_trait           = on_hint res.mt_trait;
    mt_method          = res.mt_method;
    mt_user_attributes = on_list on_user_attribute res.mt_user_attributes;
  }

and on_xhp_child c : Aast.xhp_child =
  match c with
  | ChildName id -> Aast.ChildName id
  | ChildList cl -> Aast.ChildList (on_list on_xhp_child cl)
  | ChildUnary (c, op) -> Aast.ChildUnary (on_xhp_child c, on_xhp_child_op op)
  | ChildBinary (c1, c2) -> Aast.ChildBinary (on_xhp_child c1, on_xhp_child c2)

and on_xhp_child_op op : Aast.xhp_child_op =
  match op with
  | ChildStar -> Aast.ChildStar
  | ChildPlus -> Aast.ChildPlus
  | ChildQuestion -> Aast.ChildQuestion

and kind (final, abs, static, vis) = function
  | Final -> true, abs, static, vis
  | Static -> final, abs, true, vis
  | Abstract -> final, true, static, vis
  | Private -> final, abs, static, Aast.Private
  | Public -> final, abs, static, Aast.Public
  | Protected -> final, abs, static, Aast.Protected

and on_method m : Aast.method_ =
  let body = on_block m.m_body in
  let body = Aast.NamedBody {
        Aast.fnb_nast = body;
        fnb_unsafe = true;
  } in
  let acc = false, false, false, Aast.Public in
  let final, abs, static, vis = List.fold_left kind acc m.m_kind in
  Aast.{
    m_span            = m.m_span;
    m_annotation      = ();
    m_final           = final;
    m_abstract        = abs;
    m_static          = static;
    m_visibility      = vis;
    m_name            = m.m_name;
    m_tparams         = on_list on_tparam m.m_tparams;
    m_where_constraints = on_list on_constr m.m_constrs;
    m_variadic        = Aast.FVnonVariadic; (* TODO: Set correct variadicity *)
    m_params          = on_list on_fun_param m.m_params;
    m_body            = body;
    m_fun_kind        = m.m_fun_kind;
    m_user_attributes = on_list on_user_attribute m.m_user_attributes;
    m_ret             = optional on_hint m.m_ret;
    m_ret_by_ref      = m.m_ret_by_ref;
    m_external        = m.m_external;
    (* TODO: m_doc_comment *)
  }

and on_class c : Aast.class_ =
  let tparams = on_list on_tparam c.c_tparams in
  let body = List.fold_left on_class_elt make_empty_class_body c.c_body in
  let named_class =
  Aast.{
      c_annotation            = ();
      c_span                  = c.c_span;
      c_mode                  = c.c_mode;
      c_final                 = c.c_final;
      c_is_xhp                = c.c_is_xhp;
      c_kind                  = c.c_kind;
      c_name                  = c.c_name;
      c_tparams               = (tparams, SMap.empty);
      c_extends               = on_list on_hint c.c_extends;
      c_uses                  = body.c_uses;
      c_method_redeclarations = body.c_method_redeclarations;
      c_xhp_attr_uses         = body.c_xhp_attr_uses;
      c_xhp_category          = body.c_xhp_category;
      c_req_extends           = body.c_req_extends;
      c_req_implements        = body.c_req_implements;
      c_implements            = on_list on_hint c.c_implements;
      c_consts                = body.c_consts;
      c_typeconsts            = body.c_typeconsts;
      c_static_vars           = body.c_static_vars;
      c_vars                  = body.c_vars;
      c_constructor           = body.c_constructor;
      c_static_methods        = body.c_static_methods;
      c_methods               = body.c_methods;
      c_attributes            = body.c_attributes;
      c_xhp_children          = body.c_xhp_children;
      c_xhp_attrs             = body.c_xhp_attrs;
      c_user_attributes       = on_list on_user_attribute c.c_user_attributes;
      c_namespace             = c.c_namespace;
      c_enum                  = optional on_enum c.c_enum;
      c_doc_comment           = c.c_doc_comment;
    }
  in
  named_class

and on_typedef t : Aast.typedef =
  let t_vis = match t.t_kind with
    | Alias _ -> Aast.Transparent
    | NewType _ -> Aast.Opaque
  in
  let t_kind = match t.t_kind with
    | Alias h -> on_hint h
    | NewType h -> on_hint h
  in
  Aast.{
    t_annotation = ();
    t_name = t.t_id;
    t_tparams = on_list on_tparam t.t_tparams;
    t_constraint = optional on_hint t.t_constraint;
    t_kind = t_kind;
    t_user_attributes = on_list on_user_attribute t.t_user_attributes;
    t_mode = t.t_mode;
    t_vis = t_vis;
    t_namespace = t.t_namespace;
  }

and on_constant (c : gconst) : Aast.gconst =
  Aast.{
    cst_annotation = ();
    cst_mode = c.cst_mode;
    cst_name = c.cst_name;
    cst_type = optional on_hint c.cst_type;
    cst_value = Some (on_expr c.cst_value);
    cst_is_define = (c.cst_kind = Cst_define);
    cst_namespace = c.cst_namespace;
  }

and on_ns_use (k, id1, id2): (Aast.ns_kind * Aast.sid * Aast.sid) =
  let kind = match k with
  | NSNamespace -> Aast.NSNamespace
  | NSClass -> Aast.NSClass
  | NSClassAndNamespace -> Aast.NSClassAndNamespace
  | NSFun -> Aast.NSFun
  | NSConst -> Aast.NSConst
  in
  (kind, id1, id2)

and on_def : def -> Aast.def = function
  | Fun f -> Aast.Fun (on_fun f)
  | Class c -> Aast.Class (on_class c)
  | Stmt s -> Aast.Stmt (on_stmt s)
  | Typedef t -> Aast.Typedef (on_typedef t)
  | Constant c -> Aast.Constant (on_constant c)
  | Namespace (id, p) -> Aast.Namespace (id, on_program p)
  | NamespaceUse usel -> Aast.NamespaceUse (on_list on_ns_use usel)
  | SetNamespaceEnv env -> Aast.SetNamespaceEnv env

and on_program ast = on_list on_def ast

let convert ast : Aast.program = on_program ast
