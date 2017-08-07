(* Given source and target annotation types, construct map functions that
 * transform one annotated AST to another, applying a given function to
 * every annotation
 *)
open Core
open Nast
module MapAnnotatedAST
  (Source : AnnotationType)
  (Target : AnnotationType) =
struct
  module S = AnnotatedAST(Source)
  module T = AnnotatedAST(Target)

  let rec map_expr f (p,e) =
  let map_afield af =
    match af with
    | S.AFvalue e -> T.AFvalue (map_expr f e)
    | S.AFkvalue (e1, e2) -> T.AFkvalue (map_expr f e1, map_expr f e2) in
  let map_field (e1, e2) = (map_expr f e1, map_expr f e2) in
  let map_special_func sf =
    match sf with
    | S.Gena e -> T.Gena (map_expr f e)
    | S.Genva el -> T.Genva (map_exprl f el)
    | S.Gen_array_rec e -> T.Gen_array_rec (map_expr f e) in
  let map_class_id ci =
    match ci with
    | S.CIparent -> T.CIparent
    | S.CIself -> T.CIself
    | S.CIstatic -> T.CIstatic
    | S.CIexpr e -> T.CIexpr (map_expr f e)
    | S.CI x -> T.CI x in
  let e' =
    match e with
    | S.Array afl -> T.Array (List.map afl map_afield)
    | S.Darray fl -> T.Darray (List.map fl map_field)
    | S.Varray el -> T.Varray (map_exprl f el)
    | S.Shape sm -> T.Shape (ShapeMap.map (map_expr f) sm)
    | S.ValCollection (k, el) -> T.ValCollection (k, map_exprl f el)
    | S.KeyValCollection (k, fl) ->
      T.KeyValCollection (k, List.map fl map_field)
    | S.This -> T.This
    | S.Any -> T.Any
    | S.Id id -> T.Id id
    | S.Lvar id -> T.Lvar id
    | S.Lvarvar (x,y) -> T.Lvarvar (x,y)
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
    | S.Class_get (ci,y) -> T.Class_get (map_class_id ci, y)
    | S.Class_const (ci,y) -> T.Class_const (map_class_id ci, y)
    | S.Dollardollar x -> T.Dollardollar x
    | S.Typename x -> T.Typename x
    | S.Special_func sf -> T.Special_func (map_special_func sf)
    | S.Method_id(e, id) -> T.Method_id(map_expr f e, id)
    | S.Obj_get(e1, e2, fl) -> T.Obj_get(map_expr f e1, map_expr f e2, fl)
    | S.Array_get(e1, e2) ->
      T.Array_get(map_expr f e1, Option.map e2 (map_expr f))
    | S.Call(t, e1, hl, el1, el2) ->
      T.Call(t, map_expr f e1, hl, map_exprl f el1, map_exprl f el2)
    | S.String2 el -> T.String2 (map_exprl f el)
    | S.Yield af -> T.Yield (map_afield af)
    | S.Await e -> T.Await (map_expr f e)
    | S.List el -> T.List (map_exprl f el)
    | S.Pair (e1, e2) -> T.Pair (map_expr f e1, map_expr f e2)
    | S.Expr_list el -> T.Expr_list (map_exprl f el)
    | S.Cast(h, e) -> T.Cast(h, map_expr f e)
    | S.Unop(o, e) -> T.Unop(o, map_expr f e)
    | S.Binop(o, e1, e2) -> T.Binop(o, map_expr f e1, map_expr f e2)
    | S.Pipe(id, e1, e2) -> T.Pipe(id, map_expr f e1, map_expr f e2)
    | S.Eif(e1, e2, e3) ->
      T.Eif(map_expr f e1, Option.map e2 (map_expr f), map_expr f e3)
    | S.NullCoalesce (e1, e2) -> T.NullCoalesce (map_expr f e1, map_expr f e2)
    | S.InstanceOf (e, ci) -> T.InstanceOf (map_expr f e, map_class_id ci)
    | S.New (ci, el1, el2) ->
      T.New (map_class_id ci, map_exprl f el1, map_exprl f el2)
    | S.Efun (ef, ids) -> T.Efun(map_fun f ef, ids)
    | S.Xml (id, pl, el) ->
      T.Xml (id, List.map pl (fun (p,e) -> (p,map_expr f e)), map_exprl f el)
    | S.Assert (S.AE_assert e) -> T.Assert (T.AE_assert (map_expr f e))
    | S.Clone e -> T.Clone (map_expr f e)
  in
    (f p, e')

  and map_exprl f el = List.map el (map_expr f)

  and map_fun f fd =
  {
    T.f_mode = fd.S.f_mode;
    T.f_ret = fd.S.f_ret;
    T.f_name = fd.S.f_name;
    T.f_tparams = fd.S.f_tparams;
    T.f_where_constraints = fd.S.f_where_constraints;
    T.f_variadic = map_fun_variadicity f fd.S.f_variadic;
    T.f_params = List.map fd.S.f_params (map_fun_param f);
    T.f_body = map_func_body f fd.S.f_body;
    T.f_fun_kind = fd.S.f_fun_kind;
    T.f_user_attributes =
    List.map fd.S.f_user_attributes (map_user_attribute f);
  }

  and map_user_attribute f ua =
  {
    T.ua_name = ua.S.ua_name;
    T.ua_params = map_exprl f ua.S.ua_params;
  }

  and map_func_body f b =
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
        T.fnb_nast = map_block f fnb.S.fnb_nast;
      }

  and map_stmt f s =
    let map_as_expr ae =
      match ae with
      | S.As_v e -> T.As_v (map_expr f e)
      | S.As_kv (e1, e2) -> T.As_kv (map_expr f e1, map_expr f e2)
      | S.Await_as_v (p, e) -> T.Await_as_v (p, map_expr f e)
      | S.Await_as_kv (p, e1, e2) ->
        T.Await_as_kv (p, map_expr f e1, map_expr f e2) in
    let map_case c =
      match c with
      | S.Default b -> T.Default (map_block f b)
      | S.Case (e, b) -> T.Case (map_expr f e, map_block f b) in
    let map_catch (id1, id2, b) = (id1, id2, map_block f b) in
    match s with
    | S.Expr e -> T.Expr (map_expr f e)
    | S.Break p -> T.Break p
    | S.Continue p -> T.Continue p
    | S.Throw (b, e) -> T.Throw (b, map_expr f e)
    | S.Return (p, oe) -> T.Return (p, Option.map oe (map_expr f))
    | S.GotoLabel label -> T.GotoLabel label
    | S.Goto label -> T.Goto label
    | S.Static_var el -> T.Static_var (map_exprl f el)
    | S.Global_var el -> T.Global_var (map_exprl f el)
    | S.If(e, b1, b2) -> T.If (map_expr f e, map_block f b1, map_block f b2)
    | S.Do(b, e) -> T.Do(map_block f b, map_expr f e)
    | S.While(e, b) -> T.While(map_expr f e, map_block f b)
    | S.For(e1, e2, e3, b) ->
      T.For(map_expr f e1, map_expr f e2, map_expr f e3, map_block f b)
    | S.Switch(e, cl) -> T.Switch(map_expr f e, List.map cl map_case)
    | S.Foreach(e, ae, b) ->
      T.Foreach(map_expr f e, map_as_expr ae, map_block f b)
    | S.Try (b1, cl, b2) ->
      T.Try(map_block f b1, List.map cl map_catch, map_block f b2)
    | S.Noop -> T.Noop
    | S.Fallthrough -> T.Fallthrough

  and map_block f sl = List.map sl (map_stmt f)

  and map_fun_param f fp =
  {
    T.param_hint = fp.S.param_hint;
    T.param_is_reference = fp.S.param_is_reference;
    T.param_is_variadic = fp.S.param_is_variadic;
    T.param_pos = fp.S.param_pos;
    T.param_name = fp.S.param_name;
    T.param_expr = Option.map fp.S.param_expr (map_expr f);
  }

  and map_fun_variadicity f v =
    match v with
    | S.FVvariadicArg fp -> T.FVvariadicArg (map_fun_param f fp)
    | S.FVellipsis -> T.FVellipsis
    | S.FVnonVariadic -> T.FVnonVariadic

  and map_class f c =
  {
    T.c_mode = c.S.c_mode;
    T.c_final = c.S.c_final;
    T.c_is_xhp = c.S.c_is_xhp;
    T.c_kind = c.S.c_kind;
    T.c_name = c.S.c_name;
    T.c_tparams = c.S.c_tparams;
    T.c_extends = c.S.c_extends;
    T.c_uses = c.S.c_uses;
    T.c_xhp_attr_uses = c.S.c_xhp_attr_uses;
    T.c_xhp_category = c.S.c_xhp_category;
    T.c_req_extends = c.S.c_req_extends;
    T.c_req_implements = c.S.c_req_implements;
    T.c_implements = c.S.c_implements;
    T.c_consts = List.map c.S.c_consts (map_class_const f);
    T.c_typeconsts = List.map c.S.c_typeconsts (map_class_typeconst f);
    T.c_static_vars = List.map c.S.c_static_vars (map_class_var f);
    T.c_vars = List.map c.S.c_vars (map_class_var f);
    T.c_constructor = Option.map c.S.c_constructor (map_method f);
    T.c_static_methods = List.map c.S.c_static_methods (map_method f);
    T.c_methods = List.map c.S.c_methods (map_method f);
    T.c_user_attributes = List.map c.S.c_user_attributes (map_user_attribute f);
    T.c_enum = c.S.c_enum;
  }

  and map_class_const f (h, id, e) =
    (h, id, Option.map e (map_expr f))

  and map_class_typeconst _f tc =
  {
    T.c_tconst_name = tc.S.c_tconst_name;
    T.c_tconst_constraint = tc.S.c_tconst_constraint;
    T.c_tconst_type = tc.S.c_tconst_type;
  }

  and map_class_var f cv =
  {
    T.cv_final = cv.S.cv_final;
    T.cv_is_xhp = cv.S.cv_is_xhp;
    T.cv_visibility = cv.S.cv_visibility;
    T.cv_type = cv.S.cv_type;
    T.cv_id = cv.S.cv_id;
    T.cv_expr = Option.map cv.S.cv_expr (map_expr f);
  }

  and map_method f m = {
    T.m_final = m.S.m_final;
    T.m_abstract = m.S.m_abstract;
    T.m_visibility = m.S.m_visibility;
    T.m_name = m.S.m_name;
    T.m_tparams = m.S.m_tparams;
    T.m_where_constraints = m.S.m_where_constraints;
    T.m_variadic = map_fun_variadicity f m.S.m_variadic;
    T.m_params = List.map m.S.m_params (map_fun_param f);
    T.m_body = map_func_body f m.S.m_body;
    T.m_fun_kind = m.S.m_fun_kind;
    T.m_user_attributes = List.map m.S.m_user_attributes (map_user_attribute f);
    T.m_ret = m.S.m_ret;
  }

  and map_typedef f td = {
    T.t_mode = td.S.t_mode;
    T.t_name = td.S.t_name;
    T.t_tparams = td.S.t_tparams;
    T.t_constraint = td.S.t_constraint;
    T.t_vis = td.S.t_vis;
    T.t_kind = td.S.t_kind;
    T.t_user_attributes =
      List.map td.S.t_user_attributes (map_user_attribute f);
  }

  and map_gconst f c = {
    T.cst_mode = c.S.cst_mode;
    T.cst_name = c.S.cst_name;
    T.cst_type = c.S.cst_type;
    T.cst_value = Option.map c.S.cst_value (map_expr f);
  }

  let map_def f d =
    match d with
    | S.Fun fd -> T.Fun (map_fun f fd)
    | S.Class c -> T.Class (map_class f c)
    | S.Typedef td -> T.Typedef (map_typedef f td)
    | S.Constant gc -> T.Constant (map_gconst f gc)

  let map_program f dl = List.map dl (map_def f)
end
