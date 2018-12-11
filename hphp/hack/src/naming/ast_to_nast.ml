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
  | Haccess _ -> (p, Aast.Hany) (* TODO: T37786581 *)
  | Hsoft _ -> (p, Aast.Hany)  (* TODO: T37786581 *)
  | Hreified _ -> (p, Aast.Hany) (* TODO: T37786581 *)

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
  | Omitted -> Aast.Any (* TODO: T37786581 *)
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
  | Call _ -> Aast.Any (* TODO: T37786581 *)
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
  | Pipe _ -> Aast.Any (* TODO: T37786581 *)
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
  | Let _                     -> Aast.Noop (* TODO: T37786581 *)
  | Block sl                  -> Aast.Block (on_block sl)
  | Unsafe                    -> failwith "Unsafe statements should be removed in on_block"
  | Fallthrough               -> Aast.Fallthrough
  | Noop                      -> Aast.Noop
  | Markup (s, e)             -> Aast.Markup (s, optional on_expr e)
  | Break _                   -> Aast.Noop (* TODO: T37786581 *)
  | Continue _                -> Aast.Noop (* TODO: T37786581 *)
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
  | Using _                   -> Aast.Noop (* TODO: T37786581 *)
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
  } in
  named_fun

and on_enum (e : enum_) : Aast.enum_ =
  Aast.{
    e_base = on_hint e.e_base;
    e_constraint = optional on_hint e.e_constraint
  }

and on_class c : Aast.class_ =
  let tparams = on_list on_tparam c.c_tparams in
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
      c_uses                  = [];
      c_method_redeclarations = [];
      c_xhp_attr_uses         = [];
      c_xhp_category          = [];
      c_req_extends           = [];
      c_req_implements        = [];
      c_implements            = on_list on_hint c.c_implements;
      c_consts                = [];
      c_typeconsts            = [];
      c_static_vars           = [];
      c_vars                  = [];
      c_constructor           = None;
      c_static_methods        = [];
      c_methods               = [];
      c_user_attributes       = on_list on_user_attribute c.c_user_attributes;
      c_namespace             = c.c_namespace;
      c_enum                  = optional on_enum c.c_enum
      (* TODO: T37786581
       * c.c_doc_comment
       * c.c_body
       *)
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

and on_def : def -> Aast.def = function
  | Fun f -> Aast.Fun (on_fun f)
  | Class c -> Aast.Class (on_class c)
  | Stmt s -> Aast.Stmt (on_stmt s)
  | Typedef t -> Aast.Typedef (on_typedef t)
  | Constant c -> Aast.Constant (on_constant c)
  | Namespace _ -> Aast.Stmt Aast.Noop (* TODO: T37786581 *)
  | NamespaceUse _ -> Aast.Stmt Aast.Noop (* TODO: T37786581 *)
  | SetNamespaceEnv env -> Aast.SetNamespaceEnv env

and program ast = on_list on_def ast

let convert ast : Aast.program = program ast
