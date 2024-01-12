(*
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Hh_prelude
open Typing_defs
module Reason = Typing_reason

class type ['a] decl_type_visitor_type =
  object
    method on_tany : 'a -> decl_phase Reason.t_ -> 'a

    method on_tmixed : 'a -> decl_phase Reason.t_ -> 'a

    method on_twildcard : 'a -> decl_phase Reason.t_ -> 'a

    method on_tnonnull : 'a -> decl_phase Reason.t_ -> 'a

    method on_tdynamic : 'a -> decl_phase Reason.t_ -> 'a

    method on_tthis : 'a -> decl_phase Reason.t_ -> 'a

    method on_tvec_or_dict :
      'a -> decl_phase Reason.t_ -> decl_ty -> decl_ty -> 'a

    method on_tgeneric :
      'a -> decl_phase Reason.t_ -> string -> decl_ty list -> 'a

    method on_toption : 'a -> decl_phase Reason.t_ -> decl_ty -> 'a

    method on_tlike : 'a -> decl_phase Reason.t_ -> decl_ty -> 'a

    method on_tprim : 'a -> decl_phase Reason.t_ -> Aast.tprim -> 'a

    method on_tvar : 'a -> decl_phase Reason.t_ -> Tvid.t -> 'a

    method on_type : 'a -> decl_ty -> 'a

    method on_tfun : 'a -> decl_phase Reason.t_ -> decl_fun_type -> 'a

    method on_tapply :
      'a -> decl_phase Reason.t_ -> pos_id -> decl_ty list -> 'a

    method on_ttuple : 'a -> decl_phase Reason.t_ -> decl_ty list -> 'a

    method on_tunion : 'a -> decl_phase Reason.t_ -> decl_ty list -> 'a

    method on_tintersection : 'a -> decl_phase Reason.t_ -> decl_ty list -> 'a

    method on_tshape : 'a -> decl_phase Reason.t_ -> decl_phase shape_type -> 'a

    method on_taccess :
      'a -> decl_phase Reason.t_ -> decl_phase taccess_type -> 'a

    method on_trefinement :
      'a -> decl_phase Reason.t_ -> decl_ty -> decl_class_refinement -> 'a

    method on_tnewtype :
      'a -> decl_phase Reason.t_ -> string -> decl_ty list -> decl_ty -> 'a
  end

class virtual ['a] decl_type_visitor : ['a] decl_type_visitor_type =
  object (this)
    method on_tany acc _ = acc

    method on_tmixed acc _ = acc

    method on_twildcard acc _ = acc

    method on_tnonnull acc _ = acc

    method on_tdynamic acc _ = acc

    method on_tthis acc _ = acc

    method on_tvec_or_dict acc _ ty1 ty2 =
      let acc = this#on_type acc ty1 in
      this#on_type acc ty2

    method on_tgeneric acc _ _ tyl =
      List.fold_left tyl ~f:this#on_type ~init:acc

    method on_toption acc _ ty = this#on_type acc ty

    method on_tlike acc _ ty = this#on_type acc ty

    method on_tprim acc _ _ = acc

    method on_tvar acc _ _ = acc

    method on_tfun acc _ { ft_params; ft_tparams; ft_ret; _ } =
      let acc =
        List.fold_left
          ~f:this#on_type
          ~init:acc
          (List.map ft_params ~f:(fun x -> x.fp_type))
      in
      let tparams =
        List.map ft_tparams ~f:(fun t -> List.map t.tp_constraints ~f:snd)
      in
      let acc =
        List.fold_left
          tparams
          ~f:(fun acc tp -> List.fold ~f:this#on_type ~init:acc tp)
          ~init:acc
      in
      this#on_type acc ft_ret

    method on_tapply acc _ _ tyl = List.fold_left tyl ~f:this#on_type ~init:acc

    method on_trefinement acc _ ty rs =
      let acc = this#on_type acc ty in
      Class_refinement.fold rs ~init:acc ~f:this#on_type

    method on_taccess acc _ (root, _ids) = this#on_type acc root

    method on_ttuple acc _ tyl = List.fold_left tyl ~f:this#on_type ~init:acc

    method on_tunion acc _ tyl = List.fold_left tyl ~f:this#on_type ~init:acc

    method on_tintersection acc _ tyl =
      List.fold_left tyl ~f:this#on_type ~init:acc

    method on_tshape
        acc _ { s_origin = _; s_unknown_value = kind; s_fields = fdm } =
      let acc = this#on_type acc kind in
      let f _ { sft_ty; _ } acc = this#on_type acc sft_ty in
      TShapeMap.fold f fdm acc

    method on_tnewtype acc _ _ tyl ty =
      let acc = List.fold_left tyl ~f:this#on_type ~init:acc in
      let acc = this#on_type acc ty in
      acc

    method on_type acc ty =
      let (r, x) = deref ty in
      match x with
      | Tany _ -> this#on_tany acc r
      | Tmixed -> this#on_tmixed acc r
      | Twildcard -> this#on_twildcard acc r
      | Tnonnull -> this#on_tnonnull acc r
      | Tdynamic -> this#on_tdynamic acc r
      | Tthis -> this#on_tthis acc r
      | Tvec_or_dict (ty1, ty2) -> this#on_tvec_or_dict acc r ty1 ty2
      | Tgeneric (s, args) -> this#on_tgeneric acc r s args
      | Toption ty -> this#on_toption acc r ty
      | Tlike ty -> this#on_tlike acc r ty
      | Tprim prim -> this#on_tprim acc r prim
      | Tfun fty -> this#on_tfun acc r fty
      | Tapply (s, tyl) -> this#on_tapply acc r s tyl
      | Trefinement (ty, rs) -> this#on_trefinement acc r ty rs
      | Taccess aty -> this#on_taccess acc r aty
      | Ttuple tyl -> this#on_ttuple acc r tyl
      | Tunion tyl -> this#on_tunion acc r tyl
      | Tintersection tyl -> this#on_tintersection acc r tyl
      | Tshape s -> this#on_tshape acc r s
      | Tnewtype (name, tyl, ty) -> this#on_tnewtype acc r name tyl ty
  end

class type ['a] locl_type_visitor_type =
  object
    method on_type : 'a -> locl_ty -> 'a

    method on_tany : 'a -> Reason.t -> 'a

    method on_tnonnull : 'a -> Reason.t -> 'a

    method on_tdynamic : 'a -> Reason.t -> 'a

    method on_toption : 'a -> Reason.t -> locl_ty -> 'a

    method on_tprim : 'a -> Reason.t -> Aast.tprim -> 'a

    method on_tvar : 'a -> Reason.t -> Tvid.t -> 'a

    method on_tfun : 'a -> Reason.t -> locl_fun_type -> 'a

    method on_tgeneric : 'a -> Reason.t -> string -> locl_ty list -> 'a

    method on_tnewtype :
      'a -> Reason.t -> string -> locl_ty list -> locl_ty -> 'a

    method on_tdependent : 'a -> Reason.t -> dependent_type -> locl_ty -> 'a

    method on_ttuple : 'a -> Reason.t -> locl_ty list -> 'a

    method on_tunion : 'a -> Reason.t -> locl_ty list -> 'a

    method on_tintersection : 'a -> Reason.t -> locl_ty list -> 'a

    method on_tunion : 'a -> Reason.t -> locl_ty list -> 'a

    method on_tintersection : 'a -> Reason.t -> locl_ty list -> 'a

    method on_tvec_or_dict : 'a -> Reason.t -> locl_ty -> locl_ty -> 'a

    method on_tshape : 'a -> Reason.t -> locl_phase shape_type -> 'a

    method on_tclass : 'a -> Reason.t -> pos_id -> exact -> locl_ty list -> 'a

    method on_class_refinement : 'a -> locl_class_refinement -> 'a

    method on_tlist : 'a -> Reason.t -> locl_ty list -> 'a

    method on_tunapplied_alias : 'a -> Reason.t -> string -> 'a

    method on_taccess : 'a -> Reason.t -> locl_phase taccess_type -> 'a

    method on_neg_type : 'a -> Reason.t -> neg_type -> 'a
  end

class virtual ['a] locl_type_visitor : ['a] locl_type_visitor_type =
  object (this)
    method on_tany acc _ = acc

    method on_tnonnull acc _ = acc

    method on_tdynamic acc _ = acc

    method on_toption acc _ ty = this#on_type acc ty

    method on_tprim acc _ _ = acc

    method on_tvar acc _ _ = acc

    method on_tfun acc _ { ft_params; ft_tparams; ft_ret; _ } =
      let acc =
        List.fold_left
          ~f:this#on_type
          ~init:acc
          (List.map ft_params ~f:(fun x -> x.fp_type))
      in
      let tparams =
        List.map ft_tparams ~f:(fun t -> List.map t.tp_constraints ~f:snd)
      in
      let acc =
        List.fold_left
          tparams
          ~f:(fun acc tp -> List.fold ~f:this#on_type ~init:acc tp)
          ~init:acc
      in
      this#on_type acc ft_ret

    method on_tgeneric acc _ _ tyl =
      List.fold_left tyl ~f:this#on_type ~init:acc

    method on_tnewtype acc _ _ tyl ty =
      let acc = List.fold_left tyl ~f:this#on_type ~init:acc in
      let acc = this#on_type acc ty in
      acc

    method on_tdependent acc _ _ ty =
      let acc = this#on_type acc ty in
      acc

    method on_ttuple acc _ tyl = List.fold_left tyl ~f:this#on_type ~init:acc

    method on_tunion acc _ tyl = List.fold_left tyl ~f:this#on_type ~init:acc

    method on_tintersection acc _ tyl =
      List.fold_left tyl ~f:this#on_type ~init:acc

    method on_tshape
        acc _ { s_origin = _; s_unknown_value = kind; s_fields = fdm } =
      let acc = this#on_type acc kind in
      let f _ { sft_ty; _ } acc = this#on_type acc sft_ty in
      TShapeMap.fold f fdm acc

    method on_tclass acc _ _ exact tyl =
      let acc =
        match exact with
        | Exact -> acc
        | Nonexact cr -> this#on_class_refinement acc cr
      in
      List.fold_left tyl ~f:this#on_type ~init:acc

    method on_class_refinement acc { cr_consts } =
      SMap.fold
        (fun _const_name { rc_bound; rc_is_ctx = _ } acc ->
          match rc_bound with
          | TRexact ty -> this#on_type acc ty
          | TRloose { tr_lower = l; tr_upper = u } ->
            let on_tlist acc = List.fold_left ~f:this#on_type ~init:acc in
            on_tlist (on_tlist acc u) l)
        cr_consts
        acc

    method on_tlist acc _ tyl = List.fold_left tyl ~f:this#on_type ~init:acc

    method on_tvec_or_dict acc _ ty1 ty2 =
      let acc = this#on_type acc ty1 in
      this#on_type acc ty2

    method on_neg_type acc r neg_ty =
      match neg_ty with
      | Neg_prim p -> this#on_tprim acc r p
      | Neg_class c -> this#on_tclass acc r c nonexact []

    method on_tunapplied_alias acc _ _ = acc

    method on_taccess acc _ (ty, _ids) = this#on_type acc ty

    method on_type acc ty =
      let (r, x) = deref ty in
      match x with
      | Tany _ -> this#on_tany acc r
      | Tnonnull -> this#on_tnonnull acc r
      | Tdynamic -> this#on_tdynamic acc r
      | Toption ty -> this#on_toption acc r ty
      | Tprim prim -> this#on_tprim acc r prim
      | Tvar id -> this#on_tvar acc r id
      | Tfun fty -> this#on_tfun acc r fty
      | Tgeneric (x, args) -> this#on_tgeneric acc r x args
      | Tnewtype (x, tyl, ty) -> this#on_tnewtype acc r x tyl ty
      | Tdependent (x, ty) -> this#on_tdependent acc r x ty
      | Ttuple tyl -> this#on_ttuple acc r tyl
      | Tunion tyl -> this#on_tunion acc r tyl
      | Tintersection tyl -> this#on_tintersection acc r tyl
      | Tshape s -> this#on_tshape acc r s
      | Tclass (cls, exact, tyl) -> this#on_tclass acc r cls exact tyl
      | Tvec_or_dict (ty1, ty2) -> this#on_tvec_or_dict acc r ty1 ty2
      | Tunapplied_alias n -> this#on_tunapplied_alias acc r n
      | Taccess (ty, ids) -> this#on_taccess acc r (ty, ids)
      | Tneg tneg -> this#on_neg_type acc r tneg
  end

class type ['a] internal_type_visitor_type =
  object
    inherit ['a] locl_type_visitor_type

    method on_internal_type : 'a -> internal_type -> 'a

    method on_constraint_type : 'a -> constraint_type -> 'a

    method on_locl_type : 'a -> locl_ty -> 'a

    method on_locl_type_list : 'a -> locl_ty list -> 'a

    method on_locl_type_option : 'a -> locl_ty option -> 'a

    method on_thas_member : 'a -> Reason.t -> has_member -> 'a

    method on_has_member : 'a -> Reason.t -> has_member -> 'a

    method on_thas_type_member : 'a -> Reason.t -> has_type_member -> 'a

    method on_has_type_member : 'a -> Reason.t -> has_type_member -> 'a

    method on_tcan_index : 'a -> Reason.t -> can_index -> 'a

    method on_tcan_traverse : 'a -> Reason.t -> can_traverse -> 'a

    method on_can_index : 'a -> Reason.t -> can_index -> 'a

    method on_can_traverse : 'a -> Reason.t -> can_traverse -> 'a

    method on_tdestructure : 'a -> Reason.t -> destructure -> 'a

    method on_destructure : 'a -> Reason.t -> destructure -> 'a

    method on_tcunion : 'a -> Reason.t -> locl_ty -> constraint_type -> 'a

    method on_tcintersection :
      'a -> Reason.t -> locl_ty -> constraint_type -> 'a
  end

class ['a] internal_type_visitor : ['a] internal_type_visitor_type =
  object (this)
    method on_internal_type acc ty =
      match ty with
      | LoclType ty -> this#on_locl_type acc ty
      | ConstraintType ty -> this#on_constraint_type acc ty

    method on_constraint_type acc ty =
      let (r, ty) = deref_constraint_type ty in
      match ty with
      | Thas_member hm -> this#on_thas_member acc r hm
      | Thas_type_member htm -> this#on_thas_type_member acc r htm
      | Tcan_index ci -> this#on_tcan_index acc r ci
      | Tcan_traverse ct -> this#on_tcan_traverse acc r ct
      | Tdestructure des -> this#on_tdestructure acc r des
      | TCunion (lty, cty) -> this#on_tcunion acc r lty cty
      | TCintersection (lty, cty) -> this#on_tcintersection acc r lty cty

    method on_locl_type acc ty = this#on_type acc ty

    method on_locl_type_list acc tyl =
      List.fold tyl ~init:acc ~f:this#on_locl_type

    method on_locl_type_option acc ty =
      Option.fold ty ~init:acc ~f:this#on_locl_type

    method on_thas_member acc r hm = this#on_has_member acc r hm

    method on_has_member acc _r hm =
      let { hm_name = _; hm_type; hm_class_id = _; hm_explicit_targs = _ } =
        hm
      in
      this#on_locl_type acc hm_type

    method on_thas_type_member acc r htm = this#on_has_type_member acc r htm

    method on_has_type_member acc _r htm =
      let { htm_id = _; htm_lower; htm_upper } = htm in
      let acc = this#on_locl_type acc htm_lower in
      let acc = this#on_locl_type acc htm_upper in
      acc

    method on_tcan_index acc r hm = this#on_can_index acc r hm

    method on_tcan_traverse acc r hm = this#on_can_traverse acc r hm

    method on_can_index acc _r ci =
      let acc = this#on_locl_type acc ci.ci_key in
      this#on_locl_type acc ci.ci_val

    method on_can_traverse acc _r ct =
      let acc = this#on_locl_type_option acc ct.ct_key in
      let acc = this#on_locl_type acc ct.ct_val in
      acc

    method on_tdestructure acc r des = this#on_destructure acc r des

    method on_destructure acc _r des =
      let { d_required; d_optional; d_variadic; d_kind = _ } = des in
      let acc = this#on_locl_type_list acc d_required in
      let acc = this#on_locl_type_list acc d_optional in
      let acc = this#on_locl_type_option acc d_variadic in
      acc

    method on_tcunion acc _r lty cty =
      let acc = this#on_locl_type acc lty in
      let acc = this#on_constraint_type acc cty in
      acc

    method on_tcintersection acc _r lty cty =
      let acc = this#on_locl_type acc lty in
      let acc = this#on_constraint_type acc cty in
      acc

    inherit ['a] locl_type_visitor
  end
