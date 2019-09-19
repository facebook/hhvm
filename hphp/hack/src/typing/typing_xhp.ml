(*
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
module Env = Typing_env
module Phase = Typing_phase
module Reason = Typing_reason
module Subst = Decl_subst
module TUtils = Typing_utils
module MakeType = Typing_make_type
module Cls = Decl_provider.Class

let raise_xhp_required env pos ureason ty =
  let ty_str = Typing_print.error env ty in
  let msgl = Reason.to_string ("This is " ^ ty_str) (fst ty) in
  Errors.xhp_required pos (Reason.string_of_ureason ureason) msgl

(**
 * Given class info, produces the subset of props that are XHP attributes
 *)
let xhp_attributes_for_class info : (string * class_elt) Sequence.t =
  Cls.props info
  |> Sequence.filter ~f:(fun (_, elt_) -> Option.is_some elt_.ce_xhp_attr)

(**
 * Walks a type and gathers all the XHP, adding an error when we encounter a
 * type that is not XHP.
 *)
let rec walk_and_gather_xhp_ ~env ~pos cty =
  let (env, cty) =
    Typing_solver.expand_type_and_solve
      ~description_of_expected:"an XHP instance"
      env
      pos
      cty
      Errors.unify_error
  in
  match snd cty with
  | Tany _
  | Terr
  | Tdynamic ->
    (env, [], [])
  | Tunion tyl ->
    (* If it's unresolved, make sure it can only be XHP and add every
     * possible class. *)
    walk_list_and_gather_xhp env pos tyl
  | Tintersection tyl ->
    let (env, xhp, non_xhp) = walk_list_and_gather_xhp env pos tyl in
    (* ok to have non_xhp if there are some xhp *)
    let non_xhp =
      match xhp with
      | _ :: _ -> []
      | _ -> non_xhp
    in
    (env, xhp, non_xhp)
  | Tabstract (AKdependent DTthis, _) ->
    (* This is unsound, but we want to do best-effort checking
     * of attribute spreads even on XHP classes not marked `final`. We should
     * implement <<__ConsistentAttributes>> as a way to make this hacky
     * inference sound and check it before doing this conversion. *)
    walk_and_gather_xhp_ ~env ~pos (Env.get_self env)
  | Tabstract _ ->
    let (env, tyl) = TUtils.get_concrete_supertypes env cty in
    walk_list_and_gather_xhp env pos tyl
  | Tclass ((_, c), _, tyl) ->
    begin
      (* Here's where we actually check the declaration *)
      match Env.get_class env c with
      | Some class_ when Cls.is_xhp class_ -> (env, [(cty, tyl, class_)], [])
      | _ -> (env, [], [cty])
    end
  | Tnonnull
  | Tarraykind _
  | Toption _
  | Tprim _
  | Tvar _
  | Tfun _
  | Ttuple _
  | Tanon (_, _)
  | Tobject
  | Tshape _
  | Tdestructure _ ->
    (env, [], [cty])
  | Tpu_access _
  | Tpu _ ->
    (env, [], [cty])

and walk_list_and_gather_xhp env pos tyl =
  List.fold ~init:(env, [], []) tyl ~f:(fun (env, xhp_acc, non_xhp_acc) ty ->
      let (env, xhp_acc', non_xhp_acc') = walk_and_gather_xhp_ ~env ~pos ty in
      (env, xhp_acc' @ xhp_acc, non_xhp_acc' @ non_xhp_acc))

(**
 * For a given type, enforce that we have an XHP class and return the complete
 * list of possible attributes for typechecking the spread operator.
 *)
and get_spread_attributes env pos onto_xhp cty =
  let onto_attrs =
    xhp_attributes_for_class onto_xhp
    |> Sequence.fold ~init:SSet.empty ~f:(fun acc (k, _) -> SSet.add k acc)
  in
  let (env, possible_xhp, non_xhp) = walk_and_gather_xhp_ ~env ~pos cty in
  List.iter non_xhp ~f:(raise_xhp_required env pos Reason.URxhp_spread);
  let xhp_to_attrs env (xhp_ty, tparams, xhp_info) =
    let attrs = xhp_attributes_for_class xhp_info in
    (* Compute the intersection and then localize the types *)
    let attrs = Sequence.filter attrs (fun (k, _) -> SSet.mem k onto_attrs) in
    (* XHP does not allow generics in the class declaration, so
     * we don't need to perform any substitutions *)
    let ety_env =
      {
        type_expansions = [];
        this_ty = xhp_ty;
        substs = Subst.make_locl (Cls.tparams xhp_info) tparams;
        from_class = None;
      }
    in
    List.map_env
      ~f:
        begin
          fun env (k, ce) ->
          let (lazy ty) = ce.ce_type in
          let (env, ty) = Phase.localize ~ety_env env ty in
          (env, ((pos, k), (pos, ty)))
        end
      env
      (Sequence.to_list attrs)
  in
  let (env, attrs) = List.map_env ~f:xhp_to_attrs env possible_xhp in
  (env, List.concat attrs)

(**
 * Typing rules for the body expressions of an XHP literal.
 *)
let is_xhp_child env pos ty =
  let r = Reason.Rwitness pos in
  (* XHPChild *)
  let ty_child = MakeType.class_type r SN.Classes.cXHPChild [] in
  (* Any Traversable *)
  let ty_traversable = MakeType.traversable r (MakeType.mixed r) in
  Typing_solver.is_sub_type
    env
    ty
    (MakeType.nullable_locl r (r, Tunion [ty_child; ty_traversable]))
