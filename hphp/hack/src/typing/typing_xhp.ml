(*
 * Copyright (c) 2017, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Hh_prelude
open Aast
open Common
open Typing_defs
module Env = Typing_env
module Phase = Typing_phase
module Reason = Typing_reason
module TUtils = Typing_utils
module MakeType = Typing_make_type
module Cls = Decl_provider.Class
module SN = Naming_special_names

let raise_xhp_required env pos ureason ty =
  let ty_str = lazy (Typing_print.error env ty) in
  let msgl =
    Lazy.map ty_str ~f:(fun ty_str ->
        Reason.to_string ("This is " ^ ty_str) (get_reason ty))
  in
  Typing_error.(
    xhp
    @@ Primary.Xhp.Xhp_required
         {
           pos;
           why_xhp = Reason.string_of_ureason ureason;
           ty_reason_msg = msgl;
         })

(**
 * Given class info, produces the subset of props that are XHP attributes
 *)
let xhp_attributes_for_class info : (string * class_elt) list =
  Cls.props info
  |> List.filter ~f:(fun (_, elt_) -> Option.is_some (get_ce_xhp_attr elt_))

(**
 * Walks a type and gathers all the XHP, adding an error when we encounter a
 * type that is not XHP.
 *)
let rec walk_and_gather_xhp_ ~env ~pos cty =
  let ((env, ty_err_opt), cty) =
    Typing_solver.expand_type_and_solve
      ~description_of_expected:"an XHP instance"
      env
      pos
      cty
  in
  Option.iter ~f:(Typing_error_utils.add_typing_error ~env) ty_err_opt;
  match get_node cty with
  | Tany _
  | Tdynamic ->
    (env, [], [])
  | Tunion tyl ->
    (* If it's a union, make sure it can only be XHP and add every
     * possible class. *)
    walk_list_and_gather_xhp env pos tyl
  | Tintersection tyl ->
    (* If any conjunct is dynamic then we let it pass *)
    if List.exists tyl ~f:is_dynamic then
      (env, [], [])
    else
      let (env, xhp, non_xhp) = walk_list_and_gather_xhp env pos tyl in
      (* ok to have non_xhp if there are some xhp *)
      let non_xhp =
        match xhp with
        | _ :: _ -> []
        | _ -> non_xhp
      in
      (env, xhp, non_xhp)
  | Tgeneric ("this", []) -> begin
    (* This is unsound, but we want to do best-effort checking
     * of attribute spreads even on XHP classes not marked `final`. We should
     * implement <<__ConsistentAttributes>> as a way to make this hacky
     * inference sound and check it before doing this conversion. *)
    match Env.get_self_ty env with
    | None -> (env, [], [])
    | Some ty -> walk_and_gather_xhp_ ~env ~pos ty
  end
  | Tgeneric _
  | Tdependent _
  | Tnewtype _ ->
    let (env, tyl) =
      TUtils.get_concrete_supertypes ~abstract_enum:true env cty
    in
    walk_list_and_gather_xhp env pos tyl
  | Tclass ((_, c), _, tyl) -> begin
    (* Here's where we actually check the declaration *)
    match Env.get_class env c with
    | Decl_entry.Found class_ when Cls.is_xhp class_ ->
      (env, [(cty, tyl, class_)], [])
    | _ -> (env, [], [cty])
  end
  | Tnonnull
  | Tvec_or_dict _
  | Toption _
  | Tprim _
  | Tvar _
  | Tfun _
  | Ttuple _
  | Tshape _
  | Tneg _ ->
    (env, [], [cty])
  | Taccess _
  | Tunapplied_alias _ ->
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
    |> List.fold ~init:SSet.empty ~f:(fun acc (k, _) -> SSet.add k acc)
  in
  let (env, possible_xhp, non_xhp) = walk_and_gather_xhp_ ~env ~pos cty in
  let xhp_required_err_opt =
    Typing_error.multiple_opt
    @@ List.map non_xhp ~f:(raise_xhp_required env pos Reason.URxhp_spread)
  in
  let xhp_to_attrs env (xhp_ty, tparams, xhp_info) =
    let attrs = xhp_attributes_for_class xhp_info in
    (* Compute the intersection and then localize the types *)
    let attrs = List.filter attrs ~f:(fun (k, _) -> SSet.mem k onto_attrs) in
    (* XHP does not allow generics in the class declaration, so
     * we don't need to perform any substitutions *)
    let ety_env =
      {
        empty_expand_env with
        this_ty = xhp_ty;
        substs = TUtils.make_locl_subst_for_class_tparams xhp_info tparams;
      }
    in
    List.map_env_ty_err_opt
      ~combine_ty_errs:Typing_error.multiple_opt
      ~f:
        begin
          fun env (k, ce) ->
            let (lazy ty) = ce.ce_type in
            let (env, ty) = Phase.localize ~ety_env env ty in
            (env, ((pos, k), (pos, ty)))
        end
      env
      attrs
  in
  let ((env, locl_ty_err_opt), attrs) =
    List.map_env_ty_err_opt
      ~f:xhp_to_attrs
      env
      possible_xhp
      ~combine_ty_errs:Typing_error.multiple_opt
  in
  let ty_err_opt =
    Option.merge ~f:Typing_error.both xhp_required_err_opt locl_ty_err_opt
  in
  ((env, ty_err_opt), List.concat attrs)

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
    (MakeType.nullable
       r
       (MakeType.union r [MakeType.dynamic r; ty_child; ty_traversable]))

let rewrite_xml_into_new pos sid attributes children =
  let cid = CI sid in
  let mk_attribute ix = function
    | Xhp_simple { xs_name = (attr_pos, attr_key); xs_expr = exp; _ } ->
      let attr_aux = attr_pos in
      let key = ((), attr_aux, Aast.String attr_key) in
      (key, exp)
    | Xhp_spread exp ->
      let attr_key = Format.asprintf "...$%s" (string_of_int ix) in
      let attr_key_ann = pos in
      let key = ((), attr_key_ann, Aast.String attr_key) in
      (key, exp)
  in
  let attributes =
    let attributes = List.mapi ~f:mk_attribute attributes in
    ((), pos, Darray (None, attributes))
  in
  let children = ((), pos, Varray (None, children)) in
  let file = ((), pos, Aast.String "") in
  let line = ((), pos, Aast.Int "1") in
  let args = [attributes; children; file; line] in
  let sid_ann = fst sid in
  ((), sid_ann, New (((), sid_ann, cid), [], args, None, ()))

let rewrite_attribute_access_into_call pos exp nullflavor =
  let obj_get_exp =
    ( (),
      pos,
      Aast.Obj_get
        (exp, ((), pos, Id (pos, "getAttribute")), nullflavor, Is_method) )
  in
  ( (),
    pos,
    Aast.Call
      {
        func = obj_get_exp;
        targs = [];
        args = [(Ast_defs.Pnormal, ((), pos, Aast.String ""))];
        unpacked_arg = None;
      } )
