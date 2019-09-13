(*
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Core_kernel
open Typing_defs
module SN = Naming_special_names
module Subst = Decl_subst

let make_subst tparams tyl = Subst.make tparams tyl

(*****************************************************************************)
(* Code dealing with instantiation. *)
(*****************************************************************************)

let rec instantiate subst ((r, ty) : decl_ty) =
  (* PERF: If subst is empty then instantiation is a no-op. We can save a
   * significant amount of CPU by avoiding recursively deconstructing the ty
   * data type.
   *)
  if SMap.is_empty subst then
    (r, ty)
  else
    match ty with
    | Tgeneric x ->
      (match SMap.get x subst with
      | Some x_ty -> (Reason.Rinstantiate (fst x_ty, x, r), snd x_ty)
      | None -> (r, Tgeneric x))
    | _ ->
      let ty = instantiate_ subst ty in
      (r, ty)

and instantiate_ subst x =
  match x with
  | Tgeneric _ -> assert false
  (* IMPORTANT: We cannot expand Taccess during instantiation because this can
   * be called before all type consts have been declared and inherited
   *)
  | Taccess (ty, ids) ->
    let ty = instantiate subst ty in
    Taccess (ty, ids)
  | Tarray (ty1, ty2) ->
    let ty1 = Option.map ty1 (instantiate subst) in
    let ty2 = Option.map ty2 (instantiate subst) in
    Tarray (ty1, ty2)
  | Tdarray (ty1, ty2) -> Tdarray (instantiate subst ty1, instantiate subst ty2)
  | Tvarray ty -> Tvarray (instantiate subst ty)
  | Tvarray_or_darray ty -> Tvarray_or_darray (instantiate subst ty)
  | ( Tthis | Tvar _ | Tmixed | Tdynamic | Tnonnull | Tany _ | Terr | Tprim _
    | Tnothing ) as x ->
    x
  | Ttuple tyl ->
    let tyl = List.map tyl (instantiate subst) in
    Ttuple tyl
  | Toption ty ->
    let ty = instantiate subst ty in
    (* we want to avoid double option: ??T *)
    (match ty with
    | (_, Toption _) -> snd ty
    | _ -> Toption ty)
  | Tlike ty -> Tlike (instantiate subst ty)
  | Tfun ft ->
    let (tparams, instantiate_tparams) = ft.ft_tparams in
    let outer_subst = subst in
    let subst =
      List.fold_left
        ~f:begin
             fun subst t -> SMap.remove (snd t.tp_name) subst
           end
        ~init:subst
        tparams
    in
    let params =
      List.map ft.ft_params (fun param ->
          let ty = instantiate_possibly_enforced_ty subst param.fp_type in
          { param with fp_type = ty })
    in
    let arity =
      match ft.ft_arity with
      | Fvariadic (min, ({ fp_type = var_ty; _ } as param)) ->
        let var_ty = instantiate_possibly_enforced_ty subst var_ty in
        Fvariadic (min, { param with fp_type = var_ty })
      | (Fellipsis _ | Fstandard _) as x -> x
    in
    let ret = instantiate_possibly_enforced_ty subst ft.ft_ret in
    let tparams =
      List.map tparams (fun t ->
          {
            t with
            tp_constraints =
              List.map t.tp_constraints (fun (ck, ty) ->
                  (ck, instantiate subst ty));
          })
    in
    let where_constraints =
      List.map ft.ft_where_constraints (fun (ty1, ck, ty2) ->
          (instantiate subst ty1, ck, instantiate outer_subst ty2))
    in
    Tfun
      {
        ft with
        ft_arity = arity;
        ft_params = params;
        ft_ret = ret;
        ft_tparams = (tparams, instantiate_tparams);
        ft_where_constraints = where_constraints;
      }
  | Tapply (x, tyl) ->
    let tyl = List.map tyl (instantiate subst) in
    Tapply (x, tyl)
  | Tshape (shape_kind, fdm) ->
    let fdm = ShapeFieldMap.map (instantiate subst) fdm in
    Tshape (shape_kind, fdm)
  | Tpu_access (base, sid) -> Tpu_access (instantiate subst base, sid)

and instantiate_possibly_enforced_ty subst et =
  { et_type = instantiate subst et.et_type; et_enforced = et.et_enforced }

let instantiate_ce subst ({ ce_type = x; _ } as ce) =
  { ce with ce_type = lazy (instantiate subst (Lazy.force x)) }

let instantiate_cc subst ({ cc_type = x; _ } as cc) =
  let x = instantiate subst x in
  { cc with cc_type = x }

let instantiate_typeconst
    subst ({ ttc_abstract = abs; ttc_constraint = x; ttc_type = y; _ } as tc) =
  let abs =
    match abs with
    | TCAbstract default_opt ->
      TCAbstract (Option.map default_opt (instantiate subst))
    | _ -> abs
  in
  let x = Option.map x (instantiate subst) in
  let y = Option.map y (instantiate subst) in
  { tc with ttc_abstract = abs; ttc_constraint = x; ttc_type = y }
