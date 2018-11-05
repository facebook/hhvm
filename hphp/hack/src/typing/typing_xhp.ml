(**
 * Copyright (c) 2017, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Core_kernel
open Common
open Typing_defs

module Env          = Typing_env
module Phase        = Typing_phase
module Reason       = Typing_reason
module Subst        = Decl_subst
module SubType      = Typing_subtype
module TUtils       = Typing_utils

let raise_xhp_required pos ureason ty =
  let ty_str = Typing_print.error (snd ty) in
  let msgl = Reason.to_string ("This is "^ty_str) (fst ty) in
  Errors.xhp_required pos (Reason.string_of_ureason ureason) msgl

(**
 * Given class info, produces the subset of props that are XHP attributes
 *)
let xhp_attributes_for_class info: Typing_defs.class_elt SMap.t =
  SMap.filter (fun _ elt_ -> elt_.ce_is_xhp_attr) info.tc_props

(**
 * Walks a type and gathers all the XHP, adding an error when we encounter a
 * type that is not XHP.
 *)
let rec walk_and_gather_xhp_ ~env ~ureason ~pos cty =
  let env, cty = Env.expand_type env cty in
  match (snd cty) with
  | Tany
  | Terr
  | Tdynamic -> env, []
  | Tunresolved tyl ->
      (* If it's unresolved, make sure it can only be XHP and add every
       * possible class. *)
      let (env, acc), _ = List.map_env (env, []) tyl (fun (env, acc) ty ->
        let env, acc' = walk_and_gather_xhp_ ~env ~ureason ~pos ty in
        (env, acc' @ acc), ty)
      in
      env, acc
  | Tabstract (AKdependent (`this, _), _) ->
      (* This is unsound, but we want to do best-effort checking
       * of attribute spreads even on XHP classes not marked `final`. We should
       * implement <<__ConsistentAttributes>> as a way to make this hacky
       * inference sound and check it before doing this conversion. *)
      walk_and_gather_xhp_ ~env ~ureason ~pos (Env.get_self env)
  | Tabstract _ ->
      let env, tyl = TUtils.get_concrete_supertypes env cty in
      let (env, acc), _ = List.map_env (env, []) tyl (fun (env, acc) ty ->
        let env, acc' = walk_and_gather_xhp_ ~env ~ureason ~pos ty in
        (env, acc' @ acc), ty)
      in
      env, acc
  | Tclass ((_, c), tyl) -> begin
      (* Here's where we actually check the declaration *)
      match Env.get_class env c with
      | Some class_ when class_.tc_is_xhp -> (env, [cty, tyl, class_])
      | _ -> (raise_xhp_required pos ureason cty; env, [])
  end
  | (Tnonnull | Tarraykind _ | Toption _
       | Tprim _ | Tvar _ | Tfun _ | Ttuple _ | Tanon (_, _) | Tobject
       | Tshape _) -> (raise_xhp_required pos ureason cty; env, [])

(**
 * For a given type, enforce that we have an XHP class and return the complete
 * list of possible attributes for typechecking the spread operator.
 *)
and get_spread_attributes env pos onto_xhp cty =
  let onto_attrs = xhp_attributes_for_class onto_xhp in
  let env, possible_xhp =
    walk_and_gather_xhp_ ~env ~ureason:Reason.URxhp_spread ~pos cty in
  let xhp_to_attrs env (xhp_ty, tparams, xhp_info) =
    let attrs = xhp_attributes_for_class xhp_info in
    (* Compute the intersection and then localize the types *)
    let attrs = SMap.filter (fun k _ -> SMap.mem k onto_attrs) attrs in
    (* XHP does not allow generics in the class declaration, so
     * we don't need to perform any substitutions *)
    let ety_env = {
      type_expansions = [];
      this_ty = xhp_ty;
      substs = Subst.make xhp_info.tc_tparams tparams;
      from_class = None;
      validate_dty = None;
    } in
    List.map_env ~f:begin fun env (k, ce) ->
      let lazy ty = ce.ce_type in
      let env, ty = Phase.localize ~ety_env env ty in
      env, ((pos, k), (pos, ty))
    end env (SMap.bindings attrs)
  in
  let env, attrs = List.map_env ~f:xhp_to_attrs env possible_xhp in
  env, List.concat attrs

(**
 * Typing rules for the body expressions of an XHP literal.
 * Until XHPChild type inference is improved, there is a cascading set of
 * checks here to allow for parity with what the runtime accepts.
 *)
let is_xhp_child env pos ty =
  let reason = Reason.Rwitness pos in
  (* ?XHPChild *)
  let ty_child = reason, Tclass ((Pos.none, SN.Classes.cXHPChild), []) in
  let ty_child = reason, Toption ty_child in
  (* Any ?Traversable *)
  let ty_traversable =
    reason, Tclass ((Pos.none, SN.Collections.cTraversable), [Reason.none, TUtils.tany env]) in
  let ty_traversable = reason, Toption ty_traversable in
  let tys = [ty_child; ty_traversable] in
  List.exists ~f:(fun super -> SubType.is_sub_type env ty super) tys
