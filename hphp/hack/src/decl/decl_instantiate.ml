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
module Subst = Decl_subst

let make_subst tparams tyl = Subst.make_decl tparams tyl

(*****************************************************************************)
(* Code dealing with instantiation. *)
(*****************************************************************************)

let rec instantiate subst (ty : decl_ty) =
  let merge_hk_type orig_r orig_var ty args =
    let (r, ty_) = deref ty in
    let res_ty_ =
      match ty_ with
      | Tapply (n, existing_args) ->
        (* We could insist on existing_args = [] here unless we want to support partial application. *)
        Tapply (n, existing_args @ args)
      | Tgeneric (n, existing_args) ->
        (* Same here *)
        Tgeneric (n, existing_args @ args)
      | _ ->
        (* We could insist on args = [] here, everything else is a kinding error *)
        ty_
    in
    mk (Reason.Rinstantiate (r, orig_var, orig_r), res_ty_)
  in

  (* PERF: If subst is empty then instantiation is a no-op. We can save a
   * significant amount of CPU by avoiding recursively deconstructing the ty
   * data type.
   *)
  if SMap.is_empty subst then
    ty
  else
    match deref ty with
    | (r, Tgeneric (x, args)) ->
      let args = List.map args (instantiate subst) in
      (match SMap.find_opt x subst with
      | Some x_ty -> merge_hk_type r x x_ty args
      | None -> mk (r, Tgeneric (x, args)))
    | (r, ty) ->
      let ty = instantiate_ subst ty in
      mk (r, ty)

and instantiate_ subst x =
  match x with
  | Tgeneric _ -> assert false
  (* IMPORTANT: We cannot expand Taccess during instantiation because this can
   * be called before all type consts have been declared and inherited
   *)
  | Taccess (ty, id) ->
    let ty = instantiate subst ty in
    Taccess (ty, id)
  | Tdarray (ty1, ty2) -> Tdarray (instantiate subst ty1, instantiate subst ty2)
  | Tvarray ty -> Tvarray (instantiate subst ty)
  | Tvarray_or_darray (ty1, ty2) ->
    let ty1 = instantiate subst ty1 in
    let ty2 = instantiate subst ty2 in
    Tvarray_or_darray (ty1, ty2)
  | Tvec_or_dict (ty1, ty2) ->
    let ty1 = instantiate subst ty1 in
    let ty2 = instantiate subst ty2 in
    Tvec_or_dict (ty1, ty2)
  | (Tthis | Tvar _ | Tmixed | Tdynamic | Tnonnull | Tany _ | Terr | Tprim _) as
    x ->
    x
  | Ttuple tyl ->
    let tyl = List.map tyl (instantiate subst) in
    Ttuple tyl
  | Tunion tyl ->
    let tyl = List.map tyl (instantiate subst) in
    Tunion tyl
  | Tintersection tyl ->
    let tyl = List.map tyl (instantiate subst) in
    Tintersection tyl
  | Toption ty ->
    let ty = instantiate subst ty in
    (* we want to avoid double option: ??T *)
    (match get_node ty with
    | Toption _ as ty_node -> ty_node
    | _ -> Toption ty)
  | Tlike ty -> Tlike (instantiate subst ty)
  | Tfun ft ->
    let tparams = ft.ft_tparams in
    let outer_subst = subst in
    let subst =
      List.fold_left
        ~f:
          begin
            fun subst t ->
            SMap.remove (snd t.tp_name) subst
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
      | Fvariadic ({ fp_type = var_ty; _ } as param) ->
        let var_ty = instantiate_possibly_enforced_ty subst var_ty in
        Fvariadic { param with fp_type = var_ty }
      | Fstandard as x -> x
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
        ft_tparams = tparams;
        ft_where_constraints = where_constraints;
      }
  | Tapply (x, tyl) ->
    let tyl = List.map tyl (instantiate subst) in
    Tapply (x, tyl)
  | Tshape (shape_kind, fdm) ->
    let fdm = ShapeFieldMap.map (instantiate subst) fdm in
    Tshape (shape_kind, fdm)

and instantiate_possibly_enforced_ty subst et =
  { et_type = instantiate subst et.et_type; et_enforced = et.et_enforced }

let instantiate_ce subst ({ ce_type = x; _ } as ce) =
  { ce with ce_type = lazy (instantiate subst (Lazy.force x)) }

let instantiate_cc subst ({ cc_type = x; _ } as cc) =
  let x = instantiate subst x in
  { cc with cc_type = x }

(* TODO(T88552052) is this necessary? Type consts are not allowed to
   reference type parameters, which is the substitution which is happening here *)
let instantiate_typeconst subst = function
  | TCAbstract
      { atc_as_constraint = a; atc_super_constraint = s; atc_default = d } ->
    TCAbstract
      {
        atc_as_constraint = Option.map a (instantiate subst);
        atc_super_constraint = Option.map s (instantiate subst);
        atc_default = Option.map d (instantiate subst);
      }
  | TCPartiallyAbstract { patc_constraint = a; patc_type = t } ->
    TCPartiallyAbstract
      { patc_constraint = instantiate subst a; patc_type = instantiate subst t }
  | TCConcrete { tc_type = t } -> TCConcrete { tc_type = instantiate subst t }

let instantiate_typeconst_type subst tc =
  { tc with ttc_kind = instantiate_typeconst subst tc.ttc_kind }
