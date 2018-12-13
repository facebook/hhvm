(* Given source and target annotation types, construct map functions that
 * transform one annotated AST to another, applying a given function to
 * every annotation
 *)
open Core_kernel
open Aast
module MapAnnotatedAST
  (Source : ASTAnnotationTypes)
  (Target : ASTAnnotationTypes) =
struct
  module S = AnnotatedAST(Source)
  module T = AnnotatedAST(Target)
  module SourceExpr = Source.ExprAnnotation
  module SourceEnv = Source.EnvAnnotation
  module TargetExpr = Target.ExprAnnotation
  module TargetEnv = Target.EnvAnnotation

  type mapping_env = {
    map_expr_annotation : SourceExpr.t -> TargetExpr.t;
    map_env_annotation : SourceEnv.t -> TargetEnv.t;
  }

  let rec map_afield menv af =
    match af with
    | S.AFvalue e -> T.AFvalue (map_expr menv e)
    | S.AFkvalue (e1, e2) -> T.AFkvalue (map_expr menv e1, map_expr menv e2)

  and map_field menv (e1, e2) = (map_expr menv e1, map_expr menv e2)

  and map_special_func menv sf =
    match sf with
    | S.Gena e -> T.Gena (map_expr menv e)
    | S.Genva el -> T.Genva (map_exprl menv el)
    | S.Gen_array_rec e -> T.Gen_array_rec (map_expr menv e)

  and map_class_id_ menv ci =
    match ci with
    | S.CIparent -> T.CIparent
    | S.CIself -> T.CIself
    | S.CIstatic -> T.CIstatic
    | S.CIexpr e -> T.CIexpr (map_expr menv e)
    | S.CI x -> T.CI x

  and map_class_id menv (ca, ci) =
    (menv.map_expr_annotation ca, map_class_id_ menv ci)

  and map_expr menv (p,e) =
  let e' =
    match e with
    | S.Array afl -> T.Array (List.map afl (map_afield menv))
    | S.Darray fl -> T.Darray (List.map fl (map_field menv))
    | S.Varray el -> T.Varray (map_exprl menv el)
    | S.Shape sm -> T.Shape (ShapeMap.map (map_expr menv) sm)
    | S.ValCollection (k, el) -> T.ValCollection (k, map_exprl menv el)
    | S.KeyValCollection (k, fl) ->
      T.KeyValCollection (k, List.map fl (map_field menv))
    | S.This -> T.This
    | S.Any -> T.Any
    | S.Id id -> T.Id id
    | S.Lvar id -> T.Lvar id
    | S.ImmutableVar id -> T.ImmutableVar id
    | S.Lplaceholder x -> T.Lplaceholder x
    | S.Fun_id x -> T.Fun_id x
    | S.True -> T.True
    | S.False -> T.False
    | S.Int i -> T.Int i
    | S.Float f -> T.Float f
    | S.Null -> T.Null
    | S.String s -> T.String s
    | S.Yield_break -> T.Yield_break
    | S.Method_caller (x,y) -> T.Method_caller (x,y)
    | S.Smethod_id (x,y) -> T.Smethod_id (x,y)
    | S.Class_get (ci,y) -> T.Class_get (map_class_id menv ci, y)
    | S.Class_const (ci,y) -> T.Class_const (map_class_id menv ci, y)
    | S.Dollardollar x -> T.Dollardollar x
    | S.Typename x -> T.Typename x
    | S.Special_func sf -> T.Special_func (map_special_func menv sf)
    | S.Method_id(e, id) -> T.Method_id(map_expr menv e, id)
    | S.Obj_get(e1, e2, fl) -> T.Obj_get(map_expr menv e1, map_expr menv e2, fl)
    | S.Array_get(e1, e2) ->
      T.Array_get(map_expr menv e1, Option.map e2 (map_expr menv))
    | S.Call(t, e1, hl, el1, el2) ->
      T.Call(t, map_expr menv e1, hl, map_exprl menv el1, map_exprl menv el2)
    | S.String2 el -> T.String2 (map_exprl menv el)
    | S.PrefixedString (n, e) -> T.PrefixedString (n, map_expr menv e)
    | S.Dollar e -> T.Dollar (map_expr menv e)
    | S.Yield af -> T.Yield (map_afield menv af)
    | S.Yield_from e -> T.Yield_from (map_expr menv e)
    | S.Await e -> T.Await (map_expr menv e)
    | S.Suspend e -> T.Suspend (map_expr menv e)
    | S.List el -> T.List (map_exprl menv el)
    | S.Pair (e1, e2) -> T.Pair (map_expr menv e1, map_expr menv e2)
    | S.Expr_list el -> T.Expr_list (map_exprl menv el)
    | S.Cast(h, e) -> T.Cast(h, map_expr menv e)
    | S.Unop(o, e) -> T.Unop(o, map_expr menv e)
    | S.Binop(o, e1, e2) -> T.Binop(o, map_expr menv e1, map_expr menv e2)
    | S.Pipe(id, e1, e2) -> T.Pipe(id, map_expr menv e1, map_expr menv e2)
    | S.Eif(e1, e2, e3) ->
      T.Eif(map_expr menv e1, Option.map e2 (map_expr menv), map_expr menv e3)
    | S.InstanceOf (e, ci) -> T.InstanceOf (map_expr menv e, map_class_id menv ci)
    | S.Is (e, h) -> T.Is (map_expr menv e, h)
    | S.As (e, h, b) -> T.As (map_expr menv e, h, b)
    | S.New (ci, el1, el2, ctor_annot) ->
      T.New (map_class_id menv ci, map_exprl menv el1,
        map_exprl menv el2, menv.map_expr_annotation ctor_annot)
    | S.Efun (ef, ids) -> T.Efun(map_fun menv ef, ids)
    | S.Xml (id, pl, el) ->
      T.Xml (id, List.map pl (fun attr -> match attr with
        | S.Xhp_simple (p, e) -> T.Xhp_simple (p,map_expr menv e)
        | S.Xhp_spread e -> T.Xhp_spread (map_expr menv e)
      ), map_exprl menv el)
    | S.Unsafe_expr e -> T.Unsafe_expr (map_expr menv e)
    | S.Callconv (k, e) -> T.Callconv (k, map_expr menv e)
    | S.Execution_operator e -> T.Execution_operator (map_exprl menv e)
    | S.Assert (S.AE_assert e) -> T.Assert (T.AE_assert (map_expr menv e))
    | S.Clone e -> T.Clone (map_expr menv e)
  in
  let p' = menv.map_expr_annotation p in
    (p', e')

  and map_exprl menv el = List.map el (map_expr menv)

  and map_fun menv fd =
  {
    T.f_span = fd.S.f_span;
    T.f_annotation = menv.map_env_annotation fd.S.f_annotation;
    T.f_mode = fd.S.f_mode;
    T.f_ret = fd.S.f_ret;
    T.f_name = fd.S.f_name;
    T.f_tparams = fd.S.f_tparams;
    T.f_where_constraints = fd.S.f_where_constraints;
    T.f_variadic = map_fun_variadicity menv fd.S.f_variadic;
    T.f_params = List.map fd.S.f_params (map_fun_param menv);
    T.f_body = map_func_body menv fd.S.f_body;
    T.f_fun_kind = fd.S.f_fun_kind;
    T.f_user_attributes =
    List.map fd.S.f_user_attributes (map_user_attribute menv);
    T.f_ret_by_ref = fd.S.f_ret_by_ref;
    T.f_external = fd.S.f_external;
  }

  and map_user_attribute menv ua =
  {
    T.ua_name = ua.S.ua_name;
    T.ua_params = map_exprl menv ua.S.ua_params;
  }

  and map_func_body menv b =
    match b with
    | S.UnnamedBody fub ->
      T.UnnamedBody {
        T.fub_ast = fub.S.fub_ast;
        T.fub_tparams = fub.S.fub_tparams;
        T.fub_namespace = fub.S.fub_namespace;
      }
    | S.NamedBody fnb ->
      T.NamedBody {
        T.fnb_unsafe = fnb.S.fnb_unsafe;
        T.fnb_nast = map_block menv fnb.S.fnb_nast;
      }

  and map_stmt menv s =
    let map_as_expr ae =
      match ae with
      | S.As_v e -> T.As_v (map_expr menv e)
      | S.As_kv (e1, e2) -> T.As_kv (map_expr menv e1, map_expr menv e2)
      | S.Await_as_v (p, e) -> T.Await_as_v (p, map_expr menv e)
      | S.Await_as_kv (p, e1, e2) ->
        T.Await_as_kv (p, map_expr menv e1, map_expr menv e2) in
    let map_case c =
      match c with
      | S.Default b -> T.Default (map_block menv b)
      | S.Case (e, b) -> T.Case (map_expr menv e, map_block menv b) in
    let map_catch (id1, id2, b) = (id1, id2, map_block menv b) in
    match s with
    | S.Expr e -> T.Expr (map_expr menv e)
    | S.Break p -> T.Break p
    | S.Continue p -> T.Continue p
    | S.Throw (b, e) -> T.Throw (b, map_expr menv e)
    | S.Return (p, oe) -> T.Return (p, Option.map oe (map_expr menv))
    | S.GotoLabel label -> T.GotoLabel label
    | S.Goto label -> T.Goto label
    | S.Static_var el -> T.Static_var (map_exprl menv el)
    | S.Global_var el -> T.Global_var (map_exprl menv el)
    | S.Awaitall (pos, el) ->
      let el = List.map el (fun (e1, e2) ->
        (Option.map e1 (map_expr menv), map_expr menv e2)
      ) in
      T.Awaitall (pos, el)
    | S.If(e, b1, b2) -> T.If (map_expr menv e, map_block menv b1, map_block menv b2)
    | S.Do(b, e) -> T.Do(map_block menv b, map_expr menv e)
    | S.While(e, b) -> T.While(map_expr menv e, map_block menv b)
    | S.Using(has_await, e, b) -> T.Using(has_await, map_expr menv e, map_block menv b)
    | S.For(e1, e2, e3, b) ->
      T.For(map_expr menv e1, map_expr menv e2, map_expr menv e3, map_block menv b)
    | S.Switch(e, cl) -> T.Switch(map_expr menv e, List.map cl map_case)
    | S.Foreach(e, ae, b) ->
      T.Foreach(map_expr menv e, map_as_expr ae, map_block menv b)
    | S.Try (b1, cl, b2) ->
      T.Try(map_block menv b1, List.map cl map_catch, map_block menv b2)
    | S.Def_inline d -> T.Def_inline (map_def menv d)
    | S.Noop -> T.Noop
    | S.Unsafe_block b -> T.Unsafe_block(map_block menv b)
    | S.Fallthrough -> T.Fallthrough
    | S.Let(x, h, e) -> T.Let(x, h, map_expr menv e)
    | S.Block b -> T.Block (map_block menv b)
    | S.Markup (s, eopt) -> T.Markup (s, Option.map eopt (map_expr menv))
    | S.Declare (is_block, e, b) -> T.Declare(is_block, map_expr menv e, map_block menv b)

  and map_block menv sl = List.map sl (map_stmt menv)

  and map_fun_param menv fp =
  {
    T.param_annotation = menv.map_expr_annotation fp.S.param_annotation;
    T.param_hint = fp.S.param_hint;
    T.param_is_reference = fp.S.param_is_reference;
    T.param_is_variadic = fp.S.param_is_variadic;
    T.param_pos = fp.S.param_pos;
    T.param_name = fp.S.param_name;
    T.param_expr = Option.map fp.S.param_expr (map_expr menv);
    T.param_callconv = fp.S.param_callconv;
    T.param_user_attributes =
    List.map fp.S.param_user_attributes (map_user_attribute menv);
  }

  and map_fun_variadicity menv v =
    match v with
    | S.FVvariadicArg fp -> T.FVvariadicArg (map_fun_param menv fp)
    | S.FVellipsis p -> T.FVellipsis p
    | S.FVnonVariadic -> T.FVnonVariadic

  and map_class menv c =
  {
    T.c_span = c.S.c_span;
    T.c_annotation = menv.map_env_annotation c.S.c_annotation;
    T.c_mode = c.S.c_mode;
    T.c_final = c.S.c_final;
    T.c_is_xhp = c.S.c_is_xhp;
    T.c_kind = c.S.c_kind;
    T.c_name = c.S.c_name;
    T.c_tparams = c.S.c_tparams;
    T.c_extends = c.S.c_extends;
    T.c_uses = c.S.c_uses;
    T.c_method_redeclarations =
      List.map c.S.c_method_redeclarations ~f:(map_method_redeclaration menv);
    T.c_xhp_attr_uses = c.S.c_xhp_attr_uses;
    T.c_xhp_category = c.S.c_xhp_category;
    T.c_req_extends = c.S.c_req_extends;
    T.c_req_implements = c.S.c_req_implements;
    T.c_implements = c.S.c_implements;
    T.c_consts = List.map c.S.c_consts (map_class_const menv);
    T.c_typeconsts = List.map c.S.c_typeconsts map_class_typeconst;
    T.c_static_vars = List.map c.S.c_static_vars (map_class_var menv);
    T.c_vars = List.map c.S.c_vars (map_class_var menv);
    T.c_constructor = Option.map c.S.c_constructor (map_method menv);
    T.c_static_methods = List.map c.S.c_static_methods (map_method menv);
    T.c_methods = List.map c.S.c_methods (map_method menv);
    T.c_namespace = c.S.c_namespace;
    T.c_user_attributes = List.map c.S.c_user_attributes (map_user_attribute menv);
    T.c_enum = c.S.c_enum;
    T.c_doc_comment = c.S.c_doc_comment;
    T.c_attributes = List.map c.S.c_attributes (map_attribute menv);
    T.c_xhp_children = c.S.c_xhp_children;
    T.c_xhp_attrs = List.map c.S.c_xhp_attrs (map_xhp_attr menv);
  }

  and map_xhp_attr menv (h, var, b, maybe_enum) =
    (h, map_class_var menv var, b, Option.map maybe_enum (map_xhp_attr_enum menv))

  and map_attribute menv attr =
    match attr with
    | S. CA_name id -> T.CA_name id
    | S.CA_field f -> T.CA_field T.{
      ca_type = map_ca_type f.S.ca_type;
      ca_id = f.S.ca_id;
      ca_value = Option.map f.S.ca_value (map_expr menv);
      ca_required = f.S.ca_required;
    }

  and map_ca_type ty =
    match ty with
    | S.CA_hint h -> T.CA_hint h
    | S.CA_enum sl -> T.CA_enum sl

  and map_xhp_attr_enum menv (p, b, el) = (p, b, List.map el (map_expr menv))

  and map_class_const menv (h, id, e) =
    (h, id, Option.map e (map_expr menv))

  and map_class_typeconst tc =
  {
    T.c_tconst_name = tc.S.c_tconst_name;
    T.c_tconst_constraint = tc.S.c_tconst_constraint;
    T.c_tconst_type = tc.S.c_tconst_type;
  }

  and map_class_var menv cv =
  {
    T.cv_final = cv.S.cv_final;
    T.cv_is_xhp = cv.S.cv_is_xhp;
    T.cv_visibility = cv.S.cv_visibility;
    T.cv_type = cv.S.cv_type;
    T.cv_id = cv.S.cv_id;
    T.cv_expr = Option.map cv.S.cv_expr (map_expr menv);
    T.cv_user_attributes = List.map cv.S.cv_user_attributes (map_user_attribute menv);
  }

  and map_method menv m =
    {
      T.m_span = m.S.m_span;
      T.m_annotation = menv.map_env_annotation m.S.m_annotation;
      T.m_final = m.S.m_final;
      T.m_static = m.S.m_static;
      T.m_abstract = m.S.m_abstract;
      T.m_visibility = m.S.m_visibility;
      T.m_name = m.S.m_name;
      T.m_tparams = m.S.m_tparams;
      T.m_where_constraints = m.S.m_where_constraints;
      T.m_variadic = map_fun_variadicity menv m.S.m_variadic;
      T.m_params = List.map m.S.m_params (map_fun_param menv);
      T.m_body = map_func_body menv m.S.m_body;
      T.m_fun_kind = m.S.m_fun_kind;
      T.m_user_attributes = List.map m.S.m_user_attributes (map_user_attribute menv);
      T.m_ret = m.S.m_ret;
      T.m_ret_by_ref = m.S.m_ret_by_ref;
      T.m_external = m.S.m_external;
    }

  and map_method_redeclaration menv mt =
  {
    T.mt_final = mt.S.mt_final;
    T.mt_static = mt.S.mt_static;
    T.mt_abstract = mt.S.mt_abstract;
    T.mt_visibility = mt.S.mt_visibility;
    T.mt_name = mt.S.mt_name;
    T.mt_tparams = mt.S.mt_tparams;
    T.mt_where_constraints = mt.S.mt_where_constraints;
    T.mt_variadic = map_fun_variadicity menv mt.S.mt_variadic;
    T.mt_params = List.map mt.S.mt_params (map_fun_param menv);
    T.mt_fun_kind = mt.S.mt_fun_kind;
    T.mt_ret = mt.S.mt_ret;
    T.mt_ret_by_ref = mt.S.mt_ret_by_ref;
    T.mt_trait = mt.S.mt_trait;
    T.mt_method = mt.S.mt_method;
    T.mt_user_attributes = List.map mt.S.mt_user_attributes (map_user_attribute menv);
  }

  and map_typedef menv td = {
    T.t_annotation = menv.map_env_annotation td.S.t_annotation;
    T.t_mode = td.S.t_mode;
    T.t_name = td.S.t_name;
    T.t_tparams = td.S.t_tparams;
    T.t_constraint = td.S.t_constraint;
    T.t_vis = td.S.t_vis;
    T.t_kind = td.S.t_kind;
    T.t_namespace = td.S.t_namespace;
    T.t_user_attributes =
      List.map td.S.t_user_attributes (map_user_attribute menv);
  }

  and map_gconst menv c = {
    T.cst_annotation = menv.map_env_annotation c.S.cst_annotation;
    T.cst_mode = c.S.cst_mode;
    T.cst_name = c.S.cst_name;
    T.cst_type = c.S.cst_type;
    T.cst_value = Option.map c.S.cst_value (map_expr menv);
    T.cst_is_define = c.S.cst_is_define;
    T.cst_namespace = c.S.cst_namespace;
  }

  and map_ns_use (k, id1, id2) =
    let kind = match k with
    | S.NSNamespace -> T.NSNamespace
    | S.NSClass -> T.NSClass
    | S.NSClassAndNamespace -> T.NSClassAndNamespace
    | S.NSFun -> T.NSFun
    | S.NSConst -> T.NSConst
    in
    (kind, id1, id2)

  and map_def menv d =
    let { map_expr_annotation; map_env_annotation } = menv in
    match d with
    | S.Fun fd -> T.Fun (map_fun menv fd)
    | S.Class c -> T.Class (map_class menv c)
    | S.Typedef td -> T.Typedef (map_typedef menv td)
    | S.Constant gc -> T.Constant (map_gconst menv gc)
    | S.Stmt s -> T.Stmt (map_stmt menv s)
    | S.Namespace (id, p) ->
      T.Namespace (id, map_program ~map_expr_annotation ~map_env_annotation p)
    | S.NamespaceUse usel -> T.NamespaceUse (List.map usel map_ns_use)
    | S.SetNamespaceEnv env -> T.SetNamespaceEnv env

  and map_program
    ~map_expr_annotation
    ~map_env_annotation
    dl =
    let menv = {
      map_expr_annotation;
      map_env_annotation;
    } in
    List.map dl (map_def menv)
end
