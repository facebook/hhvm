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

let make_decl_ty env p ty_ =
  mk (Typing_reason.Rhint (Decl_env.make_decl_pos env p), ty_)

(* Unpacking a hint for typing *)
let rec hint env (p, h) =
  let ty_ = hint_ p env h in
  make_decl_ty env p ty_

and context_hint env (p, h) =
  match h with
  | Hfun_context n ->
    make_decl_ty env p (Tgeneric (Format.sprintf "T/[ctx %s]" n, []))
  | Haccess ((_, (Habstr (n, []) | Hvar n)), ids) ->
    let name =
      Format.sprintf
        "T/[%s::%s]"
        n
        (String.concat ~sep:"::" (List.map ~f:snd ids))
    in
    make_decl_ty env p (Tgeneric (name, []))
  | _ -> hint env (p, h)

and shape_field_info_to_shape_field_type
    env { sfi_optional; sfi_hint; sfi_name = _ } =
  { sft_optional = sfi_optional; sft_ty = hint env sfi_hint }

and aast_user_attribute_to_decl_user_attribute
    env { Aast_defs.ua_name; ua_params } =
  {
    Typing_defs.ua_name = Decl_env.make_decl_posed env ua_name;
    ua_params =
      List.filter_map ua_params ~f:(function
          | (_, _, Class_const ((_, _, CI (_, cls)), (_, name)))
            when String.equal name SN.Members.mClass ->
            Some (Typing_defs_core.Classname cls)
          | (_, _, Aast.EnumClassLabel (_, s)) ->
            Some (Typing_defs_core.EnumClassLabel s)
          | (_, _, Aast.String s) -> Some (Typing_defs_core.String s)
          | (_, _, Aast.Int i) -> Some (Typing_defs_core.Int i)
          | _ -> None);
  }

and aast_contexts_to_decl_capability env ctxs default_pos :
    Aast.pos * decl_ty capability =
  match ctxs with
  | Some (pos, hl) ->
    let dtys = List.map ~f:(context_hint env) hl in
    (* For coeffect contexts, in general we do not simplify empty or singleton
     * intersection, we keep them as is, so we don't use Typing_make_type
     * on purpose.
     * However the direct decl parser removes the intersection when the
     * context was a single Hfun_context. Let's do the same here
     *)
    let reason = Reason.Rhint (Decl_env.make_decl_pos env pos) in
    let dty =
      match dtys with
      | [dty] -> begin
        match get_node dty with
        | Tgeneric _ -> dty
        | _ -> mk (reason, Tintersection dtys)
      end
      | _ -> mk (reason, Tintersection dtys)
    in
    (pos, CapTy dty)
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
  | Hmixed -> Tmixed
  | Hwildcard -> Twildcard
  | Hnonnull -> Tnonnull
  | Hthis -> Tthis
  | Hdynamic -> Tdynamic
  | Hnothing -> Tunion []
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
    let argl = List.map argl ~f:(hint env) in
    Tgeneric (x, argl)
  | Hclass_args h ->
    let arg = hint env h in
    Tapply ((Decl_env.make_decl_pos env p, SN.Classes.cClassname), [arg])
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
            ~has_default:false
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
    let (variadic, paraml) =
      match vh with
      | Some t -> (true, paraml @ [make_param t None])
      | None -> (false, paraml)
    in
    Tfun
      {
        ft_tparams = [];
        ft_where_constraints = [];
        ft_params = paraml;
        ft_implicit_params = implicit_params;
        ft_ret = ret;
        ft_flags =
          Typing_defs_flags.Fun.make
            Ast_defs.FSync
            ~return_disposable:false
            ~returns_readonly:(readonly_opt readonly_ret)
            ~readonly_this:(readonly_opt ro)
            ~support_dynamic_type:false
            ~is_memoized:false
            ~variadic;
        (* TODO *)
        ft_cross_package = None;
      }
  | Happly (id, argl) ->
    let id = Decl_env.make_decl_posed env id in
    let argl = List.map argl ~f:(hint env) in
    Tapply (id, argl)
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
  | Hrefinement (root_ty, members) ->
    let root_ty = hint env root_ty in
    let class_ref =
      (* We do not err on duplicate refinements as the parser will
       * already have *)
      List.fold
        members
        ~init:{ cr_consts = SMap.empty }
        ~f:(fun { cr_consts } r ->
          let (id, rc) =
            match r with
            | Rctx ((_, id), CRexact h) ->
              (id, { rc_bound = TRexact (context_hint env h); rc_is_ctx = true })
            | Rctx (((_, id) as _pos), CRloose { cr_lower; cr_upper }) ->
              let decl_bounds h =
                Option.map ~f:(context_hint env) h |> Option.to_list
              in
              let tr_lower = decl_bounds cr_lower in
              let tr_upper = decl_bounds cr_upper in
              ( id,
                { rc_bound = TRloose { tr_lower; tr_upper }; rc_is_ctx = true }
              )
            | Rtype ((_, id), TRexact h) ->
              (id, { rc_bound = TRexact (hint env h); rc_is_ctx = false })
            | Rtype ((_, id), TRloose { tr_lower; tr_upper }) ->
              let decl_bounds = List.map ~f:(hint env) in
              let tr_lower = decl_bounds tr_lower in
              let tr_upper = decl_bounds tr_upper in
              ( id,
                { rc_bound = TRloose { tr_lower; tr_upper }; rc_is_ctx = false }
              )
          in
          { cr_consts = SMap.add id rc cr_consts })
    in
    Trefinement (root_ty, class_ref)
  | Htuple hl ->
    let tyl = List.map hl ~f:(hint env) in
    Ttuple tyl
  | Hunion hl ->
    let tyl = List.map hl ~f:(hint env) in
    Tunion tyl
  | Hintersection hl ->
    let tyl = List.map hl ~f:(hint env) in
    Tintersection tyl
  | Hshape { nsi_allows_unknown_fields; nsi_field_map } ->
    let shape_kind =
      hint
        env
        ( p,
          if nsi_allows_unknown_fields then
            Hmixed
          else
            Hnothing )
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
    Tshape
      {
        s_origin = Missing_origin;
        s_unknown_value = shape_kind;
        s_fields = fdm;
      }
  | Hsoft (p, h_) -> hint_ p env h_
  | Hfun_context _
  | Hvar _ ->
    Errors.internal_error p "Unexpected context hint";
    Tunion []

and possibly_enforced_hint env h =
  (* Initially we assume that a type is not enforced at runtime.
   * We refine this during localization
   *)
  { et_enforced = Unenforced; et_type = hint env h }
