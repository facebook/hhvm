(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Hh_prelude

module Instantiation = struct
  type t = {
    this: Typing_defs.locl_ty option;
    subst: Typing_defs.locl_ty SMap.t;
  }

  let empty = { this = None; subst = SMap.empty }

  let is_empty { this; subst } = Option.is_none this && SMap.is_empty subst

  let add_this ty { subst; this = _ } = { subst; this = Some ty }

  let add tparam ty { subst; this } = { subst = SMap.add tparam ty subst; this }
end

(** Given two types ty1 and ty2, assuming ty2 is an instantiation
    of ty1, return the type substitution to apply to ty1 to obtain ty2.
    This won't fail if ty2 is not an instantiation of ty1 but instead will return
    a best effort substitution. *)
let derive_instantiation
    env
    (uninstantiated : Typing_defs.decl_ty)
    (instantiated : Typing_defs.locl_ty) =
  let rec derive_instantiation
      (uninstantiated : Typing_defs.decl_ty)
      (instantiated : Typing_defs.locl_ty)
      ((env, subst_acc) : _ * Instantiation.t) : _ * Instantiation.t =
    let (env, instantiated) = Typing_env.expand_type env instantiated in
    let open Typing_defs_core in
    match
      (Typing_defs.get_node uninstantiated, Typing_defs.get_node instantiated)
    with
    (* base cases: Tgeneric, Tthis *)
    | (Tgeneric (lname, _), Tgeneric (rname, _)) when String.equal lname rname
      ->
      (env, subst_acc)
    | (Tgeneric (name, _targs), _) ->
      (* TODO if productionising second order generics: handle _targs *)
      (env, Instantiation.add name instantiated subst_acc)
    | (Tthis, _) -> (env, Instantiation.add_this instantiated subst_acc)
    (* Strip all like types and supportdyn on both sides *)
    | (Tlike ty, _) -> derive_instantiation ty instantiated (env, subst_acc)
    | (_, Tunion tyl) when List.exists tyl ~f:Typing_defs.is_dynamic ->
      let tyl =
        List.filter tyl ~f:(fun ty -> not (Typing_defs.is_dynamic ty))
      in
      derive_instantiation
        uninstantiated
        (Typing_make_type.union Typing_reason.none tyl)
        (env, subst_acc)
    | (Tapply ((_, lname), [ty]), _)
      when String.equal lname Naming_special_names.Classes.cSupportDyn ->
      derive_instantiation ty instantiated (env, subst_acc)
    | (_, Tnewtype (rname, [ty], _))
      when String.equal rname Naming_special_names.Classes.cSupportDyn ->
      derive_instantiation uninstantiated ty (env, subst_acc)
    (* All other recursive cases *)
    | ( Tapply ((_, lname), ltargs),
        (Tclass ((_, rname), _, rtargs) | Tnewtype (rname, rtargs, _)) )
      when String.equal lname rname ->
      derive_instantiation_list ltargs rtargs (env, subst_acc)
    | (Trefinement (ty, _), _) ->
      derive_instantiation ty instantiated (env, subst_acc)
    | (Toption lty, Toption rty)
    | (Tclass_ptr lty, Tclass_ptr rty) ->
      derive_instantiation lty rty (env, subst_acc)
    | ( Ttuple { t_required = ltyl; t_extra = lextra },
        Ttuple { t_required = rtyl; t_extra = rextra } ) ->
      let (env, subst_acc) =
        derive_instantiation_list ltyl rtyl (env, subst_acc)
      in
      (match (lextra, rextra) with
      | ( Textra { t_optional = lopt; t_variadic = lvar },
          Textra { t_optional = ropt; t_variadic = rvar } ) ->
        (env, subst_acc)
        |> derive_instantiation_list lopt ropt
        |> derive_instantiation lvar rvar
      | (Tsplat lty, Tsplat rty) -> derive_instantiation lty rty (env, subst_acc)
      | (_, (Textra _ | Tsplat _)) -> (env, subst_acc))
    | ( Tshape
          ({ s_origin = _; s_unknown_value = lty; s_fields = lfields } :
            _ shape_type),
        Tshape { s_origin = _; s_unknown_value = rty; s_fields = rfields } ) ->
      (env, subst_acc)
      |> derive_instantiation lty rty
      |> TShapeMap.fold
           (fun key { sft_ty = lty; sft_optional = _ } subst_acc ->
             match TShapeMap.find_opt key rfields with
             | None -> subst_acc
             | Some { sft_ty = rty; sft_optional = _ } ->
               derive_instantiation lty rty subst_acc)
           lfields
    | ( Tfun
          ({ ft_tparams = _; ft_params = lparams; ft_ret = lret; _ } :
            _ fun_type),
        Tfun { ft_tparams = _; ft_params = rparams; ft_ret = rret; _ } ) ->
      let get_param_tys params = List.map params ~f:(fun p -> p.fp_type) in
      (env, subst_acc)
      |> derive_instantiation_list
           (get_param_tys lparams)
           (get_param_tys rparams)
      |> derive_instantiation lret rret
    | (Tunion ltyl, Tunion rtyl)
    | (Tintersection ltyl, Tintersection rtyl) ->
      derive_instantiation_list ltyl rtyl (env, subst_acc)
    | (Tvec_or_dict (lty1, lty2), Tvec_or_dict (rty1, rty2)) ->
      (env, subst_acc)
      |> derive_instantiation lty1 rty1
      |> derive_instantiation lty2 rty2
    | (Taccess (lty, (_, lname)), Taccess (rty, (_, rname)))
      when String.equal lname rname ->
      derive_instantiation lty rty (env, subst_acc)
    | ( ( Tapply _ | Tmixed | Twildcard | Tany _ | Tnonnull | Tdynamic
        | Toption _ | Tprim _ | Tfun _ | Ttuple _ | Tshape _ | Tunion _
        | Tintersection _ | Tvec_or_dict _ | Taccess _ | Tclass_ptr _ ),
        ( Tany _ | Tnonnull | Tdynamic | Toption _ | Tprim _ | Tfun _ | Ttuple _
        | Tshape _ | Tgeneric _ | Tunion _ | Tintersection _ | Tvec_or_dict _
        | Taccess _ | Tclass_ptr _ | Tvar _ | Tnewtype _ | Tclass _
        | Tunapplied_alias _ | Tdependent _ | Tneg _ | Tlabel _ ) ) ->
      (env, subst_acc)
  and derive_instantiation_list utyl ityl (env, subst_acc) =
    let ulen = List.length utyl in
    let ilen = List.length ityl in
    let (utyl, ityl) =
      if ulen < ilen then
        (utyl, List.take ityl ulen)
      else if ulen > ilen then
        (List.take utyl ilen, ityl)
      else
        (utyl, ityl)
    in
    List.zip_exn utyl ityl
    |> List.fold
         ~init:(env, subst_acc)
         ~f:(fun (env, subst_acc) (uninstantiated, instantiated) ->
           derive_instantiation uninstantiated instantiated (env, subst_acc))
  in
  derive_instantiation uninstantiated instantiated (env, Instantiation.empty)
