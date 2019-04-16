(**
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Core_kernel
open Nast

let rec expr f (p, e) =
  f p, expr_ f e

and expr_ f = function
  | Any -> Any
  | Array afl -> Array (List.map afl (afield f))
  | Darray (tap, fl) -> Darray (tap, List.map fl (fun (e1, e2) -> expr f e1, expr f e2))
  | Varray (ta, el) -> Varray (ta, List.map el (expr f))
  | Shape sh -> Shape (shape f sh)
  | True -> True
  | False -> False
  | Int n -> Int n
  | Float n -> Float n
  | Null -> Null
  | String s -> String s
  | This -> This
  | Id sid -> Id (pstring f sid)
  | Lplaceholder pos -> Lplaceholder (f pos)
  | Dollardollar (p, id) -> Dollardollar (f p, id)
  | Lvar (p, id) -> Lvar (f p, id)
  | ImmutableVar (p, id) -> ImmutableVar (f p, id)
  | Fun_id sid -> Fun_id (pstring f sid)
  | Method_id (e, pstr) -> Method_id (expr f e, pstring f pstr)
  | Method_caller (sid, pstr) -> Method_caller (pstring f sid, pstring f pstr)
  | Smethod_id (sid, pstr) -> Smethod_id (pstring f sid, pstring f pstr)
  | Yield_break -> Yield_break
  | Yield e -> Yield (afield f e)
  | Yield_from e -> Yield_from (expr f e)
  | Await e -> Await (expr f e)
  | Suspend e -> Suspend (expr f e)
  | List el -> List (List.map el (expr f))
  | Assert (AE_assert ae) -> Assert (AE_assert (expr f ae))
  | Clone e -> Clone (expr f e)
  | Expr_list el -> Expr_list (List.map el (expr f))
  | Special_func sf -> Special_func (special_func f sf)
  | Obj_get (e1, e2, x) -> Obj_get (expr f e1, expr f e2, x)
  | Array_get (e1, e2) -> Array_get (expr f e1, Option.map e2 (expr f))
  | Class_get (cid, e) -> Class_get (class_id f cid, class_get_expr f e)
  | Class_const (cid, id) -> Class_const (class_id f cid, pstring f id)
  | Call (ct, e, hl, el, uel) ->
    Call (ct, expr f e, hl, List.map el (expr f), List.map uel (expr f))
  | String2 el -> String2 (List.map el (expr f))
  | PrefixedString (n, e) -> PrefixedString (n, (expr f e))
  | Pair (e1, e2) -> Pair (expr f e1, expr f e2)
  | Cast (h, e) -> Cast (hint f h, expr f e)
  | Unop (uop, e) -> Unop (uop, expr f e)
  | Binop (bop, e1, e2) -> Binop (bop, expr f e1, expr f e2)
  | Pipe ((p, id), e1, e2) -> Pipe ((f p, id), expr f e1, expr f e2)
  | Eif (e1, e2, e3) -> Eif (expr f e1, Option.map e2 (expr f), expr f e3)
  | InstanceOf (e1, e2) -> InstanceOf (expr f e1, class_id f e2)
  | Is (e, h) -> Is (expr f e, hint f h)
  | As (e, h, b) -> As (expr f e, hint f h, b)
  | Typename n -> Typename (pstring f n)
  | New (cid, tl, el, uel, ctor_annot) ->
    New (class_id f cid, tl, List.map el (expr f), List.map uel (expr f), ctor_annot)
  | Record (cid, fl) ->
    Record (class_id f cid, List.map fl (fun (e1, e2) -> expr f e1, expr f e2))
  | Efun (fun_, idl) ->
    (* properly handling this would involve writing the mapper for all
     * statements *)
    Errors.internal_error (fst fun_.f_name)
      "Nast_pos_mapper cannot handle lambdas";
    Efun (fun_, idl)
  | Xml (sid, attrl, el) ->
    Xml (pstring f sid, attr_list f attrl, List.map el (expr f))
  | Unsafe_expr e -> Unsafe_expr (expr f e)
  | Callconv (kind, e) -> Callconv (kind, expr f e)
  | ValCollection (s, ta, el) -> ValCollection (s, ta, List.map el (expr f))
  | KeyValCollection (s, tap, fl) ->
    KeyValCollection (s, tap, List.map fl (fun (e1, e2) -> expr f e1, expr f e2))
  | Omitted -> Omitted
  | Lfun (f, idl) ->
    Errors.internal_error (fst f.f_name)
      "Nast_pos_mapper cannot handle lambdas";
    Lfun (f, idl)
  | Collection _
  | BracedExpr _
  | ParenthesizedExpr _
  | Import _ -> failwith "NAST should not contain these nodes"
  | PU_atom s -> PU_atom s
  | PU_identifier (cid, s1, s2) ->
    PU_identifier (class_id f cid, s1, s2)

and class_get_expr f = function
  | CGstring s -> CGstring (pstring f s)
  | CGexpr e -> CGexpr (expr f e)
and afield f = function
  | AFvalue e -> AFvalue (expr f e)
  | AFkvalue (e1, e2) ->
    let e1 : Nast.expr = expr f e1 in
    let e2 = expr f e2 in
    AFkvalue (e1, e2)

and shape f sm =
  List.fold_left
    ~f:begin fun acc (sf, e) ->
      let sf = shape_field f sf in
      let e = expr f e in
      (sf, e) :: acc
    end
    ~init:[]
    sm
  |> List.rev

and shape_field f = function
  | Ast.SFlit_int pstr -> Ast.SFlit_int (pstring f pstr)
  | Ast.SFlit_str pstr -> Ast.SFlit_str (pstring f pstr)
  | Ast.SFclass_const (sid, pstr) ->
    Ast.SFclass_const (pstring f sid, pstring f pstr)

and pstring f (p, s) =
  f p, s

and special_func f = function
  | Gena e -> Gena (expr f e)
  | Genva el -> Genva (List.map el (expr f))
  | Gen_array_rec e -> Gen_array_rec (expr f e)

and class_id f (pos, ci) = f pos, class_id_ f ci

and class_id_ f = function
  | CIparent -> CIparent
  | CIself -> CIself
  | CIstatic -> CIstatic
  | CIexpr e -> CIexpr (expr f e)
  | CI sid -> CI (pstring f sid)

and hint f (p, h) = f p, hint_ f h

and hint_ f = function
  | Hnothing -> Hnothing
  | Hdynamic -> Hdynamic
  | Hany -> Hany
  | Hmixed -> Hmixed
  | Hnonnull -> Hnonnull
  | Hthis -> Hthis
  | Htuple hl -> Htuple (List.map hl (hint f))
  | Habstr s ->
    Habstr s
  | Harray (h1, h2) ->
    Harray (Option.map h1 (hint f), Option.map h2 (hint f))
  | Hdarray (h1, h2) ->
    Hdarray (hint f h1, hint f h2)
  | Hvarray h -> Hvarray (hint f h)
  | Hvarray_or_darray h -> Hvarray_or_darray (hint f h)
  | Hprim tprim -> Hprim tprim
  | Hoption h -> Hoption (hint f h)
  | Hlike h -> Hlike (hint f h)
  | Hfun (is_reactive, is_coroutine, hl, kl, m, b, h, rm) ->
    Hfun (is_reactive, is_coroutine, List.map hl (hint f), kl, m, b, hint f h, rm)
  | Happly (sid, hl) -> Happly (pstring f sid, List.map hl (hint f))
  | Hshape nast_shape_info ->
    let on_shape_field_info shape_field_info =
      let map_over_shape_field_info ~fn shape_field_info =
        { shape_field_info with
          sfi_hint = fn shape_field_info.sfi_hint;
          sfi_name = shape_field f shape_field_info.sfi_name;
        } in
      let h = hint f in
      map_over_shape_field_info ~fn:h shape_field_info in
    let nsi_field_map =
      List.map
        ~f:on_shape_field_info
        nast_shape_info.nsi_field_map in
    Hshape { nast_shape_info with nsi_field_map }
  | Haccess (h, sids) -> Haccess (hint f h, List.map sids (pstring f))
  | Hsoft h -> Hsoft (hint f h)

and attr_list f attrl =
  List.map attrl begin fun attr -> match attr with
   | Xhp_simple (pstr, e) -> Xhp_simple (pstring f pstr, expr f e)
   | Xhp_spread e -> Xhp_spread (expr f e)
  end
