(*
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

(*****************************************************************************)
(* Converts a type hint into a type  *)
(*****************************************************************************)
open Hh_prelude
open Aast
open Typing_defs
module SN = Naming_special_names

(* Unpacking a hint for typing *)
let rec hint env (p, h) =
  let h = hint_ p env h in
  mk (Typing_reason.Rhint (Decl_env.make_decl_pos env p), h)

and shape_field_info_to_shape_field_type
    env { sfi_optional; sfi_hint; sfi_name = _ } =
  { sft_optional = sfi_optional; sft_ty = hint env sfi_hint }

and aast_user_attribute_to_decl_user_attribute env { ua_name; ua_params } =
  {
    Typing_defs.ua_name = Decl_env.make_decl_posed env ua_name;
    ua_classname_params =
      List.filter_map ua_params ~f:(function
          | (_, Class_const ((_, CI (_, cls)), (_, name)))
            when String.equal name SN.Members.mClass ->
            Some cls
          | _ -> None);
  }

and aast_contexts_to_decl_capability env ctxs default_pos :
    Aast.pos * decl_ty capability =
  match ctxs with
  | Some (pos, hl) ->
    let hl = List.map ~f:(hint env) hl in
    ( pos,
      CapTy
        (Typing_make_type.intersection
           (Reason.Rhint (Decl_env.make_decl_pos env pos))
           hl) )
  | None -> (default_pos, CapDefaults (Decl_env.make_decl_pos env default_pos))

and aast_tparam_to_decl_tparam env t =
  {
    tp_variance = t.Aast.tp_variance;
    tp_name = Decl_env.make_decl_posed env t.Aast.tp_name;
    tp_tparams =
      List.map ~f:(aast_tparam_to_decl_tparam env) t.Aast.tp_parameters;
    tp_constraints =
      List.map ~f:(Tuple.T2.map_snd ~f:(hint env)) t.Aast.tp_constraints;
    tp_reified = t.Aast.tp_reified;
    tp_user_attributes =
      List.map
        ~f:(aast_user_attribute_to_decl_user_attribute env)
        t.Aast.tp_user_attributes;
  }

and hint_ p env = function
  | Hany -> Typing_defs.make_tany ()
  | Herr -> Terr
  | Hmixed -> Tmixed
  | Hnonnull -> Tnonnull
  | Hthis -> Tthis
  | Hdynamic -> Tdynamic
  | Hnothing -> Tunion []
  | Hdarray (h1, h2) -> Tdarray (hint env h1, hint env h2)
  | Hvarray h -> Tvarray (hint env h)
  | Hvarray_or_darray (h1, h2) ->
    let t1 =
      match h1 with
      | Some h -> hint env h
      | None ->
        mk
          ( Typing_reason.Rvarray_or_darray_key (Decl_env.make_decl_pos env p),
            Tprim Aast.Tarraykey )
    in
    Tvarray_or_darray (t1, hint env h2)
  | Hvec_or_dict (h1, h2) ->
    let t1 =
      match h1 with
      | Some h -> hint env h
      | None ->
        mk
          ( Typing_reason.Rvec_or_dict_key (Decl_env.make_decl_pos env p),
            Tprim Aast.Tarraykey )
    in
    Tvec_or_dict (t1, hint env h2)
  | Hprim p -> Tprim p
  | Habstr (x, argl) ->
    let argl = List.map argl (hint env) in
    Tgeneric (x, argl)
  | Hoption h ->
    let h = hint env h in
    Toption h
  | Hlike h -> Tlike (hint env h)
  | Hfun
      {
        hf_is_readonly = ro;
        hf_param_tys = hl;
        hf_param_info = pil;
        hf_variadic_ty = vh;
        hf_ctxs = ctxs;
        hf_return_ty = h;
        hf_is_readonly_return = readonly_ret;
      } ->
    let make_param ((p, _) as x) param_info =
      let (readonly, kind) =
        match param_info with
        | Some p ->
          let readonly =
            match p.hfparam_readonlyness with
            | Some Ast_defs.Readonly -> true
            | _ -> false
          in
          let param_kind = get_param_mode p.hfparam_kind in
          (readonly, param_kind)
        | None -> (false, FPnormal)
      in
      {
        fp_pos = Decl_env.make_decl_pos env p;
        fp_name = None;
        fp_type = possibly_enforced_hint env x;
        fp_flags =
          make_fp_flags
            ~mode:kind
            ~accept_disposable:false
            ~has_default:
              false
              (* Currently do not support external and cancall on parameters of function parameters *)
            ~ifc_external:false
            ~ifc_can_call:false
            ~is_atom:false
            ~readonly;
      }
    in
    let readonly_opt ro =
      match ro with
      | Some Ast_defs.Readonly -> true
      | None -> false
    in
    let paraml = List.map2_exn hl pil ~f:make_param in
    let implicit_params =
      let (_pos, capability) = aast_contexts_to_decl_capability env ctxs p in
      { capability }
    in
    let ret = possibly_enforced_hint env h in
    let arity =
      match vh with
      | Some t -> Fvariadic (make_param t None)
      | None -> Fstandard
    in
    Tfun
      {
        ft_arity = arity;
        ft_tparams = [];
        ft_where_constraints = [];
        ft_params = paraml;
        ft_implicit_params = implicit_params;
        ft_ret = ret;
        ft_flags =
          make_ft_flags
            Ast_defs.FSync
            ~return_disposable:false
            ~returns_readonly:(readonly_opt readonly_ret)
            ~readonly_this:(readonly_opt ro);
        (* TODO: handle function parameters with <<CanCall>> *)
        ft_ifc_decl = default_ifc_fun_decl;
      }
  | Happly (id, argl) ->
    let id = Decl_env.make_decl_posed env id in
    let argl = List.map argl (hint env) in
    Tapply (id, argl)
  | Haccess ((_, Hvar n), [(_, id)]) -> Tgeneric ("T" ^ n ^ "@" ^ id, [])
  | Haccess (root_ty, ids) ->
    let root_ty = hint_ p env (snd root_ty) in
    let rec translate res ids =
      match ids with
      | [] -> res
      | id :: ids ->
        translate
          (Taccess
             ( mk (Typing_reason.Rhint (Decl_env.make_decl_pos env p), res),
               Decl_env.make_decl_posed env id ))
          ids
    in
    translate root_ty ids
  | Htuple hl ->
    let tyl = List.map hl (hint env) in
    Ttuple tyl
  | Hunion hl ->
    let tyl = List.map hl (hint env) in
    Tunion tyl
  | Hintersection hl ->
    let tyl = List.map hl (hint env) in
    Tintersection tyl
  | Hshape { nsi_allows_unknown_fields; nsi_field_map } ->
    let shape_kind =
      if nsi_allows_unknown_fields then
        Open_shape
      else
        Closed_shape
    in
    let fdm =
      List.fold_left
        ~f:(fun acc i ->
          TShapeMap.add
            (TShapeField.of_ast (Decl_env.make_decl_pos env) i.sfi_name)
            (shape_field_info_to_shape_field_type env i)
            acc)
        ~init:TShapeMap.empty
        nsi_field_map
    in
    Tshape (shape_kind, fdm)
  | Hsoft (p, h_) -> hint_ p env h_
  | Hfun_context n -> Tgeneric ("Tctx" ^ n, [])
  | Hvar n -> Tgeneric ("T" ^ n, [])

and possibly_enforced_hint env h =
  (* Initially we assume that a type is not enforced at runtime.
   * We refine this during localization
   *)
  { et_enforced = Unenforced; et_type = hint env h }
