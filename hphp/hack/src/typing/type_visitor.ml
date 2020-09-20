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
    method on_tany : 'a -> Reason.t -> 'a

    method on_terr : 'a -> Reason.t -> 'a

    method on_tmixed : 'a -> Reason.t -> 'a

    method on_tnonnull : 'a -> Reason.t -> 'a

    method on_tdynamic : 'a -> Reason.t -> 'a

    method on_tthis : 'a -> Reason.t -> 'a

    method on_tarray : 'a -> Reason.t -> decl_ty option -> decl_ty option -> 'a

    method on_tvarray_or_darray : 'a -> Reason.t -> decl_ty -> decl_ty -> 'a

    method on_tvarray : 'a -> Reason.t -> decl_ty -> 'a

    method on_tdarray : 'a -> Reason.t -> decl_ty -> decl_ty -> 'a

    method on_tgeneric : 'a -> Reason.t -> string -> decl_ty list -> 'a

    method on_toption : 'a -> Reason.t -> decl_ty -> 'a

    method on_tlike : 'a -> Reason.t -> decl_ty -> 'a

    method on_tprim : 'a -> Reason.t -> Aast.tprim -> 'a

    method on_tvar : 'a -> Reason.t -> Ident.t -> 'a

    method on_type : 'a -> decl_ty -> 'a

    method on_tfun : 'a -> Reason.t -> decl_fun_type -> 'a

    method on_tapply : 'a -> Reason.t -> Aast.sid -> decl_ty list -> 'a

    method on_ttuple : 'a -> Reason.t -> decl_ty list -> 'a

    method on_tunion : 'a -> Reason.t -> decl_ty list -> 'a

    method on_tintersection : 'a -> Reason.t -> decl_ty list -> 'a

    method on_tshape :
      'a ->
      Reason.t ->
      shape_kind ->
      decl_phase shape_field_type Nast.ShapeMap.t ->
      'a

    method on_taccess : 'a -> Reason.t -> taccess_type -> 'a
  end

class virtual ['a] decl_type_visitor : ['a] decl_type_visitor_type =
  object (this)
    method on_tany acc _ = acc

    method on_terr acc _ = acc

    method on_tmixed acc _ = acc

    method on_tnonnull acc _ = acc

    method on_tdynamic acc _ = acc

    method on_tthis acc _ = acc

    method on_tarray acc _ ty1_opt ty2_opt =
      let acc = Option.fold ~f:this#on_type ~init:acc ty1_opt in
      let acc = Option.fold ~f:this#on_type ~init:acc ty2_opt in
      acc

    method on_tvarray_or_darray acc _ ty1 ty2 =
      let acc = this#on_type acc ty1 in
      this#on_type acc ty2

    method on_tvarray acc _ ty = this#on_type acc ty

    method on_tdarray acc _ ty1 ty2 =
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
          (List.map ft_params (fun x -> x.fp_type.et_type))
      in
      let tparams =
        List.map ft_tparams (fun t -> List.map t.tp_constraints snd)
      in
      let acc =
        List.fold_left
          tparams
          ~f:(fun acc tp -> List.fold ~f:this#on_type ~init:acc tp)
          ~init:acc
      in
      this#on_type acc ft_ret.et_type

    method on_tapply acc _ _ tyl = List.fold_left tyl ~f:this#on_type ~init:acc

    method on_taccess acc _ (root, _ids) = this#on_type acc root

    method on_ttuple acc _ tyl = List.fold_left tyl ~f:this#on_type ~init:acc

    method on_tunion acc _ tyl = List.fold_left tyl ~f:this#on_type ~init:acc

    method on_tintersection acc _ tyl =
      List.fold_left tyl ~f:this#on_type ~init:acc

    method on_tshape acc _ _ fdm =
      let f _ { sft_ty; _ } acc = this#on_type acc sft_ty in
      Nast.ShapeMap.fold f fdm acc

    method on_type acc ty =
      let (r, x) = deref ty in
      match x with
      | Tany _ -> this#on_tany acc r
      | Terr -> this#on_terr acc r
      | Tmixed -> this#on_tmixed acc r
      | Tnonnull -> this#on_tnonnull acc r
      | Tdynamic -> this#on_tdynamic acc r
      | Tthis -> this#on_tthis acc r
      | Tarray (ty1_opt, ty2_opt) -> this#on_tarray acc r ty1_opt ty2_opt
      | Tdarray (ty1, ty2) ->
        this#on_type acc (mk (r, Tarray (Some ty1, Some ty2)))
      | Tvarray ty -> this#on_type acc (mk (r, Tarray (Some ty, None)))
      | Tvarray_or_darray (ty1, ty2) -> this#on_tvarray_or_darray acc r ty1 ty2
      | Tgeneric (s, args) -> this#on_tgeneric acc r s args
      | Toption ty -> this#on_toption acc r ty
      | Tlike ty -> this#on_tlike acc r ty
      | Tprim prim -> this#on_tprim acc r prim
      | Tvar id -> this#on_tvar acc r id
      | Tfun fty -> this#on_tfun acc r fty
      | Tapply (s, tyl) -> this#on_tapply acc r s tyl
      | Taccess aty -> this#on_taccess acc r aty
      | Ttuple tyl -> this#on_ttuple acc r tyl
      | Tunion tyl -> this#on_tunion acc r tyl
      | Tintersection tyl -> this#on_tintersection acc r tyl
      | Tshape (shape_kind, fdm) -> this#on_tshape acc r shape_kind fdm
      | Tpu_access (base, _) -> this#on_type acc base
  end

class type ['a] locl_type_visitor_type =
  object
    method on_tany : 'a -> Reason.t -> 'a

    method on_terr : 'a -> Reason.t -> 'a

    method on_tnonnull : 'a -> Reason.t -> 'a

    method on_tdynamic : 'a -> Reason.t -> 'a

    method on_toption : 'a -> Reason.t -> locl_ty -> 'a

    method on_tprim : 'a -> Reason.t -> Aast.tprim -> 'a

    method on_tvar : 'a -> Reason.t -> Ident.t -> 'a

    method on_type : 'a -> locl_ty -> 'a

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

    method on_tobject : 'a -> Reason.t -> 'a

    method on_tpu : 'a -> Reason.t -> locl_ty -> Aast.sid -> 'a

    method on_tpu_type_access : 'a -> Reason.t -> Aast.sid -> Aast.sid -> 'a

    method on_tvarray_or_darray : 'a -> Reason.t -> locl_ty -> locl_ty -> 'a

    method on_tvarray : 'a -> Reason.t -> locl_ty -> 'a

    method on_tdarray : 'a -> Reason.t -> locl_ty -> locl_ty -> 'a

    method on_tshape :
      'a ->
      Reason.t ->
      shape_kind ->
      locl_phase shape_field_type Nast.ShapeMap.t ->
      'a

    method on_tclass : 'a -> Reason.t -> Aast.sid -> exact -> locl_ty list -> 'a

    method on_tlist : 'a -> Reason.t -> locl_ty list -> 'a

    method on_tunapplied_alias : 'a -> Reason.t -> string -> 'a
  end

class virtual ['a] locl_type_visitor : ['a] locl_type_visitor_type =
  object (this)
    method on_tany acc _ = acc

    method on_terr acc _ = acc

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
          (List.map ft_params (fun x -> x.fp_type.et_type))
      in
      let tparams =
        List.map ft_tparams (fun t -> List.map t.tp_constraints snd)
      in
      let acc =
        List.fold_left
          tparams
          ~f:(fun acc tp -> List.fold ~f:this#on_type ~init:acc tp)
          ~init:acc
      in
      this#on_type acc ft_ret.et_type

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

    method on_tobject acc _ = acc

    method on_tshape acc _ _ fdm =
      let f _ { sft_ty; _ } acc = this#on_type acc sft_ty in
      Nast.ShapeMap.fold f fdm acc

    method on_tclass acc _ _ _ tyl =
      List.fold_left tyl ~f:this#on_type ~init:acc

    method on_tlist acc _ tyl = List.fold_left tyl ~f:this#on_type ~init:acc

    method on_tvarray_or_darray acc _ ty1 ty2 =
      let acc = this#on_type acc ty1 in
      this#on_type acc ty2

    method on_tvarray acc _ ty = this#on_type acc ty

    method on_tdarray acc _ ty1 ty2 =
      let acc = this#on_type acc ty1 in
      this#on_type acc ty2

    method on_tpu acc _ base _ = this#on_type acc base

    method on_tpu_type_access acc _ _ _ = acc

    method on_tunapplied_alias acc _ _ = acc

    method on_type acc ty =
      let (r, x) = deref ty in
      match x with
      | Tany _ -> this#on_tany acc r
      | Terr -> this#on_terr acc r
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
      | Tobject -> this#on_tobject acc r
      | Tshape (shape_kind, fdm) -> this#on_tshape acc r shape_kind fdm
      | Tclass (cls, exact, tyl) -> this#on_tclass acc r cls exact tyl
      | Tvarray ty -> this#on_tvarray acc r ty
      | Tdarray (ty1, ty2) -> this#on_tdarray acc r ty1 ty2
      | Tvarray_or_darray (ty1, ty2) -> this#on_tvarray_or_darray acc r ty1 ty2
      | Tpu (base, enum) -> this#on_tpu acc r base enum
      | Tpu_type_access (member, tyname) ->
        this#on_tpu_type_access acc r member tyname
      | Tunapplied_alias n -> this#on_tunapplied_alias acc r n
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
