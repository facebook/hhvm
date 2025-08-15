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
  mk (Typing_reason.hint (Decl_env.make_decl_pos env p), ty_)

(* Unpacking a hint for typing *)
let rec hint env (p, h) =
  let ty_ = hint_ p env h in
  make_decl_ty env p ty_

and context_hint env (p, h) =
  match h with
  | Hfun_context n ->
    make_decl_ty env p (Tgeneric (Format.sprintf "T/[ctx %s]" n))
  | Haccess ((_, (Habstr n | Hvar n)), ids) ->
    let name =
      Format.sprintf
        "T/[%s::%s]"
        n
        (String.concat ~sep:"::" (List.map ~f:snd ids))
    in
    make_decl_ty env p (Tgeneric name)
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
    ua_raw_val = None;
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
    let reason = Reason.hint (Decl_env.make_decl_pos env pos) in
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
          ( Typing_reason.vec_or_dict_key (Decl_env.make_decl_pos env p),
            Tprim Aast.Tarraykey )
    in
    Tvec_or_dict (t1, hint env h2)
  | Hprim p -> Tprim p
  | Habstr x -> Tgeneric x
  | Hclass_ptr (k, h) ->
    let h =
      match k with
      | CKclass -> h
      | CKenum -> (p, Happly ((p, SN.Classes.cHH_BuiltinEnum), [h]))
    in
    Tclass_ptr (hint env h)
  | Hoption h ->
    let h = hint env h in
    Toption h
  | Hlike h -> Tlike (hint env h)
  | Hfun
      {
        hf_tparams;
        hf_is_readonly = ro;
        hf_param_tys = hl;
        hf_param_info = pil;
        hf_variadic_ty = vh;
        hf_ctxs = ctxs;
        hf_return_ty = h;
        hf_is_readonly_return = readonly_ret;
      } ->
    let make_param ((p, _) as x) param_info =
      let (is_optional, readonly, named, splat, kind) =
        match param_info with
        | Some p ->
          let readonly =
            match p.hfparam_readonlyness with
            | Some Ast_defs.Readonly -> true
            | _ -> false
          in
          let named =
            match p.hfparam_named with
            | Some Ast_defs.Param_named -> true
            | _ -> false
          in
          let is_optional =
            match p.hfparam_optional with
            | Some Ast_defs.Optional -> true
            | _ -> false
          in
          let splat =
            match p.hfparam_splat with
            | Some Ast_defs.Splat -> true
            | _ -> false
          in
          let param_kind = get_param_mode p.hfparam_kind in
          (is_optional, readonly, named, splat, param_kind)
        | None -> (false, false, false, false, FPnormal)
      in
      {
        fp_pos = Decl_env.make_decl_pos env p;
        fp_name = None;
        fp_type = hint env x;
        fp_flags =
          make_fp_flags
            ~mode:kind
            ~accept_disposable:false
            ~is_optional
            ~readonly
            ~ignore_readonly_error:false
            ~splat
            ~named;
        fp_def_value = None;
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
    let ret = hint env h in
    let (variadic, paraml) =
      match vh with
      | Some t -> (true, paraml @ [make_param t None])
      | None -> (false, paraml)
    in
    let everything_sdt =
      TypecheckerOptions.everything_sdt (Decl_env.tcopt env)
    in
    let ft_tparams =
      List.map
        hf_tparams
        ~f:(fun { htp_name; htp_constraints; htp_user_attributes } ->
          let tp_name =
            let (pos, id) = htp_name in
            (Pos_or_decl.of_raw_pos pos, id)
          in
          let tp_constraints =
            List.map htp_constraints ~f:(fun (cstr_kind, cstr_hint) ->
                (cstr_kind, hint env cstr_hint))
          in

          let tp_user_attributes =
            List.map htp_user_attributes ~f:(fun (pos, id) ->
                let ua_name = (Pos_or_decl.of_raw_pos pos, id) in
                Typing_defs_core.{ ua_name; ua_params = []; ua_raw_val = None })
          in
          (* Add an implicit upper-bound if we are decling under implicit pessimisation
             and `NoAutoBound` isn't set *)
          let tp_constraints =
            if
              everything_sdt
              && not
                   (List.exists htp_user_attributes ~f:(fun (_, id) ->
                        String.equal
                          id
                          Naming_special_names.UserAttributes.uaNoAutoBound))
            then
              let pos = fst tp_name in
              let reason = Typing_reason.witness_from_decl pos in
              let ub =
                mk
                  ( reason,
                    Tapply
                      ( (pos, Naming_special_names.Classes.cSupportDyn),
                        [mk (reason, Tmixed)] ) )
              in
              (Ast_defs.Constraint_as, ub) :: tp_constraints
            else
              tp_constraints
          in

          Typing_defs_core.
            {
              tp_variance = Ast_defs.Invariant (* Meaningless here *);
              tp_name;
              tp_constraints;
              tp_reified = Erased;
              tp_user_attributes;
            })
    in
    Tfun
      {
        ft_tparams;
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
        ft_instantiated = List.is_empty ft_tparams;
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
             ( mk (Typing_reason.hint (Decl_env.make_decl_pos env p), res),
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
  | Htuple { tup_required; tup_extra } ->
    let t_required = List.map tup_required ~f:(hint env) in
    let t_extra =
      match tup_extra with
      | Hextra { tup_optional; tup_variadic } ->
        let t_optional = List.map tup_optional ~f:(hint env) in
        let t_variadic =
          match tup_variadic with
          | None -> hint env (p, Hnothing)
          | Some t -> hint env t
        in
        Textra { t_optional; t_variadic }
      | Hsplat tup_splat -> Tsplat (hint env tup_splat)
    in
    Ttuple { t_required; t_extra }
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
