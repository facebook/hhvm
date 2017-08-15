(**
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 *
 *)

open Core
open Typing_defs

module SN     = Naming_special_names
module Subst = Decl_subst

type subst = decl ty SMap.t

let make_subst tparams tyl = Subst.make tparams tyl

(*****************************************************************************)
(* Code dealing with instantiation. *)
(*****************************************************************************)

let rec instantiate subst (r, ty: decl ty) =
  (* PERF: If subst is empty then instantiation is a no-op. We can save a
   * significant amount of CPU by avoiding recursively deconstructing the ty
   * data type.
   *)
  if SMap.is_empty subst then (r, ty) else
  match ty with
  | Tgeneric x ->
      (match SMap.get x subst with
      | Some x_ty ->
        (Reason.Rinstantiate (fst x_ty, x, r), snd x_ty)
      | None ->
        (r, Tgeneric x)
      )
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
  | Tdarray (ty1, ty2) ->
      Tdarray (instantiate subst ty1, instantiate subst ty2)
  | Tvarray ty ->
      Tvarray (instantiate subst ty)
  | Tvarray_or_darray ty ->
      Tvarray_or_darray (instantiate subst ty)
  | Tthis -> Tthis
  | Tmixed -> Tmixed
  | Tany
  | Terr
  | Tprim _ as x -> x
  | Ttuple tyl ->
      let tyl = List.map tyl (instantiate subst) in
      Ttuple tyl
  | Toption ty ->
      let ty = instantiate subst ty in
      (* we want to avoid double option: ??T *)
      (match ty with
      | _, Toption _ -> snd ty
      | _ -> Toption ty
      )
  | Tfun ft ->
      let subst = List.fold_left ~f:begin fun subst (_, (_, x), _) ->
        SMap.remove x subst
      end ~init:subst ft.ft_tparams in
      let params = List.map ft.ft_params begin fun (name, param) ->
        let param = instantiate subst param in
        (name, param)
      end in
      let arity = match ft.ft_arity with
        | Fvariadic (min, (name, var_ty)) ->
          let var_ty = instantiate subst var_ty in
          Fvariadic (min, (name, var_ty))
        | Fellipsis _ | Fstandard _ as x -> x
      in
      let ret = instantiate subst ft.ft_ret in
      let tparams = List.map ft.ft_tparams begin fun (var, name, cstrl) ->
        (var, name, List.map cstrl
           (fun (ck, ty) -> (ck, instantiate subst ty))) end in
      let where_constraints = List.map ft.ft_where_constraints
          begin (fun (ty1, ck, ty2) ->
            (instantiate subst ty1, ck, instantiate subst ty2)) end in
      Tfun { ft with ft_arity = arity; ft_params = params;
                     ft_ret = ret; ft_tparams = tparams;
                     ft_where_constraints = where_constraints }
  | Tapply (x, tyl) ->
      let tyl = List.map tyl (instantiate subst) in
      Tapply (x, tyl)
  | Tshape (fields_known, fdm) ->
      let fdm = ShapeFieldMap.map (instantiate subst) fdm in
      Tshape (fields_known, fdm)

let instantiate_ce subst ({ ce_type = x; _ } as ce) =
  { ce with ce_type = lazy (instantiate subst (Lazy.force x)) }

let instantiate_cc subst ({ cc_type = x; _ } as cc) =
  let x = instantiate subst x in
  { cc with cc_type = x }

let instantiate_typeconst subst (
  { ttc_constraint = x; ttc_type = y; _ } as tc) =
    let x = Option.map x (instantiate subst) in
    let y = Option.map y (instantiate subst) in
    { tc with ttc_constraint = x; ttc_type = y }
