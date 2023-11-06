(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Hh_prelude
module SN = Naming_special_names
module Fmt = Format_helpers

let strip_ns obj_name =
  match String.rsplit2 obj_name ~on:'\\' with
  | Some (_, name) -> name
  | None -> obj_name

let pp_paramkind ppf =
  Ast_defs.(
    function
    | Pinout _ -> Fmt.string ppf "inout"
    | Pnormal -> ())

let any_type_name = "ANY_TYPE"

let rec pp_hint ~is_ctx ppf (pos, hint_) =
  match hint_ with
  | Aast.Hany
  | Aast.Herr ->
    Fmt.string ppf any_type_name
  | Aast.Hthis -> Fmt.string ppf "this"
  | Aast.Hdynamic -> Fmt.string ppf "dynamic"
  | Aast.Hnothing -> Fmt.string ppf "nothing"
  | Aast.Hmixed -> Fmt.string ppf "mixed"
  | Aast.Hwildcard -> Fmt.string ppf "_"
  | Aast.Hnonnull -> Fmt.string ppf "nonnull"
  | Aast.Hvar name -> Fmt.string ppf name
  | Aast.Hfun_context name -> Fmt.(prefix (const string "ctx ") string) ppf name
  | Aast.Hoption hint ->
    Fmt.(prefix (const string "?") @@ pp_hint ~is_ctx:false) ppf hint
  | Aast.Hlike hint ->
    Fmt.(prefix (const string "~") @@ pp_hint ~is_ctx:false) ppf hint
  | Aast.Hsoft hint ->
    Fmt.(prefix (const string "@") @@ pp_hint ~is_ctx:false) ppf hint
  | Aast.Htuple hints ->
    Fmt.(parens @@ list ~sep:comma @@ pp_hint ~is_ctx:false) ppf hints
  | Aast.Hunion hints ->
    Fmt.(parens @@ list ~sep:vbar @@ pp_hint ~is_ctx:false) ppf hints
  | Aast.Hintersection hints when is_ctx ->
    Fmt.(brackets @@ list ~sep:comma @@ pp_hint ~is_ctx:true) ppf hints
  | Aast.Hintersection hints ->
    Fmt.(parens @@ list ~sep:amp @@ pp_hint ~is_ctx:false) ppf hints
  | Aast.Hprim prim -> Fmt.string ppf (Aast_defs.string_of_tprim prim)
  | Aast.Haccess (root, ids) ->
    Fmt.(
      pair ~sep:dbl_colon (pp_hint ~is_ctx:false) @@ list ~sep:dbl_colon string)
      ppf
      (root, List.map ~f:snd ids)
  | Aast.Hrefinement (ty, members) ->
    let pp_bounds ~is_ctx ppf bounds =
      let bound ppf (kind, hint) =
        Fmt.string
          ppf
          (match kind with
          | `E -> "= "
          | `L -> "as "
          | `U -> "super ");
        pp_hint ~is_ctx ppf hint
      in
      Fmt.list ~sep:Fmt.(const string " ") bound ppf bounds
    in
    let member ppf = function
      | Aast.Rtype (ident, ref) ->
        Fmt.string ppf ("type " ^ snd ident ^ " ");
        pp_bounds
          ~is_ctx:false
          ppf
          (match ref with
          | Aast.TRexact hint -> [(`E, hint)]
          | Aast.TRloose { Aast.tr_lower; tr_upper } ->
            List.map tr_lower ~f:(fun x -> (`L, x))
            @ List.map tr_upper ~f:(fun x -> (`U, x)))
      | Aast.Rctx (ident, ref) ->
        Fmt.string ppf ("ctx " ^ snd ident ^ " ");
        pp_bounds
          ~is_ctx:true
          ppf
          (match ref with
          | Aast.CRexact hint -> [(`E, hint)]
          | Aast.CRloose { Aast.cr_lower; cr_upper } ->
            let opt_map = Option.value_map ~default:[] in
            opt_map cr_lower ~f:(fun x -> [(`L, x)])
            @ opt_map cr_upper ~f:(fun x -> [(`U, x)]))
    in
    Fmt.(suffix with_ (pp_hint ~is_ctx:false)) ppf ty;
    Fmt.(braces (list ~sep:semi_sep member)) ppf members
  | Aast.Hvec_or_dict (None, vhint) ->
    Fmt.(prefix (const string "vec_or_dict") @@ angles @@ pp_hint ~is_ctx:false)
      ppf
      vhint
  | Aast.Hvec_or_dict (Some khint, vhint) ->
    Fmt.(
      prefix (const string "vec_or_dict")
      @@ angles
      @@ pair ~sep:comma (pp_hint ~is_ctx:false) (pp_hint ~is_ctx:false))
      ppf
      (khint, vhint)
  | Aast.Happly ((p, name), hs) when is_ctx ->
    pp_hint ~is_ctx:false ppf (pos, Aast.Happly ((p, strip_ns name), hs))
  | Aast.Habstr (name, [])
  | Aast.Happly ((_, name), []) ->
    Fmt.string ppf name
  | Aast.Hclass_args h ->
    Fmt.(prefix (const string "class") @@ angles @@ pp_hint ~is_ctx:false) ppf h
  | Aast.Habstr (name, hints)
  | Aast.Happly ((_, name), hints) ->
    Fmt.(
      prefix (const string name)
      @@ angles
      @@ list ~sep:comma
      @@ pp_hint ~is_ctx:false)
      ppf
      hints
  | Aast.(
      Hfun
        {
          hf_param_tys;
          hf_param_info;
          hf_variadic_ty;
          hf_return_ty;
          hf_ctxs;
          _;
        }) ->
    let hf_param_kinds =
      List.map hf_param_info ~f:(fun i ->
          Option.bind i ~f:(fun i ->
              match i.Aast.hfparam_kind with
              | Ast_defs.Pnormal -> None
              | Ast_defs.Pinout p -> Some (Ast_defs.Pinout p)))
    in
    let pp_typed_param ppf kp =
      Fmt.(
        pair ~sep:nop (option @@ suffix sp pp_paramkind)
        @@ pp_hint ~is_ctx:false)
        ppf
        kp
    in
    let pp_fun_params ppf (ps, v) =
      Fmt.(
        parens
        @@ pair
             ~sep:nop
             (list ~sep:comma pp_typed_param)
             (option @@ surround ", " "..." @@ pp_hint ~is_ctx:false))
        ppf
        (ps, v)
    in
    let all_params =
      (List.zip_exn hf_param_kinds hf_param_tys, hf_variadic_ty)
    in
    Fmt.(
      parens
      @@ pair
           ~sep:colon
           (prefix (const string "function")
           @@ pair ~sep:nop pp_fun_params (option pp_contexts))
      @@ pp_hint ~is_ctx:false)
      ppf
      ((all_params, hf_ctxs), hf_return_ty)
  | Aast.(Hshape { nsi_allows_unknown_fields; nsi_field_map = [] }) ->
    Fmt.(
      prefix (const string "shape")
      @@ parens
      @@ cond ~pp_t:(const string "...") ~pp_f:nop)
      ppf
      nsi_allows_unknown_fields
  | Aast.(Hshape { nsi_allows_unknown_fields; nsi_field_map }) ->
    Fmt.(
      prefix (const string "shape")
      @@ parens
      @@ pair
           ~sep:nop
           (list ~sep:comma pp_shape_field)
           (cond ~pp_t:(const string ", ...") ~pp_f:nop))
      ppf
      (nsi_field_map, nsi_allows_unknown_fields)

and pp_shape_field ppf Aast.{ sfi_optional; sfi_name; sfi_hint } =
  Fmt.(
    pair
      ~sep:fat_arrow
      (pair
         ~sep:nop
         (cond ~pp_t:(const string "?") ~pp_f:nop)
         pp_shape_field_name)
    @@ pp_hint ~is_ctx:false)
    ppf
    ((sfi_optional, sfi_name), sfi_hint)

and pp_shape_field_name ppf = function
  | Ast_defs.SFlit_int (_, s) -> Fmt.string ppf s
  | Ast_defs.SFlit_str (_, s) -> Fmt.(quote string) ppf s
  | Ast_defs.SFclass_const ((_, c), (_, s)) ->
    Fmt.(pair ~sep:dbl_colon string string) ppf (c, s)

and pp_contexts ppf (_, ctxts) =
  Fmt.(brackets @@ list ~sep:comma @@ pp_hint ~is_ctx:true) ppf ctxts

let rec pp_binop ppf = function
  | Ast_defs.Plus -> Fmt.string ppf "+"
  | Ast_defs.Minus -> Fmt.string ppf "-"
  | Ast_defs.Star -> Fmt.string ppf "*"
  | Ast_defs.Slash -> Fmt.string ppf "/"
  | Ast_defs.Eqeq -> Fmt.string ppf "=="
  | Ast_defs.Eqeqeq -> Fmt.string ppf "==="
  | Ast_defs.Starstar -> Fmt.string ppf "**"
  | Ast_defs.Diff -> Fmt.string ppf "diff"
  | Ast_defs.Diff2 -> Fmt.string ppf "diff2"
  | Ast_defs.Ampamp -> Fmt.string ppf "&&"
  | Ast_defs.Barbar -> Fmt.string ppf "||"
  | Ast_defs.Lt -> Fmt.string ppf "<"
  | Ast_defs.Lte -> Fmt.string ppf "<="
  | Ast_defs.Gt -> Fmt.string ppf ">"
  | Ast_defs.Gte -> Fmt.string ppf ">="
  | Ast_defs.Dot -> Fmt.string ppf "."
  | Ast_defs.Amp -> Fmt.string ppf "&"
  | Ast_defs.Bar -> Fmt.string ppf "|"
  | Ast_defs.Ltlt -> Fmt.string ppf "<<"
  | Ast_defs.Gtgt -> Fmt.string ppf ">>"
  | Ast_defs.Percent -> Fmt.string ppf "%"
  | Ast_defs.Xor -> Fmt.string ppf "^"
  | Ast_defs.Cmp -> Fmt.string ppf "<=>"
  | Ast_defs.QuestionQuestion -> Fmt.string ppf "??"
  | Ast_defs.Eq (Some op) -> Fmt.(suffix (const string "=") pp_binop) ppf op
  | Ast_defs.Eq _ -> Fmt.string ppf "="

let pp_unop ppf op =
  match op with
  | Ast_defs.Utild -> Fmt.string ppf "~"
  | Ast_defs.Unot -> Fmt.string ppf "!"
  | Ast_defs.Uplus -> Fmt.string ppf "+"
  | Ast_defs.Uminus -> Fmt.string ppf "-"
  | Ast_defs.Uincr
  | Ast_defs.Upincr ->
    Fmt.string ppf "++"
  | Ast_defs.Udecr
  | Ast_defs.Updecr ->
    Fmt.string ppf "--"
  | Ast_defs.Usilence -> Fmt.string ppf "@"

let is_postfix_unop = function
  | Ast_defs.Updecr
  | Ast_defs.Upincr ->
    true
  | _ -> false

let pp_targ ppf (_, hint) = pp_hint ~is_ctx:false ppf hint

let pp_targs ppf = function
  | [] -> ()
  | targs -> Fmt.(angles @@ list ~sep:comma pp_targ) ppf targs

let pp_vc_kind ppf = function
  | Aast_defs.Vector -> Fmt.string ppf "Vector"
  | Aast_defs.ImmVector -> Fmt.string ppf "ImmVector"
  | Aast_defs.Vec -> Fmt.string ppf "vec"
  | Aast_defs.Set -> Fmt.string ppf "Set"
  | Aast_defs.ImmSet -> Fmt.string ppf "ImmSet"
  | Aast_defs.Keyset -> Fmt.string ppf "keyset"

let pp_kvc_kind ppf = function
  | Aast_defs.Dict -> Fmt.string ppf "dict"
  | Aast_defs.Map -> Fmt.string ppf "Map"
  | Aast_defs.ImmMap -> Fmt.string ppf "ImmMap"

let pp_lid ppf lid =
  Fmt.(prefix (const string "$") string) ppf @@ Local_id.get_name lid

let rec pp_expr ppf (_, _, expr_) = pp_expr_ ppf expr_

and pp_expr_ ppf = function
  | Aast.Darray (kv_ty_opt, kvs) ->
    Fmt.(
      prefix (const string "darray")
      @@ pair
           ~sep:nop
           (option @@ angles @@ pair ~sep:comma pp_targ pp_targ)
           (brackets @@ list ~sep:comma @@ pair ~sep:fat_arrow pp_expr pp_expr))
      ppf
      (kv_ty_opt, kvs)
  | Aast.Varray (k_ty_opt, ks) ->
    Fmt.(
      prefix (const string "varray")
      @@ pair
           ~sep:nop
           (option @@ angles pp_targ)
           (brackets @@ list ~sep:comma pp_expr))
      ppf
      (k_ty_opt, ks)
  | Aast.Shape flds ->
    Fmt.(
      prefix (const string "shape")
      @@ parens
      @@ list ~sep:comma
      @@ pair ~sep:fat_arrow pp_shape_field_name pp_expr)
      ppf
      flds
  | Aast.ValCollection ((_, kind), targ_opt, exprs) ->
    let delim =
      match kind with
      | Aast_defs.Keyset
      | Aast_defs.Vec ->
        Fmt.brackets
      | _ -> Fmt.braces
    in
    Fmt.(
      pair ~sep:nop pp_vc_kind
      @@ pair
           ~sep:nop
           (option @@ angles @@ pp_targ)
           (delim @@ list ~sep:comma pp_expr))
      ppf
      (kind, (targ_opt, exprs))
  | Aast.KeyValCollection ((_, kind), targs_opt, flds) ->
    let delim =
      match kind with
      | Aast_defs.Dict -> Fmt.brackets
      | _ -> Fmt.braces
    in
    Fmt.(
      pair ~sep:nop pp_kvc_kind
      @@ pair
           ~sep:nop
           (option @@ angles @@ pair ~sep:comma pp_targ pp_targ)
           (delim @@ list ~sep:comma @@ pair ~sep:fat_arrow pp_expr pp_expr))
      ppf
      (kind, (targs_opt, flds))
  | Aast.Null -> Fmt.string ppf "null"
  | Aast.This -> Fmt.string ppf "this"
  | Aast.True -> Fmt.string ppf "true"
  | Aast.False -> Fmt.string ppf "false"
  | Aast.Id (_, id) -> Fmt.string ppf id
  | Aast.Lvar (_, lid) -> pp_lid ppf lid
  | Aast.Dollardollar _ -> Fmt.string ppf "$$"
  | Aast.Clone expr -> Fmt.(prefix (const string "clone") pp_expr) ppf expr
  | Aast.Array_get (arr_expr, idx_expr_opt) ->
    Fmt.(pair ~sep:nop pp_expr @@ brackets @@ option pp_expr)
      ppf
      (arr_expr, idx_expr_opt)
  | Aast.(Obj_get (obj_expr, get_expr, OG_nullsafe, Is_method)) ->
    Fmt.(pair ~sep:arrow (suffix (const string "?") pp_expr) pp_expr)
      ppf
      (obj_expr, get_expr)
  | Aast.(Obj_get (obj_expr, get_expr, OG_nullsafe, _)) ->
    Fmt.(parens @@ pair ~sep:arrow (suffix (const string "?") pp_expr) pp_expr)
      ppf
      (obj_expr, get_expr)
  | Aast.(Obj_get (obj_expr, get_expr, _, Is_method)) ->
    Fmt.(pair ~sep:arrow pp_expr pp_expr) ppf (obj_expr, get_expr)
  | Aast.(Obj_get (obj_expr, get_expr, _, _)) ->
    Fmt.(parens @@ pair ~sep:arrow pp_expr pp_expr) ppf (obj_expr, get_expr)
  | Aast.(Class_get (class_id, class_get_expr, Is_method)) ->
    Fmt.(pair ~sep:dbl_colon pp_class_id pp_class_get_expr)
      ppf
      (class_id, class_get_expr)
  | Aast.Class_get (class_id, class_get_expr, _) ->
    Fmt.(parens @@ pair ~sep:dbl_colon pp_class_id pp_class_get_expr)
      ppf
      (class_id, class_get_expr)
  | Aast.Class_const (class_id, (_, cname)) ->
    Fmt.(pair ~sep:dbl_colon pp_class_id string) ppf (class_id, cname)
  | Aast.(Call { func; targs; args; unpacked_arg }) ->
    Fmt.(pair ~sep:nop pp_expr @@ pair ~sep:nop pp_targs pp_arg_exprs)
      ppf
      (func, (targs, (args, unpacked_arg)))
  | Aast.FunctionPointer (id, targs) ->
    Fmt.(pair ~sep:nop pp_function_ptr_id (angles @@ list ~sep:comma pp_targ))
      ppf
      (id, targs)
  | Aast.Int str
  | Aast.Float str ->
    Fmt.string ppf str
  | Aast.String str ->
    Fmt.(quote string) ppf
    @@ String.substr_replace_all ~pattern:"'" ~with_:"\\'" str
  | Aast.String2 exprs -> Fmt.(quote @@ list ~sep:sp pp_expr) ppf exprs
  | Aast.PrefixedString (pfx, expr) ->
    Fmt.(pair ~sep:nop string @@ quote pp_expr) ppf (pfx, expr)
  | Aast.Yield afield ->
    Fmt.(prefix (const string "yield") pp_afield) ppf afield
  | Aast.Await expr -> Fmt.(prefix (const string "await") pp_expr) ppf expr
  | Aast.ReadonlyExpr expr ->
    Fmt.(prefix (const string "readonly") pp_expr) ppf expr
  | Aast.List exprs ->
    Fmt.(prefix (const string "list") @@ parens @@ list ~sep:comma pp_expr)
      ppf
      exprs
  | Aast.Tuple exprs ->
    Fmt.(prefix (const string "tuple") @@ parens @@ list ~sep:comma pp_expr)
      ppf
      exprs
  | Aast.Cast (hint, expr) ->
    Fmt.(pair ~sep:nop (parens @@ pp_hint ~is_ctx:false) pp_expr)
      ppf
      (hint, expr)
  | Aast.Unop (unop, expr) when is_postfix_unop unop ->
    Fmt.(pair ~sep:nop pp_expr pp_unop) ppf (expr, unop)
  | Aast.Unop (unop, expr) ->
    Fmt.(pair ~sep:nop pp_unop pp_expr) ppf (unop, expr)
  | Aast.(Binop { bop; lhs; rhs }) ->
    Fmt.(pair ~sep:sp pp_expr @@ pair ~sep:sp pp_binop pp_expr)
      ppf
      (lhs, (bop, rhs))
  | Aast.Pipe (_lid, e1, e2) ->
    Fmt.(pair ~sep:(const string " |> ") pp_expr pp_expr) ppf (e1, e2)
  | Aast.Eif (cond, Some texpr, fexpr) ->
    Fmt.(
      pair ~sep:(const string " ? ") pp_expr @@ pair ~sep:colon pp_expr pp_expr)
      ppf
      (cond, (texpr, fexpr))
  | Aast.Eif (cond, _, expr) ->
    Fmt.(pair ~sep:(const string " ?: ") pp_expr pp_expr) ppf (cond, expr)
  | Aast.Is (expr, hint) ->
    Fmt.(pair ~sep:(const string " is ") pp_expr @@ pp_hint ~is_ctx:false)
      ppf
      (expr, hint)
  | Aast.As Aast.{ expr; hint; is_nullable = false; enforce_deep = _ } ->
    Fmt.(pair ~sep:(const string " as ") pp_expr @@ pp_hint ~is_ctx:false)
      ppf
      (expr, hint)
  | Aast.As Aast.{ expr; hint; is_nullable = true; enforce_deep = _ } ->
    Fmt.(pair ~sep:(const string " ?as ") pp_expr @@ pp_hint ~is_ctx:false)
      ppf
      (expr, hint)
  | Aast.Upcast (expr, hint) ->
    Fmt.(pair ~sep:(const string " upcast ") pp_expr @@ pp_hint ~is_ctx:false)
      ppf
      (expr, hint)
  | Aast.New (class_id, targs, exprs, expr_opt, _) ->
    Fmt.(
      prefix (const string "new")
      @@ pair ~sep:nop pp_class_id
      @@ pair ~sep:nop pp_targs pp_arg_exprs)
      ppf
      ( class_id,
        (targs, (List.map ~f:(fun e -> (Ast_defs.Pnormal, e)) exprs, expr_opt))
      )
  | Aast.Lplaceholder _ -> Fmt.string ppf "$_"
  | Aast.Pair (targs_opt, fst, snd) ->
    Fmt.(
      prefix (const string "Pair")
      @@ pair
           ~sep:nop
           (option @@ angles @@ pair ~sep:comma pp_targ pp_targ)
           (braces @@ pair ~sep:comma pp_expr pp_expr))
      ppf
      (targs_opt, (fst, snd))
  | Aast.Hole (expr, _, _, _) -> pp_expr ppf expr
  | Aast.EnumClassLabel (opt_sid, name) -> begin
    match opt_sid with
    | None -> Fmt.(prefix dbl_hash string) ppf name
    | Some (_, class_name) ->
      Fmt.(pair ~sep:dbl_hash Fmt.string string) ppf (class_name, name)
  end
  | Aast.Invalid (Some expr) -> pp_expr ppf expr
  | Aast.Package (_, id) -> Fmt.string ppf id
  | Aast.Nameof cid -> Fmt.(prefix (const string "nameof") pp_class_id) ppf cid
  | Aast.Invalid _
  | Aast.Efun _
  | Aast.Lfun _
  | Aast.Xml _
  | Aast.Import _
  | Aast.Collection _
  | Aast.ExpressionTree _
  | Aast.Method_caller _
  | Aast.ET_Splice _
  | Aast.Omitted ->
    ()

and pp_arg_exprs ppf (exprs, expr_opt) =
  match exprs with
  | [] ->
    Fmt.(parens @@ option @@ prefix (const string "...") pp_expr) ppf expr_opt
  | _ ->
    Fmt.(
      parens
      @@ pair
           ~sep:comma
           (list ~sep:comma pp_arg)
           (option @@ prefix (const string "...") pp_expr))
      ppf
      (exprs, expr_opt)

and pp_arg ppf (pk, e) =
  match pk with
  | Ast_defs.Pnormal -> pp_expr ppf e
  | Ast_defs.Pinout _ -> Fmt.(pair ~sep:sp pp_paramkind pp_expr) ppf (pk, e)

and pp_afield ppf = function
  | Aast.AFvalue expr -> pp_expr ppf expr
  | Aast.AFkvalue (key_expr, val_expr) ->
    Fmt.(pair ~sep:fat_arrow pp_expr pp_expr) ppf (key_expr, val_expr)

and pp_class_id ppf (_, _, class_id_) =
  match class_id_ with
  | Aast.CIparent -> Fmt.string ppf "parent"
  | Aast.CIstatic -> Fmt.string ppf "static"
  | Aast.CIself -> Fmt.string ppf "self"
  | Aast.CI (_, name) -> Fmt.string ppf name
  | Aast.CIexpr expr -> pp_expr ppf expr

and pp_class_get_expr ppf = function
  | Aast.CGexpr expr -> pp_expr ppf expr
  | Aast.CGstring (_, name) -> Fmt.string ppf name

and pp_function_ptr_id ppf = function
  | Aast.FP_id (_, name) -> Fmt.string ppf name
  | Aast.FP_class_const (class_id, (_, str)) ->
    Fmt.(pair ~sep:dbl_colon pp_class_id string) ppf (class_id, str)

let pp_user_attr ppf Aast.{ ua_name = (_, nm); ua_params; _ } =
  match ua_params with
  | [] -> Fmt.string ppf nm
  | _ ->
    Fmt.(pair ~sep:nop string @@ parens @@ list ~sep:comma pp_expr)
      ppf
      (nm, ua_params)

let pp_user_attrs ppf = function
  | [] -> ()
  | rs -> Fmt.(angles @@ angles @@ list ~sep:comma pp_user_attr) ppf rs

let pp_variance ppf =
  Ast_defs.(
    function
    | Covariant -> Fmt.string ppf "+"
    | Contravariant -> Fmt.string ppf "-"
    | Invariant -> ())

let pp_constraint_kind ppf =
  Ast_defs.(
    function
    | Constraint_as -> Fmt.string ppf "as"
    | Constraint_eq -> Fmt.string ppf "="
    | Constraint_super -> Fmt.string ppf "super")

let pp_constraint ppf (kind, hint) =
  Format.(fprintf ppf {|%a %a|})
    pp_constraint_kind
    kind
    (pp_hint ~is_ctx:false)
    hint

let pp_tp_reified ppf =
  Aast.(
    function
    | Erased -> ()
    | SoftReified
    | Reified ->
      Fmt.string ppf "reify")

let rec pp_tparam
    ppf
    Aast.
      {
        tp_variance;
        tp_name = (_, name);
        tp_parameters;
        tp_constraints;
        tp_reified;
        tp_user_attributes;
      } =
  Format.(
    fprintf
      ppf
      {|%a %a %a%s %a %a |}
      pp_user_attrs
      tp_user_attributes
      pp_tp_reified
      tp_reified
      pp_variance
      tp_variance
      name
      pp_tparams
      tp_parameters
      Fmt.(list ~sep:sp pp_constraint)
      tp_constraints)

and pp_tparams ppf ps =
  match
    List.filter
      ~f:(fun Aast.{ tp_name = (_, name); _ } ->
        not (SN.Coeffects.is_generated_generic name))
      ps
  with
  | [] -> ()
  | ps -> Fmt.(angles @@ list ~sep:comma pp_tparam) ppf ps

let pp_type_hint ~is_ret_type ppf (_, hint) =
  if is_ret_type then
    Fmt.(option @@ prefix (const string ": ") @@ pp_hint ~is_ctx:false) ppf hint
  else
    Fmt.(option @@ suffix sp @@ pp_hint ~is_ctx:false) ppf hint
