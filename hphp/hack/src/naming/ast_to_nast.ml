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
  c_xhp_category   : (pos * pstring list) option          ;
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
  c_pu_enums       : Aast.pu_enum list                    ;
}

let make_empty_class_body = {
  c_uses = [];
  c_method_redeclarations = [];
  c_xhp_attr_uses = [];
  c_xhp_category = None;
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
  c_pu_enums = [];
}

let on_list f l = List.map f l

let on_list_append_acc acc f l =
  List.fold_left
    (fun acc li -> f li :: acc)
    acc
    l
let optional f = function
  | None -> None
  | Some x -> Some (f x)

let both f (p1, p2) = (f p1, f p2)

let reification reified attributes =
  let soft = List.exists (fun { Aast.ua_name = (_, n); _ } -> n = SN.UserAttributes.uaSoft)
    attributes in
  if reified
  then if soft
    then Aast.SoftReified
    else Aast.Reified
  else Aast.Erased

let rec on_variadic_hint h =
  match h with
  | Hvariadic h -> Aast.Hvariadic (optional on_hint h)
  | Hnon_variadic -> Aast.Hnon_variadic

and get_pos_shape_name name =
  match name with
  | SFlit_int (pos, _)
  | SFlit_str (pos, _)
  | SFclass_const (_, (pos, _)) -> pos

and on_shape_info info =
  let on_shape_field sf =
    Aast.{ sfi_optional = sf.sf_optional; sfi_hint = on_hint sf.sf_hint; sfi_name = sf.sf_name } in
  let _, nfm =
    List.fold_left
      (fun (map_, list_) sf ->
        if ShapeMap.mem sf.sf_name map_
        then Errors.fd_name_already_bound (get_pos_shape_name sf.sf_name);
        let sfi = on_shape_field sf in
        let map_ = ShapeMap.add sf.sf_name sfi map_ in
        let list_ = sfi :: list_ in
        map_, list_)
      (ShapeMap.empty, [])
      info.si_shape_field_list in
  let nfm = List.rev nfm in
  Aast.{
    nsi_allows_unknown_fields = info.si_allows_unknown_fields;
    nsi_field_map = nfm;
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
  | Hsoft h -> (p, Aast.Hsoft (on_hint h))
  | Hlike h -> (p, Aast.Hlike (on_hint h))

and on_class_elt trait_or_interface body elt : class_body =
  match elt with
  | Attributes attrs ->
    let attrs = on_list_append_acc body.c_attributes on_class_attr attrs in
    { body with c_attributes = attrs; }
  | Const (hopt, el) ->
    let hopt = optional on_hint hopt in
    let consts = on_list_append_acc body.c_consts (fun (id, e) ->
      (hopt, id, Some (on_expr e))
    ) el in
    { body with c_consts = consts; }
  | AbsConst (hopt, id) ->
    let consts = (optional on_hint hopt, id, None) :: body.c_consts in
    { body with c_consts = consts; }
  | ClassUse h  ->
    let hints = on_hint h :: body.c_uses in
    { body with c_uses = hints; }
  | ClassUseAlias (_, (p, _), _, _) ->
    Errors.unsupported_feature p "Trait use aliasing"; body
  | ClassUsePrecedence (_, (p, _), _) ->
    Errors.unsupported_feature p "The insteadof keyword"; body
  | MethodTraitResolution res ->
    let redecls = on_method_trait_resolution res :: body.c_method_redeclarations in
    { body with c_method_redeclarations = redecls; }
  | XhpAttrUse h ->
    let hints = on_hint h :: body.c_xhp_attr_uses in
    { body with c_xhp_attr_uses = hints }
  | ClassTraitRequire (MustExtend, h) ->
    let hints = on_hint h :: body.c_req_extends in
    { body with c_req_extends = hints; }
  | ClassTraitRequire (MustImplement, h) ->
    let hints = on_hint h :: body.c_req_implements in
    { body with c_req_implements = hints; }
  | ClassVars cvs when List.mem Static cvs.cv_kinds ->
    let vars = on_class_vars_with_acc body.c_static_vars cvs in
    { body with c_static_vars = vars; }
  | ClassVars cvs ->
    let vars = on_class_vars_with_acc body.c_vars cvs in
    { body with c_vars = vars; }
  | XhpAttr (hopt, var, is_required, maybe_enum)  ->
    (* TODO: T37984688 Updating naming.ml to use c_xhp_attrs *)
    let hopt = optional on_hint hopt in
    let attrs =
      (hopt,
        on_class_var true hopt [] [] false None var,
        is_required,
        optional on_xhp_attr maybe_enum) :: body.c_xhp_attrs in
    { body with c_xhp_attrs = attrs; }
  | XhpCategory (p, cs) ->
    if body.c_xhp_category <> None && cs <> []
    then Errors.multiple_xhp_category (fst (List.hd cs));
    { body with c_xhp_category = Some (p, cs); }
  | XhpChild (p, c) ->
    let children = (p, on_xhp_child c) :: body.c_xhp_children in
    { body with c_xhp_children = children; }
  | Method m when snd m.m_name = SN.Members.__construct ->
    if body.c_constructor <> None
    then Errors.method_name_already_bound (fst m.m_name) (snd m.m_name);
    { body with c_constructor = Some (on_method ~trait_or_interface m) }
  | Method m when List.mem Static m.m_kind ->
    let statics = on_method m :: body.c_static_methods in
    { body with c_static_methods = statics; }
  | Method m ->
    let methods = on_method m :: body.c_methods in
    { body with c_methods = methods; }
  | TypeConst tc ->
    let typeconsts = on_class_typeconst tc :: body.c_typeconsts in
    { body with c_typeconsts = typeconsts; }
  | ClassEnum (pu_is_final, pu_name, fields) ->
    let pu_enum = on_pu pu_name pu_is_final fields in
    let pu_enums = pu_enum :: body.c_pu_enums in
    { body with c_pu_enums = pu_enums }

and on_as_expr aw e : Aast.as_expr =
  match aw, e with
  | None, As_v ev -> Aast.As_v (on_expr ev)
  | Some p, As_v ev -> Aast.Await_as_v (p, on_expr ev)
  | None, As_kv (k, ev) -> Aast.As_kv (on_expr k, on_expr ev)
  | Some p, As_kv (k, ev) -> Aast.Await_as_kv (p, on_expr k, on_expr ev)

and on_afield f : Aast.afield =
  match f with
  | AFvalue e -> Aast.AFvalue (on_expr e)
  | AFkvalue (e1, e2) -> Aast.AFkvalue (on_expr e1, on_expr e2)

and on_darray_element (e1, e2) =
  (on_expr e1, on_expr e2)

and on_shape (sfn, e) =
  (sfn, on_expr e)

and on_awaitall el b =
  (on_list on_awaitall_expr el, on_block b)

and on_awaitall_expr (e1, e2) =
  let e2 = on_expr e2 in
  let e1 =
    match e1 with
    | Some (pos, name) ->
      let e = pos, Local_id.make_unscoped name in
      Some e
    | None -> None
  in
  (e1, e2)

and on_xhp_attribute a : Aast.xhp_attribute =
  match a with
  | Xhp_simple (id, e) -> Aast.Xhp_simple (id, on_expr e)
  | Xhp_spread e -> Aast.Xhp_spread (on_expr e)

and on_targ h : Aast.targ = on_hint h

and on_collection_targ targ = match targ with
  | Some CollectionTKV (tk, tv) -> Some (Aast.CollectionTKV (on_targ tk, on_targ tv))
  | Some CollectionTV tv -> Some (Aast.CollectionTV (on_targ tv))
  | None -> None

and on_expr (p, e) : Aast.expr =
  let node = match e with
  | Array al -> Aast.Array (on_list on_afield al)
  | Varray (ta, el) -> Aast.Varray (optional on_targ ta, on_list on_expr el)
  | Darray (tap, d) -> Aast.Darray (optional (both on_targ) tap, on_list on_darray_element d)
  | Shape s -> Aast.Shape (on_list on_shape s)
  | Collection (id, tal, al) -> Aast.Collection (id, on_collection_targ tal, on_list on_afield al)
  | Record (_,_) -> Aast.Any (* TODO: T37786581 *)
  | Null -> Aast.Null
  | True -> Aast.True
  | False -> Aast.False
  | Omitted -> Aast.Omitted
  | Id id -> Aast.Id id
  | Lvar id ->
    let lid = Local_id.make_unscoped (snd id) in
    Aast.Lvar (p, lid)
  | Clone e -> Aast.Clone (on_expr e)
  | Obj_get (e1, e2, f) -> Aast.Obj_get (on_expr e1, on_expr e2, f)
  | Array_get (e, opt_e) -> Aast.Array_get (on_expr e, optional on_expr opt_e)
  | Class_get (e1, (_, (Id x2 | Lvar x2))) ->
    Aast.Class_get((p, Aast.CIexpr (on_expr e1)), Aast.CGstring x2)
  | Class_get (e1, e2) -> Aast.Class_get ((p, Aast.CIexpr (on_expr e1)), Aast.CGexpr (on_expr e2))
  | Class_const (e, s) -> Aast.Class_const ((p, Aast.CIexpr (on_expr e)), s)
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
  | InstanceOf (e1, e2) -> Aast.InstanceOf (on_expr e1, (p, Aast.CIexpr (on_expr e2)))
  | Is (e, h) -> Aast.Is (on_expr e, on_hint h)
  | As (e, h, b) -> Aast.As (on_expr e, on_hint h, b)
  | New (e, tl, el1, el2) ->
    Aast.New (
      (p, Aast.CIexpr (on_expr e)),
      on_list on_targ tl,
      on_list on_expr el1,
      on_list on_expr el2,
      p
    )
  | Efun (f, use_list) ->
    let ids =
      List.map (fun ((p, id)) -> (p, Local_id.make_unscoped id)) use_list in
    Aast.Efun (on_fun f, ids)
  | Lfun f -> Aast.Lfun (on_fun f, [])
  | BracedExpr e -> Aast.BracedExpr (on_expr e)
  | ParenthesizedExpr e -> Aast.ParenthesizedExpr (on_expr e)
  | Xml (id, xhpl, el) -> Aast.Xml (id, on_list on_xhp_attribute xhpl, on_list on_expr el)
  | Unsafeexpr e -> Aast.Unsafe_expr (on_expr e)
  | Import (f, e) -> Aast.Import (on_import_flavor f, on_expr e)
  | Callconv (k, e) -> Aast.Callconv (k, on_expr e)
  | PU_atom id -> Aast.PU_atom (snd id)
  | PU_identifier (e, id1, id2) ->
    Aast.PU_identifier ((p, Aast.CIexpr (on_expr e)), id1, id2)
  in
  (p, node)

and on_import_flavor f =
  match f with
  | Include -> Aast.Include
  | Require -> Aast.Require
  | IncludeOnce -> Aast.IncludeOnce
  | RequireOnce -> Aast.RequireOnce

and on_case c : Aast.case =
  match c with
  | Default b -> Aast.Default (on_block b)
  | Case (e, b) -> Aast.Case (on_expr e, on_block b)

and on_catch (id1, id2, b) : Aast.catch =
  let lid = Local_id.make_unscoped (snd id2) in
  (id1, ((fst id2), lid), on_block b)

and on_stmt (p, st) : Aast.stmt =
  p, on_stmt_ p st

and on_stmt_ p st :  Aast.stmt_ =
  match st with
  | Let (id, h, e) ->
    let lid = Local_id.make_unscoped (snd id) in
    Aast.Let ((p, lid), optional on_hint h, on_expr e)
  | Block sl                  -> Aast.Block (on_block sl)
  | Unsafe                    -> failwith "Unsafe statements should be removed in on_block"
  | Fallthrough               -> Aast.Fallthrough
  | Noop                      -> Aast.Noop
  | Markup (s, e)             -> Aast.Markup (s, optional on_expr e)
  | Break (Some _)            -> Errors.break_continue_n_not_supported p; Aast.Break
  | Break None                -> Aast.Break
  | Continue (Some _)         -> Errors.break_continue_n_not_supported p; Aast.Continue
  | Continue None             -> Aast.Continue
  | Throw e                   -> Aast.Throw (false, on_expr e)
  | Return e                  -> Aast.Return (optional on_expr e)
  | GotoLabel label           -> Aast.GotoLabel label
  | Goto label                -> Aast.Goto label
  | Awaitall (el, s)          -> Aast.Awaitall (on_awaitall el s)
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
  | Foreach (e, aw, ae, b)    -> Aast.Foreach (on_expr e, on_as_expr aw ae, on_block b)
  | Try (b, cl, fb)           -> Aast.Try (on_block b, on_list on_catch cl, on_block fb)
  | Def_inline d              -> Aast.Def_inline (on_def d)
  | Expr e                    -> Aast.Expr (on_expr e)

and on_block stmt_list : Aast.stmt list =
  match stmt_list with
  | [] -> []
  | (p, Unsafe) :: rest -> [p, Aast.Unsafe_block (on_block rest)]
  | x :: rest -> (on_stmt x) :: (on_block rest)

and on_tparam_constraint (kind, hint) : (constraint_kind * Aast.hint) =
  (kind, on_hint hint)

and on_tparam t : Aast.tparam =
  let attributes = on_list on_user_attribute t.tp_user_attributes in
  { Aast.tp_variance = t.tp_variance;
    tp_name = t.tp_name;
    tp_constraints = on_list on_tparam_constraint t.tp_constraints;
    tp_reified = reification t.tp_reified attributes;
    tp_user_attributes = attributes;
  }

and on_fun_param ?(trait_or_interface=false) param : Aast.fun_param =
  let p, name = param.param_id in
  if trait_or_interface && param.param_modifier <> None
  then Errors.trait_interface_constructor_promo p;
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

and determine_variadicity params =
  match params with
    | [] -> Aast.FVnonVariadic
    | [x] ->
      begin
        match x.param_is_variadic, x.param_id with
          | false, _ -> Aast.FVnonVariadic
          | true, (p, "...") -> Aast.FVellipsis p
          | true, _ -> Aast.FVvariadicArg (on_fun_param x)
      end
    | _ :: rl ->
      determine_variadicity rl

and on_user_attribute attribute : Aast.user_attribute =
  let ua_params = on_list on_expr attribute.ua_params in
  Aast.{ ua_name = attribute.ua_name; ua_params; }

and on_file_attribute (attribute:Ast.file_attributes) : Aast.file_attribute =
  Aast.
  { fa_user_attributes = on_list on_user_attribute attribute.fa_user_attributes
  ; fa_namespace = attribute.fa_namespace
  }

and on_fun f : Aast.fun_ =
  let body = on_block f.f_body in
  let body = {
    Aast.fb_ast = body;
    (* Still seems incorrect to have this as a Named body... *)
    Aast.fb_annotation = Aast.BodyNamingAnnotation.NamedWithUnsafeBlocks;
  } in
  let named_fun = {
    Aast.f_annotation = ();
    f_span = f.f_span;
    f_mode = f.f_mode;
    f_ret = (optional on_hint f.f_ret);
    f_name = f.f_name;
    f_tparams =
      on_list on_tparam f.f_tparams;
    f_where_constraints = on_list on_constr f.f_constrs;
    f_params = on_list on_fun_param f.f_params;
    f_body = body;
    f_fun_kind = f.f_fun_kind;
    f_variadic = determine_variadicity f.f_params;
    f_user_attributes = on_list on_user_attribute f.f_user_attributes;
    f_file_attributes = on_list on_file_attribute f.f_file_attributes;
    f_external = f.f_external;
    f_namespace = f.f_namespace;
    f_doc_comment = f.f_doc_comment;
    f_static = f.f_static;
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
  let tconst_type, abstract_kind =
    match tc.tconst_abstract, tc.tconst_constraint, tc.tconst_type with
    (* const type T;
     * const type T as int; *)
    | false, _, None ->
      Errors.not_abstract_without_typeconst tc.tconst_name;
      tc.tconst_constraint, Aast.TCConcrete (* this is a degenerate case, so safe to call it concrete *)
    (* const type T = int; *)
    | false, None, Some _ ->
      tc.tconst_type, Aast.TCConcrete
    (* const type T as num = int; *)
    | false, Some _, Some _ ->
      tc.tconst_type, Aast.TCPartiallyAbstract
    (* abstract const type T;
     * abstract const type T as num; *)
    | true, _, None ->
      (* Choice of tc.tconst_type based on the existing behavior, previously captured by the h, _ case
       * TODO: Change to tc.tconst_constraint and merge these two cases *)
      tc.tconst_type, Aast.TCAbstract (optional on_hint tc.tconst_type);
    (* abstract const type T = int;
     * abstract const type T as num = int; *)
    | true, _, Some _ ->
      tc.tconst_constraint, Aast.TCAbstract (optional on_hint tc.tconst_type) in

  Aast.{
    c_tconst_abstract = abstract_kind;
    c_tconst_name = tc.tconst_name;
    c_tconst_constraint = optional on_hint tc.tconst_constraint;
    c_tconst_type = optional on_hint tconst_type;
    c_tconst_user_attributes = on_list on_user_attribute tc.tconst_user_attributes;
  }

and on_class_var is_xhp h attrs kinds variadic doc_com (_, id, eopt) : Aast.class_var =
  let cv_final = List.mem Final kinds in
  let cv_visibility = List.fold_left
    begin
      fun acc k ->
        match k with
        | Private -> Aast.Private
        | Public -> Aast.Public
        | Protected -> Aast.Protected
        | _ -> acc
    end
    Aast.Public
    kinds in
  Aast.{
    cv_final;
    cv_is_xhp = is_xhp;
    cv_visibility;
    cv_type = h;
    cv_id = id;
    cv_expr = optional on_expr eopt;
    cv_user_attributes = attrs;
    cv_is_promoted_variadic = variadic;
    cv_doc_comment = doc_com
  }

(**
 * Doc comments are currently used for codegen but are not required for typing.
 * When used for codegen, doc comments are only used for the emission of the
 * first variable in a list of class vars declared together. For instance, $x:
 * `public int $x, $y;`
 *)
and on_class_vars_with_acc acc cvs =
  match cvs.cv_names with
  | cv :: rest ->
    let with_dc_name =
      on_class_var
        false
        (optional on_hint cvs.cv_hint)
        (on_list on_user_attribute cvs.cv_user_attributes)
        cvs.cv_kinds
        cvs.cv_is_promoted_variadic in
    let cv = with_dc_name cvs.cv_doc_comment cv in
    let acc = cv :: acc in
    on_list_append_acc acc (with_dc_name None) rest
  | [] -> acc

and on_xhp_attr (p, b, el) = (p, b, on_list on_expr el)

and on_constr (h1, k, h2) = (on_hint h1, k, on_hint h2)

and on_method_trait_resolution res : Aast.method_redeclaration =
  let acc = false, false, false, None in
  let final, abs, static, vis = List.fold_left kind acc res.mt_kind in
  let vis =
    match vis with
    | None ->
      Errors.method_needs_visibility (fst res.mt_name);
      Aast.Public
    | Some v -> v in
  Aast.{
    mt_final           = final;
    mt_abstract        = abs;
    mt_static          = static;
    mt_visibility      = vis;
    mt_name            = res.mt_name;
    mt_tparams         = on_list on_tparam res.mt_tparams;
    mt_where_constraints = on_list on_constr res.mt_constrs;
    mt_variadic        = determine_variadicity res.mt_params;
    mt_params          = on_list on_fun_param res.mt_params;
    mt_fun_kind        = res.mt_fun_kind;
    mt_ret             = optional on_hint res.mt_ret;
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
  | Private -> final, abs, static, Some Aast.Private
  | Public -> final, abs, static, Some Aast.Public
  | Protected -> final, abs, static, Some Aast.Protected

and on_method ?(trait_or_interface=false) m : Aast.method_ =
  let body = on_block m.m_body in
  let body = {
    Aast.fb_ast = body;
    (* Still seems incorrect to have this as a Named body... *)
    Aast.fb_annotation = Aast.BodyNamingAnnotation.NamedWithUnsafeBlocks;
  } in
  let acc = false, false, false, None in
  let final, abs, static, vis = List.fold_left kind acc m.m_kind in
  let vis =
    match vis with
    | None -> Errors.method_needs_visibility (fst m.m_name); Aast.Public
    | Some v -> v in
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
    m_variadic        = determine_variadicity m.m_params;
    m_params          = on_list (on_fun_param ~trait_or_interface) m.m_params;
    m_body            = body;
    m_fun_kind        = m.m_fun_kind;
    m_user_attributes = on_list on_user_attribute m.m_user_attributes;
    m_ret             = optional on_hint m.m_ret;
    m_external        = m.m_external;
    m_doc_comment     = m.m_doc_comment;
  }

and on_pu_mapping pum_atom mappings =
  let rec aux types exprs = function
    | (PUMappingType (id, hint)) :: tl ->
      aux ((id, on_hint hint) :: types) exprs tl
    | (PUMappingID (id, expr)) :: tl ->
      aux types ((id, on_expr expr) :: exprs) tl
    | [] -> (List.rev types, List.rev exprs) in
  let (pum_types, pum_exprs) = aux [] [] mappings in
  Aast.{ pum_atom; pum_types; pum_exprs }

and on_pu pu_name pu_is_final fields =
  let rec aux case_types case_values members = function
    | (PUCaseType id) :: tl ->
      aux (id :: case_types) case_values members tl
    | (PUCaseTypeExpr (h, id)) :: tl ->
      aux case_types ((id, on_hint h) :: case_values) members tl
    | (PUAtomDecl (id, maps)) :: tl ->
      let member = on_pu_mapping id maps in
      aux case_types case_values (member :: members) tl
    | [] -> (List.rev case_types, List.rev case_values, List.rev members) in
  let (pu_case_types, pu_case_values, pu_members) = aux [] [] [] fields in
  Aast.{ pu_name
       ; pu_is_final
       ; pu_case_types
       ; pu_case_values
       ; pu_members
       }

and on_class_body trait_or_interface cb =
  let reversed_body = List.fold_left (on_class_elt trait_or_interface) make_empty_class_body cb in
  { reversed_body with
    c_uses = List.rev reversed_body.c_uses;
    c_method_redeclarations = List.rev reversed_body.c_method_redeclarations;
    c_xhp_attr_uses = List.rev reversed_body.c_xhp_attr_uses;
    c_xhp_category = reversed_body.c_xhp_category;
    c_req_extends = List.rev reversed_body.c_req_extends;
    c_req_implements = List.rev reversed_body.c_req_implements;
    c_consts = List.rev reversed_body.c_consts;
    c_typeconsts = List.rev reversed_body.c_typeconsts;
    c_static_vars = List.rev reversed_body.c_static_vars;
    c_vars = List.rev reversed_body.c_vars;
    c_static_methods = List.rev reversed_body.c_static_methods;
    c_methods = List.rev reversed_body.c_methods;
    c_attributes = List.rev reversed_body.c_attributes;
    c_xhp_children = List.rev reversed_body.c_xhp_children;
    c_xhp_attrs = List.rev reversed_body.c_xhp_attrs;
    c_pu_enums = List.rev reversed_body.c_pu_enums;
  }

and on_class c : Aast.class_ =
  let c_tparams = {
    Aast.c_tparam_list = on_list on_tparam c.c_tparams;
    Aast.c_tparam_constraints = SMap.empty;
  } in
  let trait_or_interface = c.c_kind = Ctrait || c.c_kind = Cinterface in
  let body = on_class_body trait_or_interface c.c_body in
  let named_class =
  Aast.{
      c_annotation            = ();
      c_span                  = c.c_span;
      c_mode                  = c.c_mode;
      c_final                 = c.c_final;
      c_is_xhp                = c.c_is_xhp;
      c_kind                  = c.c_kind;
      c_name                  = c.c_name;
      c_tparams               = c_tparams;
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
      c_file_attributes       = on_list on_file_attribute c.c_file_attributes;
      c_namespace             = c.c_namespace;
      c_enum                  = optional on_enum c.c_enum;
      c_doc_comment           = c.c_doc_comment;
      c_pu_enums              = body.c_pu_enums
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
    cst_namespace = c.cst_namespace;
    cst_span = c.cst_span;
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
  | FileAttributes fa -> Aast.FileAttributes (on_file_attribute fa)

and on_program ast = on_list on_def ast

let convert ast : Aast.program = on_program ast
